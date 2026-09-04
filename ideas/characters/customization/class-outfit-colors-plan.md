# Class Outfit Colors Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let every supported hero choose two character-owned colors for the fixed Barbarian, Fighter, Monk, or Rogue outfit during creation and render the same style for owners and peers in normal play.

**Architecture:** The private asset provider derives four exact two-channel masks from Synty's licensed source mask and publishes them beside a strict class-treatment manifest without changing any body GLB. Toolkit owns the complete Appearance model and validation in the D&D module, then the independently versioned Session module consumes the CI-minted D&D tag and exposes public roster identity. The API only authenticates, converts, delegates, stores toolkit data, and translates errors; the web consumes exact provider/proto pins and extends per-character `MeshStandardMaterial` instances with stable mask uniforms.

**Tech Stack:** Blender 5.x, Python 3/Pillow, glTF 2.0, Go 1.24+, rpg-toolkit D&D and Session modules, Protocol Buffers/Buf, Go/Redis/Connect RPC, React 19, TypeScript, Three.js/React Three Fiber, Vitest/Testing Library.

**Spec:** `ideas/characters/customization/class-outfit-colors-design.md`

**Tracking:**

- Journey: `KirkDiggler/rpg-project#362`
- Design/plan PR: `KirkDiggler/rpg-project#364`
- Provider: `KirkDiggler/rpg-game-assets#119`
- Protos: `KirkDiggler/rpg-api-protos#278`
- Toolkit D&D module: `KirkDiggler/rpg-toolkit#1450`
- Toolkit Session module: `KirkDiggler/rpg-toolkit#1451`
- API: `KirkDiggler/rpg-api#897`
- Web: `KirkDiggler/rpg-dnd5e-web#912`

## Global Constraints

- Work from fresh isolated worktrees: provider/protos/toolkit from `origin/main`; API/web from `origin/dev`. Never use the dirty long-lived toolkit checkout.
- Use one branch and one PR per independently versioned module. Toolkit deliberately uses two sequential PRs because `rulebooks/dnd5e/session` pins an exact released `rulebooks/dnd5e` version.
- Local `replace` directives and pseudo-versions are iteration aids only. Never commit a local path or a branch-only pseudo-version; Session must merge against the exact CI-minted D&D tag.
- Licensed Synty source, extracted inputs, converted review assets, and `.blend` files remain private and untracked. Synced web GLBs/PNGs remain ignored and untracked.
- Existing body GLBs and manifests remain byte-identical. Immutable Dwarf v1 remains byte-identical in runtime, schema, defaults, thumbnails, and hashes.
- Fixed class authority remains Barbarian 01, Fighter 16, Monk 08, and Rogue 10. This wave adds no geometry or equipment choice. Alternate leather/chain-mail models, item-instance cosmetics, post-finalization editing, and entitlement remain deferred.
- Primary colors dominant dyeable cloth/leather; secondary colors trim/accent. Skin, hair, fur, wood, and exposed metal retain authored values.
- `OutfitCustomization` is character-owned. Its two RGB24 fields are independently optional; absence preserves the provider channel, zero is black, Reset clears both, and class changes retain both.
- Hair and facial hair keep one shared color and roughness. Existing hair selection/default/none/ref semantics remain unchanged.
- No durable `/concepts` route. Stop for Kirk's Blender verdict before provider promotion.
- Toolkit owns customization semantics and public roster projection. API performs pure shape conversion and does not inspect customization values at runtime.
- Ephemeral Redis records receive no compatibility migration. Restart and recreate the standard cast after the ownership transition.
- Missing/rejected outfit treatment renders the untouched original body. It cannot remove body, hair, animation, hand equipment, or another character's valid treatment.
- Color updates mutate per-instance uniforms in place and preserve mesh, Skeleton, mixer, material, and texture identities.
- Do not modify `/home/kirk/game-dev/rpg-project/active.md` or the unrelated weapon-presentation lane.
- Use focused red/green TDD during development. Run one final full suite and one independent whole-PR review per substantive PR; disposition every finding before readiness.
- Execution is inline unless Kirk explicitly changes the existing no-subagent preference.

## Workspace and Release Map

Create these worktrees only when execution reaches the corresponding slice:

```bash
git -C /home/kirk/game-dev/rpg-game-assets fetch origin main
git -C /home/kirk/game-dev/rpg-game-assets worktree add \
  /home/kirk/.pi/worktrees/rpg-game-assets/119-class-outfit-colors \
  -b feat/119-class-outfit-colors origin/main

git -C /home/kirk/game-dev/rpg-api-protos fetch origin main
git -C /home/kirk/game-dev/rpg-api-protos worktree add \
  /home/kirk/.pi/worktrees/rpg-api-protos/278-outfit-customization \
  -b feat/278-outfit-customization origin/main

git -C /home/kirk/game-dev/rpg-toolkit fetch origin main
git -C /home/kirk/game-dev/rpg-toolkit worktree add \
  /home/kirk/.pi/worktrees/rpg-toolkit/1450-appearance-sdk \
  -b feat/1450-appearance-sdk origin/main
# Create 1451 only after 1450 merges and its D&D tag exists.

git -C /home/kirk/game-dev/rpg-api fetch origin dev
git -C /home/kirk/game-dev/rpg-api worktree add \
  /home/kirk/.pi/worktrees/rpg-api/897-appearance-sdk \
  -b feat/897-appearance-sdk origin/dev

git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin dev
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  /home/kirk/.pi/worktrees/rpg-dnd5e-web/912-class-outfit-colors \
  -b feat/912-class-outfit-colors origin/dev
```

The exact merge/tag gates are:

```text
provider PR -> exact provider merge SHA -----------------------> web
proto PR -> generated-branch commit ---------------------> API + web
D&D toolkit PR -> CI-minted rulebooks/dnd5e tag -> Session PR
Session PR -> CI-minted rulebooks/dnd5e/session tag ------> API
API PR --------------------------------------------------------> normal game proof
```

---

### Task 1: Define and derive deterministic provider dye masks

**Issue/branch:** `rpg-game-assets#119`, `feat/119-class-outfit-colors`

**Files:**
- Create: `scripts/configs/class-outfit-colors-v1.json`
- Create: `scripts/class_outfit_colors.py`
- Create: `scripts/test_class_outfit_colors.py`
- Read-only authority: `scripts/configs/character-customization-profiles-v1.json`
- Read-only authority: `harness/models/synty/characters/customization/manifest.json`

**Interfaces:**
- Consumes: licensed archive `POLYGON_Modular_Fantasy_Hero_SourceFiles_v2.zip`, SHA-256 `9ca5e73b4a41f513c07201858e94080dd5fb9d1c8e687d4ed42c014376fbb17e`.
- Consumes: source mask member `Source_Files/Textures/PolygonFantasyHero_Texture_Mask_01.png`, SHA-256 `27320f6c2874dcc09074be870a6be975e0ebe74106381e6d1a80838f550d4821`.
- Consumes: atlas member `Source_Files/Textures/PolygonFantasyHero_Texture_01_A.png`, SHA-256 `7f84972790e530f8d83b378eb95f3151e2664c7b4ac23b1d125a55e1efcecd62`.
- Produces: `load_outfit_color_config(path: Path) -> dict[str, object]`.
- Produces: `stage_source_inputs(config, source_root, stage_root) -> StagedOutfitInputs`.
- Produces: legacy provenance helper `derive_class_mask(source_mask: bytes, primary_groups: tuple[str, ...], secondary_groups: tuple[str, ...]) -> bytes`.
- Produces: final authority helper `derive_atlas_swatch_mask(atlas: bytes, source_mask: bytes, atlas_swatches: Mapping[str, object]) -> bytes`.
- Produces: `build_class_masks(config, staged_inputs, output_root) -> dict[str, MaskReceipt]` with actual Human UV-surface coverage per class/channel.

> **Execution amendment, accepted at the Blender gate:** measured UV and eight-yaw render evidence proved the initial whole-source-group split below insufficient: Barbarian/Fighter exposed no channel and no class exposed secondary. Final `exact-atlas-swatches-v1` text config names inclusive per-class atlas rectangles and their exact atlas/source-mask RGBA; the generator verifies every texel before emitting masks. The source-group snippet remains the RED hypothesis/provenance test, not publication authority. Kirk approved the corrected four-class scene: “the characters look great very clear the coloring”.

- [ ] **Step 1: Write failing config and mask tests**

Create the strict class-order/source fixture and first encode the source-group hypothesis that the Blender task must prove or reject:

```json
{
  "schemaVersion": 1,
  "workflowVersion": "class-outfit-colors-v1",
  "source": {
    "archive": "POLYGON_Modular_Fantasy_Hero_SourceFiles_v2.zip",
    "archiveSha256": "9ca5e73b4a41f513c07201858e94080dd5fb9d1c8e687d4ed42c014376fbb17e",
    "atlasMember": "Source_Files/Textures/PolygonFantasyHero_Texture_01_A.png",
    "atlasSha256": "7f84972790e530f8d83b378eb95f3151e2664c7b4ac23b1d125a55e1efcecd62",
    "maskMember": "Source_Files/Textures/PolygonFantasyHero_Texture_Mask_01.png",
    "maskSha256": "27320f6c2874dcc09074be870a6be975e0ebe74106381e6d1a80838f550d4821",
    "dimensions": [1024, 1024]
  },
  "profileMatrix": "scripts/configs/character-customization-profiles-v1.json",
  "outputRoot": "harness/models/synty/characters/outfit-customization/v1",
  "classOrder": ["barbarian", "fighter", "monk", "rogue"],
  "outfits": {
    "barbarian": {"outfit": "01", "primaryGroups": ["cyan"], "secondaryGroups": ["blue"], "excludedMeshNames": [], "defaultPrimarySrgb": 4810366, "defaultSecondarySrgb": 13739084},
    "fighter": {"outfit": "16", "primaryGroups": ["cyan"], "secondaryGroups": ["blue"], "excludedMeshNames": [], "defaultPrimarySrgb": 4810366, "defaultSecondarySrgb": 13739084},
    "monk": {"outfit": "08", "primaryGroups": ["cyan"], "secondaryGroups": ["blue"], "excludedMeshNames": [], "defaultPrimarySrgb": 4810366, "defaultSecondarySrgb": 13739084},
    "rogue": {"outfit": "10", "primaryGroups": ["cyan"], "secondaryGroups": ["blue"], "excludedMeshNames": [], "defaultPrimarySrgb": 4810366, "defaultSecondarySrgb": 13739084}
  }
}
```

In `test_class_outfit_colors.py`, demand exact keys/order, hash refusal, safe lexical `.stage/<name>` roots, and a synthetic five-anchor source mask:

```python
SOURCE = rgba_png([
    (255, 255, 255, 255),  # preserve
    (0, 255, 255, 255),    # cyan -> primary
    (0, 0, 255, 255),      # blue -> secondary
    (255, 255, 0, 255),    # unselected -> preserve
    (255, 0, 255, 255),    # unselected -> preserve
], width=5, height=1)

result = derive_class_mask(SOURCE, ("cyan",), ("blue",))
self.assertEqual(
    [
        (0, 0, 0, 255),
        (255, 0, 0, 255),
        (0, 255, 0, 255),
        (0, 0, 0, 255),
        (0, 0, 0, 255),
    ],
    list(Image.open(io.BytesIO(result)).convert("RGBA").getdata()),
)
self.assertEqual(result, derive_class_mask(SOURCE, ("cyan",), ("blue",)))
```

Also reject an anchor named by both channels, unknown group names, non-RGBA output, metadata-bearing/noncanonical PNG bytes, and an output whose red/green channels overlap.

- [ ] **Step 2: Run the focused tests and confirm red**

Run:

```bash
cd /home/kirk/.pi/worktrees/rpg-game-assets/119-class-outfit-colors
python3 -m unittest scripts.test_class_outfit_colors -v
```

Expected: import failure for `class_outfit_colors` or missing public functions.

- [ ] **Step 3: Implement strict source staging and mask derivation**

Use these exact anchors and output encoding:

```python
MASK_GROUPS = {
    "preserve": (255, 255, 255),
    "cyan": (0, 255, 255),
    "blue": (0, 0, 255),
    "yellow": (255, 255, 0),
    "magenta": (255, 0, 255),
}
OUTPUT_PRESERVE = (0, 0, 0, 255)
OUTPUT_PRIMARY = (255, 0, 0, 255)
OUTPUT_SECONDARY = (0, 255, 0, 255)
```

Classify source-mask pixels by nearest squared RGB distance to the five anchors, breaking ties by the declaration order above. Selected primary groups emit red, selected secondary groups emit green, and every other group emits black. Encode with the repository's deterministic `_encode_png_rgba`; emit no PNG text/time metadata. Validate the licensed archive and member bytes before writing anything, and stage only the mask and atlas under a fresh lexical `.stage/<name>` root.

Load the profile matrix and require its class/outfit authority to be exactly `01/16/08/10`; derive the Human class body paths and source mesh lists from that authority rather than duplicating them in the new config.

- [ ] **Step 4: Run focused tests and deterministic double-build**

Run:

```bash
python3 -m unittest scripts.test_class_outfit_colors -v
rm -rf .stage/119-mask-a .stage/119-mask-b
mkdir -p .stage/119-mask-a .stage/119-mask-b
python3 scripts/class_outfit_colors.py --build-masks --stage .stage/119-mask-a
python3 scripts/class_outfit_colors.py --build-masks --stage .stage/119-mask-b
diff -qr .stage/119-mask-a .stage/119-mask-b
```

Expected: tests pass and `diff` prints nothing.

- [ ] **Step 5: Commit the mask contract**

```bash
git add scripts/configs/class-outfit-colors-v1.json \
  scripts/class_outfit_colors.py scripts/test_class_outfit_colors.py
git commit -m "asset: define class outfit dye masks (#119)"
```

---

### Task 2: Build the private Blender review scene and stop for Kirk

**Files:**
- Create: `scripts/build_class_outfit_color_review.py`
- Create: `scripts/test_build_class_outfit_color_review.py`
- Modify after verdict: `scripts/configs/class-outfit-colors-v1.json`
- Generated/private: `.stage/119-outfit-color-review/class-outfit-colors-review.blend`
- Generated/private: `/home/kirk/synty-review/class-outfit-colors/`

**Interfaces:**
- Consumes: Task 1's `build_class_masks` and exact Human body paths from the profile matrix.
- Produces: `build_review_scene(repo_root: Path, stage_root: Path, output_blend: Path) -> ReviewReceipt` when run inside Blender.
- Produces Outliner collections: `Barbarian_01`, `Fighter_16`, `Monk_08`, `Rogue_10`, each with deterministic class-qualified `<Class>_<Outfit>_Original` and `<Class>_<Outfit>_Editable` children because Blender collection datablock names are globally unique.

- [ ] **Step 1: Write failing scene-contract tests**

Require exact names and no viewport text:

```python
self.assertEqual(
    ("Barbarian_01", "Fighter_16", "Monk_08", "Rogue_10"),
    review.REVIEW_COLLECTIONS,
)
self.assertNotIn("FONT", review.ALLOWED_REVIEW_OBJECT_TYPES)
self.assertEqual(("Original", "Editable"), review.VARIANT_COLLECTIONS)
```

Add a Blender background test that opens the generated file and asserts:

```python
self.assertFalse(any(obj.type == "FONT" for obj in bpy.data.objects))
for class_collection in REVIEW_COLLECTIONS:
    parent = bpy.data.collections[class_collection]
    original = f"{class_collection}_Original"
    editable = f"{class_collection}_Editable"
    self.assertEqual([original, editable], [child.name for child in parent.children])
    editable_materials = {
        slot.material
        for obj in parent.children[editable].all_objects
        if obj.type == "MESH" and base_name(obj.name) in editable_mesh_names
        for slot in obj.material_slots
    }
    self.assertTrue(editable_materials)
    for material in editable_materials:
        self.assertIsNotNone(material.node_tree.nodes.get("Primary Color"))
        self.assertIsNotNone(material.node_tree.nodes.get("Secondary Color"))
        self.assertIsNotNone(material.node_tree.nodes.get("Outfit Dye Mask"))
```

- [ ] **Step 2: Run tests and confirm red**

```bash
python3 -m unittest scripts.test_build_class_outfit_color_review -v
```

Expected: missing module/functions.

- [ ] **Step 3: Implement the review builder**

Import each canonical Human body twice with Blender's glTF importer. Put originals and editable copies side by side within the named class collection; arrange classes front-to-back with enough spacing for ordinary orbiting. Do not create labels, text objects, cameras with text overlays, or compositor annotations. Apply editable treatment only to the class's non-identity source meshes after exact `excludedMeshNames` are removed; keep the head and every excluded mesh on an untouched material.

For editable materials, preserve the atlas sample and mix in diagnostic magenta `#FF00FF` through mask red, then diagnostic lime `#00FF00` through mask green. Name the RGB nodes `Primary Color` and `Secondary Color`, and the mask image node `Outfit Dye Mask`. Keep the imported armature/pose intact and save the editable `.blend` under `.stage`, never under `harness/` or `evidence/`.

Render one unlabeled original/editable PNG per class into `/home/kirk/synty-review/class-outfit-colors/` for quick opening, but treat the `.blend` and Kirk's direct inspection as authority.

- [ ] **Step 4: Build and verify the scene**

```bash
mkdir -p /home/kirk/synty-review/class-outfit-colors
blender --background --python scripts/build_class_outfit_color_review.py -- \
  --stage .stage/119-outfit-color-review \
  --blend .stage/119-outfit-color-review/class-outfit-colors-review.blend \
  --renders /home/kirk/synty-review/class-outfit-colors
blender --background \
  .stage/119-outfit-color-review/class-outfit-colors-review.blend \
  --python scripts/test_build_class_outfit_color_review.py
```

Expected: four class collections, two variants each, no text object, all editable materials wired to both controls and exact masks.

- [ ] **Step 5: Commit the reproducible review tooling**

```bash
git add scripts/build_class_outfit_color_review.py \
  scripts/test_build_class_outfit_color_review.py
git commit -m "asset: build class color Blender review (#119)"
```

- [ ] **Step 6: Human Blender gate — stop here**

Open:

```text
/home/kirk/.pi/worktrees/rpg-game-assets/119-class-outfit-colors/.stage/119-outfit-color-review/class-outfit-colors-review.blend
```

Ask Kirk to orbit each class and change both named color nodes. Record his exact verdict on `rpg-game-assets#119`. If a region is wrong, edit only that class's `primaryGroups`, `secondaryGroups`, `excludedMeshNames`, and representative defaults in the config, rebuild the scene, rerun the Blender test, and repeat this gate. An exclusion must exactly name one of that class's non-identity source meshes; it disables treatment for the complete mesh when one atlas group has conflicting semantics on that component. Do not start proto/toolkit/API/web implementation until all four mappings are approved.

---

### Task 3: Publish the approved provider manifest atomically

**Files:**
- Create: `scripts/class_outfit_color_manifest.py`
- Create: `scripts/promote_class_outfit_colors.py`
- Create: `scripts/test_class_outfit_color_manifest.py`
- Create: `scripts/test_promote_class_outfit_colors.py`
- Create: `harness/models/synty/characters/outfit-customization/v1/manifest.json`
- Create: `harness/models/synty/characters/outfit-customization/v1/masks/{barbarian-01,fighter-16,monk-08,rogue-10}.png`
- Create: `evidence/119-class-outfit-colors/README.md`
- Create: `evidence/119-class-outfit-colors/verification.json`
- Modify: `scripts/build_synty_complete_inventory.py`
- Modify: `scripts/test_build_web_asset_catalog.py`
- Modify: `harness/catalogs/synty-complete-inventory.json`
- Modify: `README.md`

**Interfaces:**
- Consumes: approved Task 2 config and masks.
- Produces: `build_outfit_manifest(config, mask_root, provider_root) -> dict[str, object]`.
- Produces: `validate_outfit_manifest(document, provider_root) -> dict[str, object]`.
- Produces: `outfit_manifest_bytes(document) -> bytes` using sorted keys, two-space indentation, UTF-8, and trailing newline.
- Produces: `apply_release(candidate_root, destination_repo)`, `check_release(...)`, and rollback/recovery behavior matching the aggregate customization promoter.

- [ ] **Step 1: Write failing manifest tests**

Demand this exact top-level schema:

```python
EXPECTED_KEYS = {
    "schemaVersion", "workflowVersion", "atlas", "profileAuthority",
    "channelEncoding", "classOrder", "outfits", "inventory",
}
self.assertEqual(1, manifest["schemaVersion"])
self.assertEqual("class-outfit-colors-v1", manifest["workflowVersion"])
self.assertEqual(["barbarian", "fighter", "monk", "rogue"], manifest["classOrder"])
self.assertEqual(
    {"primary": "red", "secondary": "green", "preserve": "black", "overlapAllowed": False},
    manifest["channelEncoding"],
)
self.assertEqual({"maskCount": 4, "runtimeFileCount": 5}, manifest["inventory"])
```

Each outfit row must have exactly:

```python
{
    "classRef", "outfit", "mask", "maskSha256",
    "defaultPrimarySrgb", "defaultSecondarySrgb", "meshNames",
}
```

Reject wrong dimensions, metadata/noncanonical PNGs, nonzero blue/alpha variation, channel overlap, empty primary or secondary coverage, wrong class/outfit ordering, duplicate/identity mesh names, unsafe paths, stale hashes, stale profile-authority hash, missing/extra files, symlinks, traversal, and any modification to the pre-release provider snapshot.

- [ ] **Step 2: Write failing promotion transaction tests**

Model apply/idempotence/rollback/recovery using temporary repositories:

```python
before = snapshot_tree(repo)
with self.assertRaisesRegex(RuntimeError, "injected install failure"):
    apply_release(candidate, repo, install=fail_after_second_file)
self.assertEqual(before, snapshot_tree(repo))

apply_release(candidate, repo)
first = snapshot_tree(repo)
apply_release(candidate, repo)
self.assertEqual(first, snapshot_tree(repo))
```

Also assert promotion governs exactly the five runtime files and never includes `characters/customization/dwarf-v1` or any `.glb`.

- [ ] **Step 3: Run focused tests and confirm red**

```bash
python3 -m unittest \
  scripts.test_class_outfit_color_manifest \
  scripts.test_promote_class_outfit_colors -v
```

Expected: missing modules/functions.

- [ ] **Step 4: Implement manifest, structural UV checks, and promotion**

Build class rows in the approved order. Derive each initial outfit set from the Human body's `sourceMeshes` minus Human identity meshes, then remove the approved class config's exact `excludedMeshNames`; emit the remaining exact ordered values as `meshNames`. Validate every exclusion names a real non-identity mesh and validate actual GLB nodes/materials against the manifest. Decode float32 `TEXCOORD_0` accessors and canonicalize each declared outfit node as the sorted multiset of exact 8-byte float32 UV pairs. Require every race variant of a class to contain each exact Human outfit node name and to match its approved Human exact-value UV digest; sorting tolerates exporter-only duplicate-vertex order while preserving values and multiplicity. Do not strip suffixes, round values, or fuzzy-match names. Require both mask channels to be sampled by at least one declared outfit triangle and require identity-only nodes to receive no treatment declaration.

Promotion builds the five-file candidate outside the canonical destination, validates it in an external temporary view, swaps the complete `outfit-customization/v1` root through a same-filesystem temporary sibling, restores the old root after any exception, and recovers an interrupted backup before a new attempt.

- [ ] **Step 5: Run focused tests and publish locally**

```bash
python3 -m unittest \
  scripts.test_class_outfit_colors \
  scripts.test_build_class_outfit_color_review \
  scripts.test_class_outfit_color_manifest \
  scripts.test_promote_class_outfit_colors -v
python3 scripts/promote_class_outfit_colors.py --build --apply
python3 scripts/promote_class_outfit_colors.py --check
```

Expected: four approved masks plus one manifest, exact check clean, zero changed GLBs.

- [ ] **Step 6: Seal evidence and inventory**

Write `verification.json` with provider head, source archive/member hashes, four mask hashes, manifest hash, per-class coverage counts and UV digests, pre-existing-tree hash, unchanged-file count, Blender scene hash, and Kirk's exact verdict. Keep local absolute paths and licensed filenames out of the portable receipt except the public source archive/member provenance already approved in provider config.

Then run:

```bash
python3 scripts/build_synty_complete_inventory.py
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/verify_web_asset_stage.py --verify-only
MESH_STATS_CHECK=$(mktemp)
python3 scripts/build_mesh_stats.py --output "$MESH_STATS_CHECK"
cmp "$MESH_STATS_CHECK" harness/models/synty/mesh-stats.json
rm -f "$MESH_STATS_CHECK"
```

Update the inventory test's exact file count/tree hash from generated output; do not hand-calculate or weaken the baseline subtraction guard.

- [ ] **Step 7: Run the one final provider suite**

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/promote_class_outfit_colors.py --check
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/verify_web_asset_stage.py --verify-only
git diff --check
git status --short
```

Expected: all tests/checks pass; status contains only intended tracked source, runtime, evidence, inventory, and README changes; no `.blend`, source texture, archive, or unrelated GLB change.

- [ ] **Step 8: Commit, open the provider PR, review once, and merge**

```bash
git add scripts/class_outfit_color_manifest.py \
  scripts/promote_class_outfit_colors.py \
  scripts/test_class_outfit_color_manifest.py \
  scripts/test_promote_class_outfit_colors.py \
  scripts/build_synty_complete_inventory.py \
  scripts/test_build_web_asset_catalog.py \
  scripts/configs/class-outfit-colors-v1.json \
  harness/models/synty/characters/outfit-customization/v1 \
  harness/catalogs/synty-complete-inventory.json \
  evidence/119-class-outfit-colors README.md
git commit -m "asset: publish class outfit color authority (#119)"
git push -u origin feat/119-class-outfit-colors
```

Open one PR for #119, run one independent whole-PR review, resolve every finding on the same branch, rerun Step 7 if the head changes materially, and wait for Kirk's squash merge. Capture the exact merge SHA with:

```bash
PROVIDER_MERGE=$(gh pr view --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq '.mergeCommit.oid')
test "${#PROVIDER_MERGE}" -eq 40
```

---

### Task 4: Add the provider-neutral protobuf contract

**Issue/branch:** `rpg-api-protos#278`, `feat/278-outfit-customization`

**Files:**
- Modify: `dnd5e/api/customization/v1alpha1/types.proto`
- Modify: `dnd5e/api/v1alpha1/character.proto`
- Modify: `dnd5e/api/session/v1alpha1/types.proto`
- Modify: `docs/architecture/components/character-service.md`
- Modify: `docs/status.md`

**Interfaces:**
- Produces: `dnd5e.api.customization.v1alpha1.OutfitCustomization`.
- Produces: `Appearance.outfit = 6`.
- Produces: `session.v1alpha1.Customization.outfit = 2`.

- [ ] **Step 1: Edit the three proto sources**

Add exactly:

```proto
// Describes provider-neutral color intent for the character's fixed class
// outfit. Field absence preserves the provider's authored channel. A future
// item cosmetic envelope may reuse this message without changing its meaning.
message OutfitCustomization {
  // Packed sRGB color in 0xRRGGBB form. Absence preserves provider primary.
  optional uint32 primary_color_srgb = 1;
  // Packed sRGB color in 0xRRGGBB form. Absence preserves provider secondary.
  optional uint32 secondary_color_srgb = 2;
}
```

Add to `Appearance`:

```proto
dnd5e.api.customization.v1alpha1.OutfitCustomization outfit = 6;
```

Add to session `Customization`:

```proto
dnd5e.api.customization.v1alpha1.OutfitCustomization outfit = 2;
```

Update comments that currently say malformed selection “must be rejected by the API” or describe Hair as an API-side addition: semantic refusal and public projection are toolkit-owned; the API converts and delegates. Keep deprecated fields 1–4 present, deprecated, and inert.

- [ ] **Step 2: Format and inspect the exact diff**

```bash
buf format -w --disable-symlinks
git diff -- dnd5e/api/customization/v1alpha1/types.proto \
  dnd5e/api/v1alpha1/character.proto \
  dnd5e/api/session/v1alpha1/types.proto
git diff --check
```

Expected: only additive fields/new comments; fields 1–5 are unchanged.

- [ ] **Step 3: Run the complete proto gates**

```bash
make lint
make breaking
make generate
make compile-go
make compile-ts
make test
```

Expected: all pass. Do not add bespoke serialization/presence tests for generated protobuf mechanics.

- [ ] **Step 4: Commit, review, and merge**

```bash
git add dnd5e/api/customization/v1alpha1/types.proto \
  dnd5e/api/v1alpha1/character.proto \
  dnd5e/api/session/v1alpha1/types.proto \
  docs/architecture/components/character.md docs/status.md
git commit -m "feat: add outfit customization contract (#278)"
git push -u origin feat/278-outfit-customization
```

Open one PR for #278, complete one independent review, merge, and wait for generated-branch publication. Verify the source merge and generated commit:

```bash
PROTO_MERGE=$(gh pr view --repo KirkDiggler/rpg-api-protos \
  --json mergeCommit --jq '.mergeCommit.oid')
PROTO_GENERATED=$(gh api repos/KirkDiggler/rpg-api-protos/git/ref/heads/generated \
  --jq '.object.sha')
test "${#PROTO_MERGE}" -eq 40
test "${#PROTO_GENERATED}" -eq 40
```

---

### Task 5: Put complete Appearance semantics in the D&D toolkit module

**Issue/branch:** `rpg-toolkit#1450`, `feat/1450-appearance-sdk`

**Files:**
- Create: `rulebooks/dnd5e/customization/customization.go`
- Create: `rulebooks/dnd5e/customization/customization_test.go`
- Modify: `rulebooks/dnd5e/README.md` or nearest architecture component that enumerates D&D packages
- Modify: `docs/status.md`
- Modify: `docs/quality.md` if its D&D score rationale changes

**Interfaces:**
- Produces: `customization.Appearance`, `HairCustomization`, `OutfitCustomization`, `StyleSelection`, and `StyleSelectionKind`.
- Produces: `ValidateAppearance(*Appearance) error` and `CloneAppearance(*Appearance) *Appearance`.
- Error owner: `rpgerr.CodeInvalidArgument` with stable field-qualified messages.

- [ ] **Step 1: Write the failing semantic refusal matrix**

Define the desired toolkit shapes in tests:

```go
appearance := &customization.Appearance{
    Hair: &customization.HairCustomization{
        Scalp: &customization.StyleSelection{
            Kind: customization.StyleSelectionStyle,
            StyleRef: "modular-fantasy-hero:hair:38",
        },
        FacialHair: &customization.StyleSelection{Kind: customization.StyleSelectionNone},
        ColorSRGB: ptr(uint32(0)),
        Roughness: ptr(float32(0)),
    },
    Outfit: &customization.OutfitCustomization{
        PrimaryColorSRGB: ptr(uint32(0)),
        SecondaryColorSRGB: ptr(uint32(0xFFFFFF)),
    },
}
require.NoError(t, customization.ValidateAppearance(appearance))
```

Table-test nil and empty Appearance as valid values for persistence helpers, while `Draft.SetAppearance` will separately require a nonnil command value. Reject:

- selection Kind empty/unknown;
- style Kind with empty ref;
- style ref over 256 UTF-8 bytes;
- none Kind with nonempty ref;
- hair color, primary, or secondary above `0xFFFFFF`;
- roughness below zero, above one, NaN, positive infinity, or negative infinity.

Assert validation does not mutate input and does not reject unknown but well-shaped provider refs.

- [ ] **Step 2: Write failing clone-isolation tests**

```go
clone := customization.CloneAppearance(appearance)
require.Equal(t, appearance, clone)
require.NotSame(t, appearance, clone)
require.NotSame(t, appearance.Hair, clone.Hair)
require.NotSame(t, appearance.Hair.ColorSRGB, clone.Hair.ColorSRGB)
require.NotSame(t, appearance.Outfit, clone.Outfit)
require.NotSame(t, appearance.Outfit.PrimaryColorSRGB, clone.Outfit.PrimaryColorSRGB)
*clone.Outfit.PrimaryColorSRGB = 0x123456
require.Zero(t, *appearance.Outfit.PrimaryColorSRGB)
```

- [ ] **Step 3: Run tests and confirm red**

```bash
cd /home/kirk/.pi/worktrees/rpg-toolkit/1450-appearance-sdk/rulebooks/dnd5e
go test ./customization -run 'Appearance|Style|RGB|Roughness' -count=1
```

Expected: package or symbols missing.

- [ ] **Step 4: Implement the low-dependency customization package**

Use these public types and constants:

```go
type StyleSelectionKind string

const (
    StyleSelectionStyle StyleSelectionKind = "style"
    StyleSelectionNone  StyleSelectionKind = "none"
)

type StyleSelection struct {
    Kind StyleSelectionKind `json:"kind"`
    StyleRef string `json:"style_ref,omitempty"`
}

type HairCustomization struct {
    Scalp *StyleSelection `json:"scalp,omitempty"`
    FacialHair *StyleSelection `json:"facial_hair,omitempty"`
    ColorSRGB *uint32 `json:"color_srgb,omitempty"`
    Roughness *float32 `json:"roughness,omitempty"`
}

type OutfitCustomization struct {
    PrimaryColorSRGB *uint32 `json:"primary_color_srgb,omitempty"`
    SecondaryColorSRGB *uint32 `json:"secondary_color_srgb,omitempty"`
}

type Appearance struct {
    Hair *HairCustomization `json:"hair,omitempty"`
    Outfit *OutfitCustomization `json:"outfit,omitempty"`
}
```

Keep provider membership, class compatibility, URL/path interpretation, and defaults out. Validate exact shape/ranges and return `rpgerr` errors. Clone every optional pointer and nested selection.

- [ ] **Step 5: Run focused and module tests**

```bash
go test ./customization -count=1
go test ./... -count=1
```

Expected: pass.

- [ ] **Step 6: Commit the semantic owner**

```bash
git add rulebooks/dnd5e/customization \
  rulebooks/dnd5e/README.md docs/status.md docs/quality.md
git commit -m "feat(dnd5e): own character appearance semantics (#1450)"
```

---

### Task 6: Carry Appearance through toolkit drafts and characters

**Files:**
- Modify: `rulebooks/dnd5e/character/inputs.go`
- Modify: `rulebooks/dnd5e/character/draft.go`
- Modify: `rulebooks/dnd5e/character/draft_data.go`
- Modify: `rulebooks/dnd5e/character/data.go`
- Modify: `rulebooks/dnd5e/character/character.go`
- Modify: `rulebooks/dnd5e/character/load.go`
- Create: `rulebooks/dnd5e/character/appearance_test.go`
- Modify: `rulebooks/dnd5e/character/character_test.go`
- Modify: `rulebooks/dnd5e/character/README.md`

**Interfaces:**
- Consumes: Task 5 `customization.Appearance` and clone/validation functions.
- Produces: `SetAppearanceInput{Appearance *customization.Appearance}`.
- Produces: `(*Draft).SetAppearance(*SetAppearanceInput) error` and `(*Draft).Appearance() *customization.Appearance`.
- Produces: `DraftData.Appearance`, `Data.Appearance`, and `(*Character).Appearance()`.

- [ ] **Step 1: Write failing Draft tests**

Test response-independent mutation and class carryover:

```go
input := completeAppearance()
require.NoError(t, draft.SetAppearance(&character.SetAppearanceInput{Appearance: input}))
input.Outfit.PrimaryColorSRGB = ptr(uint32(0xFFFFFF))
require.Equal(t, uint32(0x102030), *draft.Appearance().Outfit.PrimaryColorSRGB)

before := draft.Appearance()
require.NoError(t, draft.SetClass(validRogueClassInput()))
require.Equal(t, before, draft.Appearance())
```

Assert nil input/nil Appearance refuse without mutation, malformed hair/outfit returns the exact customization validation error, and a later valid call replaces the whole Appearance atomically.

- [ ] **Step 2: Write failing persistence/finalization tests**

Require:

```go
draftData := draft.ToData()
require.NotSame(t, draft.Appearance(), draftData.Appearance)
reloaded := character.LoadDraftFromData(draftData)
require.Equal(t, draft.Appearance(), reloaded.Appearance())

final, err := reloaded.ToCharacter(ctx, "char-1", events.NewEventBus())
require.NoError(t, err)
require.Equal(t, reloaded.Appearance(), final.Appearance())

stored := final.ToData()
loaded, err := character.Load(context.Background(), stored)
require.NoError(t, err)
require.Equal(t, final.Appearance(), loaded.Appearance())
```

Mutate every nested pointer on each returned copy and prove sources remain unchanged. Add Appearance to the existing complete Data round-trip suite.

- [ ] **Step 3: Run focused tests and confirm red**

```bash
go test ./character -run 'Appearance|LoadFromDataRoundTrip' -count=1
```

Expected: missing fields/methods.

- [ ] **Step 4: Implement one validated draft mutation and complete persistence**

Add `appearance *customization.Appearance` to `Draft` and `Character`. `Draft.SetAppearance` validates first, clones once after successful validation, updates `updatedAt`, and does not change progress flags. `SetClass` does not touch Appearance. All getters, `ToData`, loaders, and finalization return/store deep clones; no mutable pointer crosses a toolkit boundary.

- [ ] **Step 5: Run final D&D module gates**

```bash
go fmt ./...
go test ./... -count=1
golangci-lint run ./...
git diff --check
git status --short
```

Expected: complete D&D module pass and only #1450 files changed.

- [ ] **Step 6: Commit, open one D&D-module PR, review, and merge**

```bash
git add rulebooks/dnd5e/character rulebooks/dnd5e/customization \
  rulebooks/dnd5e/README.md docs/status.md docs/quality.md
git commit -m "feat(dnd5e): persist character appearance (#1450)"
git push -u origin feat/1450-appearance-sdk
```

Open one PR for #1450, complete one independent review, resolve findings, rerun Step 5 after material changes, and squash merge. Fetch tags and require exactly one D&D tag at the merge:

```bash
git fetch origin main --tags
DND_MERGE=$(gh pr view --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
DND_TAG=$(git tag --points-at "$DND_MERGE" 'rulebooks/dnd5e/v*' | sort -V | tail -1)
test -n "$DND_TAG"
printf '%s\n' "$DND_TAG"
```

Do not create the Session branch until this command yields the CI-minted tag.

---

### Task 7: Project public identity through the independently tagged Session SDK

**Issue/branch:** `rpg-toolkit#1451`, branch from fresh `origin/main` after Task 6

**Files:**
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`
- Modify: `rulebooks/dnd5e/session/types.go`
- Modify: `rulebooks/dnd5e/session/read.go`
- Modify: `rulebooks/dnd5e/session/errors.go`
- Modify: `rulebooks/dnd5e/session/convert.go`
- Create: `rulebooks/dnd5e/session/roster_test.go`
- Modify: `rulebooks/dnd5e/session/manager_test.go`
- Modify: `rulebooks/dnd5e/session/README.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md` if Session's score rationale changes

**Interfaces:**
- Consumes: exact `DND_TAG` from Task 6.
- Produces: `RosterInput`, `RosterOutput`, `PublicMember`, and Session-owned customization projection types.
- Produces: `(*Manager).Roster(context.Context, *RosterInput) (*RosterOutput, error)`.
- Produces: `ErrNotSeated` for an authenticated principal that owns no requested session seat.

- [ ] **Step 1: Create the branch and pin the released D&D module**

```bash
git -C /home/kirk/game-dev/rpg-toolkit fetch origin main --tags
git -C /home/kirk/game-dev/rpg-toolkit worktree add \
  /home/kirk/.pi/worktrees/rpg-toolkit/1451-session-roster \
  -b feat/1451-session-roster origin/main
cd /home/kirk/.pi/worktrees/rpg-toolkit/1451-session-roster/rulebooks/dnd5e/session
DND_PR=$(gh issue view 1450 --repo KirkDiggler/rpg-toolkit \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
DND_MERGE=$(gh pr view "$DND_PR" --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
DND_TAG=$(git tag --points-at "$DND_MERGE" 'rulebooks/dnd5e/v*' | sort -V | tail -1)
test -n "$DND_TAG"
GOPROXY=direct go get "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@${DND_TAG#rulebooks/dnd5e/}"
go mod tidy
```

Verify `go.mod` names a released semantic version, not `replace` or a pseudo-version:

```bash
rg 'github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e v[0-9]+\.[0-9]+\.[0-9]+' go.mod
! rg 'replace|-[0-9]{14}-[0-9a-f]{12}' go.mod
```

- [ ] **Step 2: Write failing Roster acceptance tests**

Define the exact SDK output:

```go
out, err := mgr.Roster(ctx, &session.RosterInput{
    Session: "sess",
    Player: "player-alice",
})
require.NoError(t, err)
require.Equal(t, []session.PublicMember{
    {
        ID: "alice", Kind: session.KindPlayer, Name: "Alice",
        ClassRef: "fighter", RaceRef: "human",
        Customization: session.Customization{
            Hair: &session.HairCustomization{/* exact style/color/roughness */},
            Outfit: &session.OutfitCustomization{
                PrimaryColorSRGB: ptr(uint32(0)),
                SecondaryColorSRGB: ptr(uint32(0xFFFFFF)),
            },
        },
    },
    {
        ID: "skel-1", Kind: session.KindMonster, Name: "Skeleton",
        MonsterRef: "dnd5e:monsters:skeleton",
        Customization: session.Customization{},
    },
}, out.Members)
```

Assert encounter roster order is retained, no positions/private sheet values exist in exported roster types, world NPCs are excluded, player identity is read fresh from `CharacterRepository`, and monster name/ref come from session-owned NPC data.

Test nil input, empty session, empty player, missing session/encounter/character/NPC, corrupt character, unknown member kind, and player not seated. Mutate returned nested pointers and prove repository data is unchanged. Add a restart test by rebuilding Manager over copied persistence data.

- [ ] **Step 3: Run focused tests and confirm red**

```bash
go test ./... -run 'Roster|NotSeated' -count=1
```

Expected: missing Roster types/method.

- [ ] **Step 4: Implement proto-shaped Session-owned projection types**

Add flat Session-owned twins, not aliases to inner customization types:

```go
type RosterInput struct {
    Session string
    Player string // authenticated host principal; never client-trusted
}

type RosterOutput struct { Members []PublicMember `json:"members,omitempty"` }

type PublicMember struct {
    ID string `json:"id"`
    Kind MemberKind `json:"kind"`
    Name string `json:"name"`
    ClassRef string `json:"class_ref,omitempty"`
    RaceRef string `json:"race_ref,omitempty"`
    MonsterRef string `json:"monster_ref,omitempty"`
    Customization Customization `json:"customization"`
}
```

Define Session-owned StyleSelection/Hair/Outfit/Customization flat values in `types.go`. Conversion from `character.Data.Appearance` lives in `convert.go` and deep-copies all optional pointers. It does not revalidate semantics; the D&D module owns that.

`Manager.Roster` loads session and encounter, enumerates `enc.Members()`, loads every player record through `CharacterRepository`, and passes it through strict `character.Load` before projection so corrupt Appearance remains a D&D-owned `ErrBadCharacter` refusal. It matches monsters against `SessionData.NPCs`, excludes `KindWorld`, and returns `ErrNotSeated` unless at least one player row has `PlayerID == in.Player`. It never returns positions or sheet-private fields.

- [ ] **Step 5: Run Session gates and verified transcript**

```bash
go fmt ./...
go test ./... -count=1
golangci-lint run ./...
cd /home/kirk/.pi/worktrees/rpg-toolkit/1451-session-roster
./scripts/verify.sh rulebooks/dnd5e/session
git diff --check
git status --short
```

Expected: pass; no local replace/pseudo-version.

- [ ] **Step 6: Commit, open one Session-module PR, review, and merge**

```bash
git add rulebooks/dnd5e/session docs/status.md docs/quality.md
git commit -m "feat(session): project public character customization (#1451)"
git push -u origin feat/1451-session-roster
```

Open one PR for #1451, complete one independent review, resolve findings, rerun Step 5 after material changes, and squash merge. Fetch and verify the CI-minted Session tag:

```bash
git fetch origin main --tags
SESSION_MERGE=$(gh pr view --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
SESSION_TAG=$(git tag --points-at "$SESSION_MERGE" \
  'rulebooks/dnd5e/session/v*' | sort -V | tail -1)
test -n "$SESSION_TAG"
printf '%s\n' "$SESSION_TAG"
```

---

### Task 8: Convert and delegate complete Appearance in the API

**Issue/branch:** `rpg-api#897`, `feat/897-appearance-sdk`

**Files:**
- Modify: `go.mod`
- Modify: `go.sum`
- Modify: `internal/converters/customization/converters.go`
- Modify: `internal/converters/customization/converters_test.go`
- Delete: `internal/entities/appearance.go`
- Modify: `internal/entities/character.go`
- Modify: `internal/entities/character_draft.go`
- Delete: `internal/handlers/dnd5e/v1alpha1/character/appearance_validation.go`
- Replace semantic tests in: `internal/handlers/dnd5e/v1alpha1/character/appearance_validation_test.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/handler.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/handler_test.go`
- Modify: `internal/orchestrators/character/service.go`
- Modify: `internal/orchestrators/character/orchestrator.go`
- Modify: `internal/integration/character/creation_test.go`
- Modify: `internal/repositories/character_draft/redis.go`
- Replace: `internal/repositories/character_draft/redis_appearance_test.go`
- Modify affected fixtures/tests under `internal/repositories/character/`, `internal/orchestrators/`, and `internal/sandboxseed/`

**Interfaces:**
- Consumes: generated proto commit from Task 4 and released D&D tag from Task 6.
- Produces: `ProtoToToolkit(*characterpb.Appearance) *customization.Appearance` and `ToolkitToProto(*customization.Appearance) *characterpb.Appearance`.
- Produces orchestrator `SetAppearanceInput{DraftID string; Appearance *customization.Appearance}`.
- Handler delegates semantic validation only through `Draft.SetAppearance`.

- [ ] **Step 1: Pin published dependencies**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/897-appearance-sdk
git -C /home/kirk/game-dev/rpg-api-protos fetch origin generated
PROTO_PR=$(gh issue view 278 --repo KirkDiggler/rpg-api-protos \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
PROTO_MERGE=$(gh pr view "$PROTO_PR" --repo KirkDiggler/rpg-api-protos \
  --json mergeCommit --jq '.mergeCommit.oid')
PROTO_GENERATED=$(git -C /home/kirk/game-dev/rpg-api-protos rev-parse origin/generated)
test "$(git -C /home/kirk/game-dev/rpg-api-protos rev-parse "${PROTO_GENERATED}^")" = "$PROTO_MERGE"
DND_PR=$(gh issue view 1450 --repo KirkDiggler/rpg-toolkit \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
DND_MERGE=$(gh pr view "$DND_PR" --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
DND_TAG=$(git -C /home/kirk/game-dev/rpg-toolkit tag --points-at "$DND_MERGE" \
  'rulebooks/dnd5e/v*' | sort -V | tail -1)
test -n "$DND_TAG"
GOPROXY=direct go get "github.com/KirkDiggler/rpg-api-protos/gen/go@${PROTO_GENERATED}"
GOPROXY=direct go get "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@${DND_TAG#rulebooks/dnd5e/}"
go mod tidy
```

Assert no `replace` directive and generated Go exposes `OutfitCustomization`.

- [ ] **Step 2: Rewrite converter tests first**

Demand exact optional/oneof preservation for complete Appearance:

```go
wire := &characterpb.Appearance{
    Hair: &customizationpb.HairCustomization{
        Scalp: &customizationpb.StyleSelection{Selection: &customizationpb.StyleSelection_StyleRef{StyleRef: "unknown:hair:ok"}},
        FacialHair: &customizationpb.StyleSelection{}, // malformed shape must reach toolkit
        ColorSrgb: proto.Uint32(0),
        Roughness: proto.Float32(0),
    },
    Outfit: &customizationpb.OutfitCustomization{
        PrimaryColorSrgb: proto.Uint32(0),
        SecondaryColorSrgb: proto.Uint32(0xFFFFFF),
    },
}
got := customizationconverter.ProtoToToolkit(wire)
require.Equal(t, customization.StyleSelectionKind(""), got.Hair.FacialHair.Kind)
require.NotNil(t, got.Outfit.PrimaryColorSRGB)
require.Zero(t, *got.Outfit.PrimaryColorSRGB)
require.Equal(t, wire, customizationconverter.ToolkitToProto(got))
```

Cover nil, empty, primary-only, secondary-only, both, explicit black, style, none, and malformed no-oneof. The converter must not call validation.

- [ ] **Step 3: Run converter tests and confirm red**

```bash
go test ./internal/converters/customization -count=1
```

Expected: old entity-shaped signatures fail.

- [ ] **Step 4: Convert API entities and repository wrappers to toolkit-owned data**

Keep `entities.Character` and `entities.CharacterDraft` only as storage wrappers around `*character.Data`/`*character.DraftData`; remove their sibling `Appearance` fields. Redis repositories marshal those wrappers without interpreting `Data.Appearance`. Remove cloning/preservation code that special-cases hair pointers; toolkit `ToData`/load boundaries own deep-copy behavior.

Update sandbox and repository fixtures to put Appearance inside toolkit data. Do not write a legacy outer-`appearance` migration path.

- [ ] **Step 5: Write handler/orchestrator delegation tests**

The handler test must pass malformed semantics through conversion to the mocked service rather than refuse locally. The orchestrator test must load DraftData, call toolkit mutation, refuse toolkit errors without repository Update, and persist the updated `draft.ToData()` after success:

```go
repo.EXPECT().Get(ctx, characterdraft.GetInput{ID: "draft-1"}).Return(storedDraft(), nil)
repo.EXPECT().Update(ctx, gomock.Any()).DoAndReturn(func(_ context.Context, in characterdraft.UpdateInput) (*characterdraft.UpdateOutput, error) {
    require.Equal(t, uint32(0x102030), *in.Draft.Data.Appearance.Outfit.PrimaryColorSRGB)
    return &characterdraft.UpdateOutput{Draft: in.Draft}, nil
})

out, err := orch.SetAppearance(ctx, &characterorchestrator.SetAppearanceInput{
    DraftID: "draft-1",
    Appearance: validToolkitAppearance(),
})
require.NoError(t, err)
require.Equal(t, validToolkitAppearance(), out.Draft.Appearance)
```

Change `SetAppearanceOutput` to return complete updated `*character.DraftData`, avoiding a second Get solely to rebuild the response.

- [ ] **Step 6: Implement pure conversion and one toolkit mutation**

`UpdateAppearance` checks only nil request, required draft ID, required Appearance message, authentication/ownership gates already used by the character service, and conversion errors caused by impossible transport shapes. It converts, calls the service once, and converts returned DraftData. Delete `validateAppearance` and its numeric/style logic.

The orchestrator loads toolkit DraftData, calls `draft.SetAppearance`, persists `draft.ToData`, and returns that stored result. Finalization naturally carries `Data.Appearance`; Get/List and equipment paths read/write the same toolkit Data without sibling preservation.

- [ ] **Step 7: Run focused API tests**

```bash
go test ./internal/converters/customization -count=1
go test ./internal/handlers/dnd5e/v1alpha1/character -run 'Appearance|GetCharacter|ListCharacters' -count=1
go test ./internal/orchestrators/character -run 'Appearance|Finalize|Equip' -count=1
go test ./internal/repositories/character_draft ./internal/repositories/character -run 'Appearance|Equipment' -count=1
go test ./internal/integration/character -run 'Appearance|Customization' -count=1
```

Expected: pass; semantic refusal assertions live in toolkit tests, while API integration proves errors pass through unchanged.

- [ ] **Step 8: Commit the character-side ownership migration**

```bash
git add go.mod go.sum internal/converters/customization internal/entities \
  internal/handlers/dnd5e/v1alpha1/character \
  internal/orchestrators/character internal/repositories/character \
  internal/repositories/character_draft internal/integration/character \
  internal/sandboxseed
git commit -m "refactor: delegate appearance semantics to toolkit (#897)"
```

---

### Task 9: Replace API roster state with the Session SDK roster

**Files:**
- Modify: `go.mod`
- Modify: `go.sum`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/handler.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/mock/mock_manager.go` (generated)
- Replace: `internal/handlers/dnd5e/session/v1alpha1/get_roster.go`
- Replace: `internal/handlers/dnd5e/session/v1alpha1/get_roster_test.go`
- Modify: `internal/handlers/dnd5e/sessionaccess/access.go`
- Modify: `internal/handlers/dnd5e/sessionaccess/access_test.go`
- Modify: `internal/handlers/dnd5e/sessionpresentation/v1alpha1/handler_test.go`
- Modify: `internal/orchestrators/lobby/orchestrator.go`
- Modify: `internal/orchestrators/lobby/start_encounter_session_stack.go`
- Modify: `internal/orchestrators/lobby/start_encounter_session_stack_test.go`
- Modify: `internal/orchestrators/session/orchestrator.go`
- Modify: `internal/integration/harness/harness.go`
- Modify: `internal/integration/session/acceptance_test.go`
- Delete: `internal/repositories/roster/`
- Modify: `docs/architecture/components/character-handler.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md`

**Interfaces:**
- Consumes: exact Session tag from Task 7.
- Extends handler-local `Manager` with `Roster(ctx, *sdk.RosterInput) (*sdk.RosterOutput, error)`.
- `GetRoster` supplies authenticated player ID to exactly one SDK Roster call and converts its flat output.

- [ ] **Step 1: Pin the released Session module and regenerate manager mocks**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/897-appearance-sdk
SESSION_PR=$(gh issue view 1451 --repo KirkDiggler/rpg-toolkit \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
SESSION_MERGE=$(gh pr view "$SESSION_PR" --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
SESSION_TAG=$(git -C /home/kirk/game-dev/rpg-toolkit tag --points-at "$SESSION_MERGE" \
  'rulebooks/dnd5e/session/v*' | sort -V | tail -1)
test -n "$SESSION_TAG"
GOPROXY=direct go get "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@${SESSION_TAG#rulebooks/dnd5e/session/}"
go mod tidy
go generate ./internal/handlers/dnd5e/session/v1alpha1
```

- [ ] **Step 2: Write failing thin-handler tests**

Require one call with authenticated principal and exact output conversion:

```go
manager.EXPECT().Roster(gomock.Any(), &sdk.RosterInput{
    Session: "sess-1",
    Player: "player-1",
}).Return(&sdk.RosterOutput{Members: []sdk.PublicMember{/* player + monster */}}, nil)

response, err := handler.GetRoster(auth.WithPlayerID(context.Background(), "player-1"),
    &sessionpb.GetRosterRequest{Session: "sess-1"})
require.NoError(t, err)
require.Equal(t, uint32(0), response.Members[0].Customization.Outfit.GetPrimaryColorSrgb())
```

Test unauthenticated and empty-session transport refusals before SDK call, plus translation of `ErrNoSession`, `ErrNoEncounter`, `ErrNoCharacter`, `ErrBadCharacter`, `ErrNotSeated`, and repository failures. Remove tests that expect API to loop character records or assemble customization itself.

- [ ] **Step 3: Write failing no-duplicate-roster tests**

Update lobby tests to assert StartEncounter calls only `StartSession`, `Join`, and `Spawn`; no API roster Save occurs. Update construction tests so session Handler/Orchestrator/Harness require no roster repository. Keep session access tests for authenticated ownership; where a presentation endpoint requires a seated member, inject a small `RosterReader` interface backed by `sdk.Manager.Roster` instead of storage.

- [ ] **Step 4: Run focused tests and confirm red**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1 \
  ./internal/handlers/dnd5e/sessionaccess \
  ./internal/orchestrators/lobby -run 'Roster|Access|StartEncounter' -count=1
```

Expected: old roster repository expectations fail.

- [ ] **Step 5: Implement SDK-only roster projection**

`GetRoster` extracts auth/session, calls `Manager.Roster` once, and maps Session-owned values to proto mechanically. Keep the mapping in a focused converter helper and copy all optional pointers; do not validate contents or choose defaults.

Remove roster construction from lobby launch, roster dependencies from configs/harnesses, and `internal/repositories/roster`. `sessionaccess.Access` may still compare authenticated principal to `character.Data.PlayerID` for ownership, but membership/public identity comes only from the SDK. Presentation calls that need an explicit seated check call `RosterReader.Roster`; ordinary member-taking SDK verbs retain their existing member-existence refusal.

- [ ] **Step 6: Run API integration and full CI once**

Restart miniredis fixtures rather than migrating old wrapper bytes, then run:

```bash
go test ./internal/integration/character ./internal/integration/session -count=1
make ci-check
git diff --check
! rg 'replace ' go.mod
git status --short
```

Expected: full API CI passes; no `internal/entities.Appearance`, semantic validator, or `internal/repositories/roster` remains.

- [ ] **Step 7: Commit, open one API PR, review, and merge**

```bash
git add -A
git commit -m "refactor: delegate appearance and roster to toolkit (#897)"
git push -u origin feat/897-appearance-sdk
```

Open one PR against `dev`, complete one independent whole-PR review, resolve every finding on the same branch, rerun Step 6 after material changes, and wait for Kirk's squash merge. Record the exact API merge SHA for the integrated evidence receipt.

---

### Task 10: Extend the exact generated web catalog with outfit treatment

**Issue/branch:** `rpg-dnd5e-web#912`, `feat/912-class-outfit-colors`

**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`
- Modify: `scripts/generateCharacterCustomizationCatalog.ts`
- Modify: `scripts/generateCharacterCustomizationCatalog.test.ts`
- Modify: `scripts/characterCustomizationPublication.test.ts`
- Modify: `scripts/sync-game-assets.sh`
- Modify: `scripts/sync-game-assets.test.ts`
- Regenerate: `src/generated/characterCustomizationCatalog.ts`
- Ignored sync: `public/models/synty/characters/outfit-customization/v1/`

**Interfaces:**
- Consumes exact provider path: `harness/models/synty/characters/outfit-customization/v1/manifest.json`.
- Produces `OutfitTreatment` and `CharacterCustomizationCatalog.outfits` keyed by the four starter classes.
- Receipt grows from 969 to 974 exact provider source files; publication runtime references grow from 960 to 964.

- [ ] **Step 1: Pin the generated proto commit**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/912-class-outfit-colors
git -C /home/kirk/game-dev/rpg-api-protos fetch origin generated
PROTO_PR=$(gh issue view 278 --repo KirkDiggler/rpg-api-protos \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
PROTO_MERGE=$(gh pr view "$PROTO_PR" --repo KirkDiggler/rpg-api-protos \
  --json mergeCommit --jq '.mergeCommit.oid')
PROTO_GENERATED=$(git -C /home/kirk/game-dev/rpg-api-protos rev-parse origin/generated)
test "$(git -C /home/kirk/game-dev/rpg-api-protos rev-parse "${PROTO_GENERATED}^")" = "$PROTO_MERGE"
npm i --save "github:KirkDiggler/rpg-api-protos#${PROTO_GENERATED}"
npx tsc --noEmit
```

Verify both package files resolve that exact commit.

- [ ] **Step 2: Write failing provider projection tests**

Add exact expected shape:

```ts
expect(catalog.outfits.fighter).toEqual({
  classRef: 'fighter',
  outfit: '16',
  maskUrl:
    '/models/synty/characters/outfit-customization/v1/masks/fighter-16.png',
  maskSha256: expect.stringMatching(/^[0-9a-f]{64}$/),
  defaultPrimaryColorSrgb: 0x49667e,
  defaultSecondaryColorSrgb: 0xd1a44c,
  meshNames: expect.arrayContaining(['Chr_Torso_Male_16']),
});
```

Mutation tests reject unsafe paths, wrong class order/outfit IDs, invalid RGB24 defaults, duplicate/empty mesh names, stale mask hash, unexpected manifest keys, wrong channel encoding, wrong atlas authority, and missing/extra runtime files.

Update deterministic receipt expectations to `sourceAssetCount: 974` and publication to four additional unique mask URLs. Keep zero-or-all ignored mirror behavior and require no tracked `public/models/synty` bytes.

- [ ] **Step 3: Run generator/publication tests and confirm red**

```bash
PROVIDER_PR=$(gh issue view 119 --repo KirkDiggler/rpg-game-assets \
  --json closedByPullRequestsReferences \
  --jq '.closedByPullRequestsReferences[0].number')
PROVIDER_MERGE=$(gh pr view "$PROVIDER_PR" --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq '.mergeCommit.oid')
PROVIDER_WT=/home/kirk/.pi/worktrees/rpg-game-assets/provider-${PROVIDER_MERGE:0:8}
if [ ! -d "$PROVIDER_WT" ]; then
  git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach "$PROVIDER_WT" "$PROVIDER_MERGE"
fi
RPG_GAME_ASSETS_PATH="$PROVIDER_WT" \
  npx vitest run scripts/generateCharacterCustomizationCatalog.test.ts \
  scripts/characterCustomizationPublication.test.ts \
  scripts/sync-game-assets.test.ts
```

Expected: missing outfit manifest projection/type.

- [ ] **Step 4: Extend the generator and fail-closed sync**

Add:

```ts
export interface OutfitTreatment {
  readonly classRef: CustomizationStarterClass;
  readonly outfit: '01' | '16' | '08' | '10';
  readonly maskUrl: string;
  readonly maskSha256: string;
  readonly defaultPrimaryColorSrgb: number;
  readonly defaultSecondaryColorSrgb: number;
  readonly meshNames: readonly string[];
}
```

Parse/validate the separate provider manifest and include its manifest plus four masks in the exact source inventory before rendering the generated module. Extend `CHARACTER_CUSTOMIZATION_PROVIDER` with `outfitManifestSha256`. Preserve the aggregate hair manifest/hash as independent authority.

`sync-game-assets.sh` continues generating into a temporary tracked-module candidate before either rsync destination mutates; outfit validation participates in the same transaction.

- [ ] **Step 5: Generate from the exact merged provider and verify bytes**

```bash
PROVIDER_MERGE=$(gh pr view --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq '.mergeCommit.oid')
PROVIDER_WT=/home/kirk/.pi/worktrees/rpg-game-assets/provider-${PROVIDER_MERGE:0:8}
git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach "$PROVIDER_WT" "$PROVIDER_MERGE"
RPG_GAME_ASSETS_PATH="$PROVIDER_WT" npm run assets:sync
RPG_GAME_ASSETS_PATH="$PROVIDER_WT" \
  npx vitest run scripts/generateCharacterCustomizationCatalog.test.ts \
  scripts/characterCustomizationPublication.test.ts \
  scripts/sync-game-assets.test.ts
```

Expected: generated catalog pins the exact provider merge and both manifest hashes; 964 runtime references are present and ignored; no public asset is tracked.

- [ ] **Step 6: Commit the catalog boundary**

```bash
git add package.json package-lock.json scripts/generateCharacterCustomizationCatalog.ts \
  scripts/generateCharacterCustomizationCatalog.test.ts \
  scripts/characterCustomizationPublication.test.ts \
  scripts/sync-game-assets.sh scripts/sync-game-assets.test.ts \
  src/generated/characterCustomizationCatalog.ts
git commit -m "feat: bind class outfit color authority (#912)"
```

---

### Task 11: Add stable per-character mask treatment to the renderer

**Files:**
- Create: `src/character/customization/outfitCustomization.ts`
- Create: `src/character/customization/outfitCustomization.test.ts`
- Create: `src/components/hex-grid/outfitMaterialTreatment.ts`
- Create: `src/components/hex-grid/outfitMaterialTreatment.test.ts`
- Create: `src/components/hex-grid/OutfitTreatmentSlot.tsx`
- Create: `src/components/hex-grid/OutfitTreatmentSlot.test.tsx`
- Modify: `src/components/hex-grid/runtimeSurfaceTreatment.ts`
- Modify: `src/components/hex-grid/runtimeSurfaceTreatment.test.ts`
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.test.tsx`

**Interfaces:**
- Produces `resolveOutfitPresentation({classRefId, customization}) -> OutfitPresentation | undefined`.
- Produces `prepareOutfitMaterial(source, mask, presentation) -> PreparedOutfitMaterial`.
- Produces `updateOutfitMaterial(prepared, presentation) -> void`.
- `ClassCharacterModelProps` gains `outfit?: OutfitPresentation` and `onOutfitStatus?: (status: OutfitTreatmentStatus) => void`.

- [ ] **Step 1: Write failing neutral resolver tests**

Cover absent defaults, independent values, black, invalid out-of-range values, normalized class spellings, and unsupported classes:

```ts
expect(
  resolveOutfitPresentation({
    classRefId: 'FIGHTER',
    customization: { outfit: { primaryColorSrgb: 0 } },
  })
).toMatchObject({
  classRef: 'fighter',
  primaryColor: '#000000',
  secondaryColor: undefined,
  maskUrl: expect.stringContaining('fighter-16.png'),
});
```

Invalid input returns `undefined` plus a diagnostic status; it never constructs a URL.

- [ ] **Step 2: Write failing shader/material tests**

Call `onBeforeCompile` with a minimal shader fixture and demand:

- one shared mask sampler;
- `usePrimary`, `useSecondary`, `primaryColor`, and `secondaryColor` uniforms;
- injection at `#include <map_fragment>` before stock PBR lighting;
- red/green mask mixing with original sampled map;
- a stable custom program cache key;
- source material/map unchanged;
- exact cloned material disposal only.

Then update colors and assert the same UUIDs:

```ts
const before = {
  material: prepared.material.uuid,
  map: prepared.material.map?.uuid,
  mask: prepared.uniforms.outfitMask.value.uuid,
};
updateOutfitMaterial(prepared, nextPresentation);
expect(prepared.material.uuid).toBe(before.material);
expect(prepared.material.map?.uuid).toBe(before.map);
expect(prepared.uniforms.outfitMask.value.uuid).toBe(before.mask);
expect(prepared.uniforms.primaryColor.value.getHex()).toBe(0x123456);
```

- [ ] **Step 3: Run focused tests and confirm red**

```bash
npx vitest run \
  src/character/customization/outfitCustomization.test.ts \
  src/components/hex-grid/outfitMaterialTreatment.test.ts \
  src/components/hex-grid/OutfitTreatmentSlot.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

Expected: missing modules/props.

- [ ] **Step 4: Implement resolver and material preparation**

`resolveOutfitPresentation` reads only generated authority. For a supported primary body it returns the stable class profile even when both customization fields are absent; absent colors keep optional uniform flags false so the original atlas remains sampled. This prepares material ownership at body mount and prevents the first color choice from recreating the body. It does not manufacture explicit default overrides.

`prepareOutfitMaterial` requires `MeshStandardMaterial`, clones it once, installs `onBeforeCompile`, stores the uniform object outside the shader callback, and returns a stable `customProgramCacheKey` such as `class-outfit-colors-v1`. Replace Three.js 0.181.2's `#include <map_fragment>` with the stock map sample plus dye substitution before `diffuseColor` multiplication:

```glsl
#ifdef USE_MAP
  vec4 sampledDiffuseColor = texture2D(map, vMapUv);
  #ifdef DECODE_VIDEO_TEXTURE
    sampledDiffuseColor = sRGBTransferEOTF(sampledDiffuseColor);
  #endif
  vec4 outfitMaskSample = texture2D(outfitMask, vMapUv);
  if (usePrimary > 0.5) {
    sampledDiffuseColor.rgb = mix(sampledDiffuseColor.rgb, primaryColor, outfitMaskSample.r);
  }
  if (useSecondary > 0.5) {
    sampledDiffuseColor.rgb = mix(sampledDiffuseColor.rgb, secondaryColor, outfitMaskSample.g);
  }
  diffuseColor *= sampledDiffuseColor;
#endif
```

Tests must assert the exact pinned chunk anchor exists before replacement and fail closed if a future Three.js version changes it.

- [ ] **Step 5: Give each body instance stable material ownership**

At `ClassCharacterModel` clone creation, clone source materials per `(source material, treatment eligibility)` pair: declared outfit meshes may share one treated clone, while a head/identity mesh that used the same cached source material receives a separate untreated clone. Depend only on the stable outfit profile key and declared mesh-name set—not color values—when preparing the body clone. Snapshot base color/emissive/opacity/depth values. Refactor entity overlays to reset and mutate those same instance materials instead of creating/discarding new materials on selection changes.

`OutfitTreatmentSlot` loads the shared mask texture behind its own Suspense and inner ErrorBoundary, applies `NearestFilter` and `NoColorSpace` consistently, prepares only provider-declared mesh names, and updates uniforms in place. `ClassCharacterModel` alone owns and disposes its instance materials; the slot never disposes shared atlas/mask textures and only restores its callback/uniform state when it detaches. A mask or declared-node failure reports rejected status and leaves the already-mounted original body primitive visible.

- [ ] **Step 6: Run renderer identity/isolation tests**

```bash
npx vitest run \
  src/components/hex-grid/outfitMaterialTreatment.test.ts \
  src/components/hex-grid/OutfitTreatmentSlot.test.tsx \
  src/components/hex-grid/runtimeSurfaceTreatment.test.ts \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

Require stable mesh/Skeleton/mixer/material/map identities through color and overlay changes, two-character uniform isolation, original cache immunity, animation continuation, hair coexistence, and exact cleanup.

- [ ] **Step 7: Commit renderer treatment**

```bash
git add src/character/customization/outfitCustomization.ts \
  src/character/customization/outfitCustomization.test.ts \
  src/components/hex-grid/outfitMaterialTreatment.ts \
  src/components/hex-grid/outfitMaterialTreatment.test.ts \
  src/components/hex-grid/OutfitTreatmentSlot.tsx \
  src/components/hex-grid/OutfitTreatmentSlot.test.tsx \
  src/components/hex-grid/runtimeSurfaceTreatment.ts \
  src/components/hex-grid/runtimeSurfaceTreatment.test.ts \
  src/components/hex-grid/ClassCharacterModel.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
git commit -m "feat: recolor class outfits per character (#912)"
```

---

### Task 12: Build the Hair / Facial Hair / Gear Colors accordion

**Files:**
- Create: `src/character/creation/components/AppearanceAccordionSection.tsx`
- Create: `src/character/creation/components/AppearanceAccordionSection.test.tsx`
- Create: `src/character/creation/components/SharedHairMaterialControls.tsx`
- Create: `src/character/creation/components/OutfitColorControls.tsx`
- Create: `src/character/creation/components/OutfitColorControls.test.tsx`
- Modify: `src/character/creation/components/CharacterCustomizationControls.tsx`
- Modify: `src/character/creation/components/CharacterCustomizationControls.test.tsx`
- Modify: `src/character/creation/components/CharacterCustomizationPreview.tsx`
- Modify: `src/character/creation/AppearanceSelectionModal.tsx`
- Modify: `src/character/creation/AppearanceSelectionModal.test.tsx`
- Modify: `src/character/creation/InteractiveCharacterSheet.tsx`
- Modify: `src/character/creation/InteractiveCharacterSheet.test.tsx`
- Modify: `src/character/sheet/components/DnDAppearance.tsx`
- Modify: `src/character/sheet/components/DnDAppearance.test.tsx`

**Interfaces:**
- `CharacterCustomizationControls` consumes full `Appearance`, not hair alone, and emits a complete Appearance patch.
- `AppearanceAccordionSection` is controlled by one `openSection: 'hair' | 'facialHair' | 'gear'` owner.
- `OutfitColorControls` emits independently optional `OutfitCustomization` fields.

- [ ] **Step 1: Write failing accordion behavior tests**

Require exactly one expanded region, Hair initially open, summary swatches in collapsed headers, and retained edits across section switches:

```ts
expect(screen.getByRole('button', { name: /Hair/ })).toHaveAttribute('aria-expanded', 'true');
await user.click(screen.getByRole('button', { name: /Gear Colors/ }));
expect(screen.getByRole('button', { name: /Hair/ })).toHaveAttribute('aria-expanded', 'false');
expect(screen.getByRole('button', { name: /Gear Colors/ })).toHaveAttribute('aria-expanded', 'true');
```

Change hair color in the Hair section, open Facial Hair, and assert its color input shows the same value. Change it there, reopen Hair, and assert the first control changed too. Use unique DOM IDs per section while binding both controls to one `hair.colorSrgb`.

- [ ] **Step 2: Write failing Gear controls tests**

Test provider defaults, primary-only, secondary-only, black, both, Reset, and class switch retention:

```ts
fireEvent.change(primaryInput, { target: { value: '#000000' } });
expect(onChange).toHaveBeenLastCalledWith(expect.objectContaining({
  primaryColorSrgb: 0,
}));
await user.click(screen.getByRole('button', { name: /Reset gear colors/ }));
expect(onChange).toHaveBeenLastCalledWith(undefined);
```

- [ ] **Step 3: Run component tests and confirm red**

```bash
npx vitest run \
  src/character/creation/components/AppearanceAccordionSection.test.tsx \
  src/character/creation/components/OutfitColorControls.test.tsx \
  src/character/creation/components/CharacterCustomizationControls.test.tsx \
  src/character/creation/AppearanceSelectionModal.test.tsx
```

Expected: missing accordion/gear controls.

- [ ] **Step 4: Implement one complete Appearance editor state**

`editableAppearance` deep-copies both hair and outfit with generated schemas. Every section update patches one field while preserving its sibling. Remove any `setAppearance(create(AppearanceSchema, { hair }))` call that would drop outfit.

The controlled accordion button uses `aria-expanded`, `aria-controls`, a stable region ID, and a summary area that remains visible while collapsed. Hair and Facial Hair reuse `SharedHairMaterialControls` with different ID prefixes but one value/callback. Keep roughness available without adding a second wire value.

Gear displays provider default colors when fields are absent, writes exact RGB24 on user change, and Reset clears both optionals. Rename modal copy from “Customize Hair” to “Customize Appearance” and describe all three sections.

- [ ] **Step 5: Wire live preview and readonly summaries**

Resolve outfit presentation from the current class and full Appearance; pass it to `ClassCharacterModel` beside hair accessories. A class prop change selects a new mask but does not alter editor state.

Interactive draft summary and finalized `DnDAppearance` show provider-default-aware primary/secondary swatches and labels. No finalized edit button is introduced.

- [ ] **Step 6: Run focused UI tests**

```bash
npx vitest run \
  src/character/creation/components/AppearanceAccordionSection.test.tsx \
  src/character/creation/components/OutfitColorControls.test.tsx \
  src/character/creation/components/CharacterCustomizationControls.test.tsx \
  src/character/creation/AppearanceSelectionModal.test.tsx \
  src/character/creation/InteractiveCharacterSheet.test.tsx \
  src/character/sheet/components/DnDAppearance.test.tsx
```

Expected: pass with one shared hair value, independent outfit optionals, response-authoritative Apply, no-RPC Cancel, and readonly finalized output.

- [ ] **Step 7: Commit the creation experience**

```bash
git add src/character/creation src/character/sheet/components/DnDAppearance.tsx \
  src/character/sheet/components/DnDAppearance.test.tsx
git commit -m "feat: edit hair and gear colors in one appearance flow (#912)"
```

---

### Task 13: Carry outfit customization through owner and peer game rendering

**Files:**
- Modify: `src/components/hex-grid/HexEntity.tsx`
- Modify: `src/components/hex-grid/HexEntity.test.tsx`
- Modify: `src/components/session/SessionCanvas.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Create: `scripts/classOutfitColorEvidence.test.ts`
- Create: `scripts/captureClassOutfitColorEvidence.mjs`
- Create: `docs/evidence/912-class-outfit-colors/README.md`
- Create: `docs/evidence/912-class-outfit-colors/receipt.json`
- Generated evidence: `docs/evidence/912-class-outfit-colors/*.png`

**Interfaces:**
- Owner input: `Character.appearance`.
- Peer input: `PublicMemberInfo.customization`.
- Both normalize to one local `CharacterCustomizationContainer` containing `hair` and `outfit`.
- `HexEntity` passes resolved hair accessories and outfit presentation to one `ClassCharacterModel`.

- [ ] **Step 1: Write failing owner/peer projection tests**

Replace the hair-only prop seam with a complete customization seam. Assert owner and peer values are not merged or defaulted differently:

```ts
expect(localEntity).toMatchObject({
  customization: localCharacter.appearance,
});
expect(peerEntity).toMatchObject({
  customization: rosterMember.customization,
});
```

Cover absent Appearance/customization, explicit black, unsupported/downed body fallback, monster/obstacle exclusion, remembered state, and two peers with disjoint colors.

- [ ] **Step 2: Run session/renderer tests and confirm red**

```bash
npx vitest run \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/session/SessionEncounterView.test.tsx
```

Expected: missing outfit propagation.

- [ ] **Step 3: Implement one owner/peer normalization path**

Define the local structural container once; do not introduce race/class lookup in SessionCanvas. `HexEntity` resolves treatment only for the active primary customization body. Complete/downed/generic fallbacks retain untouched texture and existing hair/body fallback behavior. Monsters and obstacles never receive player outfit treatment.

- [ ] **Step 4: Add deterministic evidence contract tests**

Require receipt fields for exact web/proto/API/provider heads, both provider manifest hashes, four class/race/body/mask rows, requested and persisted RGB24 values, owner/peer observations, material/mesh/Skeleton/mixer/map UUID observations before/after treatment, HTTP/application failures, movement, and main/off-hand witnesses. Reject missing screenshots, unsafe paths, stale hashes, nonzero failure counts, mismatched owner/peer values, or reused uniform identities between characters.

- [ ] **Step 5: Run the one final web CI**

With the exact provider mirror present:

```bash
RPG_REQUIRE_SYNCED_CUSTOMIZATION_ASSETS=1 npm run ci-check
git diff --check
git status --short
```

Expected: formatting, lint, typecheck, build, bundle guards, and full tests pass; ignored asset bytes remain untracked.

- [ ] **Step 6: Capture normal-game evidence and stop for Kirk's verdict**

Run the intended local dev path. Create/finalize four characters representing Barbarian/Fighter/Monk/Rogue with visibly distinct primary/secondary values, preserving varied hair. Start one normal multiplayer session, move characters, and equip the normal weapon witnesses. Capture creation accordion/preview and session close/orbit frames without debug-only renderer substitution.

Verify:

- all body/mask/accessory requests return 200;
- Apply responses contain exact requested outfit values;
- owner Appearance and peer Customization agree;
- color changes preserved material and animation identities;
- each character has disjoint uniform objects;
- no HTTP/application failures occurred.

Have Kirk inspect the real creation and session routes. Record his exact words in the evidence README and issue #912. Regenerate `receipt.json`, then run:

```bash
npx vitest run scripts/classOutfitColorEvidence.test.ts
```

- [ ] **Step 7: Commit evidence, review once, and merge**

```bash
git add src scripts/classOutfitColorEvidence.test.ts \
  scripts/captureClassOutfitColorEvidence.mjs \
  docs/evidence/912-class-outfit-colors package.json package-lock.json
git commit -m "test: prove class outfit colors in normal play (#912)"
npm run ci-check
git push -u origin feat/912-class-outfit-colors
```

Open one PR against `dev`, complete one independent whole-PR review, resolve every finding, regenerate evidence if runtime behavior changed, rerun the final CI, and wait for Kirk's squash merge.

---

### Task 14: Reconcile the canonical Journey and close it

**Files:**
- Modify: `ideas/characters/customization/class-outfit-colors-design.md`
- Modify: `ideas/characters/customization/class-outfit-colors-plan.md`

**Interfaces:**
- Consumes exact provider, proto, D&D toolkit, Session toolkit, API, and web merge commits/tags plus Kirk's verdicts.
- Produces the durable observed-outcome section and closed Project 19 state.

- [ ] **Step 1: Verify every implementation issue and PR**

Read back all six slices:

```bash
for issue in \
  KirkDiggler/rpg-game-assets#119 \
  KirkDiggler/rpg-api-protos#278 \
  KirkDiggler/rpg-toolkit#1450 \
  KirkDiggler/rpg-toolkit#1451 \
  KirkDiggler/rpg-api#897 \
  KirkDiggler/rpg-dnd5e-web#912; do
  repo=${issue%#*}; number=${issue#*#}
  gh issue view "$number" --repo "$repo" --json state,projectItems,url
 done
```

Require each implementation issue closed, each PR merged, all Project 19 slice statuses Done, and exact published provider/proto/toolkit pins matching API/web dependency files.

- [ ] **Step 2: Add observed results to the design**

Record exact merge commits, generated proto commit/tag, toolkit module tags, provider manifest/mask hashes, unchanged Dwarf/tree receipts, test totals, review findings/dispositions, API-deletion evidence, normal-game receipt hash, and Kirk's exact Blender/game verdicts. Mark plan checkboxes from actual evidence only.

- [ ] **Step 3: Verify and commit canonical closure**

```bash
git diff --check
python3 - <<'PY'
from pathlib import Path
needles = ("TO" + "DO", "T" + "BD", "FIX" + "ME", "place" + "holder")
for path in map(Path, (
    "ideas/characters/customization/class-outfit-colors-design.md",
    "ideas/characters/customization/class-outfit-colors-plan.md",
)):
    text = path.read_text()
    found = [needle for needle in needles if needle in text]
    if found:
        raise SystemExit(f"{path}: unfinished markers: {found}")
PY
git add ideas/characters/customization/class-outfit-colors-design.md \
  ideas/characters/customization/class-outfit-colors-plan.md
git commit -m "docs: record class outfit color outcome (#362)"
git push
```

- [ ] **Step 4: Review and merge the design PR**

Ensure `rpg-project#364` head matches the reviewed commit, all links and Project fields agree, and all findings are dispositioned. Merge the canonical PR only after the normal-game verdict. Confirm `Closes #362` closed the Journey, set the Journey and every slice to Done, and read Project 19 back after each write.
