---
status: DESIGN, proposed 2026-09-05 (supersedes the 2026-09-04 draft; the survey it carried moved to ../disposition/brainstorm.md)
journey: rpg-project#326 · brainstorm: ../disposition/brainstorm.md · tool 1: ../intel-record/design.md (shipped 2026-09-04)
law: the run's world is the only state — this slice moves two readers into it: sides, and knowledge
---

# The hold-out — design

## 0. Purpose

Three tools for the designer, proved by one scenario:

1. **Factions with a disposition** — who fights whom, and the predicate that
   changes it.
2. **The predicate** — one authorable grammar (`round | down | fact`) used by
   two fields.
3. **Arrivals** — a placement that enters the run when its predicate holds.

The scenario: a raider camp is hostile to the party until its chief comes to
know the party saved the Wiseman. A messenger's letter carries that fact; carry
it to the chief and the camp stops attacking. Kill the chief instead and
reinforcements arrive at the gate.

## 1. The file

```yaml
factions:
  - { id: raiders, mind: chief }

dispositions:
  - { between: [raiders, party], stance: hostile, until: { fact: saved-wiseman } }

intel:
  - { id: wisemans-letter, reveals: { fact: saved-wiseman } }

place:
  - { id: chief,  ref: "dnd5e:monsters:skeleton-captain", at: [12,4], faction: raiders }
  - { id: scout,  ref: "dnd5e:monsters:skeleton",         at: [4,2],  faction: raiders }
  - { id: letter, ref: "dnd5e:props:scroll", at: [1,3], holdable: true,
      blocks_movement: false, blocks_los: false,
      holds: [wisemans-letter], arrives: { round: 6 } }
  - { id: reinforcement-1, ref: "dnd5e:monsters:zombie", at: [1,4], faction: raiders, arrives: { down: chief } }
  - { id: reinforcement-2, ref: "dnd5e:monsters:zombie", at: [2,4], faction: raiders, arrives: { down: chief } }
  - { id: reinforcement-3, ref: "dnd5e:monsters:zombie", at: [1,5], faction: raiders, arrives: { down: chief } }

exits:
  - { id: front-gate, at: [1,3] }

scenarios:
  hold-out: { convince: raiders }
```

## 2. Vocabulary (dungeonspec v2, additive)

| field | type | rules |
|---|---|---|
| `factions[].id` | id | unique; `party` is never declared (nothing about it is authorable); `monsters` MAY be declared — one line gives the default monster side a mind (`{id: monsters, mind: chief}`, the mind being a placement with no faction key). FINAL 2026-09-05 after three crossings; refusal when a disposition waits on an undeclared `monsters`: "faction \"monsters\" is not declared, so it has no mind — declare it under `factions:` to give it one" |
| `factions[].mind` | placement id | the hub word spreads through: the faction knows what its mind knows. MUST name a monster placement in this faction. Optional: a faction of one has its member as mind; a faction of many with an `until: { fact }` and no mind is refused ("name a mind, or the faction cannot learn") |
| `place[].faction` | faction id | monsters only; MUST name a declared faction; absent → the reserved `monsters` faction |
| `dispositions[].between` | `[faction, faction]` | both MUST exist; the reserved `party` and `monsters` are nameable here (`between: [monsters, party], stance: neutral` makes the unauthored monsters neutral) though never declarable; unordered; a ≠ b; one disposition per pair |
| `dispositions[].stance` | `hostile \| neutral \| allied` | closed set |
| `dispositions[].until` | predicate | legal only with `stance: hostile`; when it holds the stance becomes `neutral`. **This slice: `{ fact }` only** — `round`, `down`, `stance` are refused at dungeonspec AND at the run ("in this version a disposition turns only on a fact; `until` on a round, a fall, or another stance is not built yet"): Settle keys on a flag a journal fact raises, and rounds and Standing are not journal facts yet |
| `intel[].reveals` | `{ door: id } \| { fact: id }` | exactly one key; `fact` ids are plain strings, declared by mention |
| `place[].arrives` | predicate | monsters and props; the placement is in reserve until it holds; `at` MUST be floor |
| `scenarios.hold-out.convince` | `entity_ref(faction)` | the scenario's only field |

**Predicate grammar** — exactly one key. A predicate IS an encounter `Trigger`
(`encounter/field.go:793`, the sealed set endings already use: ReachedPosition,
MemberDown, External, ExitedHolding). The designer's spelling compiles to the
engine's type; `until` and `arrives` are the Trigger set's second and third
consumers; `round`, `fact`, and `stance` are three new Trigger types.

| form | holds when | grain |
|---|---|---|
| `{ round: N }` (N ≥ 1, refusal wording "a round is counted from 1") | any fight in the run has started round N | truth |
| `{ down: <placement id> }` | that member is Down | truth (reads Standing) |
| `{ fact: <id> }` on `until` | the faction's `mind` knows the fact | audience (the mind's) |
| `{ fact: <id> }` on `arrives` | the fact exists in the run's journal, learned by anyone | truth |
| `{ stance: { between: [a, b], is: hostile \| neutral \| allied } }` | the pair's stance folds to that value | truth |

The grammar is a closed set that grows one form per use case, sealed the way
`Trigger` is. `endings[]` in the file (step B, R10) takes the same grammar:
`{ id, when: <predicate> }`, so `scenarios.hold-out.convince` is sugar for
`endings: [{ id: turned, when: { stance: { between: [raiders, party], is: neutral } } }]`.

**Defaults that keep today's dungeons unchanged:** every unauthored monster is
in `monsters`; `party` and `monsters` are mutually hostile; every faction is
allied with itself; a declared monster faction with no disposition toward
`party` is hostile to it; declared monster factions are neutral to each other.

**Refusals, written for the form-filler** (dungeon): unknown faction on a
placement; a `mind` outside its faction; `party` declared; two dispositions for
one pair; `until` on a non-hostile stance; `arrives.at` not floor; an unknown
placement in `{ down }`. The dungeon ALLOWS an `until` fact no record reveals
(pre-release: show the cost). The SCENARIO refuses it: `hold-out` refuses when
no faction is bound, the bound faction has no `mind`, no hostile disposition
with `until` exists between it and `party`, or nothing reveals that fact — "a
hold-out nobody can win".

## 3. The run's world (encounter)

**MUST**

1. One journal and one graph exist from `New` and from `Load`, whether or not
   anything is concealed. Entities: members, factions, regions, doors, props.
   Edges: `belongs-to` (member → faction), structure (as today), and one
   `hostile-to` / `allied-with` edge per direction from the declared and
   default dispositions.
2. `MemberKind` stays a kind. The three side readers — formation
   (`trigger.go` `sidesInContactOrder`), `fightIsDecided`, `bubbleHasPlayer` —
   ask the graph: two members are opposed iff a `hostile-to` edge stands
   between their factions. A member in reserve or Down appears in no pair.
3. Knowledge is facts with audiences in the one journal: `known:door:<id>` and
   `holds:intel:<record>` as shipped, with the receiver as audience;
   a `fact` reveal writes `known:fact:<id>` with the RECEIVER as Subject and
   as audience (the kernel's `Raise` flags a fact's subject, so the learner
   must be the subject for `Settle{Of: mind}` to fire). `knowsFact(member,
   id)` is a journal fold over facts of that kind whose subject is the
   member; no `fact:<id>` entity, no reader keeps a copy.
4. The disposition flip lives in the graph, not beside it (**Kirk 2026-09-05:
   "the graph should tell the truth"**). world v0.3.0 could not say it: every
   reducer moves one entity and every projection rewrites one entity's own
   edges, so a pair flipping both directions had no home. world gains a PAIR
   projection `graph.Settle{OnFlag, Of, Between, Relations, To}` (v0.4.0,
   branch `world/pair-settle`, head e9da06e, PR toolkit#1519): while the flagged entity
   carries the flag, the pair's edges settle to the target relation in both
   directions; precedence is declared order, last wins, pinned by one test.
   The encounter declares `Raise{On: known:fact:<id>}` + that projection for
   each disposition with an `until`, folded as the faction's mind. `HasEdge`
   is the whole reader; no composition fold, no stored stance. The rejected
   branch — a composition fold over a graph that keeps saying hostile — was a
   second mechanism answering a question the graph owns.
5. When a flip removes hostility between two factions and a fight is formed
   between members of those factions, the encounter dissolves it with a new
   sealed cause `ByStance()`. As built: a fight with no opposed pair left
   dissolves; a fight that still has one keeps running, and every member now
   opposed to nobody is transferred to the world clock (in no pair, in no
   fight). Presence transfer also runs at the end of Hold and Loot, so a
   letter picked up in the hut teaches the mind then, not on the next verb.
6. Presence transfer rides `sweepOccupancy`: a member holding a record whose
   `reveals` names a fact, standing in the region of a faction's `mind`,
   teaches the mind (the record copies; the holder keeps it).
7. A placement with `arrives` is in reserve: no cell, no turn, in no pair,
   absent from every projection for every member (the never-authored
   yardstick: a run with reserved placements projects byte-identically to one
   without them, until arrival). On the first verb after its predicate holds
   it is placed at `at` — nearest free floor cell in the same region if `at`
   is occupied — with an `arrived:<id>@<cell>` fact; the same verb's sight
   refresh then forms or joins a fight as for any member walking into view.
8. Predicates are Triggers and are evaluated where endings already are: at
   the one place each event is noticed (`noticeDown` for `{ down }`,
   `RoundStarted` for `{ round }`, the fact append for `{ fact }`, the fold
   after a flip for `{ stance }`), before that verb's sight refresh. `{ round }`
   outside any fight never holds. Standing does not move into the journal in
   this slice. Liveness refusals extend to the new forms (an `arrives` or
   `until` that can never hold is refused like an unreachable ending).
9. A faction whose `mind` is Down can no longer learn: `until: { fact }` can
   never hold for it. This is a consequence, not a loss. Mind succession is a
   shelf.
10. `scenarios.New` declares `Endings: [TriggerStance{Between, Stance ≠ hostile}]`;
    the existing withdrawal and defeat endings stay.

**Spawn** (rpg-api → session → encounter): `MemberInput.Faction` and
`MemberInput.Arrives` are hand-carried like `Holds`. Content for a reserved
monster resolves at launch; the encounter holds the member in reserve.

**Member ids (ruled 2026-09-05, landed in rpg-api):** an authored placement id
IS the member id, so `factions[].mind` and `{ down: <id> }` mean the same id
in the file and in the run; unnamed placements keep ref+ordinal minting, and
an ordinal is spent on every placement of a ref, named or not, so naming a
sibling never renumbers a monster nothing about which changed. A collision is
refused at compile (authored vs minted, naming both) and again before any
write at launch (against the party's ids and the demo vendor). Consequence:
the heirloom tomb's captain is now member `captain`. Fact ids and faction ids
are carried verbatim (like regions and exits), not key-minted like doors and
intel records.

## 4. Resolution

`castView.IsHostile` / `IsAllied` ask the reloaded run's graph (resolution's
`Input.World` is already the encounter's data). `castRelations` and the
`cast-side` entities are deleted. Resolution's encounter pin moves from
v0.51.0 to the tag carrying §3. Sneak Attack and Pack Tactics change nothing
and answer from the run.

## 5. Session

- `Spawn` carries `Faction` and `Arrives`.
- Roster row gains `faction`; reserved members are absent from roster and
  atlas until arrival (member-scoped by absence, as concealed doors are).
- Beats: `stance` `{between: [a, b], stance}` on a flip; `arrived`
  `{id, kind: monster|prop, cell}` on arrival; the `ended` beat names the
  hold-out ending.
- Capabilities unchanged (Witness, CheckResolver, Sight, Roller, TurnDriver).

## 6. Wire (protos, additive) — landed as rpg-api-protos PR #293

- `PublicMemberInfo.faction` — string, free-form (factions are content, never
  an enum). Empty = in no faction, which this slice reads as a world NPC
  (`place[].faction` is monsters-only; fadedpez's default is a shelf, §11).
- Beats `STANCE_CHANGED {between: [a, b], stance}` and
  `ARRIVED {id, kind: PlacementKind, cell}`; `PlacementKind` is a closed enum
  (MONSTER | PROP) because a client branches on it and a prop is not a member.
  Both beats go to everyone in the run: a stance is truth grain like a door's
  state, an arrival is physical state like HELD/DROPPED.
- `DISSOLVE_KIND_BY_STANCE` on `FightEnded.cause` — the wire mirror of
  `ByStance()` (§3.5); without it a flip's FIGHT_ENDED would carry
  UNSPECIFIED, which the file defines as a producer defect. Found by the protos
  build; the session wave maps the sealed cause to it.
- `PutDungeon` verbatim; `ListScenarios` gains the `faction` entity kind on
  its open string vocabulary.

## 7. The designer (web)

- Dungeon sections beside Scenarios and Intel: **Factions** (id; mind =
  dropdown of monsters placed in that faction) and **Dispositions** (faction,
  faction, stance select, `until` = predicate editor, shown only for hostile).
- **Predicate editor**, one component used by `until` and `arrives`: form
  select (`round` → number; `down` → dropdown of placed monsters; `fact` →
  dropdown of fact ids declared by intel records, free text allowed with the
  "no record reveals this" note).
- Placement inspector: `faction` dropdown on monsters; `arrives` predicate
  editor on monsters and props.
- Intel form: `reveals` kind select (`door | fact`) and the id.
- Scenario tab: `hold-out` with the `convince` dropdown. The description
  waits on #372's scenario-tab design: `ScenarioDescriptor` carries a name and
  per-field guidance and no description field (found by the web build).
- Play: roster coloured by faction; `stance` and `arrived` beats narrated;
  reserved placements never drawn.
- YAML round-trips byte-stable; every refusal in §2 renders inline at the
  field it names.

## 8. Rulings (proposed; Kirk vetoes on the PR)

| # | ruling | proposed |
|---|---|---|
| R1 | a flip dissolves a formed fight between the two factions | yes, `ByStance()` |
| R11 | where the pair flip lives | **RULED A (Kirk):** a world/graph pair projection, v0.4.0; the graph tells the truth |
| R2 | the stance after `until` holds | `neutral` ("not hostile"); allied is authorable only as a static stance |
| R3 | presence grain and the hub | the mind's region, the yardstick Search uses; the faction knows what its mind knows (Kirk 2026-09-05: "at a faction level it makes sense"). A pair folds as every mind of the pair (any mind knowing turns it); a pair with no mind folds as the empty observer, never Truth; the mind is the declared one or the sole member of a faction of one AS AUTHORED — no fallback (R7) |
| R4 | `party` and `monsters` | reserved; unauthored monsters are `monsters` |
| R5 | predicate grammar and grains | `round \| down \| fact`; `fact` = mind's knowledge on `until`, truth on `arrives` |
| R6 | reserved placements | spawned at launch, absent from every projection, placed on the first verb after the predicate holds |
| R7 | the dead mind | the flip is gone; consequence not loss; succession shelved |
| R8 | `until` naming an unrevealed fact | dungeon allows, scenario refuses |
| R9 | which clock `{ round }` counts | any fight in the run; outside a fight never; the world clock stays postponed |
| R10 | `endings[]` authorable in the file with the predicate grammar | step B; the hold-out's `convince` becomes sugar; a scenario package with nothing left to do is the north star's own test |

## 9. Acceptance

| # | proof | how |
|---|---|---|
| A1 | camp `hostile until {fact}`; nobody knows; a fight forms on sight | scene |
| A2 | the letter carried into the chief's region mid-fight: stance neutral, fight dissolves, hold-out ending fires | scene + walk |
| A3 | the letter carried into the scout's region: nothing | scene |
| A4 | chief Down: reinforcements at the gate on the next verb, hostile, fight forms or joins | scene + walk |
| A5 | the letter arrives at round 6 and not before | scene |
| A6 | reserved placements: projection byte-identical to a run without them, for every member, until arrival | scene (the yardstick) |
| A7 | every pre-existing encounter, resolution, and session scene passes unchanged under the default factions | the rung-1 bar |
| A8 | after a flip, a skeleton is no longer an enemy for Sneak Attack; before it, unchanged | scene |
| A9 | save after the flip, load: still neutral; no stance stored | scene |
| A10 | the whole file authored through forms; YAML round-trip byte-stable; refusals inline | screenshot + test |
| A11 | Kirk walks both branches on one stack before any PR opens | the walk |

## 9b. Walk 1 findings (Kirk, 2026-09-05, local stack 3010/8090)

1. **The lobby's Dungeon dropdown left on its placeholder silently means "the
   server's default", the heirloom tomb.** Kirk walked recover-the-artifact
   twice believing it was the camp (two skeletons, the hall scroll, the tomb's
   captain who kills a carrier). Pre-release: a cost to show, not a hazard —
   picker finding for #372 (name the default in the placeholder).
2. **The chief's own strike fails at resolution's reload** — `drive monster
   turns "chief": execute: strike: strike: encounter closed` on EndTurn and
   `… invalid encounter data` on Exit — with the fighter holding the letter in
   the yard, the chief in the fight by line of sight, and the flip not yet
   fired (state verified in redis: correct so far). The scout's strike works.
   No scene ever let the mind take a swing: the session drove the scout by
   Pass, the api's A2 dissolved the fight before the chief could act.
   ROOT-CAUSED (session builder, at the seam; resolution's reload not
   involved) to two encounter defects: (a) `driveOneMonsterTurn`'s intent
   loop keeps executing after an intent ended the fight — the chief STEPS
   into the yard where the carrier stands, presence teaches him (presence is
   symmetric by construction: same region, R3), the camp turns, the fight
   dissolves, the ending closes the run, and the loop's next intent is
   Attack → "encounter closed"; with no ending bound he strikes a party he
   is no longer opposed to. Fix: stop the driven turn when the fight it
   belongs to is gone. (b) `Exit` removes the exiter from the canvas before
   `leaveAnyClock` drives the next monster, whose strike reads the roster
   and finds a member with no place → "invalid encounter data". PRE-EXISTING
   on main, independent of factions. Fix: roster delete before the drive.
3. **The client hid the reason.** "Check available actions" was all the table
   saw for three refused EndTurns; the server's message named the chief's
   strike. Surface the server's reason (web polish).

**Walk 2 (2026-09-05, rebuilt stack from api 160f7f0): the win condition
WALKED.** Two players, the raider camp, the letter to the chief, the camp
turned, the hold-out ended. Finding filed: a party member who leaves through a
door leaves a shadow on the other player's map that blocks movement planning
(client-side — the server refused no Move for occupancy; rpg-dnd5e-web issue
filed from the walk). Step B starts.

## 10. Cut

- **Step A — sides and knowledge.** §2 without `arrives`; §3 items 1–6, 9, 10;
  §4; §5 without `arrived`; §6 without `ARRIVED`; §7 without the arrives
  field. The letter lies at the gate; no reinforcements. Walkable: hold the
  letter, carry it to the chief mid-fight, the camp turns.
- **Step B — arrivals and authored endings.** The predicate grammar in full,
  `arrives` on props and monsters, reserve, `arrived`, `endings[]` (R10), the
  predicate editor. Walkable: the letter at
  round 6; kill the chief, reinforcements.
- Both steps on branches, one local stack, Kirk's walk, then PRs bottom-up:
  dungeonspec + encounter → resolution → session → protos → api → web.

## 11. Shelves (named, empty)

- **Word spreads** (Kirk 2026-09-05: "the local goblins get the message and
  the faction mind get it 1 turn later"). Knowledge propagates along
  membership to the mind with a latency: `factions[].spread: { turns: N }`.
  Mechanism: a scheduled fact — the arrival primitive applied to a fact
  instead of a placement. Local members learn by presence or by witnessing
  the handover (the Witness capability already names bystanders); the mind
  learns N turns later; the stance stays faction-level and flips when the
  mind knows. A skeleton who knows before the edge flips hesitates only if
  behavior reads per-member knowledge (Billy's record). Step A is latency 0
  by presence in the mind's region.
- **Take, the pocket verb** (Kirk 2026-09-05: "take is like holding but goes
  into inventory. hold is meant to be a temp state and possibly taking up 1 or
  2 hands. we trade from our inventory"). Two verbs, two nouns, two lifetimes:
  Hold carries a PROP in hand for the run (`hold.go` already says so: hands a
  named shelf, inventory untouched); Take pockets an ITEM onto the character's
  sheet, which survives the run and is what Trade draws from. The shape to fill
  when it arrives: (a) a prop's item identity, `place[].item: <equipment ref>`
  — absent means hold-only, the zero value that keeps an heirloom chest in two
  hands; (b) the run's side, a `taken:<prop>` fact and the prop leaves the atlas
  as Hold's does; (c) the sheet's side, a runtime "item enters inventory" path
  the character does not have today (`InventoryItemData` is draft-compile only)
  and which Trade's receive side needs too — the second instance, so it is built
  once for both, as a game-context request event the character's keeper
  applies. Two breaks to rule then: the withdrawal ending names a HELD item, so
  it becomes `with` (held or pocketed by a member at the exit) or the artifact
  stays hold-only by having no `item`; and slice 2's drop rule protected the
  run's win from a disconnect, while a pocketed win leaves with the character —
  the campaign has it, the run does not. Both verbs apply intel reveals.
  Kirk 2026-09-05: Trade lands first, and Take is "trading an object for
  nothing" — Trade's shape with an empty give side and the field as the
  counterparty; it breaks out into its own verb only if it needs more shape.
- **A hold costs a hand** (Kirk: "we could even configure the scenario to say
  it takes away a hand"): `holdable: { hands: 1 | 2 }` on a prop, read by the
  rulebook's equipment rules when hands exist (`hold.go`'s named shelf).
- **Trade for the next quest** (Kirk: "npcs have a trade verb and I can see
  trading for a next quest"). fadedpez's Trade appends a fact; quest
  availability is a predicate over journal facts (brainstorm §10), so a trade
  that unlocks a quest is the same grammar at campaign grain. Nothing to build
  until the campaign journal exists.
- **DispositionPolicy is the singleton case.** fadedpez's per-NPC default
  stance (`npc/policy.go:38`) is a faction of one with one edge toward
  `party`; the graph can answer it when his use case pulls it. Seam note for
  his record, not a change here.
- **The fight that dissolves is the scout's, never the chief's** (found by the
  api's end-to-end scene). Inside the hut, presence transfer and the flip fold
  BEFORE that pass's sight refresh (§3.8), so by the time the chief could see
  the carrier the pair is already neutral and no fight forms with him; from
  the yard he never sees through the hut door. In the walk, entering the hut
  dissolves the yard fight. With nobody fighting, the flip still fires
  (STANCE_CHANGED → ENDED, no FIGHT_ENDED): the hold-out is winnable without
  a fight, per R1. If a design ever wants "the chief's own fight dissolves",
  that is a §3.8 ordering question for the toolkit, not a wire defect.
- **A3 as built pins presence, not learning.** In step A no non-mind can
  learn a fact (holding is not knowing: an authored holder carries the record
  unread, as doors work), so "a scout who learned it flips nothing" has no
  scene yet; the learned-scout variant waits on the word-spreads shelf.
- **`until: { round }` would oscillate.** Rounds are current state: a camp
  hostile until round 6 turns at round 6, the fight dissolves, no fight means
  no round, and it is hostile again on the next sight. A latch needs a fact —
  the second reason the run turns on facts alone this slice.
- **Rounds and Standing as journal facts.** Found by the build: `until` on
  `{ round }` / `{ down }` has nothing to flag because neither is a fact in the
  run's world yet; `{ stance }` needs an edge-keyed projection. Under the law
  these are the next two readers to move; until then dispositions turn on
  facts only and the refusal says so.
- **More monsters.** The camp is skeletons, zombies, and the skeleton captain
  because those are the stat blocks and models that exist (Kirk 2026-09-05:
  "make do with skeles, zombies and the skele captain for the leader"; he is
  bringing in more monsters in his own lane). A goblin camp is one ref swap.
- **`count` on a placement** ("3 raiders" as one line): a general placement
  feature, not this slice's tool; three lines until a second author asks.
- hand / throw the letter as a verb · a walking messenger (NPC lane, Interact
  give) · betrayal: attacking a neutral faction writes a fact that flips it
  back · mind succession · directed dispositions · allied-after-flip ·
  world-clock arrivals · behavior reading stance (Billy's record) · Standing
  as journal facts.
