# Tumult — Capability Spec & Working Model

_Created 2026-06-27. Source of truth for the **"Tumult: Combat Engine"** capability board._

## What this document is

This is the supply-side map for **tumult**, the C++ combat library — what it
provides, in what order, and **how we work while building it**. It is written
to a specific bar:

> **A fresh session, with no memory of how this doc came to be, should be able
> to pick up a single wave below and carry it through to merged PR(s) using
> subagents — and know when to stop and ask.**

That handoff is itself the first experiment. If a session takes a wave and gets
stuck for lack of context, the gap is *this document's* bug, not the session's.
Fix the doc.

This project is **new and uncharted** — the best way to build it is genuinely
unknown to us. So we do not pretend the plan below is correct. We proceed,
**watch the seams, think in primitives, and write down every non-obvious
decision** (see _How we work_). We stay fluid: waves split or combine as we
learn their real shape.

---

## Orientation — read these first

| Source | Why |
|--------|-----|
| `ideas/tumult/initial-plan/design.md` | The locked design thesis + T1–T4 decisions. Note: its Slice 2–7 sketch predates the `tumult-ue` pivot — see _Waves_ below for the reconciled cut. |
| `ideas/tumult/initial-plan/plan.md` | Slice-1 task-level detail (the implemented seam). |
| `tumult` repo `CLAUDE.md` / `AGENTS.md` | The boundary rule, binding decisions, workflow (issue-first, TDD, pre-commit). |
| In-flight branches | `origin/docs/tumult-slice-2`, `origin/docs/tumult-game-design` exist on `rpg-project` — **reconcile before starting Wave A** so we don't duplicate drafted design. |
| Boards #14 / #15 / #16 | #14 *rpgkit: Portable Combat Core* (the engine), #15 *RPGKit Unreal Workshop* (demand corpus), #16 *Tumult UE Foundations* (the active UE 5.8 host). |

### Key issues (the demand we proof against)

- **Host:** tumult-ue#4 (call tumult from BeginPlay), #5 (show HP), #10 (hand of cards), #11 (north star: draw & play cards), #13 (card-play receipt), #14 (play a card through tumult).
- **Workshop use cases (board #15):** rpgkit-ue#9 (HUD explains why damage changed), #14 (HUD shows effect apply/tick/expire), #15 (add an action without GameMode branching), #7/#19 (add an effect/status safely), #6 (author a card that applies a status), #10 (enemy intent), #11 (target selection), #8 (combat-log sink).
- **Core pressure:** rpgkit#52 (AtO debuffs, resistance, multi-hit spike), #50 (typed topic wrapper).
- **Open in tumult:** tumult#3 (refresh host-integration docs to point at `tumult-ue`, document host consumption).

---

## The chain & the boundary

```
rpgkit  ──>  tumult  ──>  tumult-ue (UE 5.8 host, someone else's project)
(engine)     (the game)    (an edge; terminal is another edge)
```

- **tumult knows the game** — characters, cards, statuses, the combat loop, the
  resolution rules. It **never** names a host (`UClass`, `FString`,
  `TObjectPtr`, `std::istream`-driven UX). If a host type appears in
  `include/tumult/`, that's a boundary violation — say so.
- **rpgkit is a dependency, not an edge.** tumult is rpgkit's *customer*: when a
  capability needs a core primitive, we push it upstream (see rpgkit lane).
- **Receipts are primary observability.** Every resolution flows through
  `(Status, Receipt)`; the question "who caused this number?" is answered by a
  receipt field (`Chain::Step::source`), never a host-side hardcoded string.
  `formatStep` is the proof — one formatter, two edges, no host re-derivation.

---

## How we work (the operating model)

We are tool builders. The discipline below is the product as much as the code.

### 1. Decision Receipts

Every non-obvious **engineering** decision — a seam, primitive, or API-shape
choice a future contributor would otherwise reverse-engineer or get wrong — gets
a receipt in **`ideas/tumult/decisions/`**, one file per decision named
`NNNN-slug.md` (e.g. `0008-consumption-model.md`), indexed by
`decisions/README.md`. A receipt is deliberately light — seam-focused, not a
heavyweight ADR:

> **Bar (recalibrated 2026-06-27):** a DR is for code/engineering decisions, not
> process or convention. How we run the board, where files live, how we cut
> waves — those live *in this section as prose*, not as ceremonial receipts.
> (The early DR-0001…0007 were mostly process scaffolding — a sign we
> over-applied the tool; leave them, but don't mint more like them.) Reserving
> DRs for engineering keeps `decisions/` high-signal. Expect few DRs, almost all
> about code.

```
## DR-NNN · YYYY-MM-DD · <one-line title>
Seam/primitive:  <what primitive or boundary this touches>
Decision:        <what we chose>
Rejected:        <the alternative(s) and the one-line reason each loses>
Why:             <the benefit that makes this worth it>
Interface delta: <before → after, when an interface changed>
```

The archetype: *positional params (`param1, param2, …`) → an `Input`/`Output`
struct.* Small change, large payoff, and now pointable-at.

**Why this matters:** decision receipts are to our process what `(Status,
Receipt)` is to combat — they make the *why* observable. That observability is
the precondition for autonomy: a wave-owning session earns more rope by leaving
receipts a reviewer can audit, not by being trusted blind. **No non-obvious
engineering call ships without a DR** — but process is *pulled by pain, not
pushed up front*: bias to shipping, and let building reveal which process earns
its keep.

### 2. Think in primitives, watch the seams

Before adding a special case or a second representation, ask: *is this actually
different, or the same primitive?* Default to one system. When you do widen a
primitive (e.g. `DamageEvent`), widen it **once** rather than letting N readers
re-derive — and record the DR.

### 3. Fluid waves + the combine rule

The wave list is a **current best guess, not a contract.** We start with more,
smaller waves because fine granularity surfaces seams early. At each wave close,
ask three questions:

1. **Primitive:** what primitive did this touch or create?
2. **Seam:** what boundary did it move, and is the boundary cleaner or muddier?
3. **Shape:** did this wave want to be two? Did the next wave just become trivial
   enough to merge into this one?

Combine when the seam says so. Split when a wave hides a real decision. The
workflow itself is a living thing — refine it as we learn what's good for us.

### 4. Per-wave standing checklist (for every session/sub-agent)

- [ ] Goal behavior named, and the demand issue it proofs cited.
- [ ] Primitive(s) and seam(s) identified up front.
- [ ] TDD: test first, `make test` green, `make pre-commit` green (never `--no-verify`).
- [ ] Every non-obvious call has a DR in `decisions/` (a new `NNNN-slug.md`).
- [ ] Boundary held: no host types in `include/tumult/`.
- [ ] Done-when criteria met and **observable** (a test or an edge proof, not "it compiles").
- [ ] Stop-and-ask if blocked on a decision that's genuinely the director's.

### 5. The handoff/autonomy goal

The target operating mode: **one session owns a whole wave**, dispatches
sub-agents for parallelizable pieces, and **persists through multiple PR
merges** (same session handles its own Copilot review + CI fixes). We don't know
yet if our artifacts carry enough for that — so each handoff is a measurement.
Pay attention to the shapes: where sessions thrive, where they stall.

### 6. Wave-plan & handoff conventions (adopted from the A1 retro)

A1 taught us how to *write* a wave and *hand it off*. These are **conventions, not
DRs** — process, not engineering, per the DR bar above:

- **Plans carry intent + acceptance, not literal diffs.** A wave plan names each
  task's *goal* and its *acceptance criteria as outcomes*, and lets the executor
  read the current code and choose the specifics. Literal `old_string`/`new_string`
  (or full code blocks pinned to today's source) rot between authoring and
  execution and make the plan lie. Give exact shapes only where a shape is the
  *contract* (e.g. a DR's interface delta); name everything else as the executor's
  call.
- **State the goal, not the toolchain incantation.** Done-when is a behavior
  ("`host_consumer` builds and runs green as a ctest"), not a specific
  generator/preset command. The executor picks the toolchain; the presets are a
  convenience, not the contract.
- **CI is the authoritative gate.** Local `make pre-commit` may lack
  `clang-format`/`clang-tidy` or run a different version — so name which local
  checks may be unavailable and tell the executor not to thrash on env-only lint
  failures. The CI `lint` + build jobs decide. Never `--no-verify`.
- **Handoff briefs end with an explicit report step.** The last instruction to any
  dispatched session is: *send your structured report via SendMessage before going
  idle.* The idle / turn-end notification is **not** the report — a wave that
  finishes without a returned report is not handed off.
- **Run examples under ASan/UBSan locally before push** (A2 retro). A dangling
  pointer-into-temporary was invisible on Linux release / `make` and only surfaced
  under MSVC-debug / ASan in CI; running the examples under sanitizers locally
  catches this class before CI does.

---

## The board — "Tumult: Combat Engine"

- **Role:** the supply/driver board — the missing middle of `rpgkit → tumult →
  tumult-ue`. #15/#16 are demand; this is what tumult provides to meet it, plus
  the rpgkit work that provision requires.
- **Spine:** capability **domains** (rows) × **tumult-ue horizon** (tag). Every
  item cites the demand it proofs.
- **rpgkit lane:** tumult-driven core issues (e.g. rpgkit#52) are **first-class
  on this board AND remain on #14** (same GitHub issue on both, shared
  open/closed state). Division rule: **tumult-driven rpgkit work surfaces here;
  rpgkit-internal work (tutorials, etc.) stays on #14 only.**
- **Source of truth:** *this doc* + `decisions/`. The board tracks **state**;
  the design lives in docs. (Knowledge-in-docs, board-tracks-status.)
- **Workflow:** issue-first; one PR per logical unit; cite the use case;
  merge never rebase; PR → Copilot → fix/reply → director merges.

---

## Capability catalog (6 domains)

| Domain | What it owns | Representative capabilities | Lead wave |
|--------|--------------|-----------------------------|-----------|
| **Host & Wrap Surface** | How a host consumes tumult | consumption contract (`#include` headers vs link the `Encounter` OBJECT lib); thin stable host API; Blueprint-friendly getters; zero host-type leakage | **A** |
| **Cards & Actions** | The verb layer | cards as `rpg::core::Action`; play → mutate → `ActionReceipt`; card self-names on HUD (no GameMode switch); multi-action cards; energy cost | **B** |
| **Damage & Resolution** | Combat math + breakdown | typed `DamageEvent {amount, type, crit, tags}`; resistance × vulnerable; multi-hit (per-hit vs total); block; rounding policy; chain / `formatStep` | **C** |
| **Statuses & Effects** | Buff/debuff system (the AtO heart) | shared `StatusEffect` base (stacks / duration / expiry→auto-remove); refresh rules; the AtO catalog (bleed, poison, weak, regen, thorns…) | **S** (parallel) |
| **Encounter & Roster** | The loop + the cast | turn / energy / draw loop; 4-vs-N roster; targeting (rows, AoE, ally/enemy); enemy intents | **L** (later) |
| **Observability & Receipts** | The "why," reaching the edge | `formatStep` breakdown; `ActionReceipt` / `EffectReceipt` to HUD; effect apply/tick/expire log; combat-log sink | cross-cutting |

> _Observability is kept as its own domain rather than a horizon tag because
> "receipts as primary observability" is the product thesis — it's a thing we
> build, not just a phase._

---

## Waves (the current cut — fluid)

Each wave lists: **goal behavior**, the **demand** it proofs, the
**primitives/seams** it touches, **done-when**, and the **DRs to expect**. This
cut reconciles design.md's Slice 2–7 sketch (which named `rpgkit-ue` as host)
against the `tumult-ue` reality.

_Horizon status: **Wrap Foundation done** (A1 merged). **First Loop done** (A2
merged — tumult#7 closed, PR #8; retro in `wave-a2/retro.md`). Next active horizon:
**Cards Visible** (Group B)._

### Group A — Easy to wrap (the foundation)

**A1 · Consumption contract + first host call.** — **done** (merged tumult#6; issue
#5 closed). _Horizon: Wrap Foundation._
Goal: a host can `#include` + link tumult and call `Encounter` from BeginPlay;
tumult#3 docs reconciled (host = `tumult-ue`).
Proofs: tumult-ue#4. Primitives/seams: the include-vs-link seam (header-only
`include/` vs the `Encounter` OBJECT library), the host↔library edge.
Done-when: documented consumption contract + tumult-ue#4 lights up.
DRs: DR-008 (vendor source + compile as a host module), DR-009 (recipe verified by CI).

**A2 · First Loop — host-observable encounter.** — **done** (merged tumult#8;
issue #7 closed; plan in `wave-a2/plan.md`, retro in `wave-a2/retro.md`; **DR-010**).
_Horizon: First Loop._
Goal: a host runs a minimal combat loop — reads every combatant's HP from a
read-model each iteration, strikes until the goblin reaches 0 and is marked dead,
and observes each resolved strike via one bus subscription whose breakdown still
self-names from receipts.
Proofs: tumult-ue#5. Seams (settled in DR-010, do not re-open): the host↔encounter
**observation edge** — `strike` publishes a `StrikeResolved` notification **after**
mutation *and* keeps its `StrikeResult` return (push + pull); a value-snapshot
`std::vector<CombatantView>` read-model (mirror-not-wrap, T3) over live pointers;
"dead" is the `alive` flag, not a death event; the host owns the loop (tumult adds
no loop primitive). rpgkit untouched (`Topic<T>` already ships in v0.3.0).
Done-when: the extended `examples/host-consumer/` ctest runs the loop, drives HP
20→0, marks the goblin dead from the read-model, observes one event per strike, and
the breakdown still self-names — green in CI. (Proof stays a tumult ctest, **not** a
UE build.)

### Group B — A card reaches UE (capable)

**B1 · One card as an `Action` (headless).**
Goal: a Strike card is an `rpg::core::Action`; activating it routes through
`Encounter` and yields an `ActionReceipt`; rides the **existing generic damage**.
Proofs: rpgkit-ue#15. Seams: the Action verb layer vs the direct
`Encounter::strike` verb (decision 10: Action ≠ Effect).
Done-when: headless test — play card → state mutates → `ActionReceipt` names the card.

**B2 · Card self-names on the HUD.**
Goal: `ActionReceipt` reflected to the host so the card names itself, no
GameMode name-switch.
Proofs: tumult-ue#13/#14. Seams: receipt→edge formatter (the `formatStep`
analog for actions).
Done-when: UE shows a card-play receipt that names the card from the receipt.

**B3 · Played, not just executed (energy + tiny hand).**
Goal: minimal energy + a small hand so a card is *drawn and played* within a turn.
Proofs: tumult-ue#10/#11 (north star). Seams: deck/energy model; turn/energy loop.
Done-when: draw a hand, spend energy, play a card — visible in UE.

### Group C — Damage-model depth

**C1 · Typed `DamageEvent` (the primitive change).**
Goal: `DamageEvent` carries `{amount, type, crit, tags}`; `strike` takes a
request struct; modifiers see the whole payload.
Seams: the `DamageEvent` primitive; the `strike` signature (positional → struct
— the archetypal DR). Done-when: existing effects pass on the typed payload;
breakdown unchanged on the untyped path. Expect DRs: typed-payload shape;
rounding policy.

**C2 · Resistance × vulnerable on typed damage.**
Goal: typed resistance (flat per-hit reduction) composes correctly and *visibly*
with vulnerable (%) in the breakdown.
Proofs: rpgkit#52 (part), rpgkit-ue#9. Seams: chain stage ordering (amplify vs
reduce); per-type resistance read. Done-when: breakdown shows the interaction in
AtO-correct order.

**C3 · Multi-hit semantics + rounding.**
Goal: a multi-hit card resolves per-hit-vs-total correctly for block &
resistance; rounding policy locked.
Proofs: rpgkit#52 (part). Seams: per-hit vs total resolution; block absorption
per hit. Done-when: multi-hit test matches AtO semantics; receipt explains each hit.

### Group S — Statuses (parallel thread)

**S1 · Shared `StatusEffect` base.**
Goal: a base owning stacks / duration / expiry→auto-remove + refresh policy;
refactor Vulnerable / ToughSkin / Bleed onto it; fix the dormant-subscriber +
expiry-unsubscribe inconsistency.
Proofs: rpgkit-ue#7/#19. Seams: the duration/stack primitive; **effect lifetime
& ownership** (the non-owning `Effect*` → likely `unique_ptr` question surfaces
here). Done-when: 3 effects on the base, tests green, no dormant subscribers.
Expect DRs: stacks-vs-duration model; expiry policy; ownership decision.

**S2 · Begin the AtO status catalog.**
Goal: add a few canonical statuses (poison, weak, regen…) on the base to prove
it composes. Proofs: rpgkit-ue#6. Done-when: N new statuses, each tested and
breakdown-visible.

### Group L — Later

Encounter & Roster (multi-enemy roster; targeting — rows/AoE/ally-enemy,
rpgkit-ue#11; enemy intents, rpgkit-ue#10); full AtO status catalog; combat-log
sink (rpgkit-ue#8); terminal edge (rpgkit-demo-game) as the second-edge proof.

---

## What we don't know yet (shapes to watch)

We name these so a handoff session knows they're *open*, not settled:

- **Damage types taxonomy** — no fire/cold/physical/etc. system exists anywhere.
  rpgkit#52 is the spike where resistance/multi-hit semantics get settled; the
  type set itself is undesigned. Decide it *in* C1/C2 with DRs.
- **Effect ownership** — Slice 1 uses a non-owning `Effect*` with caller-owned
  lifetime (a known rough edge). S1 is where `unique_ptr` ownership likely
  lands. Don't pre-decide; let the dynamic gain/expire use case force it.
- **Where effects read cross-entity state** — at contribution time or execute
  time? Conditional effects ("+5 if target is bleeding") will force this. The
  chain re-evaluates on `execute`, so execute-time reads are "free" but the
  modifier signature `T(T)` doesn't hand you the world. Open.
- **Roster shape** — `hero_`/`enemy_` is a two-fighter stopgap; the by-id
  interface is chosen so the swap to a roster is contained, but the targeting
  model (rows, AoE) is undesigned. Group L.
- **Handoff sufficiency** — whether one session can carry a wave through
  multiple PR merges on these artifacts alone. Measured per handoff.

When one of these resolves, it becomes a DR and (if it changes the plan) a wave
edit.

---

## Glossary — rpgkit primitives a wave session must know

- **Bus** — synchronous, subscription-order, fail-fast event router; `std::any`
  erased under a typed `Topic<T>` veneer. The `Encounter` owns one (decision T1).
- **`Topic<T>` vs `ChainedTopic<T>`** — plain topic *notifies* (`Handler =
  Status(const T&)`); chained topic *collects modifiers* into a `Chain<T>`
  (`Handler = Status(const T&, Chain<T>&)`). `TopicDef<T>::on(bus)` vs
  `.onChained(bus)`.
- **`Chain<T>`** — staged fold. Constructed with an ordered stage list (no
  priorities). Subscribers `chain.add({stage, id, modifier, source})`;
  `chain.execute(data)` folds in stage order, then insertion order within a
  stage, and returns `{value, breakdown}` where each `Step` carries
  `{id, stage, source, before, after}`. **Re-evaluates every modifier on
  execute** — multi-hit and "reroll each time" effects are free.
- **`Effect`** — persistent listener; `apply(bus)` calls `onApply` (subscribe via
  `track(id)`); `remove()` sweeps tracked subscriptions. Non-copyable. Carries
  `id` (instance) + `source` (kind). Returns `(Status, EffectReceipt)`.
- **`Action`** — transient verb (a card). Distinct from `Effect` (decision 10).
  Yields an `ActionReceipt`. Not used in Slice 1; arrives in Group B.
- **`Status`** — exception-free `ok()/error(msg)`, `[[nodiscard]]`. Receipts are
  populated even on failure. Never throw across the API; never return `(nil, nil)`.
