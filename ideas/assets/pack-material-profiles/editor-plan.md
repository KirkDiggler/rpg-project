# Direct material editor: implementation plan

> Supplements [editor-amendment.md](editor-amendment.md) (approved at `9901ec0`)
> and [design.md](design.md); supersedes plan.md Tasks 3–4 semantics for the
> editor loop (family-option review stays available for suggestions).
> Execution: one GLM implementer, serial slices, no agents. Parent owns
> acceptance and publication.

## What the amendment changes in the shipped mechanism

Today the profile selects discovery-generated **options** (base color + normal +
wrap only), and every trial reruns the Blender batch through
`prepare_material_review`. The amendment replaces that loop with **direct
editing**: reusable material definitions owning independent channels
(base-color image | neutral solid, normal image | none + strength, emissive
image | none + color + strength, explicit wrap), assigned to scopes (a family
group default with explicit per-slot overrides), applied **live in the browser**
to slot-preserving preview geometry, and later compiled by Assets against the
original source for the real exported GLB. The vendor slot label never gates an
image choice; wrong-looking combinations are allowed and only technical errors
(failed loads, non-finite settings, invalid targets) stop.

Shipped pieces that carry forward unchanged: audited source-first slot facts
(`audit_material_sources.py` / `material_source_audit.py`), slot-preserving
neutral-policy conversion (`fbx_to_glb.py --unbound-material-policy neutral`,
`material_review_layout.py`), content-addressed preview staging, the Lab
route/scene, and the whole approval/activation boundary. Legacy v1 profiles and
the family-option mechanism stay readable and functional.

## Architecture: who owns what

```
Profile (editable, human)      v2: definitions (channel recipes) + assignments
                               (family default + explicit slot overrides)
                               + legacy selections (readable, conflict-checked)
Assets discovery/prepare (generated): inventory of pack images, source audits,
                               editor geometry GLBs, served image URLs,
                               resolved recipes with fingerprints
Web editor (draft state):      live slot/definition editing + client-side
                               application to slot-preserving GLB; exports the
                               same profile JSON
Assets compile (authoritative): recipe -> original source -> final GLB via
                               extended fbx_to_glb; normal Lab reviews it
Onward path (unchanged):       approval -> authority update -> cache generation
                               -> activation -> review/Ready/provider export
```

The profile stays the one editable choice document. Browser state is draft.
Definitions carry **paths and settings only** — never hashes; resolved recipes
own fingerprints and expanded bindings.

## Shared contracts (exact)

### Profile schemaVersion 2 (the editor's document)

Exact keys: `schemaVersion`, `profileId`, `packSlug`, `packVersion`,
`definitions`, `assignments`, plus optional `selections` (legacy family-option
choices carried through unchanged; absent = none). v1 documents parse exactly as
today. A family present in both `selections` and `assignments.familyDefaults` is
a conflict error (no order-dependent winner).

```json
{
  "schemaVersion": 2,
  "profileId": "dark-fortress-editor",
  "packSlug": "polygon-dark-fortress",
  "packVersion": "v3",
  "definitions": {
    "curve-wall-stone": {
      "label": "Curve wall stone",
      "baseColor": {"image": "SourceFiles/DarkFortress/Texture/Env/SmoothStone_Dark_Texture_01.png"},
      "normal": {"image": "SourceFiles/DarkFortress/Texture/Env/Brick_Medium_Normals_01.png", "strength": 1.0},
      "emissive": {"image": null, "color": "000000", "strength": 0.0},
      "wrap": "repeat"
    },
    "bare-clay": {
      "label": "Neutral clay",
      "baseColor": {"image": null},
      "normal": null,
      "emissive": null,
      "wrap": "repeat"
    }
  },
  "assignments": {
    "familyDefaults": {"Brick_Large_01": "curve-wall-stone"},
    "slotOverrides": [
      {"sourcePath": "SourceFiles/DarkFortress/FBX/SM_Bld_Wall_L_Curve_01.fbx",
       "objectName": "SM_Bld_Wall_L_Curve_01", "slot": 1, "definition": "bare-clay"}
    ]
  },
  "selections": {"SmoothStone_01": "smooth-stone-repeat"}
}
```

Rules:

- `definitions` values have exact keys `label`, `baseColor`, `normal`,
  `emissive`, `wrap` (no channel absent — import/export preserves every channel
  and explicit None). `label` bounded control-free string. `wrap`: `repeat` |
  `clamp`.
- `baseColor` is always an object with exactly `{image}`; `image` is a
  pack-relative path or `null`. `image: null` means the neutral solid base
  (constant `#666666` sRGB on both sides; see mapping below). A material always
  has a base, so base color has no "off" state.
- `normal`: `null` (channel off) or exactly `{image, strength}` with a
  pack-relative path (present channel must name an image).
- `emissive`: `null` (off) or exactly `{image, color, strength}`; `image` may be
  `null` (solid-color emissive), `color` is 6 hex digits (stored canonical
  uppercase), `strength` finite.
- `strength` (both channels): finite number, `0.0 <= strength <= 100.0`.
  Mapping: Blender Normal Map node `Strength` = s and glTF exporter writes
  `normalScale` from it; three.js sets `normalScale.set(s, s)`. Emissive:
  three.js `emissive` = color, `emissiveIntensity` = s, `emissiveMap` = image;
  Blender `Emission Color` = image/color, `Emission Strength` = s, and the
  exporter maps them to emissiveFactor + KHR_materials_emissive_strength. Both
  implementations consume the same numbers; cross-renderer pixel equality is not
  claimed, identical contract application is.
- `assignments.familyDefaults`: familyId → definitionId (definition must exist).
- `assignments.slotOverrides`: array of exact `{sourcePath, objectName, slot,
  definition}`; `sourcePath` is a pack-relative `.fbx` path, `objectName` exact
  name, `slot` int ≥ 0, unique targets. The referenced definition must exist.
  An override target that does not exist in the catalog fails resolution loudly
  (changed source membership/identity is disclosed, never silently dropped or
  silently applied). Unmatched vendor declarations cannot fabricate a target.
- Fingerprints/hashes appear nowhere in the editable document; resolution owns
  them.

### Editor inventory (generated, served with the preview manifest)

Manifest `schemaVersion: 3` adds `inventory` and carries the v2 profile
verbatim:

```json
"inventory": {
  "schemaVersion": 1,
  "images": [{"path": "SourceFiles/DarkFortress/Texture/Env/SmoothStone_Dark_Texture_01.png",
              "sha256": "<64hex>", "bytes": 12345,
              "url": "/models/synty/asset-review-materials/images/<sha256>.png"}]
}
```

Sorted by `path`; `url` serves the original image bytes (source quality,
content-addressed, ignored by Web Git). Folders/search are derived client-side
from `path`.

### Editor geometry

Existing prepared preview GLBs already preserve slots and remain usable as
editor geometry. Sources the sampler did not cover (e.g. the curved wall) get
one neutral, slot-preserving editor GLB on explicit request (launcher/prepare
`--editor-source`, repeatable; deterministic sampler never silently omits a
named source). GLB slot identity = (node name, material index); the manifest
carries each preview's `bindingObjectNames` (declared object → exported node
names) from the existing layout report so the editor can label slots.

### Resolution output (Assets, slice 1)

`resolve_profile_v2(profile, catalog, source_root) -> RecipeResolution` with
`bindings` (per catalog use: source/object/slot/declared material, definition
id, resolved channels with verified texture paths+sha), `unresolved_uses`
(families without a default, minus overridden slots), `unused_definitions`,
`overrides` (which target used a group default vs an explicit override — the
UI shows the exact affected objects/slots/assets and the return-to-group
state), and `profile_sha256`/`catalog_sha256`. v1 `resolve_profile` is
unchanged.

## Serial slices

**Slice 1 — Direct-edit contract in Assets (proposed first slice).**
Create `scripts/synty_material_recipe.py` (channel/definition records, strict
parse/serialize, canonical fingerprints, `resolve_profile_v2`, assignment
semantics) and extend `scripts/synty_material_profile.py` (`parse_profile`
dispatches v1/v2, serialization covers both). Tests:
`test_synty_material_recipe.py` (new) + `test_synty_material_profile.py`
(extended). Synthetic files only; no Blender, no web, no manifest changes.
Red→green: `PYTHONPATH=scripts python3 -m unittest test_synty_material_recipe
test_synty_material_profile -v`. Cases: v1 unchanged; v2 round-trip; explicit
None preserved; every malformed channel/assignment rejected (NaN/inf strength,
out-of-range, bad color, path-shaped IDs, duplicate override targets, unknown
definition, legacy+assignment conflict, wrong pack); resolution expansion
exactness (group default minus overrides), stale-override loud failure,
drift-detected fingerprints, unused-definition disclosure, deterministic
serialization.

**Slice 2 — Prepare/manifest v3: inventory + editor geometry (Assets).**
Extend `prepare_material_review.py` + tests: manifest v3 with `inventory` and
the v2 profile; content-addressed image copies into the ignored review tree
(check-ignore gate as today); per-preview `bindingObjectNames`; neutral editor
GLBs for explicitly requested sources (`--editor-source` on the launcher path)
without disturbing sampled option previews; reuse/failure-injection tests
unchanged in kind.

**Slice 3 — Web editor loop (the first usable editing surface).**
Create `src/dev/asset-review/materialRecipe.ts` (mirrored v2 profile/definition
parsing, serialize, apply function) and `src/dev/asset-review/MaterialEditorLab.tsx`
(or a mode inside MaterialReviewLab): slot list from the loaded GLB, definition
editing per channel, inventory browse by folder + search + thumbnails, live
application to the loaded scene via `MeshStandardMaterial` (no batch rerun),
override display with return-to-group, save/export/import of the v2 profile.
Vitest with real THREE material objects and mirrored literal fixtures; full
`npm run ci-check` once at the PR boundary.

**Slice 4 — Assets compile: recipe → final GLB.**
Extend `fbx_to_glb.py` (recipe input adding emissive/normal-strength/neutral
base on top of existing bindings; Blender glTF export agreement) with a real
Blender synthetic fixture asserting the exported glTF JSON (baseColorTexture,
normalTexture scale, emissiveTexture/factor/KHR strength, sampler wrap).
Agreement test: the same definition numbers applied in three.js (slice 3
semantics) and read back from the exported GLB agree field by field.

**Slice 5 — Curved-wall end-to-end reference.**
Run the real Dark Fortress curved wall through edit → save → prepare →
exported GLB → normal Lab (private evidence, licensed files never committed).
Multi-slot proof: an edit leaves other slots/UVs/geometry untouched (layout
evidence), independent channels exercised including None.

**Slice 6 — Onward ingestion boundary (explicit approval gate).**
Approved recipe/scope → mechanical authority update → validated cache
generation → explicit activation → normal review/Ready/provider export, with
published bytes protected and unresolved neighbours retained. Reuses verified
transition mechanisms; no `trusted` flag, no silent published-appearance
changes.

Slices 1–5 complete the amendment's first end-to-end milestone; slice 6 is the
onward delivery. Each slice is one commit set per repo, green before the next.

## Safety and boundaries (unchanged from the brief)

Synthetic fixtures first; real Blender/source fixtures where slot/export
semantics matter. Images stay source-quality. No work in canonical checkouts,
active cache, user profile, or the registered workspace; experiments use fresh
temp roots. Licensed files never enter public Web. Legacy behavior preserved;
technical integrity strict; no invented trust flags. Web `npm run ci-check`
once at the PR boundary. Parent owns publication.