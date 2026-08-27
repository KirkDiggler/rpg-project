# Synty Subscription Race Catalog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a private index of all 34 current Synty archives and a visual, noncanonical candidate gallery for the classic 2014 PHB nine races without promoting runtime assets.

**Architecture:** A pure-Python scanner records archive facts without bulk extraction; a separate curated catalog references those stable inventory IDs; a selective materializer extracts only reviewed candidates; Blender records rig/mesh facts and renders front/three-quarter views; a Pillow gallery builder produces pack sheets and the classic-nine coverage sheet. Discovery output remains under `library/synty-subscription/` and issue-scoped `evidence/73-synty-race-catalog/`; `.stage/` holds ignored licensed working bytes.

**Tech Stack:** Python 3.11+ standard library (`argparse`, `dataclasses`, `hashlib`, `json`, `pathlib`, `tarfile`, `zipfile`), `unittest`, Blender 5.0.1 Python API, Pillow, Git/GitHub CLI.

**Spec:** `rpg-project/ideas/assets/synty-subscription-fast-lane/design.md`

## Global Constraints

- Implementation issue is `KirkDiggler/rpg-game-assets#73`; Project 19 fields remain Assets / Game Screen / Build / Ready until execution starts.
- Start `asset/73-synty-race-catalog` from fresh `origin/main` in an isolated worktree; never edit the stale main checkout.
- The subscription root is supplied through `--source-root` or `SYNTY_SUBSCRIPTION_ROOT`; no tracked file stores the operator's absolute source path.
- Scan all 34 current archives, including ZIP and Unity-package logical pathnames, without bulk extraction.
- Preserve archive-relative identity and duplicate basenames; never flatten source members into one namespace.
- Raw FBXs, textures, Unity payloads, and staged extracts remain ignored/private and are never committed.
- Index Modular Fantasy Hero and Sidekick Modular, but mark both deferred; do not assemble either system.
- Treat Emotes/Taunts, Sword Combat, and Bow Combat as animation inventory, not character bodies or race-gallery gates.
- Race mappings are optional, many-to-many, noncanonical suggestions. The catalog may leave honest gaps.
- Produce no runtime aliases, promoted GLBs, game contracts, customization UI, portraits, downed poses, animation bindings, or web sync.
- Use one implementation branch/PR and one final whole-change review. Do not run task-by-task SDD review/fix/re-review rounds.
- Tests cover source safety, determinism, classification, references, and one individual/combined Blender smoke; do not add release seals, transaction installers, screenshot hashes, or JSON key-order assertions without consumer value.

## File Map

| File | Responsibility |
| --- | --- |
| `scripts/index_synty_subscription.py` | Scan ZIP/Unity archives, classify members conservatively, emit/check deterministic inventory JSON |
| `scripts/test_index_synty_subscription.py` | Fixture ZIP/Unity scans, duplicate-path, unsafe-path, classification, determinism, and CLI tests |
| `library/synty-subscription/index.json` | Generated factual inventory for the current 34-archive snapshot |
| `scripts/build_synty_race_catalog.py` | Nominate individual bodies and combined character sources, seed/validate curated candidates, and derive classic-nine coverage data |
| `scripts/test_build_synty_race_catalog.py` | Candidate nomination, reference, vocabulary, deferral, and coverage tests |
| `library/synty-subscription/race-candidates.json` | Human-reviewed candidate mappings, adaptations, render profiles, rejections, and deferred systems |
| `scripts/materialize_synty_race_catalog.py` | Safely extract only referenced candidate/texture members into ignored `.stage/` |
| `scripts/test_materialize_synty_race_catalog.py` | ZIP/Unity extraction, traversal, missing-member, collision, and source-immutability tests |
| `scripts/render_synty_race_catalog.py` | Enumerate combined FBXs, inspect rigs/meshes/materials, render front/three-quarter candidate images, record failures |
| `scripts/test_render_synty_race_catalog.py` | Pure render-job contract tests and externally gated real Blender smoke assertions |
| `scripts/build_synty_race_gallery.py` | Compose labeled pack contact sheets, error cards, README, and classic-nine coverage sheet |
| `scripts/test_build_synty_race_gallery.py` | Synthetic-image layout, candidate labeling, error-card, and honest-gap tests |
| `evidence/73-synty-race-catalog/inspection.json` | Generated candidate inspection results and render/error records |
| `evidence/73-synty-race-catalog/packs/*.png` | Pack-level candidate contact sheets |
| `evidence/73-synty-race-catalog/classic-nine.png` | Initial race coverage review sheet |
| `evidence/73-synty-race-catalog/README.md` | Commands, interpretation, source boundaries, and Kirk's gallery verdict |
| `README.md` | Private subscription-index layout and repeatable discovery commands |

---

### Task 1: Track the Build and Generate the Factual Archive Index

**Files:**
- Create: `scripts/index_synty_subscription.py`
- Create: `scripts/test_index_synty_subscription.py`
- Create: `library/synty-subscription/index.json`

**Interfaces:**
- Produces `CatalogError`, `normalize_member_path(raw: str) -> str`, `classify_member(archive_name: str, member_path: str) -> tuple[str, str]`, `scan_subscription(source_root: Path) -> dict[str, object]`, and `index_bytes(document: dict[str, object]) -> bytes`.
- Inventory member IDs use the readable exact format `{archive_filename}::{normalized_logical_member_path}`.
- Later tasks consume top-level `archives`, `members`, `duplicateBasenameGroups`, and `summary` from `index.json`; no later task rescans archives independently.

- [ ] **Step 1: Confirm tracking and create the isolated asset worktree**

Run:

```bash
gh issue view 73 --repo KirkDiggler/rpg-game-assets --json state,url,projectItems
asset_repo="$(git -C rpg-game-assets rev-parse --show-toplevel)"
git -C "$asset_repo" fetch origin
worktree="$HOME/.pi/worktrees/rpg-game-assets/73-synty-race-catalog"
git -C "$asset_repo" worktree add "$worktree" -b asset/73-synty-race-catalog origin/main
cd "$worktree"
python3 -m unittest discover -s scripts -p 'test_*.py'
gh project item-edit \
  --id PVTI_lAHOAASbwc4Bcj4vzg4NEnc \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id a434eab1
```

Expected: issue #73 is open and Project 19 Todo before work begins; the worktree starts at current `origin/main`; the existing provider suite passes with only established skips; then only #73 moves to Project 19 `In Progress`.

- [ ] **Step 2: Write failing ZIP, Unity, classification, and determinism tests**

Create fixture helpers that write archives entirely under `tempfile.TemporaryDirectory()`. The first tests must include:

```python
class SubscriptionIndexTests(unittest.TestCase):
    def test_scans_zip_and_preserves_duplicate_basenames(self):
        self.write_zip("Pack_SourceFiles_v1.zip", {
            "Characters/SK_Chr_Hero_01.fbx": b"body-a",
            "Unreal/SK_Chr_Hero_01.fbx": b"body-b",
            "Characters/SM_Chr_Attach_Horns_01.fbx": b"horns",
            "Animations/A_POLY_BOW_Stand_Attack_01.fbx": b"anim",
        })
        document = scan_subscription(self.root)
        self.assertEqual(1, document["summary"]["archiveCount"])
        self.assertEqual(4, document["summary"]["fbxCount"])
        bodies = [item for item in document["members"] if item["category"] == "complete-body"]
        self.assertEqual(2, len(bodies))
        self.assertEqual(1, len(document["duplicateBasenameGroups"]))
        self.assertNotEqual(bodies[0]["id"], bodies[1]["id"])

    def test_reads_unity_logical_pathnames_without_extracting_payloads(self):
        self.write_unitypackage({
            "abc123": ("Assets/Synty/Models/Characters.fbx", b"combined"),
            "def456": ("Assets/Synty/Prefabs/Characters/SM_Chr_Chief_01.prefab", b"prefab"),
        })
        document = scan_subscription(self.root)
        by_path = {item["path"]: item for item in document["members"]}
        self.assertEqual("combined-needs-split", by_path["Assets/Synty/Models/Characters.fbx"]["delivery"])
        self.assertEqual("abc123/asset", by_path["Assets/Synty/Models/Characters.fbx"]["containerMember"])

    def test_index_bytes_are_deterministic_and_contain_no_source_root(self):
        first = index_bytes(scan_subscription(self.root))
        second = index_bytes(scan_subscription(self.root))
        self.assertEqual(first, second)
        self.assertNotIn(str(self.root).encode(), first)
```

Also pin these classifications: `ANIMATION_*` archives and `A_*` FBXs as `animation`; `ModularParts_` as `modular-part/modular-deferred`; individually exported `SK_Chr_*`, `SK_Character_*`, bare `Chr_*`, and `Character_*` FBXs as `complete-body` except names containing `Attach`, `Hair`, or `Cape`; character presets in Unity packages as `character-preset/combined-needs-split`; `Characters.fbx`, `Character_BR.fbx`, `CharactersBR.fbx`, and `ModularCharacters*.fbx` as `combined-character` with the appropriate combined or modular readiness; unknown files remain `unknown`.

- [ ] **Step 3: Run focused tests and verify RED**

Run:

```bash
python3 -m unittest scripts.test_index_synty_subscription -v
```

Expected: FAIL with `ModuleNotFoundError: No module named 'index_synty_subscription'`.

- [ ] **Step 4: Implement the minimal scanner and CLI**

Implement strict POSIX normalization and deterministic records. Core shapes:

```python
class CatalogError(ValueError):
    pass


def normalize_member_path(raw: str) -> str:
    value = raw.replace("\\", "/")
    parts = value.split("/")
    if not value or value.startswith("/") or any(part in {"", ".", ".."} for part in parts):
        raise CatalogError(f"unsafe archive member path: {raw!r}")
    return PurePosixPath(*parts).as_posix()


def member_id(archive_name: str, logical_path: str) -> str:
    return f"{archive_name}::{logical_path}"


def index_bytes(document: dict[str, object]) -> bytes:
    return (json.dumps(document, indent=2, sort_keys=True, ensure_ascii=True, allow_nan=False) + "\n").encode()
```

The CLI accepts `--source-root`, `--output` (default `library/synty-subscription/index.json`), and `--check`. It hashes each archive once, records ZIP members from the central directory, resolves Unity `GUID/pathname` to `GUID/asset`, sorts records by `(archive.casefold(), path.casefold(), path)`, computes summary counts from emitted records, writes through a temporary sibling plus `os.replace`, and leaves the prior output untouched if any archive is unreadable.

- [ ] **Step 5: Add refusal and continuation tests**

Add exact coverage for unsafe `../` and absolute member paths, a malformed archive, case-insensitive duplicate basenames, two archive versions with independent identities, one unknown extension, and a source directory containing no supported archives. Require errors to name only archive/member-relative identities.

Run:

```bash
python3 -m unittest scripts.test_index_synty_subscription -v
```

Expected: PASS.

- [ ] **Step 6: Generate and inspect the real 34-archive index**

Run:

```bash
: "${SYNTY_SUBSCRIPTION_ROOT:?set SYNTY_SUBSCRIPTION_ROOT to the private subscription drop}"
python3 scripts/index_synty_subscription.py \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT" \
  --output library/synty-subscription/index.json
python3 scripts/index_synty_subscription.py \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT" \
  --output library/synty-subscription/index.json \
  --check
jq '.summary' library/synty-subscription/index.json
```

Expected summary includes `archiveCount: 34`, `fbxCount: 17794`, and no absolute path. Differences in other derived category counts are evidence to inspect, not numbers to force.

- [ ] **Step 7: Run the focused tests and commit the factual index**

```bash
python3 -m unittest scripts.test_index_synty_subscription -v
git diff --check
git add scripts/index_synty_subscription.py scripts/test_index_synty_subscription.py library/synty-subscription/index.json
git commit -m 'feat: index Synty subscription archives (#73)'
```

---

### Task 2: Build and Validate the Curated Race-Candidate Contract

**Files:**
- Create: `scripts/build_synty_race_catalog.py`
- Create: `scripts/test_build_synty_race_catalog.py`
- Create: `library/synty-subscription/race-candidates.json`

**Interfaces:**
- Consumes Task 1's inventory document only.
- Produces `CLASSIC_NINE`, `candidate_id(member: dict[str, object], collides: bool) -> str`, `nominate_candidates(index: dict[str, object]) -> list[dict[str, object]]`, `seed_catalog(index: dict[str, object]) -> dict[str, object]`, `validate_catalog(index: dict[str, object], catalog: dict[str, object], require_reviewed: bool) -> list[str]`, and `build_coverage(catalog: dict[str, object]) -> dict[str, object]`.
- Task 3 consumes each candidate's `inventoryId`, `renderProfile`, optional `objectNames`, and each render profile's `textureMemberIds`.

- [ ] **Step 1: Write failing nomination, reference, deferral, and coverage tests**

Use a minimal in-memory index and pin the classic-nine vocabulary:

```python
class RaceCatalogTests(unittest.TestCase):
    def test_seed_nominates_bodies_but_not_accessories_animations_or_modular_parts(self):
        catalog = seed_catalog(self.index())
        self.assertEqual(
            ["Human", "Elf", "Dwarf", "Halfling", "Dragonborn", "Gnome", "Half-Elf", "Half-Orc", "Tiefling"],
            catalog["targetRaces"],
        )
        self.assertEqual(
            ["pack.zip::Characters/SK_Chr_Hero_01.fbx", "pack.zip::Characters/Characters.fbx"],
            [item["inventoryId"] for item in catalog["candidates"]],
        )
        self.assertEqual(["modular-fantasy-hero", "sidekick-modular"], [item["id"] for item in catalog["deferredSystems"]])

    def test_validation_rejects_unknown_inventory_ids_and_canonical_claims(self):
        catalog = seed_catalog(self.index())
        catalog["candidates"][0]["inventoryId"] = "missing.zip::missing.fbx"
        self.assertIn("unknown inventoryId", "\n".join(validate_catalog(self.index(), catalog, False)))
        catalog = seed_catalog(self.index())
        catalog["candidates"][0]["canonicalRace"] = "Human"
        self.assertIn("unsupported candidate keys", "\n".join(validate_catalog(self.index(), catalog, False)))

    def test_coverage_preserves_an_honest_gap(self):
        catalog = self.reviewed_catalog(possible_mappings=["Human"])
        coverage = build_coverage(catalog)
        self.assertEqual("direct", coverage["Human"]["status"])
        self.assertEqual("gap", coverage["Dragonborn"]["status"])
```

Candidate keys are exact: `candidateId`, `inventoryId`, `sourceDisplayName`, `neutralVisualGroup`, `possibleMappings`, `adaptations`, `adaptationEffort`, `rigFamily`, `sourceReadiness`, `reviewStatus`, `renderProfile`, `objectNames`, and `notes`. `possibleMappings` accepts the classic nine plus the format `Other:{display-name}`; it never accepts a `canonicalRace` field.

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_build_synty_race_catalog -v
```

Expected: FAIL with missing module.

- [ ] **Step 3: Implement seed/check/coverage commands**

Top-level catalog shape:

```python
{
    "schemaVersion": 1,
    "targetRaces": list(CLASSIC_NINE),
    "renderProfiles": {},
    "candidates": nominate_candidates(index),
    "rejections": [],
    "deferredSystems": [
        {"id": "modular-fantasy-hero", "readiness": "modular-deferred", "reason": "720 skinned parts require assembly"},
        {"id": "sidekick-modular", "readiness": "unreal-deferred", "reason": "higher-poly Unreal extraction and fit are a later investigation"},
    ],
}
```

Each `renderProfiles` value has exact keys `archive`, `textureMemberIds`, `importScale`, and `materialMode`; `materialMode` is `source-links` or `single-atlas`. `textureMemberIds` may be empty only for `source-links`, and every listed texture must belong to the profile archive. Candidates reference a profile by its map key.

`seed` writes every generated individual-body or combined-character nomination with a stable `candidateId`, `reviewStatus: "unreviewed"`, empty mappings/adaptations, `adaptationEffort: "low"`, empty `rigFamily`, and its source readiness. Candidate IDs are `{pack-slug}--{source-stem-slug}`; duplicate IDs gain `--{first-eight-hex-of-sha256(inventoryId)}`. Pack slugs strip the extension and the `_SourceFiles_*`, `_Source_Files_*`, or `_Unity_*` suffix before replacing non-alphanumeric runs with `-` and lowercasing; source-stem slugs apply the same character normalization to the member basename without its extension. Allowed review states are `unreviewed`, `metadata-reviewed`, and `visual-reviewed`. Each `rejections` entry is exactly `{inventoryId, reason}`. `check` validates references and schema; `check --require-reviewed` additionally requires every nomination to be either `visual-reviewed` or represented by one rejection with a nonempty reason. `coverage` derives statuses only from visually reviewed candidates and writes no candidate choices back into the catalog.

- [ ] **Step 4: Add exact mutation tests and make them pass**

Cover duplicate candidate IDs, a candidate listed as both accepted/rejected, invalid effort/status/readiness, unsupported race names, a metadata/visual-reviewed candidate missing its render profile, a metadata/visual-reviewed combined candidate without `objectNames`, an individual candidate with `objectNames`, and a render profile referencing a non-texture member. Unreviewed stubs may keep `renderProfile` and combined `objectNames` empty until metadata curation/enumeration.

Run:

```bash
python3 -m unittest scripts.test_build_synty_race_catalog -v
```

Expected: PASS.

- [ ] **Step 5: Seed the real catalog without inventing race mappings**

```bash
python3 scripts/build_synty_race_catalog.py seed \
  --index library/synty-subscription/index.json \
  --output library/synty-subscription/race-candidates.json
python3 scripts/build_synty_race_catalog.py check \
  --index library/synty-subscription/index.json \
  --catalog library/synty-subscription/race-candidates.json
jq '{candidateCount:(.candidates|length),deferredSystems}' library/synty-subscription/race-candidates.json
```

Expected: individual-body and combined-source nominations are present, possible mappings remain empty, and both modular systems are deferred.

- [ ] **Step 6: Commit the candidate contract and validator**

```bash
git add scripts/build_synty_race_catalog.py scripts/test_build_synty_race_catalog.py library/synty-subscription/race-candidates.json
git commit -m 'feat: define Synty race candidate catalog (#73)'
```

---

### Task 3: Selectively Materialize Candidate Sources

**Files:**
- Create: `scripts/materialize_synty_race_catalog.py`
- Create: `scripts/test_materialize_synty_race_catalog.py`

**Interfaces:**
- Consumes Task 1 member records and Task 2 reviewed/seeded candidates.
- Produces `MaterializedCandidate`, `materialize_member(source_root: Path, member: dict[str, object], destination: Path) -> Path`, and `materialize_catalog(source_root: Path, index: dict[str, object], catalog: dict[str, object], stage_root: Path, candidate_ids: set[str] | None) -> tuple[MaterializedCandidate, ...]`.
- Task 4 consumes `.stage/synty-race-catalog/materialized.json` with top-level `{schemaVersion: 1, candidates: [...]}`; each record contains `candidateId`, `sourcePaths`, `texturePaths`, `sourceReadiness`, and `objectNames`, and all paths are stage-relative.

- [ ] **Step 1: Write failing ZIP/Unity safety and selectivity tests**

```python
class MaterializeTests(unittest.TestCase):
    def test_extracts_only_referenced_body_and_texture_with_source_paths_preserved(self):
        result = materialize_catalog(self.source, self.index(), self.catalog(), self.stage, None)
        files = sorted(path.relative_to(self.stage).as_posix() for path in self.stage.rglob("*") if path.is_file())
        self.assertIn("candidates/hero/source/Characters/SK_Chr_Hero_01.fbx", files)
        self.assertIn("candidates/hero/textures/Textures/Character_A.png", files)
        self.assertNotIn("Props/SM_Prop_Chair_01.fbx", "\n".join(files))
        self.assertEqual("candidates/hero/source/Characters/SK_Chr_Hero_01.fbx", result[0].source_paths[0].as_posix())

    def test_materializes_unity_logical_asset_from_container_member(self):
        output = materialize_member(self.source, self.unity_member(), self.stage / "unity")
        self.assertEqual("Characters.fbx", output.name)
        self.assertEqual(b"combined", output.read_bytes())
```

Also snapshot every source archive's size/mtime/hash before and after materialization and require exact equality.

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_materialize_synty_race_catalog -v
```

Expected: FAIL with missing module.

- [ ] **Step 3: Implement safe exact-member extraction**

Use the inventory record rather than searching by basename:

```python
@dataclass(frozen=True)
class MaterializedCandidate:
    candidate_id: str
    source_paths: tuple[PurePosixPath, ...]
    texture_paths: tuple[PurePosixPath, ...]
    source_readiness: str
    object_names: tuple[str, ...]


def safe_destination(root: Path, relative: str) -> Path:
    path = PurePosixPath(relative)
    if path.is_absolute() or any(part in {"", ".", ".."} for part in path.parts):
        raise CatalogError(f"unsafe materialized path: {relative!r}")
    return root.joinpath(*path.parts)
```

For ZIPs, open the exact indexed `containerMember`; for Unity packages, open the exact indexed `GUID/asset`. Stream bytes to a temporary sibling and `os.replace`. Refuse symlink destinations, duplicate output paths, missing archive/member/hash, and stage roots inside `library/`, `harness/`, or `evidence/`. Recreate only `.stage/synty-race-catalog` when invoked with `--clean`.

- [ ] **Step 4: Add refusal tests and make them pass**

Cover traversal, absolute logical paths, a changed archive hash, missing member, two candidates targeting one output, output symlink, an unreferenced texture, and a stage root inside tracked provider paths.

Run:

```bash
python3 -m unittest scripts.test_materialize_synty_race_catalog -v
```

Expected: PASS.

- [ ] **Step 5: Materialize the initial individual and combined smoke inputs**

Add reviewed render profiles for the exact inventory IDs discovered by Task 1 for:

- Fantasy Kingdom `SK_Chr_Peasant_Male_01.fbx` plus its character atlas;
- Dark Fantasy `Characters.fbx` plus its character atlas.

Then run:

```bash
: "${SYNTY_SUBSCRIPTION_ROOT:?}"
python3 scripts/materialize_synty_race_catalog.py \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT" \
  --index library/synty-subscription/index.json \
  --catalog library/synty-subscription/race-candidates.json \
  --stage-root .stage/synty-race-catalog \
  --candidate-id polygon-fantasy-kingdom--sk-chr-peasant-male-01 \
  --candidate-id polygon-dark-fantasy--characters \
  --clean
```

Expected: only the two sources and their profile textures exist under `.stage`; `git status --short` shows no staged source bytes.

- [ ] **Step 6: Commit the materializer and smoke-profile declarations**

```bash
git add scripts/materialize_synty_race_catalog.py scripts/test_materialize_synty_race_catalog.py library/synty-subscription/race-candidates.json
git commit -m 'feat: materialize selected Synty race candidates (#73)'
```

---

### Task 4: Inspect and Render Individual and Combined Candidates

**Files:**
- Create: `scripts/render_synty_race_catalog.py`
- Create: `scripts/test_render_synty_race_catalog.py`
- Create during real run: `evidence/73-synty-race-catalog/inspection.json`
- Create during real run: `evidence/73-synty-race-catalog/renders/*.png`

**Interfaces:**
- Consumes `.stage/synty-race-catalog/materialized.json` and Task 2 catalog records.
- Produces `RenderJob`, `build_render_jobs(materialized: dict[str, object], catalog: dict[str, object], out_dir: Path) -> tuple[RenderJob, ...]`, `rig_fingerprint(armature) -> str`, `enumerate_combined(job) -> list[dict[str, object]]`, and one inspection record per candidate.
- Task 5 consumes `inspection.json` with top-level `{schemaVersion: 1, candidates: [...]}`; each candidate record has exact keys `candidateId`, `status`, `sourceReadiness`, `meshes`, `armatures`, `dimensionsMeters`, `rigFingerprint`, `frontPath`, `threeQuarterPath`, `warnings`, and `error`.

- [ ] **Step 1: Write failing pure contract tests**

Keep `bpy` imports inside Blender-only functions so these tests run under normal Python:

```python
class RenderContractTests(unittest.TestCase):
    def test_builds_two_views_and_keeps_combined_object_selection(self):
        jobs = build_render_jobs(self.materialized(), self.catalog(), self.out)
        self.assertEqual(("front", "three-quarter"), jobs[0].views)
        self.assertEqual(("SK_Chr_Demon_01",), jobs[1].object_names)
        self.assertTrue(str(jobs[0].source_path).startswith(str(self.stage)))

    def test_missing_materialized_source_becomes_a_job_error_not_an_exception(self):
        jobs = build_render_jobs(self.materialized(missing=True), self.catalog(), self.out)
        self.assertEqual("missing materialized source", jobs[0].preflight_error)
```

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_render_synty_race_catalog -v
```

Expected: FAIL with missing module.

- [ ] **Step 3: Implement job parsing, scene inspection, and neutral rendering**

Reuse the neutral camera/lighting conventions from `scripts/render_neutral_views.py` and the label/framing conventions from `scripts/render_prop_contact_sheet.py`; do not modify those runtime-oriented scripts.

For each job:

1. reset to an empty Blender scene;
2. import the FBX with Blender 5's `bpy.ops.wm.fbx_import` fallback;
3. for combined sources, list mesh/armature names in `--enumerate-combined` mode, then retain only curated `objectNames` plus their armature dependencies in render mode;
4. apply the render profile's extracted character atlas to imported mesh materials when source material links are unresolved;
5. compute world-space dimensions and a SHA-256 fingerprint of ordered bone names plus parent names;
6. render 640×640 front and three-quarter PNGs under neutral lighting;
7. append a record with `status: "rendered"`, or catch the candidate-local exception and append `status: "error"` while continuing.

The CLI accepts `--catalog`, `--materialized`, `--stage-root`, `--out-dir`, optional repeated `--candidate-id`, and `--enumerate-combined`. It writes `inspection.json` through a temporary sibling after all jobs finish.

- [ ] **Step 4: Add contract/refusal tests and make them pass**

Cover unsafe output paths, output outside the issue evidence directory, duplicate candidate IDs, missing textures, combined jobs without object names, individual jobs with object names, and deterministic render-job ordering. Tests assert error records, not screenshot bytes.

```bash
python3 -m unittest scripts.test_render_synty_race_catalog -v
```

Expected: PASS.

- [ ] **Step 5: Run the required real Blender smoke**

First enumerate the combined Dark Fantasy source and write its chosen internal body name into `race-candidates.json`. Then render both smoke candidates:

```bash
blender --background --python scripts/render_synty_race_catalog.py -- \
  --catalog library/synty-subscription/race-candidates.json \
  --materialized .stage/synty-race-catalog/materialized.json \
  --stage-root .stage/synty-race-catalog \
  --out-dir evidence/73-synty-race-catalog \
  --enumerate-combined \
  --candidate-id polygon-dark-fantasy--characters

blender --background --python scripts/render_synty_race_catalog.py -- \
  --catalog library/synty-subscription/race-candidates.json \
  --materialized .stage/synty-race-catalog/materialized.json \
  --stage-root .stage/synty-race-catalog \
  --out-dir evidence/73-synty-race-catalog \
  --candidate-id polygon-fantasy-kingdom--sk-chr-peasant-male-01 \
  --candidate-id polygon-dark-fantasy--characters
```

Expected: each candidate has front/three-quarter PNGs and an inspection success record; if the combined candidate cannot be separated honestly, it instead has a visible error record and the individual smoke still succeeds.

- [ ] **Step 6: Commit renderer, tests, and smoke evidence**

```bash
git add scripts/render_synty_race_catalog.py scripts/test_render_synty_race_catalog.py library/synty-subscription/race-candidates.json evidence/73-synty-race-catalog
git commit -m 'feat: inspect Synty race candidates in Blender (#73)'
```

---

### Task 5: Generate Pack Sheets and the Classic-Nine Coverage Sheet

**Files:**
- Create: `scripts/build_synty_race_gallery.py`
- Create: `scripts/test_build_synty_race_gallery.py`
- Create during real run: `evidence/73-synty-race-catalog/packs/*.png`
- Create during real run: `evidence/73-synty-race-catalog/classic-nine.png`
- Create during real run: `evidence/73-synty-race-catalog/README.md`

**Interfaces:**
- Consumes Task 2 coverage data and Task 4 inspection/render records.
- Produces `build_pack_sheets(catalog, inspection, out_dir) -> list[Path]`, `build_classic_nine_sheet(catalog, inspection, out_path) -> Path`, and `build_gallery_readme(catalog, inspection) -> str`.

- [ ] **Step 1: Write failing synthetic-image gallery tests**

Use Pillow to create tiny red/blue fixture renders; no licensed assets enter tests.

```python
class RaceGalleryTests(unittest.TestCase):
    def test_pack_sheet_labels_candidate_source_mapping_and_effort(self):
        outputs = build_pack_sheets(self.catalog(), self.inspection(), self.out)
        self.assertEqual([self.out / "packs" / "fantasy-kingdom.png"], outputs)
        image = Image.open(outputs[0])
        self.assertGreaterEqual(image.width, 800)
        self.assertGreaterEqual(image.height, 400)

    def test_classic_nine_sheet_keeps_dragonborn_gap_and_render_error_card(self):
        path = build_classic_nine_sheet(self.catalog(), self.inspection(with_error=True), self.out / "classic-nine.png")
        self.assertTrue(path.is_file())
        readme = build_gallery_readme(self.catalog(), self.inspection(with_error=True))
        self.assertIn("Dragonborn — gap", readme)
        self.assertIn("render failed", readme)
        self.assertIn("Candidate only—not game canon", readme)
```

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_build_synty_race_gallery -v
```

Expected: FAIL with missing module.

- [ ] **Step 3: Implement deterministic card and sheet layout**

Use fixed 320×420 cards with front and three-quarter panels, source display name, pack, possible mappings, adaptation effort, readiness, and rig-family short ID. Missing/failed renders use a high-contrast error card containing the portable candidate ID and error summary. Pack sheets sort by `(neutralVisualGroup, sourceDisplayName, inventoryId)`; classic-nine rows follow `CLASSIC_NINE` exactly but do not assert JSON key order.

The CLI accepts `--catalog`, `--inspection`, and `--out-dir`. It removes/rebuilds only `packs/*.png`, `classic-nine.png`, and generated README content; candidate renders remain Task 4 output.

- [ ] **Step 4: Add honest-gap and portability tests and make them pass**

Cover multiple candidates for one race, one candidate mapped to several races, `Other:*`, all five coverage statuses, no candidate for a race, no absolute path in README, long labels, and deterministic output dimensions/order without requiring deterministic PNG hashes.

```bash
python3 -m unittest scripts.test_build_synty_race_gallery -v
```

Expected: PASS.

- [ ] **Step 5: Generate the smoke gallery and commit**

```bash
python3 scripts/build_synty_race_gallery.py \
  --catalog library/synty-subscription/race-candidates.json \
  --inspection evidence/73-synty-race-catalog/inspection.json \
  --out-dir evidence/73-synty-race-catalog

git add scripts/build_synty_race_gallery.py scripts/test_build_synty_race_gallery.py evidence/73-synty-race-catalog
git commit -m 'feat: build Synty race candidate gallery (#73)'
```

Expected: Fantasy Kingdom/Dark Fantasy pack sheets, `classic-nine.png`, and README exist; unreviewed races appear as gaps rather than invented matches.

---

### Task 6: Complete the Real Catalog, Obtain Kirk's Verdict, and Publish Once

**Files:**
- Modify: `library/synty-subscription/race-candidates.json`
- Modify: `evidence/73-synty-race-catalog/inspection.json`
- Create/Modify: `evidence/73-synty-race-catalog/renders/*.png`
- Create/Modify: `evidence/73-synty-race-catalog/packs/*.png`
- Modify: `evidence/73-synty-race-catalog/classic-nine.png`
- Modify: `evidence/73-synty-race-catalog/README.md`
- Modify: `README.md`

**Interfaces:**
- Consumes the complete pipeline from Tasks 1–5 and Kirk's visual review.
- Produces the reviewed discovery catalog that enables the later nine-model production goal.

- [ ] **Step 1: Curate every generated complete-body nomination**

For each individual-body nomination, choose exactly one metadata outcome:

- keep it in `candidates` with a neutral visual group, zero or more provisional mappings, bounded adaptations, honest effort, render profile, and `reviewStatus: "metadata-reviewed"`; or
- remove it from `candidates` and add `{inventoryId, reason}` to `rejections`, using a concrete reason such as engine duplicate, accessory-only, corrupt import, or not a complete body.

Add one render profile per actual character texture/import family, not per model. Combined-source stubs remain `unreviewed` until Step 2 enumerates them. Run the normal `check` command after editing; it permits those explicit stubs but still rejects malformed references.

Expected: PASS with every individual nomination metadata-classified and no forced race mappings; combined stubs remain visible and `--require-reviewed` remains RED.

- [ ] **Step 2: Materialize and render in feedback-first pack order**

Process Fantasy Kingdom, Fantasy Characters, Adventure, Elven Realm, Dark Fantasy, Dungeons Realms, Pirates, Dungeon creatures, Boss Zombies/Werewolves, then combined-only packs. For each combined source, enumerate it, replace the source stub with one metadata-reviewed candidate per recoverable complete body using `candidateId` suffix `--{object-name-slug}` and exact `objectNames`, or reject the source with a concrete blocker. After each pack, regenerate its contact sheet so Kirk can inspect while later packs run. Use the same commands from Tasks 3–5 without creating new issues, branches, PRs, or review rounds.

Expected: every metadata-reviewed individual body has two renders or an inspection error; every combined source is replaced by rendered/error candidates or a concrete rejection; Modular Fantasy Hero and Sidekick have no materialized files.

- [ ] **Step 3: Present the gallery and record Kirk's verdict**

Ask Kirk to review `evidence/73-synty-race-catalog/classic-nine.png` and pack sheets. Record the verdict verbatim in the evidence README, update candidate mappings/adaptation notes only where the verdict changes the visual judgment, and mark every retained rendered/error candidate `visual-reviewed`. Regenerate coverage once, then run `check --require-reviewed`.

Expected: the sheet identifies likely options and honest gaps for Human, Elf, Dwarf, Halfling, Dragonborn, Gnome, Half-Elf, Half-Orc, and Tiefling; every nomination is visually reviewed or concretely rejected; the catalog does not declare canonical races or promote assets.

- [ ] **Step 4: Document repeatable commands and boundaries**

Add `library/synty-subscription/` to the root README layout. Document environment setup and exact scan/check/materialize/render/gallery commands using `SYNTY_SUBSCRIPTION_ROOT`, explain that `.stage/` is disposable, and state that the next goal chooses and productionizes nine candidates by rig family.

The evidence README must include inventory summary, gallery interpretation, failed/blocked candidates, deferred modular systems, current classic-nine coverage, commands, and Kirk's verdict. It must not contain source-root absolute paths or raw archive contents.

- [ ] **Step 5: Run the one final verification gate**

```bash
python3 -m unittest scripts.test_index_synty_subscription -v
python3 -m unittest scripts.test_build_synty_race_catalog -v
python3 -m unittest scripts.test_materialize_synty_race_catalog -v
python3 -m unittest scripts.test_render_synty_race_catalog -v
python3 -m unittest scripts.test_build_synty_race_gallery -v
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/index_synty_subscription.py --source-root "$SYNTY_SUBSCRIPTION_ROOT" --check
python3 scripts/build_synty_race_catalog.py check --index library/synty-subscription/index.json --catalog library/synty-subscription/race-candidates.json --require-reviewed
git diff --check
if rg -n '/home/|/tmp/|Downloads/synty' library/synty-subscription evidence/73-synty-race-catalog README.md; then exit 1; fi
if git status --porcelain | grep -E '\.(fbx|blend|unitypackage|zip)$'; then exit 1; fi
```

Expected: focused and full tests pass with only established skips; generated index/candidate checks pass; no private absolute paths or raw licensed source files are tracked.

- [ ] **Step 6: Commit the reviewed catalog once**

```bash
git add README.md library/synty-subscription evidence/73-synty-race-catalog
git commit -m 'docs(evidence): catalog Synty race candidates (#73)'
```

- [ ] **Step 7: Perform one final whole-branch review**

Review `origin/main...HEAD` once for archive-path safety, license leakage, factual/generated separation, candidate reference integrity, conservative race language, Blender failure isolation, modular deferral, and the absence of runtime promotion. Fix only valid Critical/Important findings on the same branch, rerun Step 5 once, and do not introduce task-level review loops.

- [ ] **Step 8: Push and open the provider PR**

```bash
candidate_count="$(jq '.candidates | length' library/synty-subscription/race-candidates.json)"
rendered_count="$(jq '[.candidates[] | select(.status == "rendered")] | length' evidence/73-synty-race-catalog/inspection.json)"
error_count="$(jq '[.candidates[] | select(.status == "error")] | length' evidence/73-synty-race-catalog/inspection.json)"
cat > /tmp/rpg-game-assets-73-pr.md <<EOF
## Summary

- index all 34 current private Synty subscription archives without bulk extraction
- classify individual bodies, combined sources, accessories, animations, modular systems, and unknowns without flattening source identity
- provide ${candidate_count} reviewed race candidates with ${rendered_count} rendered and ${error_count} visibly failed
- generate pack galleries and the noncanonical classic-nine coverage sheet
- defer Modular Fantasy Hero and Unreal-only Sidekick Modular

## Evidence

- evidence/73-synty-race-catalog/classic-nine.png
- evidence/73-synty-race-catalog/README.md
- full Python provider suite plus real ZIP/Unity scan and Blender individual/combined smoke
- Kirk's gallery verdict is recorded verbatim in the evidence README

## Non-goals

No runtime GLB promotion, canonical race decision, customization UI, animation binding, or web sync.

Closes #73

— assets agent, on behalf of KirkDiggler
EOF

git push -u origin asset/73-synty-race-catalog
gh pr create \
  --repo KirkDiggler/rpg-game-assets \
  --base main \
  --head asset/73-synty-race-catalog \
  --title 'asset: index Synty subscription race candidates' \
  --body-file /tmp/rpg-game-assets-73-pr.md

gh project item-edit \
  --id PVTI_lAHOAASbwc4Bcj4vzg4NEnc \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id 9dac2cae
```

Read the PR and board fields back after creation. Do not merge; Kirk owns merge approval.

- [ ] **Step 9: Close the design record after implementation merges**

After GitHub proves the provider PR merged, update PR `rpg-project#297` with the merged provider commit and final gallery path. Kirk then merges #297 last; Project automation closes #296. Remove only the two worktrees/branches created for #296/#73 after their merges are proven.
