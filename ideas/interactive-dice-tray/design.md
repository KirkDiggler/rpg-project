---
name: Interactive Collectible 3D Dice Tray
description: A shared player ritual for throwing collectible dice while authoritative server outcomes remain unchanged
updated: 2026-08-13
confidence: high on the approved interaction, authority, preset, and Concepts Lab boundaries; production transport, ownership validation, and individual damage-die contracts remain later work
status: approved conversational design awaiting Kirk review of this written specification; tracked by rpg-project#219
---

# Interactive Collectible 3D Dice Tray

**Tracking:** [rpg-project#219](https://github.com/KirkDiggler/rpg-project/issues/219)

**Related work:**

- [rpg-project#216](https://github.com/KirkDiggler/rpg-project/issues/216) — staged lightning d20 rendering proof
- [rpg-dnd5e-web#749](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/749) — current web concept implementation
- [rpg-game-assets#47](https://github.com/KirkDiggler/rpg-game-assets/issues/47) — verified lightning d20 contract
- [rpg-game-assets#49](https://github.com/KirkDiggler/rpg-game-assets/issues/49) — collectible dice-set preset contracts

## Summary

Turn the existing attack-roll popup into a production-intent **3D dice tray**. A player sees their selected collectible d20 inside the tray and explicitly starts the throw by either pressing **Roll** or grabbing, shaking, and releasing the die. The gesture affects presentation only. The already-resolved server result remains the sole authority and determines the exact settled face.

The die may travel outside the tray's rounded rectangle during its decorative throw, but its resting geometry must finish fully inside that rectangle. Monster attacks use the same tray and die-preset architecture but may roll automatically with a visibly different preset.

The Concepts Lab is the first approval surface. It will prove one attack d20, player and monster modes, stable collectible-die preset IDs, result-driven settlement, explicit player input, and a side-by-side roller/spectator experience. The implementation should leave a clear layout and API seam for future damage-dice groups without pretending that current damage totals contain individual die results.

## Why this is more than an attack popup

Dice are part of the game's identity and player expression. A player should be able to equip a favorite set, show it to the table, retire a misbehaving die to “dice jail,” and pull out another. That requires a die to be an identity with an asset contract, not an incidental color attached to a player/monster boolean.

The tray also creates a durable presentation boundary:

- today: one authoritative attack d20;
- later: one or more authoritative damage dice when the event contract provides their individual results;
- throughout: accessible interaction, fallback, reduced motion, and clear ownership of presentation versus game authority.

This design does not build a universal dice simulator. It establishes the smallest boundaries that can support the approved d20 experience now without blocking later dice-set growth.

## Established facts

- The server resolves the attack before the web presents it. `AttackResolved.attackRoll` is an authoritative integer from 1 through 20.
- The current web beat sequence includes an `armed` player beat and a **Roll d20** action.
- The current sequencer also auto-throws a player's die after 1.5 seconds. That conflicts with this design and must be removed before production promotion.
- Monster and spectator attacks already bypass the local player's armed interaction and can auto-play.
- `EntityDamaged` currently provides a total and optional per-source totals, not the individual authoritative damage-die faces needed for honest 3D damage settlement.
- The lightning d20 is the only currently promoted game-ready die model in the web runtime.
- The asset team is building full dice sets. Asset issue #49 owns the reusable preset/set contract and the distinction between material skins and distinct models.
- A local pointer gesture is not currently delivered to teammates. Shared witnessing therefore needs a later presentation-coordination seam.

## Product decisions

1. **Player input is explicit.** A player attack waits indefinitely in the armed state until the player presses **Roll** or releases a grabbed die. There is no timer and no player autoplay in this version.
2. **The gesture is optional.** Pointer/touch grab, shake, and release provide a tactile ritual; the Roll button remains the keyboard, assistive-technology, and low-effort path.
3. **The gesture has no game authority.** It may influence decorative position, spin, and effect phase. It may not generate, reroll, bias, clamp, reinterpret, or conceal the authoritative result.
4. **Monster rolls may auto-play.** There is no player gesture to await for an NPC. Monster dice use the same tray and preset machinery with a distinct configured identity.
5. **The tray is the popup.** The rounded rectangle is the resting/interaction tray. Throw motion may cross its boundary; settlement must fit inside it.
6. **Collectible identity uses stable preset IDs.** Presets describe dice such as `lightning` or a future named crypt set. They do not encode `player` or `monster`; roller role selects a preset ID.
7. **Spectators see the roller's die.** The selected preset is part of the player's shared presentation identity. Spectators do not substitute their own local skin. Production must project the roller's authoritatively equipped preset before the tray arms; the release gesture is not the source of ownership or loadout truth.
8. **Shared release is compact.** Production will coordinate one release signal rather than streaming live pointer movement. That signal identifies the presentation and starts the decorative throw for witnesses; it may repeat the already-known preset ID for validation/fallback, but it does not establish ownership.
9. **Damage dice are reserved, not fabricated.** The tray may support a future dice group, but this concept renders only the d20 until individual authoritative damage results exist.
10. **Concept approval precedes production promotion.** The Concepts Lab proves reusable production-intent components. Combat wiring, real transport, ownership validation, and persistence require separately approved implementation work.

## Experience flow

### Player roller

1. The attack is already resolved authoritatively, but its result remains in the presentation layer.
2. The tray opens with the player's selected d20 resting fully inside the rounded boundary.
3. The tray enters **armed** and waits without a timeout.
4. The player either:
   - presses **Roll**; or
   - grabs the die, moves/shakes it, and releases it.
5. Pointer capture keeps the interaction valid if the pointer leaves the die or tray. Releasing outside the tray still commits the throw so the interaction cannot become stuck.
6. A decorative throw begins. Its path may extend outside the rounded tray.
7. The d20 converges on the hash-bound face-map quaternion for the authoritative result.
8. The exact result face is uppermost and the resting die is fully inside the rounded tray.
9. Verdict and impact presentation continue from authoritative combat state.

### Monster roller

1. The tray selects the configured monster die preset.
2. The armed interaction is skipped.
3. The decorative throw starts automatically.
4. The d20 settles from the same authoritative result contract as a player die.

### Spectator

1. The spectator resolves the roller's authoritatively equipped preset and sees it waiting in the tray. In the Concepts Lab, both panes receive the same local fixture; production needs an approved profile/loadout projection before this behavior can ship.
2. Live pointer movement is not streamed.
3. Release emits one compact presentation-coordination signal.
4. Roller and spectators start the same decorative throw from that signal.
5. Every client settles from its own authoritative attack event, never from a result carried in the gesture signal.
6. If coordination is absent or stale, the spectator shows a readable authoritative fallback/settlement rather than stalling combat.

## Architecture

### 1. Tray owns interaction and layout

A production-intent tray component owns:

- rounded popup/tray composition;
- resting bounds and responsive sizing;
- armed, grabbed, released, rolling, settled, and fallback interaction states;
- pointer/touch capture and cancellation;
- Roll-button semantics;
- player versus automatic roller behavior;
- one-shot release commitment;
- reduced-motion interaction behavior;
- roller/spectator presentation roles; and
- a future-capable list/group layout, initially containing one d20.

The tray does not load GLBs, patch materials, map faces, or decide outcomes.

### 2. Die renderer owns visual truth

The existing production-intent `AttackDie3D` should evolve behind a stable renderer boundary. It owns:

- resolving the selected preset contract;
- loading/cloning the selected model;
- applying material/effect treatment;
- decorative tumble and translation;
- exact result-to-quaternion settlement;
- renderer readiness and lifecycle;
- fail-closed SVG fallback; and
- telemetry needed by the concept and verification tooling.

It receives a validated authoritative result. It never reads gesture motion as a result source.

### 3. Preset registry owns die identity

A stable preset registry separates game selection from asset implementation. A conceptual entry contains:

```ts
interface DiePreset {
  id: string;
  displayName: string;
  familyId: string;
  dieKind: 'd20';
  modelContract: ModelContract;
  materialTreatment: MaterialTreatment;
  faceMap: FaceMapContract;
  presentationScale: number;
  effects: EffectContract;
}
```

This is illustrative, not a locked TypeScript interface. The important boundary is that a preset can be either:

- a material/effect variant over shared geometry; or
- a distinct model with its own URL, selectors, bounds, hash, scale, and face map.

Callers select a preset ID and do not branch on those implementation details.

The first Concepts Lab presets may use the existing lightning model with clearly different temporary material treatments. This proves the skin boundary while the asset team completes promoted sets. Provisional presets and face maps must remain visibly labeled as unverified and must not be mistaken for asset-owned canonical contracts.

### 4. Presentation coordination owns shared release

The eventual production coordination message is presentation-only. It needs enough data to identify and replay the shared ritual, for example:

- correlation/presentation ID;
- roller identity;
- selected preset ID repeated from the authoritative equipped-preset projection for consistency checking and fallback;
- release/variation identifier;
- bounded decorative throw parameters; and
- protocol version.

It must not carry attack result, hit/miss, damage, target HP, or any other game authority. It does not stream pointer movement.

The exact transport is intentionally undecided here. Platform must choose and validate both real seams: an authoritative equipped-preset projection available before the tray arms, and release coordination tied to the encounter presentation. Both must be checked against encounter stream behavior, reconnect, ordering, and spectator delivery before implementation. No Concepts Lab code should fake a production network contract.

## Concept API shape

The concept should exercise an API whose responsibilities are apparent at the call site. Illustratively:

```tsx
<DiceTray3D
  rollerRole="player"
  witnessRole="roller"
  phase="armed"
  dice={[
    {
      id: 'attack',
      kind: 'd20',
      presetId: 'lightning',
      authoritativeResult: 10,
    },
  ]}
  onRelease={handlePresentationRelease}
/>
```

Names may change during planning. The stable ideas are:

- the tray accepts a list, though the first list has exactly one d20;
- result and preset are explicit;
- roller/witness role and phase are independent from die appearance;
- release is presentation intent, not outcome; and
- the die renderer remains independently testable.

## Concepts Lab surface

Extend the existing `?concept=attack-die-3d` work or introduce a focused dice-tray stage without creating a disconnected mock renderer. The lab provides:

- authoritative result input, 1–20;
- player and monster roller modes;
- roller and spectator side-by-side panes;
- at least two clearly distinct provisional preset identities;
- preset selection on the roller, mirrored to the spectator;
- Roll button;
- pointer/touch grab, shake, and release;
- replay/variation without changing the result;
- reduced motion;
- responsive sizing/scale controls for review;
- explicit provisional/canonical contract status; and
- fallback exercises.

The side-by-side surface simulates a compact release signal locally. It must label that simulation and must not imply that production networking has been implemented.

## Sizing and geometry

The current concept die is intentionally large for visual review. The next iteration should try a presentation scale near **1.1**, down from 1.4, without shrinking the 440×360 review surface.

Sizing is a tray responsibility informed by preset asset facts:

- each preset supplies bounds and recommended scale;
- the tray computes a resting pose whose full projected geometry fits inside the rounded rectangle;
- travel may begin or continue outside that rectangle;
- the resting pose must remain readable from the approved overhead three-quarter camera;
- responsive layouts may reduce scale further but may not crop the authoritative face; and
- future multiple-die groups will require a group layout policy rather than applying the single-d20 scale blindly.

The first concept must visually review the 1.1 proposal. It is not a permanent universal scale.

## Result mapping and authority

Every physical result displayed in 3D requires a hash-bound face map for that exact model contract. The current geometry-inspected result-10 pose is a useful visual proof, not a complete production map.

Concept rules:

- canonical mapped results settle in 3D;
- provisional geometry-derived mappings are loudly labeled;
- unmapped results fall back to the truthful SVG result rather than settling on an incorrect face;
- changing GLB hash, selectors, coordinate convention, or root correction invalidates mappings; and
- gesture/replay variation cannot select or alter the target quaternion.

Asset issue #47 owns the canonical lightning map. Asset issue #49 owns how maps and model contracts attach to reusable preset identities.

## Damage-dice expansion seam

The tray is list/group-shaped so a later hit can transition from an attack group to a damage group. That future experience may present the weapon's die pool after the d20 verdict and let the player explicitly throw those dice.

This design does not implement it because `EntityDamaged` currently exposes totals, not individual die faces. A future rules/wire design must decide how individual rolls, modifiers, critical doubling, rerolls, source components, and privacy are represented. Only after those authoritative values arrive may 3D damage dice settle to them.

Until then:

- no client decomposition of a total into plausible dice;
- no client damage rolls;
- no decorative damage dice labeled as authoritative; and
- no requirement to promote full asset sets into the web runtime merely to fill empty tray space.

## Accessibility and input

- The Roll button remains present for keyboard, switch, assistive-technology, touch, and players who do not want the gesture.
- Grab/shake/release is an optional equivalent input, not a skill check.
- The gesture cannot affect result quality or timing bonuses.
- Pointer cancellation, lost capture, release outside bounds, and component unmount must leave one deterministic state: either still armed or committed once, never half-thrown.
- A release commits at most once.
- Reduced motion suppresses tumble and animated effects while preserving explicit player input and exact settlement.
- The authoritative result remains available through the existing semantic/live presentation surface; WebGL is never the only source of result meaning.
- Failure to load a model, preset, shader, or face map uses a readable fallback and does not block the presentation queue.

## Failure, discontinuity, and fallback

- **Unknown preset:** use the declared safe default if its authoritative mapping is valid; otherwise use SVG.
- **Unavailable asset or invalid hash:** fail closed to SVG.
- **Unmapped result:** show SVG; never display a different physical face.
- **Missed shared-release signal:** do not stall. When authoritative presentation must advance, show a readable settled/fallback result.
- **Missing equipped-preset projection:** use the published safe default or SVG; never trust a release signal as proof that the roller owns a preset.
- **Signal before attack event:** hold bounded presentation metadata only if Platform proves a safe correlation contract; otherwise ignore/fallback. Never infer the attack result.
- **Reconnect, snapshot, sequence gap, or mode change:** flush stale tray choreography and converge on current authoritative state.
- **Unknown signal version or unsafe decorative values:** discard or clamp presentation metadata; never affect gameplay state.
- **Unknown roller preset on spectator client:** resolve the published safe default/fallback while retaining the roller's displayed identity label where honest.

## Security and trust boundaries

A future client request to equip or display a preset is not proof of ownership. Production must validate equipped/owned presets at an authoritative account/profile boundary and project the equipped preset to witnesses before broadcasting it as the roller's identity.

The presentation signal itself is untrusted decorative input:

- validate IDs and versions;
- bound all numeric motion parameters;
- never use arbitrary asset URLs from a client;
- resolve only allowlisted preset contracts;
- ignore duplicate releases by presentation ID; and
- do not let a missing or malicious signal delay combat state.

Ownership validation, catalog delivery, and persistence are out of scope for the Concepts Lab but are explicit production gates.

## Testing and evidence

### Unit/component behavior

- Player mode enters armed and does not advance without explicit input.
- No player auto-timeout exists.
- Monster mode can start automatically.
- Roll button and gesture commit the same presentation transition.
- Multiple pointer-up/cancel/click paths commit at most once.
- Gesture data changes decorative motion only; authoritative result remains unchanged.
- Release outside the tray still commits safely.
- Reduced motion preserves explicit input and exact target.
- Preset switching changes visual contract without changing result.
- Unknown preset, invalid hash, unmapped result, and renderer failure fall back truthfully.
- Settled geometry is fully inside the rounded tray for each tested viewport/preset.

### Face correctness

- Complete provisional/canonical maps are exercised for results 1–20.
- Each mapped result reaches and holds its exact target within the approved angular tolerance.
- Human review confirms the engraved numeral is uppermost and readable from the approved cameras.
- Face verification is invalidated when bound asset facts change.

### Shared Concepts Lab proof

- Roller and spectator show the same selected preset.
- Spectator does not receive live pointer movement.
- One simulated release starts both decorative throws.
- Both converge on the same supplied authoritative result.
- Missing/duplicate/unknown simulated signals exercise non-stalling fallback.
- The lab labels coordination as simulated rather than production transport.

### Manual/device review

- Mouse, touch, keyboard, and reduced-motion paths.
- Responsive widths and representative device pixel ratios.
- Tray escape during motion and full containment at rest.
- Low-GPU/WebGL failure behavior.
- Readability of player versus monster identities.
- No licensed GLB or private evidence is committed to a public repository.

## Repository ownership and routing

### `rpg-project`

Owns this cross-repository design, plan, product decisions, and promotion gates.

### `rpg-dnd5e-web`

Owns the Concepts Lab, tray interaction/layout, reusable die renderer boundary, provisional preset registry, accessibility, fallback, and later combat-presentation integration.

### `rpg-game-assets`

Owns promoted dice-set assets, provenance status, hashes, selectors, bounds, scale facts, face maps, preset/set manifests, and private visual review. See #47 and #49.

### Platform repositories

A later approved production plan will route shared-release transport, authoritative equipped-preset validation, and any profile/catalog persistence to their actual owning repositories. The transport must be verified against real per-viewer encounter delivery rather than assumed by this design.

### Rules/proto/API

No changes for the d20 Concepts Lab. A future damage-dice design may require explicit individual authoritative roll data, but that work is not implied or pre-approved here.

## Concept acceptance criteria

The concept is ready for production-promotion design review when:

- [ ] The tray displays one production-intent 3D d20 selected by stable preset ID.
- [ ] Player mode waits indefinitely for Roll or grab/shake/release.
- [ ] Monster mode may auto-play.
- [ ] Gesture changes decorative motion but never the supplied result.
- [ ] The die may leave the rounded rectangle while moving and settles fully inside it.
- [ ] The proposed smaller scale is visually reviewed at representative widths.
- [ ] Roller and spectator panes show the roller's same selected preset.
- [ ] One compact simulated release starts both panes without streaming pointer motion.
- [ ] At least two visually distinct provisional preset identities prove the registry boundary.
- [ ] Results 1–20 are either truthfully mapped or visibly fall back; no wrong physical face is shown.
- [ ] Keyboard, touch, pointer cancellation, reduced motion, fallback, and one-shot release behavior pass.
- [ ] Provisional asset facts and simulated networking are labeled honestly.
- [ ] No production combat, transport, inventory, or persistence wiring is included.

## Production promotion gates

Production wiring is a separate decision and requires:

1. canonical hash-bound face maps and provenance status for promoted presets;
2. approved player/monster visual identities and tray sizing;
3. removal of the current player auto-throw timeout without introducing a presentation deadlock;
4. an approved non-stalling interaction contract with the production beat/FIFO sequence;
5. an approved shared-release transport and correlation design for roller/spectators;
6. authoritative equipped-preset ownership validation and a safe default policy;
7. reconnect, snapshot, sequence-gap, and unknown-version behavior;
8. device/performance evidence for the actual Discord Activity surfaces; and
9. separate implementation issues/PRs in each owning repository.

Kirk alone merges the resulting PRs.

## Out of scope

- Production combat integration in the concept PR.
- A universal physics-based dice simulator.
- Letting gesture quality influence a result.
- Player autoplay or an armed-state timeout.
- Streaming pointer movement to spectators.
- Inventory UI, ownership, purchasing, loadout persistence, or dice-jail persistence.
- Individual damage dice before the server supplies individual authoritative faces.
- Proto/API/toolkit changes in the d20 concept slice.
- Hard synchronization of all clients beyond one presentation release signal.
- Publishing licensed source assets or private evidence in public repositories.
