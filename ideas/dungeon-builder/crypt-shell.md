# Archetype-driven crypt shell — design

*Issue: rpg-project#284 · Parent journey: rpg-project#169 · Team: Assets*

## Outcome

Raise the dungeon’s environmental quality by making `archetype: crypt` select
one coherent floor-and-wall shell in both the dungeon builder’s 3D preview and
the playable session.

The first version deliberately supports one effective archetype for the whole
dungeon. It does not add floor or wall pickers. A crypt gets its complete
approved treatment automatically; topology, movement, sight, doors, authored
wall height, and YAML remain mechanics-owned facts exactly as they are today.

The visual target is an **intact worked crypt**: calm continuous flagstone,
finished two-sided masonry, coherent bases and caps, and architecture that lets
rubble, bones, ritual pieces, and other props carry localized decay.

![Current shell before the archetype pass](evidence/crypt-shell/before-shell.png)

The current props read as authored objects, but the shell does not yet match
them:

- the floor resets the same texture coordinates on every hex and repeats the
  texture twice inside each cell, producing a dense patchwork;
- wall runs repeat and non-uniformly fit one half-wall GLB, with a placeholder
  floor-skirt box and no finished cap treatment;
- the Synty wall piece has a finished front and an unfinished back, so a free
  camera exposes the wrong side of roughly half the possible views
  (rpg-dnd5e-web#791);
- `AtlasRegion.archetype` reaches the client but does not currently select the
  shared floor and wall presentation.

## Decisions

1. **One effective archetype per dungeon for version one.** The existing wire
   remains region-shaped for the future, but the first resolver activates a
   profile only when every atlas region names the same known archetype.
2. **`crypt` selects the whole shell.** There are no individual floor texture,
   wall family, random-variant, or per-region presentation controls.
3. **Continuous masonry, invisible permanent hexes.** Hexes remain the
   mechanical and rendering cells; their floor UVs share one absolute world
   frame so the stonework crosses cell boundaries. Movement, targeting, and
   authoring overlays reveal the grid when needed.
4. **Intact architecture first.** The base profile has no random broken or
   alcove wall pieces and no rune/ornate floor plates. Damage remains localized
   dressing.
5. **True two-sided walls.** Every wall body has two finished masonry faces
   with correct geometry, normals, UVs, and materials. `THREE.DoubleSide` is not
   a substitute for an authored back face.
6. **Provider owns visual truth.** Exact source selection, derivative source,
   artifact bytes, calibration, hashes, bounds, provenance, and evidence live
   in private `rpg-game-assets`. The web consumes the generated runtime
   profile; it does not repair a bad wall or hardcode private calibration.
7. **Builder and game share the same resolver and leaves.** No builder-only
   floor, wall, profile, or artifact URL exists.
8. **Atomic profile or atomic fallback.** A missing, invalid, mixed, or unknown
   profile renders the complete legacy shell, never a new floor with old walls
   or the reverse. The builder names why it fell back.
9. **Lighting remains separate.** The floor keeps the established unlit,
   `toneMapped={false}` material path. Region-authored lighting is the next
   fidelity lane under journey #169, not hidden inside this shell slice.
10. **Rug overlay occlusion remains separate.** Movement markers currently sit
    below the approved rug; rpg-dnd5e-web#823 owns that shared overlay-depth
    defect and does not block this work.

## Existing asset inventory and ownership

The licensed asset flow has three distinct layers:

- Original archives remain outside Git, currently including
  `POLYGON_Dungeon_Pack_SourceFiles_v3.zip` and
  `POLYGON_Dungeons_Map_SourceFiles_v2.zip` under the operator’s Downloads.
- Complete converted private libraries live in
  `rpg-game-assets/library/polygon-dungeon/` (814 GLBs) and
  `library/polygon-dungeons-map/` (57 GLBs).
- The reviewed runtime subset lives under
  `rpg-game-assets/harness/models/synty/` and is mirrored by the web sync into
  its gitignored `public/models/synty/` tree.

The Dungeons Map pack is primarily supplemental book, tomb, and library
dressing. The main Dungeon pack contains the broad shell inventory: floor
textures and plates, full/half/textured/double-sided wall families, trims,
frames, corners, caves, ceilings, entrances, and structural pieces.

The first provider task is a bounded visual audition from those existing
private libraries. It compares the current promoted floor texture families and
suitable full/textured/double-sided wall, trim, and door-frame candidates at
real tactical scale. Kirk’s selected treatment becomes the one profile. Source
filenames are candidates, not consumer authority; only the approved generated
profile is authoritative.

## Runtime profile contract

`rpg-game-assets` generates a consumer-safe manifest at:

```text
harness/models/synty/env/shell-profiles.json
```

It is part of the private runtime tree synced into the web’s ignored static
asset directory. The web repository commits the schema/parser and tests, but
never licensed GLBs, source files, or a copied private manifest.

The version-one schema is equivalent to:

```ts
type Sha256 = string; // exactly 64 lowercase hexadecimal characters
type Vec3 = readonly [number, number, number];
type MeasuredBounds = { min: Vec3; max: Vec3 };

type ShellProfilesV1 = {
  schemaVersion: 1;
  profiles: {
    crypt: {
      floor: {
        diffuse: `textures/${string}.png`;
        sha256: Sha256;
        worldUnitsPerRepeat: number;
      };
      wall: {
        body: {
          file: `env/${string}.glb`;
          sha256: Sha256;
          localSpanAxis: '+X';
          localFaceAxis: 'Z';
          twoSided: true;
          bounds: MeasuredBounds;
        };
        base: { file: `env/${string}.glb`; sha256: Sha256; bounds: MeasuredBounds };
        cap: { file: `env/${string}.glb`; sha256: Sha256; bounds: MeasuredBounds };
        doorSurround: {
          file: `env/${string}.glb`;
          sha256: Sha256;
          bounds: MeasuredBounds;
        };
      };
    };
  };
};
```

The provider generator writes every exact approved path, measured value, and
digest and rejects any absent value. `worldUnitsPerRepeat` is the selected
floor’s measured visual calibration, not a consumer-side tuning knob.

Provider validation enforces:

- one immutable profile key and schema version;
- path containment under the licensed runtime root;
- exact file presence, size, and SHA-256 against the complete promoted
  inventory;
- finite measured bounds and declared local axes;
- compatible body/base/cap spans and floor contact;
- textured triangles with outward normals on both `+localFaceAxis` and
  `-localFaceAxis` for every wall body;
- deterministic generated bytes;
- checked-in private source provenance and required visual evidence.

A structural two-face result is necessary but not sufficient: front and back
renders are mandatory because a geometrically present face can still carry an
unfinished material.

## Profile resolution and shared rendering

The client validates the runtime manifest once and resolves an atlas using a
pure shared rule:

1. an atlas with no regions returns `no-regions`;
2. trim every region’s `archetype`; any empty or unrecognized value returns
   `unknown-archetype`;
3. exactly one distinct recognized value returns that complete immutable
   profile; more than one returns `mixed-archetypes`;
4. manifest load/schema failures return `manifest-unavailable` or
   `invalid-profile`;
5. every fallback selects the complete legacy floor and wall pair.

No server validation is added. The wire intentionally remains capable of mixed
region archetypes later. Version one simply declines to partially render a
shape it does not yet support.

`buildScene3D` carries the atlas archetype set beside its existing floor tiles,
wall runs, door gaps, and props. One shared shell component resolves the
profile and draws:

- the existing `SyntyHexFloor` leaf with profile floor inputs;
- the existing `AtlasWalls`/wall-run leaf with profile wall inputs.

Both `DungeonPreview3D` and `SessionCanvas` mount that same component. Props,
lights, camera, facing, offsets, and artifact URLs remain on their existing
shared paths.

Profile loading is one Suspense/error boundary. It waits for the complete
profile and its required shell resources; an error activates the complete
legacy shell. A failed wall body never leaves a themed floor standing alone.
The builder displays the named fallback reason. The playable route remains
usable and does not expose a developer diagnostic as game content.

## Floor presentation

The floor keeps one mesh per known cell. That preserves current fog, memory,
selection, and interaction behavior and avoids inventing a polygon-union or
large-plate clipping system.

The change is the UV frame. For each floor vertex:

```text
u = absoluteWorldX / profile.floor.worldUnitsPerRepeat
v = absoluteWorldZ / profile.floor.worldUnitsPerRepeat
```

The texture uses repeat wrapping. Absolute world origin is the anchor, so
adding or removing a distant cell never shifts existing stonework and the same
dungeon renders identically after reopen or reconnect. Negative world
coordinates are valid through repeat wrapping.

Consequences:

- adjacent cells evaluate the same UV at their shared world edge;
- the texture no longer restarts inside every hex;
- the selected scale can place larger stones across cell boundaries;
- no random per-cell rotation can break continuity;
- remembered/fogged cells may still change material color while retaining the
  same masonry alignment.

Version one has one diffuse treatment. It does not consume normal/mask maps,
random colorways, modeled square floor plates, ornate tiles, runes, cracks, or
procedural damage. Those can join a later profile without changing this world-
space contract.

`DUNGEON_SURFACE_Y = 0.2`, zero floor extrusion, prop grounding, and tactical
overlay heights do not change in this slice.

## Wall presentation

Authored edges and the existing canonical, order-independent wall-run engine
remain the sole geometry authority. The profile changes how each derived run
is dressed, not which walls exist.

### Wall body

The approved body is tiled along each run through the existing residual-width
strategy: choose a stable piece count and distribute the exact run length
across those pieces instead of creating a tiny last sliver. Profile bounds and
pivot calibration replace hardcoded measurements.

The body fills the run’s effective height. Existing standard height, authored
height multipliers, and optional cutaway calculation still produce that
height. Non-uniform scale remains geometry-baked with corrected normals through
the shared GLB primitive; visual approval must prove that the selected body
tolerates the required fit.

The body must be true two-sided geometry. No runtime duplicate-backface trick,
material-side override, or orientation choice is allowed to hide an unfinished
back. A camera orbit and a partition between two playable spaces must both show
finished masonry.

### Base and top

The base and cap are separate profile pieces because stretching one composite
wall would distort them whenever authored wall height changes.

- Base remains registered to `DUNGEON_SURFACE_Y` and overlays the body at the
  floor joint.
- Cap remains at the final effective wall height and overlays the body at the
  top joint.
- Neither trim’s own vertical thickness is multiplied by wall height.
- The current placeholder `FloorSkirtBox` is absent for an active crypt
  profile.

The selected trims must remain coherent at the optional cutaway stub height;
overlap is acceptable, missing architecture is not.

### Corners and junctions

The existing overlap-miter run extension remains the first-version corner
strategy. Body, base, and cap use the same canonical run endpoints and facing,
so they cannot derive different corners. The provider/game specimen includes:

- one straight seam;
- inside and outside turns;
- a three-run junction;
- both viewing sides.

A dedicated corner post is not introduced unless the selected family fails
that specimen. The previously investigated post-shaped Dungeon pieces are not
silently reused: their relief collapses under the wall-height fit and they have
already failed the intended role visually.

### Doors

Door state, click behavior, opening pose, gaps, and mechanics remain unchanged.
The crypt profile chooses a compatible surround registered to the same run
plane. It must meet the wall body/base/cap from both viewing sides and at
standard and raised height. There is no door style picker and no new door wire
field.

## Candidate and final visual gates

The provider slice has two human gates, both at real game scale:

1. **Candidate gate.** Compare floor texture families and viable
   full/textured/double-sided wall families in a compact shell specimen. Each
   view uses the tactical camera and includes a character for scale.
2. **Integrated gate.** Sync the exact approved provider tree into a web branch
   and capture this same dungeon in builder preview and Save & Play.

The integrated frame must include continuous open floor, a long wall, an
inside/outside corner, a junction, a closed or locked doorway, standard and
raised wall sections, props from the accepted crypt specimen wave, and a camera
angle that sees both wall faces. Kirk alone gives visual approval.

## Verification

### Provider

- candidate source and derivative provenance;
- deterministic shell-profile generation and `--check` mode;
- exact promoted inventory/path/size/SHA validation;
- measured bounds, pivots, axes, floor contact, and span compatibility;
- two-sided normal/material structural checks;
- mesh statistics and explicit warning disposition;
- neutral front/back and detail renders;
- tactical shell contact sheet and accepted-candidate ledger;
- clean web-stage verification of the complete provider tree.

### Web

- manifest parser rejects unknown fields, invalid versions, non-finite
  calibration, unsafe paths, absent required components, and false two-sided
  declarations;
- resolver tests cover uniform known, mixed, unknown, no-region, missing
  manifest, and invalid-profile cases;
- named adjacent-cell UV tests assert float-equal coordinates on their shared
  edge and stable UVs when unrelated cells are added;
- remembered and visible cells retain the same UV frame;
- wall transforms cover straight runs, residual widths, both facings, base and
  cap placement, standard/raised/cutaway heights, corners, junctions, and
  doorway alignment;
- profile-load failure renders the complete legacy pair;
- builder preview and session tests assert the same profile key, component,
  artifact paths, floor surface, wall runs, door gaps, and scene lights;
- focused renderer suites, format/lint/type/build, full `npm run ci-check`, and
  all required GitHub checks pass.

### Real-path evidence

- before/candidate/final builder frames;
- final playable frame from Save & Play;
- Save → reopen preserves the YAML and effective archetype;
- artifact URLs and provider hashes captured from the exact synced tree;
- Kirk’s candidate and integrated verdicts recorded verbatim.

## Delivery and issue reconciliation

This remains one cross-repository wave under rpg-project#284 and journey #169:

1. `rpg-game-assets`: one provider issue and PR for candidate selection,
   derivative sources if required, runtime artifacts, manifest, validation,
   provenance, and private evidence;
2. `rpg-dnd5e-web`: one consumer issue and PR for profile loading/resolution,
   continuous floor UVs, modular two-sided wall presentation, fallback, tests,
   and integrated public-safe screenshots;
3. provider merges before the web pins and consumes its exact merged tree;
4. the project design PR stays open until both slices land and the integrated
   evidence is recorded;
5. rpg-dnd5e-web#791 closes with the web slice;
6. rpg-dnd5e-web#823 remains separate;
7. journey #169 remains In Progress for authored lighting and later shell
   profiles.

## Not now

- mixed archetypes within one dungeon;
- a dungeon-level archetype wire field;
- per-floor, per-wall, per-region, or per-door visual controls;
- modeled square floor plates or merged floor geometry;
- random floor textures, random broken walls, procedural decay, runes, ornate
  plates, ceilings, caves, or additional shell profiles;
- low-wall mechanics, cover, elevation, or changes to movement/sight;
- authored lighting, shadows, VFX, or audio;
- rug/overlay layering (#823);
- new wall topology, corner identity, door mechanics, or YAML fields;
- runtime prop composition.
