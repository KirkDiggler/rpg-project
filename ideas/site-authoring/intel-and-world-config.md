# Intel and world configuration in the World Builder

**Status:** DESIGN, for review. Companion to `ideas/site-authoring/design.md`
(the site root, inhabitants and policy). Tracking issue: rpg-project#476.
**Consumers:** the World Builder (`rpg-dnd5e-web`,
`src/concepts/world-building/`), the strict single-room decoder and compiler
(`rpg-toolkit`, `rulebooks/dnd5e/encounter/dungeonspec`).
**Driving scenario:** The Front Room — *a failed persuasion calls in the thug
for reinforcements.*
**Load-bearing instruction from the human:** **hold at the `concealed` edge** —
concealed-door and everything that rides on them is deferred, not half-built.

## The one sentence

The builder today gets one social scenario to *work*; this wave gets it the
**configuration** to *make* any scenario. The lightweight outcome choices a
failed `persuade`/`intimidate` can already teach are a proof that something can
work; what is missing is exposing the **intel** and **world** nouns — the
knowledge records creatures carry, the checks an author prices, the reserves a
creature waits in — so a scenario an author invents is not guessed at in YAML.

> "we want to bring in the ability to configure things. we do not need anything
> to just work. … it is time for us to expose the intel and world configurations
> so we can make robust scenarios." — Kirk, 2026-09-20

## Why now

The v4 World Builder landed, in order: the site root (factions, dispositions),
then `monsterBindings` (a creature's own `on`/`temper`/`actions`), then doors
(`doorBindings`). Each was a **key arriving inside a version** (rpg-toolkit#1825)
behind the builder's form. What a scenario needs to be *robust* is exactly the
next set of keys, and every one of them exists on the v2 `PlaceSpec` or in the
v2 file today:

| The noun a robust scenario needs | v2 (hand-authored, works) | v4 builder today |
|---|---|---|
| a creature taught a fact on a failed check | `on.persuade_failed: [{ say, fact: cellar-is-clear }]` | **YES** — the answer table ships (`sample 04-front-room-in-v4.yaml`) |
| a reserve creature held out until the fact is known | `place[].arrives: { fact: cellar-is-clear }` the bandits | **NO** — no `arrives` anywhere in v4 |
| the check an author prices | `place[].intimidate:[dc:12]`, `persuade:[dc:10]` | **NO** — v4 has no social-check field; the goblin's DC 12/10 asymmetry cannot be written (recorded in sample 04) |
| intel — a record that reveals something | `intel:`, `place[].holds:` | **NO** — no intel section, no holders |
| who stands neutral to whom | `dispositions:` | **YES** — `SiteDisposition` with `until` |

So the **fact the scenario turns on already travels**; what is missing is
everything *around* it that makes the fact matter — the check that can fail, the
creature that answers the failure by arriving, and the intel that ties knowledge
to a held record. Those are the four v4 keys below.

## The driving scenario, spelled as the target

The Front Room, in the dialect this design is working toward. The *content* is
not the deliverable — the deliverable is that an author can say this through the
forms. Two halves:

```yaml
# A creature that talks, with the check priced and the failed outcome a lie.
monsterBindings:
  front-goblin:
    intimidate: [{ ability: intimidation, dc: 12 }]
    persuade:   [{ ability: persuasion,   dc: 10 }]
    on:
      persuade_failed:
        - { weight: 100, say: "Cellar's empty, friend.", fact: cellar-is-clear }

# A thug held in reserve, called in by the lie it would believe.
monsters:
  - { id: thug-1, ref: "dnd5e:monsters:thug", cell: { q: 12, r: 4 }, faction: bandits }
monsterBindings:
  thug-1:
    arrives: { fact: cellar-is-clear }
    on:
      time:
        - { when: { enemy: none }, toward: enemy }   # walks to the front room
        - { when: { enemy: reach }, attack: enemy }
```

The engine already owns every word: `intimidate`/`persuade` and `arrives` are
`PlaceSpec` fields in v2; the `fact` predicate is documented as *"on `arrives`,
holds when anyone in the run does"* (`dungeonspec/predicate.go`); `toward`/`attack`
are v0.90.0's `time` words. None of it has a **v4 home** — and this design is
naming that home, against shapes that already exist rather than shapes to invent.

## What exists, precisely

Engine (`dungeonspec`, v4):

- `RoomMonsterSource` — `{ id, ref, cell, faction? }`. Identity and placement.
  **No `arrives`.**
- `RoomMonsterBinding` — `{ on?, temper?, actions? }`. Orders. **No
  `intimidate`, `persuade`, `arrives`, `holds`.**
- `RoomDoorBinding` — `{ closed?, locked?, concealed }` where **`concealed` is
  refused by name** (R2). The one refusal we keep, and the edge we stop at.
- The compile (`single_room_placement.go` / `single_room_compile.go`) builds
  each monster as `ordersOf(creatureOrders{ … })` and sets nothing for
  Precise/Arrive — the canonical `MonsterPlacement` already *has*
  `Intimidate`, `Persuade`, `Arrives`, `Holds`, so the change is threading,
  not inventing.
- `Fact` predicate already compiles for `arrives`.

Web (`src/concepts/world-building/`):

- `RoomMonsterBinding` mirrors `{ on?, temper?, actions? }` plus `doorBindings`.
- `SiteScope` — `{ factions?, dispositions? }`; `SiteDisposition.until` is a
  `PredicateDoc` (the shared predicate editor already exists).
- **No intel section.** The v2 builder's intel panel (web#933) is not ported.
- The form rule (site design, 2026-09-19): **the builder is a form builder and
  nothing more** — it renders controls over what the toolkit accepts, carries
  a key it does not interpret, and never decides what an entry "means."

## Four v4 keys this wave brings

Each *arrives inside a version*, judged by the one engine grammar at
`PutDungeon`, carried by the web as a shape and never graded twice
(rpg-project#481/#483, web#1171's "carry, let the engine grade" pattern).

### 1. `intimidate:` / `persuade:` — the priced check

On the creature's binding, as v2's `PlaceSpec`: a **list** of
`{ ability, dc [, tool?] }` rows, the author's routes, carried verbatim.
Absent means the engine derives the DC from the stat block's passive Insight —
the rulebook still answers when the author says nothing. This is the current
front room's deliberate asymmetry (Intimidate DC 12, Persuade DC 10, *"this
goblin would rather be talked to than shouted at"*), which sample 04 records as
the one thing v4 cannot yet say.

### 2. `arrives:` — the reserve / reinforcement

On the creature's binding or source (see the ruling below), v2's `PlaceSpec`
predicate: `round`, `down`, `fact`, `stance` — the **same `PredicateSpec`** the
builder already authors on a disposition's `until` via `PredicateEditor`. A
creature with an `arrives` predicate is held out of the run until it holds;
the Front Room thug is only *called in* by the lie. This is the "call in
reinforcements" ability, and it is the same grammar as `until`, which is the
point — one predicate, every place a scenario waits on.

### 3. `intel:` — the knowledge record

A **site-level** (v2 `intel:` was dungeon-level) section, ported to the new
builder: a record has an id and a `reveals` target; placements *hold* records.
In v2 today `reveals: { door: <id> }` is the only shipped kind. The record list
and "new intel" live in the document's working set beside the policies, per
v2's R7 (the record is a dungeon/site section, not a palette item) — and the
same choice carries to the site builder. This is the **"configurable intel
system"** the sentence is about.

### 4. `holds:` — who carries the record

On the creature's/prop's declaration, v2 `place[].holds`: a list of record ids,
legal on monsters **and props** (R6 — the letter in the hall). A holder carries
the records from spawn; the engine resolves `reveals` at transfer.

## The one ruling this design needs

**Where do `arrives`, `intimidate`, `persuade`, `holds` live in v4?** The site
design's Decision 4 rule is — *if the faction supplies it, it belongs in the
orders block; if it selects the faction, it belongs on the actor* — and none of
these four is either. They are per-creature declarations that no faction
supplies and nothing inherits, which makes `monsterBindings` their natural
home, **exactly the way a prop's `propDeclarations[itemId]` is the whole
authored fact block for a placed thing**. Recommendation:

```
monsterBindings[monsterId] = { on?, temper?, actions?,           # orders, shipped
                               intimidate?, persuade?,           # checks, this wave
                               arrives?, holds? }                # reserve + intel, this wave
```

Membership stays on the actor (`RoomMonsterSource.faction`), unchanged. A
binding still can never outlive its creature (rebuilt on removal), and a site
with none of these keys stays byte-identical.

## Ownership and the seam

| noun | owner |
|---|---|
| the priced check (`intimidate`/`persuade`) | authored content; the engine stores the rows and the rulebook derives the DC when absent |
| `arrives` predicate | authored content; the compile lowers it to a composition trigger |
| intel record and `reveals` | authored content (like a door); the engine reads reveals at transfer |
| `holds` | authored content → encounter holdings at spawn |
| what a record reveals | the **engine**, read at transfer, never the builder |

The web **renders the forms; it never resolves a ref, never decides which entry
fires, never derives a DC, never guesses what `arrives: { fact: X }` will do.** A
key the engine refuses comes back with the engine's own path and sentence, which
is the builder's only opinion. Carried, not graded — the same contract `doorBindings`
was landed on (web#1171).

## UI surfaces (extends the working set, not the canvas)

Separate by task, per the site design. The site's working set already carries
`monsters`, `doors`, `policies`. This wave adds the nouns that belong beside
them, as panels — **not** a second YAML editor and not a palette item:

- **Intel** — the record list and "new intel", beside the policies. Select a
  record: name it, choose what it `reveals`, see who holds it. Select a
  creature: see what it holds read-only with a link back to the record — the
  creature is not where you edit intel (v2 R7/R2).
- **A creature's interaction** — the priced check rows (`intimidate`/`persuade`)
  and, on a creature that waits in reserve, its `arrives` predicate — using the
  shared predicate editor already used for a disposition's `until`.
- **Placement** (a reserve creature) — `arrives` reuses the predicate editor;
  the reserve is authored as a placed creature with an `arrives`, not a new noun.

Nothing new on the canvas: the reserve creature is still a creature marker with
a binding. The copy is in the document the floor already draws.

## Slices — each ends observable

Ordering is **inside-out** for merge (engine → web) but **outside-in** to
develop (the web form is what proves the key is missing). Each key is a slice,
so a slice is a vertical cut that closes a round-trip claim rather than a
horizontal layer.

1. **`intimidate`/`persuade`** — engine accepts the two check fields on
   `RoomMonsterBinding`, threads them into `creatureOrders`/`ordersOf`
   (canonical `MonsterPlacement.Persuade`/`Intimidate` already exist); web
   declares the fields and renders the check-row editor on a creature. Walk:
   the front-room goblin's DC 12/10 asymmetry re-derived from a v4 file through
   the form. *(Closes the sample-04 gap.)*
2. **`arrives`** — engine accepts the predicate on a reserved creature and
   lowers it; web edits it with the shared predicate editor. Walk:
   the thug is held out of a run until the `cellar-is-clear` fact lands, then
   is *in* the run and walks/turns hostile.
3. **`intel:` + `holds`** — engine accepts the site-level records and the
   hold-list on a binding; web gains the intel section and the held-by/read-only
   viewers. Walk: a record held by a creature reveals a door/fact to whoever
   comes into contact.
4. **The Front Room, re-authored in v4** — the content itself becomes the
   fixture for the whole wave: goblin with priced checks + the lie, thug in
   reserve called in by it. Same bytes through the form and by hand.
5. **The form-builder guardrails** — a reselection/rename of a faction, door, or
   intel id cannot leave a `holds`/`arrives`/`until` pointing at a name that no
   longer exists; every claim is a test.

Each slice is **one repo pair** (toolkit + web), one branch each, with the api
content file (the real front room) only re-authored in slice 4.

## Done when

- The Front Room's goblin carries its authored DC 12/10 in a v4 document, saved
  through the form, and the engine rolls those checks.
- A thug with `arrives: { fact: cellar-is-clear }` is absent from a run until a
  failed persuasion teaches the fact, then walks to the fight — held in
  reserve is a real authored state, not a line the engine ignores.
- An author can create an intel record, say what it reveals, and place it in a
  creature or prop through the forms, byte-exact on re-save.
- A site with none of these keys is byte-identical to today; an engine refusal
  arrives with the engine's own path and sentence.
- Every claim above is a test; a walk of the front room proves the wiring.

## Deliberately absent — deferred at the `concealed` edge

- **`concealed` doors and everything that reveals them.** `RoomDoorBinding` refuses
  `concealed` by name today; the concealed-door laws are written for a door on a
  crossing, and the World Builder has not asked that picture question. **Hold
  here.** Not because it is hard — because it is a geometry/`intel` coupling we
  are choosing not to build on top of before the seams are proven. A `reveals:
  { door: … }` intel target exists in v2 and would be *the* concealed use case;
  it waits with this.
- **`kind`**, **`sites`**, non-monster actors — already named and deferred in the
  parent design.
- **Scenario-endings** (`when`) — v2's `endings` stays out of this wave; a robust
  *scene* does not need the run to end yet.
- **Allegiance** (spawn-a-faction-at-runtime) — the parent design's second wave;
  none of these four keys depends on it.

## Residual risk, named

- **The engine's `PredicateSpec` and `CheckSpec` are richer than the v2 `arrives`/
  `intimidate` the web has edited.** `PredicateSpec` already covers round/down/
  fact/stance and the shared editor already authors all four on a disposition's
  `until`, so `arrives` reuses a shipped editor rather than a new one — the risk
  is low but the `down`/`stance` forms must be exercised on a *reserve* cream partner
  (a creature that falls, a faction that folds) in the walk, not just the `fact` form.
- **Intel is scenario-independent and stayed that way for a reason (R5).** A
  reveal target is read by the engine at transfer, not authored into a scenario.
  The builder must not grow a "search this to reveal that" opinion — that is
  the DM's scenario, invented in the file, not a builder feature.
- **`sample 04` already recorded one finding and it is still true**: v4's
  `RoomMonsterBinding` has no social-check field. This wave closes it; the
  record moves into this design so the next reader does not re-discover it.

— platform agent, on behalf of KirkDiggler