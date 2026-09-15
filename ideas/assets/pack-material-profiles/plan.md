# Pack Material Profiles: Profile-and-Preview Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Execution for this task:** Kirk explicitly wants inline, visible work without
agents or heavyweight review loops. Use executing-plans inline; the generic
recommendation above does not override that instruction.

**Goal:** Let an operator edit a pack profile, run the existing review launcher,
compare script-prepared material options by family, and export/reuse their choices.

**Architecture:** Assets owns a pure profile/resolution contract and a staged
preview producer. Web owns a material-review view inside the existing Lab and
round-trips the same profile JSON. Both are separate from Ready/provider exports;
this first slice produces review-only proposals, not trusted cache activation.

**Tech Stack:** Python, existing Blender FBX→GLB converter, React/TypeScript,
existing Three.js AssetReviewScene, unittest/Vitest. No new runtime dependencies.

**Spec:** `ideas/assets/pack-material-profiles/design.md`, approved by Kirk in chat
at design head `7121be8e0e54053685723004d026fa1d4b2602d0` (Project PR451).

## Global Constraints

- “The product here is the reusable preparation process, not a textured floor or a published batch.”
- “Blender is an optional exception inspector, not the primary workbench.”
- “The profile is the one editable choice document.”
- “Browser state is only a draft; it does not become a second authoritative profile.”
- “Changing preview choices cannot mark all members Ready or publish them.”
- “Do not change previously published bytes incidentally.”
- “No manual checkpoint insertion substitutes for the operator's own execution.”
- Use isolated owning-repository worktrees: Assets issue217; create/link a Web
  issue for the consumer before its branch. Preserve canonical checkouts and all
  existing caches/Lab decisions. No real floors export/run is authorized.
- Normal hooks; focused tests during edits, Web `npm run ci-check` once at its PR
  boundary. No full unrelated Assets development suite. Human owns merges.
- Paths below are relative to the named owning repo. Plan/design live in Project.

## Scope and next boundary

This plan covers design slices1–2: resolver/profile, prepared options, viewer and
profile round-trip. Design slice3—canonical material authority, accepted-profile
cache activation and release binding—is a subsequent implementation plan. It is
not quietly skipped: the first slice must visibly refuse promotion/activation.
Do not claim normal ingestion is complete after this plan, or compensate with
manual pack edits, publication or journal records.

## Files and contracts

### Human profile (the same JSON in Python, Web and on disk)

```json
{
  "schemaVersion": 1,
  "profileId": "dark-fortress-review",
  "packSlug": "polygon-dark-fortress",
  "packVersion": "v3",
  "selections": {
    "Brick_Large_01": "brick-large-01-repeat",
    "SmoothStone_01": null
  }
}
```

IDs above illustrate the format, not shipped asset choices. Discovery supplies
real stable IDs and a template; the operator never invents internal slot IDs.
Exact keys only. IDs are nonempty bounded strings, not paths. Null means unresolved.
Missing family selections are also unresolved; foreign family/option IDs are
errors when resolving against a catalogue. No approval flag or hand-edited hash.

### Python public interfaces

Create `scripts/synty_material_profile.py` for typed records, strict parsing,
serialization and resolution. Use frozen dataclasses, deterministic ordering and
explicit errors. The JSON records emitted by `catalog_document` define the Web
contract; use camelCase on the wire, snake_case in Python.

```python
@dataclass(frozen=True)
class PackProfile:
    profile_id: str
    pack_slug: str
    pack_version: str
    selections: Mapping[str, str | None]

@dataclass(frozen=True)
class SlotUse:
    source_path: str
    source_sha256: str
    group: str
    object_name: str
    slot: int
    declared_material: str

@dataclass(frozen=True)
class MaterialOption:
    id: str
    label: str
    base_color: str          # pack-relative file
    normal: str | None       # pack-relative file, absence disclosed
    wrap: Literal['repeat', 'clamp']
    textures_sha256: Mapping[str, str]
    basis: Literal['reviewed-binding', 'authored-name-match', 'compatible-candidate']
    reasons: tuple[str, ...]

@dataclass(frozen=True)
class MaterialFamily:
    id: str
    label: str
    kind: Literal['atlas', 'tiling', 'unresolved', 'unsupported']
    uses: tuple[SlotUse, ...]
    options: tuple[MaterialOption, ...]
    recommended_option_id: str | None
    reasons: tuple[str, ...]

@dataclass(frozen=True)
class MaterialCatalog:
    pack_slug: str
    pack_version: str
    input_sha256: str        # source/config/declaration/texture facts
    families: tuple[MaterialFamily, ...]

@dataclass(frozen=True)
class ProfileResolution:
    bindings: tuple[ResolvedMaterialBinding, ...]
    unresolved_uses: tuple[SlotUse, ...]
    profile_sha256: str
    catalog_sha256: str

parse_profile(payload: bytes) -> PackProfile
profile_bytes(profile: PackProfile) -> bytes
catalog_document(catalog: MaterialCatalog) -> dict[str, object]
resolve_profile(profile: PackProfile, catalog: MaterialCatalog,
                source_root: Path) -> ProfileResolution
```

Import `ResolvedMaterialBinding` from the existing bindings module. Resolution
never changes its catalogue or source files. It expands choices to the existing
exact source/object/slot binding format, checks texture/source bytes again, and
rejects overlapping targets or unknown/incompatible options. Preview resolution
is not an approval or runtime recipe.

Create `scripts/synty_material_discovery.py` for:

```python
discover_materials(source_root: Path, pack_config: Mapping[str, object],
                   pack_version: str) -> MaterialCatalog
new_profile(catalog: MaterialCatalog, profile_id: str) -> PackProfile
```

`new_profile` emits null choices and lets the UI display recommendations
separately. Do not silently turn suggested matches into the operator's selection.

### Prepared preview contract

Create `scripts/prepare_material_review.py` for:

```python
prepare_material_review(config: WorkspaceConfig, profile_path: Path,
                        *, samples_per_family: int = 2) -> Path
```

Returns the installed ignored manifest path after an atomic stage. Emit a strict
schema-v1 JSON object containing `schemaVersion`, `mode: "material-preview"`,
`profile`, `profileSha256`, `catalogSha256`, `catalog` and `previews`.

Each preview has `familyId`, `optionId`, `sourcePath`, `objectNames`, `url`,
`glbSha256`, `layoutReportSha256` and `context` (other proposed/neutral families).
Catalogue families expose the full use list; previews expose their limited sample
coverage. URLs are constrained to `/models/synty/asset-review-materials/`.
No `readyEligible`, provider refs or runtime-promotion fields exist in this schema.

## Task 1: Strict editable profile and deterministic discovery/resolution

**Files (Assets):**
- Create `scripts/synty_material_profile.py`
- Create `scripts/synty_material_discovery.py`
- Create `scripts/test_synty_material_profile.py`
- Create `scripts/test_synty_material_discovery.py`
- Reuse `scripts/synty_material_preflight.py`, `scripts/synty_material_bindings.py`
  and `scripts/convert_synty_pack.py` discovery; do not fork their parsers.

**Consumes:** existing `audit_group`, `parse_material_declarations`, `plan_group`
and `resolve_material_overrides`. **Produces:** the Python interfaces above and
strict, deterministic profile/catalogue documents.

- [ ] Write profile tests using literal small documents. First failing case:

```python
def test_unknown_family_is_not_silently_ignored(self):
    profile = parse_profile(b'{"schemaVersion":1,"profileId":"review",'
        b'"packSlug":"fixture-pack","packVersion":"v1",'
        b'"selections":{"not-in-pack":"stone-01-repeat"}}')
    with self.assertRaisesRegex(ValueError, 'unknown family'):
        resolve_profile(profile, self.catalog, self.source_root)
```

  Fixture catalogue: two families, three uses; family stone has two matching uses
  in distinct sources and a third source has atlas trim. Two tiling options and
  one atlas option. Assert literal targets `(a.fbx, MeshA, 1)` and
  `(b.fbx, MeshB, 0)` only; trim must not be reassigned. All files are temporary
  synthetic data. Add null/missing selection, wrong pack/version, extra keys,
  unknown option, cross-family option and source/texture drift cases.
- [ ] Run `PYTHONPATH=scripts python3 -m unittest test_synty_material_profile -v`;
  observe the missing contract/behavior, not a broken fixture import.
- [ ] Implement the records/parser and resolver. Assemble existing bindings with
  pack-relative checked files and hashes; do not construct Blender shaders here.
  Compare profile/catalogue pack identities before resolving any selection.
- [ ] Add discovery fixtures with a vendor declaration, minimal configured FBX
  placeholders for pure discovery, and named image files: Stone_Texture_01,
  Stone_Normals_01, atlas01 A/B/C and atlas04 C. Add conflicting normal candidates,
  a similarly named but unrelated image, a missing map, a material alias and an
  FX/custom case. Shuffled directory order must give identical catalogue bytes.
- [ ] Run discovery tests red, then implement evidence-ranked candidates:
  existing reviewed bindings first; exact declared family/number texture pairs
  second. Filename token normalization is for suggestions only. Never merge
  conflicting material identities or suffix aliases merely by normalized name.
  Keep numbered atlas layouts separate. Unknown compatibility/shader behavior
  stays unresolved; surface substitution options require supported family facts,
  not an asset filename substring. Report why every candidate was suggested.
- [ ] Verify all fixture cases and commit with normal hooks:
  `git add scripts/synty_material_profile.py scripts/synty_material_discovery.py scripts/test_synty_material_profile.py scripts/test_synty_material_discovery.py`;
  `git commit -m "feat: resolve editable pack material profiles"`.

## Task 2: Slot-preserving proposal conversion and staged preview producer

**Files (Assets):**
- Modify `scripts/fbx_to_glb.py`
- Create `scripts/material_review_layout.py`
- Create `scripts/prepare_material_review.py`
- Create `scripts/test_material_review_layout.py`
- Create `scripts/test_prepare_material_review.py`
- Create `scripts/test_fbx_to_glb.py` for the new opt-in preview behavior;
  do not move unrelated converter tests.

**Consumes:** `discover_materials`, `resolve_profile`, existing Blender binding
and conversion helpers. **Produces:** `prepare_material_review` and manifest above.
`material_review_layout.layout_facts(objects: Iterable[bpy.types.Object]) -> dict[str, object]` runs inside
Blender and records object/slot structure plus topology, polygon material-index
and UV-layer fingerprints immediately before/after material assignment.

- [ ] Write a native Blender fixture with two mesh objects, three distinct slots,
  per-face slot indices, one UV layer and small generated PNG color/normal pairs.
  Record a literal expected assignment array `[0, 1, 2, 1]`. A missing binding must
  preserve that array and render a neutral slot—not collapse the object to atlas.
- [ ] Run the fixture against current `assign_materials`; observe the collapse
  behavior. Keep a legacy-mode control so unchanged default conversion behavior
  remains protected.
- [ ] Add opt-in `--unbound-material-policy neutral` (default remains `atlas`) and
  optional `--material-layout-report PATH` to the existing converter. For neutral
  preview mode, create one neutral fallback material and retain the original slot
  list length/index assignments with `replace_material_slots`. Explicit bindings
  replace only their target slots; known default-atlas slots are also explicit
  bindings in the generated preview request. No unreviewed slot receives atlas by
  accident. Compare layout facts around assignment and fail on layout mutation.
- [ ] Record imported material names as evidence, not an equality requirement
  against vendor labels: existing real sources can call a declared brick slot
  `lambert1`. Match the audited source/object/slot correspondence and validate
  ranges/topology; ambiguous object correspondence fails visibly. Do not chase
  stale absolute PSD/image paths embedded by the vendor outside the source root.
- [ ] Add staging tests: mismatched source/profile bytes, failed conversion,
  interrupted write, linked output path, unchanged rerun and changed choice.
  Use a fake converter only for failure-injection tests; the layout fixture above
  exercises real Blender. An old installed manifest must remain unchanged on
  failure, and a material proposal must never appear in normal Ready catalogue.
- [ ] Implement producer sequencing:

```python
catalog = discover_materials(source_root, pack_config, profile.pack_version)
resolution = resolve_profile(profile, catalog, source_root)
# Deterministic samples: include a multi-slot use when available; record coverage.
# For each family/option, vary that family only. Other selected/recommended
# context bindings are explicitly labelled; truly unresolved context is neutral.
# Convert into a private stage, verify reports/GLB hashes, then publish the
# material-only manifest and byte-copied files into the ignored review directory.
```

  Source/config locations come from the existing WorkspaceConfig. Require the
  destination to be ignored by Web Git and disjoint from sources/cache/runtime
  publication roots. Use input/context/option fingerprints to reuse verified
  stage outputs, never existence-only resume or hard links. Samples=2 is preview
  coverage, not an asset/texture budget; preserve source-resolution maps.
- [ ] Run `PYTHONPATH=scripts python3 -m unittest test_material_review_layout test_prepare_material_review test_fbx_to_glb -v`, confirm the legacy-mode preservation control, and commit the exact changed files with normal hooks.

## Task 3: Family choices and profile round-trip in the existing Lab

**Files (Web):**
- Create `src/dev/asset-review/materialProfile.ts`
- Create `src/dev/asset-review/materialProfile.test.ts`
- Create `src/dev/asset-review/MaterialReviewLab.tsx`
- Create `src/dev/asset-review/MaterialReviewLab.test.tsx`
- Modify `src/dev/asset-review/AssetReviewLab.tsx`
- Modify `src/dev/asset-review/AssetReviewLab.test.tsx`
- Reuse `src/dev/asset-review/AssetReviewScene.tsx`; add scoped styles alongside
  existing `AssetReviewLab.css`, without restructuring its normal batch UI.

**Consumes:** material-preview manifest and the exact profile JSON above.
**Produces:** TypeScript `PackMaterialProfile`, `MaterialReviewManifest`,
`parseMaterialProfile(value: unknown): PackMaterialProfile`,
`parseMaterialReviewManifest(value: unknown): MaterialReviewManifest`,
`selectFamilyOption(profile: PackMaterialProfile, manifest: MaterialReviewManifest, familyId: string, optionId: string | null): PackMaterialProfile`
and `serializeMaterialProfile(profile: PackMaterialProfile): string`.

- [ ] Add literal shared-shape JSON fixtures in both language test suites. Test
  wrong pack, unknown/duplicate IDs, unsafe preview URLs, out-of-family option,
  malformed fingerprints and missing preview records. Round-trip choices without
  extra approval/source-hash fields in the editable profile.
- [ ] Run `npm test -- --run src/dev/asset-review/materialProfile.test.ts` red;
  implement strict parsing and immutable family selection using catalogue IDs.
- [ ] Add behavior tests for the view, including this interaction:

```tsx
render(<MaterialReviewLab />);
fireEvent.change(await screen.findByLabelText('Material option for Stone'), {
  target: { value: 'stone-dark-repeat' },
});
expect(screen.getByText('2 affected pieces')).toBeVisible();
fireEvent.click(screen.getByRole('button', { name: 'Export pack profile' }));
// Read the captured download JSON and assert selections.Stone is
// 'stone-dark-repeat'; assert no provider export or Ready action exists here.
```

  Stub the network catalogue and WebGL scene boundary only; retain real parsing,
  selection, import and serialization. Also test null choice, missing preview,
  failed load, changed catalogue/profile import and stale load callbacks.
- [ ] Implement grouped families, category filters, recommendation evidence,
  affected-use list/count, sample coverage and a compatible-option selector.
  Use the same `AssetReviewScene` with source GLB URLs, scale1, yaw0, zero offset,
  orbit camera and raw overlay off. Selecting an option changes the prepared URL;
  never dynamically repaint an existing GLB and call it converter evidence.
- [ ] Route `?assetReview=1&materialReview=1` to the new view through a small
  wrapper in AssetReviewLab; move its existing normal component name internally
  if necessary, preserving hooks/order and exports. Normal mode, storage key,
  calibration/Ready validation and provider export remain unchanged. Material
  mode fetches only its own manifest and cannot require a normal Ready catalogue.
- [ ] Implement import/export of the same profile using the Lab's existing file
  patterns. Unsaved browser selections are visibly draft. Import with mismatched
  pack fails without replacing current choices. Changed/unknown IDs get explicit
  errors. No hidden approval or automatic overwrite of an existing on-disk file.
- [ ] Run focused Lab/profile/scene tests plus typecheck. Commit and publish a
  draft at the first working push, explicitly pending review; run normal
  `npm run ci-check` at the PR boundary, not a separate full suite beforehand.

## Task 4: One profile-driven launcher path and usable handoff

**Files (Assets):**
- Modify `scripts/asset_review_launcher.py`
- Modify `scripts/test_asset_review_launcher.py`
- Modify `docs/pipelines/world-asset-quickstart.md`
- Modify `docs/human/asset-ingestion/material-preflight.md`
- Create `docs/human/asset-ingestion/pack-material-profiles.md`

**Consumes:** `prepare_material_review` and the Web material-mode route.
**Produces:** optional `--material-profile PATH` on the existing `asset-review`
entry point. No extra standalone GUI/server is introduced.

- [ ] Test CLI profile dispatch with existing fixture workspaces. Omitted flag
  must preserve today's normal Lab behavior. Explicit missing profile prompts
  once to generate its unresolved template (noninteractive mode stops); refuse
  overwrite and pack mismatch. Invalid profiles create no staged outputs.
- [ ] Extend `launch_review(..., material_profile: Path | None = None)` and parser.
  Under its existing owned-review lock, first verify the dedicated review checkout
  contains the tracked material-view implementation; old Web code must produce a
  clear upgrade-required stop before conversion, not open normal Lab and claim
  material-mode success. Do not reset or switch that checkout automatically.
  Prepare material output instead of normal queue when supplied; use the existing
  dependency/scene-assets/server flow.
  Construct the material-review URL. Existing-listener identity checks must verify
  the material manifest in this mode, not mistake Vite HTML or normal catalogue
  for the requested material site. Preserve stable port3180 and ordinary storage.
- [ ] Verify actual shell argument dispatch with a controlled no-network workspace
  and missing/invalid profile. In a local fixture harness, start the existing
  viewer against the generated material manifest and fetch actual GLB hashes.
  No runtime models or real asset release are required for this check.
- [ ] Document the one-file loop, template creation, option IDs supplied by
  discovery, family choices, profile import/download and rerun command:

```bash
"$ASSETS_SOURCE/scripts/asset-review" --workspace PACK --material-profile PROFILE.json
```

  Label this delivery **material preview only**. Do not replace the trusted cache
  or claim profile approval already feeds normal ingestion. Explain that an
  exported profile can be passed directly to the same command without copying
  hashes, expanded slot mappings or directories. Blender is for exceptions.
- [ ] Run all focused Assets profile/discovery/layout/producer/launcher tests and
  normal Web PR gate. Inspect that diffs contain only source/tests/docs, never
  licensed GLBs/screenshots or private profiles. Commit/push with normal hooks.
- [ ] Show Kirk this actual operator path early. He chooses whether/when to use
  his real pack and gives feedback on family grouping/options. Fix the operator,
  not individual asset mappings, when the interaction falls short.

## Self-review and completion checklist

- [ ] Profile schema/field names agree in Python, manifest, TS and exports.
- [ ] Missing values mean unresolved; recommendations are not approval.
- [ ] Source-slot/UV preservation is checked with actual Blender, including an
  unbound multi-slot mesh, not merely a mocked converter result.
- [ ] Filename clues produce explained proposals; neither aliases nor atlas
  layouts gain compatibility merely by similar names.
- [ ] Normal Lab/release inputs, cached published bytes and old decisions remain
  unchanged; material mode cannot produce Ready/provider exports.
- [ ] Profile change/source drift invalidates affected staged output; retry never
  silently overwrites unrelated files or reuses stale bytes.
- [ ] Written design coverage: discovery, profile, options, grouped preview and
  round-trip are covered by Tasks1–4. Canonical approval/activation/promotion
  binding is explicitly outside this slice and still required by design slice3.
- [ ] Report implemented/merged/installed separately. No asset publication is
  acceptance. No automatic merge, agent dispatch or manual release checkpoint.
