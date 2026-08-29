---
name: Interactive Collectible 3D Dice Tray
description: A shared player ritual for throwing collectible dice while authoritative server outcomes remain unchanged
updated: 2026-08-15
confidence: high on the merged authority/presentation boundary, corrected direct-tag Original carved d20 semantics, exact 2,684/7,798 roles, independent upward-result observation, accepted Stone 0 evidence, and reviewed Stone 1 tactile choreography; production transport/ownership, rigid-body settlement, and individual damage-die authority remain gated
status: Stones 0 and 1 complete — corrected asset authority merged through rpg-game-assets#60 at 7fed0fc, Stone 0 web semantics through rpg-dnd5e-web#752 at 8bc2a27, and Stone 1 tactile choreography through rpg-dnd5e-web#756 at 7b049d5; later stones remain separately gated
---

# Interactive Collectible 3D Dice Tray

**Tracking:** [rpg-project#219](https://github.com/KirkDiggler/rpg-project/issues/219)

**Related work:**

- [rpg-project#216](https://github.com/KirkDiggler/rpg-project/issues/216) — staged lightning d20 rendering proof
- [rpg-dnd5e-web#749](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/749) / [PR #750](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/750) — merged Concepts Lab authority, event, witness, drawer, gesture-release, fallback, and renderer boundaries
- [rpg-game-assets#47](https://github.com/KirkDiggler/rpg-game-assets/issues/47) — earlier lightning-d20 contract work, now superseded for new usage by the Original carved set
- [rpg-game-assets#49](https://github.com/KirkDiggler/rpg-game-assets/issues/49) / PRs [#50](https://github.com/KirkDiggler/rpg-game-assets/pull/50), [#51](https://github.com/KirkDiggler/rpg-game-assets/pull/51), and [#52](https://github.com/KirkDiggler/rpg-game-assets/pull/52) — merged Original carved and painted-number sets plus structurally complete hash-bound face/triangle metadata; Stone 0 live review later disproved the carved d20's semantic face and cutwall roles
- [rpg-game-assets#53](https://github.com/KirkDiggler/rpg-game-assets/issues/53) / [PR #55](https://github.com/KirkDiggler/rpg-game-assets/pull/55) — merged the first Stone 0 consumer runtime manifest/provider-root slice
- [rpg-game-assets#57](https://github.com/KirkDiggler/rpg-game-assets/issues/57) / [PR #60](https://github.com/KirkDiggler/rpg-game-assets/pull/60) — corrected carved result identity from direct tags, published strict runtime v2 witnesses, fixed the exact 2,684/7,798 roles, and merged at `7fed0fc`
- [rpg-dnd5e-web#751](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/751) / [PR #752](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/752) — consumed strict v2, independently observed upward results, replaced the circular evidence oracle, and merged Stone 0 at `8bc2a27`
- [rpg-dnd5e-web#755](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/755) / [PR #756](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/756) — implemented and reviewed Stone 1 tactile one-member roll groups, sanitized `VisualThrowProfile@1`, deterministic choreography, and exact-SHA evidence; merged to `dev` at `7b049d5`

## Summary

Turn the existing attack-roll popup into a production-intent **3D dice tray**. A player sees their selected collectible d20 inside the tray and explicitly starts the throw by either pressing **Roll** or grabbing, shaking, and releasing the die. The gesture affects presentation only. The already-resolved server result remains the sole authority and determines the exact settled face.

The die may travel outside the tray's rounded rectangle during its decorative throw, but its resting geometry must finish fully inside that rectangle. Monster attacks use the same tray and die-preset architecture but may roll automatically with a visibly different preset.

The first Concepts Lab slice fulfilled that boundary and merged through web PR #750 at `627184f` (merge `1322dc4`). Its later local held-motion experiment was intentionally not merged: the die moved on a world-axis rail and a full-tray blue grabbed border communicated the wrong physical model. The continuation replaces that experiment rather than layering fixes onto it.

The Concepts Lab was the first approval surface. PR #750 proved one attack d20, player and monster modes, stable preset IDs, result-driven settlement, explicit player input, and a side-by-side roller/spectator experience. The implementation leaves a clear layout and API seam for future damage-dice groups without pretending that current damage totals contain individual die results.

## Approved continuation: tactile roll groups through stepping stones

The player fantasy is not that gesture quality secretly changes the server result. It is that the die feels responsive enough that the player can pretend their release technique mattered. The durable rule is:

> The gesture changes **how** the dice roll, never **what** they roll.

The continuation is staged rather than attempting a universal simulator in one slice.

### Stone 0 — consume the permanent asset contract

The merged Original carved set is the default provider for new dice work. Stone 0's intended scope integrates `dice.original.carved.d20`, its exact runtime hash, material-free carved mesh, bounds, body/numeral triangle groups, and complete 1–20 settlement map behind the already-merged `DiceTrayPresentation → DiceTray3D → renderer` boundary; the first integration attempt remains blocked until the semantic correction below replaces its structurally valid but physically wrong metadata. The painted-number family merged in PR #52 proves a future distinct-preset path but is not selected in this slice: recessed carved geometry is the more physical baseline for the settlement and motion work. Stone 0 does not add tactile motion yet.

PR #50 supplies strong asset facts:

- seven promoted dice: d20, d12, d10 percentile, d10, d8, d6, and d4;
- one cohesive material-free carved family;
- human-approved private review renders;
- finite normalized face-map records bound to each exact runtime GLB hash, whose carved d20 labels require the semantic correction below;
- carved body/numeral triangle groups from PR #51, enabling two runtime materials without changing the GLB hash, with the d20's 879 misclassified cutwalls requiring correction;
- a second painted-number family from PR #52 for later preset work; and
- Original d20 hash `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e` (491,312 bytes).

It also exposes a cross-repository contract gap that Stone 0 must close before web consumption:

- the assets README still declares `harness/models/synty/` as the sole consumer tree;
- Original dice were promoted under `harness/models/custom-dice/`;
- the web sync script mirrors only `harness/models/synty/`; and
- the canonical dice manifest remains under authoring-only `library/custom-dice/`.

Stone 0 therefore lands inside-out in two ordered repository slices:

1. **Assets provider slice:** declare `harness/models/custom-dice/` as a supported consumer runtime root; generate a consumer-safe runtime preset manifest beside the promoted GLBs; validate its reproducibility, promoted paths, hashes, selectors, bounds, complete face maps, and direct-tag semantic witnesses; keep `library/custom-dice/` as authoring authority only.
2. **Web consumer slice:** sync both approved runtime roots into separate ignored public paths; strictly reconstruct and validate the runtime manifest; hash the GLB before parsing; validate the declared glTF node→mesh-definition binding through the GLTF parser metadata; apply runtime materials to the single carved mesh from asset-owned triangle groups; resolve authoritative results only through the asset map; coalesce provider loading; and fail closed to semantic SVG on every unavailable, malformed, stale, unmapped, or hash-mismatched input. Pending provider load shows result-free loading and no tray; terminal provider failure mounts the shared accessible presentation without Canvas, keeps the armed result concealed, and settles to SVG after the normal release rather than blocking the queue.

The web must not reach into `library/`, copy numeric face facts into source, or preserve Lightning-specific two-material assumptions in the generic provider.

### Stone 0 recovery — direct carved semantics and independent observation

The first exact-SHA Stone 0 package proved that the renderer faithfully reached the supplied quaternion, but Kirk's live browser review exposed that the supplied asset contract was wrong. The package therefore passed a circular oracle: `mappedTarget` came from the provider, the renderer applied it, and telemetry compared the observed pose to the same tuple. Digest-bound screenshots archived the outcome but did not independently identify the upward carved numeral.

Two asset-owned defects are now confirmed against exact GLB SHA-256 `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e`:

- `build_dice_tray_face_maps.py` copied result order from the painted d20 onto a carved d20 with a different physical numeral layout. Only results 1, 2, 9, 19, and 20 settle correctly; the other 15 entries are a deterministic permutation.
- the body/numeral derivation treated every retained normal cluster as an exterior-face candidate. It assigned 879 cutwall triangles to body. The correct exact partition is 2,684 body + 7,798 numeral = 10,482, not 3,563 + 6,919.

The GLB geometry and hash remain unchanged. Recovery is ordered and inside-out:

1. **Assets semantic correction:** derive result identity from the carved tagged scene's `D20_Result_##` carriers rather than painted decal node labels; bind those tags to exact runtime triangles; publish corrected face entries and the exact 2,684/7,798 partition; regenerate the consumer manifest; and invalidate the previous carved d20 visual approval until a readable all-20 contact sheet is approved.
2. **Assets semantic oracle:** for every result, independently resolve the carved tagged face normal against the exact runtime GLB, rotate it by the published quaternion, and require that result alone to reach world up. Require published body/numeral ordinal sets to match the tagged outer-plane versus recess/cutwall roles exactly. Structural completeness, unit quaternions, and a screenshot digest are not sufficient.
3. **Web observation hardening:** consume the corrected asset contract without a web-local permutation; retain target-hold telemetry but add an independently derived upward-result observation from asset-provided face witnesses and the actual rendered world transform; require requested result = observed upward result before 3D evidence can pass.
4. **Readable browser proof:** capture close-up Roller and Spectator witnesses for all 20 results in addition to full-page layout evidence, record explicit numeral contrast/coverage facts, and require human visual review. Canvas visibility must be labeled as canvas visibility, never as carved-numeral correctness.

The recovery must also repair deterministic source binding: the generated runtime manifest's `sourceManifestSha256` must equal the current tracked authoring bytes under `--check`. The functional settlement and triangle-role correction belongs to `rpg-game-assets`; the independent runtime observation and truthful evidence vocabulary belong to `rpg-dnd5e-web`. No merge-ready verdict may be restored until both owners pass their semantic gates and Kirk rechecks the live results.

### Stone 1 — tactile held and personalized release choreography

A roll is modeled as a **roll group**, even when the group currently contains one attack d20. Pointer-down must begin on or near a member die; grabbing any member of a future pending group picks up the whole group. The hit region is the projected die silhouette plus forgiving mouse/touch padding, not the whole tray. Pointer capture keeps the held group attached through outside movement. The grabbed visual is communicated by lift, shadow, and pose—never a full-tray blue border. Keyboard and assistive users retain an explicit per-group Roll/Grab control and visible focus.

While held, raw pointer samples remain local. The group follows the pointer in a camera-aligned two-dimensional tray plane with a small constant lift. Filtered velocity drives bounded tilt and wobble; repeated motion accumulates bounded visual shake energy. Reduced motion keeps only one static lifted cue.

On accepted release, the controller emits one frozen, bounded `VisualThrowProfile@1` containing only:

- normalized release position within the tray;
- normalized release direction and speed;
- accumulated shake energy;
- spin bias;
- deterministic motion seed; and
- schema version.

It contains no raw client coordinates, path history, timestamps, result, target, damage, URL, transport field, or new external correlation. `presentationId` remains the sole external identity. Button/keyboard Roll uses a neutral profile; Monster release profiles remain host-produced. Roller and Spectator consume the same immutable profile values and independently reproduce the same throw personality.

### Stone 1 implementation gate — 2026-08-15

Web PR [#756](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/756) implemented the approved slice at exact reviewed head `fe19dc7fff00890d6e7fec18ad8a03b982ee6b28` and merged to `dev` as `7b049d57f7956a7db963e622b5ec681a5ba3ce97` without changing the server-selected result or the merged `DiceTrayPresentation` authority boundary. The merge tree `46bbc6356b14303f2e92026a663727f1b6826189` exactly equals the reviewed-head tree, so the accepted package binds the merged bytes unchanged.

- `RollGroupGestureController` keeps raw pointer coordinates, IDs, timestamps, bounds, histories, velocity, and tilt local; the Concepts evidence bridge exposes only generation-fenced monotonic booleans and final sanitized profile/observation facts.
- Native pointer ownership lives on the renderer surface, so the exact 14 px mouse / 24 px touch envelope and outside-capture release are reachable while the explicit keyboard-neutral control remains intact.
- `ChoreographedSolverV1` consumes only the sanitized profile, elapsed presentation time, authoritative provider-backed target, one-member descriptor, reduced-motion preference, and optional local held state. Roller and Spectator retain independent contexts, clones, resources, motion, rendered poses, observations, and telemetry.
- Malformed profiles, invalid seeds, provider/renderer failures, lifecycle interruption, and evidence terminal publication fail closed. Reduced motion retains one static lifted cue and exact settlement without tumble, shake, bounce, or scatter.
- Final review reports 0 Blocker / 0 High / 0 Medium. The exact package contains 12/12 scenarios, 18 protocol PNGs, 12 independent contexts, and a PASS-only marker; package-manifest SHA-256 is `947ee1c884d698c295588f01deb9a5fd9d19fee29bcf8b4066c51e35f2711e54`.
- The exact-head full suite passed 192 files / 3,394 tests; local gates and all GitHub checks passed. Kirk independently inspected the live exact head and approved it.

One nonblocking Low remains in a static source-order test assertion; direct behavioral terminal-publication tests cover failure before, during, and after PASS. Production transport/reconnect, equipped-preset projection, physical touch hardware, real Discord/mobile/low-GPU coverage, multi-die groups, rigid-body settlement, and formal paired performance remain explicitly ungraduated.

**Restart direction:** do not reopen Stone 1 or smuggle later scope through its Concepts bridge. Production delivery first needs an approved transport/correlation/reconnect contract; Spectator collectible identity first needs authoritative equipped-preset projection; Stone 2 must wait for authoritative per-die/subset results; Stone 3 must preserve the same solver boundary and consume verified set-wide face maps plus tray collision geometry. Start each as a separately tracked slice owned by the repository/lane that supplies its missing authority.

### Permanent solver boundary

Stone 1 introduces internal seams without changing the merged public authority boundary:

- **RollGroupGestureController** owns local hit testing, pointer capture, raw samples, filtering, held energy, and terminal cleanup.
- **SettlementResolver** validates asset identity and maps each authoritative result to the asset-owned target quaternion.
- **DiceMotionSolver** receives roll-group descriptors, one sanitized throw profile, authoritative targets, and elapsed presentation time, and returns Three.js poses only.
- **ChoreographedSolverV1** deterministically selects and parameterizes arc, spin, bounce, scatter, and continuous final convergence from the throw profile.
- **RigidBodySolverV2** later replaces V1 behind the same interface.
- **DiceRuntimeProvider** owns coalesced manifest/GLB loading and immutable validated provider values.

All movement is applied to Three.js die groups. Canvas and renderer CSS transforms remain forbidden. A solver/provider/renderer failure preserves the merged truthful SVG behavior.

### Later stones

- **Stone 2 — multi-die roll groups:** after authoritative per-die results exist, grabbing any member gathers the group into a compact springy “invisible handful”; release scatters each die with deterministic offsets while preserving one shared throw personality.
- **Stone 3 — physical settlement:** consume verified set-wide face maps and tray collision geometry; introduce deterministic rigid-body simulation and a landing solver that chooses a physically valid trajectory settling naturally on each server-selected face.

Lasso selection, partial-group rolling, rolling individual damage dice, inventory/equipment, production transport, and collectible persistence remain separately designed later work. Partial-group interaction cannot ship until the authority contract names individual dice/subsets rather than only a damage total.

### Stone 0 retro criteria

The Stone 0 retro asks two questions:

1. Could the Lightning concept provider be replaced by Original carved d20 without rewriting `DiceTrayPresentation`, authority reconciliation, witness roles, or fallback semantics?
2. Does every physical face and model fact originate in the asset runtime contract, with no web-authored quaternion, selector, path, or hash exception?

**Retro outcome — yes to both (2026-08-15).**

1. `DiceTrayPresentation` and its request/release, Roller/Spectator, accessibility, and truthful SVG fallback semantics survived unchanged as the literal authority boundary. Stone 0 replaced the provider/renderer behind that seam, then added geometry-backed observation as a confirmation gate rather than a second source of authority.
2. The selected model path, size/hash, selectors, bounds, material roles, settlement quaternions, direct face witnesses, and result directions all come from the strict asset runtime contract. Web production code contains no corrective permutation, copied quaternion table, carved-label table, model-path exception, or model-hash exception. The exact accepted provider is assets merge `7fed0fc`; the web merge is `8bc2a27`; the accepted package-manifest SHA-256 is `7fcd64b814629e23b5fa3278d7e3166754c5de3b41ab0c7e813eaf2bfa769809`.

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
- The lightning d20 was the only model available to the merged Concepts Lab slice and is now explicitly superseded for new dice usage.
- Assets PR #50 merged the complete Original carved set and promoted runtime GLBs. Its published face/triangle metadata was structurally complete but not semantically sufficient: live Stone 0 review later proved the Original d20 result order and 879 triangle roles wrong.
- Assets PR #55 promoted the consumer-safe runtime manifest and `harness/models/custom-dice/` provider root. The web consumes that boundary in PR #752, but the PR is blocked until a correcting assets contract is merged and resynced.
- A local pointer gesture is not currently delivered to teammates. Shared witnessing therefore needs a later presentation-coordination seam.

## Product decisions

1. **Player input is explicit.** A player attack waits indefinitely in the armed state until the player presses **Roll** or releases a grabbed die. There is no timer and no player autoplay in this version.
2. **The gesture is optional.** Pointer/touch grab, shake, and release provide a tactile ritual; the Roll button remains the keyboard, assistive-technology, and low-effort path.
3. **The gesture has no game authority.** It may influence decorative position, spin, and effect phase. It may not generate, reroll, bias, clamp, reinterpret, or conceal the authoritative result.
4. **Monster rolls may auto-play, but rendered witnesses do not produce that event.** There is no player gesture to await for an NPC. The fixture host—and later the single authoritative production adapter—appends one deterministic release for a monster presentation. Roller and spectator components only consume it, preventing duplicate autoplay from multiple clients, remounts, or StrictMode.
5. **The tray is the popup.** The rounded rectangle is the resting/interaction tray. Throw motion may cross its boundary; settlement must fit inside it.
6. **Collectible identity uses stable preset IDs.** Presets describe dice such as `dice.original.carved.d20`; they do not encode `player` or `monster`. Roller role selects an authoritatively projected preset ID, while runtime material treatment may distinguish table roles without changing mesh identity. A safe preset ID is 1–64 characters, one to eight dot-separated segments, and each segment matches `[a-z][a-z0-9-]{0,31}`. This admits historical `lightning` and canonical dotted IDs while rejecting URLs, slashes, colons, traversal, empty segments, and arbitrary paths.
7. **Spectators see the roller's die.** The selected preset is part of the player's shared presentation identity. Spectators do not substitute their own local skin. Production must project the roller's authoritatively equipped preset before the tray arms; the release gesture is not the source of ownership or loadout truth.
8. **Shared release is compact.** Production will coordinate one release signal rather than streaming live pointer movement. That signal identifies the presentation and starts the decorative throw for witnesses; it may repeat the already-known preset ID for validation/fallback, but it does not establish ownership.
9. **Damage dice are reserved, not fabricated.** The tray may support a future dice group, but this concept renders only the d20 until individual authoritative damage results exist.
10. **Concept approval precedes production promotion through outside-in contracts.** The Concepts Lab hosts fixture producers around the literal shared components and component-input contracts intended for production; it must not create a duplicate concept renderer, disposable event model, or concept-only projection layer. Roller and spectator proofs instantiate the same shared component against the same append-only presentation-event contract. Later production work maps authoritative server/profile/transport facts into that contract so the approved components begin working unchanged. Combat wiring, real transport, ownership validation, and persistence still require separately approved implementation work.
11. **The tray lives in a left presentation drawer.** In the gameplay composition, an always-visible drawer floats over the lower-left map area immediately above the current encounter dock. It should read as a physical drawer seen from above and pulled open toward the player: a wider, foreshortened rolling floor, visible back plane, angled side planes, lower front face with a restrained handle, and directional shadow surround the real 3D die. It must not read as an upright safe, monitor, appliance, or stack of nested rounded cards; the interior floor—not the front face—is the dominant plane. Any left-edge attachment cue stays shallow and subordinate rather than becoming a vertical rail. The first concept builds that carcass with responsive CSS/DOM planes rather than Three.js geometry, so it does not disturb the die camera, lighting, pointer coordinates, containment, or fallback. The existing default-open combat log remains on the right, and center-map verdict/damage presentation remains a separate surface. The drawer contains dice only: no hit/miss/crit label, damage total, modifier equation, or combat-log breakdown.
12. **Always visible is the default, not the only future preference.** The first concept shows the drawer open at all times. A later promotion design may offer always-open, contextual auto-open, or collapsed display preferences without changing the dice data or authority boundary. This concept does not build that preference control.
13. **The shared visualization proof may stay on result 10.** The goal of the current completion slice is tactile Roll/grab/release and event-fed roller/spectator witnessing. It does not need client randomness or a web-owned 1–20 map. The fixture supplies the already-known result 10; asset-owned metadata later broadens physical settlement without changing the shared component contract.
14. **Combat-log fixtures remain structured data.** The real `CombatLog` already consumes the shared `CombatLogEntry` union containing typed event facts and synthesizes prose only at render time. The dice concept reuses that production component contract; fixtures must not supply authored description/message strings or introduce a second combat-log vocabulary.

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
- validating the declared glTF object-node to mesh-definition binding before scene preparation;
- applying material/effect treatment;
- decorative tumble and translation;
- exact result-to-quaternion settlement;
- renderer readiness and lifecycle;
- fail-closed SVG fallback; and
- telemetry needed by the concept and verification tooling.

It receives a validated authoritative result. It never reads gesture motion as a result source.

### 3. Presentation events drive the literal shared component

`DiceTrayPresentation` consumes an ordered append-only list of strictly validated component-input events: an authoritative presentation request followed by at most one compact presentation-only release. One bounded `presentationId` is the sole external correlation; renderer item IDs, numeric render generations, and telemetry tokens are allocated locally and never leak into the input contract. The first valid request fixes all result, roller, and preset facts for that presentation. The first matching release wins by presentation ID; conflicting requests and later releases are ignored.

It projects those events into the existing `DiceTray3D` and owns only local renderer-observation settlement. Concepts Lab and production instantiate this same component. The concept appends fixture events in memory; later production adapters map authoritative attack/profile/transport facts into the same input contract. A release callback requests an append but is not proof of delivery, so the component remains armed until the matching event appears in its input list.

Lifecycle semantics are part of the component contract: a release appended to an already-mounted armed request animates; initial hydration with released history or any non-prefix discontinuity converges directly to settled/fallback rather than replaying stale choreography; a new presentation ID resets local observation. Renderer failure remains concealed while armed and converges to truthful semantic SVG only after release. Unknown but syntactically safe preset IDs never become asset URLs and degrade to SVG rather than erasing the result.

The event contract is not itself a transport protocol. It contains no URL, authored prose, pointer stream, damage, target, renderer token, or network timing field. Pointer movement remains local until one sanitized release is requested.

### 4. Preset registry owns die identity

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

The merged Concepts Lab used the existing Lightning model as a clearly labeled provisional provider. New dice usage resolves only the Original carved runtime contract from PR #50. Material/effect variants may share that geometry, but provisional providers and face maps must remain visibly labeled and must never be mistaken for asset-owned canonical contracts.

### 5. Presentation coordination owns shared release

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
<DiceTrayPresentation
  label="Alice's attack die"
  witnessRole="roller"
  events={presentationEvents}
  onEventRequest={appendValidatedFixtureEvent}
/>
```

Names may change during implementation. The stable ideas are:

- the literal shared component consumes append-only validated events rather than concept-owned phase state;
- the request event carries one already-known d20 result and stable preset ID;
- the release event carries presentation intent, never outcome authority;
- roller/spectator role is a viewer input independent from die appearance;
- both views consume the same event values while maintaining their own renderer observation; and
- `DiceTray3D` and the die renderer remain independently testable beneath the event-fed component.

## Merged Concepts Lab surface (#750)

The `?concept=attack-die-3d` Tray stage fulfilled the following surface without creating a disconnected mock renderer. The Tray stage is a fixture-backed sample gameplay screen rather than an isolated component card: it composes the real current `EncounterDock` along the bottom, its default-open combat log on the right, a neutral map stand-in, and the always-open dice drawer floating on the left above the dock. It does not wire encounter state, alter the center verdict/damage presentation, or duplicate combat-log math.

The lab provides:

- fixed authoritative fixture result 10 for the current visualization proof;
- player and monster roller modes;
- roller and spectator side-by-side panes;
- the current lightning preset mirrored from roller to spectator;
- **Deferred A:** multiple collectible preset identities and selection;
- **Deferred B:** asset-owned authoritative result input 1–20;
- Roll button;
- pointer/touch grab, shake, and release;
- replay/variation without changing the result;
- reduced motion;
- responsive sizing/scale controls for review;
- explicit provisional/canonical contract status; and
- fallback exercises.

The side-by-side surface simulates delivery of the shared component-input event locally. The reusable event type, reducer/projection, and tray experience belong with shared production-intent UI code; only the fixture producer/delivery harness belongs to the concept. The surface must label delivery as simulated and must not imply that production networking has been implemented.

## Sizing and geometry

The merged concept established a presentation scale of **1.1** for the provisional Lightning model on a 440×360 review surface. Original carved integration does not reuse that asset-local number blindly.

Sizing is a tray responsibility informed by asset facts:

- the asset contract supplies exact authored bounds, coordinate units/convention, and cohesive set-relative dimensions;
- the web owns a target visual extent for the approved tray/camera and deterministically computes one uniform normalization from those facts;
- no caller supplies an arbitrary model scale, and the web does not maintain per-result or per-preset exception tables;
- before normalization, every axis must satisfy `bboxMax - bboxMin > 0`, `dimensions > 0`, and `abs(dimensions - (bboxMax - bboxMin)) <= 0.000001`; zero, reversed, negative, or internally inconsistent bounds fail closed; `0.000001` is reconciliation tolerance, not a minimum authored unit size;
- the approved Stone 0 d20 target maximum extent is `0.55` world units: recenter on `(bboxMin + bboxMax) / 2`, then compute `uniformScale = 0.55 / max(dimensions)`; Original d20 dimensions `[10,10,10]` therefore yield scale `0.055`;
- that extent remains constant at the approved 1440/1241/1240/760 review widths because the 356px drawer remains constant, and settled projected geometry keeps at least 8 CSS px clearance from every well edge;
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

Assets owns the canonical Original carved maps and their binding to exact runtime hashes. Web validates and consumes that asset-owned metadata; it does not independently reconstruct, label, permute, or patch face quaternions. If a physical result is wrong, the correction returns to the asset contract owner rather than becoming a web-local exception. Asset generation must derive carved labels from direct carved tags, not from a geometrically congruent die with a different numeral layout. Earlier Lightning and painted-d20 mapping work remain supporting evidence for their own models, not result-order authority for the carved provider.

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
- Accessible settled status follows matching renderer telemetry (`observed` 3D versus failed/fallback), never a literal preset-name check.
- Failure to load a model, preset, shader, or face map uses a readable fallback and does not block the presentation queue.

## Failure, discontinuity, and fallback

- **Unknown preset:** use the declared safe default if its authoritative mapping is valid; otherwise use SVG.
- **Unavailable asset or invalid hash:** while loading, show result-free polite loading and mount no tray/Canvas. On terminal failure, mount the shared presentation without Canvas, preserve Roller input/Spectator non-authority, conceal the armed face as `?`, and settle to truthful SVG only after the matching release.
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
- **Deferred A:** preset switching changes visual contract without changing result.
- Unknown preset, invalid hash, unmapped result, and renderer failure fall back truthfully.
- Settled geometry is fully inside the rounded tray for each tested viewport/preset.

### Stone 0: full face correctness

- The corrected asset runtime contract is exercised for Original d20 results 1–20 after the consumer-safe manifest is regenerated.
- For each result, an assets-side semantic oracle independently binds the direct carved result tag to exact runtime geometry and proves the published quaternion rotates that tagged face alone to world up.
- Published geometry roles exactly match 2,684 outer-body triangles and 7,798 numeral recess/cutwall triangles for the unchanged Original d20 hash; no cutwall is body-colored.
- Web independently derives the observed upward result from asset face witnesses and actual rendered world transforms; target-tuple equality alone cannot pass.
- Each mapped result reaches and holds its exact target within the approved angular tolerance and reports `observedUpwardResult === requestedResult`.
- A readable close-up for every result and both witness roles is reviewed; full-page 78px dice and PNG digests alone are insufficient identity/readability evidence.
- Face verification is invalidated when the bound runtime hash, selector, coordinate convention, bounds, direct tag witness, or geometry partition changes.
- Web integration evidence proves strict consumption, independent upward observation, contrast/readability, and fail-closed behavior without duplicating or overriding asset authority.

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

Owns promoted dice-set assets, provenance status, hashes, selectors, authored units/bounds, face maps, preset/set manifests, and private visual review. PR #50/#49 provide the Original set; the Stone 0 provider slice promotes its consumer-safe runtime contract.

### Platform repositories

A later approved production plan will route shared-release transport, authoritative equipped-preset validation, and any profile/catalog persistence to their actual owning repositories. The transport must be verified against real per-viewer encounter delivery rather than assumed by this design.

### Rules/proto/API

No changes for the d20 Concepts Lab. A future damage-dice design may require explicit individual authoritative roll data, but that work is not implied or pre-approved here.

## Merged concept acceptance criteria

These criteria record the completed PR #750 proof. Deferred collectible/full-map items move into Stone 0 rather than reopening the merged concept.

- [ ] The Tray stage shows a sample gameplay composition with the real current encounter dock below the map, the default-open combat log on the right, and the dice-only drawer floating on the left.
- [ ] The drawer is always visible by default, reads as a wider horizontal drawer with a foreshortened floor and distinct back/side/front planes, and contains no verdict, damage total, modifier equation, or combat-log breakdown.
- [ ] At gameplay scale the drawer does not read as an upright safe, monitor, appliance, or nested rounded card.
- [ ] The CSS/DOM drawer carcass leaves the Three.js canvas untransformed and preserves die camera, scale, containment, fallback, and reduced-motion behavior.
- [ ] The tray displays one production-intent 3D d20 selected by stable preset ID.
- [ ] Player mode waits indefinitely for Roll or grab/shake/release.
- [ ] Monster mode may auto-play from one host-produced release event; rendered witnesses do not compete to produce it.
- [ ] Gesture changes decorative motion but never the supplied result.
- [ ] The die may leave the rounded rectangle while moving and settles fully inside it.
- [ ] The proposed smaller scale is visually reviewed at representative widths.
- [ ] Roller and spectator instances of the literal shared component consume the same append-only event values and show the same lightning preset and fixed result 10.
- [ ] One compact fixture-delivered release starts both panes without streaming pointer motion.
- [ ] Fixed result 10 settles truthfully; invalid/missing mapping or renderer failure visibly falls back without early reveal or stall.
- [ ] **Deferred A:** at least two visually distinct preset identities prove the registry boundary.
- [ ] **Deferred B:** asset-owned results 1–20 are either truthfully mapped or visibly fall back; no wrong physical face is shown.
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
- A universal physics-based dice simulator in Stones 0–2; rigid-body settlement is explicitly reserved for Stone 3.
- Letting gesture quality influence a result.
- Player autoplay or an armed-state timeout.
- Streaming pointer movement to spectators.
- Inventory UI, ownership, purchasing, loadout persistence, dice-jail persistence, or drawer-display preference persistence.
- Individual damage dice before the server supplies individual authoritative faces.
- Proto/API/toolkit changes in the d20 concept slice.
- Hard synchronization of all clients beyond one presentation release signal.
- Publishing licensed source assets or private evidence in public repositories.
