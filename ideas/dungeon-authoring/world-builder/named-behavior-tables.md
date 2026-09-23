# Named behavior tables at the site root — declared once, referenced by id

**Status:** DESIGNED 2026-09-24, not implemented. Supersedes the pointer of the
same name (2026-09-23), which recorded the idea and its trigger and deliberately
designed nothing. The trigger it was waiting for has arrived.

## The one sentence

**The engine already has no place for a table's name, and that is the whole
design: a named table lives in the document, and resolves to the inline table
every creature already accepts — so this is a web slice and the engine does not
move.**

## What this is

A place in the World Builder to **declare and name behavior tables at the site
root**, the way `intel`, `factions` and `dispositions` are declared today — a
named block carrying a creature's `time` table (and whatever an entry grows to
carry) that a creature **references** rather than pastes.

Kirk, 2026-09-23:

> "I think having the place in the builder where we can name the tables at the
> root like we do disposition and others. this will obviously come in waves and
> slices."

Kirk, 2026-09-24, on the order of work:

> "I wonder if we should move the definitions off a monster and add it to the
> root node first. we would reference them on a given monster."

## Why the engine does not move (the deciding finding)

The question that decides whether this is a web slice or a cross-repo wave is:
**does the engine need to learn the name?** It does not, and this is not a
judgement call — it is what the code already says.

1. **A table has no name, and no place to put one.** `mind/behavior.Answer` and
   the `encounter.Table` it builds carry no `Name`/`ID` field; the table is a
   `map[AnswerKey][]Answer` and nothing else (`mind/behavior/answer.go`,
   `rulebooks/dnd5e/encounter/table.go`).
2. **The driver reads one table off the creature.** `tabledriver.go` rolls
   `view.Table` — the creature's own compiled table. There is no lookup, no
   registry, no indirection to satisfy.
3. **There is exactly one point where authored YAML becomes that table.**
   `dungeonspec/compile.go` `ordersOf` → `encounter.Layer(tableOf(from.on[side]),
   tableOf(c.On))`. The faction's table is laid under the creature's, key by key.
   Everything upstream of `tableOf` is authored text; everything downstream is
   the composition's. **A reference is resolved entirely upstream of this line.**

So a named table is a **document-level** fact that the builder resolves at
publish time into the inline form. The compiled dungeon the engine receives is
byte-identical to one where the author pasted the table by hand. The engine
cannot tell the difference, which is exactly right: the name is the site's, the
engine never learns one.

This also settles the pointer's open question — *"whether a referenced table may
carry inputs beyond its entries."* **It may not, in this slice.** Inputs would
make resolution a computation rather than a copy, and that is precisely the
change that would drag the engine in. The patrol-route question stays the
engine's (rpg-project#498) and stays separate. If inputs ever land, resolution
stops being a copy and this doc is superseded — not silently stretched.

## What it sits beside — the precedent to copy

The site root today is `version`, `key`, `play`, `room`, `factions`,
`dispositions`, `intel`, `exits`, `endings`, `scenarios`, `concealments`.

**`intel[] + holds` is the pattern, and it is already proven in this codebase.**
`intel` declares records at the root with an id; `place[].holds` names them by
id; a dangling reference is refused **by name at author time**, in the
validator, in the author's own terms:

```
validate.go:  "%q holds intel %q, and no record in this dungeon has that id"
```

and the reason it gives is the reason to copy it: *"a placement holding nothing
the author declared is a secret they think they placed and did not."* That is
the same class of failure a named table can have — a creature pointing at a table
nobody wrote, that reads as orders and behaves as none. (`knowsRefusal` makes the
same point from the other side: *"declare it under `intel:` with an id … then put
its id in this placement's `holds:`"*.) The toolkit's own comment on the field
calls it out as the general shape:

> "A RECORD IS A THING IN THE FILE, like a door. It has an id and it says what
> it reveals, and it is placed by NAME on whatever carries it — the same shape a
> door has, declared once and referred to from wherever it is used."

A named table joins them, with a panel beside `IntelPanel`. Same declaration,
same reference-by-id, same refusal of a name that resolves to nothing.

**And the rename discipline is already written down**, in `intelEdits.ts`:

> "Renaming a record id changes the declaration and nothing else — if a
> creature's `holds` still names the old id, the ENGINE refuses that by name, and
> that sentence is the one to surface. Semantic authority is the server's, not
> the form's."

A named table behaves identically. The builder **never rewrites references** and
never protects the author from a name it cannot resolve.

## Why this is worth building now — a hazard it removes

`time:` **replaces the kind's whole default table.** Authoring one entry on a
creature silently discards everything the kind's own table carried. This bit us
three times on 2026-09-23; the memorable one is a goblin that lost
`enemy: seen → toward` the moment a `time` was authored, and stood still in a
fight it should have walked into.

A named table does not merely add reuse — **it makes that hazard visible and
survivable.** The table is declared once, seen whole, and a creature either
references it or does not. A reference is total by construction: there is no
invisible partial edit to lose a default in.

## The shape (proposed, for the design review to rule on)

```yaml
# at the site root
tables:                       # NAME PROVISIONAL — see below
  - id: cornered-goblin
    time:
      - { when: { enemy: reach },  attack: enemy }
      - { when: { attacked: { within: 3 } }, attack: attacker, weight: 3 }
      - { when: { enemy: none },   toward: enemy }

# on a creature — a reference, never a copy
place:
  - { ref: dnd5e:monsters:goblin, at: [3, 4], table: cornered-goblin }
```

**A reference IS the table; inline and reference are mutually exclusive.**
Authoring both `table:` and `time:` on one creature is refused by name. This is
consistent with `time` replacing wholesale everywhere else: two ways to say what
a creature does with its time, both present, is an author asking a question with
no answer that is not a guess. The refusal's wording is the design review's.

### Not decided here, and flagged as such

- **The key's name.** `tables` above is a placeholder. The pointer correctly
  recorded that `time` was rejected as a root key (it conflates *when a creature
  acts* with *a named block*), and the real name should come from the panel that
  gets built. Not invented in this doc.
- **The reference key on the creature.** `table:` above is a placeholder for the
  same reason.
- **Whether the reference sits on `place[]` or on the orders block.** The
  single-room dialect reads a creature's orders off a block keyed by id while v2
  puts them on the placement; `creatureOrders` reconciles both. The design review
  rules on which spelling wins, and the compile already has the seam for it.

## Where it lands

- **A web slice, one PR: rpg-dnd5e-web#1199's dependency and predecessor.**
  Root declaration + panel + reference-by-id + refusal of a dangling name.
  Evidence is a published document the engine accepts.
- **No toolkit PR.** Per the finding above, nothing in `mind/behavior` or
  `encounter` moves. If implementation discovers otherwise, that is a finding
  worth an issue, not a quiet widening of this slice.
- **No API change.** The compiled document is the one the engine already takes.

## Why it goes before the `when` scope editor (rpg-dnd5e-web#1199)

Both changes touch the same object: #1199 adds `on:`/`as:` to a `when` inside a
table entry, and this changes *where that table lives* and *how a creature points
at it*. Done in this order, #1199 lands on the shape that stays. Done the other
way, the scope work is small and survives but **the plumbing around it is written
twice**.

Ordering is the whole argument. Neither change is large; together they are one
line of work on one object, and splitting them the wrong way doubles the part
that has no design value.

— rpg-toolkit agent, on behalf of KirkDiggler