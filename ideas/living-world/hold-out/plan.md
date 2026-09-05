# The hold-out — plan

**Status:** RULED 2026-09-05 (R1–R10 adopted; Kirk: "I am ready to proceed").
Built under the integrate-before-pr rule: **every wave on pushed branches,
never opened; the local stack up from the worktrees at the FIRST walkable
increment; Kirk walks; findings fold back onto the branches; then one bounded
PR per module, bottom-up.** Toolkit modules merge as they come; rpg-api takes
ONE pin at the end.

**Kirk is in the room.** He wants to understand the building blocks as they
land and steer the evolution choices. Three rules for every builder:

1. **Push after every green increment** and end each report with one line of
   "what went in" written for a reader who did not see the code.
2. **A choice the design does not decide is not yours.** STOP, report the
   fork with the smallest honest example of each branch, and wait. Never pick
   silently; never build both.
3. **A wall in the build is a design signal.** If a MUST in design §3 cannot be
   made true without a mechanism the design does not name, report it as a
   finding on the design, not as a workaround in the code.

The director keeps `hold-out/how-it-works.md` (mermaid, team-facing) growing
one mechanism at a time as each lands: sides · knowledge · the flip · reserve.

## Ports and manifest

`envs/local/hold-out.env`: web **3010**, api **8090**, project `rpg-local--hold-out`.
Refs point at the pushed branch heads below; the api image is built from the
api worktree; content registry seeds `reference-raider-camp` from the shipped
copy (verify with `docker exec … grep -c raiders /content/reference-raider-camp.yaml`
before handing Kirk the walk).

## The fixture — `reference-raider-camp.yaml`

Design §1's file on a small camp: **gate** region (the front gate exit at the
letter's cell), **yard** (the scout), **hut** (the chief). Party start at the
gate, facing the yard. One copy in toolkit dungeonspec testdata, one in
rpg-api `content/`, one in the web Concepts Lab — byte-identical, pinned by
test in api and web against the toolkit's bytes (the intel-record precedent).
`reference-tomb` and `reference-tomb-heirloom` are NOT touched; A7 says every
existing scene passes under the default factions.

## Wave 0 — protos (branch, opened at once, merges first)

Additive: `PublicMemberInfo.faction` (string); beat payloads
`STANCE_CHANGED {between: [string, string], stance: string}` and
`ARRIVED {id, kind: MONSTER|PROP, cell}`; `ListScenarios` entity kind
`faction`. `buf lint`/`format`/`breaking` green; no hand-written tests. The
generated SHA is the pin for api and web.

## Wave 1a-0 — toolkit `world` (branch `world/pair-settle`, pushed first)

The pair projection (design §3.4, R11): declared on graph.Config, validated
at New, folded never stored, both directions, precedence pinned, per-observer.
The encounter pins its pseudo-version; the world PR opens once the encounter
compiles against it (consumer proof), merges as it comes → world/v0.4.0.

## Wave 1a — toolkit `rulebooks/dnd5e/encounter` (branch `encounter/hold-out`)

Step A first, pushed; step B on the same branch after walk 1.

**Step A (design §2 minus `arrives`; §3 items 1–6, 9, 10):**
- dungeonspec: `factions[]{id, mind}`, `place[].faction`,
  `dispositions[]{between, stance, until}`, `intel[].reveals.fact`, the
  predicate grammar as a type (`round | down | fact | stance`) with the
  refusals in §2 written for the form-filler; `Compiled` carries factions,
  dispositions, and per-placement faction.
- encounter: `MemberInput.Faction` (+ `JoinInput`), the reserved `party` and
  `monsters` factions, ONE journal + ONE graph built at `New` and `Load`
  regardless of concealment (the concealment world and the holdings journal
  fold into it — same fact kinds, now with audiences), the three side readers
  ask the graph, `known:fact:<id>` on transfer, `knowsFact`, the reducer +
  `AdoptStance` projection, `ByStance()` dissolve, presence transfer in
  `sweepOccupancy`, `TriggerStance`, `TriggerFact`, `TriggerRound` as Trigger
  types with liveness validation, `EncounterData` unchanged in shape (facts
  only; stance never stored).
- scenes A1, A2, A3, A7, A9 (+ A8's encounter half: the graph answers
  hostile/allied for resolution to read). Mutation pass: with the stance
  table forced to "never hostile", A1 and every pre-existing formation scene
  MUST fail; with the flip reducer removed, A2 MUST fail. Verify `-v` leaf
  names against the filtered-PASS trap.

**Step B (after walk 1):** `place[].arrives` (props and monsters), reserve
state (§3.7), the scheduled placement on the first verb after the predicate
holds, `arrived:<id>@<cell>`, `endings[]` in the file (R10) compiling to the
same Trigger types, scenes A4, A5, A6 (byte-identical projection with and
without reserved placements, for every member).

## Wave 1b — toolkit `rulebooks/dnd5e/resolution` (branch `resolution/hold-out`, pseudo-pins 1a's head)

`castView.IsHostile`/`IsAllied` ask the reloaded run's graph; `castRelations`
and the `cast-side` entities deleted; pin encounter to 1a's pseudo-version.
Scene A8 (Sneak Attack before and after a flip). Every existing resolution
scene passes unchanged. Starts when 1a's step A head is pushed.

## Wave 1c — toolkit `rulebooks/dnd5e/session` (branch `session/hold-out`, pseudo-pins 1a + 1b)

`SpawnInput.Faction` (and `Arrives` in step B); roster `faction`; reserved
members absent from roster and atlas (member-scoped by absence); `stance` and
`arrived` beats; the `ended` beat names the hold-out ending. Scenes: spawn
with a faction and form a fight by faction; the flip beat reaches every
recipient; a reserved member is absent from every projection (step B).

## Wave 2 — rpg-api (branch `api/hold-out`, off dev, pseudo-pins 1a/1b/1c + protos SHA)

Forward `Faction` (and `Arrives`) through Spawn the way `Holds` is; roster
`faction`; translate `stance`/`arrived` beats; `ListScenarios` serves the
`hold-out` descriptor; the fixture in `content/`; acceptance scenes A2 and A4
end to end through the gRPC surface.

## Wave 3 — web (branch `web/hold-out`, off dev; authoring half starts now)

- **3a authoring (YAML only, no pin):** Factions and Dispositions sections
  beside Scenarios and Intel; the predicate editor (one component); `faction`
  on the placement inspector; `reveals: door | fact` on the intel form;
  `arrives` on the inspector (step B); YAML round-trip byte-stable; every §2
  refusal inline at the field it names; the raider-camp fixture in the
  Concepts Lab; screenshots in `docs/evidence/hold-out/`.
- **3b play (after protos SHA + api):** roster coloured by faction; `stance`
  and `arrived` beats narrated; reserved placements never drawn; the scenario
  tab's `hold-out` form with `convince`.

## The walks (Kirk, on `local/hold-out`)

- **Walk 1 (step A):** author the camp through the forms (or load the
  fixture); the letter lies at the gate; a fight forms on sight; Hold the
  letter, carry it into the hut mid-fight; the raiders stop; the hold-out
  ends. Then: kill the chief before delivering; confirm the camp cannot turn.
- **Walk 2 (step B):** the letter arrives at round 6 and not before; kill the
  chief; reinforcements pour through the gate; an ending authored in the file
  fires.

## Then the PRs (bottom-up, one per module)

toolkit world → toolkit encounter (on world/v0.4.0) → toolkit resolution (on the encounter tag) → toolkit
session (on both tags) → protos already merged → rpg-api (ONE pin) → web.
Slice issues filed under #326 at that point; the numbers relayed to the
builders (never let a builder search for its issue).

## Lanes touched, and the heads-up

encounter `trigger.go`/`standing.go`/`clocks.go` and resolution `cast.go` are
on Billy's path-overlap radar (toolkit#201): a courtesy note goes there when
1a starts. fadedpez's `DispositionPolicy` is a shelf note only (design §11).

## Done-when

Both walks pass on the merged stack; every pre-existing scene in encounter,
resolution, session, and rpg-api passes unchanged; the whole camp was authored
through the forms; the how-it-works page has four mechanisms.
