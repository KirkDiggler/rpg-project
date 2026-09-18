# Authoring a site — inhabitants and policy in the World Builder

**Status:** DESIGN 2026-09-18, for review. Tracking issue: rpg-project#476.
**Consumers:** the World Builder (`rpg-dnd5e-web`), the strict single-room decoder
and compiler (`rpg-toolkit`, `rulebooks/dnd5e/encounter/dungeonspec`).
**Vocabulary authority:** rpg-project#465, the creature's table (RULED).

## The one sentence

An author writes what the creatures in their site are and do — a
faction's temperament mix and its shared answer table, a creature's overrides
and its weapons — through the visual builder, instead of hand-writing `on:`
and `temper` into YAML.

## Why now

Kirk, 2026-09-18: *"authoring their weapons and mind tables … getting that
able to be authored would be our next goal."*

Blocking made a room real (#1125, merged). The behaviour side is authored
today only by editing `rpg-api/content/reference-front-room.yaml` by hand, and
that file has moved past what the builder can even read:

| In the file today | In the World Builder today |
|---|---|
| `on:` on the **faction**, so four goblins answer ONE table | no `on:` anywhere |
| `temper: { coward: 2, soldier: 1, aggressive: 1 }` on the faction | no `temper`; the web still types `mind`, which the engine replaced |
| `actions: [weapons]` on a placement | no `actions` |
| `dispositions:` deciding who is neutral to whom | no `dispositions` |

The World Builder authors one v3 room — `{version, key, play, room}` — whose
`room` carries `implicitRegionId`, `walkableHexes`, `propDeclarations`,
`arrangementDeclarations`, `partyStart` and `monsters: {id, ref, cell}`. There
is **nowhere for any of this to live**. That is the whole problem, and it is a
modelling problem before it is a UI problem.

## The world shape

```yaml
site:
  key: reference-front-room
  name: The Front Room
  factions:
    - id: goblins
      temper: { coward: 2, soldier: 1, aggressive: 1 }
      on:
        intimidated: [...]
        intimidate_failed: [...]
        persuaded: [...]
        persuade_failed: [...]
  dispositions:
    - { between: [goblins, party], stance: neutral }
    - { between: [bandits, party], stance: hostile }
  rooms:
    - ...            # the room draft exactly as it is today
```

Three layers, and each one already exists somewhere:

1. **The site** — identity, factions, dispositions, and its rooms. This
   is v2's document root (`key`, `name`, `regions`, `factions`,
   `dispositions`, `endings`), which the World Builder has no equivalent of.
2. **The room** — geometry: walkable cells, props, the party start, creature
   markers. This is what the World Builder authors today.
3. **The creature** — a marker in `room.monsters`, plus declarations attached
   by its own id.

## Decisions taken (Kirk, 2026-09-18)

### 1. The root is `site`

**`adventure` is reserved, not rejected.** In D&D an adventure sits *above* a
single place: a published module spans several sites, a campaign spans several
adventures. Calling one authored place an `adventure` would consume the word for
the level above, so a real multi-site adventure would have no name left — the
shortcut that forecloses the thing the game will need. `site` claims only what
is true: *"this is a site where things happened."*

Consequently the deferred word for a collection of sites is **`adventure`**, and
the hierarchy reads `campaign -> adventure -> site -> room`.

It covers dungeon, cave, town and camp; *"dungeon is a very specific thing …
a free roam town could be built here."*

Three candidates were rejected on **verified** collisions, not taste:

- **`dungeon`** — too narrow, and it is already the wire's word
  (`PutDungeon`, `GetDungeon`, `dungeon_key`).
- **`encounter`** — the worst of the three: `dungeonspec` **lives inside**
  `rulebooks/dnd5e/encounter`, so an authored `encounter` would be compiled by
  the `encounter` package into an `encounter` runtime field. One word, three
  meanings, with the compiler as one of them.
- **`scene`** and **`region`** — both already taken (`WorldScene` is the
  props/groups scene; v2's `regions:` are painted floor areas). **`level`** is
  unusable in a D&D codebase.

**The toolkit and wire names do not change.** Kirk: *"protos are versioned …
what it is called in the toolkit can stay and that's fine. maybe the module
move to 1.0 will change it. that is what versions are for."* So the authored
model says `site`, the wire keeps `dungeon`, and the beta pass or the 1.0
module move is the revisit point. **The divergence is deliberate and is recorded
here**, in this design. An ADR is owed by whichever repository next touches the
dialect, so the divergence is visible one hop from the code rather than only in
a design doc.

### 2. `rooms[]` is flat, and there is no `sites` layer yet

v2 is flat today, and nothing needs grouping. Nested places — a town with a
dungeon beneath it — are the business of the level ABOVE a site, not of a site.
So this is not a limitation nobody noticed: see `adventure` below for the named,
deferred word that owns them.

### 3. There is no `kind` field yet

A town is **already expressible**: declare a faction and *say* it is neutral, and
a fight bubble forms only between opposed pairs — which is why the front-room
goblin stands there instead of attacking.

**Neutrality is authored, not free.** `DefaultStance`
(`rulebooks/dnd5e/encounter/disposition.go`) returns hostile whenever either side
is `party`, so a declared faction facing the party is at war with it until it
writes `{ between: [townsfolk, party], stance: neutral }`. A town works because
it says so, not because nobody said anything.

`kind: dungeon | town | camp | cave` arrives when something **reads** it: a rule
(*no combat here at all*), the library browsing by kind, or a mode that changes.
*A use case brings the mechanism*, and an unread field can only lie.

### 4. The creature splits: the actor carries identity, the binding carries orders

```yaml
monsters:                  # identity and placement
  - { id: goblin-1, ref: dnd5e:monsters:goblin, cell: { q: 2, r: -1 }, faction: goblins }
monsterBindings:           # the orders — what this creature does
  goblin-1: { on: {...}, temper: coward, actions: [dnd5e:weapons:shortbow] }
```

`faction` is **optional, and absent when unauthored** — never written out as
`faction: monsters`. `factionOf` (`rulebooks/dnd5e/encounter/field.go`) stores it
*"as given, never resolved here, so a member in the default faction persists
byte-identically to one from before factions existed"*, and resolves the kind's
default on every read: `party` for a player, `monsters` for a monster, and none
for a world NPC, *"which is never a side"*. Absence must keep meaning what its
author meant, so an unauthored faction stays out of the file.

**The rule that decides which side of the split a field lands on:**

> **If the faction supplies it, it belongs in the orders block — the faction
> supplying it is exactly what makes it overridable. If it selects the faction,
> it belongs on the actor.**

Both override rules are the engine's own words. `FactionSpec.On`
(`rulebooks/dnd5e/encounter/dungeonspec/spec.go`) is *"LAYERED, NEAREST KEY WINS
WHOLESALE. A placement that writes its own `time` key replaces this one's
entirely"*; `TemperSpec` says *"A PLACEMENT'S OWN WORD WINS and the mix is not
dealt for it."* Both are overrides, so both belong with the overrides. Nothing
overrides `faction` — it is what *determines* the defaults. Keeping it on the
actor also keeps it out of the panel whose whole job (slice 2) is showing
inherited-versus-overridden, where it would be the one row that is not a row.

**And membership must not require a binding.** Put `faction` in the orders block
and four goblins in `goblins` with no overrides need four bindings that exist
only to record membership. The engine's comment on the answers block calls
absence the common case — *"Absent means the creature answers nothing, which is
the common case: a scared goblin does not turn the camp unless the author planted
the fact that says so"* — and a block whose ordinary state is absence cannot be
the only home for a fact that is always present.

**Creatures keep their own list and their own placement.** They are cell-snapped
(`RoomMonsterPlacement { id, ref, cell, faction? }`) while props are
free-transformed, and they carry different data. Folding them into `scene.items`
would blur a real distinction to win an administrative one.

This is the **third** declaration kind on a placed thing, and it is the pattern
already shipped twice:

```
propDeclarations[itemId]    = { blocksMovement, blocksLineOfSight, footprint }   shipped
doorBindings[itemId]        = { doorId, state, locked, passage }                 proposed, v4
monsterBindings[monsterId]  = { on?, temper?, actions? }                         this design
```

It is the **whole** authored fact block, not a subset of one — the way a prop's
declaration carries every prop fact while `scene.items` carries identity and
transform. `on` and `actions` land first. `intimidate`, `persuade` and `arrives`
are already on the engine's `PlaceSpec` (`dungeonspec/spec.go`) and have a home
here when a use case brings them.

`roomDraft.ts` already validates that a declaration's owner is live, and
`remapRoomDeclarations` exists to keep declarations attached across duplicate and
stamp. A creature binding inherits that discipline, including dropping a binding
when its creature is removed.

## Ownership and the seam

| Noun | Owner |
|---|---|
| The site document, factions, dispositions, rooms | the **authored content** — the builder writes it, the compiler reads it |
| What a creature does with time | the rulebook's tables + the encounter's `TableDriver`, *"THE ONE DRIVER: it rolls the creature's own authored table"* (`encounter/tabledriver.go`) |
| Temperament's arithmetic | rulebook content; the placement or the faction's mix names one |
| Whom a creature opposes | the stance graph, projected per observer |
| A faction the run creates, and who is in it | the **encounter** — live state, persisted on the blob. See *Allegiance* below |
| The truth about a running fight | the **encounter**. None of this design is live state |

**The authored document is design-time; the encounter is run-time.** Nothing
here lets the builder decide what a creature does — it writes the table the
engine will roll on. That boundary is why the placement compile path
(`CanonicalPlacedProps`, `single_room_placement.go`) is the model to copy: a
pure source-to-canonical conversion with no interpretation.

## Allegiance, and what changes it

Kirk, 2026-09-18: *"being able to spawn a new faction at runtime and make it
hostile to one or more current factions is what I am after."*

### What ships: the whole camp

`encounter/scenarios/holdout.go` ships a scenario for exactly the fiction that
motivated this — a camp you cannot beat, only turn. `HoldOutID = "hold-out"`,
`FieldConvince = "convince"`, and the run ends *"when that faction's stance
toward the party folds to neutral — which is exactly the disposition's own
`until` holding"*. The reference file
(`dungeonspec/testdata/reference-raider-camp.yaml`) is:

```yaml
factions:     [ { id: raiders, mind: chief } ]
dispositions: [ { between: [raiders, party], stance: hostile, until: { fact: saved-wiseman } } ]
scenarios:    { hold-out: { convince: raiders } }
```

This is the **faction-level** case, and it needs nothing new. It is the pattern
to copy, not a mechanism to invent.

### What is new: part of the camp

A **spawn**: a faction the run creates, together with the relationships it
declares. The hold-out flips a faction; a spawn splits one — say a faction
`goblins-broken`, hostile to `[goblins, party]`.

The authored side says what the *situation* is (an answer-table entry planting a
fact); the encounter owns what that creates and who is in it.

**The relationships are stated, not inferred.** `DefaultStance`
(`encounter/disposition.go`) is purely lexical and consults no registry:

```go
case a == b:                                  allied
case a == FactionParty || b == FactionParty:   hostile
default:                                       neutral
```

So a bare spawn is hostile to `party` *for free* — and that cuts both ways: a
bare split meant to leave the fight **adds** an enemy group instead of removing
one. A spawn naming whom it is hostile to is a statement; leaning on a default
that happens to produce hostility is an inference.

**And it is born hostile, not transitioning — so no invariant is weakened.**
`stanceReachable` (`encounter/disposition.go`) enforces *"the only change there
is (R2)"* — declared hostile → neutral — and refuses, as unreachable, a predicate
waiting for a neutral pair to *become* hostile (`untilNotBuilt`). A spawned
faction never transitions: the pair is hostile from the moment the faction
exists, so **no authored predicate claims an impossible transition and
`stanceReachable` stands untouched.** That is the strongest argument for this
shape over a disposition that turns neutral → hostile, which would have had to
relax a stated rule.

**The fold needs nothing.** The seed (`encounter/world.go`) writes one directed
edge per pair for the declared-or-default stance, and *neutral is the absence of
an edge*. A spawned pair's hostile edge therefore reads hostile with zero wiring:
`stanceBetween` falls back to a sentinel observer when a pair has no minds —
*"A pair no mind can turn folds as nobody, which is the declaration alone"* — and
the `graph.Raise`/`graph.Settle` projections exist **only to remove** edges when
a `until` fact lands (`To: ""`, *"which is what neutral means (R2)"*). A spawned
hostility has nothing removing it. **The whole cost of the spawn's hostility is
the edge itself in the seed.**

**The group defines itself.** `answerInput.witnesses` (`encounter/answer.go`) is
documented as *"who saw it — who learns a fact, and the beat's audience"*, and it
becomes the appended fact's `Audience` (`encounter/flip.go`, whose own doc says
*"the audience is whose fold carries it"*). So "the merchant and his friend" is
"those who saw it" — the group needs no authored membership list, because the
engine already keeps the thing that defines it.

### The seam underneath, which decides whether any of it bites

In `classify` (`encounter/trigger.go`) the pair enumeration is `KindPlayer ×
KindMonster`, and **only the opposition test is by faction**:

```go
players, monsters := e.sidesInContactOrder(contact)   // member.Kind
for _, player := range players {
  for _, monster := range monsters {
    if !e.opposed(player, monster) { continue }        // factions
```

The engine's own comment: *"Under the default dispositions that is every player
and every monster, which is the whole table this loop ran on before factions
existed."* For this design, exactly:

- A defector moved into `party` **stops fighting us** — `party` against itself is
  allied, `opposed` fails, the pair is skipped. That half works.
- Spawned hostile to **`party`** — **works**, because a player is in the pair.
- Spawned hostile to **another monster faction** — the edge exists, `opposed()`
  returns true, and **nothing ever asks**, because two monsters are never a pair.
  A split that turns on its former camp is declared and inert.

So *"hostile to one or more current factions"* bites today only when one of them
is the party. **One question — who fights whom — currently has two mechanisms**,
and the kind buckets are the pre-faction world the graph never replaced. Pairing
from the factions is finishing an existing mechanism, not adding one — and
without it, the rest of this section buys nothing.

### Four costs, measured

1. **The faction list is closed at construction.** `field.factionIDs()` is *"every
   faction this field has, reserved first, then declared in authored order — the
   entity list the graph is seeded from"*, and `isFaction`/`validateMemberFaction`
   refuse any other id. A spawned faction must be enumerable and valid; its
   *hostility* needs nothing, its *existence* does.
2. **A member's faction becomes writable.** Every write is at `Join` or
   construction today (`encounter.go`, `Faction: in.Faction`). `factionOf` already
   reads it live, so this is a write, not a new read path.
3. **A mind — and a spawned faction has none, which is part of why it is cheap.**
   `mindOf` (`encounter/world.go`) reads the **declared** faction only: *"the
   DECLARED mind while it is one of them, else nobody"*, set for a faction of one
   by the authoring compiler, *"never inferred here from whoever happens to be
   standing in the faction"*. A spawned faction is not in `factionIndex`, so it
   has no mind — which is precisely why its seeded hostility reads cleanly
   through the sentinel path. The day a spawned faction must *learn*, to be
   turned, it needs a mind, and that is work this design has not yet paid for.
4. **Persistence is the real one.** `EncounterData` already carries a member's
   `Faction` (`encounter/data.go`), so a moved member round-trips; the **spawned
   faction and its stance list** must join it, with **stable ids** — a reloaded
   fight has to remember the merchant's grudge, and ids from a counter that
   resets will not do.

## The vocabulary it must carry, sealed

Everything below is rpg-project#465's ruled vocabulary. This design **transports**
it; it does not extend it.

- **Faction `temper`** — a weight profile and nothing else, dealt from a mix at
  spawn with the faction as the die's entity. Sealed words, in the engine's own
  order (`encounter/table.go`, `TemperWords`): `soldier`, `coward`,
  `aggressive` — absent with no mix means a soldier, *"every factor 100"*
  (`dungeonspec/spec.go`). The order is the declaration's, so it is the picker's
  too.
- **Faction `on:`** — the shared answer table, inherited by every placement in
  the faction.
- **Placement `on:`** — the nearest layer wins **wholesale**; a placement that
  writes one key replaces that key and inherits the rest.
- **Entry words** — `fact` and `flee`, which are **social-only**; `hold`,
  `attack`, `toward` and `away`, which are **`time`-only**; plus `weight` and the
  author's `say`. A word offered on a trigger that cannot take it is refused in
  the engine's own sentence.
- **`when` conditions and the `time` trigger** — **shipped**, not pending.
  `encounter` **v0.90.0** (`f672c888`, rpg-toolkit#1816) took the whole grammar:
  the fifth trigger `time`, `when` as one exclusive enemy band
  (`reach | seen | remembered | none`) or one deed with a span
  (`attacked | intimidated | persuaded | fled: { within: N }`), the selectors
  (`enemy | attacker | actor | { at: [col, row] }`), and `temper`. `TableKeys`
  (`encounter/table.go`) is the social four plus `AnswerTime`. The builder offers
  a word only once the engine accepts it — and the engine accepts these.
- **`actions`** — **the authored field already ships, on the v2 placement.**
  `PlaceSpec.Actions` (`dungeonspec/spec.go`) is *"what this monster can do, in
  the author's own order"*: full `dnd5e:weapons:<id>` refs, monsters only,
  *"CARRIED, NOT INTERPRETED"*, and *"THE ORDER IS THE POINT"* — both drivers take
  the first action whose target is in reach, which is how two goblins in one room
  tell different stories without a line of Go. It is validated (`validate.go`) and
  compiled for the host (`compile.go`: *"for a host to hand to the session's
  `SpawnInput.Actions` when it spawns the sheet"*). What does **not** exist is the
  single-room equivalent: `RoomMonsterSource` is `{ id, ref, cell }`, and the
  single-room compile leaves `Actions` empty — its own test asserts
  `require.Empty(t, compiled.Monsters[0].Actions)`. **That** is the gap this
  design fills, against a shape that already exists rather than a shape to invent.

The vocabulary grows **one word per use case**, and the builder's declaration
of it is the single source for what the parser accepts, what the panel offers,
and what a refusal suggests — the mechanism #1119 already established.

## UI surfaces

Kirk, 2026-09-18: *"our ui is rough and unorganized … building a room out does
not need quick access to loading and saving; it is building."* And: *"props:
completely other screen … configuring a monster table is its own thing."*

**The principle: separate by task, not by document.** Three concrete
violations to fix, all of them observed:

- The room header carries five persistence/navigation controls (*saved locally ·
  Save room draft · Reload room draft · Save room snapshot to world · Expand
  workspace · New room*) while the author is laying out a room.
- The right panel stacks four different tasks in one scroll — Edit (selection),
  Room setup (actors), Publish & Play (an action), and three libraries.
- Composition affordances leak into room mode: *Placement anchor · X0/Z0* and
  *Show composition bounds* appear while building a room, where they mean
  nothing.

So:

- **Rooms** — building. Tools, palette, canvas, the selected thing's
  declarations. No save/load in the chrome.
- **Prop compositions** — its own screen, as it already is. The orange anchor
  belongs here and nowhere else.
- **The site** — the scope that belongs to **no single selection**:
  identity, factions, temperament mixes, dispositions. It is a **document
  section**, not a property panel, and it is deliberately different in shape
  from everything else because its nouns are inherited rather than selected.
- **Library** — saving, loading, snapshots, arrangements. A place you *go*.

Factions are the reason the site scope cannot be avoided: a shared table is
inherited by many creatures and belongs to none of them, so a
select-then-declare panel can never show it.

## Slices

Each ends in something observable, and slice 1 is the one that makes room.

1. **Shape only.** The document carries the `site` root (`factions`,
   `dispositions`) and the creature split — `faction` on the actor,
   `monsterBindings` for its orders; it round-trips through the web encode and
   the toolkit's strict decode; **no UI**. Proof: a hand-written binding saves,
   reloads and plays unchanged, and a site with none of the new keys is
   byte-identical.
2. **Read-only.** The panel shows what a creature **inherits** (its faction's
   table and temperament) versus what it **overrides**. Inheritance is what
   will confuse authors, so making it legible precedes making it editable.
3. **Edit the table.** Per trigger, weighted entries with `say` and one word,
   driven by the vocabulary declaration from #1119 rather than a second one,
   and now current with the shipped grammar (rpg-dnd5e-web#1137) rather than
   behind it.
4. **`temper` mixes.** The faction's dealt mix, sealed words only.
5. **Weapons.** `actions`, which already has a shipped shape to copy
   (`PlaceSpec.Actions`) and needs the single-room equivalent.

The **allegiance** work above is deliberately not one of these five. It is a
second wave with its own toolkit home — the spawn, the writable membership, and
the kind-versus-faction pairing — and none of the five depends on it: they author
the document, and allegiance changes what the run does with it.

## Done when

- A creature's `on:` and `actions` can be authored, saved, reopened and played,
  and the engine rolls the table the author wrote.
- A faction's answer table is inherited by every placement in it, and a
  placement that overrides one key replaces that key wholesale.
- A site with none of the new keys — no factions, no dispositions, no `faction`
  on a creature, no bindings — is byte-identical to today's.
- The builder never decides which entry fires, never sums weights, and never
  reads a `when` — it writes a document.
- A declaration can never outlive the creature it names.
- Every claim above is a test; a walk proves wiring.

## Deliberately absent, with the use case that would earn each

- **`kind: dungeon | town | camp | cave`** — when a rule, the library, or a mode
  reads it.
- **`sites`** — when a site spans more than one place.
- **List-valued entries** (two things happening from one entry) — when a use
  case needs it; today one entry does one thing, which is what keeps an
  author from guessing an ordering the engine does not promise.
- **A semantic edit of a shared table from inside one creature's panel** — the
  faction owns it; editing it from a creature would make one creature's screen
  write another's policy.
- **Non-monster actors** (a merchant, a hostage, a townsperson) — when a scene
  needs someone who is not a combatant. There is no authoring surface for one
  today: `MONSTER_REF_RE`
  (`rpg-dnd5e-web/src/concepts/world-building/roomDraft.ts`) is
  `/^[-a-z0-9]+:monsters:[-a-z0-9]+$/`, so a placement's `ref` must carry the
  `monsters` type segment. The hostage and the pickpocketed merchant both wait
  on this.

## Residual risk, named

- **The vocabulary shipped; our copy of it is hand-made, and that is the risk
  that moved rather than went away.** `time`, `when`, the movement words and
  `temper` are accepted by the engine — `encounter` **v0.90.0** (`f672c888`) —
  and the builder's declaration now carries them (rpg-dnd5e-web#1137). But the
  declaration's trigger/word applicability and the engine's refusal sentences are
  two transcriptions with nothing mechanical between them, and the review of
  rpg-dnd5e-web#1141 found the test meant to check them derived *both* sides from
  the same field, so it could not fail. Pinning the engine's grammar facts as a
  generated snapshot with a sha (rpg-dnd5e-web#1143) is the mitigation; until
  then the editor's copy is what moves when the engine's does not.
- **A split that turns on its former camp is declared and inert** until the pair
  enumeration comes from factions rather than kinds (see *Allegiance*). Spawning
  is worth building for the party-facing case either way, but the
  camp-turning fiction — *"some to join our faction or at least split from the
  main faction"* — does not work until that seam is closed.
- **`site` versus the wire's `dungeon`** is a deliberate divergence, and
  divergences rot. **No ADR exists yet** — this design is currently the only
  record. Whichever repository next touches the dialect owes one, and together
  with the beta/1.0 revisit point that is the mitigation; if neither happens
  this becomes two words for one thing.
- **The site scope is the first noun in this UI that is not a selection.**
  Everything else the panel does is keyed to a selected thing. Getting this one
  surface wrong is what would push the whole UI back into one flat stack.