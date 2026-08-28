# Hostility, world members, and the room before the dungeon

The WHY, including the wrong turns. Normative shapes live in [design.md](design.md).

## The ask

Kirk, 2026-08-28: a room *before* the main dungeon start, entered through a real
door, with a merchant standing in it. Not a second zone — a region of the same
dungeon. The merchant is an NPC and not hostile. It happens in free roam. The
destination is [rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275).

Parent: **rpg-project#311**. Sibling: **rpg-project#310**. FadedPez takes the
merchant's actions; this slice makes a space for one to be placed.

## The first reading, and why it was wrong

The initial finding was that `MemberKind` is a two-value enum whose every
hostile-path site switches on it **exhaustively with no default arm**, so a third
value inherits four behaviours with none of those sites edited:

| Behaviour | Decided in |
|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` |
| Not attackable by players | `session/attack.go`, two sites |
| Ignored by monster AI | `behavior/basic.go` |

**Every line of that is true, and it was still the wrong foundation.** The
observation is real; the conclusion — "therefore add a third kind and take the
four behaviours for free" — was building on an accident. A world member would
have been non-hostile because four switches happened to omit it, not because
anything anywhere said it was harmless. That is luck wearing the costume of
design, and it only holds while no world member is ever hostile to anyone.

## How it came apart, in Kirk's order

**1. "Having monsters in the npcs category feels off."** The name `npc` defines
the kind by NEGATION — "not a player" — which monsters satisfy too, so the
category had no edge. Renamed to `world`: *belongs to the place*, positive rather
than negative, and matching #311's own title. The test that settled it was the
hired mercenary: an ally who fights is not a player and not a monster, so `npc`
would have welcomed it in and it would have broken all four behaviours.

**2. "Possibly an npc could become an enemy — caught pickpocketing."** This broke
the immutability rule the design had just written. If a merchant can turn, then
`world` describes where he belongs, not whether he will fight you — a shopkeeper
who catches a thief still belongs to the place. Kirk's verdict on the overreach
was exact: *"I think we made some real calls treating it like something it may or
may not be."* The offending rule (`Kind` MUST NOT change at runtime) was an
invented invariant: nothing in the engine assigns `Kind` after construction, so
the door was merely closed, and the rule would have bolted it.

**3. "Using kind to denote the side of a fight feels off."** The one that found
the actual mistake. Kirk's cases: a three-way fight with two monster types; city
guards fighting beside the party; those guards turning once they believe the
party killed the king.

## The synthesis

`MemberKind` conflates **control** (who decides a member's actions) with
**allegiance** (who it is hostile to). That works only while three coincidences
hold — exactly two sides, kinds mapping 1:1 onto sides, no intra-kind hostility —
and each of Kirk's cases breaks one.

**The structural reason it can never work: hostility is a fact about a PAIR, and
`Kind` is a property of ONE member.** "Are these two enemies" cannot be answered
by looking at either alone. It is a type error, not a matter of taste — which is
why it kept feeling off from three different directions.

**And it is already a bug, twice filed.** rpg-toolkit#899 (*"Opportunity attacks
ignore hostility: allies OA allies, monsters OA monsters"*) and rpg-toolkit#766
(*"reaction attack fires between ALLIES during FREE_ROAM — missing hostility
check"*) are both open, and both are this defect. A grep of `rulebooks`, `play`,
`tools` and `core` finds **no faction, allegiance or hostility concept anywhere**;
"side" exists only in comments, and `sidesInContactOrder` is the single place
sides are computed.

Kirk's cases against a relation model, all of which work:

- **Three-way fight** — three factions, pairwise hostility.
- **Allied city guards** — AI-driven *and* on your side. **Impossible to express
  today**: AI-controlled means `KindMonster` means hostile to all players. Not a
  corner case; it is a party with an NPC fighter in it.
- **Guards who turn on a believed regicide** — one relation flips, nothing is
  re-typed. The *belief* half already has a home in `play/intel` /
  `SurveilOutput` and rpg-project#305/#306.
- **The pickpocketed merchant** — the same mechanism as the guards, which is the
  tell that it is the right one.

## The recommended shape — for Pez, NOT this slice

One predicate, `(*Encounter).hostile(a, b)`, owning enmity. A first
implementation returning exactly what the engine already computes makes the
conversion a **refactor with zero behaviour change**; the four sites ask it
instead of switching on `Kind`.

**A METHOD, not a free function.** Kirk: *"dynamic and based on intel, possibly
it could change."* A pure function of two members could never consult factions or
belief, so the guards case would be foreclosed on day one by a signature. This is
the single most useful thing in this document for whoever builds hostility.

What it would buy: a merchant hostile to nobody as a **stated fact** rather than
an omission; rpg-toolkit#899 and #766 collapsing into a one-function fix instead
of four-site archaeology; factions as a named attachment point rather than a
rewrite.

### Why it is not in this slice

Kirk, after the design had been rewritten around it: *"So I think we want to make
minimal changes. FadedPez will take this work, we only want to be able to place a
KindWorld npc into the dungeon that can exist on a free roam clock that is not
hostile to us. Everything else will be managed by Pez. I do not want to
overreach."*

Correct, and it is the second overreach in this thread — the first was inventing
a `Kind` immutability rule, the second was reaching for the hostility refactor
because the analysis behind it was satisfying. **The analysis being right does not
make it ours to build.** Hostility belongs to whoever owns what an NPC does, and
that is Pez.

So the slice reverts to adding a value the hostile-path switches do not name, and
changing none of them. That yields non-hostility and the free-roam clock with
zero engine edits — the world clock follows because a member is moved to a turn
clock only by entering a bubble, and bubbles form only from hostile pairs.

**The honest caveat, stated in design.md rather than buried:** that is true today
and is not a guarantee. It holds because nothing hostile is authored as
`KindWorld`. The pickpocket case is what turns it from luck into a thing someone
has to state — and that is the moment the predicate above earns its keep. The
design's job is to not foreclose it, which is why no immutability rule is
written.

## Rejected

**Region typing (entrance / chamber / corridor).** `RegionSpec` v2 removed region
kinds deliberately — v1 *derived* `start` from an archetype, which the spec names
as "the shape of defaulting rpg-toolkit#1033 forbids". `Archetype` is ruled
presentation-only (rpg-project#256). Withdrawn once the boss case dissolved.

**A region type for the boss room.** `PlaceSpec.Boss` already exists, validated
one boss per region. The boss is a placement fact, not an area fact.

**Disposition as an enum value beside player/monster.** Considered and argued
against early, on the grounds that the axis answers *what a member is*. That was
right about the axis and wrong about the conclusion: allegiance is not a third
value on that enum, it is a different structure entirely.

**Merchant placed as a monster.** `classify` pairs players against monsters, so
it forms a bubble the instant anyone sees him.

**A separate zone for the shop.** The point of putting the room in the dungeon
spec is avoiding a second world concept, its load path and its camera rules.

**Renaming `public/models/synty/npcs/` to `monsters/`.** Its manifest shows it is
the polygon-dungeon promotion wave — seven models on one 55-joint armature — not
a taxonomy, and rpg-dnd5e-web#559 already put identity in each entry's
`rulesRef` rather than the path. Renaming would swap one aspirational label for
another and rot immediately, since the merchant comes from the same Fantasy
Kingdom family the class models did.

## Rulings (Kirk, 2026-08-28)

0. **Minimal changes.** Place a `KindWorld` member that exists on a free-roam
   clock and is not hostile to the party. Everything else is Pez's. *"I do not
   want to overreach."*
1. **Hostility as a predicate is a RECOMMENDATION, not this slice** — dynamic,
   able to grow to read intel, and a method so that stays possible.
2. **The merchant cannot be attacked** — *"definitely not a launch goal"*.
   Achieved by the hostile-path switches not naming the new value, with the
   caveat above recorded rather than hidden.
3. **The kinds are `player`, `monster`, `world`**; the ref type is `npcs`.
4. **World members carry an authored `facing`** — they never turn in play.
5. **`merchant` is a type, not a look** — one ref, several GLBs, which the
   existing `MONSTER_REF_MODELS` + `pickStableCandidateIndex` pattern already
   supports.
6. **`bystander` is a role within the kind**, not a kind.
7. **Actions are FadedPez's** — and so is hostility; this slice is placement
   only.

## Shelves left empty

- **Factions** — `hostile`'s eventual data source.
- **Belief-driven allegiance** — reading `play/intel`; kept reachable purely by
  `hostile` being a method.
- **Control as its own axis** — `monster` still conflates AI-control with
  fiction, so an allied city guard is today a `KindMonster` hostile to nobody on
  the party's side: functionally correct, semantically odd, fixable later with no
  behaviour change.
- **Asymmetric hostility** — an ambusher hostile to a party not yet hostile back.
- **Everything an NPC can be asked** — capabilities, the interaction verb, its
  wire shape, adjacency. FadedPez's, deliberately unspecified here.
- **An authored `offset` on a world member** — refused for now because refusing
  is the reversible default.

## What "a space to be placed" costs

More than it sounds. `paletteData.ts` applies a **ref-AND-GLB test**: the builder
offers a thing only when a toolkit ref *and* a promoted GLB both exist, and it
documents refs excluded for failing either half. So placing a merchant reaches
into a `TypeNPCs` constant and a ref, promoted art, a model-resolution path
beside `monsterModels.ts`, and the palette/place/inspector work.

**The art already exists**: `SK_Chr_Merchant_01` ships in the Synty Fantasy
Kingdom pack and is already converted to GLB under `assets/synty/converted/`. The
Assets task is a promotion through the same pipeline that promoted the four
player classes — not a hunt for new art.

## Open, and not blocking

Whether the door into the first chamber is **locked** or merely closed is an
authoring choice, not a code one. A locked front door makes the vestibule a
deliberate threshold rather than a hallway.
