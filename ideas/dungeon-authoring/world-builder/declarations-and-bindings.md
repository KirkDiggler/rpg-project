# Declarations and bindings — a permanent thing and a reference to a shared one

**Status:** DESIGN, written 2026-09-24, from a conversation between Kirk and a
session that had spent the day getting this wrong in several directions.

**What this doc is for.** It states one rule that decides which block a field
belongs in, and renders the shape that follows. It is written to be *checkable*
— every claim in §1 was re-run before it was written down, and §6 names what is
still open rather than pretending the design is finished.

**It is not a migration plan, and it is not a ruling on its own.** Two fields
named below (`tables:` at the root, `table:` on a binding) are in an open PR.
**§6 records Kirk's decisions of 2026-09-24** on five of the six questions this
doc raised; where a question is settled it says so, and the residue is named
per item rather than left to be re-litigated.

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
| **`mind` names a MONSTER in its OWN faction** — a prop, a stranger or a missing id are each refused by name | `factions.go` — `minds()`, three refusals |
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
    mind: guard-1           # the side KNOWS WHAT THIS MEMBER KNOWS — see §6.3,
                            # which is where this field collides with §2: it is
                            # permanent here but names a changeable membership
    temper: soldier         # a word, or a mix — dealt per member
    table: watch-drill      # DECIDED (§6.2). Today inline under `on:`; the
                            # by-name spelling is added.

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

**SETTLED (§6.1): the NESTED shape.** `location` is required, `facing` is
optional, and they are one noun. Kirk: *"I liked your starting cell shape."*

The argument that decided it is his own — **a hex with a location has an
orientation**, so they are one thing and the nesting says so. The argument on the
other side is worth keeping in view even though it lost: `facing` is optional
inside a required noun, which is the exception §4 names rather than hides.

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

## 6. The decisions (2026-09-24), and what remains open

Kirk ruled on five of the six. **The decisions are decisions; the residue is
named per item** so a settled question is not re-opened and an unsettled one is
not mistaken for settled.

### 6.1 `startingCell` — SETTLED: the nested shape

```yaml
startingCell: { location: { q: 2, r: 0 }, facing: ne }
```

Kirk: *"I liked your starting cell shape."* So `location` is required, `facing`
is optional, and they are **one noun** — a hex with a location has an
orientation, and the nesting says so.

### 6.2 `factions[].table` — SETTLED: added

Kirk: *"i think we add table to faction."* A faction may name a root table. A
faction's table and a monster's table are one mechanism (§1), so naming both the
same way is the shape the rule points at.

**Residue:** whether a faction keeps its INLINE `on:` spelling beside the new
by-name one. Not answered, and not urgent — the inline spelling is what every
existing document uses.

### 6.3 `mind` — SETTLED: it stays, and it learns from another member

Kirk: *"ok with leaving mind a faction that can have the mind updated can learn
from another member of the faction… for now getting what a member knows is
enough. we will want to update that field to another member possibly but can
leave it for now."*

**This is a capability, not a rename.** Today the mind is validated at `Join`
and a member that is a mind **cannot join another faction** (`validateMemberFaction`).
So the field reads as **fixed once set**. Kirk's answer states the want:
**a faction's mind may be UPDATED to another member**, and the faction then
learns from that one.

**That settles §2's classification of the field, and against its current home.**
If the mind can change, it is a binding-shaped fact by §2's own rule — it is a
reference, and it is mutable. It does **not** need to move today, but the reason
is *"we are not building it yet"*, **not** *"it is permanent."* §8 records why
moving it now would be churn.

**And it strengthens §8's finding rather than weakening it.** "The mind can be
updated" is a step toward a faction that manages its own knowledge. Faction-level
perception is the further step, and Kirk deferred it: *"will be tricky keeping it
coherent and feeding it intel so when we have a use case we can circle back."*

### 6.4 `actions` — SETTLED: monster overrides, imperfect, to be improved

Kirk: *"actions are monster overrides and will need improving. for now they allow
us to add ranged or melee to simple monsters."*

So `actions` stays on the **binding** — it is an override, which is exactly what
a binding is for. **The measured gap is real and now stated as known debt:** a
faction does **not** supply arms (`inherited` carries only `on` and `temper`;
`FactionSpec` has no `actions` field), so today the list is a way to give a plain
monster a melee or ranged option, not a real equipment layer. Improving it is
future work, not this doc's.

### 6.5 `intimidate`/`persuade` — SETTLED: remove, and defer

Kirk: *"remove intimidate and persuade. they will come back when we have the use
case but monsters that are hostile cannot have them and they should come from
interacting with an npc so remove and deferred."*

**Remove them from the builder, and record the reason rather than the fact.** The
reason is the useful part: these come from **interacting with an NPC**, and a
**hostile** monster cannot have them — so their presence on every monster binding
was wrong, not merely unused.

They are live v4 fields on `RoomMonsterBinding` (**correcting an earlier claim
that they were v2-only** — declared at `single_room.go:296+`, validated in
`single_room_site.go`). Removing them from the builder stops it authoring
something the engine still reads; the engine side is deferred with the use case.

### 6.6 The builder YAML — SETTLED: clean it up

Kirk: *"I am ready to cleanup our world builder yaml."* That is the work, and it
is where the other five decisions land.

**What remains genuinely open**, and nowhere else in this doc:

- a faction's inline `on:` versus the new `table:` name (§6.2 residue)
- whether `mind` moves to a binding when the update capability is built (§6.3 —
  §2 says it is binding-shaped; today it stays put)
- whether any of §3's renames (`monsters` → `monsterDeclarations`) are
  implemented, and when

## 7. The rule of thumb, for the next field

When a field's block is not obvious, ask the two questions in §2 in this order:

1. **Could it change mid-run?** If yes, it is a binding — whatever else it is.
2. **Is it a reference to a named declaration?** If yes, it is a binding.
3. Otherwise it is a declaration, and it is required.

And the test for whether the whole shape is right: **write the front room in it.**
Four goblins, one table, one faction, three of them plain. If that document is
still readable, the shape is holding. If the control case got noisier, the rule
is being paid for by the thing it was meant to serve.

## 8. `faction.mind` — the purpose, and why it does not move

Kirk, working it out:

> "so then it would be like the leader? are we thinking the intel guard-1 has
> flows through the faction? … mind is not a great name then especially that we
> have a mind package. before we go moving anything around let's find the
> purpose of it. I suspect we just need to be a little more honest with what it
> is."

### What it does today — measured

**A faction knows what its mind knows** (design R3). The engine's own words
(`field.go`):

> "Mind is the member whose knowledge is the faction's — **the hub word spreads
> through**."

So a fact known by that one member becomes known by the whole side, and the
`disposition` `until:` predicate reads *the faction's* knowledge. A faction with
no mind **cannot learn**, which the doc calls "a consequence, not a loss": an
`until:` naming a fact simply never fires for it.

Three refusals (`factions.go`, `minds()`), each naming what is wrong:

| written | refused with |
|---|---|
| an id no placement has | "no placement in this dungeon has that id" |
| a **prop** | "a mind is a monster in the faction" |
| a monster in **another** faction | "a mind is a monster in its own faction" |

Two defaults do real work. **A faction of one *is* its own mind** — `singletonMind`,
never written. And a faction of many that waits for a fact with no mind is
refused: *"name a mind, or the faction cannot learn."*

### It is NOT a leader — but it is implementing a leader's plumbing

**Nothing flows through it but knowledge.** No orders, no authority, no
cohesion. If "the captain whose knowledge becomes the group's" is the fiction
you want, this is already the mechanism — what is missing is *authority*, and
that is a different concern with no field today.

So the honest sentence is: **`mind` is the faction's eyes, not its captain.**

### What it is really working around

**A faction cannot have a mind of its own.** `FactionInput` carries exactly two
fields — an `ID` and a `Mind MemberID`. It has no knowledge store and no
behaviour table. It is a *label* plus a *pointer to a member who does the
knowing*.

**And the architecture is already built for the alternative.** The `mind/`
packages split an agent in two, and neither half knows a game:

| package | owns | its own words |
|---|---|---|
| `mind/perception` | what an agent **KNOWS** — testimony, which may be false and stale | "what an observer holds" |
| `mind/behavior` | what it **DOES** with that — tables, deeds, temperament | "the mind's POLICY primitive" |

Neither says "creature": they say *observer* and *decider*. **A faction owning
its own `perception` store and its own `behavior` table would be an agent on
exactly the same two primitives a creature uses.**

There is a tell that this was always the shape: the reserved `monsters` faction
**"MAY be declared, which is how the unauthored monsters' side is given a
mind"** (`field.go`). The *side itself* is being given a mind. Today it borrows
one from a member. **The natural next step is that it does not need to borrow.**

### Why it stays where it is, and nothing moves

Kirk:

> "I do not think we need a faction to have a mind to get started right? I think
> when a use case arrives we can see it more clearly"

**Agreed, and it is the project's own rule**: *a use case brings the mechanism;
absent is a cost not yet paid, not a gap.* No room exists that needs a side to
know something no member knows, and guessing the shape now would produce the
kind of answer this whole doc exists to correct.

**And it is safe to wait, because the workaround works.** A faction of many that
must learn names a mind and gets the behaviour; a faction of one gets it free.
The only thing missing is knowledge that routes through no member — and when
that use case arrives it will arrive as **a room that cannot be authored**,
which teaches far more than speculation would.

**So: `mind` does not move to the bindings, and it is not renamed here.** One
thing worth noting precisely, because it looks like the engine already
disagreeing with §2 and does not: `field.go` says the mind is **"validated when
that member ENTERS THE RUN rather than at construction, because the authored
roster is not how monsters get in"** — the host spawns each one through
`Encounter.Join`. `validateMemberFaction` (`disposition.go`) is therefore asked
**at construction and at Join**, and refuses a member that is some faction's
mind from joining another.

**That is DEFERRAL, not mutability.** The check is not skipped, it is asked at
the moment the member arrives. So the engine is not declaring the mind mutable —
it is holding the same invariant at two doors. Whether a mind may *change* is
still unanswered, and §8's point is that the question goes away if the faction
gets its own mind.

**When a use case arrives, the question to ask is not "where does `mind` go" but
"does the faction get its own mind."** If it does, the field disappears rather
than moving.

### The name

`mind` is a poor name for a *member reference*, and the collision is not only
stylistic: `mind/perception` and `mind/behavior` are packages, and `play/intel`
is the *store* of what `perception` describes. So one word covers a package
family, a member pointer, and a store.

**Renaming it is deliberately not done here**, because §8's finding is that the
field may not survive its use case. Renaming a field we may delete is churn. If
it does survive, `knowsThrough` or `witness` says what it is; `leader` would say
something it does not yet do.

— rpg-project agent, on behalf of KirkDiggler