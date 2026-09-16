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
- `strength` (both channels): finite number ≥ 0 — reject booleans, negatives
  and non-finite values; no universal file-format ceiling (a UI slider range is
  not a schema limit). Mapping: Blender Normal Map node `Strength` = s, which
  the glTF exporter writes as `normalTexture.scale`; three.js applies it as
  `material.normalScale.set(s, s)` (its in-memory property name). Emissive:
  three.js `emissive` = color, `emissiveIntensity` = s, `emissiveMap` = image;
  Blender `Emission Color` = image/color, `Emission Strength` = s, exported as
  emissiveFactor + KHR_materials_emissive_strength. Emissive image and color
  multiply (in both renderers the sampled map is tinted by the color and scaled
  by the strength). Both implementations consume the same numbers; cross-renderer
  pixel equality is not claimed, identical contract application is.
- Color handling is explicit: stored colors are sRGB hex; the neutral solid base
  (`image: null`) is sRGB `#666666` converted to linear on each side (three.js
  via its color management, Blender via the standard sRGB→linear transfer); a
  base-color image uses an untinted (white) factor — never the neutral solid as
  a tint — and an emissive image is multiplied by the emissive color.
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

Sorted by `path`. Compilation keeps original source-image identity/quality;
browser URLs must not promise raw original bytes — this pack contains TGA and
other formats ordinary browser image loading may not support — so preparation
also identifies a browser-compatible lossless preview derivative (its own hash
and URL) where the original is not directly loadable. Folders/search are
derived client-side from `path`.

### Editor geometry

Existing prepared preview GLBs already preserve slots and remain usable as
editor geometry. Sources the sampler did not cover (e.g. the curved wall) get
one neutral, slot-preserving editor GLB on explicit request (launcher/prepare
`--editor-source`, repeatable; deterministic sampler never silently omits a
named source). **Source-slot identity is never inferred from GLB indexing**:
exporters may reorder, share or merge materials/primitives, and the existing
before/after layout reports do not prove glTF slot identity. The editor target
identity is `sourcePath + actual objectName + source slot index` (imported FBX
authority), and slice 2 must carry that identity explicitly into distinct,
verifiable editor materials and manifest mappings — including a tested
export/reload case with equal-looking slots — so slot targeting survives the
round trip.

### Resolution output (Assets, slice 1)

`resolve_editing_profile(profile, catalog, source_root, audited) ->
RecipeResolution` takes an explicit **audited actual-slot inventory** (per
source: actual object names, mesh-data names, actual slot indices, and the
established declared→actual object correspondence from the source audit).
Actual imported slots are the target authority: vendor declarations alone never
validate a target. Family defaults expand only through established actual
membership (a declared ghost slot absent from the actual source is disclosed,
never bound); a real audited slot absent from the vendor list remains directly
editable via an explicit slot override. Result fields: `recipe_bindings` (per
established actual slot: source/object/slot, definition id, resolved channels
with verified texture paths+sha, scope = family-default vs slot-override, plus
the overridden group default for return-to-group display), `option_bindings`
(legacy selections, v1 mechanism), `unresolved_uses`, `unassigned_actual_slots`
(visible, directly editable), `unused_definitions`, and
`profile_sha256`/`catalog_sha256`. v1 `resolve_profile` is unchanged.

## Serial slices

**Slice 1 — Direct-edit contract in Assets (approved first slice).**
Create `scripts/synty_material_recipe.py` (channel/definition records, strict
parse/serialize, canonical fingerprints, audited actual-slot targeting,
`resolve_editing_profile`, assignment semantics). `parse_profile` stays
v1-strict so legacy callers explicitly reject v2 — no silent v2 acceptance
until downstream consumers are wired (slice 2+); a separate v2 parser lives in
the recipe module. A null legacy selection means no choice; only a non-null
legacy selection conflicts with a family default. Tests:
`test_synty_material_recipe.py` (new) + `test_synty_material_profile.py`
(extended). Synthetic files only; no Blender, no web, no manifest changes.
Red→green: `PYTHONPATH=scripts python3 -m unittest test_synty_material_recipe
test_synty_material_profile -v`. Cases: v1 unchanged and v2 loudly rejected by
the legacy parser (including the existing all-null template); v2 round-trip;
explicit None preserved; malformed channels/assignments rejected (NaN/inf
strength, booleans, negatives, bad color, path-shaped IDs, duplicate override
targets, unknown definition, non-null legacy+assignment conflict, wrong pack);
resolution expansion exactness through audited actual membership (family
default minus overrides), real-but-undeclared actual slots directly editable,
declared ghosts disclosed and unbindable, overrides targeting ghosts rejected,
conflicting family defaults after alias reconciliation, source-hash drift,
unused-definition disclosure, deterministic serialization.

**Slice 2 — Prepare/manifest v3: inventory + editor geometry (Assets).**
Extend `prepare_material_review.py` + tests: manifest v3 with `inventory` and
the v2 profile; content-addressed images into the ignored review tree
(check-ignore gate as today), keeping original bytes for compilation and
serving browser-compatible lossless preview derivatives (own hashes/URLs)
where the original format is not browser-loadable; per-preview explicit
source-target identity mapping; neutral editor GLBs for explicitly requested
sources (`--editor-source` on the launcher path) without disturbing sampled
option previews; export/reload identity test with equal-looking slots;
reuse/failure-injection tests unchanged in kind.

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

**Slice 5 — Curved-wall end-to-end reference (checkpoint, not delivery).**
Run the real Dark Fortress curved wall through edit → save → prepare →
exported GLB → normal Lab (private evidence, licensed files never committed).
Multi-slot proof: an edit leaves other slots/UVs/geometry untouched (layout
evidence), independent channels exercised including None. This is an
intermediate checkpoint: it does not complete the amendment's ingestion handoff
(slice 6 owns source/material authority and cache custody).

**Slice 6 — Onward ingestion boundary (explicit approval gate).**
Approved recipe/scope → mechanical authority update → validated cache
generation → explicit activation → normal review/Ready/provider export, with
published bytes protected and unresolved neighbours retained. Reuses verified
transition mechanisms; no `trusted` flag, no silent published-appearance
changes.

The amendment's first end-to-end acceptance milestone is **one milestone
spanning slices 1–6**: reaching the normal Lab with proper source/material
custody depends on the authority/cache integration of slice 6, so slice 5
cannot honestly claim that handoff passed, and intermediate checkpoints are not
user-ready delivery. Each slice is one commit set per repo, green before the
next.

## Safety and boundaries (unchanged from the brief)

Synthetic fixtures first; real Blender/source fixtures where slot/export
semantics matter. Images stay source-quality. No work in canonical checkouts,
active cache, user profile, or the registered workspace; experiments use fresh
temp roots. Licensed files never enter public Web. Legacy behavior preserved;
technical integrity strict; no invented trust flags. Web `npm run ci-check`
once at the PR boundary. Parent owns publication.