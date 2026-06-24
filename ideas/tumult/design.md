# Idea: tumult — the game

## What This Is

`tumult` is **the game** — an Across-the-Obelisk-style, buff/debuff-heavy,
card-driven combat library. It is not a host. It is not Unreal. It is the
game itself: characters, cards, statuses, an encounter loop, and the
resolution rules that turn authored intent into observable fact. Hosts
(Unreal today, a terminal later) hook into it as edges.

It runs on [`rpgkit`](https://github.com/KirkDiggler/rpgkit) — the
Bus/Chain/Action/Effect nervous system extracted from the Go toolkit. With
rpgkit's v0.3.0 observation API landed (`Action::activate` and
`Effect::apply`/`remove` return `(Status, …Receipt)`, `Chain` breakdowns
carry `source`), the receipts now exist to drive breakdowns and combat logs
from structured data instead of ad-hoc host strings. `tumult` is the first
real consumer of those receipts.

The immediate impetus: combat-encounter logic currently lives **inline in
`rpgkit-ue`** (the Unreal workshop repo) — `ARPGKitGameMode` plus the
just-extracted `URPGKitEncounterRuntime` (PR #4, still open). That code is a
prototype of *the game* trapped inside a host. The clean move is to extract
that prototype into a real, bounded, testable library and let UE be an edge
again. rpgkit-demo-game (the qwen terminal experiment) is positioned to
become a second edge that consumes the same engine.

## Key Framing

- **rpgkit is a dependency, not an edge.** tumult depends on rpgkit core
  via FetchContent at a pinned release. Hosts (UE, terminal) depend on
  *both*. tumult never appears in rpgkit; rpgkit never names tumult.
- **A game, not a game implementation.** tumult knows the game —
  characters, statuses, intent, the combat loop — and never host types
  (`UClass`, `UFUNCTION`, `std::istream`-driven UX). UE types and terminal
  I/O are *edges* that wrap a `tumult::Encounter`.
- **Receipts are primary observability.** Every resolution surface flows
  through `(Status, Receipt)`. The breakdown's "who granted each modifier"
  question is answered by `Chain::Step::source`, not a baked-in string.
  The HUD explanation tool consumes receipt-shaped facts; an explanation
  formatter is testable in plain C++ with no host.
- **Cross-repo discipline (this is a multi-repo change).** Per
  `rpg-project/CLAUDE.md`, this design lives in `rpg-project/ideas/tumult/`
  and the eventual implementation plan cites the board use cases on
  `rpgkit-ue` that proved the pressure (board #15: `rpgkit-ue#9`, `#14`,
  `#15`, `#16`).
- **Issue-first, test-first, no noisy back-and-forth while tests are green.**
  tumult is fully Linux-buildable / GoogleTest-able. Forward motion during
  the cross-platform gap (no Windows access) is gated on green tests; UE
  hookup is the *last* Windows step, not a gate at each commit.

## Boundaries

```text
tumult        owns the GAME (characters, statuses, encounter loop, resolution)
              knows rpgkit core types only; never UObject / std::istream UX

rpgkit core   owns the nervous system (Bus/Chain/Action/Effect/Receipts);
              system-agnostic; never names "rage", "bleed", "block"

rpgkit-ue     owns *the Unreal facet*; renders, hosts input; reflects
              tumult state into Blueprint/HUD; consumes no rules
rpgkit-demo-game   owns the terminal facet; renders; consumes no rules
```

### What tumult owns

- `Character` (id, name, curHP/maxHP, block).
- `Encounter` (owns a `rpg::core::Bus`, its characters, the active `Effect`
  set, and subscriptions to request topics; drives the turn loop / resolver
  calls).
- Cards as `Action`s, statuses/buffs/debuffs as `Effect`s, intent (enemy
  cards), energy, target modes — grown **slice by slice**, not all on day
  one.
- A breakdown explanation formatter (`formatStep`) that reads receipt data
  (not hardcoded strings) and is the simplification proof.

### What tumult deliberately does NOT own

- UE types, Blueprint reflection, UMG widgets, asset authoring.
- Terminal input parsing, terminal rendering.
- Persistence / save state / networking / replication.
- The eighteenth buff; the final boss. YAGNI — one concrete use case at a
  time.

## Architecture

### Module layout (repo)

```text
tumult/
  CMakeLists.txt           FetchContent-pins rpgkit v0.3.0; defines `tumult`
  Makefile                 build / test / fmt-clean (mirror rpgkit's)
  AGENTS.md                 issue-first, merge-never-rebase, test-first rules
  CLAUDE.md (repo-local)   binding decisions cross-linked from rpg-project
  include/tumult/
    character.hpp
    encounter.hpp
    effects/{vulnerable,tough_skin,bleed}.hpp
    breakdown.hpp            BreakdownStep + formatStep
  src/*                      compiled TUs only where header-inline is wrong
  tests/                     GoogleTest via FetchContent; the simplification proof
  docs/status.md             living doc — current health, active, paused, edges
  docs/tutorials/ (later)     how to host tumult at an edge
```

Header-leaning, compiled TUs only when header-inlining hurts build times or
ABI — same posture as `rpgkit` core today. C++20. (rpgkit's binding decision
2; inherited.)

### Core types

```cpp
namespace tumult {

struct Character {
  std::string id;      // "hero", "goblin"
  std::string name;    // display
  int curHP = 0;
  int maxHP = 0;
  int block = 0;
  [[nodiscard]] bool isAlive() const { return curHP > 0; }
};

// One step of a folded chain, mirrored (cheap struct copy) from
// rpg::core::Chain<DamageEvent>::Result::breakdown for host consumption.
struct BreakdownStep {
  std::string id;      // modifier id ("vulnerable-goblin")
  std::string stage;   // "base" | "effects" | "final"
  std::string source;  // effect source ("vulnerable") — the receipt-driven bit
  int before = 0;
  int after  = 0;
};

struct StrikeResult {
  int finalDamage = 0;
  int blocked     = 0;
  int hpAfter     = 0;
  std::vector<BreakdownStep> breakdown;
};

//Formatter — the simplification proof: reads receipt `source`, not string.
//Plain C++; tested in tests/format_test.cpp; mirrored by host renderers.
std::string formatStep(const BreakdownStep& s);

}  // namespace tumult
```

### Encounter

`Encounter` owns a `rpg::core::Bus` and exposes the verbs that were inline
in rpgkit-ue's `URPGKitEncounterRuntime`:

```cpp
class Encounter {
 public:
  void setup(const Character& hero, const Character& enemy);
  void shutdown();   // unsubscribes request topics, removes effects

  // Damage resolution. Publishes combat.damage (chained), executes the
  // chain, applies block, mutates HP. Modifiers contributed by effects
  // carry .source — captured by the returned breakdown.
  StrikeResult strike(const std::string& attacker, const std::string& target, int base);

  void dealRawDamage(const std::string& target, int amount);
  void addBlock(const std::string& fighter, int amount);
  void clearAllBlock();

  // Effect lifecycle consumes the v0.3.0 receipt form.
  std::pair<rpg::core::Status, rpg::core::EffectReceipt>
  applyEffect(std::unique_ptr<rpg::core::Effect>);
  std::pair<rpg::core::Status, rpg::core::EffectReceipt>
  removeEffect(const std::string& effectId);

  const Character* findCharacter(const std::string& id) const;
  rpg::core::Bus& bus();

  // Receipt-friendly access (later slices route these to observers; today
  // they are the test seam for "what happened").
  const std::vector<std::string>& recentCombatLog() const;
};
```

### Damage resolution — where the simplification lives

```text
Encounter::strike(attacker, target, base)
  -> publish combat.damage (ChainedTopic<DamageEvent>)
       each subscribed Effect calls chain.add({
            .stage = "effects",
            .id    = "vulnerable-goblin",
            .source= "vulnerable",          <-- THE new field
            .modifier = ...})
  -> chain.execute(damageEvent)
  -> breakdown[i].source is populated BY THE CHAIN, not interpolated
  -> StrikeResult.breakdown copies Step -> BreakdownStep (1:1, no behavior)
  -> formatStep(step) -> "effects / vulnerable-goblin (vulnerable): 5 -> 8"
  -> host renderer (UE HUD later, terminal renderer later) maps BreakdownStep
     to its native string/widget — never re-derives the source.
```

Today's rpgkit-ue `GetDamageBreakdownSummary` produces
`"%s / %s: %d -> %d"` with `ModifierId` and never shows *who granted* the
modifier. The "who" was implicit (Vulnerable adds, Tough Skin subtracts) and
only knowable by reading the effect source. `tumult`'s contract is: the
chain receipt carries `source`; the formatter reads it; the HUD explains it.
That is the simplification v0.3.0 bought us, proven by a single GoogleTest.

### Effect lifecycle

`applyEffect`/`removeEffect` mirror the v0.3.0 `(Status, EffectReceipt)`
pair. In the first slice the `EffectReceipt` is not yet routed to a HUD
observer — that is `rpgkit-ue#14` and lives in a later slice. But the
plumbing is the v0.3.0 shape so later slices insert observers without a
signature change.

The first-slice effects ported from rpgkit-ue `RPGKitEffect.cpp`:

- `VulnerableEffect` — source `"vulnerable"`, +PercentBonus for N turns;
  ticks down on `turn.ended`.
- `ToughSkinEffect` — source `"tough-skin"`, flat reduction to the
  protected entity.
- `BleedEffect` — source `"rend"`, stacking DoT; ticks on `turn.ended`,
  publishes `raw_damage.requested`.

## Data flow

```text
Host (UE / terminal) calls Encounter::strike(attacker, target, base)
  -> Encounter publishes combat.damage (chained) on its Bus
  -> each applied Effect's chained subscriber calls chain.add({..., .source})
  -> chain.execute() fills Result.breakdown[i].source
  -> Encounter copies BreakdownStep[]; returns StrikeResult
  -> Host reads BreakdownStep (or formatStep(step)) and renders
```

Effects' own state changes (Vulnerable `RemainingTurns` decrement, Bleed
stacks, Tough Skin none) happen in the `turn.ended` notification topic
subscriber, same as rpgkit-ue today. Bleed's tick publishes
`raw_damage.requested`; the Encounter's topic handler applies flat damage to
the target Character. Block request topic similarly.

## Error handling

- Fallible operations return `rpg::core::Status`; never `(nil, nil)` (the Go
  rule ported by rpgkit binding decision 1). A failed `chain.add` returns
  its `Status` to the publishing subscriber; the publish stops at the first
  error (rpgkit decision 8: synchronous, ordered, fail-fast).
- Receipts are populated **even on failure** (rpgkit v0.3.0 contract), so
  an effect that fails mid-apply still yields a `EffectReceipt` with id and
  source — the host can explain "what failed and where."
- `Encounter::findCharacter` returns `const Character*` (nullable) — not a
  pretend dummy — but every public verb asserts its ids against `setup`'s
  roster before mutating; an unknown target yields a `Status::error`.

## Testing

GoogleTest via FetchContent, mirroring rpgkit core's harness. Tests live in
`tumult/tests/`:

- `character_test.cpp` — `isAlive`, HP bounds.
- `encounter_test.cpp` —
  - setup populates hero/goblin; `findCharacter` returns both;
  - `strike` against an unmodified target → exactly one `base` step with
    `source == ""` and `after == before == base`; final == base.
  - `strike` vs Vulnerable (source `"vulnerable"`, +50%) on the target →
    one `effects` step with `source == "vulnerable"`, `after == ceil(before*1.5)`.
  - `strike` vs Tough Skin (source `"tough-skin"`, -1) on the target → one
    `effects` step with `source == "tough-skin"`, `after == before - 1`.
  - block absorption is applied after chain execute: `StrikeResult.blocked`
    and `hpAfter` reflect it; block is reduced by the blocked amount.
  - `dealRawDamage` (Bleed tick via the request topic) mutates HP; ticks the
    active Bleed; eventually removes the effect when stacks hit zero.
  - `addBlock` via the request topic adds to the target's block.
  - `applyEffect` returns `(ok, EffectReceipt)` with `id`+`source`+non-empty
    `subscriptions`; `removeEffect` returns the swept receipts.
- `format_test.cpp` — THE simplification assertion:
  - `formatStep({"base", "", "", 7, 7})` produces a line where `source` is
    empty and gracefully omitted (no parens).
  - `formatStep({"effects","vulnerable-goblin","vulnerable",5,8})` produces
    a line containing `vulnerable` and `5 -> 8` (or per an exact formatter
    spec we nail in the plan).
- Passbar: `make test` green on Linux (clang + gcc). CI mirrors rpgkit (clang/gcc
  + MSVC once a Windows runner is wired; UE build verification belongs to the
  UE edge, not core CI).

## Repository setup

- New sibling repo `tumult` on GitHub (KirkDiggler/tumult). MIT license to
  match rpgkit/rpgkit-ue.
- Project board: track alongside `rpg-project` ideas/rpgkit-demo-game and
  board #15 (rpgkit-ue). A dedicated `tumult` board can wait until there
  are 3+ issues; the first issues live on board #15's "I Can …" view and
  cite `rpgkit-ue#9/#14/#15/#16` as the source use cases.
- CI mirrors rpgkit: clang-format-check + clang-tidy + tests, fail-fast
  pre-commit. `make pre-commit` must be green before every push; never
  `--no-verify` (per `rpg-project/CLAUDE.md`).
- Issue-first; one slice per PR; merge (never rebase); never merge without
  Kirk's explicit approval; never commit local `replace` directives.

## Scope — phased rollout

The NORTH STAR is the Across-the-Obelisk capability surface (`cards,
buffs, debuffs, items, class abilities, enemy intents, target selection,
chain breakdowns, combat logs, debug/inspection`). tumult is the home for
each of those as it lands. Phases — one PR per phase issue:

1. **Slice 1 — Core seam + simplification proof (THIS slice).**
   Repo skeleton + Character + Encounter + strike/chain/source wiring +
   Vulnerable/Tough Skin/Bleed + `formatStep`. Tests green on Linux. No
   host edge wired yet; rpgkit-ue PR #4 lands in parallel as the UE
   prototype (its `URPGKitEncounterRuntime` is what we port FROM).
2. **Slice 2 — Edges adopt the engine.**
   rpgkit-ue migrates `URPGKitEncounterRuntime` to hold a
   `tumult::Encounter` and reflect its `StrikeResult`/`BreakdownStep` into
   the HUD (UE build verification on Windows — Kirk's last step). HUD reads
   `source` from the receipt; the `EmitCombatLog` paren-strings retire.

   **Slice 2 design decisions (locked 2026-06-24):**

   - **D2.1 — tumult as UE module:** Vendor tumult `v0.1.0` under
     `rpgkit-ue/ThirdParty/tumult/` and expose it as a UE module via
     `ThirdParty/tumult/tumult.Build.cs` (PrivateDependencyModuleNames
     includes `Core`, `CoreUObject` for the module runtime; the module's
     PublicIncludePaths adds `include/`; PrivateIncludePaths or the
     module's own .cpp list adds `src/` so the compiled stateful TUs —
     `src/encounter.cpp`, `src/effects/{vulnerable,tough_skin,bleed}.cpp`
     — build as part of the module). `RPGKitUE.Build.cs` adds `"tumult"`
     to `PrivateDependencyModuleNames`. A pinned `scripts/sync-tumult.sh`
     fetches a named tag (`v0.1.0`) into `ThirdParty/tumult/` so the pin
     is explicit and reproducible. Matches the existing
     `ThirdParty/rpgkit/` vendor pattern (extended for the few `.cpp`
     files tumult has); no CMake dependency added to the UE build.

   - **D2.2 — PR #4 superseded:** rpgkit-ue PR #4 (`Extract encounter
     runtime`) closes WITHOUT merging. Its `URPGKitEncounterRuntime`
     design is sound (the user reviewed it during the original session);
     we salvage its good parts (Blueprint compat wrappers, the
     request-topic plumbing shape, the STATUS.md update prose) into Slice
     2 directly. Rationale: Slice 2's runtime wraps a
     `tumult::Encounter` from the start; landing PR #4 first would create
     a hand-rolled runtime that Slice 2 then immediately replaces — two
     seams for the same job. One seam, not two.

   - **D2.3 — `URPGKitBus` deleted in Slice 2 (no-shortcut path):**
     `URPGKitBus` (a `UGameInstanceSubsystem`) has a scope mismatch with
     `tumult::Encounter`: the subsystem is session-wide, but each
     `Encounter::setup()` creates a fresh bus by construction (the fix
     that kills stale cross-encounter subscriptions). Keeping
     `URPGKitBus` as a thin delegator forever preserves the scope
     mismatch — that's the shortcut, not the fix. Three-step delete in
     Slice 2:
       1. `ARPGKitGameMode::GetEncounter()` exposes the
         `tumult::Encounter` (the new authority). All current C++ callers
         of `URPGKitBus` (`ExecuteDamageChain`, `ApplyEffect`,
         `RemoveEffect`, `GetRawBus` publish paths) move to
         `Encounter::strike`/`applyEffect`/`removeEffect`/`bus()` and
         translate the results at the UE-boundary
         (`StrikeResult`→`FRPGKitChainResult`, etc.).
       2. `URPGKitBus::PublishEvent` (the one `BlueprintCallable`) moves
         onto `ARPGKitGameMode` as a `BlueprintCallable` forwarder
         `PublishEvent(FName, FRPGKitEventPayload)` that calls
         `GetEncounter().bus().publish(...)` via the right typed
         `Topic<T>` for the topic name (today: `turn.ended`). Blueprint
         assets keep working.
       3. `URPGKitBus` deleted (`RPGKitBus.h`/`.cpp`/`.generated.h`
         references cleared from `RPGKitUE.Build.cs` source list;
         `URPGKitBus` field on `ARPGKitGameMode` removed).
     **Windows gate:** Kirk scans `.uasset` Blueprint assets for any
     `URPGKitBus` references before the delete. If references surface,
     deletion defers to Slice 2.1 with one-PR deprecated alias
     (`URPGKitBus` kept as a thin shim that routes to the Encounter);
     otherwise cold delete in Slice 2. The plan's "verify" step before
     the delete names this explicitly so the Linux-buildable side lands
     everything up to (but not including) the delete, and Kirk's
     Windows step is the verification + the final delete commit OR the
     deferred-alias PR.

   - **D2.4 — UE effects wrap tumult effects:** `URPGKitVulnerableEffect`
     / `URPGKitToughSkinEffect` / `URPGKitBleedEffect` become thin
     `UObject`s that own a `tumult::VulnerableEffect`
     /`tumult::ToughSkinEffect`/`tumult::BleedEffect` (heap-allocated
     inside the UObject, lifetime tied to the UObject). The UE base
     `URPGKitEffect` keeps its existing `BlueprintImplementableEvent`
     hooks (`OnEffectApplied`, `OnEffectRemoved`); its
     `GetRawEffect()`/`RawEffectPtr` indirection is replaced by a
     pointer to the owned `tumult::` effect. `applyEffect`/`removeEffect`
     on the Encounter return `(Status, EffectReceipt)`; the UE wrapper
     surfaces the receipt to the blueprint hook (Kirk's
     `OnEffectApplied`/`OnEffectRemoved` get called with the receipt's
     source/id so a HUD can render "Vulnerable applied by card
     strike-1"). Blueprint-facing `UPROPERTY` param fields
     (`TargetEntityId`, `PercentBonus`, `Stacks`, `DurationTurns`, etc.)
     stay on the `URPGKitEffect` subclass and are passed to the tumult
     effect's constructor in `BeginPlay`/`PostInitProperties`. This
     proves the host-adapter pattern end-to-end: authored UE content →
     JSName→tumult effect → receipt surfaced back to Blueprint.

   - **Verification (Linux-buildable + Windows last step):** Linux side
     of this slice is mostly structural (Build.cs files, sync script,
     salvable Blueprint compat API). The Linux-buildable-but-cannot-link
     line is: anything that `#include`s Unreal headers but isn't part of
     a UE module is unverifiable on Linux. The plan draws this line
     explicitly per step so a Linux-buildable subagent doesn't take a
     Blue ≠ Blue step_markers build and falsely claim DONE.
   - **Acceptance:** the existing visual demo loop in rpgkit-ue
     (`docs/status.md` — cards, energy, enemy attack, block timing,
     bleed/vulnerable effect, HUD with combat log + damage breakdown)
     still plays end-to-end on Windows after the migration, and the
     damage breakdown on the HUD now reads source from the chain receipt
     (visible by matching the paren-source label in
     `Integration.HeroStrikeBreakdownNamesEveryModifierFromReceipts`
     output OR by confirming no `EmitCombatLog` paren-strings remain on
     the strikes path).
3. **Slice 3 — Cards as Actions + Action receipts in HUD.**
   Implement Cards as `rpg::core::Action` subclasses; route
   `ActionReceipt` from `tumult` through the UE executor so a card names
   itself on the HUD without a GameMode name-switch (`rpgkit-ue#15`).
4. **Slice 4 — Energy + deck + hand model; player loop.**
5. **Slice 5 — Enemy intent model (`rpgkit-ue#10`) and authored intents.**
6. **Slice 6 — Target selection (`rpgkit-ue#11`).**
7. **Slice 7 — Terminal edge (rpgkit-demo-game) migrates to consume
   tumult** — proves the second edge cleanly. (Optional earlier if it
   reveals a cross-host seam that needs pressure.)
8. **Later — AtO buff catalog, items, class abilities, multi-enemy,
   win/loss feedback (`rpgkit-ue#13`), structured observer API routed to
   combat-log sink (`rpgkit-ue#8`).**

The first slice alone answers the question "does v0.3.0 simplify UE?" —
Slice 2 demonstrates the answer by retiring host strings on the live HUD.

## Non-Goals (this slice, YAGNI)

- No cards-as-Actions yet (`Strike` is a direct `Encounter::strike` verb).
- No ActionReceipt routing to HUD (effect receipts plumb, but actions are
  not yet discrete verbs routed through receipts).
- No combat-log reform; `recentCombatLog` still seeds from receipt facts
  but is not a generic observation sink (`rpgkit-ue#8` and the rpgkit
  observation-API follow-up doc own that).
- No enemy intent content, no multi-target, no win/loss flag, no second
  edge.
- No C ABI (Unity) layer. No engine-binding rewrite. No async/transport
  swap.

## Open Questions

1. **BrowserRouter vs Encounter-owned Bus.** Today rpgkit-ue's bus is a
   per-GameInstance subsystem shared across encounters. tumult's `Encounter`
   owns its bus. For Slice 1 the Encounter-owns-bus posture is simplest and
   matches rpgkit tutorials; for cross-encounter effects (later) a host-side
   broker switches in. No action needed now; flagged so we know it moves.
2. **Formatter contract.** Exact `formatStep` string shape ("effects /
   vulnerable-goblin (vulnerable): 5 -> 8") is a Slice-1 micro-decision.
   Pinned by `format_test.cpp`'s exact assertions in the implementation
   plan, not here.
3. **Where the second edge lands first (terminal).** Optional earlier port
   would prove cross-host from day one; the recommendation is *after* Slice 2
   so we don't destabilize the UE migration with two edges at once. Decide
   at Slice 6 boundary.
4. **Status template vs. plain struct.** `BreakdownStep` is plain today. If
   a later receipt shape needs host-extension seams (host-supplied opaque
   metadata, dnd5e-rulebook-enriched source), revisit. Not now.

## Success Criteria

- tumult's Slice 1 PR is merged: repo skeleton, Character + Encounter +
  strike + the three effects, `formatStep`, all tests green on Linux.
- `strike` against a Vulnerable target produces a breakdown whose step
  carries `source == "vulnerable"` and is read by `formatStep` — proven by
  a GoogleTest, not the UE HUD.
- rpgkit-ue still builds against v0.3.0 (its `ThirdParty/rpgkit/` is
  bumped; this is done as part of Slice 2's UE-migration branch, not in
  Slice 1 — Slice 1 stays unreferenced by UE until Slice 2).
- A future UE HUD step shows "who granted this modifier" from the receipt,
  retired paren-strings, no host re-derivation — verified by a recorded
  playtest once Kirk is back on Windows.
- The path to a second edge (terminal) is mechanical, not architectural:
  someone following `tumult`'s public headers can host it without reading
  UE.

## Relationship to existing repos / PRs

- **rpgkit-ue PR #4 (`Extract encounter runtime`)** lands independently on
  its own merits; it is a sound UE-only seam that the user/conversation
  reviewed. Its `URPGKitEncounterRuntime` is the *prototype* this effort
  ports INTO tumult. Migration happens in Slice 2; PR #4 is not blocked.
- **rpgkit** stays unchanged during Slice 1 — tumult only *consumes* v0.3.0
  receipts. If Slice 2/3 surface a genuine core gap (e.g., richer action
  context), file a `tumult` → `rpgkit` pressure issue per board #15's
  operating principle; fix core; resume.
- **rpgkit-demo-game** stays unchanged during Slice 1; Slice 7 (or earlier,
  if forced) migrates it to host tumult, turning the qwen experiment into a
  terminal-edge proof of the same engine both UE and terminal share.
- **rpg-project/ideas/tumult/{design.md, plan.md, memories.json}** are the
  cross-repo home; this design is the source of truth, not a copy in any
  consumer repo.

## Pointers

- Source use cases (the pressure): 
  `rpgkit-ue#9` (HUD explains why damage changed), 
  `rpgkit-ue#14` (HUD shows effect apply/tick/expire), 
  `rpgkit-ue#15` (add an action without GameMode branching), 
  `rpgkit-ue#16` (gameplay requests behind observable seams).
- rpgkit v0.3.0 contracts:
  `rpgkit/core/include/rpg/core/{action,effect,chain}.hpp` — the headers
  Slice 1 consumes.
- Existing prototype to port from:
  `rpgkit-ue/Source/RPGKitUE/{RPGKitGameMode,RPGKitEncounterRuntime,RPGKitBus,RPGKitEffect}.{h,cpp}` (post-PR-#4 merges).
- Board: `KirkDiggler/rpgkit-ue` project 15 ("RPGKit Unreal Workshop") and
  its `RPGKit Board Seeds` sections in each use-case issue.

## Status

Design captured 2026-06-23, validated against the rpgkit-ue PR #4 review,
board #15 use cases, and rpgkit v0.3.0 receipt headers. Next concrete step
is the implementation plan (`plan.md`), scoped to Slice 1 only.