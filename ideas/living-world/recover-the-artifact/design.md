---
status: RULED 2026-09-04 (R1–R10) — built, WALKED by Kirk the same day (both paths, on a dungeon he authored through the form); merging — plan.md beside this file; PR rpg-project#368 stays open through the build
journey: rpg-project#326 (Living World), slice 2
predecessor: concealed-door/design.md (slice 1, shipped 2026-09-02)
spike: rpg-toolkit examples/world/scenarios/tomb (UC-4)
---

# Recover the artifact — slice 2 of Living World

The scenario, in Kirk's words (2026-09-02): a concealed room, an artifact
placed in it, and a captain who has the map to that room. Two ways to win,
one ending:

1. get in, find the artifact, and leave;
2. kill the captain, learn the room from them, grab the artifact, and leave.

"Filling in a scenario config on the dungeon builder" is the prize. This
document is the design of that form and of the two capabilities it binds.
Capabilities before whole functionality (ruled 2026-09-01) still governs:
each piece below earns its place on its own.

## 0. Vocabulary

- **placement** — one `place:` entry in dungeonspec v2: a ref and a cell.
  Props and monsters are both placements.
- **placement id** — the author's name for one placement, the way a door
  and a region already carry `id:`. Does not exist today.
- **knowledge link** — an author-declared "this placement knows this
  secret" (concealed-door design, named 2026-09-01, deferred until now).
- **intel** — a knowledge fact in the run's journal: `known:door:<id>` or
  `known:region:<id>` with an audience (encounter/conceal.go).
- **loot** — a player verb on a downed member that transfers what the body
  holds to the looter.
- **hold** — the player verb that picks a holdable prop off the floor into
  a holding (R10). Never *take*: that word lands things in inventory.
- **holding** — what a member carries during a run. Does not exist today.
- **the form** — the scenario's config as the builder renders it: field key,
  label, type, guidance; validated by the scenario package's own `New(cfg)`
  (ruled 2026-09-01).

## 1. Ruled 2026-09-04 (Kirk, in session)

- **R1 — Loot the body.** The intel changes hands by an explicit act on the
  fallen captain. The looter gets it **for free — no check.**
- **R2 — That is the victory path.** Path 2 is loot → open → take → leave.
  No knowledge transfers on the kill itself; a party that kills the captain
  and never loots learns nothing.
- **R3 — Shelved, not built** (Kirk's own list, §9): the intel as a
  parchment item; a knowledge check to read it; handing the item to a
  party member transfers the intel with no check on the document; how one
  player shares intel with another.
- **R4 — All the verbs on the table; consolidate later.** Early alpha.
  **Loot** and **Take** are their own seam verbs beside Search, OpenDoor,
  Unlock and Interact. Interact stays what fadedpez shipped: the NPC verb.
  If one button is wanted later, Loot and Hold become declarations under
  one verb the way Unarmed Strike sits under Attack (§4.1) — the rule
  halves never change.
- **R5 — The artifact is held** (§5 option B; the word was Take until R10):
  an explicit verb on an adjacent holdable prop; the prop leaves the
  atlas for everyone; the holder holds it.
- **R6 — The scenario sets the exit; exiting there with the artifact
  wins.** Kirk: "this scenario could have the exit set and if on exit
  with artifact then we win." The form gains a second field (§3.2); the
  ending is one trigger (§6). Answers Q3 and Q4 together: the package
  declares two bindings and one ending, the encounter runs it.
- **R7 — Explicit.** The ending fires on the Exit verb at the bound exit,
  never on arrival. Kirk: "explicit."
- **R8 — Endings come from scenarios; the boss flag is retired by the
  next step, not this one.** Kirk: "we setup when a scenario ends, not
  because one monster has a flag on it. that flag is too simple for where
  we are now" — then: "boss:true is fine for now. when we get this in, we
  turn that tomb into a scenario: kill the captain." So: this slice leaves
  `boss:` working and untouched; the **named follow-up** converts the
  reference tomb to the second scenario, **kill-the-captain**
  (`boss: entity_ref(monster)` → `TriggerMemberDown`), and deletes the
  flag then, with the content re-put as its runbook gate. Two scenarios
  bound on one dungeon are legal and visible on the form; the collision
  refusal proposed earlier is dissolved.
- **R10 — Hold, not Take.** Kirk: "the merchant is looking at a verb to
  take things off a merchant. ours will not work because it is just a
  holding fact… take would land in inventory. if ours was hold then it is
  just a fact." The run-scoped verb is **Hold**: it creates the `holds:`
  fact and nothing lands in inventory. **Take** is reserved for the act
  that lands a thing in the character's inventory — buying from a
  merchant (fadedpez's lane) or carrying the artifact back to town (a
  later slice). Two eras, two words; the record never says "took" for a
  thing that is only held. The yaml flag is `holdable:`; the beat is
  `held`, rendered present-tense ("Aldric holds the heirloom"); Drop is
  its inverse. Wire renamed in a wave-0 follow-up.
  **Settled after a round** (same hour): Hold was pulled back for a
  moment because in 5e "hold" is about hands, then kept for exactly that
  reason — Kirk: "hold means something, it is a fact; maybe it is a
  one-handed hold or two-handed; maybe someone has to hold it. hold is a
  great word." Hands are a shelf (§9), not a collision. The merchant's
  verb is **Trade** (money for item; later player for player), not Take.
- **R9 — They drop it.** Kirk: "oh i like that. they drop it." A carrier
  who leaves from anywhere but the bound exit drops the artifact where
  they stood, with a `dropped` beat to everyone present (§6).

## 2. Proposed, awaiting ruling

- **P1 — Intel lives on the placement, not on the scenario.** Slice 1 put
  concealment on the door declaration because it belongs to the door
  wherever the author puts it. The same test puts "knows the vault door"
  on the captain's `place:` entry. Then the scenario form binds only the
  artifact, and nothing in the game needs the word *captain*: the captain
  is a monster who knows a door. (The spike needed a `Captain` field only
  because its `Defeat` verb was scenario-owned; in the game the fall is a
  real event the encounter already notices.)
- **P2 — Placement ids.** `id:` on a `place:` entry, optional, unique
  within the dungeon, refused by name on collision — the third instance of
  the id regions and doors already carry. Required only by whatever binds
  to it (a knowledge link's *subject* needs none; a scenario binding does).
- **P3 — Loot is offered on every body.** The affordance must not say
  which monster carries intel. Every downed member is lootable; a body
  with nothing to give transfers nothing. Same law as search: the answer
  never leaks the question.
- **P4 — Loot is a second writer of the fact search writes.** Success is
  `learnDoor(looter, door, cause="loot")` plus the looter's own
  DOOR_REVEALED beat — the beat slice 1 already sends. Zero new reveal
  shape. The room still hides with its door; it reveals when the door is
  perceived open (slice 1, unchanged).
- **P5 — One mechanism for "who has what".** *Holding* is the noun. The
  captain holds intel; a member who picked up the artifact holds the artifact;
  a fallen holder's body holds it still, and the same loot verb takes it
  back. The parchment shelf (§9) is this noun with an item in it.

## 3. The authoring model — the file and the form

### 3.1 dungeonspec v2 additions (toolkit `dungeonspec`, carried; validated)

```yaml
place:
  - { id: captain, ref: "dnd5e:monsters:skeleton-captain", at: [23,5],
      targeting: closest, knows: [vault-door] }
  - { id: heirloom, ref: "dnd5e:props:reliquary", at: [30,3],
      blocks_movement: false, blocks_los: false, holdable: true }

exits:
  - { id: front-gate, at: [0, 1] }

scenarios:
  recover-the-artifact:
    artifact: heirloom
    exit: front-gate
```

The plain reference tomb is untouched by this slice; the follow-up binds
it to `kill-the-captain: {boss: captain}` and drops its flag.

- `id` — P2. Optional. Refused on collision, naming both lines.
- `knows` — a list of door ids (regions later, if a use case arrives).
  **It must reach a spawned monster** (rpg-api build, 2026-09-04): the
  host builds the world empty of members and every monster arrives
  through `session.Spawn`, while `MemberInput.Knows` is construction-only.
  So `encounter.JoinInput.Knows` seeds the same path setup uses, and
  `session.SpawnInput.Knows` forwards it beside the placement's other
  authored facts. In-slice: without it path 2 cannot be walked.
  Refused by name when the door does not exist. Refused on a prop
  (a prop holds nothing, for `blocks_movement`'s reason). Legal on a
  monster whether or not the door is concealed — knowing an ordinary
  door is inert, not an error.
- `exits` — where the party can leave: an id and a floor cell, the shape
  `start` already has. Structure, not scenario: a dungeon has ways out
  whatever the party is there for. `start` is not implicitly an exit —
  nothing is defaulted; the reference tomb authors its entrance as one.
  Refused off the floor (ErrNoEnding's reason: an exit nobody can reach
  is a liveness hole).
- `scenarios` — a map from scenario id to that scenario's bindings; a
  dungeon may bind several, and the run ends when any bound ending
  fires. Each key maps to bindings, **carried opaquely by dungeonspec** and validated by the
  scenario package's `New(cfg)`: dungeonspec checks only that every
  binding names a placement id that exists. Ruled 2026-09-01: "the
  dungeon spec stores `{scenario_id, bindings}` as pure references".

### 3.2 The form

Ruled 2026-09-01 and unchanged: `ListScenarios` returns, per scenario
package, a descriptor (field key, label, type, guidance = the constructor's
refusal text); rpg-api translates verbatim; the builder renders one picker
per field type; submitting validates through `New(cfg)`; descriptor and
`Config` pinned both ways by test.

One scenario ships in this slice; the second, **kill-the-captain**
(`boss: entity_ref(monster)`), is R8's named follow-up and gives the
pinning test and the picker their second instance when the tomb converts.

**recover-the-artifact** (R6):

| key | type | guidance (the refusal, verbatim) |
|---|---|---|
| `artifact` | `entity_ref(prop)` | this scenario needs an artifact — which holdable thing is the party here to recover |
| `exit` | `entity_ref(exit)` | this scenario needs a way out — which exit counts as escaping with the artifact |

### 3.3 Refusals dungeonspec owns (fail closed, name the line)

- a binding names a placement id or exit id that does not exist;
- an exit off the floor; duplicate exit id;
- `knows` names a door id that does not exist;
- `knows` on a prop;
- duplicate placement id;
- `holdable` on a monster; a holdable prop with no `id` (the binding and
  the `held` beat both need the name);
- `boss:` stays legal in this slice (R8). A dungeon binding
  recover-the-artifact whose captain also carries the flag has two
  endings, and the boss's fall would end the run before anyone loots —
  the author's business, visible on the form; the heirloom fixture simply
  authors no flag.

## 4. The verbs — Loot and Hold (R4, R10)

### 4.1 The naming rule (deliberate, not "for now")

**A verb is named by what the record will say.** "Looted the captain" and
"holds the heirloom" are statements; the beat kind, the journal fact and
the rule-half file all carry that word. Interact is not a statement — it
is a fine button and a poor fact — so it names no rule half here.

The seam already separates the verb from the offer: `Afford` returns
declarations, each carrying a verb from the closed enum the seam owns,
and one verb may produce several declarations (Unarmed Strike is a second
Attack declaration on the bonus slot; the client renders identity
verbatim and derives nothing). That is the consolidation landing if it is
ever wanted: one button, several declarations, the same rule halves.

Each verb owns its target kind, its refusal order and its beat, exactly
as Search, OpenDoor, Unlock and Interact do today. The cost of a verb is
one RPC, one rpg-api handler and one web action — all deletable pre-v1.

### 4.2 Loot

Rule half in `encounter` beside `search.go`; entry in `session` beside
`Search`; the same law: keep the secret at the rule half.

```go
type LootInput struct {
    Member MemberID   // who loots
    Target MemberID   // the body
    Range  int        // cells; zero means adjacent, as Interact's does
}
type LootOutput struct{} // the rule half says nothing; the seam adds its reports
```

Validation order mirrors search: nil → empty member → closed → not a
member → target not a member → target not down → not in range. Refusals
for "not down" and "not in range" are ordinary (the body is visible; there
is no secret in whether it is down). Effect: for every holding of the
target, transfer to the looter — **a prop moves, intel copies** (found by
the toolkit build's mutation pass, 2026-09-04: keying a holding by its
subject alone let a second monster with the same `knows` silently take
the door off the first; knowledge is not exclusive, a thing is). Intel:
`learnDoor` with cause `loot`, audience the looter alone, one
DOOR_REVEALED beat; a prop: the looter now holds it. And a `looted` beat
to everyone present, naming looter and body and nothing of what moved.

**Q1 — closed at wave 0 (2026-09-04).** The rule half returns nothing.
The seam's response carries the standard `saved` and `delivery` reports
every mutating verb carries (a half-failed save must be an error with a
report, never a shrug) and nothing about what moved. The reports go to
the caller, who is the looter and learns anyway; other members learn
only from the beats. The parchment shelf, when stocked, adds what it
needs additively.

**Q2 (open, plan-level):** the body must be reachable after the fight
dissolves — a removed initiative slot must keep its position. Check in
the plan against `noticeDown`'s Remove consequences.

### 4.3 Hold

```go
type HoldInput struct {
    Member MemberID   // who picks it up
    Target PropID     // the placement, by its id (the wire mirrors it as `target`)
    Range  int        // as Loot
}
type HoldOutput struct{} // as Loot's
```

Validation: nil → empty member → closed → not a member → no such prop →
not holdable → already held → not in range. **The probe law is the whole gate, not one pair of refusals** (toolkit
build, 2026-09-04): for a prop inside space the member cannot see, EVERY
refusal — no such prop, not holdable, already held, out of range — is the
same bytes, because "out of range" about an unseen id answers "yes, there
is something by that name in a room you have not found" as loudly as
"not holdable" would. The visibility gate is hoisted above the whole
order; the order above holds unchanged for every prop the member can
see, and a visible pillar refuses by name. Effect: the prop leaves the atlas — a `held` beat to everyone
present (physical state folds on the truth grain, ruled 2026-09-01) — and
the member holds it (§5).

### 4.4 Pricing in combat — shelf

Out of combat both verbs are free. 5e gives one free object interaction a
turn and charges an action past that. Loot and Hold join the `Afford`
enum with a slot only when an acceptance scene takes something mid-fight;
until then they are refused while the taker's fight is on the turn clock
and it is not their turn, and free on their turn. Named here so the
first mid-fight hold is a ruling, not a surprise.

## 5. The artifact — holdings (R5)

A prop today is scenery: a ref, a cell, two blocking flags. Two additions:

- `holdable: true` on the placement, refused on a monster, defaulting to
  false — a thing nobody declared holdable stays scenery. The scenario's
  `artifact` binding refuses a placement that is not holdable, in
  form-filler words.
- **Holding** — a run-scoped journal fact on the member, audience
  everyone (truth grain). Each kind carries its own prefix
  (`holds:prop:<id>`, `holds:intel:door:<id>`) because a prop id and a
  door id are both plain strings and no id needs a shape rule to keep the
  fold safe (toolkit build, 2026-09-04). A fallen holder's body holds it
  still, and Loot (§4.2) takes it back — P5's one mechanism.

Rejected: reach-its-cell-then-leave (`TriggerReachedPosition` exists, but a
member who stood there and walked away would "have" it — zero values must
tell the truth); the character sheet's inventory (the artifact is
run-scoped until the run ends; what keeping it means across runs is the
journal's business, brainstorm §10, a later slice).

**On the wire (found by the web build, ruled 2026-09-04):** `AtlasProp`
carries `holdable` — structure on the truth grain, a holdable thing looks
holdable — so the client offers Hold only where it is true and never
guesses from an id. `GetAtlasResponse` carries `exits` (id + cell), the
authored ways out, same for every member like `start`, so the map can
draw the way out. **Leave is offered everywhere and the server decides**
what a departure means (R9 needs a departure from the vault to be
possible); when the member stands on an exit the button may say so.
The session seam mirrors the encounter's atlas by copy, field for field
(`projectAtlas`), so `exits`, `holdable` and the prop `id` are CARRIED
explicitly there, not inherited — only the held-prop filter comes for
free (session build, 2026-09-04).

## 6. The ending — exit holding the artifact (R6; shape proposed)

"Both paths end in withdrawal with the artifact" — one ending, not two:
**a member exits through the bound exit while holding the artifact.**
Brainstorm §9 ruled run-ending is quest v0 and to promote the run-goal
predicate when the second goal type arrives. This is the second goal type,
and it lands in the endings model, which stays the one owner of "the run
is over".

Today the host declares two endings on every authored dungeon:
`withdrawn` (TriggerExternal, fired by the lobby when the party abandons
the run) and `boss-down` (TriggerMemberDown, when a boss is authored).
This slice keeps both and adds every ending each bound scenario
declares; the follow-up retires the flag arm when the tomb becomes
kill-the-captain. This scenario's:

```go
// TriggerExitedHolding fires when a member standing on Exit's cell
// declares Exit while holding Item. Evaluated in the one place a
// departure is noticed, after the departure beat, so the record reads
// "left through the front gate with the heirloom" and then "ended".
type TriggerExitedHolding struct {
    Exit ExitID
    Item PropID
}
```

Two shapes were weighed:

- **Explicit — the Exit verb at the exit (recommended).** `Exit` already
  exists as a member's departure and carries what they knew out. At an
  authored exit, holding the artifact, it also ends the run. Nobody's
  run ends because the carrier stepped on a cell while retreating to
  help a friend; the player says when. "You always feel like you are in
  control" (Kirk on the builder, 2026-09-04) is the same instinct.
- **Automatic — a `Holding` filter on `TriggerReachedPosition`.** The
  smaller toolkit change (one field on an existing trigger, evaluated
  where arrivals already are). Rejected for the reason above; named so
  the trade is visible.

Consequences the trigger must state:

- Exiting **without** the artifact is today's Exit: the member departs,
  the run continues for the others; when the last member leaves, the
  encounter auto-closes as it does now.
- **A departure that did not end the run drops what the member carried,
  wherever they left from** (R9, sharpened by the toolkit build: with two
  exits authored and one bound, "anywhere but the exit" was ambiguous —
  R9's reason is the hole, not the cell). The artifact reappears as a
  holdable prop on the cell the carrier stood on, with a `dropped` beat
  to everyone present. Otherwise a carrier who leaves through the lobby —
  or disconnects — takes the only win out of the run with them. Journal
  stays append-only: `dropped:<prop>@<cell>` is a new fact; the
  projection shows the prop where it was dropped.
- The artifact leaves with the exiting member; a run that ended this way
  records who carried it out (the `holds` fact and the exit beat agree).
- A dungeon with a scenario bound and no reachable exit refuses at
  `New(cfg)` in form-filler words; `ErrNoEnding` is the encounter's own
  backstop.
- **`Exited.holding` is what actually left the run with the member**
  (toolkit#1507, found by the rpg-api acceptance scene, ruled 2026-09-04):
  the carried list when the exited-holding ending fired, EMPTY when the
  departure dropped it. The drop is decided first, then the departure
  beat is appended with the truthful list, then `dropped`, then `ended`
  if it fired — §6's beat order holds and no two beats about one
  departure disagree.
- **A member who withdrew before the ending fired is not told** (session
  build, 2026-09-04): the `ended` beat's audience is everyone still in
  the run plus the carrier the same verb just removed. "For everyone" in
  §8 means everyone still in the run; a departed player who wants to
  know how it finished asks with a read (`GetStatus`), which the Leave
  flow already lands them on.
- Win path 1 and win path 2 are indistinguishable at the ending, and
  that is correct: the journal's silence about a fight is the record of
  the skipped fight (use-cases UC-4).

**Q3 and Q4 closed by R6:** the ending lives in the encounter's endings
model; the form exists because two bindings and one ending is a form,
and the package is what makes the descriptor, the refusals and the
pinning test exist. rpg-api's `endingsFor` grows one arm: the scenario's
declared ending, translated verbatim.

**Q1 (loot output)** closed at wave 0 — see §4.2.

## 7. Ownership

| noun | holder | why |
|---|---|---|
| placement id | dungeonspec (author) | third id after region and door |
| knowledge link `knows` | dungeonspec placement → encounter concealment world at load | like `concealed:` on a door: belongs to the thing, wherever placed |
| `scenario:` bindings | dungeonspec, opaque; scenario package `New(cfg)` validates | ruled 2026-09-01 |
| `Loot` rule half | encounter (`loot.go` beside `search.go`) | the secret is kept where the fact is written |
| `Loot` entry | session (beside `Search`, `Interact`) | the seam; range is the host's truth |
| the button | web | renders verbs and offers verbatim; consolidation is a client menu or an Afford declaration, never a rule |
| holdings | encounter world journal facts | run-scoped truth-grain state; the parchment shelf inherits it |
| `Hold` | encounter rule half + session entry | same shape as Loot; owns the `held` beat |
| `holdable` | dungeonspec (author) | a thing nobody declared holdable stays scenery |
| `exits` | dungeonspec (author) | structure: ways out, like `start` |
| the ending | encounter endings (`TriggerExitedHolding`), declared by the scenario package, wired by rpg-api's `endingsFor` | single owner of "the run is over" |
| form descriptor | toolkit scenario package export → rpg-api verbatim → web renders | ruled 2026-09-01 |

Charter checks run: session gains two entry verbs and no rule (holds);
encounter gains two rule halves and one fact kind (holds — it already
owns search and the knowledge facts); dungeonspec gains three carried
fields and refusals it can decide alone (holds — it never resolves
content, C1); rpg-api translates and learns no scenario word (holds).
Singularity: "who has what" answered once, by holdings (P5); "who knows
what" answered once, by the journal (unchanged).

## 8. Acceptance (draft — completes when §6's shape is ruled)

| item | proof |
|---|---|
| a party that never loots and never searches finishes the run blind | acceptance scene: kill the captain, never loot, the door stays a wall for everyone (secrecy check, not a win rule — Kirk 2026-09-02) |
| loot on the captain reveals the door to the looter alone | DOOR_REVEALED to one recipient; the other member's atlas unchanged until the door is opened in their presence |
| loot on a body with nothing gives the same bytes as loot on the captain before the reveal beat | P3 |
| hold removes the prop for everyone and the holder holds it | `held` beat to all present; `holds` fact |
| Exit at the bound exit while holding ends the run once, for everyone, naming the carrier; Exit without it departs one member and the run continues | scene: two members, one carries, the other leaves first |
| Exit at a cell that is not the exit while holding does not end the run | scene: carrier exits from the vault |
| the form refuses a missing or non-holdable artifact in form-filler words | descriptor↔Config pin |
| every beat names the verb as a statement: `looted`, `held`, `dropped` | beat kinds pinned; no beat says `interacted` or `took` |

## 9. Shelves — named, empty (Kirk 2026-09-04)

- **The parchment.** Loot yields an *item* that carries the intel instead
  of the intel itself. Needs holdings (§5 B) to exist first; then loot is
  pure item transfer and the intel arrives on reading.
- **Reading is a check.** A knowledge check (Arcana, History, or the
  author's list — multi-approach, as every check is) to get the intel out
  of the parchment. The document's check, distinct from loot's none.
- **Handing an item transfers its intel, no check on the document.** The
  receiver learns what the parchment says by being handed it. Reconcile
  with the 2026-08-30 ruling that social sharing stays out of the game
  ("friends talk over voice — the asymmetry is the fun"): sharing by
  *telling* stays out; sharing by *acting in front of others* is slice 1
  (a knower opens the door, those present see); sharing by *handing a
  thing over* is this shelf. Three different verbs, one of them ours.
- **Player-to-player intel sharing** in general — Kirk's open question.
  No verb until a use case beyond the parchment names one.
- **Hands.** Kirk 2026-09-04, verbatim rules for the shelf: "hold can
  take 0–2 hands; if it takes a hand, no two-handed weapon; if it takes
  two, no weapon." Someone has to hold it while the party fights. Seam:
  the holdable declaration grows a `hands: 0|1|2` count when this is
  built (today's `holdable: true` is `hands` unstated), and the weapon
  rule reads the `holds:` facts of the wielder. Nothing here reads hands
  now; the flag stays a bool.
- **Take.** The act that lands a held thing in the character's inventory
  — carrying the artifact back to town and asking the old man. The word
  is reserved; the run-end fold that turns `holds:` into an item is the
  later slice.
- **The scenario describes itself** (Kirk, after the walk 2026-09-04): the
  descriptor gains a `description` — "in this scenario … you can hide the
  artifact in a concealed room and set a monster that knows the loot" —
  rendered above the fields, so the form reads as a page, not a panel.
  Additive on `ScenarioDescriptor`; the package writes it beside its
  refusals. "The form feels very customized. we will want that to be data
  driven" — audit what the panel still hardcodes (title, grouping,
  ordering) and move it into the descriptor with the same pinning test.
- **Which exit wins is not on the wire** (web follow-up #927, review
  finding 2026-09-04): `GetAtlasResponse.exits` lists every authored way
  out and nothing says which one the scenario bound, so on a two-exit
  dungeon the client cannot promise that Leave here ends the run — the
  button states what is carried and promises nothing. If the party is
  meant to KNOW its way out (the mission says "escape through the front
  gate"), the descriptor's binding is the fact to put on the wire, per
  member like everything else; nothing until a walk wants it.
- **Loot beyond intel** — coin and gear are rpg-project#310's (wallet,
  shop, prices). Holdings is the seam they will use; nothing stocked.
- **Who sees a body is lootable** — sight-scoped affordances ride the
  sight-scoped-movement follow-up, not this slice.

## Boundaries held

- The room hides with its door; loot reveals the door only (slice 1).
- Search is unchanged. Loot is beside it, never inside it.
- Rooms carry no scenario roles; neither do monsters — `knows` is a fact
  about a monster, not a role.
- No composer adoption; packages are the units (2026-09-01).
- Victory conditions are unchanged by concealment.
