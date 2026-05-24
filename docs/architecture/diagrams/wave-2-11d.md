# Wave 2.11d — reaction surface diagrams

> Sequence diagrams for the new flows Wave 2.11d shipped. Read `../system.md` first for repo + boundary context.

## What this wave added

**Goal sentence (scope-adjusted)**: orchestration plumbing (PhasedCombatResolver SDK + Dnd5eCombatResolver impl), readiness toggle infrastructure (SetReactionReady RPC + per-character UI), Shield + OA condition predicate verification (toolkit unit tests + rpg-api `NotReady_NoPrompt` integration test), publish-before-save + single-reactor enforcement fixes, `cmd/devseed` real-character seed pipeline, interactive visual playtest harness.

**What's NOT verified end-to-end yet** (deferred to Wave 2.11e):
- Shield-fires-and-blocks-NPC-attack (needs `CompleteTakeAction` NPC-attacker symmetry — rpg-toolkit#662)
- Player-OA-on-movement (needs `MovementResolver` SDK extension — rpg-toolkit#658)

## Flow 1: Shield-not-ready (the path that DOES work end-to-end)

The negative-case path — proves the readiness gate works. NPC attacks wizard, wizard's Shield is NOT readied, no prompt fires, attack hits inline.

```mermaid
sequenceDiagram
    participant W as web (wendy tab)
    participant API as rpg-api EndTurn handler
    participant SDK as encounter SDK
    participant NPCAct
    participant DR as Dnd5eCombatResolver
    participant TK as toolkit combat
    participant R as Redis
    participant Broker

    Note over W: it's the goblin's turn
    W->>API: EndTurn(previous actor)
    API->>R: GET enc:v2:dev-encounter
    API->>SDK: LoadFromData + applyReactionConditions
    Note over SDK: Shield Apply()'d to wendy<br/>(hasFirstLevelSpellSlot=true<br/>after rpg-toolkit#660 fix)
    SDK->>NPCAct: NPCAct(goblin-1)
    NPCAct->>NPCAct: installTriggerBuffer<br/>(for ReactionTriggerEvents)
    NPCAct->>DR: ResolveAttackHit(attacker=goblin, target=wendy)
    DR->>TK: combat.ResolveAttackHit
    TK->>TK: AttackChain → roll d20 → wouldHit=true
    TK->>TK: PostAttackRollChain publishes
    Note over TK: Shield's predicate runs<br/>predicate: target=wendy ✓<br/>wouldHit ✓<br/>+5 deflects ✓<br/><b>IsReactionReady?</b> ❌ NO<br/>(readiness=false)
    Note over TK: predicate returns false<br/>no ReactionTriggerEvent published
    TK-->>DR: AttackContext{hit=true}
    DR-->>NPCAct: AttackContext
    NPCAct->>NPCAct: drain trigger buffer → empty
    NPCAct->>DR: ApplyAttackOutcome(modifiers=[])
    DR->>TK: combat.ApplyAttackOutcome
    TK-->>DR: AttackResult{damage=4, AttackResolvedEvent, DamageDealtEvent}
    NPCAct->>Broker: publish AttackResolved + DamageDealt
    Broker-->>W: stream: AttackResolved, DamageDealt
    Note over W: wendy HP 8→4
    NPCAct->>SDK: write-back wendy character state
    SDK->>R: SAVE enc:v2:dev-encounter
    SDK->>R: SAVE character:char-wendy
```

Verified by integration test `TestPlayerShield_NotReady_NoPrompt` in `internal/handlers/dnd5e/v2/encounter/integration_player_shield_test.go` (rpg-api #542).

## Flow 2: Shield-ready (the path Wave 2.11e completes)

The path Wave 2.11d's predicate validates but the SDK can't resume. Shows where the **B12 gap** sits.

```mermaid
sequenceDiagram
    participant W as web (wendy tab)
    participant API as rpg-api EndTurn handler
    participant SDK as encounter SDK
    participant NPCAct
    participant DR as Dnd5eCombatResolver
    participant TK as toolkit combat
    participant R as Redis
    participant Broker

    Note over W: wendy toggles "Shield: ready"
    W->>API: SetReactionReady(wendy, Shield, true)
    API->>SDK: SetReactionReady
    SDK->>R: SAVE encounter.ReactionReadiness[wendy][Shield]=true
    Note over W: it's the goblin's turn
    W->>API: EndTurn
    API->>SDK: LoadFromData + applyReactionConditions
    SDK->>NPCAct: NPCAct(goblin-1)
    NPCAct->>NPCAct: installTriggerBuffer
    NPCAct->>DR: ResolveAttackHit
    DR->>TK: combat.ResolveAttackHit
    TK->>TK: PostAttackRollChain publishes
    Note over TK: Shield's predicate ✓ all true<br/>(target=wendy, wouldHit,<br/>+5 deflects, readiness=true)
    TK->>NPCAct: publish ReactionTriggerEvent<br/>(reactor=wendy, kind=PostHit)
    TK-->>DR: AttackContext{hit=true}
    DR-->>NPCAct: AttackContext
    NPCAct->>NPCAct: drain trigger buffer<br/>found 1 player trigger
    NPCAct->>SDK: errNPCPausedForReaction
    SDK->>R: SAVE PendingReactionPrompt[wendy]={AttackContextJSON, ...}
    SDK->>Broker: publish InputRequiredDelivered<br/>(per-viewer to wendy)
    Broker-->>W: stream: InputRequiredDelivered
    Note over W: modal pops on wendy's tab<br/>"Cast Shield to gain +5 AC?"
    W->>W: clicks "Take"
    W->>API: SubmitCheck(take_reaction: true)
    API->>SDK: CompleteTakeAction(attackCtx, [Shield+5AC])
    Note over SDK: ⚠️ <b>Wave 2.11d B12 gap</b><br/>CompleteTakeAction checks<br/>findPlayerByEntityID(attackerID)<br/>attacker=goblin-1 → not a player<br/>RETURNS ERROR
    SDK-->>API: "attacker 'goblin-1' not in encounter"
    API-->>W: rpc error: Internal
    Note over W: <b>Wave 2.11e fix</b>:<br/>CompleteTakeAction accepts<br/>either attacker shape<br/>(rpg-toolkit#662)
```

After Wave 2.11e #662 lands: the dashed-arrow path completes (CompleteTakeAction → ApplyAttackOutcome with Shield modifier baked in → publish AttackResolved with deflected outcome → spell slot decremented → wendy survives).

## Flow 3: Player-OA-on-movement (the path B8 surfaced as broken)

The path that doesn't fire at all today because `Encounter.Move` bypasses MovementChain entirely. Shows the **B8 gap** and what Wave 2.11e #658 changes.

```mermaid
sequenceDiagram
    participant W as web (alice tab)
    participant API as rpg-api handler
    participant SDK as encounter SDK
    participant Move as Encounter.Move
    participant TK as toolkit combat

    Note over W: alice retreats past goblin
    W->>API: MoveEntity(alice, path=[...])
    API->>SDK: enc.Move(alice, path)
    SDK->>Move: directly mutate position
    Note over Move: ⚠️ <b>Wave 2.11d B8 gap</b><br/>Move never calls combat.MoveEntity<br/>Never publishes MovementChain<br/>OA condition's onMovementChain<br/>subscriber never fires
    Move->>Move: publish MoveEvent + HexRevealedEvent<br/>(no trigger drain)
    Move-->>SDK: ok
    Note over SDK: goblin's OA condition never<br/>got the chance to publish<br/>a ReactionTriggerEvent
    SDK-->>API: ok
    API-->>W: MoveEntityResponse
    Note over W: alice retreated, no OA fired<br/>(should have!)
```

After Wave 2.11e #658 lands: new `MovementResolver` interface (parallel to `PhasedCombatResolver`); rpg-api implements wrapping `combat.MoveEntity`; `Encounter.Move` calls resolver per-step + drains ReactionTriggerEvents via `installTriggerBuffer`. NPC OAs (Player trigger) fan out the same way as Flow 2.

## Flow 4: Sneak Attack invariant (Wave 2.11c regression canary)

The path that proves 2.11d's dep bumps didn't break 2.11c's foundation. UsedThisTurn must persist across attacks via JSON round-trip + write-back.

```mermaid
sequenceDiagram
    participant W as web (alice tab)
    participant API as rpg-api TakeAction handler
    participant SDK as encounter SDK
    participant TK as toolkit combat
    participant SA as SneakAttackCondition
    participant R as Redis

    Note over W: alice attacks goblin (turn 1, first attack)
    W->>API: TakeAction(alice, attack, goblin-1)
    API->>R: GET character:char-alice
    Note over API: SneakAttackCondition has<br/>used_this_turn=false in JSON
    API->>SDK: LoadFromData(bus)
    SDK->>SA: re-Apply() from JSON
    Note over SA: SneakAttack subscribes to<br/>DamageChain (per-attack)<br/>+ TurnEndTopic (once-per-turn)
    API->>TK: combat.ResolveAttack
    TK->>TK: AttackChain → roll → hit
    TK->>SA: DamageChain publishes
    Note over SA: predicate checks:<br/>finesse weapon ✓<br/>advantage OR ally adjacent? ✓<br/>used_this_turn = false ✓
    SA->>SA: roll 1d6 sneak die<br/>add to DamageChain
    SA->>SA: <b>used_this_turn = true</b>
    TK-->>API: AttackResult{damage = base + sneak, components}
    API->>SDK: cond.ToJSON() → write-back<br/>(used_this_turn=true now in JSON)
    SDK->>R: SAVE character:char-alice<br/>(with used_this_turn=true)
    API-->>W: TakeActionResponse

    Note over W: alice attacks again (same turn, 2nd attack)
    W->>API: TakeAction(alice, attack, goblin-1)
    API->>R: GET character:char-alice<br/>(used_this_turn=true)
    API->>SDK: LoadFromData
    Note over SA: predicate: used_this_turn = true<br/>NO sneak this attack
    TK-->>API: AttackResult{damage = base only}

    Note over W: end of turn
    W->>API: EndTurn(alice)
    API->>SDK: publishTurnEndAndPersistReset
    SDK->>SA: TurnEndTopic fires
    SA->>SA: used_this_turn = false
    SDK->>R: SAVE character:char-alice<br/>(used_this_turn=false again)
```

The persistence flow that B10 indirectly verified — when `LoadFromData` was silently dropping SpellSlots (the pre-existing bug fixed in rpg-toolkit#660), this same write-back pattern was the ONLY thing keeping the SneakAttack invariant working. The integration tests in rpg-api are the regression canary.

## Where the new SDK surface lives

```mermaid
graph TB
    subgraph "rpg-toolkit/rulebooks/dnd5e/combat (B6 refactor)"
        ahit["combat.ResolveAttackHit<br/>(phase 1, returns AttackContext)"]
        aapply["combat.ApplyAttackOutcome<br/>(phase 2, takes EventBus + Roller)"]
        actx["AttackContext<br/>(pure data, JSON-serializable)"]
        postroll["combat.PostAttackRollChain<br/>(NEW: Shield's subscription seam)"]
    end

    subgraph "rpg-toolkit/rulebooks/dnd5e/conditions"
        oa["OpportunityAttackCondition<br/>subscribes: MovementChain"]
        shield["ShieldSpellCondition<br/>subscribes: PostAttackRollChain"]
    end

    subgraph "rpg-toolkit/encounter"
        phased["PhasedCombatResolver<br/>(optional interface ext)"]
        takeact["Encounter.TakeActionPhased<br/>buffered subscriber +<br/>partitions NPC vs player triggers"]
        complete["Encounter.CompleteTakeAction<br/>⚠️ player-attacker only<br/>(B12 gap → 2.11e)"]
        pending["Encounter.PendingReactionPrompts<br/>(persists across RPC gap)"]
        delivered["InputRequiredDeliveredEvent<br/>(single-viewer, metadata only)"]
    end

    subgraph "rpg-api Dnd5eCombatResolver"
        dr["implements PhasedCombatResolver"]
        apply_cond["applyReactionConditions<br/>(OA universal, Shield to spellcasters)"]
    end

    actx --> ahit
    actx --> aapply
    postroll --> shield
    shield --> takeact
    oa -.MovementChain not<br/>currently drained.-> takeact
    takeact --> phased
    takeact --> pending
    takeact --> delivered
    complete -.B12 gap.-> phased
    dr --> phased
    apply_cond --> oa
    apply_cond --> shield

    classDef toolkit fill:#5f4a1e,stroke:#ff9800,color:#fff
    classDef api fill:#2d4a2b,stroke:#4caf50,color:#fff
    classDef gap stroke:#f44336,stroke-width:3px
    class ahit,aapply,actx,postroll,oa,shield,phased,takeact,pending,delivered toolkit
    class dr,apply_cond api
    class complete gap
```

The red-outlined `Encounter.CompleteTakeAction` is the B12 SDK gap — works for player-attacks (the path TakeActionPhased was originally built for), can't accept NPC attackers. Wave 2.11e #662 extends or splits it.

## What got verified this wave

| Verification | Where it lives | Status |
|---|---|---|
| Toolkit unit tests for OA + Shield predicates | `rulebooks/dnd5e/conditions/{opportunity_attack,shield_spell}_test.go` | ✅ green in #656 |
| Toolkit unit tests for phased combat (inline + NPC pause + player pause + legacy fallback) | `encounter/combat_phased_test.go` | ✅ green in #656 |
| AttackContext JSON round-trip + EventBus validation | `rulebooks/dnd5e/combat/attack_phases_test.go` | ✅ green in #656 |
| Character data round-trip (SpellSlots + ClassResources) | `rulebooks/dnd5e/character/character_test.go` | ✅ green in #660 |
| rpg-api `TestPlayerShield_NotReady_NoPrompt` integration | `internal/handlers/dnd5e/v2/encounter/integration_player_shield_test.go` | ✅ green in #542 |
| Sneak Attack canary (Wave 2.11c regression) | `internal/handlers/dnd5e/v2/encounter/integration_sneak_attack_test.go` | ✅ still green post-bump |
| Web harness tests (modal + panel + tri-state + stream + visual layer) | `src/components/playtest/PlaytestHarness.test.tsx` + `useSetReactionReady.test.ts` + `playtestMapHelpers.test.ts` | ✅ green in #408 + #412 |
| MCP playtest: seed loads + RPCs fire + #410 tri-state + attack flow + encounter-end | this session, driven via Chrome DevTools on :9222 | ✅ verified |

What did NOT get verified (deferred to Wave 2.11e):
- Shield resume with NPC attacker (B12)
- Player-OA on player movement (B8)
- Multi-Shield-style modifier stacking (not currently possible — only one post-hit reaction shipped)

Updated 2026-05-19.
