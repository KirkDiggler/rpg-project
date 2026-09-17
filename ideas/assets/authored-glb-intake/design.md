# Authored GLBs through the original Asset Review Lab

Status: approved in chat, including explicit source switching between the full
cached library and the focused authored trial; implementation pending.
Tracking: [#459](https://github.com/KirkDiggler/rpg-project/issues/459).
Parent journey: [#365](https://github.com/KirkDiggler/rpg-project/issues/365).
Builds on [#412](https://github.com/KirkDiggler/rpg-project/issues/412).

## Approved outcome and boundary

Blender exports → original Asset Review Lab → existing metadata/calibration controls
→ human Ready selection → Ready JSON → existing release workflow → Assets/Web PRs
and human merges. The operator supplies files, not hashes or manifests.

The first batch contains exactly a floor tile and a double-door assembly. Both
have already loaded in an isolated Three.js export inspection. The door's four
nodes and opposed hinge motion survived export. This is useful evidence about
inputs, **not** evidence of Lab registration, ingestion or gameplay integration.

This slice delivers intake/review/release. Door open/closed gameplay binding and
automatic upper-wall geometry/UV adjustment are separate follow-ups. The imported
door is a preserved assembly rendered in its authored closed pose; category `env`
and node names do not grant interactive behavior.

The browser material editor remains parked. No full-pack conversion, material
rewriting, texture downsampling, automatic Ready, publication or merges.

The full existing pack catalogue remains a first-class browsing source, including
material-review, FX-review and broken-preview entries. A focused trial must not
replace that library or require ingesting/converting it again. Browsing an asset
is distinct from marking it Ready or publishing it.

## Verified current seams

Inspected Assets `d632f42` and the installed Web review implementation:

- `scripts/world_asset_review.py::_normalized_source_path` and Web
  `src/dev/asset-review/model.ts::parseSource` require lowercase `.fbx` sources.
  Their schema-v1/v2 formats are exact-key validated.
- `scripts/prepare_asset_review.py` stages verified copies into an ignored Web
  model directory and emits the catalogue used by the original Lab.
- `scripts/promote_world_assets.py::_source_for_entry` resolves only trusted
  converter manifest rows. Renaming an authored file to `.fbx`, making a fake
  converter receipt or overwriting a trusted cache is not an intake mechanism.
- `scripts/normalize_prop_for_runtime.py::normalize_static_glb` copies transformed
  geometry, removes parents and resets object matrices. A door can look correct
  while losing its hinges. The authored path must not use that bake.
- `scripts/world_asset_release_{state,commands,workflow}.py` own frozen input,
  journal, transaction, reconciliation and human publication boundaries. Extend
  source resolution there; do not create another release runner.
- `AssetReviewScene.tsx::CandidatePreview` already previews nested GLB scenes.
  Its calibration is centering/flooring followed by scale/offset and yaw, with
  the existing shared Synty scale. The authored output must match this screen.
- `asset_review_launcher.py` already selects named registered workspaces and
  prepares a cached queue without conversion. However, `AssetReviewLab.tsx`
  hardcodes one `catalog.json` URL and one `rpg.asset-review.batch.v1` storage
  key. A source switcher with isolated drafts is new work, not an existing UI.

## Ownership and scope

**Assets owns** capture, immutable private custody, GLB inspection, catalogue
production, authored source resolution, calibrated output and release receipts.
**Web owns** parsing the versioned review contract and showing it through the
existing Lab, retaining its human controls and rendering the published output.
**Project owns** this cross-repository contract. No API/proto/toolkit changes.

Only two active implementation worktrees are needed, one per owning repository.
Parked editor worktrees, user Blender files, old cache generations, existing
browser drafts and historical releases remain untouched.

## 1. One Lab, explicit selectable sources

Add a small **Source** selector to the original Lab. Initial choices are the
existing complete Dark Fortress library and the focused authored-export trial.
Changing source changes the catalogue being browsed; it does not ingest, convert,
refresh, merge catalogues, clear drafts or approve assets. The chosen source name
stays visible alongside its counts/status. The two-asset restriction applies to
the trial source and release, not to the user's ability to browse the full pack.

The launcher generates an ignored `sources.json` index with schemaVersion1,
`defaultSourceId`, and `sources` rows containing exactly `id`, `label`, `kind`
(`converted-fbx` or `authored-glb`) and `catalogUrl`. IDs are stable workspace
registration identities, not catalogue hashes or display labels. Catalogue URLs
are same-origin prepared paths under the review asset root, not arbitrary
filesystem paths or remote URLs. No human maintains this index.

Use namespaced prepared locations for new sources, with content-hashed GLB URLs.
Register the existing legacy catalogue at its existing path without rewriting or
deleting its prepared models. Reuse verified cached files on subsequent launches;
switching an already registered source only loads its prepared catalogue. Source
preparation and index installation use the review checkout's existing writer
lock and atomic writes. Preparing the authored source must never overwrite the
legacy catalogue or reset the shared serving directory. No unowned server stops.

Keep blocked/material/FX-review candidates searchable. A valid catalogue row
whose preview cannot load stays visible with a reason and no Ready eligibility;
do not hide it or substitute an approved-looking fake preview. Integrity failures
remain explicit failures, not trusted material status. A failed source refresh
must preserve its previous prepared catalogue and other sources' usability.

Store review batches under source-scoped keys and remember the selected source
separately. Names, calibration, decisions and selected-item/filter context belong
to that source; switching out and back restores them. Persist edits before the
switch and gate asynchronous catalogue/model results by active source identity,
so a late old request cannot overwrite the new source or its load/Ready state.
Exports and imports operate on the active source only; never silently combine
Ready entries from both sources.

Legacy compatibility: if no source index exists, retain the current single-source
path and storage behavior. On first indexed launch, migrate the old global draft
only to the uniquely matching legacy source, preserving the original stored JSON
as a backup. Never copy it into the authored trial or clear it on ambiguity; show
a recoverable migration message instead. V1/v2 catalogue payloads remain unchanged.

This is catalogue selection, not a material editor or a filesystem picker. The
existing workspace setup/launcher owns which sources can be offered.

## 2. Explicit source kind, not converter impersonation

Introduce schema **v3** for authored review catalogues, persisted review batches
and exported Ready batches. V1/v2 retain their current exact behavior and bytes.
This first v3 workflow is authored-only; mixing converter palettes into it is
not required. It has no palette selections or browser material editing.

V3 keeps the familiar candidate and provider metadata fields. Its `source` has
these exact fields:

```text
kind: "authored-glb"
packSlug: existing registered pack namespace
packVersion: existing registered pack version
sourcePath: normalized relative .glb path inside the chosen export folder
glbSha256: SHA-256 of the immutable authored export
capture: { path: normalized receipt path relative to archive root, sha256: receipt SHA-256 }
```

The pack association is an operator-declared provenance/namespace association,
not a claim that a converter generated the model or that the filename proves a
specific FBX composition. Do not invent constituent-source mappings for renamed
Blender objects. Existing licensed source catalogues remain independent authority.

New v3 candidates use `reviewStatus: "authored"`. This means capture and technical
inspection passed, **not** that appearance was approved. The existing load,
calibration, naming and Ready checks still apply. Add this status to the original
Lab's badge/filter; do not add another screen.

Candidate identity includes source kind, pack/version, relative source path and
GLB hash. Two files cannot collide merely because their basenames match; reject
ambiguous proposed refs rather than silently suffixing or overwriting a release.

## 3. Immutable capture and refresh

Register a separate authored workspace. Keep export folder and archive root
separate from each other, existing converter caches and publication destinations.
Workspace schema v2 introduces a discriminated authored input configuration
rather than putting authored files under the v1 `cache` field. V1 workspace files
continue to load without migration or reinterpretation.

Extend the existing interactive setup with authored-folder and pack/version
inputs. It derives repository paths, archive paths and registrations, previews
writes and asks before installing the workspace, as setup already does. Normal
commands remain `asset-review --workspace NAME` and `asset-release READY.json`.
The existing source-match selection can restrict the trial to the two filenames.
No manually maintained configuration or mapping file is added to the human flow.

Assets captures selected regular files without changing the export folder.
Reject path escapes and symlink traversal. Copy bytes into private immutable
storage, hash and verify the copy, then inspect the captured bytes—not a moving
working file. A concurrent source rewrite must not produce a partially trusted
capture. Installation is atomic; verify source/copy identities and refuse a
changed read. An identical retry reuses the verified capture and receipt.

A deterministic receipt records source kind, pack association, relative path,
GLB hash/size, capture format version and a hash-bound structural inspection.
Originals are addressed by content; receipts are addressed by their own canonical
content. Observational timestamps do not change the reusable source identity.
Receipt paths are machine-generated. Local absolute paths do not enter public
catalogues, recipes or consumer metadata.

Technical inspection reuses the provider's GLB/accessor/image machinery. First
scope: one default static scene, triangle meshes, finite valid transforms and
geometry, usable UV/material references, embedded PNG/JPEG images and embedded
buffer data. Reject external resources, skins, animation, morphs, cameras/lights,
malformed graphs and unsupported required extensions. Preserve valid negative
scale, nested nodes and named pivots. Do not silently strip unsupported content.

A bad asset is reported separately and cannot become Ready; good captures still
produce a review queue. Tool/runtime or archive-integrity failures are fatal,
not mislabeled as a bad model. Texture/size observations use world-asset advisory
policy; legacy prop limits remain unchanged.

Refresh stages hash-named GLB URLs through existing ignored-copy machinery. It
preserves unchanged assets' decisions. Changed bytes keep useful human metadata
but clear successful-load/Ready authority and require fresh review; an old Ready
JSON cannot silently select new bytes. Removing a working export does not delete
its archived revision. An in-flight release stays bound to its frozen captures,
not whatever the working folder contains later.

## 4. Calibration without losing the authored graph

For authored inputs, add a hierarchy-preserving normalizer. It repacks the GLB
JSON with one new calibration parent above the original default-scene roots.
Original nodes, parent/child relationships, local transforms, meshes, accessors,
UVs, materials, image bytes and BIN payload remain unchanged. Record the wrapper
node index in output evidence; do not depend on a magic object name. The receipt
is authority for authored versus derived output; do not guess from a node name.

The new normalizer must reproduce **the Lab preview**, not copy the old static
baker's different order of transforms. For an original source-world point `p`,
let `c` be `(bounds.centerX, bounds.minY, bounds.centerZ)` before calibration,
`s` the human scale, `f` the fine-offset vector in preview metres, `R` the human
Y-axis yaw, and `g` the existing shared runtime Synty scale. The wrapper is:

```text
M = R × T(f / g) × S(s) × T(-c)
runtime point = g × M × p = R × (g × s × (p - c) + f)
```

The scene's dungeon-surface placement belongs to the existing renderer, not the
asset. Use the existing shared scale definition; do not apply it twice. Decode
actual reachable geometry for bounds, including node transforms and negative
scale. Tests must use asymmetric geometry, nonzero offsets and nonzero yaw, not
only a centred cube with identity calibration.

Validate the authored graph and compare every original node/mesh/image payload
against the captured input. The only permitted document delta is the wrapper
and its default-scene root binding. Test world vertex positions, named-node
origins and relative transforms through the common calibration matrix. A test
that only compares the closed door silhouette is insufficient.

Keep legacy identity-matrix validation for legacy normalized outputs. Add an
explicit authored-output validator; do not weaken the legacy validator globally
or pretend the hierarchy-preserving output has identity node transforms.

## 5. Existing release custody and consumption

Extend source resolution by source kind. Legacy entries still require their
trusted converter manifests. Authored entries require the exact captured receipt
and original bytes named by the Ready source. Validate both before normalization.

Freeze selected originals, their receipts and inspection facts alongside the
Ready input before stage work. Seal only the selected immutable capture set;
appending another capture must not invalidate an unrelated frozen release.
Copy the selected originals/receipts into the private provider release as durable
provenance. Consumer sync receives runtime assets/metadata, not the authoring
folder, local paths or `.blend` files.

Promotion dispatches explicitly to the authored normalizer and validator. Bind
recipe, source capture, calibration, normalizer version, derived output hash and
preservation evidence in the provider receipt. Existing stage/validate/apply/check
transactions, release revision gates, warning acknowledgement, local-only stop,
GitHub reconciliation and both human merge pauses remain in force.

Use the ordinary generated asset catalogue and static renderer to display the
trial assets. Verify these consumers inspect transformed geometry correctly;
where a current inspector assumes baked identity nodes, add an explicit authored
path and regression tests rather than changing historical receipt semantics.
The source hierarchy survives future gameplay integration, but this trial neither
exposes a working door action nor treats the upper part as already resizable.

## 6. Delivery sequence and evidence

1. **Source selection + contract/intake + original Lab checkpoint.** Preserve the
   complete cached library, capture synthetic fixtures and then the two private
   exports. Switch between the full library (including blocked entries) and the
   two-asset trial in the original Lab without conversion or catalogue replacement.
   Demonstrate isolated/restored drafts, legacy draft migration, metadata,
   calibration, active-source Ready-only export, defer, unchanged refresh and
   changed-source invalidation. This is the first human-visible checkpoint.
2. **Preserving normalization + release adapter.** Prove graph/image preservation,
   preview/output calibration parity, strict kind separation, durable private
   custody, tamper refusal and idempotent frozen-source resume. Produce a real
   local-only trial stage; do not fabricate a release journal boundary.
3. **Normal release delivery.** After human Ready selection and inspection, run
   the existing release workflow to Provider PR, pause for human merge, reconcile
   the actual merged provider, then produce Web PR and pause again. A release is
   not complete until its real merge/checkpoint requirements are met.

Expected code seams:

- New focused Assets modules `authored_world_asset_source.py`,
  `prepare_authored_asset_review.py`, `normalize_authored_world_asset.py`, with
  explicit input/output dataclasses and synthetic tests.
- Existing Assets `world_asset_review.py`, `asset_review_launcher.py`,
  `promote_world_assets.py`, release state/setup/input/commands/workflow and their
  focused tests; reuse lower-level geometry validation rather than duplicating it.
- Web `src/dev/asset-review/model.ts`, `AssetReviewLab.tsx`, associated tests and
  any provider-generated contract consumption actually affected by authored
  outputs. Keep the existing scene, controls and browser draft mechanism.
- Existing operator guides: `asset-review-lab.md`, `world-asset-batch-runbook.md`
  and `world-asset-quickstart.md`.

Run focused suites while implementing. At the PR boundary, run full Web
`npm run ci-check` once and explicitly named Assets suites. One independent
review round at the substantive implementation boundary, not an autonomous
review campaign per helper. Keep licensed fixtures/screenshots private; commit
only synthetic fixtures to public repositories. Never overwrite parked work.

## Acceptance checklist

- [ ] V1/v2 converter reviews and historical outputs remain valid and unchanged.
- [ ] Full cached pack remains browsable, including blocked/broken-preview rows.
- [ ] Source switching preserves separate catalogues/drafts and does no conversion.
- [ ] Late source responses cannot overwrite another source's state or Ready status.
- [ ] Legacy drafts migrate only to their matching source, with originals retained.
- [ ] Real floor and door enter the existing Lab from authored captures.
- [ ] Human metadata/Ready controls and Ready-only JSON are the operator handoff.
- [ ] Changed bytes cannot reuse old Ready authority; unchanged captures can.
- [ ] Door nodes/pivots, mirrored transforms, materials and image bytes survive.
- [ ] Nontrivial preview calibration equals published rendered calibration.
- [ ] Invalid files do not prevent independently valid files being reviewed.
- [ ] Fake converter authority, stale receipts, tampering and path escapes fail.
- [ ] Local-only stage/resume and actual human PR/merge gates are demonstrated.
- [ ] No claims of interactive door or automatic wall-height support.
