# The v4 gameplay shape: every key the engine can run, and where it goes

**Status:** RULED 2026-09-21 (R1–R5 below). Slice 2+3 SHIPPED: rpg-toolkit#1855 → encounter v0.96.0 (intel, holds, intimidate/persuade, arrives; propBindings fails closed pending rpg-toolkit#1854). Slice 1 (exits/endings/scenarios) next. Sources checked at
rpg-toolkit main `494ef265` (`rulebooks/dnd5e/encounter/dungeonspec`) and the
five seeded rpg-api reference dungeons (`rpg-api/content/*.yaml`, all
`version: 2`).

## Purpose — the tool this adds

The World Builder's document (root `version: 4`, room draft 3) can say what a
room LOOKS like and, since #477/#485, who is in it and which items are doors.
It cannot say what the room is FOR. Everything the old builder used to plant —
intel a monster carries, a scroll on the floor, a countdown that brings
reinforcements, an artifact and the way out, the check that decides whether
the goblin is cowed, the run's ending — exists in the engine today and is
reachable from the v2 dialect only. `CompileSingleRoom` hands the engine
`Key/Name/Field/PartyStart/Monsters/Factions/Dispositions` and nothing else
(`single_room_compile.go:152-155`).

This document is the shape that closes that gap: **one key per engine
capability, each in the home the existing declaration rules already give it,
proved by re-authoring a v2 reference dungeon in v4 and requiring the same
compiled picture.** No engine change. No new predicate, trigger, or scenario
machinery. The World Builder team gets a fixed list of what can be configured
and a fixed rule for where it goes, which is what a form needs.

Not in this document: the secret room. Intel that reveals a door needs a
concealed door, and a concealed door in a single room opens onto nothing. That
is the sites layer (rooms joined at seams; `authored-doors/design.md` §"The
sites layer"). `reveals: { fact }` does not wait; `reveals: { door }` does.

## The placement rule (already law, restated once)

Three rules decide every key below. None is new.

1. **Identity on the actor, orders on the binding** (#477 D4). `monsters[]`
   says who and where; `monsterBindings[id]` says what it does. A key that
   changes what a placed thing DOES is a binding key.
2. **A declaration is keyed by the id of the placed thing it is about**, and a
   declaration naming a thing that does not exist is refused by name (#485
   R2). `propDeclarations`, `monsterBindings`, `doorBindings` all follow it.
   A fourth kind, `propBindings`, follows it too.
3. **What is not placed lives at the root, beside `factions` and
   `dispositions`**: intel records, exits, endings, scenario bindings. v2 keeps
   them at its root for the same reason (`spec.go:212-268`).

The root already carries the two site-scope keys under v4; the room's gameplay
block carries the placed things. The line between them does not move.

## The shape, whole

Keys marked `# TODAY` are accepted on main. Keys marked `# NEW` are this
design. Everything else is the World Builder's own presentation, carried
verbatim and not modelled (`presentation-is-content/design.md`).

```yaml
version: 4
key: tomb-heirloom
play: { void: transparent, lighting: bright, standing: centre-covered }

factions:                                        # TODAY (#477)
  - { id: raiders, mind: chief, temper: [...], on: { ... } }
dispositions:                                    # TODAY (#477)
  - { between: [raiders, party], stance: hostile, until: { fact: saved-wiseman } }

intel:                                           # NEW  — v2 `intel`, verbatim
  - { id: wisemans-letter, reveals: { fact: saved-wiseman } }
  # reveals: { door } is REFUSED in v4 until the sites layer (R3)

exits:                                           # NEW  — v2 `exits`, cell in this dialect's own frame
  - { id: entrance, cell: { q: 1, r: 3 } }

endings:                                         # NEW  — v2 `endings`, verbatim
  - { id: held-out, when: { round: 6 } }
  - { id: turned,   when: { stance: { between: [raiders, party], is: neutral } } }

scenarios:                                       # NEW  — v2 `scenarios`, verbatim
  recover-the-artifact: { artifact: heirloom, exit: entrance }
  hold-out:             { convince: raiders }

room:
  version: 3
  id: ...
  coordinateFrame: ...                           # presentation, carried
  workspace: ...                                 # presentation, carried
  scene: ...                                     # presentation, carried
  room:
    walkableHexes: [...]                         # TODAY
    partyStart: { q: 1, r: 3 }                   # TODAY
    propDeclarations:                            # TODAY — the DEFINITION (footprint, blocking)
      heirloom: { blocksMovement: false, blocksLineOfSight: false, footprint: {...} }
      vault-door: { footprint: {...} }
    doorBindings:                                # TODAY (#485)
      vault-door: { closed: true, locked: [{ ability: str, dc: 15 }] }
    monsters:                                    # TODAY — identity + side
      - { id: chief, ref: "dnd5e:monsters:skeleton-captain", cell: { q: 12, r: 4 }, faction: raiders }
      - { id: reinforcement-1, ref: "dnd5e:monsters:zombie", cell: { q: 1, r: 4 }, faction: raiders }
    monsterBindings:                             # TODAY block; three NEW keys inside
      chief:
        on: { ... }                              # TODAY
        temper: berserker                        # TODAY
        actions: [...]                           # TODAY
        holds: [vault-map]                       # NEW — v2 `place[].holds`
        intimidate: [{ ability: intimidation, dc: 12 }]   # NEW — v2 `place[].intimidate`
        persuade:   [{ ability: persuasion,   dc: 10 }]   # NEW — v2 `place[].persuade`
      reinforcement-1:
        arrives: { down: chief }                 # NEW — v2 `place[].arrives`
    propBindings:                                # NEW — fourth declaration kind, same keying law
      heirloom: { holdable: true }               # v2 `place[].holdable`
      hall-scroll: { holdable: true, holds: [hall-notes] }
      letter: { holdable: true, holds: [wisemans-letter], arrives: { round: 6 } }
```

## Key table

Every key, its home, the v2 spelling it copies, the engine field it feeds, and
the seeded reference that proves it. "Shape" means the v2 type is reused as
is, judged by the shared grammar (`grammar.go`), so the refusal sentences
already exist.

| Capability | v4 home | v2 spelling | Engine field | Proof | Status |
|---|---|---|---|---|---|
| A record of knowledge | root `intel[]{id, reveals{fact}}` (`door` refused, R3) | `Spec.Intel` (`spec.go:212`) | `FieldInput.Intel` | raider-camp | NEW |
| Who carries it | `monsterBindings[id].holds[]`, `propBindings[id].holds[]` | `PlaceSpec.Holds` (`spec.go:1529`) | `MemberInput.Holds`, `PropInput.Holds` | tomb-heirloom, raider-camp | NEW |
| A prop the party can take | `propBindings[id].holdable` | `PlaceSpec.Holdable` (`spec.go:1544`) | `PropInput.Holdable` | tomb-heirloom (heirloom, scroll) | NEW |
| Something that comes in later | `monsterBindings[id].arrives`, `propBindings[id].arrives` | `PlaceSpec.Arrives` (`spec.go:1645`) | `MemberInput.Arrives`, `PropInput.Arrives` | raider-camp (`{round: 6}`, `{down: chief}`) | NEW |
| The price of leaning on a creature | `monsterBindings[id].intimidate`, `.persuade` | `PlaceSpec.Intimidate/Persuade` (`spec.go:1572,1583`) | `MemberInput.Intimidate/Persuade` | front-room | NEW |
| A way out | root `exits[]{id, cell{q,r}}` | `Spec.Exits` (`spec.go:230`), `at:[col,row]` | `FieldInput.Exits` | tomb-heirloom, raider-camp | NEW |
| How the run ends | root `endings[]{id, when}` | `Spec.Endings` (`spec.go:263`) | `SetupInput.Endings` | raider-camp | NEW |
| What the room is for | root `scenarios{id: {field: id}}` | `Spec.Scenarios` (`spec.go:247`) | `Compiled.Scenarios` | tomb-heirloom, raider-camp | NEW |
| A side | root `factions[]` | same | `Compiled.Factions` | front-room | TODAY |
| How sides stand, and until when | root `dispositions[]` | same | `Compiled.Dispositions` | raider-camp | TODAY |
| Membership | `monsters[].faction` | `PlaceSpec.Faction` | `MemberInput.Faction` | front-room | TODAY |
| What a creature says and does | `monsterBindings[id].on` | `PlaceSpec.On` | member table | front-room | TODAY |
| Temperament, arms | `monsterBindings[id].temper`, `.actions` | same | same | front-room | TODAY |
| A door and its state | `doorBindings[id]{closed, locked}` | `DoorSpec` minus `at` | door state | (doors wave) | TODAY |
| A hidden door | `doorBindings[id].concealed` | `DoorSpec.Concealed` | concealed door + Search | tomb-heirloom | REFUSED until sites |
| A hidden room | — | `RegionSpec.Concealed` | concealed region | tomb-heirloom | sites layer |
| The boss | — | `PlaceSpec.Boss` (`spec.go:1505`) | boss ending | tomb | see R4 |

## How things hook together — there is one wire, and it is the fact

The World Builder asked for a clear path to "how it can be hooked together".
The engine already answers with one grammar. Everything that fires is a
**source** of a fact or a state; everything that waits reads a **predicate**
over `{round | down | fact | stance}` (`predicate.go:150-153`). Nothing else
exists, and this design adds nothing.

| Sources (teach a fact / change a state) | Sinks (read a predicate) |
|---|---|
| `intel[].reveals.fact` — learned when its holder's record is read or taken | `dispositions[].until` |
| `on:` answer entry `fact:` — a creature's answer teaches it | `monsterBindings[].arrives`, `propBindings[].arrives` |
| a creature going down (`down: <id>`) | `endings[].when` |
| the round clock (`round: N`) | a scenario's own field (`convince:` is sugar for a `stance` ending) |
| a disposition ending (`stance`) | `on:` entry `when:` |

So "a message that changes sides" is already three lines: a record with
`reveals: { fact }`, a `holds:` naming it, and a disposition with
`until: { fact }`. "A countdown that spawns things" is `arrives: { round: N }`
on the thing that arrives. "Intel on a monster that leads to a secret" is
`holds:` on that monster's binding. No verb, trigger, or event key is needed
to wire any of them, which is what makes the form finite.

**A fact is declared by mention, on purpose** (`RevealsSpec.Fact` doc: "nothing
declares a fact"). For the picker, the World Builder derives the vocabulary
from the sources in the document. A sink naming a fact no source teaches is
NOT refused by the dungeon (pre-release: allow and show the cost; the scenario
refuses a hold-out nobody can win). Showing that cost in the builder is a
validate-only verdict at the sink's path, the channel the web already reads;
whether `validateSingleRoom` gains a non-refusing verdict list for it is the
one mechanism question this design leaves to the slice.

## Rulings — RULED by Kirk 2026-09-21

- **What #488 got wrong (found by the #1853 builder, 2026-09-21):** this design
  claimed the props half was threading only. It is not. A v4 item compiles to
  `PlacedPropInput`, a footprint that by its own doc is not holdable and never
  arrives; holdable/holds/arrives hang off the legacy `PropInput`, which needs a
  content ref and an anchor cell a v4 item does not have. The primitive is
  rpg-toolkit#1854 (a placed footprint that can be held and can arrive). Until it
  lands, `propBindings` decodes with every refusal and is then REFUSED at compile
  by name at its path, never carried inert.
- **R1 — `propBindings` is the fourth declaration kind. RULED YES.** `propDeclarations`
  is the prop's definition (footprint, blocking; World Builder owns it).
  `holdable`, `holds`, `arrives` are what a PLACED prop does, which is a
  binding by rule 1. Implications, so nobody discovers them in the slice:
  - A placed item may carry up to three declarations: definition (required,
    it is where the footprint comes from), door state, orders. An id in BOTH
    `doorBindings` and `propBindings` is refused by name until a use case
    says what a door that is also taken means.
  - A holdable prop is walked onto to be taken; v2's holdables all declare
    `blocks_movement: false`. The slice pins whatever v2 already refuses here
    and adds nothing.
  - A prop that `arrives` is NOWHERE until its predicate holds, exactly as a
    monster is. The scene still carries its node (presentation is content);
    what is on the floor is the engine's `placed` answer, never the scene.
    The web already reads `placed` since the doors wave; this is the second
    reason it must.
  - Keyed by item id only. An arrangement's members get no orders, the same
    refusal an arrangement door gets.
  - Web pass-through: web#1177 carries the monster-binding keys and root
    `intel`; `propBindings` is one more block for the same rule.
- **R2 — an exit has `partyStart`'s shape (`cell: {q, r}`). Kept, with a note.**
  Kirk: a placed door's hex is a threshold — step into it and you are seen
  from the other side. When the sites layer joins rooms, that hex is where a
  crossing happens, and an exit cell that IS a door hex composes with it
  without a second spelling. Exit-as-door is deferred to sites, not rejected.
  Kirk's easy solve for now: you must OPEN the door to stand on the hex it is
  in — which is already DoorState's law (closed/locked block movement, open
  blocks nothing), so an exit cell on a door hex needs no new rule.
- **R3 — `reveals: { door }` is REFUSED in v4.** Kirk: "reject the reveal or at
  least return a warning." There is no warning channel, so it is a refusal by
  name at `intel[<i>].reveals.door`, in the sentence `doorBindings.<id>.concealed`
  already uses: the word means something, it is simply not built until the
  sites layer. `reveals: { fact }` is the only target in v4.
- **R4 — `boss` DEFERRED.** No key in this design; Kirk suspects no and wants
  the decision left open. The ending `when: { down: <id> }` says what the flag
  said. Nothing here forecloses adding it.
- **R5 — `intimidate`/`persuade` on the monster binding. RULED YES, with a
  direction.** The DOCUMENT carries the DCs as orders (this slice). Kirk's
  direction for the VERBS, not this slice: both are offered OUT OF COMBAT ONLY,
  when interacting with a non-hostile creature, and the options offered are
  the ones CONFIGURED on that creature — the seed of a dialogue system that
  can grant intel. That changes how session prices and offers the verbs
  (today Intimidate is a combat action and Persuade is priced on both clocks),
  and it retires the derived passive-Insight default for the OFFER. Separate
  design; the document shape is unaffected.

## Slices — one module, three PRs, each walks

All three land in `rulebooks/dnd5e/encounter` (dungeonspec), one PR per slice
so each ships as its own encounter tag and each is walked before the next
merges. Each slice's proof is the same shape: re-author the reference in v4,
compile both, and pin `Compiled` equal on the fields the slice adds (the v2
goldens already exist under dungeonspec testdata).

1. **The prize and the way out** — `propBindings{holdable}`, `exits`,
   `endings`, `scenarios`. Proof: tomb-heirloom in v4 minus its concealed
   door (the heirloom stands in the open until sites), compiled artifact +
   exit + scenario binding equal to v2's. Walk: pick up the heirloom, stand on
   the entrance, the run ends.
2. **Knowledge** — `intel`, `holds` on both binding kinds, `intimidate`,
   `persuade`. Proof: front-room in v4 (already the web's own sample fixture
   `04-front-room-in-v4.yaml` for the `on:` half) and raider-camp's letter.
   Walk: Intimidate the goblin at the authored DC; read the letter, the
   raiders stand down.
3. **The clock** — `arrives` on both binding kinds. Proof: raider-camp in v4;
   three zombies arrive when the chief falls, the letter on round 6. Walk: drop
   the chief, count three.

Every slice is unit-test proof of the compile and a one-click-per-seam walk
on the stack; no probability is sampled on the stack.

### Precondition, not ours to build

The web's pass-through of gameplay keys it does not render (rpg-dnd5e-web#1171,
"one key, two acceptances") is NOT merged into `dev`; what shipped as #1172
added `doorBindings` by name. Until the pass-through lands, each root and
binding key above bounces once more at `strictShape.ts`. The World Builder
lane owns that; this design lists it so nobody briefs slice 1 as if it did
not exist.

### Out of scope, named

- A generic trigger, event, or script key. The predicate set is the trigger
  system; widening it is a separate design with its own use case.
- Any allied flip. The hold-out ruling stands: a side stops being hostile, it
  never becomes the party's.
- The secret room and `concealed` on a placed door (sites layer).
- The web form itself. This is the document the form fills; the form is the
  World Builder lane's.

## Done-when

- All `NEW` keys accept in v4 with their v2 refusal sentences at the v4 path.
- tomb-heirloom, front-room, raider-camp each exist as a v4 document in
  dungeonspec testdata with a compile-equivalence test against the v2 file.
- The three walks above are recorded on the slice PRs.
- `single_room.go`'s doc comment lists `propBindings` as the fourth
  declaration kind, keyed by the same law.
