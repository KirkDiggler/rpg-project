# World members — the third kind, and the room before the dungeon

The WHY. Normative shapes live in [design.md](design.md).

## The ask

Kirk, 2026-08-28: a room *before* the main dungeon start, entered through a real
door, with a merchant standing in it. Not a second zone — a region of the same
dungeon. The merchant is an NPC and **not hostile**. It happens in free roam.
The destination is [rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275),
the vendor/NPC inventory foundation.

**Parent: rpg-project#311**, World NPC Foundation — filed the same day and
asking for exactly this abstraction. **Sibling: rpg-project#310**, the buy-only
vendor, which layers on top. This document is the design for #311; the vendor is
designed in #310 / rpg-toolkit#1275, not here.

Two motivations were bundled at the start and came apart under reading:

- *"We cannot set the type on the regions — we had entrance, chamber, corridor."*
- *"I was thinking the boss room needed something."*

Both dissolved. See **Rejected** below.

## What we found

`MemberKind` is a two-value enum — `KindPlayer`, `KindMonster` — and every place
that decides hostile behaviour switches on it **exhaustively, with no default
arm**. A third value therefore inherits the right answer in three separate
places without any of them being edited:

| Behaviour | Where it is decided | Why a third kind is already correct |
|---|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` | Partitions standing members into `players` and `monsters`. Its own doc states the law: *"a member who is not on a side is in no pair, and a pair is the only thing that forms or joins a bubble."* |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` | Counts players and monsters, returns `players == 0 \|\| monsters == 0`. An uncounted kind cannot keep a decided fight running. |
| Cannot be attacked | `session/attack.go` (two sites) | The candidate target set is built with `if member.Kind == KindMonster`. A non-monster is never a candidate; naming one returns `ErrStaleDeclaration`. |
| Ignored by monster AI | `behavior/basic.go` | `if sm.Kind != encounter.KindPlayer \|\| !sm.Standing { continue }` — monsters consider players and nothing else. |

The pattern underneath all four is the thing worth naming: **every decision in
the hostile path is an allow-list keyed on `MemberKind`, never a deny-list.**
Nothing anywhere asks "is this member harmless?" — each site asks "is this a
player?" or "is this a monster?" and ignores everything else. That is why a
third value is safe, and it is also why the audit in design.md rule 9 is the
real work: the claim is only as good as the sites we have actually read.

We are not adding a concept. We are adding the value the existing law was
written to tolerate. The cost is an audit, not a redesign.

## Rejected

**Region typing (entrance / chamber / corridor).** `RegionSpec` v2 carries
`ID`, `Name` (carried, never read), `Archetype`, `Lighting`, `Cells`. Region
kinds were removed deliberately: v1 *derived* `start` from an archetype, and the
spec comment names that as "the shape of defaulting rpg-toolkit#1033 forbids."
`Archetype` is the surviving label and is ruled **presentation only, never a
mechanic** (rpg-project#256). Kirk withdrew the ask once the boss case dissolved.

**A region type for the boss room.** Not needed — `PlaceSpec.Boss` already
exists and `validate.go` enforces one boss per region ("region %q already names
%q as its boss"). The boss is a placement fact, not an area fact.

**A hostility / faction / disposition model.** This is the wrong axis. Kirk:
*"if it was ally, hostile then neutral makes sense — monster, player and npc seem
right to me."* The enum answers **what a member is**, not how it feels about you.
Disposition is a second axis that would need its own use case; nothing has one.

**Merchant placed as a monster.** `classify` loops `players × monsters`, so a
merchant carrying `KindMonster` forms a bubble the instant anybody sees him.
This is why the third kind is required rather than merely tidy.

**A separate zone / scene for the shop.** The whole point of putting the room
inside the dungeon spec is avoiding a second world concept, its load path, and
its camera rules. One compile, one map.

## How the third kind got its name

It was `npc` for most of a day, and Kirk broke it himself: *"having monsters in
the npcs category feels off... tech these are also not players so could be put in
the npcs with monsters. If that feels wrong then the monsters are in the wrong
place."*

He was right, and the diagnosis is that **`npc` defines the kind by negation**.
"Not a player" is a description monsters satisfy too, so the category had no
edge — which is exactly why monsters kept sliding into it, in the art directory
and in the argument.

`world` defines it positively, and it makes the whole set one statement:

- `player` — belongs to a person
- `monster` — belongs to the fight
- `world` — belongs to the place

Kirk's own gloss, and it is load-bearing rather than decorative: without the
shared axis stated, `world` reads as "the environment". design.md rule 1 puts it
in the godoc for that reason.

**The test that settled it was the hired mercenary** — an ally who fights. Not a
player, not a monster, so `npc` would have welcomed it in, and it would have
broken all four inherited behaviours the moment somebody placed one. `world`
refuses it by name: a mercenary belongs to a side, not to the place. A name that
excludes the right things is worth more than a name that reads smoothly.

The vocabulary also matches #311's own title — *World NPC Foundation* — which
had the answer in it the whole time.

**The ref type stays `npcs`** (`dnd5e:npcs:merchant`, Kirk's ruling). The two
words are deliberate, because they answer different questions: `MemberKind` says
what something is in a fight, while the ref type names a content bucket beside
`dnd5e:props:pillar` and `dnd5e:monsters:skeleton`. `dnd5e:world:merchant` reads
badly — "world" is not a category of thing. Forcing one word to serve both
questions is what produced the original confusion; the compiler is the single
place the two vocabularies meet, and design.md rule 10 requires it to say so.

## Rulings (Kirk, 2026-08-28)

1. **The merchant cannot be attacked.** *"That may come, but is definitely not a
   launch goal and probably not needed."* `Kind` therefore stays a fixed authored
   fact and never mutates in play — which is what keeps this slice small.
2. **The kinds are `player`, `monster`, `world`**; the ref type is `npcs`.
3. **NPCs carry an authored facing.** *"They have no behavior so will need a
   facing."* Monsters are refused facing because they turn dynamically in play;
   an NPC never does, so the authored value is the only one there will ever be.
4. **`bystander` is a role *within* the kind**, alongside `merchant` — *"that
   is something else, we don't have a use case for yet."*

## The scope split (Kirk, 2026-08-28)

*"FadedPez will be taking the work to give the merchant the actions. We are only
making a space for them to be placed in the dungeon builder."*

So this slice is the **placement seam**: a kind that is on no side, a ref, a
dungeonspec type segment, one additive enum value, and the builder affordance to
put one down. It gives the NPC no actions at all.

That split is the reason the capability set (`TALK`, `VENDOR`, `TRAINER`,
`QUEST_GIVER`, `QUEST_TARGET`) moved OUT of this design's normative rules after
being briefly written into them. rpg-project#311 suggests that set, and it is a
good suggestion — but it describes what an NPC can be *asked*, which is the half
FadedPez owns. Specifying his contract before he has the actions in hand would
be designing someone else's seam from the outside, and it would very likely be
wrong in the details that matter to him. design.md states the seam he attaches
to and stops.

**What we owe him instead of a design** is the finding: nothing in the hostile
path will act on his NPC, and here is why, site by site. That is in the table
above and on #311.

## What "a space to be placed" actually costs

Larger than it sounds, because the builder's palette applies a **ref-AND-GLB
test** — `paletteData.ts` offers a thing only when a toolkit ref *and* a promoted
GLB both exist, and it documents refs excluded for failing either half (`ghoul`
and `skeleton-archer` have refs and no art; `ghost` and `specter` have art and no
ref). Neither half alone is placeable.

So "place a merchant" reaches further than the encounter: a `TypeNPCs` constant
and a merchant ref in `rulebooks/dnd5e/refs`, a promoted merchant GLB, and a
model-resolution path beside `monsterModels.ts`. The GLB is **Assets-lane work**
and is filed under the journey rather than pulled into this slice.

Two things found while sizing that, both good news and bad news:

**The art already exists.** `SK_Chr_Merchant_01` is in the Synty Fantasy Kingdom
pack and is already converted to GLB under `assets/synty/converted/`. The Assets
task is a promotion through the pipeline that already promoted the four player
classes from townfolk source — not a hunt for new art.

**The name `npcs` is already taken, by monsters.** `public/models/synty/npcs/`
holds ghosts, skeletons, zombies and the tormented soul, keyed `npcs` in its own
manifest — "NPC" in the older sense of *anything not a player*.

Reading its manifest is what dissolved the question. That directory is **not a
taxonomy**: it is the polygon-dungeon promotion wave, all seven sharing one
55-joint armature and one retarget script. A pipeline fact wearing a fiction
name. And rpg-dnd5e-web#559 already ruled where identity lives — each entry's
`rulesRef`, explicitly *not* the filename or the path, because the art-to-ref
mapping is not 1:1.

So renaming it `monsters/` would swap one aspirational label for another and rot
immediately: the merchant comes from Fantasy Kingdom, the same family the four
class models were promoted from, and a wave mixing a merchant and a skeleton on
one rig breaks `monsters/` exactly as `npcs/` is breaking now. **Recommendation:
leave it, or rename it for what organizes it (the wave), and keep `rulesRef` as
identity.** Merchant art goes with its own wave regardless. Out of scope for this
slice; recorded so it is a decision rather than a discovery.

## Shelves left empty

Named attachment points with nothing on them, per the standing rule that the
shelf matters and its contents do not until a real case arrives:

- **Everything an NPC can be asked.** Capabilities, the interaction verb, its
  wire shape, adjacency. Named here so the seam is legible, owned by FadedPez,
  and deliberately unspecified by us. Kirk's `bystander`-is-a-role-not-a-kind
  point is what a capability set would express — which is exactly why it is his
  to shape rather than ours to pre-empt.
- **Attackable NPCs**, and with them a mutable `Kind` — deliberately deferred.
- **Disposition**, if a member ever needs to change sides.
- **Interaction beyond the shop** — the verb seam is shared, the vendor is its
  first caller.
- **An authored `offset` on an NPC.** design.md refuses it, matching monsters,
  because refusing is the reversible default: allowing it later is additive,
  withdrawing it is not. If authoring wants a merchant nudged behind a counter,
  that is a ruling to make then, with the case in hand.

## Open, and not blocking

Whether the door into the dungeon proper is **locked** or merely closed is an
authoring choice, not a code one — locked-with-a-DC already exists. A locked
front door makes the vestibule a deliberate threshold rather than a hallway.
