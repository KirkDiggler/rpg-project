# Authored crypt lighting — design

*Issue: rpg-project#190 · Parent journey: rpg-project#169 · Team: Assets*

Approved in conversation with Kirk on 2026-08-27.

## Outcome

An author can turn a crypt region's ambient exposure down to near-black, place
torches or other known light-source props, and see those fixtures illuminate the
nearby floor and 3D scene in both the dungeon builder and playable session.

This is a visual-fidelity slice. It makes authored light placement meaningful
without changing fog of war, line of sight, character sight, darkvision, or any
other game rule.

## Reconciled current reality

The original form of rpg-project#190 proposed a new dungeon-wide
`lighting.ambient` field and toolkit/proto/API passthrough. That work is no
longer needed:

- dungeon spec v2 already authors `regions[].lighting.intensity` in `[0,1]`;
- `AtlasRegion` already carries region cells, archetype, and lighting to the
  web;
- the builder's 2D view already darkens region swatches from that intensity;
- the builder preview and playable session share the crypt shell, scene lights,
  floor, walls, doors, props, and world calibration after rpg-dnd5e-web#827 and
  #829;
- the shared `DungeonSceneLights` still renders one static ambient `0.6` plus
  directional `0.8` rig and `buildScene3D` currently discards region lighting;
- the device-stable crypt floor deliberately remains an unlit
  `MeshBasicMaterial` with `toneMapped={false}`, so ordinary Three.js lights do
  not illuminate it; and
- the older renderer contains useful mood-light and deterministic unlit-floor
  pooling precedent, but that treatment is not wired into the current shared
  builder/session environment.

The reference tomb proves why one uniform-light resolver is insufficient: its
entrance, hall, and tomb are intentionally authored at `0.6`, `0.4`, and `0.15`.
Mixed intensity is normal content, not a fallback condition.

## Decisions

1. **Region intensity is ambient exposure.** It establishes how visible a
   region is without a placed source. It does not act as a master dimmer for
   light-source props.
2. **Known lighting props emit independently.** A torch in an intensity `0`
   region still illuminates its surroundings.
3. **Visuals only.** Lighting changes no fog, visibility, line-of-sight,
   darkvision, movement, targeting, or encounter behavior.
4. **One composed slice.** Ambient darkness and local sources land together so
   Kirk judges their contrast once. Shipping ambient alone would force it
   brighter for readability and then require retuning when sources arrive.
5. **One source manifest.** The builder Lighting category and runtime emitter
   behavior derive from the same immutable client manifest.
6. **The floor remains unlit and device-stable.** Source pools are deterministic
   color composition over the existing `toneMapped={false}` floor, not a return
   to the reverted lit-material path from rpg-dnd5e-web#566/#585.
7. **Builder and game share one environment component and one lighting plan.**
   No builder-only source table, intensity mapping, floor pool, or light value
   exists.
8. **Atomic fallback.** Invalid lighting facts preserve the complete current
   static rig and current floor treatment. The renderer never produces a dark
   floor without a functioning source plan.
9. **No inferred door glow.** Doors illuminate only when an authored source is
   placed nearby.
10. **Determinism over animation.** No random flicker ships in this slice.

## Approaches considered

### Chosen: regional exposure plus authored source props

Retain region lighting, use a low crypt fill, create real point lights from
known authored props, and reuse those source descriptions to color the unlit
floor deterministically. This delivers the desired dark-room/placed-torch
composition and exercises the existing authored contract.

### Rejected: ambient first, source props later

This is smaller only by line count. It cannot prove the desired result and
encourages calibrating ambient too brightly so source-free rooms remain
readable. The source slice would then reopen the first slice's visual values.

### Rejected: fixed global mood plus sources, regions later

This provides quick atmosphere but leaves the existing per-region builder
control misleading and ignores the reference tomb's intended brightness
progression.

### Deferred: true regional volumetric lighting

Three.js has no inexpensive ambient light bounded to an irregular authored
cell set. Applying regional illumination to every floor, wall, prop, door, and
character simultaneously would require a material/shader volume system,
per-object light layers, or shadows. That is disproportionate for this
pre-playable visual pass.

## Visual model

The first version combines three layers.

### 1. Low crypt fill

A fixed, calibrated low ambient/directional pair keeps non-source geometry
barely legible and gives real point lights room to read. Exact values are visual
calibration, seeded from the older crypt mood-light work rather than invented
without comparison.

Unsupported/non-crypt scenes keep the current `0.6` ambient and `0.8`
directional values exactly.

### 2. Regional floor exposure

For every crypt floor cell:

1. start from the approved crypt shell floor tint;
2. mix toward a near-black crypt floor color using the containing region's
   clamped `[0,1]` intensity;
3. blend active source colors over that base using deterministic distance
   falloff; and
4. render through the existing unlit, `toneMapped={false}` material.

All cells in one region receive the same base exposure. The shell's
absolute-world UV frame stays unchanged, so masonry remains continuous and
individual hexes do not reappear. Exposure changes only at authored region
boundaries, which normally coincide with walls or doorways.

This is the deliberate lean approximation for regional ambient. The dominant
floor surface carries the region's base exposure; nearby point lights make
walls, doors, props, and characters respond spatially. A future shader/material
volume can extend region ambient across every 3D surface without changing the
authored data model.

### 3. Authored source lights

A recognized placed prop produces both:

- one real Three.js point light for lit 3D materials; and
- one deterministic floor-pool input using the same position, color, radius,
  and strength.

The source's cell center, authored planar offset, authored elevation, and
preset model-relative light height determine the light origin. Moving,
raising, or removing the prop moves, raises, or removes the complete visual
source.

Source output is independent of region intensity. Floor pools are clipped to
the source's containing region in this no-shadow version. Conservative point
light radii limit obvious bleed through walls; true occlusion remains deferred.

## Light-source manifest

One committed web manifest owns this exact first-wave vocabulary:

| Prop ref | Family | Relative role |
|---|---|---|
| `dnd5e:props:brazier` | warm fire | strongest, widest room source |
| `dnd5e:props:torch-ornate` | warm fire | focused medium wall source |
| `dnd5e:props:candle-stand` | warm fire | soft medium source |
| `dnd5e:props:lantern` | warm fire | compact source |
| `dnd5e:props:candles` | warm fire | small local pool |
| `dnd5e:props:glowing-orb` | cool arcane | strong blue/cyan source |
| `dnd5e:props:rune-pillar` | cool arcane | medium blue source |
| `dnd5e:props:rune-marker` | cool arcane | subtle low blue source |

Each immutable entry carries color, point-light intensity, radius/falloff,
model-relative height, and floor-pool strength. Exact values are calibrated in
the integrated crypt, not treated as provider measurements or author-facing
configuration.

The builder's Lighting category derives from these keys. A key cannot appear as
Lighting while remaining visually inert. The old author-walk and legacy mood
paths consume this manifest where they remain live, or stop claiming authority.

The plain `dnd5e:props:torch` (`TorchStick`) and
`dnd5e:props:stone-lantern` remain ordinary props in this version. Names do not
make an asset luminous.

## Shared data and component flow

`buildScene3D` retains the atlas lighting facts it currently drops and produces
one static scene description equivalent to:

```ts
type DungeonLightingFacts = {
  regionByCell: ReadonlyMap<string, string>;
  intensityByCell: ReadonlyMap<string, number>;
  sources: readonly DungeonLightSource[];
  issues: readonly DungeonLightingIssue[];
};
```

Each source contains its stable prop identity, ref, region, world position, and
immutable preset. No rule calculation enters this adapter.

A pure resolver combines those facts with the current world-space view focus
and returns one complete plan:

```ts
type DungeonLightingPlan = {
  mode: 'crypt' | 'legacy';
  ambientIntensity: number;
  directionalIntensity: number;
  directionalPosition: readonly [number, number, number];
  pointLights: readonly DungeonPointLight[];
  floorExposureByCell: ReadonlyMap<string, number>;
  floorPools: readonly DungeonFloorPool[];
  diagnostics: readonly DungeonLightingDiagnostic[];
};
```

A shared `DungeonEnvironment` component replaces duplicated composition in
`DungeonPreview3D` and `SessionCanvas`. It owns:

- lighting-plan resolution;
- `DungeonSceneLights`;
- `DungeonShell` and its floor-light inputs; and
- atlas prop rendering.

Characters, monsters, interaction overlays, door commands, and camera controls
remain outside this component. They respond to its scene lights without
becoming lighting owners.

The flow is:

```text
Atlas regions + atlas props
          |
      buildScene3D
          |
  DungeonLightingFacts
          |
  resolveDungeonLighting(focus)
          |
  DungeonEnvironment
      |              |
scene point lights   region floor exposure + source pools
```

The builder passes its preview target as focus. Gameplay passes the local
player's position. Under the same scene and focus, both routes resolve the same
plan and render the same environment leaves.

## Performance budget

The existing mood-light precedent caps real point lights at 12. This slice
preserves that bound:

- select the nearest 12 source props to the current focus;
- break equal-distance ties by stable cell key and ref;
- use the same active selection for real point lights and floor pools; and
- show a builder diagnostic when authored sources exceed the active budget.

Every recognized prop remains a source; the render budget determines which
sources are active for the current view. As the player focus moves, nearby
sources enter the active set deterministically.

This is a rendering bound, not a dungeon validation rule. No toolkit or API
limit is added.

## Failure and fallback

Lighting-plan resolution is all-or-nothing.

Use the complete current legacy rig and floor treatment when:

- regions are absent;
- a region cell or intensity is malformed;
- cells have conflicting region ownership;
- the effective archetype is absent, unknown, mixed, or unsupported; or
- the lighting plan otherwise cannot be constructed deterministically.

Mixed **intensity** is supported. Mixed **archetype** remains the shell's
existing version-one fallback boundary.

Unknown prop refs simply do not emit. A recognized source ref continues to
emit if its GLB temporarily renders through the existing placeholder: the
atlas ref is the light-source fact, while model loading has its own visible
fallback.

The builder names fallback and source-budget diagnostics. Gameplay remains
usable and does not expose developer diagnostics as game content.

## Verification

Implementation follows test-driven development around pure lighting functions
before renderer wiring.

### Pure contract tests

- the manifest contains exactly the eight approved refs;
- the builder Lighting category derives from the manifest;
- mixed region intensities map to the correct cells;
- source strength is identical in intensity `0` and intensity `1` regions;
- authored planar/elevation offsets move source origins correctly;
- floor pools cannot affect a different region;
- intensity `0`, a midpoint, and `1` produce distinct deterministic base
  exposure;
- nearest-12 selection and tie-breaking are stable; and
- every malformed lighting shape activates the complete legacy plan.

### Renderer tests

- legacy scenes retain ambient `0.6` and directional `0.8` exactly;
- valid crypt scenes mount the calibrated low fill and expected point lights;
- point-light positions, colors, radii, and strengths match the resolved plan;
- the floor remains `MeshBasicMaterial` with `toneMapped={false}`;
- absolute-world floor UVs are unchanged;
- floor composition applies regional exposure before source pools; and
- builder and game mount `DungeonEnvironment` rather than composing separate
  light/shell/prop paths.

### Visual gate

Use the real reference crypt with its `0.6 -> 0.4 -> 0.15` regions and place:

- a warm torch or brazier in a dark region;
- a small candle or lantern source;
- a cool orb or rune source; and
- one intentionally source-free dark area.

Capture one builder/game pair plus a close view proving local floor and wall
illumination. Start from the older calibrated mood-light values, allow one
focused adjustment round, and let Kirk choose the final treatment.

Run focused tests during implementation, then one full web `npm run ci-check`.
Request one Copilot review on the implementation PR, address or explicitly
decline every finding, and never request a second round. Kirk performs visual
approval and merge.

## Delivery

This design is the current form of rpg-project#190 under journey #169.
By Kirk's explicit 2026-08-27 ruling, that project issue also directly owns the
one `rpg-dnd5e-web` implementation branch and PR against `dev`; this slice does
not create a second web issue. The web PR uses
`Closes KirkDiggler/rpg-project#190` as cross-repository completion intent.

This is a deliberate lean exception to the normal owning-repository issue and
one-issue-per-PR rules: #190 also owns this design/plan PR, which remains open
until implementation completes. No provider, toolkit, proto, API, deployment,
or Project 19 mutation belongs to worker sessions for this slice. Because web
`dev` is not its default branch, the board-management session verifies issue
closure after merge and reconciles it manually if GitHub defers the closing
keyword until `dev` reaches `main`.

## Not now

- fog, sight, line of sight, darkvision, reveal, or any mechanical visibility;
- shadows or wall-occluded point-light propagation;
- authored source color, radius, strength, or source type;
- animated flicker, flame VFX, particles, or audio;
- carried lights or lights attached to characters/monsters;
- more light-source prop refs;
- non-crypt lighting profiles;
- true regional shader/material volumes across every 3D surface; or
- changing the static session dungeon-name banner.
