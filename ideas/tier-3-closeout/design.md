# Tier 3 close-out — the composable encounter review, finished

Tracking: [rpg-toolkit#1965](https://github.com/KirkDiggler/rpg-toolkit/issues/1965)
tier 3, and items 2–4 of
[rpg-toolkit#2018](https://github.com/KirkDiggler/rpg-toolkit/issues/2018).
Walkthrough: [README.md](README.md). Slices: [slices.md](slices.md).

## Shape

```mermaid
flowchart LR
    subgraph gate[before any slice runs]
        R["R1–R9<br/>one-word answers"]
    end
    subgraph toolkit[rpg-toolkit, one module per PR, inside-out]
        ROOT["1 · dnd5e root<br/>dead methods · second identities<br/>stored free reactions · retired set<br/>target lines name their owner"]
        ENC["2 · encounter<br/>Participation · Conditions fields<br/>Search and discovery import gone<br/>one journal key · refusal codes"]
        RES["3 · resolution<br/>Standing gone · one hit decision<br/>actions like monsters · names not refs"]
        SES["4 · session<br/>Search verb gone · discovery required<br/>one down answer · its own sentinels"]
    end
    subgraph api[rpg-api dev]
        API["5 · carry<br/>pins · sdkerr arms<br/>v1alpha1 names from the registry"]
    end
    RESET["reset at merge-out<br/>character · session · session-enc · session-seat"]
    R --> ROOT
    R --> ENC
    ROOT --> RES
    ENC --> RES
    RES --> SES --> API --> RESET
```

Closure is a deletion wave. Every line of the review's tier 3, every tier 3
item its comments added, and items 2–4 of rpg-toolkit#2018 gets exactly one
disposition below. Nothing here adds a capability; where a deletion forces a
behaviour choice, the choice is a ruling the operator answers before any slice
runs.

## Law

### What closure means

- **Closure leaves no fail-silent default, no second identity for one fact,
  no dead code the review named, and no reader of a stored shape no build
  writes.** Each of the four is a place a future change lands in the wrong
  copy.
- Pre-release data resets rather than migrates (A6). A reader whose only job
  is an old stored shape is deleted, and the reset at merge-out removes the
  data it read.
- A deletion that changes what a player can observe carries one walk box. A
  deletion of dead code carries none; its proof is the build and the tests.
- A count pin in a test file a slice already edits becomes a membership or
  identity assertion. A count is never bumped.

### The audit

Dispositions: **gone** (already deleted, naming the module tag that did it),
**delete** (pure deletion or mechanical cleanup, no behaviour choice),
**ruling** (a behaviour choice; see Rulings), **out** (not closure work).
Audited at rpg-toolkit dnd5e v0.211.0 · encounter v0.125.0 · resolution
v0.70.0 · session v0.130.0 and rpg-api `dev`.

| # | Item | Disposition |
|---|---|---|
| 1 | Binary `Standing` (encounter interface, resolution entry, `ErrNoParticipation`, session helper) | delete |
| 2 | Deprecated `Search` verb and `ErrSearchRetired`, `ErrElsewhere` | delete |
| 3 | Discovery mode chosen by type assertion and a nil `Explorations` | ruling R1 |
| 4 | Exploration memory stored twice: the session copy is gone (session v0.121.0); the encounter's explicit import survives | delete |
| 5 | `TurnDriver` alias | gone (encounter v0.120.0) |
| 6 | `Equipment` capability field that also carries `Conditions` | delete |
| 7 | `Character.MakeSavingThrow` and its input | delete |
| 8 | `Character.SpendHitDice` | gone (dnd5e v0.206.0) |
| 9 | `Character.ShortRest` | out: live, `resolution/short_rest.go:220` calls it |
| 10 | `Character.Cleanup` | delete |
| 11 | `StructurallyPermanentConditionRefs` | gone (dnd5e v0.210.0) |
| 12 | `monstertraits.AllTraitRefs` | gone (dnd5e v0.210.0) |
| 13 | `conditions/types.go` | gone (dnd5e v0.210.0) |
| 14 | `ConditionAppliedEvent.Type` and the `ConditionType` constants | delete |
| 15 | `loadedEffect` and the routed-ref use of `peekEffectRef` | delete |
| 16 | `HeldAddresses` free-reaction merge | ruling R2 |
| 17 | `Data.ClassResources` | gone (dnd5e v0.206.0) |
| 18 | `DecodeSightPayload` and the `SightPayload` type | delete |
| 19 | `events.ChildRef` alias | delete |
| 20 | `resolution.AttackFacts`: characters project one main-hand attack, monsters every definition | ruling R3 |
| 21 | Strike decides the hit twice | delete |
| 22 | Empty `afterDamage` step (and its pass-through `afterNotify`) | delete |
| 23 | Unknown Unarmored Defense variant defaults to barbarian | delete: tier 2 A removed the create-path default; the load path still reads an unknown or empty variant as barbarian |
| 24 | `GetSpeed` answers 30 for an unknown race | ruling R4 |
| 25 | `NobodyPerceives` installed beside a real `Sight` on the read path | delete |
| 26 | `CompileOnlySetup` passes where `CompileOnlyLoad` refuses | gone (encounter v0.120.0) |
| 27 | Two attack offers surface as `ShortfallUnreadable` / `ErrBadAttack` | ruling R5 |
| 28 | "No side" encoded two ways | gone (encounter v0.117.0) |
| 29 | One journal saved under two keys, losing interleaving | delete |
| 30 | Refusal codes: `ErrNoMember` for non-empty ids, duplicates and authored defects; missing `encounter:` prefixes; sentinel-less roll-trace errors; session maps both member sentinels to one | ruling R6 |
| 31 | `ErrOutOfRange` vs `ErrOutOfReach` for one refusal | gone (encounter v0.117.0) |
| 32 | `Len(…, N)` count pins across resolution, encounter, session, character | out: a separate mechanical sweep; slices convert pins only in files they already edit |
| 33 | `ConditionAddress` prose calls an empty `SourceID` "legacy unqualified" | delete |
| 34 | resolution AGENTS.md says a runtime area belongs to nobody | gone (resolution v0.65.0) |
| 35 | dnd5e CLAUDE.md cites `character.HasCondition` | delete |
| 36 | Stale root prose | delete |
| 37 | Case-file narration in the display catalog | gone (dnd5e v0.210.0) |
| 38 | Case-file narration in the compelled driver's doc | delete |
| 39 | Guiding Bolt loader error says "sanctuary immune" | gone (dnd5e v0.210.0) |
| 40 | Q1: is a monster trait a condition | gone (A2, dnd5e v0.210.0) |
| 41 | Q2: a condition that never subscribes | gone (A3, dnd5e v0.210.0) |
| 42 | Q3: who owns the stored AC | gone (dnd5e v0.204.0) |
| 43 | Q4: a ward-on-attempt primitive | out: one ward exists; a second ward brings the mechanism |
| 44 | Q5: untagged sight testimony read as known; `PerceptionSubjects == 0` upgraded in place | delete |
| 45 | Q5: room-local dialect refused loudly | out: a required field refusing its absence is validation, not a reader |
| 46 | Q6: Shield | gone (dnd5e v0.210.0) |
| 47 | Q7: `PausedTurn` / `HeldDirective` as one sum type | gone (encounter v0.121.0) |
| 48 | Q7: the v2 map format and its "legacy" `PropInput` | ruling R8 |
| 49 | Q7: monk Unarmored Defense while holding a shield | ruling R9 |
| 50 | Q7: a saving-throw frame kind with no producer | gone (resolution v0.63.0: `contestDamageFrame` produces it) |
| 51 | Q7: which rolls carry a presentation id | out: decided per verb by the client that renders it; a use case brings the rule |
| 52 | `combat` exports the effective damage multiplier | gone (dnd5e v0.205.0: `SettleDamage`) |
| 53 | A target-answer trace line carries `sourceId: ""` | delete |
| 54 | Session `Arrival` and `ArrivesAt*` / `ArrivesOn*` | delete |
| 55 | `UnconsciousCondition` compatibility shell | delete |
| 56 | Rage converges on a life-state condition | out: nothing goes down without damage; Sleep brings the primitive |
| 57 | Sanctuary modelled as concentration | out: a behaviour change with its own ruling, not a deletion |
| 58 | `gamectx.Cast.StanceBetween` has no reader | delete |
| 59 | Two Launch refusals flatten their inner sentinel | out: session's translate law carries session's sentinel alone and keeps the inner message as text |
| 60 | #2018-2: resolution prints a full ref where a reader wants the name | delete |
| 61 | #2018-3: rpg-api v1alpha1 keeps its own condition display names | delete (the enum mapping stays: it is a wire contract) |
| 62 | The retired condition set (a reader of old blobs, from tier 2 A) | ruling R7 |
| 63 | Lenient `character.LoadFromData` drops what it cannot read | out: not on the review's list; filed as a follow-up |

#2018 item 4 is rows 14, 15 and 16.

Counts: 19 gone, 26 delete, 9 ruling, 9 out. Each row's file and line live in
[slices.md](slices.md), "Where each row lives".

### Per-module deletion plans

**dnd5e root.** Rows 7, 10, 14, 15, 19, 23, 33, 35, 36, 53, 55, 58, plus R2, R4,
R5's sentinel, R7 and R9 as ruled. `refs.Conditions.Unconscious` goes with
the shell when no toolkit or rpg-api reader remains. `refs.Conditions.InFog`
stays: Fog Cloud's area membership names it. The `ConditionType` constants go
with the field: nothing else reads them. Unarmored Defense's load refuses a
missing or unknown variant, and `SecondaryAbility` has no default, so the
law the condition descriptor states for creation holds for load too.

**encounter.** Rows 1, 2, 4, 6, 18, 29, 44, the encounter half of row 25, plus
R1, R6 and R8 as ruled. `Capabilities.Standing` becomes
`Capabilities.Participation` typed `Participation`; `Capabilities.Equipment`
is typed `Equipment` beside a new required `Capabilities.Conditions`.
`NobodyPerceives` becomes unexported and serves only `RefusingCapabilities`; a
new `RefusingWitness` serves a host that reads and never refreshes sight.

**resolution.** Rows 1, 19, 20 as ruled, 21, 22, 58, 60, plus R5's sentinel.
The hazard prose on `afterDamage` (publishing `DamageReceivedEvent` applies
damage twice to a monster) moves to `doc.go`, where it already stands.

**session.** Rows 1, 2, 25, 38, 54, plus R1, R3, R5 and R6 as ruled.
Session keeps its own sentinels and maps each encounter sentinel to one; a
translated error carries session's sentinel alone.

**rpg-api.** Row 61, the sdkerr arms rows 2, R5 and R6 move, the reference
content of row 4, and the pins. Under R1, the `AutomaticDiscovery` switch goes:
the server always wires an exploration repository and the shared-store
locker, and the two acceptance tests that call `Manager.Search` reveal by
approach instead.

**rpg-api-protos.** None. The deprecated `Search` RPC stays a tombstone;
R5 needs no enum value.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| A6 | settled | Pre-release character data is reset, not translated. Applied here to every stored shape the deletions stop reading. | KirkDiggler | 2026-10-11 |
| R1 | open | With `Search` deleted, automatic discovery is the only mode: `session.Config.Explorations` is required (`ErrIncompleteConfig` at construction) and `DiscoveryCheckResolver` folds into `CheckResolver`. Recommended: yes. | — | — |
| R2 | open | A new sheet carries its free reactions in its stored conditions from creation (Finalize and the monster mint write them; Attach applies what is stored and adds nothing), so `HeldAddresses` stops merging. Recommended: yes. | — | — |
| R3 | open | `ProjectCharacter` answers every attack definition the character can make, projected by the one function monsters use; `MainHand` and `AttackFacts` go. Changes no offer and nothing on the player's sheet. Recommended: yes. | — | — |
| R4 | open | Loading a sheet refuses a race id the catalogue does not hold, so `GetSpeed` has no default. Recommended: yes. | — | — |
| R5 | open | Two attack-assembly offers for one swing reach the host as `ShortfallUnavailable` with sentinel `ErrConflictingOffers` (FailedPrecondition), never as unreadable or `ErrBadAttack`. Recommended: yes. | — | — |
| R6 | open | One meaning per refusal code: `ErrNoMember` only for an empty id, `ErrNotMember` only for an id naming nobody, `ErrDuplicateMember` for a member already present, `ErrBadMember` for an authored member the encounter cannot seat; every encounter sentinel's text starts `encounter:`; roll-trace refusals wrap `ErrInvalidData`; session maps each to its own sentinel. Recommended: yes. | — | — |
| R7 | open | The retired condition set is deleted; a blob of a removed type is `ErrUnknownCondition` like any unregistered ref. If accepted, it amends the condition descriptor's error posture and removed kinds; that amendment is staged in `ideas/condition-descriptor/` in this PR. Recommended: yes. | — | — |
| R8 | open | The v2 map format stays: it is what the web builder authors and what five rpg-api reference dungeons use. `PropInput` loses the word "legacy" in its docs. Recommended: stays. | — | — |
| R9 | open | A monk holding a shield loses Unarmored Defense; a barbarian keeps it. Recommended: yes. | — | — |

What each recommended answer makes true here:

- **R1.** Row 3 is a deletion: no type assertion picks the mode and no nil
  repository means "no discovery". Cost: every session test `Config` wires an
  exploration fake, and a host without one cannot start.
- **R2.** Row 16 is a deletion. Cost: two creation paths write the reaction;
  test fixtures that hand-build a sheet without it read it absent. It holds
  only after the reset.
- **R3.** One projection of actions for both sheet kinds. **What a player
  sees does not change.** The attack offer is compiled in session's offers
  from `character.AssembleAttack` on the main hand alone, today and after R3,
  and never reads the projection. The projection's actions feed only the
  sheet facts encounter reads for a driven member: its reach, its driver
  view and its creature-table predicates. After R3 a character driven by a
  compulsion has both hands' attacks in that view. Cost: resolution's output
  field changes shape, and rpg-api's character view calls `ProjectCharacter`
  twice, so an off-hand weapon that does not compile becomes a new way for
  `GetCharacter` to fail, beside the main-hand failure it has today.
- **R4.** Row 24 is a deletion. Cost: fixtures with an empty race id fail to
  load.
- **R5.** The player owing a pick is told so. Cost: one sentinel each in root,
  resolution and session, and one rpg-api arm. The alternative, a new
  shortfall enum value, is an additive proto.
- **R6.** `ErrBadMember` also answers `Transfer`'s world-NPC guard, which
  means the same thing at runtime: a member the encounter cannot seat in a
  fight. Its one caller is encounter's own classification, which never hands
  it a world NPC; were it reached, session's translate turns it into
  `ErrInvalidWorld`, which is true of a stored world holding that seat. Cost:
  about twenty call sites, rpg-api arms, and tests that pinned `ErrNoMember`
  on those paths.
- **R7.** If accepted, the condition descriptor's law reads: a removed
  type's ref is unregistered, and a stored blob naming one fails `LoadJSON`,
  `HeldAddresses` and `AssessTargetHeldEffects` with `ErrUnknownCondition`;
  no list of removed refs stands beside the registry. If refused, this PR
  restores that design's retired-set text before it merges. Cost: the reset
  at merge-out runs again on every store, including the deployed one, which
  the operator runs or approves.
- **R8.** Cost: none now. Retiring v2 is a content and builder wave.
- **R9.** Cost: one branch on `HasShield`, one test, one walk box.

## Open

- R1–R9 await the operator. A slice whose rows name an open ruling does not
  start; the other rows of that slice may.
