# Active handoff — 2026-08-14

## Now

**The session SDK lane** (`rulebooks/dnd5e/session`) — rpg-api's one interface to the toolkit.
W0/W1/W2 shipped. **W3 step 1** (characters load) → `session/v0.3.0`. **W3 step 2** (conditions
across a suspension) → PR #950 merged at **132f018**, tagged **`session/v0.3.1`** — a `test:`
commit takes a **patch** bump. **W3 step 3** (NPCs — `Join`/`Spawn`) → PR #952 merged at
**f0c7152**, tagged **`session/v0.4.0`**. Tracking
[toolkit#945](https://github.com/KirkDiggler/rpg-toolkit/issues/945).

**Remaining in W3: T3.6's scene only**, and it is blocked — see Open questions. PR #953
(ADR-0037 + enforced decisions digest) merged at **f9b720f**; PR #955 (the `play/` layering docs
+ the W4 charter) merged at **ad19574**, taking `rulebooks/dnd5e/v0.80.1`.

**W4 is combat, built new** — chartered (plan.md, six deflect/ponder clauses), sliced onto board
19 as toolkit **#963→#966** under tracker **#959**, and its foundation is **decided end to
end**: **ADR-0038 — resolution owns the bus** (merged `8f74220`, 2026-08-14). Step 4.1 shipped
(PR #960 → **`encounter/v0.5.0`**: members on clocks, bubbles persist, `ClockOf` answers).
**Step 4.2 shipped** (PR #969 → **`encounter/v0.6.0`**, #963 closed): the composition can make a
fight — `Form`/`Transfer`/`EndTurn`/`Dissolve`, the DOS2 split-party scenario runs through the
composition with R6 asserted after every transition, and the coexistence rules landed with it
(**a fight member is the fight's alone**: Move/Traverse reject with `ErrInBubble`, Pump skips —
the world thinks on the tick, a fight thinks in turns). **Next: #965 — the resolution package**
under ADR-0038; its first task is the suspension-point probe. The trigger question (#964) is
answered in direction: **mutual awareness** (Kirk), revisable — and rides along later as
`encounter.NewWalk`'s `Pose`.

## Solid

**Shipped, tag verified against the merge commit in every case:**

| Module | Version | Merge | What it brought |
|---|---|---|---|
| `dnd5e/encounter` | v0.1.0 → v0.4.0 | #921/#924/#932/#939 | the composition; transitions; world anchoring; bounded story log |
| `dnd5e/encounter` | v0.5.0 | #960 | members are on clocks; bubbles persist; `ClockOf`; R6 at the trust boundary |
| `dnd5e/encounter` | v0.6.0 | #969 | `Form`/`Transfer`/`EndTurn`/`Dissolve`; coexistence gates; husk pruning; DOS2 through the composition |
| `dnd5e/session` | v0.1.0 | 407bd57 | shell, 10 free-roam verbs, event stream, the boundary test |
| `dnd5e/session` | v0.2.0 | 556b2e1 | the interrupt spine — suspend, persist, resume in a fresh process |
| `dnd5e/session` | v0.3.0 | 5c79152 | characters load through the host's repository |
| `dnd5e/session` | v0.3.1 | 132f018 | conditions survive a suspension; the store is never damaged |
| `dnd5e/session` | v0.4.0 | f0c7152 | `Join` loads players, `Spawn` instantiates content from a ref |

**The strategy — REVISED 2026-08-13, and the old falsifiable claim is retired.** The plan was
*wrap what exists → migrate rpg-api at W4 → replace behind it*, claiming *after the migration
wave, no subsequent wave changes an rpg-api source file.* **Kirk retired that**: a wrapper would
satisfy the claim while teaching nothing, and worse, **a wrapper would dictate what the session
contract looks like** — the SDK would inherit the old encounter's assumptions invisibly. There is
**no adapter to the old encounter**; the idea stays technically open and unused.

**We are building new, and the goal is to get it right.** Getting the SDK into rpg-api is still a
goal, but it **comes later** — after the session package can run free roam *and* combat. No
integration slice now. What replaces the old claim: *a real consumer used the SDK and told us
where the contract was wrong* — payable only once there is something whole enough to consume.

Still standing, and still mechanical: `gorelease` from the first tag, and
`TestNoInnerTypeCrossesTheBoundary` — it parses the package's own AST and fails on any leaked
toolkit type. rpg-api imports **zero** of the composition today, so nothing is owed yet.

**Measured 2026-08-13, so nobody re-derives it:** rpg-api already ships exploration. Its encounter
has `ModeFreeRoam`, players call `MoveEntity`, and `checkCombatEntry` (old `encounter/combat.go:523`)
runs `perception.CanSeeAt` inline after every move, flipping to `ModeTurnBased` and rolling
initiative on first sighting. **The difference the SDK actually brings is narrower than "exploration":
the old stack flips a mode; the SDK stops and asks.** Do not describe free roam as new capability.

**Rulings that decide future calls — don't re-derive these:**

- **Start from the reversible direction.** Adding a `Config` field is compatible; removing one
  a host implemented is not. Loosening a rule is compatible; tightening breaks every host. So:
  path adjacency and the event stream were required from day one; repositories were deferred
  until something called them.
- **`gorelease` has a blind spot — a new *required* `Config` field.** It reports compatible, it
  compiles everywhere, and it fails every existing host at its first `NewManager` (S8:
  construction is total). Demonstrated live on #949. Discipline: **every required `Config` field
  lands at or before W4.**
- **The repo AUTO-TAGS on merge to main** (`auto-tag-modules-safe.yml`; the other two tag
  workflows are disabled). Bump comes from the conventional-commit prefix — `feat:` → minor,
  **`test:`/`fix:`/`docs:` → patch**. **It keys on the module DIRECTORY, not on whether code
  changed**: a test-only PR consumes a version (#950 → `session/v0.3.1`), and so does a
  markdown-only one — PR #955 edited `rulebooks/dnd5e/CLAUDE.md` and nothing else in that
  module, and took **`rulebooks/dnd5e/v0.80.1`** (verified: the tag points at `ad19574`).
  Consequence worth planning around: **editing a module's own CLAUDE.md/README costs that
  module a version**, so batch module-scoped doc edits rather than dribbling them.
  **A wave's version is decided by its FIRST merge, not its last** — never write a version next
  to a milestone.
- **The compat gate races the tagger, and it fails CLOSED.** `compat.yml` derives its base from
  the newest git tag, but `gorelease` resolves "most recent version" through the module proxy. A
  PR opened soon after a merge therefore passes a base the proxy has not indexed yet, and the job
  dies with *"Can only suggest a release version when compared against the most recent version of
  this major"* — which reads like an API problem and is not. **Re-run the job once the proxy
  catches up.** Cost this one hit: a red gate on PR #952 that had nothing to do with its code.
- **Contract types vs persistence shapes are two different promises.** A persistence shape
  (`encounter.EncounterData`, `interrupt.LedgerData`) is bytes the host round-trips and never
  builds — it promises *replaceability*. A contract type (`spatial.Position`, `character.Data`)
  is shared vocabulary the host constructs — it promises the opposite: a change is *announced*.
  Kirk's framing: a character is a thing, not an implementation detail we'd refactor without
  telling the API. The allow-list now names both categories with a disjointness pin.
- **There is no session process.** Every verb loads, attaches to a bus made for that call, acts,
  saves, and dies with the response; `Answer` is the same load-and-attach done again from data.
  Consequence: **`character.Cleanup` must not be called** — it nils the conditions `ToData`
  serializes, so cleaning up before the save persists a character with zero conditions, silently.
  Safe to skip because conditions intercept on the bus rather than mutating fields.
- **Stateless-per-call does not need a cache.** Benchmarked, not assumed: a player join costs
  ~5.1µs / 64 allocs more than a monster join — the character load exactly — so ~30µs at party
  scale. The caching-repository escape hatch stays theoretical.
- **Write world-then-session.** Partial failure must leave a collectable orphan, never a session
  pointing at nothing, and never a window resuming past cells nobody walked.
- **There is no mode. There are clocks — and they already exist in `play/clock`.** This was asked
  as an open question and answered by reading, not by designing. `Tick` is the player-driven world
  clock; `Turn` is a **localized initiative bubble**; `Transfer` moves an entity between them
  atomically under **R6 — an entity belongs to at most one clock**, which makes "which clock is
  this player on" well-defined by construction. `Merge` already implements "if two bubbles overlap
  they merge" (caller supplies the union order — R7 keeps randomness out). `dos2_test.go` asserts
  Kirk's exact split-party scenario end to end, including a straggler falling into a live fight at
  a rolled slot. **`FREE_ROAM`/`TURN_BASED` is an artifact of the OLD encounter, not a D&D
  concept** — do not reintroduce it. Ownership, from `play/clock`'s own test comment: *"trigger
  detection is the composition's business; here the rulebook has rolled initiative."*
  `rulebooks/dnd5e/initiative` is **dead** — imported by nothing; `play/clock` superseded it.
- **Concurrency is pessimistic, via Redis** (Kirk, 2026-08-13): a lock with a buffer and timeout
  covers the simple cases. It must arrive as an optional capability the manager type-asserts for
  (`SessionLocker`), **never** as a method added to `SessionRepository` — that stops every host
  compiling. The optimistic/checksum-CAS alternative is fully documented and left open in
  `session/repositories.go`; it is not being built. Position writes are NOT independent (every
  move evaluates triggers/sight/endings), so serializing free roam through the lock is *correct*.
- **ADR-0038 — resolution owns the bus** (2026-08-14, merged `8f74220`). The wave-4 seam,
  decided: everything at the seams is data; a **resolution package** is the ONE place a bus
  exists — created per interaction, every participant attached through an *instrumented*
  surface, dead with the call; rules packages (`combat`, `character`) never see the bus — every
  verb is a **step machine** over data yielding the sealed vocabulary `Gather | Pose | Request |
  Done` (forced by suspension: no phase may live on a stack); effects keep `Apply(ctx, bus)`
  unchanged (~26 sites untouched); **granted** effects live on the beneficiary with a link to
  what kills them (Bless → `BlessedCondition{Source}` on the target, `Concentrating{Granted}` on
  the caster, break = `RevokeGrants` across dirty participants, claims validated at load as the
  crash net), **projected** effects never leave their owner (the aura). Full deliberation:
  toolkit journeys **052→053→054**; the digest entry is in `docs/adr/DECISIONS.md`.
- **No simulation server, measured not argued:** one free-roam click = full load-act-save at
  **~187µs** (2.7KB blob, 3-room world, 4-cell walk) — ~1ms with Redis, under a 30–100ms client
  RTT. A click is a PATH not a cell; retention bounds the blob. The host-side RAM-repository
  escape hatch is already reserved in `EncounterRepository`'s doc. rpg-api already ships
  load-per-RPC, so the pattern is in prod.
- **Monster behavior has an on-ramp**: `encounter/README.md` (decider contract, PR #958) +
  `monster/README.md` corrected to route behavior work away from old `NPCAct`/`TakeTurn`.
  Kirk's friend (dammitbilly0ne) shipped `docs/rules/standard-conditions.md` (#956) — treat it
  as the authority on condition semantics; knockdown pair #961/#962 on board (Team: Monster AI),
  sequenced behind the saving-throws prerequisite — **now tracked as toolkit#970** (filed in the
  2026-08-14 sweep; #961 prone does not wait on it, #962 knockdown does).

**The parked question W2 named honestly — NOW DUE:** *"stop the walk when the walker sees something
new"* is a game rule living in a module whose charter says it owns no rules. It's there because no
module owns *when* a resolution should pause. It moves when the second checkpoint kind makes the
deciding thing's shape visible — **W4**, which is now. **Located 2026-08-13:** the rule is the bare
`if len(sighted) == 0 { continue }` in `runWalk` (`session/suspend.go:180`), immediately before
`poseWalk`. Combat entry is the second kind.

**The machinery was built in advance for this and does not need changing:** `Prompt` deliberately
carries *the moment, never the mechanism* (no field names which checkpoint fired, so clients branch
on `OptionKind` and never learn a reason code), and `frozenResolution.Kind` already discriminates
which resolution froze — today only `kindWalk` — "so that a later resolution kind is additive
rather than an ambiguous payload nobody can safely parse."

## Open questions

- **The trigger's shape (#964) — direction set, details open.** Kirk (2026-08-14): **mutual
  awareness forms the bubble** — "if the monster doesn't see or hear me I can duck out of view" —
  revisable. Still open inside that: per-member windows (5e surprise is per-creature), and the
  detection shape — under ADR-0038 the walk is `encounter.NewWalk`, a step machine emitting
  `Pose` on first contact, so #964 became a *restatement* of moving the rule, not a migration.
  `IntelDeltas` is keyed by every member, so both directions of "who saw whom" already exist.
- **Pose vs Request at the reaction-window edge** — settled by the first implementation, not
  argument. Lean Pose-is-primitive (1:1 with the existing `interrupt.Ledger`).
- **The suspension-point probe** (first thing #965 does): verify each phase boundary's chain
  output is fully self-describing data; any boundary holding a closure cannot suspend.
- ~~Player movement does not drive the tick yet~~ — **ruled by Kirk (2026-08-14): free-roam
  `Move` stays thin.** Its job right now is tracking players moving around, nothing more; the
  tick/Pump pattern is grown into later, and **the turn clock is where the economy matters now**
  (movement budget under initiative arrives with the resolution work). The leaf's
  mover-as-driver accrual model stays deliberately unwired.
- **`ConditionBehavior` cannot name itself, and it blocks T3.6.** The interface is
  `IsApplied/Apply/Remove/ToJSON` with no `Ref()`. Reporting a character's active conditions at
  the seam would mean unmarshalling `ToJSON()` and reading `ref` — the exact anti-pattern #941
  already files against us. Without it **T3.6's scene is not expressible** ("Alice raging when she
  is loaded, without the caller ever mentioning rage"). Needs a `Ref()` on the interface in
  `rulebooks/dnd5e/events` plus its ~24 implementers. **Kirk's call; not started.**
- **#946 — character as its own module.** Direction only, blocks nothing. The open part is whether
  the unit is `character` alone or a `dnd5e/core`-shaped module holding the shared enums.

**Resolved since the last handoff, recorded because the reasoning generalises:**

- *NPCs entering the session.* Kirk's ruling (2026-08-13): **a ref is loader routing** — it names
  the package that can load some data. That killed the `dnd5e:characters:<id>` idea on its own
  merits: no toolkit package can load a player character, so such a ref would claim something
  false. Hence **two verbs split by where the data comes from** — `Join` loads a player by ID,
  `Spawn` instantiates code-resident content by ref — an axis that survives durable NPCs and
  homebrew, where player-vs-monster does not. **The encounter needs no change**: `monster.Data`
  carries its own `Ref`, and `SessionData.NPCs` holds the instance, so an earlier "the encounter
  must record which monster a member is" finding was measuring the encounter as if it were the
  sole record.

_(#916's dangling `closes #916` references were removed on PR #950.)_

## Next

**W3 is one step from done.** Everything in [#945](https://github.com/KirkDiggler/rpg-toolkit/issues/945)
has shipped except **T3.6's scene** — *Alice raging when she is loaded, without the caller ever
mentioning rage* — which is **blocked** on `ConditionBehavior` having no `Ref()` (see Open
questions). Nothing else in the wave depends on it, so W4 could start first if that decision
stays open.

Then **W4 — combat, built new in the session package.** rpg-api does **not** migrate here; that
comes later, once free roam and combat both work (Kirk, 2026-08-13). The old stack's ~6,700 lines
of orchestration are *evidence about what a game server needs, never a specification to port* —
and now explicitly not something we adapt to either.

**#963 — bubble town — is done** (PR #969 → `encounter/v0.6.0`). Every done-when item landed:
DOS2 through the composition, R6 after every transition (`ClockOf` per member + a full
trust-boundary reload), the on-no-clock guard reached and tested (white-box, fabricating the one
state no public verb can produce), reject-not-merge pinned. Two things the issue didn't name but
the work forced: **`EndTurn`** (the round cannot wrap without it — done-when governs the pointer
list), and the **coexistence rules** (fight members rejected by Move/Traverse, skipped by Pump —
a decider author never detects "am I in combat"; if `Decide` is called, the monster is
free-roaming, now stated in the encounter README).

**Next is #965 — the resolution package under ADR-0038.** First task: the suspension-point probe
(verify each phase boundary's chain output is self-describing data — any boundary holding a
closure cannot suspend). Then the driver over the sealed vocabulary (`Gather | Pose | Request |
Done`), the instrumented bus surface, and `combat.NewStrike` as the first machine. After that the
seam (#966); #964's trigger rides along as `encounter.NewWalk`'s `Pose`.

## Decision log

| Date | Decision | Visible at |
|---|---|---|
| 2026-08-12 | Session SDK pivot — encounter is *the world*, session is *the table* | toolkit#935, PR #936 (merged) |
| 2026-08-12 | Six-wave plan; anything shaping a public type goes early | `docs/ideas/session-sdk/plan.md` |
| 2026-08-13 | Seam takes IDs; `character.Data` only on the repository | PR #947 (merged) |
| 2026-08-13 | W3 scoped to "get the character loaded", steps allowed | PR #949 (merged) |
| 2026-08-13 | Tag column stops predicting after W3 (auto-tag on merge) | `plan.md` |
| 2026-08-13 | Local `replace`/`go.work` overrides are fine — *committing* them is the ban | `rpg-toolkit/CLAUDE.md` |
| 2026-08-13 | Entity entry splits on **load-vs-instantiate**, not player-vs-monster; a ref names the package that can *load* the data | ADR-0037 |
| 2026-08-13 | Seam decisions get genuine options + trade-offs in the open, then an ADR with the rejects recorded | ADR-0037's process note |
| 2026-08-13 | **No adapter/wrapper over the old encounter** — it would dictate the session contract. Idea stays open and unused | this handoff; ADR pending |
| 2026-08-13 | **rpg-api integration is deferred** until the session package runs free roam *and* combat. No slice now | this handoff |
| 2026-08-13 | The old "no rpg-api file changes after migration" claim is **retired** — a wrapper would satisfy it while teaching nothing | this handoff |
| 2026-08-13 | W4 = combat built new; its first gate is a decision, not code | this handoff, Open questions |
| 2026-08-13 | **No mode — clocks.** `play/clock`'s `Tick`/`Turn`/`Transfer` already model it; the mode question was answered by reading, not designing | toolkit PR #955, `play/README.md` |
| 2026-08-13 | Concurrency is **pessimistic (Redis lock)**; CAS stays documented and unbuilt | `session/repositories.go`, plan.md W4 scope |
| 2026-08-13 | W4 scope: world tick **+ one** turn bubble, linear per encounter; v1 feature is the straggler joining a live fight at a rolled slot | plan.md W4 scope |
| 2026-08-13 | W4 chartered — six clauses, each split into *deflect* / *ponder* | toolkit PR #955, plan.md |
| 2026-08-13 | The `play/` layering gets a CLAUDE.md pointer; per-rulebook casting lives in the rulebook's own CLAUDE.md | toolkit #954 / PR #955 |
| 2026-08-14 | W4 sliced onto board 19: #963 clocks, #964 trigger, #965 resolution, #966 seam; knockdown pair #961/#962 → Monster AI | board 19, tracker #959 |
| 2026-08-14 | Step 4.1 shipped: members on clocks, bubbles persist, `ClockOf` | toolkit PR #960 → `encounter/v0.5.0` |
| 2026-08-14 | Trigger = **mutual awareness** (see-or-hear), revisable | #964 |
| 2026-08-14 | **ADR-0038: resolution owns the bus; rules packages are step machines; granted vs projected effects** | merged `8f74220`; journeys 052–054 |
| 2026-08-14 | No sim server — per-click load-act-save measured at ~187µs; host RAM-repo hatch reserved | journey 053 |
| 2026-08-14 | Step 4.2 shipped: `Form`/`Transfer`/`EndTurn`/`Dissolve`; coexistence = fight members are the fight's alone (Move/Traverse reject, Pump skips); drained bubbles pruned, idle bubbles rejected at load | toolkit PR #969 → `encounter/v0.6.0` |
| 2026-08-14 | Transfer direction is explicit (`To ClockKind`), never inferred — a toggle under load-act-save silently moves stale state the wrong way | PR #969, `ErrBadClock` |
| 2026-08-14 | Free-roam `Move` stays thin (track positions, no `Advance` wiring); grow into the tick/Pump pattern later — the turn clock is where the economy matters now | this handoff, Open questions |
| 2026-08-14 | **A condition/effect record that fails to load never fails silently in the new stack** — condition loading leaves the character package; resolution's single attach loop rejects loudly ("it is not valid in our new world"). #948's in-character fix options are interim maintenance only | toolkit#948, ADR-0038 clause 2 |
| 2026-08-14 | **Issue-tracker sweep executed against the new canon** (5 Sonnet reviewers, ~99 toolkit issues; every verdict re-verified before posting): 12 closes (shipped/answered/dead-premise), 21 `old-stack` labels with annotations, ~54 comments reconciling issue text with ADR-0037/0038 + clocks. Canonical trackers picked: #899 (OA hostility, was triple-filed), #721 (coverage-script bug, was triple-filed). Gap issues filed: **#970** saving throws (gates #962), **#971** ConditionBehavior.Ref() (Kirk's decision, made visible), **#972** character.Data round-trip/dual-home audit, **#973** delete dead mechanics/* | toolkit issues, board 19 |
| 2026-08-14 | Design inputs for #965/#966/wave 5 harvested from old-stack defects, recorded on the tracker: hostility gate at reaction triggers, consciousness gate on Walk, weapon identity in the damage shape, no-magnitudes-in-host pin, does-Dissolve-sweep-Rage (#809) | toolkit#959 comment |

## Carried follow-ups — filed, none blocking

- **[#948](https://github.com/KirkDiggler/rpg-toolkit/issues/948)** `character.LoadFromData`
  silently drops conditions it cannot load, and the next `ToData()` persists the loss. Pinned not
  fixed — the policy belongs to the character package. **Directly relevant to W3's second step.**
- **[#933](https://github.com/KirkDiggler/rpg-toolkit/issues/933)** `Members()` reports room but
  not position. Felt already: a W3 test had to use `Exit` because no read verb lists members.
- **[#940](https://github.com/KirkDiggler/rpg-toolkit/issues/940)** beats addressed to every member
  regardless of perception — harmless as a query, a fog-of-war leak as a push channel.
- **[#941](https://github.com/KirkDiggler/rpg-toolkit/issues/941)** story tags coarser than beats,
  so the SDK reads event kind from a payload it doesn't own — and fails *silently* if that shape
  changes.
- **[#934](https://github.com/KirkDiggler/rpg-toolkit/issues/934)** two validation asymmetries from
  the anchoring wave.
- **[#951](https://github.com/KirkDiggler/rpg-toolkit/issues/951)** `encounter.ErrNoMember` means
  an empty ID, an **absent** member, and a **duplicate** one — opposites under one sentinel. The
  SDK cannot translate what it cannot distinguish, so joining someone twice reports "no such
  member". An `ErrMemberExists` sentinel was written and **removed before shipping** rather than
  exported unpopulated; current (wrong) behaviour is pinned through both verbs so the fix announces
  itself.
- **[rpg-project#218](https://github.com/KirkDiggler/rpg-project/issues/218)** every hand-maintained
  module list in the toolkit **fails open** — 3 instances found in one afternoon. Rule to encode:
  *stable claims in hand-written docs, volatile claims derived or tested.*
- **Retention default → 8** — fold into the next encounter touch, not a standalone PR.

In every composition case the layering holds: **fixing it in the composition fixes the SDK for
free.**

## Other lanes

- **Semantic scope (#180)** — parked. Wave 0 is live-verified (proto root `v0.1.120`, toolkit
  `encounter/v0.49.1`, rpg-api#771; evidence on rpg-project#192). Wave 1 implementation has not
  started; issues can be cut when the lane is picked back up.
- **rpg-api#793 wiring** is Platform's whenever they pick it up. Nothing blocks it; the map surface
  is live and the Locate→Move trap is called out for them explicitly.

## Pointers

- **Decisions:** toolkit **`docs/adr/DECISIONS.md`** — the cliffnotes digest of all 38 ADRs, one
  or two lines each with the rule each generalises to. **Read this, not the ADR corpus:** loading
  38 ADRs is expensive and imports baggage (three propose modules never built; numbers collide;
  status fields are unmaintained). Enforced by `scripts/check-decisions.sh` in CI, so a new ADR
  fails until it is summarised. Open a full ADR only to contradict one or to get its trade-offs.
- **Before designing anything touching turns, time, perception, story or suspension:** read
  toolkit **`play/README.md`** and the relevant `play/*` `doc.go`. Four leaves — `clock`, `intel`,
  `interrupt`, `record` — each with a numbered R1–R10 contract, each returning values and never
  publishing. **This lane already lost a full turn re-deriving `play/clock` from scratch**
  (toolkit#954); the root `CLAUDE.md` now carries the layering, and each rulebook's own
  `CLAUDE.md` says which of its packages plays which part.
- **W4's charter** (deflect / ponder, six clauses) is in `docs/ideas/session-sdk/plan.md`.
- **Deep record:** toolkit `docs/ideas/session-sdk/{design,plan}.md` (the triplet — design is
  ratified-and-annotated, plan is rewritten freely), `docs/ideas/encounter-transitions/`,
  `docs/ideas/encounter/`, `docs/journey/051+052`.
- **Grades and what would raise them:** toolkit `docs/quality.md` (session holds **A-**).
- **Doc ownership:** each code repo owns its own status/quality/architecture; rpg-project says how
  we *work*, not what was done — except `docs/ideas`, which records the plan as it stood.
- **Co-location rule:** each wave's implementation PR amends the design doc in the same commit that
  makes the amendment true. The drift #936 had to fix existed *because* the doc lived on a
  different branch from the code.
- **Full narrative history** of this lane, session by session, is in this file's git history —
  `git log -p sessions/active.md`, tip of the long-form version at **b21a49e**.
