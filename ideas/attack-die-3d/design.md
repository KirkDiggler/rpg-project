# Staged 3D Attack Die

**Status:** written specification awaiting Kirk review

**Tracking:**
[rpg-project#216](https://github.com/KirkDiggler/rpg-project/issues/216)

**Concept:**
`?concept=attack-die-3d` in the web's existing development-only concept registry

## Summary

Improve the existing attack-roll presentation with the private lightning d20
GLB. The ultimate acceptance condition is exact and visible: every authoritative
server d20 result from 1 through 20 tumbles decoratively, then settles with that
engraved numeral visibly on top. The server result is never simulated or changed
in presentation code.

The concept will prove a production-intent shared `AttackDie3D` component in
four stages: appearance, calibration, result-driven roll, and all-face
verification. It does not promote the component into production. The existing
SVG `DiceTray` remains the accessible fallback for load, WebGL, shader, and
unmapped-result failures and for any environment in which the 3D presentation
cannot truthfully show the requested result.

The conversational design is approved; this written specification requires
Kirk's review before a plan or implementation begins. The design PR remains the
cross-repository review surface through later implementation.

## Context

The current production theater already establishes the interaction contract
around an attack roll: it stages an authoritative result, preserves combat
pacing, and exposes an accessible presentation. The 3D die is a visual upgrade
to that presentation, not a new rules path or a replacement source of truth.

A promoted lightning d20 model now exists in the private asset repository. It
supplies the geometry and engraved numerals, but not the metadata needed to
place each result on top and not the advertised lightning treatment. The concept
must therefore prove three things before production promotion can be considered:

1. the asset can achieve an approved magical appearance at runtime;
2. all twenty engraved faces can be mapped to exact settlement orientations; and
3. the result-driven animation can preserve existing presentation behavior and
   performance.

## Established facts

- Production web receives the authoritative `AttackResolved.attackRoll`, passes
  it through `CombatPresentation`, and presents it in `DiceTray`.
- Presentation randomness is decorative only. It must never generate, choose,
  reroll, clamp, reinterpret, or otherwise alter the outcome.
- The recommended design is a result-driven settle, so no proto, API, or toolkit
  change is needed.
- The runtime asset is `/models/synty/props/SM_Prop_D20_Lightning_01.glb`,
  sourced and promoted through `rpg-game-assets`.
- The runtime GLB is approximately 204 KB with 1,906 triangles, two material
  slots, and no textures, animations, or face-orientation metadata.
- Its source look is cloudy white/gray marble with gold numerals. Advertised
  lightning is not embedded in the GLB and requires a narrow web-owned material
  treatment.
- Current runtime material names include Blender suffixes:
  `D20_Lightning_Material.010` and `Paint_Material.010`.
- The repository and source path are private and correct, but current metadata
  does not establish an explicit ownership or license grant. Provenance and
  licensing confirmation is a shipping precondition, not an inferred fact.

## Decisions

1. **Server result is authoritative.** `AttackDie3D` receives a validated result
   from 1 to 20. Decorative randomness controls only the approach motion and
   never final face choice.
2. **Settlement is kinematic, not physical.** A decorative tumble transitions
   through a deterministic presentation schedule, then slerps/damps into the
   calibrated quaternion for the authoritative result. Physics and collision
   cannot be used to decide the face.
3. **Calibration is an asset contract.** Each numeral maps to one normalized
   quaternion tied to an exact GLB content hash, camera contract, material mode,
   and mesh/material selector contract. No face orientation is assumed resolved
   until calibrated and verified.
4. **The concept exercises shared production-intent code.** It uses
   `AttackDie3D`; it does not create a throwaway concept-only renderer.
5. **SVG stays truthful and available.** Existing `DiceTray` is retained as the
   visible fallback and accessible result presentation. A broken or incomplete
   3D path must never show the wrong physical face.
6. **The model is not a dungeon prop.** Do not add the d20 to the generic
   dungeon prop palette merely to obtain a URL. Use the established public
   runtime path directly through a component-owned asset contract.
7. **An isolated overlay is the preferred first architecture.** It best matches
   the current theater, but a second WebGL context is a measurable
   Discord/mobile GPU risk and must be measured before production promotion.
8. **Promotion is separate work.** This concept PR proves the contract;
   production wiring requires written design approval, implementation evidence,
   and a separately approved promotion issue/PR.

## Goals

- Render the promoted lightning d20 as a high-quality attack-roll presentation.
- Settle each authoritative result 1–20 to an exact calibrated orientation.
- Make the engraved result readable from the top and canonical three-quarter
  settlement cameras.
- Prove raw and magical appearance modes without rewriting the model.
- Provide a repeatable calibration and all-face evidence workflow.
- Preserve existing combat sequencing, accessibility, fallback, and control
  semantics.
- Define repository ownership and evidence sufficient to invalidate stale
  calibration or visual approval after relevant changes.

## Non-goals

- Physics or collision simulation.
- Damage dice or discarded advantage/disadvantage dice.
- Reproducing a server seed or trajectory.
- A universal dice platform or generic prop-palette addition.
- Rules, toolkit, proto, or API changes.
- Production promotion during the concept implementation.
- Publishing, releasing, or changing the source asset in this design PR.

## Product experience

For a normal attack presentation, the existing theater begins as it does today.
When the attack-roll beat is released, the 3D die appears in its presentation
layer, follows a short decorative tumble, and settles to the calibrated pose for
`AttackResolved.attackRoll`. The final face remains visible for the existing
result/damage release window. Hit, miss, natural 1, and critical presentation
still come from authoritative combat state rather than from inspecting the mesh.

The tumble should feel varied without implying that the client rolled the
result. A seeded or unseeded client visual path may vary position, spin axes,
intermediate quaternions, and lighting phase, but the path is discarded as an
authority source and always converges on the supplied target quaternion. The
final settle is exact rather than the incidental end of a simulation.

When reduced motion is requested, there is no tumble and no animated lightning.
The die briefly settles statically to the same result orientation. When 3D
cannot truthfully render the result, the authoritative SVG result appears
visibly instead.

## Architecture and ownership

### `rpg-game-assets`

Assets owns:

- explicit provenance and licensing confirmation;
- the promoted GLB and its content hash;
- a stable material/mesh selector contract, either through corrected export
  names or a documented normalized-prefix contract;
- the face-map asset contract tied to that GLB hash; and
- visual QA for engraved numeral legibility and the approved raw/magical
  appearance.

A future asset-owned mapping may be stored with source metadata and
copied/generated into a web-consumable form. Its single canonical source,
generation path, and hash binding must be specified in the implementation plan;
two independently edited maps are not allowed.

### `rpg-dnd5e-web`

Web owns:

- loading the asset from its public runtime URL;
- the narrow magical material/shader treatment;
- `AttackDie3D` and its deterministic, result-driven presentation state machine;
- development concept registration and controls;
- SVG fallback, accessibility integration, and reduced-motion behavior;
- unit/integration/visual tests; and
- Discord, mobile, and low-GPU performance playtests.

### Rules, proto, and API

Rules, proto, and API own no changes for this scope. They already deliver the
authoritative attack roll required by the recommended design. Any proposal to
move trajectory, face mapping, material state, or presentation randomness across
the network widens the scope and requires a new design decision.

## Component boundaries

### `AttackDie3D`

The shared component consumes presentation truth and reports presentation
completion; it does not own combat sequencing. Its boundary must support:

- authoritative integer `result` in `[1, 20]`;
- presentation activation/visibility controlled by the existing theater;
- `materialMode` selected from the approved raw/magical modes;
- reduced-motion state;
- the established completion callback semantics; and
- an explicit failure/fallback signal or rendering boundary that lets `DiceTray`
  remain visible when 3D is unavailable.

Internally it owns asset loading, validated contract lookup, decorative motion,
exact target orientation, camera/rendering state, and error containment. A
result outside `[1, 20]`, a contract/hash mismatch, or a missing mapping is an
invalid 3D presentation and takes the fallback path.

### Concept host

The `attack-die-3d` concept supplies staged controls and evidence views around
the same `AttackDie3D` used for production-intent behavior. Calibration tooling
may expose low-level orientation controls, but that tooling remains outside the
component's production API.

### Existing presentation

`CombatPresentation` remains the timing and sequencing owner. `DiceTray` remains
the SVG truth/fallback and the existing accessible announcement surface.
Introducing 3D must not change FIFO ordering, movement wait, attack-result
release, damage release, keyboard control, or completion callback behavior.

## Data flow and authority

```text
server AttackResolved.attackRoll (authoritative integer 1–20)
  -> CombatPresentation (existing FIFO/theater timing authority)
  -> attack-roll presentation beat
       -> AttackDie3D(result)
            -> validated contract lookup by result
            -> decorative visual path
            -> exact mapped target quaternion
            -> visual completion callback
       -> DiceTray authoritative SVG/accessibility path
            (visible whenever 3D is unavailable or untruthful)
```

No value flows back from mesh orientation into combat state. Neither animation
randomness, a physics engine, raycasts, nor face detection may determine
hit/miss, critical state, the announced value, or the settled target. The face
map is presentation calibration, not game logic.

The completion callback fires under the same once-only semantics for animated,
reduced motion, and fallback paths. Error handling must release the presentation
rather than stall the FIFO.

## Face-map and asset contract

The calibrated contract is valid only as a complete, hash-bound unit. It
records:

- contract schema version;
- cryptographic content hash of the exact runtime GLB bytes;
- runtime asset URL and stable mesh/material selectors;
- one mapping for every integer 1–20;
- each target as a finite normalized quaternion in one documented
  coordinate/order convention;
- model normalization assumptions, including any fixed root correction;
- the top and canonical three-quarter camera transforms and projection settings;
- material mode/version used for visual confirmation; and
- verification evidence identity and result.

The runtime accepts a map only when all twenty keys exist exactly once, each
quaternion is finite and normalizes within the defined numerical epsilon, the
asset hash and selector contract match, and the camera/material invalidation
keys match the approved contract. Unknown extra keys, duplicates, zero-length
quaternions, or a partial map invalidate the 3D path.

Quaternion sign equivalence (`q` and `-q`) must be handled in normalization,
comparison, and slerp so interpolation follows the shortest arc and verification
does not report a false mismatch. Final orientation is measured with
sign-invariant quaternion angular distance. The implementation plan must choose
and document a small numeric angular tolerance in degrees/radians based on the
rendering and transform pipeline; the same value must be used in tests and
evidence. This specification does not claim a tolerance has already been
measured.

Calibration saves normalized quaternions only; it does not claim that a saved
pose is correct. A face is complete only after human confirmation from both
settlement cameras. Any change to GLB bytes, selector/name contract, mapping,
root normalization, cameras, or material mode invalidates stale human
confirmations and requires a new 20-face evidence run.

## Material approach

The raw mode renders the GLB's source cloudy white/gray marble and gold numeral
treatment as faithfully as the runtime materials permit. The magical mode starts
from that look and adds a narrow web-owned lightning treatment; it must not
require a general material system or alter game state.

The treatment should remain confined to the die body, preserve gold numeral
contrast, and be parametrically disableable for raw comparison and reduced
motion. Suitable production implementation is a small material extension or
shader injection using stable selectors, with restrained emissive/color
variation and optional animated lightning. It must not rely on exact suffixed
Blender names alone. Accept either:

- an asset correction that exports stable exact names; or
- a documented normalized-prefix selector that strips Blender numeric suffixes
  and requires exactly one `D20_Lightning_Material` body match and one
  `Paint_Material` numeral match.

Ambiguous, absent, or unexpected matches are contract failures and fall back to
SVG rather than applying a shader to the wrong surface. Animated lightning is
disabled under reduced motion. Shader compile or runtime errors are contained by
the same fallback boundary.

## Concept stages and controls

The existing development-only registry exposes `?concept=attack-die-3d`. It must
not appear in production navigation or become a production feature flag.

### 1. Appearance

Purpose: approve the asset's baseline and magical treatment before calibration.

Controls and evidence:

- switch between `Raw` and `Magical` material modes;
- pause/enable magical animation when motion is allowed;
- inspect the die under the canonical top and three-quarter cameras;
- display the loaded GLB hash and resolved body/numeral material selectors; and
- capture comparable raw/magical frames with identical camera and lighting.

### 2. Calibrate

Purpose: author a proposed target quaternion for each engraved numeral.

Controls and evidence:

- select result 1–20;
- rotate with precise controls and reset to the existing saved pose;
- switch between top and canonical three-quarter cameras without changing the
  die pose;
- save only a normalized quaternion associated with the selected result; and
- show mapped/unmapped state, current quaternion, hash, camera contract, and
  material mode.

Saving does not mark visual verification complete. No default or inferred
orientations are represented as approved.

### 3. Roll

Purpose: prove result-driven decorative presentation.

Controls and evidence:

- choose an authoritative input 1–20;
- replay multiple decorative paths for the same input;
- toggle reduced motion; and
- display requested result, mapped target, final measured angular error,
  completion state, and whether SVG fallback was used.

Every animated path uses kinematic interpolation and a damped/slerped settle to
the exact mapped target. Decorative path variation cannot alter the endpoint.

### 4. Verify

Purpose: produce complete, repeatable all-face acceptance evidence.

Controls and evidence:

- run results 1 through 20 in a fixed order;
- capture final top and canonical three-quarter settlement views for every
  result;
- record expected result, final normalized quaternion, angular error, and
  pass/fallback;
- provide a 20-result evidence grid with both camera views available at review
  resolution; and
- record human visible/readable confirmation per result and camera.

The run identifies the exact GLB hash, mapping contract, camera contract,
material mode, build/commit, and test tolerance. A partial run cannot graduate.

## Failure, accessibility, and reduced motion

### Failure behavior

Asset load, WebGL creation/context loss, shader compile/runtime, contract/hash
mismatch, invalid result, and missing/invalid face mapping are recoverable
presentation failures. They must:

1. avoid rendering a potentially wrong physical face;
2. show the authoritative SVG result visibly;
3. preserve the existing result announcement and theater timing;
4. complete/release the presentation exactly once; and
5. provide diagnostic state in development without exposing noisy duplicate user
   messaging.

The 3D layer must not briefly display an arbitrary face while loading or before
its mapping is known. Keep it hidden until the truthful target and rendering
path are ready.

### Accessibility

The visual 3D die is `aria-hidden`. `DiceTray`/the existing accessible
presentation remains the one announcement source for the authoritative value, so
the result is announced exactly once. The concept's controls require
keyboard-accessible labels, focus visibility, and operable ordering, but
production keyboard behavior remains owned by the existing theater. Do not infer
accessible text from a rendered face.

### Reduced motion

Honor the existing preference path and `prefers-reduced-motion`. Skip tumble and
animated lightning, move directly through a brief static settle to the same
calibrated target, and preserve the normal visible hold and callback semantics.
Reduced motion changes motion, not authority, final orientation, result timing
order, or fallback requirements.

## Performance behavior

Start with an isolated DOM-overlay renderer because it matches the current
combat theater and contains failure. Treat its second WebGL context as a
hypothesis, not an accepted production cost. Instrument and compare it in a
Discord iframe and representative mobile/ low-GPU devices.

Measure at minimum:

- GLB transfer/decode and first-ready latency, cold and warm;
- context creation failures/context loss;
- peak and retained GPU/JS memory where available;
- frame time and dropped/long frames while tumbling and settling;
- effect on the underlying dungeon renderer and input responsiveness; and
- teardown/reuse behavior across repeated FIFO attack presentations.

The implementation should cache/reuse the loaded asset and rendering resources
within a bounded lifecycle rather than reload per roll, while disposing
resources when the owning presentation surface is destroyed. Production
promotion requires measured evidence that the chosen context/lifecycle is
acceptable. If it is not, redesign renderer sharing/compositing in the separate
promotion work; do not silently trade away existing theater semantics.

## Validation and evidence

Graduation evidence binds to one immutable tuple:

```text
web commit/build
+ exact runtime GLB hash
+ face-map/schema version and content
+ selector/root-normalization contract
+ top and three-quarter camera contract
+ material mode/version
+ angular tolerance
```

The concept graduates only when:

- all 20 results have valid calibrated mappings;
- all 20 animated final orientations are within the one defined small angular
  tolerance;
- all 20 reduced-motion final orientations reach the same mapped targets;
- all 20 engraved numerals are human-confirmed visible and readable from both
  the top and canonical three-quarter settlement cameras;
- failures and unmapped states visibly use authoritative SVG without a wrong 3D
  face; and
- the evidence grid and machine-readable run record identify the immutable tuple
  above.

Machine checks establish completeness and orientation. Human review establishes
numeral identity and readability; neither substitutes for the other. Evidence
should use screenshots or other non-licensed render output. Do not publish the
private GLB itself as evidence.

## Testing and playtest

### Automated tests

- Pure face-map completeness, exact key set, finite values, normalization,
  duplicate/extra rejection, and `q`/`-q` equivalence.
- Target-orientation and shortest-arc slerp math.
- Final sign-invariant angular-distance calculation and selected tolerance
  enforcement.
- Invalidation keys for GLB hash, selectors/root normalization, mapping, camera,
  and material mode/version.
- Loader, WebGL, shader, context-loss, hash-mismatch, invalid-result, and
  unmapped-result fallback without a wrong-face flash.
- Reduced-motion suppression of tumble/lightning and convergence on the same
  target.
- Once-only completion across success, reduced motion, and fallback.
- Existing presentation sequencing regression: FIFO order, movement wait, result
  and damage release, keyboard control, and completion callback semantics.

### Visual evidence

- Raw versus magical comparison under identical lighting/cameras.
- All-face 20-result grid with top and canonical three-quarter views.
- Final orientation error recorded for every result.
- Human confirmation for numeral identity, visibility, and readability in both
  views.

### Playtest matrix

Exercise hit, miss, natural 1, and critical flows; keyboard-only operation;
reduced motion; narrow viewport; Discord iframe; repeated FIFO attacks; and
representative low-GPU/mobile hardware. Confirm input remains responsive, the
existing result is announced exactly once, damage/result timing does not
regress, and fallback is visible under forced load/WebGL/shader/ unmapped
failures.

## Graduation and promotion criteria

### Concept graduation

The concept is ready for a production-promotion decision only after the complete
validation and playtest requirements above pass, including 20/20 mapping,
angular tolerance, and human-confirmed two-camera readability. Graduation means
the concept and shared component have proved the design; it does not place the
3D die in production combat.

### Production promotion

Production work starts only after:

1. Kirk approves this written design;
2. asset provenance/licensing is explicitly confirmed;
3. the stable asset/material/face-map contract is reviewed by Assets;
4. concept graduation evidence is accepted;
5. second-context performance risk is measured and accepted or redesigned; and
6. a separate approved promotion issue/PR defines web integration and rollback.

After written design approval, implementation PRs receive their own
owning-repository issues in `rpg-game-assets` and/or `rpg-dnd5e-web` as their
scopes require. They reference this canonical design and its open review PR. No
implementation issue is implied to exist yet.

## Incremental path

1. **Asset/material proof:** confirm provenance path, exact GLB hash and
   selectors; compare raw and narrow magical treatment.
2. **Face calibration:** establish camera/root conventions and normalized
   mappings for all 20 engraved numerals.
3. **Authoritative tumble/settle:** feed selected authoritative inputs through
   decorative kinematic motion to exact mapped targets in shared `AttackDie3D`.
4. **All-face verification:** produce bound machine evidence and human-reviewed
   two-camera evidence for every result, plus fallback/accessibility/performance
   playtests.
5. **Separate promotion:** after approval, plan and implement production wiring
   in the web under its own issue/PR.

Physics remains deferred throughout this path.

## Risks and preconditions

- **Unverified provenance/license:** the correct private source path is not
  itself an ownership grant. Shipping is blocked until explicit metadata or
  review establishes the right to use the asset.
- **Unknown face orientations:** the GLB supplies no face metadata. No result is
  considered mapped until calibrated and human-verified.
- **Fragile material names:** Blender suffixes can change. Exact suffixed names
  without an enforced export or normalized-prefix contract are not stable enough
  to ship.
- **Wrong-face failure:** partial maps, stale hashes, transient loading, and
  shader errors can create false results unless the 3D layer stays hidden and
  falls back atomically.
- **Legibility:** gold numerals and magical emission may lose contrast at
  settlement size. Two-camera human review and restrained treatment are
  required.
- **Coordinate drift:** model normalization, quaternion conventions, camera
  changes, or transform hierarchy changes can invalidate all mappings.
  Hash/invalidation contracts and angular tests must make drift explicit.
- **Second WebGL context:** Discord/mobile GPU limits may make the preferred
  overlay unsuitable for production. Promotion is blocked on measurement.
- **Sequencing regression:** new loading and animation states could stall or
  double-release the combat theater. Existing timing and once-only completion
  tests are mandatory.
- **Scope creep:** generic dice, network trajectory, physics, damage dice, and
  production wiring are intentionally excluded; each requires a separately
  approved design change.
