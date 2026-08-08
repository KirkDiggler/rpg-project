# Monster AI: implementation plan

**Parent:** rpg-project#201 · **Tracking:** PR #202 · **Authority:** [`design.md`](design.md) (approved 2026-08-08)

Phase 1 is three slices; each runs internal → visible in the game and
leaves a tool behind. Slice 1 is planned to the bone here. Slices 2–3 are
planned at task level **deliberately** — their fine detail gets written
after the preceding slice teaches us the seams (that's the method, not a
gap). Slice 4 (world tick) stays deferred; no planning here.

## Delivery order

### Slice 1 — targeting end-to-end, first decision breadcrumbs

**Scope:** `targeting:` settable on a placed monster in dungeon YAML, the
monster attacks *and moves toward* the strategy-chosen target, and the
combat log says why — rendered from a reference, in D&D voice. Rider: the
no-fallback rule lands (empty `DataJSON` can no longer enter an encounter).

**Contract decisions (locked for this slice):**

- YAML vocabulary: `targeting: closest | lowest-health | lowest-ac`
  (the builder concept's exact strings). Parsing lives in the monster
  package (`ParseTargetingStrategy(string)`); `dungeonspec` validates
  through it — same coupling direction as its existing `monsters.Refs()`
  use.
- Precedence — and the zero-value trap: `TargetClosest` is the iota
  zero, so `SpawnInstruction` carries `Targeting
  *monster.TargetingStrategy`, **nil = unset**. Nil leaves the
  constructor's choice standing (wolf ships `TargetLowestHP`); non-nil
  always wins, *including* an explicit `targeting: closest` overriding
  the wolf. A plain value field cannot express this — don't "simplify"
  it. (`omitempty` on `DataJSON` still makes explicit-closest look unset
  at that layer; fine there, because by then the value is already
  applied.)
- `defaults:` (dungeon-wide per-ref, design §2) is **deliberately
  deferred to slice 3** — it earns its keep alongside profiles. Slice 1
  is per-placement only.
- Movement semantics: the strategy target is computed once from pre-move
  perception and movement paths toward it (replacing the hardcoded
  `ClosestEnemy()`). Action-time target selection is already
  strategy-based and stays as is. Two edge policies locked for slice 1
  so they aren't relitigated mid-implementation: an unpathable strategy
  target means the monster stalls in place (acceptable — the breadcrumb
  still says what it wanted); a monster adjacent to a non-target will
  walk out of melee and eat the opportunity attack (acceptable — that's
  honest disposition behavior). Deeper turn-target coherence (post-move
  re-sorts, ranged preferences) is slice-3 territory.
- Breadcrumb shape: **rationale is a reference, not prose** —
  `dnd5e:targeting:lowest-hp` rides a new additive field on
  `ActionResolved`; the client maps ref → narration ("turns on the most
  wounded"), per the Boundary Rule. Server-authored prose stays
  impossible, by design. This is the breadcrumb shelf's first board:
  one structured field, two renderings (log now, debug view later).

**Cross-repo motion** — the first full wave, so it also proves the rails.
**Develop outside-in**: the edge proves the shape before the contract
transcribes it. The builder concept already proved the `targeting:`
vocabulary; the remaining edge work is the web combat-log line built
against fixture events (which rationale refs exist, what the D&D voice
says) *before* the proto field is released. **Merge inside-out**:
`rpg-api-protos` releases → `rpg-toolkit` tags → `rpg-api` pins → `rpg-dnd5e-web`
lands last. The numbered steps below are the merge/landing order. One
issue/branch/PR per repo, cut from each repo's base branch
(protos/toolkit: `origin/main`; api/web: `origin/dev`).

1. **rpg-api-protos** — additive field on `ActionResolved`
   (`dnd5e/api/v1alpha2/encounter/events.proto`): `string
   target_rationale` (a ref; empty = no rationale). Additive only — no
   `buf breaking` involvement. Release the contract. *This invokes
   design §7's revisit clause (the breadcrumb needs a signal) and
   amends the design's "no proto changes in phase 1" expectation —
   recorded in design.md §2/§7 alongside this plan, not silently.*
2. **rpg-toolkit** — one branch, accumulating:
   - `rulebooks/dnd5e/monster/targeting.go`: `ParseTargetingStrategy`,
     `String()` for the three values.
   - `encounter/dungeonspec/spec.go`: `Targeting *string` on
     `PlacedEntry` **and** `BossEntry` (decode is
     `KnownFields(true)`-strict — the field must exist before any YAML
     uses the key). `validate.go`: reject `targeting` on prop refs
     (mirror of the `blocks_movement only valid on props` pattern);
     reject unparseable values. `compile.go`: thread through both wiring
     sites — the `refTypeMonsters` case (~:252) *and* the boss spawn in
     `compileWithConfig` (~:126).
   - `encounter/seed_monsters.go`: `SpawnInstruction` gains `Targeting
     *monster.TargetingStrategy` (nil = unset; single-site change —
     dungeonspec aliases this type); `SeedMonsters` applies non-nil via
     `SetTargeting` after `ctor(...)`, **before**
     `json.Marshal(mon.ToData())`.
   - `rulebooks/dnd5e/monster/monster.go`: extract the strategy pick to
     return the `PerceivedEntity` (index-based variant of
     `selectTargetByStrategy`, which keeps `Position`); `moveTowardEnemy`
     takes that entity instead of calling `ClosestEnemy()` (:642);
     `TakeTurn` computes it before the move (:461). Rationale recorded
     alongside so captured attacks carry it into
     `publishAttackOutcome` → `ActionResolvedEvent.TargetRationale`.
   - **Rider:** `AddMonster` rejects empty `DataJSON`; `npcActScripted`
     deleted, and `NPCAct` **errors loudly** (typed sentinel) if it ever
     meets an empty-`DataJSON` monster — the back door guarded as well
     as the front. Persisted-encounter compatibility is a non-issue and
     the plan says so deliberately: both production seeders have always
     marshaled `DataJSON`, and we're pre-pre-alpha besides. Fixtures
     rebuilt on real registry monsters.
   - Tag on merge, in two steps because these are separate Go modules:
     `rulebooks/dnd5e` tags first, then `encounter` bumps its pin to
     that tag and tags itself. Intra-branch, `encounter` needs a
     pseudo-version bump of `rulebooks/dnd5e` so `dungeonspec` can call
     `ParseTargetingStrategy` — expected, not a smell.
3. **rpg-api** — pin the tags; `translate.go` maps the new event field;
   `internal/content/dungeons/reference-tomb.yaml` gives one hall
   skeleton `targeting: lowest-health`. Content must land **after** the
   toolkit pin — `TestEveryEmbeddedSpecLoads` is the canary that enforces
   the ordering (it reds if the YAML key precedes the toolkit field).
4. **rpg-dnd5e-web** — `encounterStreamDispatch.ts` passes the field
   through; `useCombatLog.ts` carries it on the `actionResolved` entry;
   `CombatLog.tsx` maps rationale ref → D&D-voice line. Ref-keyed map,
   no server prose.

**Tests (by existing convention):**

- toolkit `dungeonspec/validate_test.go` — table entries: targeting on a
  prop ref rejected; bad value rejected; valid values on place + boss
  accepted. `compile_test.go` — spawn instruction carries the strategy
  from both wiring sites.
- toolkit `monster/targeting_test.go` (suite) — parse/String round-trip;
  `monster/behavior_test.go` — **the missing test**: monster with
  `TargetLowestHP` moves toward the wounded distant enemy, not the
  closer healthy one.
- toolkit `encounter/seed_monsters_test.go` — YAML strategy survives
  into `DataJSON`; constructor default preserved when YAML is silent;
  YAML override beats wolf's constructor default — **including explicit
  `closest`** (the nil-pointer semantics test). Plus: `AddMonster`
  empty-`DataJSON` rejection, and `NPCAct`'s loud error on an
  empty-`DataJSON` monster.
- toolkit `encounter` events — the middle link the other tests miss: an
  NPC attack emits `ActionResolvedEvent` with the rationale populated
  and it **survives the broker serialization round-trip**; player-path
  attacks emit it empty. Without this, slice 1 could ship green with
  the field silently empty end-to-end.
- web `useCombatLog.test.ts` / `CombatLog.test.tsx` — rationale ref
  renders the narration line; absent rationale renders today's line.

**Acceptance (local env, PR evidence per test-before-merge):** in the
reference tomb, a `lowest-health` skeleton visibly ignores the closer
healthy fighter, moves toward and attacks the wounded wizard, and the
combat log reads the choice in the game's voice. Screenshot + log excerpt
on the toolkit and web PRs.

**Authoring surface (added 2026-08-08, Kirk):** hand YAML is slice 1's
authoring path, not the destination — the dungeon builder needs controls
to *set* these values. Tracked as
[rpg-dnd5e-web#736](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/736):
a targeting select on the placement/boss inspector (the doc model already
carries the field) plus un-stripping it from `stripToV1Subset` once the
server accepts the key. Blocked on this slice's merge train; aligned with
dungeon YAML v0.4 tranche C (rpg-project#206). Builder controls for
`profile:`/`params:`/`defaults:` follow slice 3 the same way.

### Slice 2 — the perception component

Planned in detail **after slice 1 merges** (the rationale threading will
have taught us how decision context travels). Committed shape from the
design: stimulus → knowledge with timestamped beliefs; sight is channel
one (`CanSeeAt` + `SensesData`); allies populated; last-seen memory;
**[shelf — sound]** reserved as a typed stimulus kind. `buildPerception`
(`encounter/npc.go:1096`) is the seam — today it hands monsters every
living player, unfiltered. Acceptance: a monster neither attacks nor
paths toward what it hasn't perceived; breadcrumbs state beliefs
("last saw wizard at [5,3]"); sneaking behind a wall mid-combat works.

### Slice 3 — `behavior/` v1: two modes + lapse

Planned in detail **after slice 2 merges**. Committed shape:
`behavior/` implements `NewMachine(cfg *MachineConfig)` + `Step`
(`StepInput`/`StepOutput` with `Because`); dnd5e monster declares
`ModeCombat`/`ModeFleeing`; profile → knobs at compile time
(`flee_at_hp_pct` first); the `defaults:` block (dungeon-wide per-ref,
deferred from slice 1) lands here with profiles; N-turn search window;
lapse via the existing pocket-cleared exit path. `hiding` is **not
built** — it's the owner's first extension. Acceptance: a `timid` ghoul at low HP flees, vanishes
from player screens, the search window runs, and combat lapses back to
free roam mid-dungeon.

### Slice 4 — world tick (deferred)

Not planned. Architecture re-decided after slice 3, per design §6.

## Cross-repo rules and gates

- One issue, one branch, one PR per repo per slice; issues on board 19
  (Feature: The Dungeon). Branches cut from `origin/<base>` fresh
  (protos/toolkit `main`; api/web `dev`).
- A slice's toolkit branch **accumulates** — integration findings are the
  method working, not new PRs (Fog of War lesson). The consumer defines
  done.
- Merge inside-out with real version pins: toolkit tags on merge, api
  bumps to tags, web follows. Pre-release tags if api needs unmerged
  toolkit work; local override only for tight iteration, never committed.
- Evidence standard: branch verification in the local env before merge,
  screenshots/log excerpts on the PR. No draft PRs.
- PR #202 stays open as the tracking surface; slice status gets recorded
  here as each slice merges.
