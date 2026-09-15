# Repeat placement — first wall slice

**Status:** Kirk approved this design ("it matches"). Implementation planning
subsequently exposed the provider-enrollment assumption below; that decision is
pending. No implementation has started.

## Intent and limits

Choose a wall asset, drag a straight line, and get aligned copies without placing
each one by hand. The first proof uses the existing Crypt wall. **Dark Fantasy is
the next style case, not a prerequisite.** Making its import a blocker was an
incorrect dependency introduced during discovery; Kirk corrected it.

This is a placement convenience in the existing room-authoring editor, not a new
wall subsystem. No live regeneration, patterns, randomness, automatic enclosure,
corner fitting, floor filling, or new gameplay/YAML integration. Separate runs
are not automatically connected. Intact/broken variation remains an ordinary
piece-editing use case; a dedicated variant-replacement UI is not required for
the first repeat proof.

## Interaction and result

1. Choose the enrolled wall from the normal asset catalog and activate **Repeat**.
2. Press/drag on the ground to preview a straight run. The direction is free in
   world X/Z, not constrained to hex edges; authored height is floor-relative 0.
3. Preview whole copies at their verified natural spacing, without stretching.
   Snap length to the nearest positive whole-piece count; a simple placement can
   produce one piece. The first piece begins at the chosen start of the run.
4. Release to commit the complete result once. Two or more pieces become an
   ordinary group; one remains an ordinary prop. Select the result; Repeat stays
   armed for another run until the author chooses another tool.
5. Normal group movement, rotation, ungrouping, deletion, arrangements, saving,
   and reload work on the ordinary pieces. No endpoint/regeneration state survives
   placement. Undo removes the whole placement; Redo restores its same identities.

Escape, pointer cancellation/lost capture, or changing tools discards the preview
without touching the draft. Respect existing camera/touch controls and gesture
ownership; do not copy the old brush's pre-fix pointer behavior.

## Small implementation boundary

- A pure layout helper takes start/end and the asset's verified span/origin offset
  in scene units, returning transforms and snapped length/count. It knows nothing
  about Crypt, asset loading, room regions, or game rules.
- The catalog/rendering adapter supplies those visual placement measurements.
  For the first centered, +X-spanning wall, its verified width supplies the step
  and half that width supplies the first origin offset. Do not generalize this
  into a claim that every future model's bounding box is its join contract.
- One scene operation allocates prop IDs, creates the pieces, optionally uses the
  existing `groupSelection`, and passes the complete scene through the existing
  commit/room-declaration reconciliation path. Preview does not write history or
  storage. No new persisted run type or room-draft format is needed.
- Missing/non-finite/zero measurements disable placement with an explanation.
  Existing scene limits and validation still apply. Check the requested count
  before allocating a preview. Reject an invalid result as a whole rather than
  silently placing part of the requested run.

### Existing asset enrollment, not a URL shortcut

`Crypt_Wall_Body_01.glb` is already available in the retained editor's licensed
asset tree and returns HTTP 200 at :3030. However, it is not yet a World Builder
catalog entry. The normal generated catalog already supports category `env`.

The Assets-owned preparation is one normal catalog enrollment of the existing
wall through the established private provider workflow, with a stable exact ref,
verified normalized geometry and dimensions. Reuse the known source/provenance;
no new pack import, generated-file hand edit, arbitrary-URL scene entry, or
Crypt-specific renderer branch. Preserve the existing game shell artifacts.
If enrollment exposes a missing provider capability, report it before expanding
scope or bypassing the provider's checks.

**Planning correction, left visible:** checking current provider main
`4c069294895518a2571bada1797aab244935f5b0` showed that the generic world-asset
promotion path registers only `polygon-dark-fortress`. Its source contract names
a reviewed pack/version/FBX-source path/hash; it does not directly enroll an
already-promoted authored shell artifact. The Crypt GLB really is present and
served, but the claim that its catalog enrollment was routine was premature.
Do not fake a Dark Fortress source identity or hand-edit generated metadata.

The unresolved choice is proper provider support for the existing authored wall
before the first UI proof, versus using an already-cataloged barricade for the
initial generic repeat-tool proof and addressing wall enrollment separately.
Neither change to the approved preparation has been selected. This is not a
reason to wait for the Dark Fantasy import or to modify Kirk's ongoing imports.

### Correct reported dimensions before consuming them

Read-only discovery verified a current-dev measurement defect:

- The provider's `world_asset_bounds_meters` already applies shared scale `0.75`.
- `WorldAssetModel` correctly applies `0.75` to the GLB when rendering, but applies
  it again to catalog dimensions when reporting bounds.
- Actual normalized Alchemy Tools 01 GLB width is `0.24207866191864014`, with
  identity node transforms. Rendered width and catalog value are both
  `0.1815589964389801`; the reported width becomes `0.13616924732923508`.

Correct the reported bounds and prove the unit boundary. **Do not change rendered
model scale or existing placement transforms.** Repetition must use actual scene
extents, not this currently undersized report. This is a dependency of trustworthy
alignment, not an unrelated asset-system redesign.

## Proof and delivery

- Focused proofs: counts and origins at cardinal/diagonal/reverse directions;
  whole-step snapping; malformed measurements; fresh identities; one-copy case;
  atomic limit rejection; Undo/Redo and save/reload preservation.
- A real geometry/metadata test catches double scaling independently of the
  implementation's formula. Verify several actual wall copies join cleanly at
  multiple yaws in the browser. If they do not, return the measured seam problem
  to Kirk rather than guessing overlap constants or inventing a join framework.
- Exercise real pointer capture/release/cancel and retain current touch-pan
  behavior. Preserve the existing failed-load storage protections.
- First hands-on checkpoint: the existing wall repeated in :3030, movable as a
  group, undoable once, and restored unchanged after reload. Destructive storage
  probes use isolated contexts, never Kirk's saved draft.
- Prepare the asset through its owning Team; implement the UI in one fresh issue
  worktree from current `origin/dev` (discovery head `e83dd2db`, including #1071
  touch-pan). Do not add code to the completed #1068 branch.
- Keep :3030 and its storage origin; plan source repointing without erasing the
  environment's data. API/toolkit consumer #1753 stays parked.
- One complete web `ci-check` at the PR boundary and one scoped GLM review. Bring
  Kirk into the first working result and any substantive findings before further
  loops. No merge or cleanup is authorized by this design.
