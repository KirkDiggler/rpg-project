# Hostility as a relation, and the world member — the placement seam

Normative. Reasoning and rejected options live in [brainstorm.md](brainstorm.md).

Parent: **rpg-project#311** (World NPC Foundation).

## The mistake this corrects

`MemberKind` conflates two axes: **control** (who decides a member's actions) and
**allegiance** (who it is hostile to). Today `sidesInContactOrder` computes
"sides" by partitioning on `Kind`, and it is the only place sides are computed.

That works only while three coincidences hold: exactly two sides, kinds mapping
1:1 onto sides, and no intra-kind hostility. Every near-term case breaks one —
a three-way fight, city guards fighting beside the party, a shopkeeper who turns
on a caught pickpocket.

**The structural reason `Kind` cannot do this job: hostility is a fact about a
PAIR, and `Kind` is a property of ONE member.** "Are these two enemies" is not
answerable by looking at either alone.

This is not speculative. **rpg-toolkit#899** ("Opportunity attacks ignore
hostility: allies OA allies, monsters OA monsters") and **rpg-toolkit#766**
("reaction attack fires between ALLIES during FREE_ROAM — missing hostility
check") are both open, and both are this defect.

## Scope

Introduce **the question, not the answer**: one predicate that owns hostility.
Today it returns exactly what the engine already computes; later it reads
factions and intel. Then place a world member — a merchant — that is hostile to
nobody.

**This slice gives the merchant no actions.** Capabilities, the interaction verb
and adjacency are **FadedPez's** (Kirk, 2026-08-28), via rpg-project#310 and
[rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275).

## 1. The predicate — `rulebooks/dnd5e/encounter`

```go
// hostile reports whether a and b are enemies right now.
//
// Hostility is a fact about a PAIR, which is why it cannot live on MemberKind:
// a kind describes one member, and no property of one member answers "are these
// two enemies". A METHOD rather than a free function because the answer will
// grow to depend on world state the pair does not carry — factions, and what a
// side BELIEVES (rpg-project#305/#306; the guards who turn on the party they
// think killed the king).
func (e *Encounter) hostile(a, b MemberID) bool
```

1. `hostile` **MUST** be the only place the engine decides whether two members
   are enemies.
2. Its first implementation **MUST** reproduce current behaviour exactly — one
   side `KindPlayer`, the other `KindMonster` — so this slice is a **refactor
   with zero behaviour change**, provable by the existing suite.
3. Its signature **MUST NOT** foreclose reading encounter state. It is a method
   for that reason, and **MUST NOT** be reduced to a pure function of two
   members.
4. These sites **MUST** be converted to ask it instead of switching on `Kind`:
   - `encounter/trigger.go` `sidesInContactOrder` / `classify` — which pairs form or join a bubble
   - `encounter/standing.go` `fightIsDecided` — whether two mutually hostile standing sides remain
   - `session/attack.go`, two sites — a player's legal targets
   - `behavior/basic.go` — a monster's candidate targets
5. `hostile` **MUST** be symmetric in this slice, and any future asymmetry
   **MUST** be a deliberate ruling rather than an accident of implementation.
6. Factions **MUST NOT** be built here. The predicate is the named attachment
   point; it stays empty until a real multi-faction case arrives.

## 2. The kind — `KindWorld`

With allegiance moved to the predicate, `Kind` goes back to meaning what it says
and the third value stops carrying harmlessness.

```go
// KindWorld is a member who belongs to the PLACE rather than to a person or to
// a fighting side: a merchant, a trainer, a quest-giver.
//
// It says where a member BELONGS, and deliberately NOT whether it is dangerous.
// A shopkeeper who turns on a caught pickpocket is still KindWorld; what
// changed is the hostile() relation, not what he is.
KindWorld MemberKind = "world"
```

7. The godoc **MUST** state that the kind describes belonging and **not**
   harmlessness, and **MUST** carry the turned-shopkeeper example.
8. `Kind` **MUST NOT** be changed by this slice. This is a **scope statement,
   not an invariant** — nothing in the engine assigns `Kind` after construction
   today, and this slice **MUST NOT** add a rule forbidding it, because a future
   model may legitimately want to.
9. A world member's non-hostility **MUST** come from `hostile` returning false,
   **never** from a kind switch omitting it. The distinction is the whole point:
   the first is a true statement, the second was luck.
10. It **MUST** be visible, hold a position, appear in sight percepts, and
    render like any other member.
11. It **MUST** occupy its cell as any member does, so it blocks movement as an
    occupied tile (rpg-project#311) — a consequence of membership, not a
    declared property.
12. It **MUST NOT** roll initiative or take a turn. This is **derived, not
    declared**: turns come from bubble membership, bubbles form from hostile
    pairs, and a member hostile to nobody is in none.

## 3. The ref — `rulebooks/dnd5e/refs`

13. `module.go` **MUST** gain `TypeNPCs core.Type = "npcs"` beside
    `TypeMonsters`.
14. `dnd5e:npcs:merchant` **MUST** exist (Kirk's ruling). `MemberKind` answers
    what a member is in a fight; the ref type names a content bucket beside
    `dnd5e:props:pillar`. The compiler is the one place the two vocabularies
    meet and **MUST** say so in a comment.
15. **`merchant` is a TYPE, not a look.** One ref **MUST** be renderable as
    several GLBs, per the existing `MONSTER_REF_MODELS: Record<string,
    string[]>` + `pickStableCandidateIndex` pattern and rpg-dnd5e-web#559's
    ruling that art-to-ref is not 1:1. That candidate array is **positionally
    indexed**, so its order is load-bearing and **MUST NOT** be reshuffled.

## 4. The placement — `encounter/dungeonspec`

16. A ref of type `npcs` **MUST** compile to a member with `Kind == KindWorld`.
17. The placement **MUST** accept `facing` — one of the eight true-compass names,
    validated as props' — because a world member never turns in play, so the
    authored value is the only one there will be.
18. It **MUST** be refused `blocks_movement`, `blocks_los`, `targeting`, `boss`
    and `offset`, each by name.
19. One-placement-per-cell already holds and is unchanged.

## 5. The wire — `rpg-api-protos`

20. `MEMBER_KIND_WORLD = 3` **MUST** be added to the existing `MemberKind` enum.
    `Member.kind` is already on the wire: no new field, no new message.
21. Hostility **MUST NOT** reach the wire in this slice. What a client may know
    about who hates whom is a perception question and is not answered here.
22. The proto **MUST** merge before toolkit, api, or web build against it.

## 6. The builder — `rpg-dnd5e-web/src/author`

The palette applies a **ref-AND-GLB test**: placeable only if a toolkit ref *and*
a promoted GLB both exist.

23. `PaletteCategory` **MUST** gain `npcs`, and the palette **MUST** offer every
    `npcs` ref passing that test — verified by resolution, not asserted.
24. The `place` tool **MUST** write an `npcs` placement and `dungeonYaml.ts`
    **MUST** round-trip it.
25. The Inspector **MUST** offer `facing`, and **MUST NOT** offer `offset`,
    `blocks_movement`, `blocks_los` or `boss`.
26. The renderer **MUST** resolve an `npcs` ref to one of its candidate models
    the way `monsterModels.ts` does.

## 7. Assets — Assets lane, not this slice

27. A promoted merchant GLB **MUST** exist before the builder can offer one.
    Filed under the journey with Team **Assets**.
28. Source art already exists: `SK_Chr_Merchant_01`, converted to
    `assets/synty/converted/polygon-fantasy-kingdom/SK_Chr_Merchant_01.glb`. The
    Assets task is a **promotion, not new art**.
29. `public/models/synty/npcs/` already holds MONSTER art from the
    polygon-dungeon wave, seven models on one 55-joint armature. **It is a
    promotion-wave bucket, not a taxonomy**, and rpg-dnd5e-web#559 already puts
    identity in each entry's `rulesRef`, not the path. Merchant art **MUST NOT**
    be filed there merely because the ref type is `npcs`. Renaming it is out of
    scope.
30. Separately filed defect: that manifest's `monk` entry carries
    `rulesRef: null` **and** `rulesRefNote: null` — the silent gap #559 called
    unacceptable.

## 8. The room — authoring only, no code

31. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
32. `start` **MUST** be a cell inside it, and a `DoorSpec` **MUST** separate it
    from the first chamber.
33. Whether that door is locked is authoring, and **MUST NOT** become a rule.

## Shelves — named, deliberately empty

- **Factions.** `hostile`'s eventual data source. Nothing built until a
  multi-faction fight is real.
- **Belief-driven allegiance.** `play/intel` and `SurveilOutput` already exist,
  and rpg-project#305/#306 are "what a monster knows". The guards who turn on
  the party they *believe* killed the king read from there. `hostile` being a
  method is what keeps this reachable.
- **Control as its own axis.** `monster` still conflates AI-control with
  fiction, so an allied city guard would today be a `KindMonster` hostile to
  nobody on the party's side — functionally correct, semantically odd. Fixable
  later with no behaviour change; **not** fixed here.
- **Asymmetric hostility.** An ambusher hostile to a party that is not yet
  hostile back.

## Handoff — what FadedPez attaches to

A member with `Kind == KindWorld` at an authored cell and facing, addressable by
`MemberID`, reaching the client as `MEMBER_KIND_WORLD`, and hostile to nobody by
`hostile`'s answer rather than by omission.

Capabilities (`TALK`, `VENDOR`, `TRAINER`, `QUEST_GIVER`, `QUEST_TARGET` are
#311's suggestion, not this document's ruling), the interaction verb, its wire
shape, and whether interaction requires adjacency are his. **Note:** the existing
door `Interact` has no adjacency check, so #311's "adjacent player" is either a
new rule or a shared correction to both.

## Out of scope

Factions; intel-driven hostility; fixing rpg-toolkit#899 and #766 (the predicate
makes them a one-function change, but this slice **MUST NOT** change behaviour);
actions, capabilities, the interaction verb, adjacency; vendor stock, wallets,
buying, selling; dialogue; quests; AI, turns, damage or death; region typing;
renaming the existing art directory.

## Acceptance

- `hostile` is the only place the engine decides enmity, and the four converted
  sites ask it.
- The existing suite passes **unchanged**, demonstrating zero behaviour change.
- A dungeon authored with an `npcs` placement compiles; the member appears at its
  authored cell and facing with `Kind == KindWorld`.
- A player walks into line of sight of it and **no bubble forms** — because
  `hostile` says so, verified by a test that asserts the predicate, not the enum.
- A player in a running fight cannot name it as an attack target.
- A monster in a running fight never selects it as a target.
- A fight in which every monster is downed ends, with it still standing.
- It takes no turn and never appears in an initiative order.
- A player cannot walk through its cell.
- The builder offers the merchant in an `npcs` palette category, places it, the
  YAML round-trips, and one ref resolves to several candidate looks.
- `Member.kind` reaches the client as `MEMBER_KIND_WORLD` and it renders.
