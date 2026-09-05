# Pack-by-Pack Asset Ingestion and Prop Promotion Design

Status: approved by Kirk

Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)

Design slice: [rpg-project#366](https://github.com/KirkDiggler/rpg-project/issues/366)

## Purpose

Give asset collaborators a small, understandable toolchain for turning licensed Synty source packs into private browsable GLB libraries and then promoting a reviewed set of static floor props into the game.

The end-to-end flow is:

```text
preserved source archive
  -> configured pack conversion
  -> ignored local GLB cache
  -> tracked contact sheets
  -> human selects 10-20 props
  -> in-game calibration property sheet
  -> one exported batch recipe
  -> headless Blender normalization
  -> reviewed private provider promotion
  -> generated web catalog
  -> Dungeon Builder and playable game
```

A collaborator chooses and calibrates props visually. Scripts own cache paths, source hashes, Blender baking, provider publication, and generated metadata. Opening a source GLB in Blender remains optional for close inspection or intentional modeling work.

## Pack-by-pack ingestion decisions

Each pack is handled separately with Kirk. Before conversion, record:

- whether the pack contains models, animation, 2D art, or deferred Unreal content;
- the correct atlas texture;
- the approved static scale profile;
- source model families to include or exclude; and
- preview camera and lighting settings.

These facts live in one reviewed JSON configuration under `scripts/configs/synty-packs/`. After configuration, conversion is mechanical. New decisions are added only when a family genuinely needs a different atlas or treatment.

## Ingestion tools

### `index_synty_archives.py`

Indexes every ZIP and Unity package in the licensed local source collection. It records archive/member names, source-relative paths, formats, sizes, and hashes in the private catalog. It never modifies or deletes source archives.

### `convert_synty_pack.py`

Runs one configured model family, such as `characters`, `items`, or `props`. It delegates FBX conversion to `fbx_to_glb.py`, launches Blender with factory settings so personal add-ons cannot affect the run, preserves original source directories, and writes a manifest of source and output hashes.

### `fbx_to_glb.py`

Imports FBX, assigns the selected Synty atlas, preserves rigged characters, applies the configured pack scale and static-object floor alignment, and exports GLB.

### `render_glb_previews.py`

Imports converted GLBs in one background Blender process. For props it renders independently framed primary and 180-degree reverse orthographic views using neutral lighting and a soft ground shadow. Those view PNGs live only in ignored `.preview-cache/`.

### `build_preview_sheets.py`

Builds the tracked browsing surface from cached render views. Prop tiles show both opposing views plus source filename, short GLB hash, and measured dimensions. Sheets retain the original source directory, split large directories by meaningful filename family, and paginate at 20 assets.

## Ingestion output and security

Raw archives remain local, immutable, and never automatically deleted. Temporary extracted sources may be removed after verified conversion.

Bulk converted GLBs and disposable render views are local implementation details:

```text
library/<pack>/v<version>/.glb-cache/
library/<pack>/v<version>/.preview-cache/
```

Both paths are ignored. The normal collaborator workflow does not require opening, copying, or organizing them by hand. Licensed collaborators regenerate them from their own source archives and the reviewed pack configuration.

Tracked private discovery output contains configurations, contact sheets, and portable sheet manifests. Individual preview PNGs are not tracked. The 39 already-merged Fantasy Kingdom character GLBs are grandfathered at their documented paths; future bulk families use the ignored cache.

`library/` is for discovery. `harness/` contains only separately reviewed runtime assets. No source archive, bulk cache, temporary calibration GLB, or licensed GLB enters a public repository.

## Verified reference ingestion

POLYGON Fantasy Kingdom v5 established the reference process:

- 39 character-family GLBs and five character sheets;
- 272 item GLBs in ignored cache and 75 item sheets;
- 899 prop GLBs in ignored cache and 152 paired prop sheets;
- four colliding same-named FBX pairs preserved through source-directory paths; and
- bounds-scaled lighting and opposing prop views accepted by Kirk.

POLYGON Dark Fortress v3 established the second pack profile:

- measured `dark-fortress: 1.0` scale profile;
- reviewed Dark Fortress atlas;
- 352 visual `SM_Prop_*` GLBs in ignored cache;
- 31 collision helpers excluded; and
- 98 paired prop sheets.

The paired-sheet update covers 1,251 props across both packs. It keeps 2,502 primary/reverse renders in ignored cache, tracks 250 sheets, removes redundant tracked individual previews, and reduces the current tracked asset tree by approximately 386 MiB. Kirk accepted the paired presentation as **“oh yeah that is nice.”**

## First prop-promotion boundary

Version 1 promotes a human-selected batch of 10-20 props with these limits:

- static and rigid;
- one GLB per placeable prop;
- floor-standing;
- visually calibrated against one standard fighter, a hex, and representative dungeon lighting;
- normalized by scale, base yaw, horizontal centering, and ground contact; and
- authored as obstacle, cover, or decor with explicit movement and line-of-sight behavior.

Wall-mounted and ceiling-mounted props, multi-part assemblies, particle companions, animation, runtime physics, and irregular multi-cell occupancy are later slices. A candidate outside the v1 boundary is refused rather than approximated.

## Calibration preparation

A provider-owned `prepare_prop_calibration.py` command accepts a pack identity and one contact-sheet source path. It:

1. resolves the source through the pack's ignored conversion manifest;
2. verifies the source GLB exists and matches its recorded hash;
3. copies it to the web checkout's ignored `public/models/synty/prop-calibration/` directory;
4. updates an ignored local calibration catalog; and
5. prints the exact loopback Prop Calibration Lab URL.

The command may be repeated to add candidates to the same working batch. The temporary copy is never a promoted asset and is never staged in Git. No local daemon or repository-writing browser service is introduced.

## Prop Calibration Lab

The Prop Calibration Lab is a local development tool, not a product concept. Its code lives under `rpg-dnd5e-web/src/dev/prop-calibration/`; it receives no Concepts Lab tab, Home link, or Dungeon Builder navigation entry. The existing historical Asset Anchor Lab remains unchanged.

The direct route is `http://127.0.0.1:<vite-port>/?propCalibration=1`. It renders only when all three conditions hold:

1. Vite runs in `development` mode;
2. `window.location.hostname` is loopback (`127.0.0.1`, `localhost`, or the browser's `::1`/`[::1]` IPv6 representation); and
3. the explicit `propCalibration=1` query parameter is present.

The app uses a development-conditional lazy import, following the existing local harness pattern, so a production build cannot activate the lab and can omit its implementation from the production bundle. A production build served from loopback still refuses the route; a development server deliberately exposed on a non-loopback hostname also refuses it.

The lab loads the temporary candidate catalog and shows one selected prop:

- on the real hex/floor presentation;
- beside a standard fighter for scale judgment;
- under orbit and representative play cameras;
- through all supported compass facings; and
- as raw and calibrated overlays.

The lab starts a property-sheet row for the prepared source. Source identity, source hash, measured bounds, and the v1 `floor` placement kind are supplied and read-only.

The collaborator fills or adjusts:

- display name;
- three-part family ref;
- exact four-or-more-part ref;
- whether this asset is the family default;
- bake scale, initially `1.0`;
- base yaw in degrees;
- automatically derived center/floor alignment plus an optional fine adjustment;
- role: `obstacle`, `cover`, or `decor`;
- themes;
- `blocksMovement`;
- `blocksLoS`; and
- notes.

Form inputs and visual controls share one state: changing a slider updates the exact value, and typing a valid value updates the preview. The preview applies the proposed bake transform followed by the game's existing shared `SYNTY_SCALE`, so it represents the final runtime convention rather than inventing a second scaling system.

The lab autosaves draft state in browser local storage under a prop-calibration-specific key. It can import a prior batch JSON, export incomplete draft JSON for backup, and export a provider-ready batch only when every required field passes local validation. Browser storage is convenience, not authority. The exported portable JSON becomes the provider recipe. It makes no API call and receives no filesystem-writing capability.

## Reference identity

New props distinguish semantic family from exact visual identity:

```text
dnd5e:props:market-stall
  -> reviewed default exact variant

dnd5e:props:market-stall:01
  -> exactly one promoted GLB
```

More descriptive additional segments are allowed, for example `dnd5e:props:market-stall:blue-canopy:01`.

Rules:

- every exact ref has at least four non-empty segments and resolves exactly one promoted GLB;
- every family ref has exactly three non-empty segments;
- every segment uses only letters, digits, `_`, or `-`;
- exactly one promoted exact ref is the default for each newly declared family;
- a family ref resolves only that reviewed default;
- exact refs never randomize, substitute, or fall back to the family default;
- unsupported exact refs render empty and surface a diagnostic;
- new Builder placements persist exact refs; and
- existing three-part authored content remains valid through the default alias.

Gameplay behavior is never inferred from ref spelling. Movement and line-of-sight flags remain explicit authored facts.

The encounter prop seam already carries the ref as an opaque string. Before publication, an end-to-end test must prove that Builder serialization, API persistence/projection, and web resolution preserve every segment. Any `core.Ref` or proto bridge on that path must support the full tail; a parser that still requires exactly three segments cannot be introduced into this seam.

## Batch recipe

The lab exports one versioned recipe with a batch ID and ordered entries. A generated entry contains portable source identity rather than an absolute machine path. Conceptually:

```json
{
  "$schemaVersion": 1,
  "batchId": "dark-fortress-floor-props-v1",
  "entries": [
    {
      "source": {
        "packSlug": "polygon-dark-fortress",
        "packVersion": "v3",
        "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Example_01.fbx",
        "glbSha256": "<generated>"
      },
      "displayName": "Example",
      "familyRef": "dnd5e:props:example",
      "ref": "dnd5e:props:example:01",
      "defaultForFamily": true,
      "calibration": {
        "scale": 1.0,
        "yawDegrees": 0,
        "fineOffsetMeters": [0, 0, 0]
      },
      "placement": "floor",
      "role": "decor",
      "themes": ["crypt"],
      "blocksMovement": false,
      "blocksLoS": false,
      "notes": ""
    }
  ]
}
```

A tracked template contains the schema and one visibly incomplete starter row. The preparation command normally fills source identity automatically, so the collaborator does not need to understand `.glb-cache` paths or compute hashes.

The provider derives the runtime filename from the exact-ref segments after `dnd5e:props`, joined by `--`: `dnd5e:props:market-stall:blue-canopy:01` becomes `props/market-stall--blue-canopy--01.glb`. The recipe never asks the collaborator to invent a second output identity. Existing legacy prop filenames remain unchanged.

## Provider promotion

A provider-owned `promote_props.py` command exposes `stage`, `validate`, `apply`, and `check` operations. `validate --only <exact-ref>` may inspect one row during calibration, but publication remains batch-atomic.

`stage` performs no canonical mutation. It:

1. validates schema, required fields, portable paths, refs, family defaults, uniqueness, finite transforms, v1 placement scope, and source hashes;
2. builds every candidate in a disposable repository-root-shaped stage;
3. invokes Blender in factory mode to bake scale, base yaw, centering, grounding, and fine adjustment into the GLB;
4. exports identity-transform runtime geometry compatible with the existing shared `SYNTY_SCALE`;
5. verifies embedded textures, finite geometry, floor contact, bounds, output hashes, and path collisions; and
6. generates the complete staged prop catalog and dependent reports.

`apply` first repeats validation against the exact staged bytes. It then records the canonicalized batch recipe at `scripts/configs/prop-promotion/<batchId>.json`, installs all staged GLBs under `harness/models/synty/props/`, and swaps generated metadata as one transaction. Any failure leaves canonical outputs unchanged.

Tracked batch recipes become the source of truth for newly promoted props. Existing `library/prop-role-map.json` remains the legacy source for already-promoted props. `build_prop_manifest.py` merges both sources, rejects conflicting exact refs/files/defaults, measures the actual promoted files, and generates the combined runtime manifest.

The provider regenerates and verifies:

- `harness/models/synty/props/manifest.json`;
- the compact license-safe `harness/catalogs/synty-props-web.json` catalog;
- mesh statistics;
- the complete Synty runtime inventory; and
- deterministic promotion evidence containing source/output hashes and measured bounds.

`check` rebuilds expected metadata from tracked recipes and promoted bytes and fails on drift.

## Web consumption

After the private provider PR merges, one web command:

1. locks or verifies the merged provider revision;
2. runs the existing private asset synchronization;
3. consumes the compact provider prop catalog;
4. generates committed, license-safe TypeScript resolver metadata; and
5. checks that generated metadata and locally synced files agree.

Exact refs are selectable Builder entries grouped by family. Three-part family aliases resolve defaults but do not create duplicate palette tiles. Calibration preview, Builder preview, and playable dungeon use the same generated resolver and `PropModel` path. New code must not maintain a second hand-written list of the promoted batch.

The synchronized GLBs remain ignored in the public web checkout. Only license-safe ref, label, role, behavior, dimensions, runtime size/hash, and path metadata may be committed there.

## Error handling

- Preparation refuses an unknown source path, absent cache, stale conversion manifest, or hash mismatch.
- The lab marks incomplete/invalid fields inline and cannot label an incomplete batch provider-ready.
- Draft export remains available so unfinished work is not lost.
- Provider validation reports errors by entry/ref and mutates nothing.
- Duplicate exact refs, duplicate runtime paths, multiple defaults, absent defaults, unsupported placements, and changed source hashes are hard failures.
- Calibration values must be finite; scale must be greater than `0` and at most `100`; yaw is normalized into `[-180, 180)`; horizontal fine offsets must remain within `[-0.5, 0.5]` metres; and vertical fine offset must remain within `[-0.1, 0.1]` metres.
- The baked result's lowest visible point must match the declared vertical fine offset within `0.01` metres. Zero is normal floor contact; the narrow adjustment exists only for a visually reviewed slight sink/lift.
- Every final bounds axis must be positive and no greater than `20` metres in this floor-prop slice; larger architecture returns to a later placement design.
- Unsupported exact refs never render a misleading substitute.
- Consumer generation fails before writing when provider metadata or synced files disagree.

## Verification

Ingestion remains verified through real pack acceptance runs plus focused deterministic checks.

The calibration and promotion slice adds:

- preparation tests for source-path resolution, hashes, ignored destination, and traversal refusal;
- lab tests for the development/mode/loopback/query route matrix, production refusal, catalog loading, property-sheet validation, control/form synchronization, local draft recovery, and JSON import/export round trip;
- recipe tests for exact/family ref rules, default uniqueness, required booleans, finite calibration, and portable paths;
- provider tests proving stage purity, source binding, Blender bake results, floor contact, identity output transforms, atomic apply, and `check` drift detection;
- generated-catalog tests proving exact resolution, family-default resolution, unsupported-exact emptiness, and no duplicate Builder aliases;
- an exact four-or-more-part ref round trip through Builder save, reopen, Save & Play, API projection, and gameplay rendering; and
- human review of all staged props beside a fighter and in one representative dungeon under play lighting.

Runtime inventory and repository-wide existing checks must remain unchanged except for the intentionally promoted files and regenerated metadata.

## Execution guardrail

Journey #365 does not use spawned subagents or autonomous review/fix loops. Work is inline and bounded. Independent review, if requested, is a separately initiated top-level session at the final PR head.

No daemon, database, generalized transaction framework, lock manager, or production asset service is introduced. The provider may reuse the repository's existing static staging/atomic-swap helpers rather than inventing another framework.

## Out of scope

- automatic promotion of every converted prop;
- wall, ceiling, or tabletop anchoring;
- multi-part prop assembly or companion effects;
- irregular multi-hex collision geometry;
- animation, physics, destruction, or interaction behavior;
- held-item placement, posing, and equipment attachment;
- changing gameplay behavior based on visual ref names; and
- exporting Sidekick assets from Unreal.

## Completion criteria

The first promotion slice is complete when:

- Kirk can prepare 10-20 selected cached GLBs without navigating `.glb-cache`;
- the lab shows each beside a fighter and captures a complete property sheet;
- one portable batch JSON round-trips through export/import;
- the provider stages, validates, bakes, and atomically applies the full batch;
- three-part family refs resolve their declared defaults;
- exact four-or-more-part refs resolve only their declared GLBs;
- the generated catalog removes hand-copying for the new batch;
- the Builder exposes the exact variants without duplicate alias tiles;
- save, reopen, Save & Play, and gameplay preserve exact refs and authored behavior; and
- no licensed cache or runtime GLB is tracked by a public repository.
