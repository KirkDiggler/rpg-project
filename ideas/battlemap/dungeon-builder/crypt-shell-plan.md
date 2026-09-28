# Archetype-Driven Crypt Shell Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `archetype: crypt` select one continuous floor and one finished, truly two-sided modular wall family in the shared builder/playable renderer.

**Architecture:** A private provider slice auditions and exports one approved crypt shell family, then generates and validates `harness/models/synty/env/shell-profiles.json`. A web slice strictly loads that ignored runtime manifest, resolves only uniform known archetypes, and passes one atomic profile into shared floor/wall leaves; every failure renders the complete legacy shell.

**Tech Stack:** Blender Python, Python 3 stdlib/unittest, glTF/GLB, SHA-256, React 19, TypeScript, React Three Fiber, drei, Three.js, Vitest, Testing Library, GitHub Project 19.

**Spec:** `ideas/battlemap/dungeon-builder/crypt-shell.md`

## Global Constraints

- Version one supports one effective archetype for the whole dungeon; no mixed-region shell rendering.
- `archetype: crypt` selects the complete profile automatically; add no floor, wall, door, or variant picker.
- Do not change YAML, protos, toolkit, API, topology, movement, sight, door state, wall-height fields, or wall-run derivation.
- Floor meshes remain per-cell at `DUNGEON_SURFACE_Y = 0.2`; profile UVs use one absolute world frame and permanent hex boundaries stay invisible.
- Keep the floor’s unlit `MeshBasicMaterial` with `toneMapped={false}`; authored lighting is not part of this wave.
- The approved wall body must contain two finished textured faces with correct geometry/normals; do not use `THREE.DoubleSide` or runtime mirrored duplicates as the shipped fix.
- Crypt wall body, base, cap, and surround load and render atomically; any unavailable/invalid resource selects the complete legacy floor+wall pair.
- `rpg-game-assets` owns Blender sources, artifacts, manifest generation, hashes, bounds, provenance, and private evidence.
- Never commit licensed GLBs, synced runtime files, or private provider manifests to public `rpg-dnd5e-web` or `rpg-project`.
- Builder preview and playable session must mount the same `DungeonShell` component, resolver, floor leaf, wall leaf, lights, profile paths, and wall runs.
- Preserve standard/raised/cutaway wall heights and existing open/closed/locked door behavior.
- Kirk alone approves candidate/final visuals and performs merges.
- `rpg-dnd5e-web#791` closes only after the two-sided web path lands; `rpg-dnd5e-web#823` remains separate.
- Journey `rpg-project#169` remains In Progress for authored lighting and later shell profiles.
- Before every GitHub post, use the Assets charter signature: `— assets agent, on behalf of KirkDiggler`.

## File Structure

### `rpg-game-assets`

- Create `scripts/crypt_shell_candidates.py` — exact private candidate declarations and selection-schema validation.
- Create `scripts/render_crypt_shell_candidates.py` — Blender candidate contact-sheet renderer.
- Create `scripts/test_crypt_shell_candidates.py` — candidate IDs, source containment, selection, and deterministic-order tests.
- Create `library/assemblies/crypt-shell-v1.json` — exact Kirk-approved floor choice, wall source family, derivative artifact paths, axes, and evidence IDs.
- Create `library/assemblies/crypt-shell-v1.blend` — private normalized body/base/cap/surround source.
- Create `scripts/export_crypt_shell.py` — deterministic four-artifact exporter from the reviewed Blend source.
- Create `scripts/test_export_crypt_shell_blender.py` — explicit Blender export/reimport invariants.
- Create `scripts/build_shell_profiles.py` — strict source-record/artifact validator and deterministic runtime-manifest generator.
- Create `scripts/test_build_shell_profiles.py` — schema, path, hash, bounds, two-face, determinism, and `--check` tests.
- Create `scripts/render_crypt_shell_evidence.py` — front/back/run/corner/junction/door/height evidence renderer.
- Create generated private files under `harness/models/synty/env/`: `Crypt_Wall_Body_01.glb`, `Crypt_Wall_Base_01.glb`, `Crypt_Wall_Cap_01.glb`, `Crypt_Wall_Door_Surround_01.glb`, and `shell-profiles.json`.
- Modify `scripts/verify_web_asset_stage.py` and create `scripts/test_verify_web_asset_stage.py` — require and regenerate/validate the shell manifest during staging.
- Regenerate `harness/models/synty/mesh-stats.json` and `harness/catalogs/synty-complete-inventory.json`.
- Create `evidence/284-crypt-shell/` — candidate/final private renders, approval ledger, and verification report.
- Modify `README.md` — document complete-library versus runtime-profile ownership and generation commands.

### `rpg-dnd5e-web`

- Create `src/rendering/dungeonShellManifest.ts` and test — strict consumer-safe manifest parser/types.
- Create `src/rendering/dungeonShellProvider.ts` and test — cached runtime fetch/snapshot with controlled failures.
- Create `src/rendering/dungeonShellProfile.ts` and test — uniform-archetype resolver and named legacy reasons.
- Create `src/components/session/useDungeonShellCatalog.ts` and test — React bridge over the cached provider.
- Create `src/components/session/DungeonShell.tsx` and test — atomic profile resources plus shared floor/wall leaves and complete legacy fallback.
- Create `src/components/hex-grid/dungeonFloorUv.ts` and test — absolute world-space UV function.
- Create `src/components/hex-grid/dungeonShellWallHelpers.ts` and test — body/base/cap/door transforms from measured profile bounds.
- Modify `src/components/session/atlasToScene3D.ts` and test — carry the atlas archetype words into `Scene3D`.
- Modify `src/components/hex-grid/SyntyHexFloor.tsx` and test — optional profile texture and world UVs; exact legacy behavior when absent.
- Modify `src/components/hex-grid/GlbInstance.tsx` and test — additive world-Y placement for cap/base/body registration.
- Modify `src/components/hex-grid/WallRunMesh.tsx`, `src/components/hex-grid/wallRunMeshHelpers.ts`, and tests — profile body/base/cap path while preserving legacy.
- Modify `src/components/session/AtlasWalls.tsx` and test — profile surround and existing leaf/state behavior.
- Modify `src/author/preview3d/DungeonPreview3D.tsx`, `src/components/session/SessionCanvas.tsx`, and tests — replace separate floor/wall calls with `DungeonShell`.
- Modify `src/author/fixtures/cryptPropShowcase.ts` and related fixture tests only as needed to assert uniform `crypt` regions.
- Create `docs/evidence/284-crypt-shell/README.md` plus public-safe before/final screenshots; never add synced assets.

---

### Task 1: Establish the Provider Slice and Candidate Visual Gate

**Files:**
- Create: `rpg-game-assets/scripts/crypt_shell_candidates.py`
- Create: `rpg-game-assets/scripts/render_crypt_shell_candidates.py`
- Create: `rpg-game-assets/scripts/test_crypt_shell_candidates.py`
- Create: `rpg-game-assets/evidence/284-crypt-shell/README.md`
- Create after Kirk’s verdict: `rpg-game-assets/evidence/284-crypt-shell/candidate-verdict.json`

**Interfaces:**
- Consumes: private converted sources under `library/polygon-dungeon/` and promoted floor textures under `harness/models/synty/textures/`.
- Produces: `FLOOR_CANDIDATES`, `WALL_CANDIDATES`, `floor_candidate(id)`, `wall_candidate(id)`, `load_candidate_verdict(path)`, candidate sheets, and a validated verdict consumed by Task 2.

- [ ] **Step 1: Create and board the provider issue, then isolate the branch**

Use the authenticated login and Assets signature. Board the new `rpg-game-assets` issue on Project 19 as `Assets · The Dungeon · Build · Ready · Four-player Level-3 Dungeon`, link `rpg-project#284/#169`, fetch first, and create `asset/${ASSET_ISSUE}-crypt-shell` from `origin/main` in an isolated worktree.

```bash
LOGIN=$(gh api user --jq .login)
test "$LOGIN" = KirkDiggler
ASSET_URL=$(gh issue create -R KirkDiggler/rpg-game-assets \
  --title 'Build the approved crypt floor and two-sided wall shell profile' \
  --body 'Provider slice for KirkDiggler/rpg-project#284 under journey #169. Audition, author, validate, and promote one crypt floor/wall profile; no consumer or mechanics changes.\n\n— assets agent, on behalf of KirkDiggler')
ASSET_ISSUE=${ASSET_URL##*/}
: "${RPG_GAME_ASSETS_DIR:?set RPG_GAME_ASSETS_DIR to the assets checkout}"
git -C "$RPG_GAME_ASSETS_DIR" fetch origin
# Use superpowers:using-git-worktrees; branch from origin/main, never local main.
```

- [ ] **Step 2: Write the failing candidate-contract tests**

Pin exact candidate IDs, real source paths, repeat scales, and verdict strictness:

```python
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from crypt_shell_candidates import (  # noqa: E402
    FLOOR_CANDIDATES,
    WALL_CANDIDATES,
    CandidateError,
    load_candidate_verdict,
)


class CandidateContractTest(unittest.TestCase):
    def test_floor_candidates_cover_three_families_at_three_world_scales(self):
        self.assertEqual([c.id for c in FLOOR_CANDIDATES], [
            "floor-tiles-01-u2", "floor-tiles-01-u4", "floor-tiles-01-u6",
            "floor-09-01-u2", "floor-09-01-u4", "floor-09-01-u6",
            "floor-10-01-u2", "floor-10-01-u4", "floor-10-01-u6",
        ])

    def test_wall_candidates_are_explicit_two_face_treatments(self):
        self.assertEqual([c.id for c in WALL_CANDIDATES], [
            "wall-double-01-worked",
            "wall-texture-01-mirrored-worked",
            "wall-texture-02-mirrored-worked",
        ])
        self.assertTrue(all(c.candidateTwoFace for c in WALL_CANDIDATES))

    def test_verdict_refuses_unknown_keys_and_unknown_ids(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "candidate-verdict.json"
            path.write_text('{"schemaVersion":1,"floor":"missing","wall":"missing","extra":true}')
            with self.assertRaises(CandidateError):
                load_candidate_verdict(path)
```

- [ ] **Step 3: Run the tests and verify RED**

```bash
python3 -m unittest scripts.test_crypt_shell_candidates -v
```

Expected: import failure because `crypt_shell_candidates.py` does not exist.

- [ ] **Step 4: Implement the exact candidate declarations and strict verdict loader**

Use immutable dataclasses. The three wall treatments compare the pack’s explicit double-sided source against two mirrored audition-only treatments; a mirrored audition may be selected as source for the provider-authored derivative, but runtime mirroring is forbidden.

```python
@dataclass(frozen=True)
class FloorCandidate:
    id: str
    texture: str
    worldUnitsPerRepeat: float

@dataclass(frozen=True)
class WallCandidate:
    id: str
    body: str
    base: str
    cap: str
    doorSurround: str
    candidateTwoFace: bool
    mirrorBackForAudition: bool

FLOOR_CANDIDATES = tuple(
    FloorCandidate(f"{family}-u{units}", texture, float(units))
    for family, texture in (
        ("floor-tiles-01", "harness/models/synty/textures/Dungeons_Texture_FloorTiles_01.png"),
        ("floor-09-01", "harness/models/synty/textures/Dungeons_Texture_FloorTile_09_01.png"),
        ("floor-10-01", "harness/models/synty/textures/Dungeons_Texture_FloorTile_10_01.png"),
    )
    for units in (2, 4, 6)
)
```

Define the three wall candidates with these exact source families:

- `SM_Env_Wall_01_DoubleSided.glb`;
- `SM_Env_Wall_01_Texture.glb` auditioned with a mirrored back;
- `SM_Env_Wall_02_Texture.glb` auditioned with a mirrored back;
- `SM_Env_Wall_Trim_Floor_01.glb`, `SM_Env_Wall_Trim_Top_02.glb`, and `SM_Env_Wall_DoorFrame_01.glb` as the common worked trim/surround baseline.

`floor_candidate` and `wall_candidate` return the unique immutable declaration for a known ID and raise `CandidateError` otherwise. `load_candidate_verdict` accepts exactly `schemaVersion`, `floor`, `wall`, and `verdict`; requires version 1, known IDs, and a non-empty verbatim Kirk verdict; it rejects booleans as numbers and every unknown field.

- [ ] **Step 5: Run candidate-contract tests GREEN**

```bash
python3 -m unittest scripts.test_crypt_shell_candidates -v
```

Expected: all tests pass.

- [ ] **Step 6: Implement and run the Blender candidate renderer**

The renderer must produce two sheets from one command:

```bash
blender --background --python scripts/render_crypt_shell_candidates.py -- \
  --out evidence/284-crypt-shell/candidates \
  --width 1600 --height 900
```

`floor-candidates.png` renders every floor ID as a 12×8-world-unit plane from the tactical camera with a 1.8m reference figure and visible candidate label. `wall-candidates.png` renders each wall ID as a long run, inside/outside corner, T-junction, and doorway from front and back. Use the same camera/lights for every cell and do not tint individual candidates.

- [ ] **Step 7: Run the human candidate gate and record the exact verdict**

Stop and give Kirk both sheets. After he chooses, write `candidate-verdict.json` against this closed shape:

```ts
type CandidateVerdictV1 = {
  schemaVersion: 1;
  floor: FloorCandidateId;
  wall: WallCandidateId;
  verdict: string;
};
```

Use only the IDs Kirk actually selects and his verbatim non-empty approval text. Validate the written file with `load_candidate_verdict`. Do not continue to authoring without approval.

- [ ] **Step 8: Commit the candidate gate**

```bash
git add scripts/crypt_shell_candidates.py scripts/render_crypt_shell_candidates.py \
  scripts/test_crypt_shell_candidates.py evidence/284-crypt-shell
git commit -m "asset: audition crypt shell floor and wall families (#${ASSET_ISSUE})"
```

### Task 2: Author and Deterministically Export the Selected Wall Family

**Files:**
- Create: `rpg-game-assets/library/assemblies/crypt-shell-v1.json`
- Create: `rpg-game-assets/library/assemblies/crypt-shell-v1.blend`
- Create: `rpg-game-assets/scripts/export_crypt_shell.py`
- Create: `rpg-game-assets/scripts/test_export_crypt_shell_blender.py`
- Create: `rpg-game-assets/harness/models/synty/env/Crypt_Wall_Body_01.glb`
- Create: `rpg-game-assets/harness/models/synty/env/Crypt_Wall_Base_01.glb`
- Create: `rpg-game-assets/harness/models/synty/env/Crypt_Wall_Cap_01.glb`
- Create: `rpg-game-assets/harness/models/synty/env/Crypt_Wall_Door_Surround_01.glb`

**Interfaces:**
- Consumes: validated `candidate-verdict.json` and the selected candidate declarations from Task 1.
- Produces: four normalized GLBs and `crypt-shell-v1.json`, consumed by Task 3.

- [ ] **Step 1: Write the source-record and Blender RED tests**

The source record must bind the approved IDs and exact component sources. Blender tests reimport every exported GLB and assert:

```python
EXPECTED = {
    "body": "Crypt_Wall_Body_01.glb",
    "base": "Crypt_Wall_Base_01.glb",
    "cap": "Crypt_Wall_Cap_01.glb",
    "doorSurround": "Crypt_Wall_Door_Surround_01.glb",
}


def assert_normalized(obj):
    assert tuple(round(v, 7) for v in obj.location) == (0.0, 0.0, 0.0)
    assert tuple(round(v, 7) for v in obj.rotation_euler) == (0.0, 0.0, 0.0)
    floor_y = min(world_corner[1] for world_corner in object_bbox(obj))
    assert abs(floor_y) <= 1e-6


def test_body_has_real_positive_and_negative_z_faces():
    normals = triangle_normals(imported("Crypt_Wall_Body_01.glb"))
    assert sum(n.z > 0.5 for n in normals) > 0
    assert sum(n.z < -0.5 for n in normals) > 0
```

Also assert no animation, no camera/light objects, finite transforms, embedded images, exact node names `CryptWallBody`, `CryptWallBase`, `CryptWallCap`, and `CryptWallDoorSurround`, +X span, +Y up, and deterministic export hashes across two runs.

- [ ] **Step 2: Run the explicit Blender test RED**

```bash
blender --background --python scripts/test_export_crypt_shell_blender.py
```

Expected: failure because the source Blend/exporter/artifacts do not exist.

- [ ] **Step 3: Write the exact source record from the accepted verdict**

Generate `library/assemblies/crypt-shell-v1.json` directly from the validated verdict so no candidate value is copied by hand:

```python
verdict = load_candidate_verdict(Path("evidence/284-crypt-shell/candidate-verdict.json"))
floor = floor_candidate(verdict.floor)
wall = wall_candidate(verdict.wall)
record = {
    "schemaVersion": 1,
    "profile": "crypt",
    "candidateVerdict": "evidence/284-crypt-shell/candidate-verdict.json",
    "sourceBlend": "library/assemblies/crypt-shell-v1.blend",
    "floor": {
        "candidateId": floor.id,
        "texture": floor.texture,
        "worldUnitsPerRepeat": floor.worldUnitsPerRepeat,
    },
    "wall": {
        "candidateId": wall.id,
        "sources": {
            "body": wall.body,
            "base": wall.base,
            "cap": wall.cap,
            "doorSurround": wall.doorSurround,
        },
    },
    "artifacts": {
        "body": "harness/models/synty/env/Crypt_Wall_Body_01.glb",
        "base": "harness/models/synty/env/Crypt_Wall_Base_01.glb",
        "cap": "harness/models/synty/env/Crypt_Wall_Cap_01.glb",
        "doorSurround": "harness/models/synty/env/Crypt_Wall_Door_Surround_01.glb",
    },
    "axes": {"span": "+X", "up": "+Y", "faces": ["+Z", "-Z"]},
}
```

Serialize with sorted deterministic indentation and a final newline. The record now carries both approved IDs and their exact resolved source paths.

- [ ] **Step 4: Author the private Blend source**

Import only the selected component GLBs. Normalize each exported object to bottom-center registration, +X span, +Y up, and finished ±Z faces. If the selected candidate used an audition mirror, duplicate the selected finished face in Blender, mirror it onto the opposite side, recalculate outward normals, preserve UV/material assignments, apply transforms, and save that result in the private Blend. Do not mirror at runtime.

Keep one collection per artifact and no unrelated source objects. Save `library/assemblies/crypt-shell-v1.blend`; the exporter never edits or saves it.

- [ ] **Step 5: Implement deterministic export**

`export_crypt_shell.py` loads the source record and Blend, validates the verdict, exports each named collection with:

```python
bpy.ops.export_scene.gltf(
    filepath=str(destination),
    export_format="GLB",
    use_selection=True,
    export_yup=True,
    export_apply=False,
    export_animations=False,
    export_cameras=False,
    export_lights=False,
)
```

Export in sorted artifact-key order, delete no source, reject unexpected objects, and fail if any destination resolves outside `harness/models/synty/env/`.

- [ ] **Step 6: Run exporter and Blender tests GREEN**

```bash
blender --background --python scripts/export_crypt_shell.py
blender --background --python scripts/test_export_crypt_shell_blender.py
```

Expected: four artifacts, deterministic second-run hashes, all geometric invariants pass.

- [ ] **Step 7: Commit authored sources and artifacts**

```bash
git add library/assemblies/crypt-shell-v1.json \
  library/assemblies/crypt-shell-v1.blend scripts/export_crypt_shell.py \
  scripts/test_export_crypt_shell_blender.py harness/models/synty/env/Crypt_Wall_*.glb
git commit -m "asset: author the two-sided crypt wall family (#${ASSET_ISSUE})"
```

### Task 3: Generate and Validate the Runtime Shell Profile

**Files:**
- Create: `rpg-game-assets/scripts/build_shell_profiles.py`
- Create: `rpg-game-assets/scripts/test_build_shell_profiles.py`
- Create: `rpg-game-assets/harness/models/synty/env/shell-profiles.json`
- Modify: `rpg-game-assets/scripts/build_mesh_stats.py`
- Modify generated: `rpg-game-assets/harness/models/synty/mesh-stats.json`

**Interfaces:**
- Consumes: `crypt-shell-v1.json`, Task 1 candidate declarations/verdict, four Task 2 GLBs, selected existing floor PNG.
- Produces: exact `shell-profiles.json` matching `ShellProfilesV1` in the spec and a reusable `build_profile(repo_root) -> dict` function for Task 4 staging.

- [ ] **Step 1: Write strict generator tests RED**

Use temp roots and small fixture GLBs/PNGs to pin:

```python
EXPECTED_TOP_KEYS = {"schemaVersion", "profiles"}
EXPECTED_CRYPT_KEYS = {"floor", "wall"}
EXPECTED_WALL_KEYS = {"body", "base", "cap", "doorSurround"}


class ShellProfileGeneratorTest(unittest.TestCase):
    def test_generated_profile_has_exact_closed_shape(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            write_valid_shell_fixture(root)
            profile = build_profile(root)
            self.assertEqual(set(profile), EXPECTED_TOP_KEYS)
            self.assertEqual(profile["schemaVersion"], 1)
            self.assertEqual(set(profile["profiles"]), {"crypt"})
            crypt = profile["profiles"]["crypt"]
            self.assertEqual(set(crypt), EXPECTED_CRYPT_KEYS)
            self.assertEqual(set(crypt["wall"]), EXPECTED_WALL_KEYS)
```

`write_valid_shell_fixture` creates the complete minimum tree used by every case: closed source record and verdict, a real 1×1 RGBA PNG, and four minimal indexed GLBs with POSITION/NORMAL/TEXCOORD_0, one textured material, finite bottom-grounded bounds, and opposing ±Z face triangles. Mutation helpers change one fact per rejection test. Add tests for unknown record/verdict fields, traversal/absolute paths, missing files, uppercase or wrong-length hashes, stale candidate IDs, non-finite/degenerate bounds, nonzero floor contact, wrong axes, animation, absent image/material/UV, only-one-face normals, unfinished opposite-face material, nondeterministic key order, and stale `--check` bytes.

- [ ] **Step 2: Run generator tests RED**

```bash
python3 -m unittest scripts.test_build_shell_profiles -v
```

Expected: import failure.

- [ ] **Step 3: Implement strict record/artifact inspection and deterministic bytes**

Implement these stable entry points:

```python
class ShellProfileError(ValueError):
    pass


def build_profile(repo_root: Path) -> dict[str, object]:
    record = load_closed_source_record(repo_root / SOURCE_RECORD)
    verdict = load_candidate_verdict(repo_root / record["candidateVerdict"])
    floor = validate_floor(record, verdict, repo_root)
    wall = {
        role: validate_wall_artifact(role, record, verdict, repo_root)
        for role in ("body", "base", "cap", "doorSurround")
    }
    return {"schemaVersion": 1, "profiles": {"crypt": {"floor": floor, "wall": wall}}}


def profile_bytes(document: dict[str, object]) -> bytes:
    return (json.dumps(document, indent=2, ensure_ascii=False) + "\n").encode()
```

Read GLB JSON/BIN chunks with stdlib, compute transformed POSITION bounds, inspect indexed triangles and NORMAL/TEXCOORD_0 accessors, and prove meaningful outward areas on both ±Z faces. Require both face sets to reference textured materials. Fill every path relative to `harness/models/synty/`, every exact lowercase SHA-256, measured `bounds`, fixed axes, `twoSided: true`, and the selected candidate’s finite positive `worldUnitsPerRepeat`.

The CLI supports only:

```bash
python3 scripts/build_shell_profiles.py
python3 scripts/build_shell_profiles.py --check
```

- [ ] **Step 4: Run unit tests GREEN and generate the real manifest**

```bash
python3 -m unittest scripts.test_build_shell_profiles -v
python3 scripts/build_shell_profiles.py
python3 scripts/build_shell_profiles.py --check
```

Expected: tests pass; check reports byte identity.

- [ ] **Step 5: Regenerate mesh statistics and assert the new env artifacts are measured**

```bash
python3 scripts/build_mesh_stats.py
python3 - <<'PY'
import json
p=json.load(open('harness/models/synty/mesh-stats.json'))
files={entry['file'] for entry in p['assets']}
for name in ('Body','Base','Cap','Door_Surround'):
    assert f'harness/models/synty/env/Crypt_Wall_{name}_01.glb' in files
PY
```

Record warnings explicitly; do not make unrelated existing warnings strict.

- [ ] **Step 6: Commit the generated provider contract**

```bash
git add scripts/build_shell_profiles.py scripts/test_build_shell_profiles.py \
  harness/models/synty/env/shell-profiles.json scripts/build_mesh_stats.py \
  harness/models/synty/mesh-stats.json
git commit -m "asset: publish the crypt shell runtime profile (#${ASSET_ISSUE})"
```

### Task 4: Bind Staging, Produce Final Provider Evidence, and Land Provider First

**Files:**
- Modify: `rpg-game-assets/scripts/verify_web_asset_stage.py`
- Create: `rpg-game-assets/scripts/test_verify_web_asset_stage.py`
- Modify generated: `rpg-game-assets/harness/catalogs/synty-complete-inventory.json`
- Create: `rpg-game-assets/scripts/render_crypt_shell_evidence.py`
- Modify: `rpg-game-assets/evidence/284-crypt-shell/README.md`
- Create: final PNGs and `verification.json` under `evidence/284-crypt-shell/final/`
- Modify: `rpg-game-assets/README.md`

**Interfaces:**
- Consumes: `build_profile()`/`profile_bytes()` from Task 3 and complete provider tree.
- Produces: independently staged exact provider bundle, final approved provider hashes, merged provider PR SHA for Task 5.

- [ ] **Step 1: Write staging RED tests**

Add cases proving staging refuses a missing/stale shell manifest, one changed shell GLB byte, one changed floor texture byte, and a candidate manifest that differs from `build_profile()`:

```python
def test_stage_requires_regenerated_shell_profile(self):
    fixture = self.fixture_tree()
    (fixture.source_tree / "env/shell-profiles.json").write_text("{}\n")
    with self.assertRaisesRegex(CatalogError, "shell profile"):
        verify_candidate(
            source_tree=fixture.source_tree,
            catalog_path=fixture.catalog_path,
            complete_inventory_path=fixture.complete_inventory_path,
            declarations_path=fixture.declarations_path,
            repo_root=fixture.repo_root,
        )
```

The fixture helper copies the existing safe-catalog fixture pattern, adds a valid generated shell source record/artifacts, builds its independent inventory, and returns those five exact paths as an immutable dataclass.

- [ ] **Step 2: Run the scoped staging test RED**

```bash
python3 -m unittest scripts.test_verify_web_asset_stage -v
```

Expected: new test fails because staging ignores the shell manifest.

- [ ] **Step 3: Extend staging without duplicating validation**

Import `build_profile` and `profile_bytes`; require `env/shell-profiles.json` in complete inventory; byte-compare it with regeneration before copying and after staging. Add its path/hash/schema/profile keys to the deterministic report. Do not reimplement shell validation in `verify_web_asset_stage.py`.

- [ ] **Step 4: Regenerate inventory and run provider gates**

```bash
python3 scripts/build_synty_complete_inventory.py
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/verify_web_asset_stage.py --verify-only
python3 -m unittest scripts.test_verify_web_asset_stage -v
python3 -m unittest discover -s scripts -p 'test_*.py'
blender --background --python scripts/test_export_crypt_shell_blender.py
```

Expected: all pass; unrelated warn-only mesh messages remain documented, not hidden.

- [ ] **Step 5: Render final provider evidence**

```bash
blender --background --python scripts/render_crypt_shell_evidence.py -- \
  --record library/assemblies/crypt-shell-v1.json \
  --profile harness/models/synty/env/shell-profiles.json \
  --out evidence/284-crypt-shell/final --width 1600 --height 900
```

Produce `floor-tactical.png`, `wall-front.png`, `wall-back.png`, `straight-corner-junction.png`, `door-standard.png`, `door-raised.png`, and deterministic `verification.json`. README records candidate verdict, exact paths/hashes/bounds, ±Z face counts/materials, mesh warnings, and commands.

- [ ] **Step 6: Run Kirk’s final provider visual gate**

Stop and present every final frame. If rejected, iterate on the same provider branch and rerun Tasks 2–4; do not open the web slice. Record the verbatim accepted verdict in the evidence README.

- [ ] **Step 7: Commit, push, open one provider PR, and request one Copilot review**

```bash
git add scripts/verify_web_asset_stage.py scripts/test_verify_web_asset_stage.py \
  scripts/render_crypt_shell_evidence.py harness/catalogs/synty-complete-inventory.json \
  evidence/284-crypt-shell README.md
git commit -m "asset: verify the complete crypt shell provider stage (#${ASSET_ISSUE})"
```

Push normally, open the PR against `main`, include all hashes/verdicts and the Assets signature, fix valid Copilot findings with scoped tests, rerun all provider gates, and ask Kirk to merge. Agents do not merge.

### Task 5: Create the Web Slice and Pin the Exact Merged Provider Tree

**Files:**
- No licensed tracked files.
- Create later: `rpg-dnd5e-web/docs/evidence/284-crypt-shell/README.md`

**Interfaces:**
- Consumes: merged provider commit and exact `env/shell-profiles.json` digest from Task 4.
- Produces: web issue/branch, exact ignored runtime sync, and provenance recorded for all later web tasks.

- [ ] **Step 1: Verify the provider merge before creating consumer work**

```bash
gh pr view "$ASSET_PR" -R KirkDiggler/rpg-game-assets --json mergedAt,mergeCommit
```

Expected: non-null merge time/SHA. If not merged, stop.

- [ ] **Step 2: Create/board the web issue and isolated branch**

Create one `rpg-dnd5e-web` issue linked to #284/#169/#791, board it as `Assets · The Dungeon · Build · Ready · Four-player Level-3 Dungeon`, fetch, and create `feat/${WEB_ISSUE}-crypt-shell` from `origin/dev`.

```bash
WEB_URL=$(gh issue create -R KirkDiggler/rpg-dnd5e-web \
  --title 'Render the archetype-driven crypt floor and two-sided wall shell' \
  --body 'Consumes the merged provider profile for KirkDiggler/rpg-project#284; closes #791 when landed. #823 remains separate.\n\n— assets agent, on behalf of KirkDiggler')
WEB_ISSUE=${WEB_URL##*/}
```

- [ ] **Step 3: Install the exact lock graph and establish baseline**

```bash
npm ci
npm run test:run -- \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/SessionCanvas.test.tsx \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/hex-grid/WallRunMesh.test.tsx \
  src/components/session/AtlasWalls.test.tsx
```

Expected: current tests pass before edits.

- [ ] **Step 4: Sync the exact merged provider and prove it remains ignored**

```bash
: "${RPG_GAME_ASSETS_DIR:?set RPG_GAME_ASSETS_DIR to the assets checkout}"
npm run assets:sync
git check-ignore -v public/models/synty/env/Crypt_Wall_Body_01.glb \
  public/models/synty/env/shell-profiles.json
git status --short
sha256sum public/models/synty/env/shell-profiles.json \
  public/models/synty/env/Crypt_Wall_{Body,Base,Cap,Door_Surround}_01.glb
```

Expected: synced files exist and are ignored; no licensed path appears in Git status. Record provider merge SHA and exact hashes in the web evidence README.

### Task 6: Strictly Parse and Cache the Runtime Shell Manifest

**Files:**
- Create: `rpg-dnd5e-web/src/rendering/dungeonShellManifest.ts`
- Test: `rpg-dnd5e-web/src/rendering/dungeonShellManifest.test.ts`
- Create: `rpg-dnd5e-web/src/rendering/dungeonShellProvider.ts`
- Test: `rpg-dnd5e-web/src/rendering/dungeonShellProvider.test.ts`
- Create: `rpg-dnd5e-web/src/components/session/useDungeonShellCatalog.ts`
- Test: `rpg-dnd5e-web/src/components/session/useDungeonShellCatalog.test.tsx`

**Interfaces:**
- Produces: `DungeonShellCatalog`, `DungeonShellProfile`, `DungeonShellFloorProfile`, `DungeonShellWallProfile`, `parseDungeonShellManifest(value)`, `preloadDungeonShellCatalog()`, `getDungeonShellCatalogSnapshot()`, and `useDungeonShellCatalog()`.

- [ ] **Step 1: Write parser RED tests**

Pin valid shape plus strict refusal of unknown keys, schema versions, unsafe paths, non-64-lowercase hashes, booleans/non-finite/zero repeat, missing components, false `twoSided`, wrong axes, inverted/degenerate bounds, and mutable output.

```ts
const parsed = parseDungeonShellManifest(validManifest());
expect(parsed).toEqual({ ok: true, catalog: expect.any(Object) });
if (!parsed.ok) throw new Error(parsed.reason);
expect(Object.isFrozen(parsed.catalog.profiles.crypt.wall.body.bounds.min)).toBe(true);
```

- [ ] **Step 2: Run parser tests RED**

```bash
npm run test:run -- src/rendering/dungeonShellManifest.test.ts
```

Expected: import failure.

- [ ] **Step 3: Implement manual closed-shape parsing**

Use no new dependency. Export:

```ts
type ShellVec3 = readonly [number, number, number];
export interface DungeonShellArtifact {
  readonly file: `env/${string}.glb`;
  readonly sha256: string;
  readonly bounds: { readonly min: ShellVec3; readonly max: ShellVec3 };
}
export interface DungeonShellFloorProfile {
  readonly diffuse: `textures/${string}.png`;
  readonly sha256: string;
  readonly worldUnitsPerRepeat: number;
}
export interface DungeonShellWallProfile {
  readonly body: DungeonShellArtifact & {
    readonly localSpanAxis: '+X';
    readonly localFaceAxis: 'Z';
    readonly twoSided: true;
  };
  readonly base: DungeonShellArtifact;
  readonly cap: DungeonShellArtifact;
  readonly doorSurround: DungeonShellArtifact;
}
export interface DungeonShellProfile {
  readonly floor: DungeonShellFloorProfile;
  readonly wall: DungeonShellWallProfile;
}
export interface DungeonShellCatalog {
  readonly schemaVersion: 1;
  readonly profiles: { readonly crypt: DungeonShellProfile };
}
export type DungeonShellManifestResult =
  | { ok: true; catalog: DungeonShellCatalog }
  | { ok: false; reason: string };

export function parseDungeonShellManifest(
  value: unknown
): DungeonShellManifestResult;
```

Paths must match the closed regular expressions `^textures/[A-Za-z0-9_.-]+\.png$` or `^env/[A-Za-z0-9_.-]+\.glb$`, contain no slash escapes/query/fragment, and resolve later only under `/models/synty/`. Deep-freeze the accepted catalog.

- [ ] **Step 4: Write provider RED tests**

Pin one in-flight owner, exact URL `/models/synty/env/shell-profiles.json`, UTF-8/JSON/HTTP/validation controlled failures, immutable ready/failed snapshots, reset isolation, and failure classification: HTTP/fetch is `manifest-unavailable`; UTF-8/JSON/schema rejection is `invalid-profile`.

```ts
expect(getDungeonShellCatalogSnapshot()).toEqual({ status: 'idle' });
const first = preloadDungeonShellCatalog();
const second = preloadDungeonShellCatalog();
expect(first).toBe(second);
await first;
expect(getDungeonShellCatalogSnapshot()).toMatchObject({ status: 'ready' });
```

- [ ] **Step 5: Implement provider snapshot and hook**

```ts
export type DungeonShellCatalogSnapshot =
  | { readonly status: 'idle' | 'loading' }
  | { readonly status: 'ready'; readonly catalog: DungeonShellCatalog }
  | {
      readonly status: 'failed';
      readonly failureKind: 'manifest-unavailable' | 'invalid-profile';
      readonly failureReason: string;
    };

export function preloadDungeonShellCatalog(): Promise<void>;
export function getDungeonShellCatalogSnapshot(): DungeonShellCatalogSnapshot;
export function __resetDungeonShellProviderForTests(): void;
```

`useDungeonShellCatalog` initializes from the snapshot, calls preload once in an effect for idle/loading, and replaces local state with the final snapshot on resolution/rejection. Guard unmount; StrictMode’s second effect reuses the same owner.

- [ ] **Step 6: Run focused tests GREEN and commit**

```bash
npm run test:run -- \
  src/rendering/dungeonShellManifest.test.ts \
  src/rendering/dungeonShellProvider.test.ts \
  src/components/session/useDungeonShellCatalog.test.tsx
npm run format:check

git add src/rendering/dungeonShellManifest* src/rendering/dungeonShellProvider* \
  src/components/session/useDungeonShellCatalog*
git commit -m "feat(rendering): load strict dungeon shell profiles (#${WEB_ISSUE})"
```

### Task 7: Carry Atlas Archetypes and Resolve One Atomic Profile

**Files:**
- Create: `rpg-dnd5e-web/src/rendering/dungeonShellProfile.ts`
- Test: `rpg-dnd5e-web/src/rendering/dungeonShellProfile.test.ts`
- Modify: `rpg-dnd5e-web/src/components/session/atlasToScene3D.ts`
- Modify: `rpg-dnd5e-web/src/components/session/atlasToScene3D.test.ts`

**Interfaces:**
- Consumes: Task 6 snapshots/catalog.
- Produces: `Scene3D.archetypes: readonly string[]`, `ShellFallbackReason`, `DungeonShellSelection`, and `resolveDungeonShellProfile(archetypes, snapshot)`.

- [ ] **Step 1: Write resolver and scene RED tests**

```ts
expect(resolveDungeonShellProfile(['crypt', 'crypt'], readyCatalog())).toEqual({
  kind: 'profile',
  key: 'crypt',
  profile: readyCatalog().catalog.profiles.crypt,
});
expect(resolveDungeonShellProfile(['crypt', 'cave'], readyCatalog())).toEqual({
  kind: 'legacy', reason: 'mixed-archetypes',
});
expect(resolveDungeonShellProfile(['crypt'], { status: 'loading' })).toEqual({
  kind: 'loading',
});
```

Cover no regions, empty/whitespace, unknown, mixed, idle/loading, failed `manifest-unavailable`, failed `invalid-profile`, and deterministic unique ordering. Extend `buildScene3D` tests so atlas regions `crypt`, `crypt` produce frozen `['crypt', 'crypt']` without deriving visual behavior there.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/rendering/dungeonShellProfile.test.ts \
  src/components/session/atlasToScene3D.test.ts
```

Expected: missing resolver and `Scene3D.archetypes`.

- [ ] **Step 3: Implement the pure resolver and additive scene field**

```ts
export type ShellFallbackReason =
  | 'no-regions'
  | 'mixed-archetypes'
  | 'unknown-archetype'
  | 'manifest-unavailable'
  | 'invalid-profile';

export type DungeonShellSelection =
  | { readonly kind: 'loading' }
  | { readonly kind: 'profile'; readonly key: 'crypt'; readonly profile: DungeonShellProfile }
  | { readonly kind: 'legacy'; readonly reason: ShellFallbackReason };
```

`buildScene3D` adds `regions` to its `Pick<GetAtlasResponse, 'cells' | 'props' | 'boundaries' | 'doorways' | 'regions'>` input and copies `atlas.regions.map(region => region.archetype)`; it does not default, trim, validate, or select.

- [ ] **Step 4: Run GREEN and commit**

```bash
npm run test:run -- \
  src/rendering/dungeonShellProfile.test.ts \
  src/components/session/atlasToScene3D.test.ts

git add src/rendering/dungeonShellProfile* src/components/session/atlasToScene3D*
git commit -m "feat(rendering): resolve uniform dungeon shell archetypes (#${WEB_ISSUE})"
```

### Task 8: Make Profile Floors Continuous in World Space

**Files:**
- Create: `rpg-dnd5e-web/src/components/hex-grid/dungeonFloorUv.ts`
- Test: `rpg-dnd5e-web/src/components/hex-grid/dungeonFloorUv.test.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/SyntyHexFloor.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/SyntyHexFloor.test.tsx`

**Interfaces:**
- Consumes: `DungeonShellFloorProfile` from Task 6.
- Produces: `dungeonFloorUv(worldX, worldZ, worldUnitsPerRepeat)` and optional `profile?: DungeonShellFloorProfile` on `SyntyHexFloor`.

- [ ] **Step 1: Write UV continuity RED tests**

```ts
expect(dungeonFloorUv(4, -2, 4)).toEqual([1, -0.5]);
expect(() => dungeonFloorUv(0, 0, 0)).toThrow(/positive/);

// Two named adjacent pointy hexes share the same absolute corner.
expect(profileUvsForHex(A, 4).filter(atSharedWorldVertex)).toEqual(
  profileUvsForHex(B, 4).filter(atSharedWorldVertex)
);
```

Also assert adding a distant cell does not alter an existing cell’s UVs, negative coordinates remain finite, profile texture URL equals `/models/synty/` plus `profile.diffuse`, remembered and visible cells share geometry UVs, and absent profile preserves the current normalized 0..1 per-hex UV plus `repeat.set(2,2)`.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/hex-grid/dungeonFloorUv.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx
```

- [ ] **Step 3: Implement the minimal profile branch**

```ts
export function dungeonFloorUv(
  worldX: number,
  worldZ: number,
  worldUnitsPerRepeat: number
): readonly [number, number] {
  if (!Number.isFinite(worldUnitsPerRepeat) || worldUnitsPerRepeat <= 0)
    throw new Error('worldUnitsPerRepeat must be finite and positive');
  return [worldX / worldUnitsPerRepeat, worldZ / worldUnitsPerRepeat];
}
```

When profile is active, compute UV before `rotateX` with absolute coordinates:

```ts
const absoluteX = tileWorld.x + pos.getX(i);
const absoluteZ = tileWorld.z - pos.getY(i);
[uv[i * 2], uv[i * 2 + 1]] = dungeonFloorUv(
  absoluteX,
  absoluteZ,
  profile.worldUnitsPerRepeat
);
```

Use `RepeatWrapping`, texture repeat `(1,1)`, the profile diffuse URL, existing unlit/tone-mapped-false material, and unchanged `DUNGEON_SURFACE_Y`. Keep the legacy branch byte-equivalent when profile is absent.

- [ ] **Step 4: Run GREEN and commit**

```bash
npm run test:run -- \
  src/components/hex-grid/dungeonFloorUv.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/hex-grid/floorOverlayHeights.test.ts

git add src/components/hex-grid/dungeonFloorUv* src/components/hex-grid/SyntyHexFloor*
git commit -m "feat(rendering): align crypt floor masonry across hexes (#${WEB_ISSUE})"
```

### Task 9: Render the Profile Wall Body, Base, Cap, and Door Surround

**Files:**
- Create: `rpg-dnd5e-web/src/components/hex-grid/dungeonShellWallHelpers.ts`
- Test: `rpg-dnd5e-web/src/components/hex-grid/dungeonShellWallHelpers.test.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/GlbInstance.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/GlbInstance.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/WallRunMesh.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/WallRunMesh.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/AtlasWalls.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/AtlasWalls.test.tsx`

**Interfaces:**
- Consumes: `DungeonShellWallProfile`, existing `tileWallSegment`, effective/authored/cutaway wall heights, existing door gaps/state.
- Produces: `shellBodyScale`, `shellTrimScale`, `shellDoorSurroundScale`, `shellComponentY`, additive `positionY?: number` on `GlbInstance`, and optional `profile?: DungeonShellWallProfile` on `WallRunMesh`/`AtlasWalls`.

- [ ] **Step 1: Write transform and placement RED tests**

For measured bounds, assert:

```ts
expect(shellBodyScale(body, 1, 2.4)).toEqual([
  1 / (body.bounds.max[0] - body.bounds.min[0]),
  2.4 / (body.bounds.max[1] - body.bounds.min[1]),
  SYNTY_SCALE,
]);
expect(shellTrimScale(base, 1)).toEqual([
  1 / (base.bounds.max[0] - base.bounds.min[0]),
  SYNTY_SCALE,
  SYNTY_SCALE,
]);
expect(shellDoorSurroundScale(doorSurround, 2.4)).toEqual([
  DOOR_FRAME_CALIBRATED_WIDTH /
    (doorSurround.bounds.max[0] - doorSurround.bounds.min[0]),
  2.4 / (doorSurround.bounds.max[1] - doorSurround.bounds.min[1]),
  SYNTY_SCALE,
]);
expect(shellComponentY('base', 2.4)).toBe(DUNGEON_SURFACE_Y);
expect(shellComponentY('cap', 2.4)).toBe(DUNGEON_SURFACE_Y + 2.4);
```

Cover body/base/cap X fit, finite positive bounds, base/cap vertical scale fixed at `SYNTY_SCALE`, body height at standard/2×/cutaway, residual run widths, both facings, exact files, no `FloorSkirtBox` in profile mode, profile body rendered once per tile (not mirrored), and legacy output unchanged without profile.

Extend `GlbInstance` test to assert `positionY` defaults to 0 and sets primitive world Y additively.

For doors, assert the profile surround path replaces only the frame; existing leaf file, open omission, locked/closed presence, click ID, rotation, and height remain.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/hex-grid/dungeonShellWallHelpers.test.ts \
  src/components/hex-grid/GlbInstance.test.tsx \
  src/components/hex-grid/WallRunMesh.test.tsx \
  src/components/session/AtlasWalls.test.tsx
```

- [ ] **Step 3: Implement pure measured-bounds transforms**

```ts
export function shellComponentY(
  kind: 'body' | 'base' | 'cap' | 'doorSurround',
  effectiveHeight: number
): number {
  return kind === 'cap'
    ? DUNGEON_SURFACE_Y + effectiveHeight
    : DUNGEON_SURFACE_Y;
}
```

Implement `shellBodyScale` as `[pieceWidth / rawSpan, effectiveHeight / rawHeight, SYNTY_SCALE]`, `shellTrimScale` as `[pieceWidth / rawSpan, SYNTY_SCALE, SYNTY_SCALE]`, and `shellDoorSurroundScale` as `[DOOR_FRAME_CALIBRATED_WIDTH / rawSpan, wallHeight / rawHeight, SYNTY_SCALE]`. Reject non-positive dimensions before division. Base/cap provider-normalized min Y is zero. Apply profile bound-min pivot correction along X so every actual bbox begins/ends at the run tile boundary.

- [ ] **Step 4: Implement the profile path without disturbing legacy**

In `WallRunMesh`, branch only inside each existing run after final effective height is known. Profile mode renders body+base+cap from the same `tileWallSegment` result and omits `FloorSkirtBox`; legacy mode remains the current `TiledWallRun` + skirt. Do not add random variants, runtime tint, backface material changes, or a second wall-run derivation.

In `AtlasWalls`, pass `profile` to `WallRunMesh`; use `profile.doorSurround.file`, `shellDoorSurroundScale(profile.doorSurround, wallHeight)`, and `positionY={DUNGEON_SURFACE_Y}`, while the current door leaf/state/click behavior stays unchanged.

- [ ] **Step 5: Run GREEN and commit**

```bash
npm run test:run -- \
  src/components/hex-grid/dungeonShellWallHelpers.test.ts \
  src/components/hex-grid/GlbInstance.test.tsx \
  src/components/hex-grid/WallRunMesh.test.tsx \
  src/components/hex-grid/wallRunMeshHelpers.test.ts \
  src/components/session/AtlasWalls.test.tsx \
  src/components/session/atlasWallRuns.test.ts

git add src/components/hex-grid/dungeonShellWallHelpers* \
  src/components/hex-grid/GlbInstance* src/components/hex-grid/WallRunMesh* \
  src/components/session/AtlasWalls*
git commit -m "feat(rendering): draw the finished two-sided crypt wall family (#${WEB_ISSUE})"
```

### Task 10: Mount One Atomic Dungeon Shell in Builder and Game

**Files:**
- Create: `rpg-dnd5e-web/src/components/session/DungeonShell.tsx`
- Test: `rpg-dnd5e-web/src/components/session/DungeonShell.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/preview3d/DungeonPreview3D.tsx`
- Modify: `rpg-dnd5e-web/src/author/preview3d/DungeonPreview3D.test.ts`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.test.tsx`

**Interfaces:**
- Consumes: `Scene3D`, Tasks 6–9 catalog/resolver/profile leaves.
- Produces: one `DungeonShell` used by both routes and optional `onFallbackReason(reason | null)` for builder diagnostics.

- [ ] **Step 1: Write atomic/parity RED tests**

```tsx
<DungeonShell
  scene={scene}
  doors={doors}
  onDoorClick={onDoorClick}
  onFallbackReason={onFallbackReason}
/>
```

Cover:

- ready uniform crypt mounts one profiled floor and profiled walls;
- loading mounts both legacy leaves with no builder warning; failed, unknown, mixed, empty, parser failure, texture rejection, and each of four GLB load rejections mount both legacy leaves and no profile leaf;
- resource loading never reveals only one profile leaf;
- builder and session each mount exactly one `DungeonShell` and no direct `SyntyHexFloor`/`AtlasWalls`;
- both pass the same scene archetypes/floor tiles/wall runs/door gaps;
- game forwards door map/click; builder omits live state/click;
- builder names fallback reason; game does not render that diagnostic;
- both still mount `DungeonSceneLights` once outside the shell.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/session/DungeonShell.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/SessionCanvas.test.tsx
```

- [ ] **Step 3: Implement one resource gate and complete fallback**

`DungeonShell` resolves catalog/archetypes first. Loading selection returns both legacy leaves with `onFallbackReason(null)`. Legacy selection returns both legacy leaves and its named reason. Profile selection wraps one `ProfileResources` child in one ErrorBoundary and one Suspense whose fallback is `LegacyShell` without a warning while resources are pending.

`ProfileResources` calls `useTexture` for the profile diffuse and `useGLTF` for body/base/cap/surround before returning either leaf; these calls populate drei’s URL caches. Therefore child leaf loaders are already ready and cannot independently suspend into a half shell.

Use exact runtime roots:

```ts
const modelUrl = (file: `env/${string}.glb`) => `/models/synty/${file}`;
const textureUrl = (file: `textures/${string}.png`) => `/models/synty/${file}`;
```

A resource error returns `LegacyShell` and reports `manifest-unavailable` to the builder; do not retry in a render loop.

- [ ] **Step 4: Replace both call sites and add builder diagnostic**

Replace adjacent direct floor/wall leaves in `DungeonPreview3D` and `SessionCanvas` with `DungeonShell`. Keep `DungeonSceneLights`, props, monsters, markers, camera, and controls exactly where they are. Builder diagnostic sits beside the existing stale-atlas banner and renders ``Legacy shell: ${reason}`` from the closed `ShellFallbackReason` union.

- [ ] **Step 5: Run parity suites GREEN and commit**

```bash
npm run test:run -- \
  src/components/session/DungeonShell.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/SessionCanvas.test.tsx \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonSceneLights.test.tsx

git add src/components/session/DungeonShell* \
  src/author/preview3d/DungeonPreview3D* src/components/session/SessionCanvas*
git commit -m "feat(rendering): share one atomic dungeon shell across builder and play (#${WEB_ISSUE})"
```

### Task 11: Exact-Provider Integration, Real-Path Evidence, Review, and Web Landing

**Files:**
- Modify if required: `rpg-dnd5e-web/src/author/fixtures/cryptPropShowcase.ts`
- Modify related fixture tests.
- Create: `rpg-dnd5e-web/docs/evidence/284-crypt-shell/README.md`
- Create: `before-builder.png`, `candidate-builder.png`, `candidate-game.png`, `final-builder.png`, `final-game.png`.

**Interfaces:**
- Consumes: all web tasks and exact merged provider sync.
- Produces: accepted integrated proof, merged web PR SHA, closed #791, and data for Task 12.

- [ ] **Step 1: Pin the fixture and focused integration tests**

Assert every region in the showcase carries `archetype: crypt`; add a long wall, inside/outside corner, T-junction, locked doorway, standard wall, and raised wall if the existing fixture lacks any forcing case. Preserve accepted cage/table/rug refs and transforms.

```bash
npm run test:run -- \
  src/author/fixtures/cryptPropShowcase.test.ts \
  src/author/dungeonYaml.test.ts \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/DungeonShell.test.tsx
```

- [ ] **Step 2: Run format, focused suites, and full CI**

```bash
npm run format
npm run test:run -- \
  src/rendering/dungeonShellManifest.test.ts \
  src/rendering/dungeonShellProvider.test.ts \
  src/rendering/dungeonShellProfile.test.ts \
  src/components/hex-grid/dungeonFloorUv.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/hex-grid/dungeonShellWallHelpers.test.ts \
  src/components/hex-grid/WallRunMesh.test.tsx \
  src/components/session/AtlasWalls.test.tsx \
  src/components/session/DungeonShell.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/SessionCanvas.test.tsx
npm run ci-check
```

Record exact file/test counts and every CI sub-gate. Do not claim success from exit code alone.

- [ ] **Step 3: Start the real local path and capture candidate evidence**

Use the project local-dev runbook with the branch web, authoring-enabled API, and exact synced provider. Open the crypt fixture in the builder, Save, reopen it, then Save & Play. Capture 1600×900 builder/game frames that visibly include continuous floor, both wall faces, cap/base, corner, junction, doorway, standard/raised wall, and accepted props.

Verify network requests use the exact manifest/body/base/cap/surround paths and hashes recorded in the evidence README. Never copy the manifest or GLBs into docs.

- [ ] **Step 4: Run Kirk’s integrated visual gate**

Stop and provide both URLs and candidate frames. If rejected, iterate on the owning provider/web branch without consumer-only visual repairs. Record the verbatim verdict only after Kirk sees both builder and playable routes.

- [ ] **Step 5: Commit final public-safe evidence**

README records provider PR/merge SHA, manifest and artifact hashes, web head, commands/counts, Save/reopen YAML identity, Save & Play route, #791 two-face proof, #823 separation, screenshot hashes, and verdict.

```bash
git add src docs/evidence/284-crypt-shell
git diff --check
git commit -m "feat(dungeon): render the approved crypt shell (#${WEB_ISSUE})"
```

- [ ] **Step 6: Push, open one web PR, and request one Copilot review**

Open against `dev`, not `main`; include exact provider authority and Assets signature. Fix valid findings with failing tests first, rerun focused tests and full `npm run ci-check`, reply to each inline finding with commit/test evidence, and request no second automated review.

- [ ] **Step 7: Ask Kirk to merge and verify landing**

Agents do not merge. After Kirk merges, verify merge tree equals reviewed head or document the exact difference, all checks green, web issue Done, and #791 closed/Done. Keep #823 open/Shaping.

### Task 12: Record the Landed Wave and Keep Lighting Active

**Files:**
- Modify: `rpg-project/ideas/battlemap/dungeon-builder/crypt-shell.md`
- Modify: `rpg-project/ideas/battlemap/dungeon-builder/crypt-shell-plan.md`
- Create: `rpg-project/ideas/battlemap/dungeon-builder/evidence/crypt-shell/README.md`
- Copy only accepted public-safe final builder/game PNGs into the project evidence folder.

**Interfaces:**
- Consumes: merged provider/web PRs, exact hashes, tests, screenshots, and Kirk verdict.
- Produces: durable landed ledger on PR #285, reconciled Project 19 state, and implementation closeout.

- [ ] **Step 1: Add the landed ledger**

Record exact provider/web issue+PR links, reviewed heads, merge SHAs/trees, manifest/artifact/screenshot hashes, selected candidate IDs, test counts, seven web CI gates, provider commands, fallback behavior, Save/reopen/Save & Play result, and verbatim verdict. Do not mark historical command checkboxes that were superseded; add a dated execution outcome.

- [ ] **Step 2: Verify the public/private boundary and evidence hashes**

```bash
! git ls-files | grep -E '\.(glb|blend)$|shell-profiles\.json$'
sha256sum ideas/battlemap/dungeon-builder/evidence/crypt-shell/*.png
git diff --check
rg -n 'TBD|TODO|FIXME|<approved|<exact|<selected|<number|<filename|<diffuse|<named' ideas/battlemap/dungeon-builder/crypt-shell*.md && exit 1 || true
```

Expected: no licensed/provider runtime files tracked; no placeholders; exact accepted hashes.

- [ ] **Step 3: Commit and push the project record**

```bash
git add ideas/battlemap/dungeon-builder/crypt-shell.md \
  ideas/battlemap/dungeon-builder/crypt-shell-plan.md \
  ideas/battlemap/dungeon-builder/evidence/crypt-shell
git commit -m 'docs(dungeon-builder): record crypt shell landing (#284)'
git push
```

- [ ] **Step 4: Reconcile issues and Project 19**

Confirm provider/web slices closed Done, #791 closed Done, #823 open Shaping, #284 In Review until PR #285 merges, and #169 open In Progress for lighting. Post signed landed comments with exact links/hashes. Do not close journey #169.

- [ ] **Step 5: Final review and human merge gate**

Run one broad cross-repository review over provider, web, and project ranges. Fix Critical/Important findings with scoped tests. Ask Kirk to merge project PR #285; after merge, verify #284 Done and remove only this wave’s clean worktrees/branches/processes using the finishing-a-development-branch workflow.

## Task 12 execution outcome — initially recorded 2026-08-26

The approved provider and web slices are landed and recorded without changing
those repositories. Provider issue #65 / PR #68 merged at
`f183c96d6d89ecdaf9a2f5dd2c452de485882ed3`; web issue #825 / PR #827 merged at
`548f561bf8ddab41da53a174e5b69a08358b11e1`. The full landed ledger, exact
provider/runtime hashes, review disposition, real-path result, and verdicts are
in `crypt-shell.md`; public-safe final builder/game evidence is in
`evidence/crypt-shell/README.md`.

Before editing, `origin/main` at
`9ae62e020c2220954f223c61393a743fd099e92a` was merged into this design branch
as `0435af0ab94544ada2cf430d2d3097e269be7cce`; that merge commit is the
post-merge docs base. A fresh visible `npm run test:run` on web reviewed head
`a770746d73c6bfe35cc743383005e7f796ec672e` reported 231 files passed and 1
skipped, with 3,648 tests passed and 1 skipped.

The initial record's then-current PNGs were verified at 1600×900:
`final-builder.png`
`0854b0d0bc4dd56a62185ffcfb774230ad77583f90696068308f6200368f7a83` and
`final-game.png`
`b44ef4dd027eaefc02db77f35bb31bb4461b1948dc80974b1bbbcfbe74d9baaa`. They
are superseded pre-registration evidence, not current final evidence. The
public boundary scan excludes GLB, Blend, shell-manifest, placeholders,
private/local metadata, and unrelated provider evidence. Before and against
Task 12 documentation commit `d961de0`, `git diff --check`, the
placeholder/private/license scans, image SHA-256/dimension/`cmp` checks, and
the scoped final review all ran clean. The only pending item is the manual
post-publication external reconciliation of #825 and #791; no external state
was mutated here.

Per the explicit closeout constraint, this session did not push, post comments,
change issues or Project 19, merge PR #285, modify web, or clean worktrees and
processes. Copilot was unavailable for the final supported attempts and had no
review event; the static banner ruling, separate #823 follow-up, and active
lighting work under #169 remain recorded.

## Task 12 finalization report (2026-08-27)

Web follow-up issue #828 / PR #829 merged as
`c38ab663a9ced71bd494035854ec67c662205f0c`; its reviewed head was
`9ca2bf4d86a8a164c2b1ebe6fd54180c0f924a61` and its merge tree was
`786bc4bbff12406f9721c918c950675d3f85691e`. The root defect was fixed with
geometry-derived scale and child-local registration under the exact `gapStart`
hinge. Standard and raised walls across four facings measured left/right/top
cover at least `0.020000901`, with floor contact `0`; thinness was deliberately
unchanged. The verdict was **`door is pretty thin but no gaps`**.

The provider, profile, door-frame, and closed-leaf hashes are unchanged. The
post-fix `final-builder.png`, `final-game.png`, and new `close-door.png` are all
`1600×900`, with hashes recorded in the evidence README. Prior `0854...` and
`b44e...` frames are marked there as superseded pre-registration evidence.
Focused coverage was 21 files / 372 tests; full coverage was 231 files / 3,653
tests, with one file and one test skipped. `npm run ci-check` passed all seven
gates and all four GitHub checks passed. Copilot’s one SHA typo was fixed in
`9ca2bf4` and answered in the review reply.

The current published head `30544fc` was merged with `origin/main`
`f4415270ff14d6ca7ab21f6cb1b2bb79da6a688d` without rebasing as
`4cb86155e3edf8a5047c8e889b2a3b52e27e7267`, the post-merge docs base. #828
remains open pending post-publication manual close; #823 remains separate and
#169 remains active for authored lighting. This finalization made no external
comments, issue/board changes, pushes, merges, cleanup, or web/provider edits.
