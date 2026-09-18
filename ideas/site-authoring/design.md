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

A town is **already expressible**: its factions are neutral, and a fight bubble
forms only between opposed pairs — which is why the front-room goblin stands
there instead of attacking. `kind: dungeon | town | camp | cave` arrives when
something **reads** it: a rule (*no combat here at all*), the library browsing
by kind, or a mode that changes. *A use case brings the mechanism*, and an
unread field can only lie.

### 4. Creature declarations key by creature id

```ts
monsterBindings[monsterId]   // { on?, actions? } — and the faction is the fallback
```

**Creatures keep their own list and their own placement.** They are
cell-snapped (`RoomMonsterPlacement { id, ref, cell }`) while props are
free-transformed, and they carry different data. Folding them into
`scene.items` would blur a real distinction to win an administrative one.

This is the **third** declaration kind on a placed thing, and it is the pattern
already shipped twice:

```
propDeclarations[itemId]    = { blocksMovement, blocksLineOfSight, footprint }   shipped
doorBindings[itemId]        = { doorId, state, locked, passage }                 proposed, v4
monsterBindings[monsterId]  = { on?, actions? }                                  this design
```

`roomDraft.ts` already validates that a declaration's owner is live, and
`remapRoomDeclarations` exists to keep declarations attached across duplicate
and stamp. A creature binding inherits that discipline, including dropping a
binding when its creature is removed.

## Ownership and the seam

| Noun | Owner |
|---|---|
| The site document, factions, dispositions, rooms | the **authored content** — the builder writes it, the compiler reads it |
| What a creature does with time | the rulebook's tables + the encounter's `TurnDriver` (rpg-project#465) |
| Temperament's arithmetic | rulebook content; the placement or the faction's mix names one |
| Whom a creature opposes | the stance graph, projected per observer |
| The truth about a running fight | the **encounter**. None of this design is live state |

**The authored document is design-time; the encounter is run-time.** Nothing
here lets the builder decide what a creature does — it writes the table the
engine will roll on. That boundary is why the placement compile path
(`CanonicalPlacedProps`, `single_room_placement.go`) is the model to copy: a
pure source-to-canonical conversion with no interpretation.

## The vocabulary it must carry, sealed

Everything below is rpg-project#465's ruled vocabulary. This design **transports**
it; it does not extend it.

- **Faction `temper`** — a weight profile and nothing else, dealt from a mix at
  spawn with the faction as the die's entity. Sealed words: `coward`,
  `soldier` (and absent = soldier), `aggressive`.
- **Faction `on:`** — the shared answer table, inherited by every placement in
  the faction.
- **Placement `on:`** — the nearest layer wins **wholesale**; a placement that
  writes one key replaces that key and inherits the rest.
- **Entry words** — `fact`, `flee` (and rpg-project#465's ruled `hold`,
  `attack`, `toward`, `away` when the engine takes them), plus `weight` and the
  author's `say`.
- **`when` conditions and the `time` trigger** — ruled, and arriving with the
  toolkit wave. The builder offers a word only once the engine accepts it.
- **`actions`** — the weapons a placement carries (rpg-project#448 owns the shape).

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

1. **Shape only.** The document carries the `site` root and
   `monsterBindings`; it round-trips through the web encode and the toolkit's
   strict decode; **no UI**. Proof: a hand-written binding saves, reloads and
   plays unchanged, and a site without one is byte-identical.
2. **Read-only.** The panel shows what a creature **inherits** (its faction's
   table and temperament) versus what it **overrides**. Inheritance is what
   will confuse authors, so making it legible precedes making it editable.
3. **Edit the table.** Per trigger, weighted entries with `say` and one word,
   driven by the vocabulary declaration from #1119 rather than a second one.
4. **`temper` mixes.** The faction's dealt mix, sealed words only.
5. **Weapons.** `actions`, against rpg-project#448's shape.

## Done when

- A creature's `on:` and `actions` can be authored, saved, reopened and played,
  and the engine rolls the table the author wrote.
- A faction's answer table is inherited by every placement in it, and a
  placement that overrides one key replaces that key wholesale.
- A site with no bindings is byte-identical to today's.
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

## Residual risk, named

- **The vocabulary is rpg-project#465's, still arriving.** `time`, `when` and
  the movement words are ruled and not yet accepted by the engine. Building the
  editor against the declaration means each arrives by adding a row — but if
  the engine's shape differs from the ruling, the editor, not the engine, is
  what moves.
- **`site` versus the wire's `dungeon`** is a deliberate divergence, and
  divergences rot. **No ADR exists yet** — this design is currently the only
  record. Whichever repository next touches the dialect owes one, and together
  with the beta/1.0 revisit point that is the mitigation; if neither happens
  this becomes two words for one thing.
- **The site scope is the first noun in this UI that is not a selection.**
  Everything else the panel does is keyed to a selected thing. Getting this one
  surface wrong is what would push the whole UI back into one flat stack.