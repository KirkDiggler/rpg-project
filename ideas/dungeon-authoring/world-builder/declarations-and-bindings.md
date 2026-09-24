# Declarations and bindings — a permanent thing and a reference to a shared one

**Status:** DESIGN, written 2026-09-24, from a conversation between Kirk and a
session that had spent the day getting this wrong in several directions.

**What this doc is for.** It states one rule that decides which block a field
belongs in, and renders the shape that follows. It is written to be *checkable*
— every claim in §1 was re-run before it was written down, and §6 names what is
still open rather than pretending the design is finished.

**It is not a migration plan, and it is not a ruling.** Nothing here has been
implemented. Two fields named below (`tables:` at the root, `table:` on a
binding) are in an open PR; everything else is a proposal.

## 0. Where this came from, honestly

Kirk, reading the current shape:

> "so faction in your example looks like it belongs on bindings. what is the
> difference with monsters and monsterBindings. seems like we are mixing
> concerns. seems to me like we need 1 thing for monsters id, ref, actions,
> temper, faction table and then where they are starting from."

The first read of that was "merge the two blocks." **That was wrong**, and the
measurement in §1 is why. But the *complaint* was right, and the rule it led to
(§2) is better than the one the code documents today.

Two corrections are recorded here rather than quietly fixed, because the
reasoning is the useful part:

- **The session's first objection to a binding-per-monster was wrong.** It
  argued that a factioned monster with no binding would need a block "that says
  nothing." A block naming a faction says exactly one thing, and saying it *is*
  what a binding is. The objection treated short as empty.
- **Kirk's framing is the one that survived**: a declaration is permanent, a
  binding is a reference that can change. That is a stronger rule than either
  "required vs optional" or "identity vs orders", and §2 is that rule.

## 1. The measured state (re-verified 2026-09-24)

Every line below was run against `rpg-toolkit` `main` at `e00b2977`, plus the
open `feat/1897-root-tables`.

| claim | evidence |
|---|---|
| A monster's position is `monsters[].cell`, axial `{q, r}` | `single_room.go:233` |
| `cell` is read ONCE, at compile, into `MonsterPlacement.At` | `single_room_compile.go:139` |
| **`cell` is never written back from a live position** | nothing assigns `.Cell` outside construction |
| A faction declares `id`, `mind`, `on` (a table), `temper` — and **no `actions`** | `spec.go:364-389` |
| **A faction does NOT supply arms.** `inherited` carries only `on` and `temper` | `compile.go` — `type inherited struct` |
| A binding carries `on`, `temper`, `actions`, `holds`, `intimidate`, `persuade`, `arrives` | `single_room.go:256+` |
| **A faction's table and a monster's table are ONE mechanism** | `compile.go:1131` — `encounter.Layer(tableOf(from.on[side]), tableOf(c.On))` |
| A factioned monster with NO binding is real and load-bearing | `rpg-api/content/reference-front-room.yaml` — four goblins, one faction, three with no orders |
| Prop `facing` is carried and **never read by the engine** | `field.go` — "THIS MODULE NEVER DOES, and never turns it into an angle… a presentational fact" |
| Root `tables:` + binding `table:` exist and work | PR `rpg-toolkit#1898` (open), `single_room.go:152` and `:321` on that branch |

**The load-bearing measurement is the front room.** Its own comment says it:

> "THE OTHER THREE, AND THEY ARE DELIBERATELY PLAIN. Same ref, same faction, no
> `on:`, no `temper:`, no priced check… THEY ARE STILL THE CONTROL THE WALK
> NEEDS… Four identical goblins, one inherited table, one faction mix."

Three goblins that must inherit a faction's table while writing nothing of their
own. **Any shape that forces them to carry a block to get it is wrong**, and
that is the test §2 and §3 are held to.

## 2. The rule

> **A declaration is permanent. A binding is a reference to something shared,
> and can change.**

That is the whole test. Apply it to a field and the block follows:

- Could this change during a run? → **binding.**
- Is it a reference to a named, shared declaration? → **binding.**
- Is it what the thing *is*, or where it *started*? → **declaration.**

**Why this rule and not the two the code states today.** The current comments
argue "THE ACTOR CARRIES IDENTITY; THE BINDING CARRIES ORDERS" (`single_room.go:224`).
That reads well until `faction` arrives, because a faction is a *pointer to
defaults* — an orders concern wearing identity clothes — and the doc half-admits
it: "everything a faction SUPPLIES… is overridable and lives in
`RoomMonsterBinding`." So the stated rule does not classify its own hardest
case.

Permanence classifies it without strain, and it says something the other rule
cannot: **if faction is a binding, it can be re-bound.** We have no use case for
a defection today, and that is not the point. A declaration *forcloses* it
silently, which is the failure this project keeps naming. The word "binding"
tells the truth about what the fact is.

The project already has the machinery on the other side of this: `dispositions`
carry an `until:` predicate, so a side's stance already changes mid-run. A
monster whose *stance* can change while its *faction* is declared is a shape
where those two facts are allowed to disagree.

## 3. The shape

Four blocks, two kinds.

### Declarations — things that are, and where they started

```yaml
factions:
  - id: watch
    mind: guard-1           # what the side knows through — one of its members
    temper: soldier         # a word, or a mix — dealt per member
    table: watch-drill      # PROPOSED. Today this is written inline under `on:`

tables:
  watch-drill:              # a table written once, named by many
    time:
      - { when: { enemy: reach }, attack: enemy }
      - { when: { enemy: none }, hold: {} }

room:
  room:
    monsterDeclarations:
      - id: guard-1
        ref: dnd5e:monsters:thug
        startingCell: { location: { q: 2, r: 0 }, facing: ne }
      - id: guard-2
        ref: dnd5e:monsters:thug
        startingCell: { location: { q: 1, r: 1 }, facing: ne }
```

**`factions[].table` IS A PROPOSAL, NOT TODAY'S SHAPE.** A faction carries its
table inline under `on:` right now (`spec.go:389`). Whether the by-name spelling
is added, replaces the inline one, or waits, is open — see §6 item 2. It is
written here because a faction's table and a monster's table are one mechanism
(§1), so naming both the same way is the shape the rule points at.

### Bindings — what references a shared thing, and can change

```yaml
    monsterBindings:
      guard-1:
        faction: watch          # a reference to a declared side
        table: watch-drill      # a reference to a declared table
      guard-2:
        faction: watch          # one line, and it is not nothing
```

### The two names that changed, and why

**`monsters` → `monsterDeclarations`.** Every other placed noun in this document
uses the pattern `xDeclarations` + `xBindings` (`propDeclarations` /
`propBindings`, `arrangementDeclarations`). `monsters` was the one bare list,
and beside `monsterBindings` it read as two names for one thing. Kirk:

> "I like monsterDeclarations especially if we have prop declarations."

**`cell` → `startingCell`, holding `{ location, facing }`.** Two reasons, and the
second is the one that matters:

1. `cell` does not say *when*. §1 measured that it is read once at compile and
   never written back, so it is a starting position — but the name invites the
   reading that it tracks movement. `startingCell` says it.
2. **A hex with a location has a facing.** Kirk: *"if a hex has a location it
   should be easy to see it has a facing."* Facing is not a new concept bolted
   onto a cell; it is the other half of what an oriented position is. And it is
   cheap: §1 measured that the engine carries `facing` and never reads it — it
   is presentation, deciding which way the model is turned when the door opens.
   No rules work, no geometry, no AI.

`facing` takes the eight true-compass names (`n|ne|e|se|s|sw|w|nw`), which are
world-space and therefore valid under both hex orientations. The vocabulary
already exists on props and in the builder.

**A NOTE ON WHAT THIS DOC DOES NOT DECIDE.** Whether `startingCell` is spelled as
a nested `{location, facing}` or as two sibling fields (`startingCell` +
`facing`) is not settled here. The argument for nesting is Kirk's — a cell that
has a location has a facing, so they are one noun. The argument against is that
`facing` is optional and `location` is required, and nesting an optional field
inside a required one is exactly the shape §4 is about. §6 carries this.

## 4. What a declaration requires, and the one exception

Kirk: *"declaration fields are required, binding add info on top."*

| block | required |
|---|---|
| `monsterDeclarations` | `id`, `ref`, `startingCell.location` |
| `monsterBindings` | nothing — every field is optional, including the block |

That is clean, and it is clean *because* `faction` moved to the binding. Under
the old shape `faction` sat in the required block while being optional to write,
which is what made the rule hard to state.

**The exception worth naming:** `facing` is optional inside `startingCell`. A
creature with no authored facing spawns at its asset's default, which is
truthful — there is no "no facing" for a model, only "the one it came with." So
this is a default, not an absence, and §1 confirms the engine treats it that way
(`""` means the asset's own facing).

## 5. What this buys, and what it costs

### The same table, four ways — all measured to work

```yaml
# 1. one monster, its own table
monsterBindings: { guard-1: { table: watch-drill } }

# 2. four monsters, one table, written once
monsterBindings:
  guard-1: { table: watch-drill }
  guard-2: { table: watch-drill }

# 3. a faction's table, inherited by everyone in it (the front room)
factions: [{ id: watch, table: watch-drill }]
monsterBindings: { guard-1: { faction: watch } }

# 4. the faction's table, with one monster overriding it
monsterBindings:
  guard-1:
    faction: watch
    table: guard-1-own-drill     # laid OVER the faction's, key by key
```

**Forms 1–2 are what PR #1898 builds. Form 3 is today's behaviour. Form 4 is
the stack the engine already implements** — `encounter.Layer` under the
creature's own `on:`. Kirk: *"in the engine a faction can have a table, the
monster binding could have an override table assigned to it. these are the same
thing."* They are: one `Layer` call, two levels.

**Cost: the front room grows three one-line blocks.** Four goblins, all
`faction: goblins`, three with no orders:

```yaml
monsterBindings:
  front-goblin:   { faction: goblins, intimidate: [...], persuade: [...] }
  front-goblin-2: { faction: goblins }
  front-goblin-3: { faction: goblins }
  front-goblin-4: { faction: goblins }
```

That is the real price, and it is the price of the rule. It was argued against
and lost: a block naming a faction is a binding doing its one job. The
alternative — `faction` on the declaration — buys those three lines back and
costs the ability to ever re-bind a side.

## 6. What is open

Named rather than answered, so the next reader does not mistake this doc for a
ruling.

1. **`startingCell` nesting.** One noun (`{location, facing}`) or two sibling
   fields? §3 carries both arguments.
2. **Can a faction reference a table by name?** §3 writes `factions[].table`.
   Today a faction carries its table INLINE under `on:`. Whether the two
   spellings both survive, or the inline one dies, is not decided here.
3. **`mind` — and it is the interesting one.** VERIFIED: `FactionSpec.Mind` is a
   reference to a **monster placement** ("MUST name a MONSTER placement in this
   faction"), and the faction "knows what its mind knows". So a *declaration*
   names another *declaration* — the one case in this shape where the reference
   does not point at a shared, permanent noun but at a creature. That is not
   obviously wrong (a side's memory living in one of its members is a real
   thing), but it does not fit §2's rule cleanly, and §3 does not place it.
4. **`actions` — VERIFIED, and the answer is no.** `inherited` carries exactly
   `on` and `temper` (`compile.go`), and `FactionSpec` has **no** `actions`
   field. So a faction does **not** supply arms: `ordersOf` reads `c.Actions`
   from the creature alone. "The faction's weapons, overridden per monster" is
   **not implemented**, and whether a side should own its own armoury is a real
   question this doc leaves open. It also means §3's `factions[].table` spelling
   is the *only* thing a faction currently shares by name.
5. **The `intimidate`/`persuade` removal.** Kirk has ruled they come off the
   builder. They are live fields on `RoomMonsterBinding` in v4 (**correcting an
   earlier claim that they were v2-only** — they are declared at
   `single_room.go:296+` and validated in `single_room_site.go`). Removing them
   from the builder stops it authoring something the engine still reads.
6. **Whether any of this is implemented at all.** This is a design doc. The
   renames and the `faction` move are proposals.

## 7. The rule of thumb, for the next field

When a field's block is not obvious, ask the two questions in §2 in this order:

1. **Could it change mid-run?** If yes, it is a binding — whatever else it is.
2. **Is it a reference to a named declaration?** If yes, it is a binding.
3. Otherwise it is a declaration, and it is required.

And the test for whether the whole shape is right: **write the front room in it.**
Four goblins, one table, one faction, three of them plain. If that document is
still readable, the shape is holding. If the control case got noisier, the rule
is being paid for by the thing it was meant to serve.

— rpg-project agent, on behalf of KirkDiggler