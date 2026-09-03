# Asset Library Ingestion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a private, content-addressed Synty source catalog that can safely inventory licensed archives, search their metadata, and materialize explicitly selected static candidates through a portable `game-dev` command.

**Architecture:** `rpg-game-assets` supplies the Python schemas, archive readers, conversion adapters, receipts, batch transaction, and private catalog; it is the only repository that tracks durable asset authority. `game-dev` provides the ignored machine-local workbench and a thin shell wrapper, while `rpg-project` adds a short shared skill only after both executable repositories have merged. Archive bytes remain immutable local inputs; every cache result is reusable only when a complete deterministic receipt closes over the source identities and recipe.

**Tech Stack:** Python 3 standard library (`argparse`, `dataclasses`, `fcntl`, `hashlib`, `json`, `pathlib`, `tarfile`, `unittest`, `zipfile`), POSIX shell, headless Blender subprocesses, GLB binary parsing, JSON Schema draft 2020-12 documents, and GitHub pull requests.

**Spec:** `ideas/assets/asset-library-ingestion/design.md`

## Global Constraints

- The provider repository is the private `rpg-game-assets` repository; `game-dev` and all public repositories keep both source archives and converted candidate bytes untracked.
- V1 inventories ZIP archives and Unity package metadata, records Unreal/Sidekick inputs as `deferred-unreal-export`, and makes other containers visible as `unsupported` rather than omitting them.
- V1 materializes FBX with declared texture support, validates existing static GLB, and converts static OBJ; it does not add adapters for Maya, Unreal, Unity binary assets, animation, or arbitrary formats.
- No Blender placement: do not create a Blender placement scene, manipulate placement transforms, promote an asset to a runtime tree, edit a runtime manifest, or create a web catalog in this wave. Blender is permitted only as a background FBX/OBJ converter.
- Reject member traversal, duplicate normalized names, links, drive-qualified paths, URI paths, archive/member size-limit violations, decompression-ratio violations, and every source/output ancestor symlink before bytes are read or published.
- Use these V1 limits: 4 GiB archive file, 100,000 members, 1 GiB decompressed member, 8 GiB total decompressed archive data, 1,000:1 per-member compression ratio, 1,024 UTF-8 bytes per member name, 64 support members per selection, and a 64-character lowercase SHA-256 digest. The compression bound exceeds the measured current-source maximum of 726:1 while remaining finite.
- Portable catalog records, candidate receipts, private batch receipts, event summaries, JSON command output, and documentation must never contain an absolute path, home-directory prefix, temporary path, drive path, `file:` URI, or licensed member payload.
- Canonical JSON is UTF-8, two-space indented, key-sorted, newline-terminated, and has no generated timestamp. Lists have an explicitly documented sort key.
- Mutating `scan`, `index`, `batch create`, `batch select`, `batch materialize`, and private receipt sealing acquire the exclusive lock. `search`, `batch status`, `verify`, `--help`, and every `--check`/`--dry-run` path are read-only and never acquire it.
- One existing Project 19 Build issue and one fresh branch are prerequisites for each implementation repository. Do not create a branch until its issue is on the board; do not invent an issue number in this plan.
- This is one wave: keep one branch and one PR in `rpg-game-assets`, then one dependent branch and one PR in `game-dev`. PR-only integration is mandatory: merge reviewed PRs with `rpg-game-assets` before `game-dev`, then update and merge the existing `rpg-project#366` design PR last.
- Use one fresh Terra execution session for each implementation repository and the final `rpg-project` documentation update. Do not use an agent per archive, candidate, or checklist item. After Terra implements the deterministic provider workflow, use one Luna xhigh session for the initial private scan/index/materialization acceptance and for ordinary future catalog refreshes; Luna mutates catalog state only through the tested CLI, never by editing generated JSON.
- Each substantive PR receives exactly one independent review of its current head by a fresh read-only reviewer. The published verdict names the reviewed SHA, scope, commands considered, Critical/Important/Minor counts, and disposition. A material head change requires a new current-head verdict.

---

## Repository map and stable interfaces

### Files created or changed

| Repository | Path | Responsibility |
|---|---|---|
| `rpg-game-assets` | `scripts/asset_library_catalog.py` | Portable-path validation, catalog/receipt schema validation, canonical encoding, member identity, and pure metadata search. |
| `rpg-game-assets` | `scripts/asset_library_archives.py` | Source-root scan locator, safe ZIP/Unity inspection, format classification, limits, and deterministic archive/member records. |
| `rpg-game-assets` | `scripts/asset_library_conversion.py` | Static GLB validation, isolated FBX/OBJ adapter invocation, support closure validation, candidate recipe/receipt construction, and candidate-directory publication. |
| `rpg-game-assets` | `scripts/asset_library.py` | CLI parsing, flock lifecycle, batch manifests, status/verify, JSON/human presentation, and stable exit codes. |
| `rpg-game-assets` | `scripts/obj_to_glb.py` | Narrow background Blender OBJ-to-GLB script used by the required static OBJ fixture and the static-OBJ adapter. |
| `rpg-game-assets` | `scripts/test_asset_library_catalog.py` | Synthetic pure schema, privacy, canonicalization, identity, and search tests. |
| `rpg-game-assets` | `scripts/test_asset_library_archives.py` | Synthetic ZIP, Unity package, changed-locator, unsafe-member, and archive-limit tests. |
| `rpg-game-assets` | `scripts/test_asset_library_workflow.py` | Synthetic adapters, lock, receipt, restart, batch-state, privacy, and CLI tests. |
| `rpg-game-assets` | `scripts/test_asset_library_private.py` | Opt-in private licensed acceptance checks; it skips only when `ASSET_LIBRARY_PRIVATE_ROOT` is unset. |
| `rpg-game-assets` | `library/source-catalog/v1.schema.json` | Versioned JSON Schema for the portable source catalog. |
| `rpg-game-assets` | `library/source-catalog/curation-v1.json` | Hand-authored portable input for archive slug/alias/acquisition metadata and accepted member tags. |
| `rpg-game-assets` | `library/source-catalog/receipt-v1.schema.json` | Versioned JSON Schema shared by `receipt.json` and `batch-receipt.json`. |
| `rpg-game-assets` | `library/source-catalog/error-codes-v1.json` | Stable portable outcome-code registry used by events and diagnostics. |
| `rpg-game-assets` | `library/source-catalog/README.md` | Catalog, receipt, adapter, and error-code reference without local-machine instructions. |
| `rpg-game-assets` | `library/source-catalog/v1.json` | Generated, tracked, private metadata catalog; never contains archive bytes or local locators. |
| `rpg-game-assets` | `library/source-catalog/batches/private-smoke/{batch-receipt.json,retrospective.json}` | Sanitized, deterministic private evidence produced by the named private acceptance batch. |
| `rpg-game-assets` | `README.md` | Collaborator-facing private catalog and receipt runbook, CLI links, and format boundary. |
| `game-dev` | `scripts/asset-library` | Portable shell entry point that selects the sibling private repository, ignored local workspace, default nested source root, and provider CLI. |
| `game-dev` | `scripts/ingest-assets.sh` | Deprecation shim only; retains non-mutating dry-run discovery while delegating operational work to `scripts/asset-library`. |
| `game-dev` | `scripts/test_asset_library.sh` | Shell-level wrapper and legacy-dry-run contract test using a temporary fake provider. |
| `game-dev` | `.gitignore` | Explicit ignored locator, cache, workbench, and local batch paths under the already-ignored `assets/` tree. |
| `game-dev` | `CLAUDE.md`, `README.md` | Nested `~/Downloads/synty/` setup, folder handoff, normal commands, and licensed-byte boundary. |
| `rpg-project` | `.agents/skills/asset-library-ingestion/SKILL.md` | Approved, progressive-disclosure shared procedure that links to the two repository-owned references and commands. |
| `rpg-project` | `.agents/skills/README.md` | Catalog index updated after the individual skill review accepts the first canonical skill. |

### Provider interfaces

Keep these signatures exact. All public metadata return values use only Python primitives and portable strings.

```python
# scripts/asset_library_catalog.py
from pathlib import Path, PurePosixPath
from typing import Sequence

portable_path(value: object, label: str) -> PurePosixPath
member_id(archive_sha256: str, member_path: str, member_sha256: str) -> str
canonical_json(value: object) -> bytes
load_catalog(path: Path) -> dict[str, object]
search_catalog(catalog: dict[str, object], terms: Sequence[str]) -> list[dict[str, object]]

# scripts/asset_library_archives.py
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

@dataclass(frozen=True)
class ArchiveLimits:
    max_archive_bytes: int = 4 * 1024 ** 3
    max_members: int = 100_000
    max_member_bytes: int = 1024 ** 3
    max_total_member_bytes: int = 8 * 1024 ** 3
    max_compression_ratio: int = 1_000
    max_member_name_bytes: int = 1_024

discover_catalog_inputs(root: Path) -> list[dict[str, object]]
scan_roots(roots: Sequence[Path], locator_path: Path) -> dict[str, object]
index_archive(path: Path, archive_sha256: str, *, limits: ArchiveLimits) -> dict[str, object]
safe_member_name(name: str) -> str
index_zip(path: Path, archive_sha256: str, *, limits: ArchiveLimits) -> dict[str, object]
index_unitypackage(path: Path, archive_sha256: str, *, limits: ArchiveLimits) -> dict[str, object]

# scripts/asset_library_conversion.py
from pathlib import Path

convert_fbx(source: Path, output_dir: Path, *, pack: str, atlas: Path, blender: str = "blender") -> Path
convert_obj(source: Path, output_dir: Path, *, pack: str, atlas: Path | None, blender: str = "blender") -> Path
validate_static_glb(path: Path) -> dict[str, object]

# scripts/asset_library.py
from collections.abc import Iterator, Sequence
from pathlib import Path

create_batch(workbench: Path, name: str) -> dict[str, object]
select_batch(batch: Path, member_ids: Sequence[str]) -> dict[str, object]
materialize_batch(batch: Path, catalog: dict[str, object], workspace: Path, *, blender: str = "blender") -> dict[str, object]
status_batch(batch: Path) -> dict[str, object]
verify_state(catalog: Path, batch: Path | None, workspace: Path) -> list[str]
mutation_lock(lock_path: Path) -> Iterator[None]
```

`member_id()` validates all three digest inputs, canonicalizes `member_path` through `portable_path()`, and returns `sha256(archive_sha256 + b"\0" + member_path.encode("utf-8") + b"\0" + member_sha256)`. A candidate ID is `sha256(member_id + "\0" + adapter-script-sha256 + "\0" + canonical-json-options-sha256)`. This makes recipe changes intentionally produce a different cache directory.

The generated catalog top level is exactly `schemaVersion`, `curationSha256`, `archives`, and `curatedTags`. Archives sort by `sha256`; members sort by normalized `path`; token arrays and support-member-ID arrays sort bytewise. An archive record includes `sha256`, `filename`, `packSlug`, `aliases`, `sizeBytes`, `containerFormat`, `acquisitionCategory`, `capability`, `memberCount`, `extensions`, `memberTreeSha256`, `predecessorSha256`, `successorSha256`, and `members`. A member includes `id`, `path`, `basename`, `extension`, `sizeBytes`, `sha256`, `tokens`, `capability`, and `discoveredSupportMemberIds`.

`library/source-catalog/curation-v1.json` is the non-generated input for facts that archive bytes cannot determine. It has exactly `schemaVersion`, `archives`, and `memberTags`; archive entries are keyed by archive SHA-256 and may set `packSlug`, sorted `aliases`, and `acquisitionCategory` (`owned`, `subscription`, or `unknown`), while member tags are keyed by stable member ID. A fresh catalog build consumes this file and binds its SHA-256 as `curationSha256`; it never preserves hand-edited fields from an older generated catalog.

A candidate `receipt.json` and `candidate.json` contain only schema version, candidate/member/archive IDs, sorted dependency closure, adapter path relative to the provider repository plus its SHA-256, canonical options, output records, static validation facts, warning codes, and event IDs. A receipt is complete only when its file records exactly match the candidate directory and every source/dependency identity still matches the selected catalog records. `model.glb` is required, `support/` is optional, and `preview.png` is optional.

The CLI has these stable commands and exit codes:

```text
asset-library scan --root ROOT [--root ROOT] [--dry-run] [--check] [--json]
asset-library index [--check] [--json]
asset-library search TERMS [--extension EXT] [--capability STATUS] [--json]
asset-library batch create NAME [--json]
asset-library batch select BATCH MEMBER_ID [--support-member MEMBER_ID] [--role ROLE] [--json]
asset-library batch materialize BATCH [--seal-private] [--blender EXECUTABLE] [--json]
asset-library batch status BATCH [--json]
asset-library verify [--batch BATCH] [--json]
```

`--json` is accepted both before and after the subcommand and emits one newline-terminated portable JSON object on stdout. Diagnostics go to stderr and the ignored `cache/logs/` tree. Exit `0` means the requested operation is valid and complete, `1` is an unexpected internal failure, `2` is CLI/schema misuse, `3` is a security rejection, `4` is unavailable or hash-drifted input, `5` means a batch reached terminal state with at least one rejected or failed independent selection, and `6` means `--check` or `verify` found stale or conflicting state.

### Waves, issue, branch, and review protocol

1. Before each repository session, the operator selects an existing Project 19 Build issue for that repository and confirms it is board-backed. Store the real selected number in `ISSUE` and the harness-created isolated checkout in `ASSET_WORKTREE`, `GAME_DEV_WORKTREE`, or `PROJECT_WORKTREE` as applicable; this plan intentionally does not choose issue numbers or machine-specific worktree paths.
2. In the implementation repository, fetch and create one branch from its actual base: `git fetch origin && git checkout -b "feat/${ISSUE}-asset-library-ingestion" origin/main`. Keep all work for that repository’s portion of this wave on that branch, including fixes found by its reviewer.
3. The `rpg-game-assets` Terra session completes Tasks 1–4. One Luna xhigh session then executes Task 5's deterministic private catalog refresh and acceptance through the CLI on the same branch. Terra prepares the one asset-library PR. Its independent reviewer reads the final diff and considers the provider gates on that exact head. The reviewer publishes the final verdict on the PR; the author addresses valid findings on the same branch and obtains a replacement current-head verdict only if the resulting head is materially different.
4. Only after the provider PR merges to `main` does a fresh Terra session begin the `game-dev` branch for Task 6. Its independent reviewer follows the same final-current-head protocol.
5. Only after both provider and wrapper PRs are merged does a fresh Terra session add Task 7’s skill to the already-open `rpg-project#366` design PR. It does not open a second design PR. A fresh read-only reviewer validates that skill on its final head and publishes the verdict before the design PR is merged.
6. Luna receives the existing board-backed worktree and operates only through `asset-library` commands for scan, index, search, selection, materialization, status, and verification. It may commit deterministic generated catalog/receipt outputs when Task 5 explicitly requests that result, but it never hand-edits hashes or schemas, changes source archives, pushes, opens PRs, or merges. Future catalog-only refreshes may use the same bounded pattern.

**Merge order:** (1) merge the reviewed `rpg-game-assets` provider PR to `main`; (2) merge the reviewed dependent `game-dev` wrapper PR to `main`; (3) after both are present, merge the reviewed existing `rpg-project#366` design PR containing the shared skill. No implementation branch merges directly or out of this order.

## Tasks

### Task 1: Provider catalog primitives and versioned schemas

**Repository/session:** `rpg-game-assets`, first fresh Terra session, one provider branch for the whole provider wave.

**Files:**
- Create: `scripts/asset_library_catalog.py`
- Create: `scripts/test_asset_library_catalog.py`
- Create: `library/source-catalog/v1.schema.json`
- Create: `library/source-catalog/curation-v1.json`
- Create: `library/source-catalog/receipt-v1.schema.json`
- Create: `library/source-catalog/error-codes-v1.json`
- Create: `library/source-catalog/README.md`
- Modify: `README.md`
- Reuse without modification: `scripts/index_weapon_source_archives.py` and `scripts/test_index_weapon_source_archives.py` as the isolated weapon-query compatibility boundary.

**Consumes:** No new provider module. Preserve the existing `canonical_json()` behavior in `scripts/index_weapon_source_archives.py`; do not import it or change its sorted-query output contract.

**Produces:** The catalog functions listed in the interface section; schema IDs `rpg-game-assets/source-catalog/v1` and `rpg-game-assets/candidate-receipt/v1`; error codes `E_ARCHIVE_PATH_UNSAFE`, `E_ARCHIVE_LIMIT`, `E_ARCHIVE_UNSUPPORTED`, `E_ARCHIVE_DEFERRED_UNREAL`, `E_LOCATOR_HASH_DRIFT`, `E_DEPENDENCY_AMBIGUOUS`, `E_DEPENDENCY_UNRESOLVED`, `E_PACK_PROFILE_REQUIRED`, `E_CONVERSION_FAILED`, `E_RECEIPT_INCOMPLETE`, `E_CACHE_CONFLICT`, and `E_PRIVATE_PATH`.

- [ ] **Step 1: Write failing catalog and privacy tests.**

```python
# scripts/test_asset_library_catalog.py
import unittest
from asset_library_catalog import canonical_json, member_id, portable_path, search_catalog


class CatalogTests(unittest.TestCase):
    def test_member_identity_is_path_sensitive_and_deterministic(self) -> None:
        archive = "a" * 64
        payload = "b" * 64
        self.assertEqual(member_id(archive, "Models/A.fbx", payload), member_id(archive, "Models/A.fbx", payload))
        self.assertNotEqual(member_id(archive, "Models/A.fbx", payload), member_id(archive, "Models/B.fbx", payload))

    def test_portable_path_rejects_machine_and_escape_spellings(self) -> None:
        for unsafe in ("../x", "/private/x", r"C:\\x", "file:///private/x", "Models//A.fbx", "Models/../A.fbx"):
            with self.subTest(unsafe=unsafe), self.assertRaisesRegex(ValueError, "portable|non-canonical|drive"):
                portable_path(unsafe, "member")

    def test_search_requires_all_casefolded_terms_and_returns_safe_metadata(self) -> None:
        results = search_catalog(SYNTHETIC_CATALOG, ["pirate", "sabre"])
        self.assertEqual(["c" * 64], [row["memberId"] for row in results])
        self.assertNotIn("locator", results[0])
        self.assertNotIn("/private", canonical_json(results).decode("utf-8"))
```

Define `SYNTHETIC_CATALOG` immediately above the class with two closed archive records, 64-hex stable member IDs (the sabre uses `"c" * 64`), case variants, curated tags, and an intentionally unsafe string used only as a rejection input. Do not add `pytest`.

- [ ] **Step 2: Run the new tests before implementation.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_catalog -v`

Expected: FAIL with `ModuleNotFoundError: No module named 'asset_library_catalog'`.

- [ ] **Step 3: Implement the minimal pure catalog module.**

```python
# scripts/asset_library_catalog.py
_SHA256 = re.compile(r"^[0-9a-f]{64}$")


def portable_path(value: object, label: str) -> PurePosixPath:
    if not isinstance(value, str) or not value or "\\" in value or value.startswith(("/", "file:")):
        raise ValueError(f"{label}: portable relative path required")
    if re.match(r"^[A-Za-z]:", value):
        raise ValueError(f"{label}: drive-qualified path forbidden")
    path = PurePosixPath(value)
    if any(part in ("", ".", "..") for part in path.parts) or path.as_posix() != value:
        raise ValueError(f"{label}: non-canonical path")
    return path


def member_id(archive_sha256: str, member_path: str, member_sha256: str) -> str:
    for label, digest in (("archive", archive_sha256), ("member", member_sha256)):
        if not isinstance(digest, str) or _SHA256.fullmatch(digest) is None:
            raise ValueError(f"{label}: lowercase SHA-256 required")
    canonical_path = portable_path(member_path, "member").as_posix()
    return hashlib.sha256(
        archive_sha256.encode("ascii") + b"\0" + canonical_path.encode("utf-8") + b"\0" + member_sha256.encode("ascii")
    ).hexdigest()
```

Implement `canonical_json()` with `json.dumps(value, indent=2, ensure_ascii=False, sort_keys=True) + "\n"`. `load_catalog()` validates the complete top-level schema shape before returning it. `search_catalog()` lower-case-tokenizes input with `re.findall(r"[a-z0-9]+", text.casefold())`, requires every requested token, searches pack slug, alias, path, extension, capability, raw tokens, and curated tags, and returns stably sorted safe rows with `memberId`, `archiveSha256`, `packSlug`, `path`, `extension`, `capability`, and sorted `discoveredSupportMemberIds` only.

- [ ] **Step 4: Add the catalog, receipt, and error-code schema documents.**

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "rpg-game-assets/source-catalog/v1",
  "type": "object",
  "required": ["schemaVersion", "curationSha256", "archives", "curatedTags"],
  "additionalProperties": false,
  "properties": {
    "schemaVersion": {"const": 1},
    "curationSha256": {"type": "string", "pattern": "^[0-9a-f]{64}$"},
    "archives": {"type": "array"},
    "curatedTags": {"type": "object"}
  }
}
```

Make nested archive/member definitions closed (`additionalProperties: false`) and require every field named in the interface section. Add a canonical empty `curation-v1.json` (`schemaVersion: 1`, empty `archives` and `memberTags`) and tests proving only its closed portable shape can supply aliases, acquisition category, pack-slug overrides, and curated member tags. The receipt schema requires `schemaVersion`, `candidateId`, `memberId`, `archiveSha256`, `dependencies`, `adapter`, `options`, `outputs`, `validation`, `warnings`, and `events`; it forbids `sourcePath`, `locator`, `timestamp`, and `uri` keys by omission. The error-code registry is a sorted JSON object whose values each have `category`, `retryable`, and a portable human message; include `E_PACK_PROFILE_REQUIRED` for a selected static source whose pack has no accepted scale profile.

- [ ] **Step 5: Document the narrow provider contract.**

In `library/source-catalog/README.md`, document the schema IDs, canonical ordering, error-code meanings, candidate receipt closure rule, and the three V1 materializers. In the provider `README.md`, add a short “Asset library catalog” section that links to this reference and says source archives/candidate bytes stay outside the repository’s tracked `library/source-catalog/` metadata.

- [ ] **Step 6: Run the RED/GREEN and regression gates.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_catalog scripts.test_index_weapon_source_archives -v`

Expected: PASS; the old weapon-query test still reports its existing cases without importing the new catalog module.

- [ ] **Step 7: Commit the focused schema primitive.**

```bash
cd "$ASSET_WORKTREE"
git add scripts/asset_library_catalog.py scripts/test_asset_library_catalog.py \
  library/source-catalog/v1.schema.json library/source-catalog/curation-v1.json \
  library/source-catalog/receipt-v1.schema.json library/source-catalog/error-codes-v1.json \
  library/source-catalog/README.md README.md
git commit -m "feat: add asset library catalog schema"
```

### Task 2: Provider source scan and safe ZIP/Unity indexing

**Repository/session:** Continue the same `rpg-game-assets` Terra session and branch.

**Files:**
- Create: `scripts/asset_library_archives.py`
- Create: `scripts/test_asset_library_archives.py`
- Modify: `scripts/asset_library_catalog.py`
- Reuse without modification: `scripts/synty_pack_profiles.py` only as the authoritative known pack-slug/profile list for later materialization.
- Preserve: `scripts/index_weapon_source_archives.py`, `scripts/test_index_weapon_source_archives.py`, `scripts/test_simple_weapon_candidate_inventory.py`, and `scripts/test_specialist_weapon_candidate_inventory.py`.

**Consumes:** Task 1’s `portable_path()`, `member_id()`, and canonical JSON contract.

**Produces:** `ArchiveLimits`, `safe_member_name()`, `discover_catalog_inputs()`, scan locator schema version 1, archive records with `ready`, `deferred-unreal-export`, `unsupported`, or `invalid` capability, and no persistent extraction tree.

- [ ] **Step 1: Write synthetic archive fixtures and failing tests.**

```python
# scripts/test_asset_library_archives.py
import tempfile
import unittest


def write_zip(path: Path, entries: dict[str, bytes]) -> None:
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name, payload in entries.items():
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            archive.writestr(info, payload)


class ArchiveTests(unittest.TestCase):
    def test_zip_index_is_stable_and_never_extracts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "nested" / "Nature_SourceFiles.zip"
            archive.parent.mkdir()
            write_zip(archive, {"Models/Tree.FBX": b"fbx", "Textures/Tree.png": b"png"})
            digest = sha256_file(archive)
            self.assertEqual(index_archive(archive, digest, limits=ArchiveLimits()), index_archive(archive, digest, limits=ArchiveLimits()))
            self.assertFalse((root / "Models").exists())

    def test_rejects_traversal_links_and_declared_bombs(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixtures = write_unsafe_archives(Path(temporary))
            for name, message, limits in (
                ("traversal", "unsafe member", ArchiveLimits()),
                ("symlink", "symlink", ArchiveLimits()),
                ("ratio", "compression ratio", ArchiveLimits(max_compression_ratio=2)),
            ):
                path = fixtures[name]
                with self.subTest(name=name), self.assertRaisesRegex(ValueError, message):
                    index_archive(path, sha256_file(path), limits=limits)
```

Implement `sha256_file()` and `write_unsafe_archives()` as test-only helpers in this file. Generate, without licensed files: a safe nested ZIP, same-name changed ZIP, malformed bytes with a `.zip` suffix, a `../escape` ZIP, a Unix-symlink ZIP made with `external_attr`, a high-ratio ZIP, an over-count ZIP using a small test limit, a Unity `.unitypackage` gzip tar containing `pathname` metadata and an `asset` payload, a Sidekick archive containing `.uasset`, and an arbitrary unsupported regular file.

- [ ] **Step 2: Run the archive tests to establish RED.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_archives -v`

Expected: FAIL with `ModuleNotFoundError: No module named 'asset_library_archives'`.

- [ ] **Step 3: Implement safe names, limit accounting, and locator reuse.**

```python
# scripts/asset_library_archives.py
@dataclass(frozen=True)
class ArchiveLimits:
    max_archive_bytes: int = 4 * 1024 ** 3
    max_members: int = 100_000
    max_member_bytes: int = 1024 ** 3
    max_total_member_bytes: int = 8 * 1024 ** 3
    max_compression_ratio: int = 1_000
    max_member_name_bytes: int = 1_024


def safe_member_name(name: str) -> str:
    if len(name.encode("utf-8")) > ArchiveLimits().max_member_name_bytes:
        raise ValueError("unsafe member: name exceeds 1024 UTF-8 bytes")
    return portable_path(name, "archive member").as_posix()
```

`scan_roots()` recursively visits real regular files below every real configured root, sorted by root-relative portable path. It records every file, including unrecognized containers, so nothing in a configured source root disappears merely because no adapter exists. The ignored locator may contain `path`, `sizeBytes`, `mtimeNs`, and `sha256`; when size and mtime match a prior locator entry, it may reuse that hash. A new, changed, missing, invalid, and recognized-deferred result is explicit in the summary. The locator is atomically rewritten only for a non-dry-run scan and is never copied into the catalog or JSON stdout.

- [ ] **Step 4: Implement ZIP and Unity metadata indexers without extraction.**

For ZIP, inspect `ZipInfo` records in normalized member-path order; reject directories masquerading as links, any Unix symlink mode, duplicate normalized names, unsafe paths, declared member/total/ratio limit violations, and malformed containers. Hash each decompressed member through a bounded streaming reader rather than `ZipFile.extract()` or `ZipFile.read()` of an unbounded payload.

For Unity packages, use `tarfile.open(path, "r:gz")`, reject `TarInfo.issym()`, `islnk()`, devices, FIFOs, unsafe tar member names, and all limits before reading. Pair a safe `pathname` metadata entry with its same-package `asset` member, use the logical `pathname` where it is reconstructable, and otherwise retain a safe tar-relative member path. Hash member bytes through `extractfile()` without writing a source tree.

A ZIP is `deferred-unreal-export` when its normalized filename or enumerated members identify Unreal/Sidekick content (`unreal`, `sidekick`, or `.uasset`); still record its members and extension summary. A recognized but unimplemented container is one `unsupported` record with a reason code. Malformed or unsafe input is an `invalid` record with a reason code, not an exception that hides other scan entries.

- [ ] **Step 5: Build deterministic archive records and support relationships.**

For every safely indexed member, calculate the member digest and `member_id`. Build tokens from the pack slug, alias, basename, extension, and path components. Discover only proof-backed dependencies: parse a same-archive OBJ’s safe MTL `map_Kd` references, and accept an FBX/OBJ/GLB support relationship only when a selected `--support-member` identity is in the same archive and its extension is a permitted support type (`.png`, `.jpg`, `.jpeg`, `.tga`, `.mtl`). Never select the first texture by filename. A missing or multiple required support relation remains materialization-time `E_DEPENDENCY_UNRESOLVED` or `E_DEPENDENCY_AMBIGUOUS`.

- [ ] **Step 6: Run public scan/index gates.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_catalog scripts.test_asset_library_archives scripts.test_index_weapon_source_archives scripts.test_simple_weapon_candidate_inventory scripts.test_specialist_weapon_candidate_inventory -v`

Expected: PASS; public tests use only generated archives, and the three legacy inventory suites retain their tracked query/inventory contracts.

- [ ] **Step 7: Commit the archive boundary.**

```bash
cd "$ASSET_WORKTREE"
git add scripts/asset_library_archives.py scripts/test_asset_library_archives.py scripts/asset_library_catalog.py
git commit -m "feat: index asset library archives safely"
```

### Task 3: Provider static conversion adapters and validation

**Repository/session:** Continue the same `rpg-game-assets` Terra session and branch.

**Files:**
- Create: `scripts/asset_library_conversion.py`
- Create: `scripts/obj_to_glb.py`
- Modify: `scripts/test_asset_library_workflow.py`
- Reuse without modification: `scripts/synty_pack_profiles.py`, `scripts/weapon_glb.py`, `scripts/promote_weapons.py`, `scripts/verify_web_asset_stage.py`, and `scripts/static_release_transaction.py`.
- Modify only if a test proves import safety is required: `scripts/fbx_to_glb.py`; retain its `convert(fbx_path, texture_path, out_dir, non_skinned_scale)` contract and its existing Blender CLI unchanged, adding only an `if __name__ == "__main__": main()` guard.

**Consumes:** Task 2 safe staged member paths and exact support identities. `PACK_PROFILES` supplies the FBX profile authority. Existing `weapon_glb.inspect_weapon()`, `normalize_weapon_glb()`, `validate_geometry_rewrite()`, and `validate_atlas_rewrite()` are weapon-specific references only; do not apply their weapon material/count assumptions to generic candidates.

**Produces:** Static-only conversion paths and `validate_static_glb()` facts used by receipts.

- [ ] **Step 1: Add failing conversion tests using only synthetic geometry.**

```python
# scripts/test_asset_library_workflow.py
import tempfile
import unittest


class ConversionTests(unittest.TestCase):
    def test_valid_static_glb_returns_hash_and_mesh_facts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            glb = write_triangle_glb(Path(temporary) / "triangle.glb")
            facts = validate_static_glb(glb)
            self.assertEqual(1, facts["triangles"])
            self.assertEqual(3, facts["vertices"])
            self.assertRegex(facts["sha256"], r"^[0-9a-f]{64}$")

    def test_rejects_animated_or_external_static_glb(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with self.assertRaisesRegex(ValueError, "animations forbidden"):
                validate_static_glb(write_animated_glb(root / "animated.glb"))
            with self.assertRaisesRegex(ValueError, "external URI"):
                validate_static_glb(write_external_uri_glb(root / "external.glb"))

    def test_fbx_adapter_surfaces_a_nonzero_blender_exit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source_fbx, atlas_png, failing_blender = write_fbx_adapter_fixtures(Path(temporary))
            with self.assertRaises(subprocess.CalledProcessError):
                convert_fbx(source_fbx, Path(temporary) / "out", pack="dungeon", atlas=atlas_png, blender=str(failing_blender))
```

Implement all named fixture writers in `scripts/test_asset_library_workflow.py`. `write_triangle_glb()` emits one embedded-buffer, indexed triangle. The invalid fixtures separately contain an animation, a skin, `NaN` position data, a non-triangle index count, an external image URI, and an out-of-bounds buffer view. The required OBJ fixture is a triangle OBJ plus a tiny PNG atlas; fake Blender scripts record argv and either emit a deterministic triangle GLB or exit nonzero.

- [ ] **Step 2: Run conversion tests before adapter implementation.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_workflow -v`

Expected: FAIL with `ModuleNotFoundError: No module named 'asset_library_conversion'`.

- [ ] **Step 3: Implement strict static GLB validation.**

`validate_static_glb()` reads the GLB header/chunks directly and rejects bad magic/version/length, a missing JSON or BIN chunk, external buffer/image URI, multiple buffers, nonzero buffer offsets, malformed accessor/buffer-view ranges, non-finite `POSITION` values, unindexed primitives, index counts not divisible by three, missing meshes, `animations`, `skins`, and non-finite node transforms. Finite source-node transforms are reported rather than rejected because provider normalization belongs to a later subsystem. It returns exactly this portable fact shape:

```python
{
    "sha256": sha256_file(path),
    "triangles": 1,
    "vertices": 3,
    "materialCount": 1,
    "textureCount": 0,
    "positionMin": [0.0, 0.0, 0.0],
    "positionMax": [1.0, 1.0, 0.0],
    "static": True,
}
```

Keep this validator focused on generic static candidates. Do not alter `weapon_glb.py` or weaken its existing byte-parity validations.

- [ ] **Step 4: Implement the isolated FBX and OBJ adapters.**

```python
def convert_fbx(source: Path, output_dir: Path, *, pack: str, atlas: Path, blender: str = "blender") -> Path:
    if pack not in PACK_PROFILES:
        raise ValueError(f"unknown FBX pack profile: {pack}")
    output_dir.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [blender, "--background", "--python", str(FBX_TO_GLB), "--", "--pack", pack,
         "--texture", str(atlas), "--out", str(output_dir), str(source)],
        check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )
    output = output_dir / f"{source.stem}.glb"
    validate_static_glb(output)
    return output
```

Run `fbx_to_glb.py` only as a Blender subprocess because its current module executes `main()` on import. Record the adapter script’s repository-relative path and SHA-256 in the receipt. Both FBX and OBJ conversion require an accepted `PACK_PROFILES` entry; otherwise materialization emits `E_PACK_PROFILE_REQUIRED` rather than guessing scale. `obj_to_glb.py` is a background-only script: clear the scene, import one OBJ, apply the selected pack's non-skinned scale, optionally apply the staged atlas, reject armatures/animations, export one GLB, and exit. It has no camera, placement, scene-save, hand socket, or runtime promotion behavior. `convert_obj()` uses it only for a static OBJ plus the required synthetic OBJ fixture coverage; it rejects an OBJ with no resolved MTL/explicit atlas when a texture is required.

- [ ] **Step 5: Add candidate recipe construction and publication primitives.**

Build a sorted dependency list from the selected model plus declared/discovered support IDs, stage exactly those bytes under `workspace/cache/stages/`, and verify each staged member’s hash. Write `candidate.json`, `model.glb`, optional deterministic support files named by the full support member ID followed by its lowercased extension, and `receipt.json` into a fresh sibling stage directory. Validate the GLB and full receipt closure before publishing.

Do not use `static_release_transaction.atomic_apply_targets()` for a candidate directory: it protects file targets but does not prove every output ancestor is a real directory. Instead, walk every ancestor from `workspace` through `workbench/batches/` plus the validated batch ID plus `/converted-candidates`, reject a symlink or non-directory with `lstat()`, `fsync` written files, atomically `os.replace(stage, destination)`, then `fsync` the destination parent. If a destination has a complete matching receipt, reuse it. If it exists but is incomplete or mismatched, atomically quarantine it under `cache/quarantine/`, emit `E_RECEIPT_INCOMPLETE` or `E_CACHE_CONFLICT`, and rebuild; never treat folder presence as success.

- [ ] **Step 6: Run adapter and legacy regressions.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_workflow scripts.test_asset_library_catalog scripts.test_asset_library_archives scripts.test_promote_weapons scripts.test_static_release_transaction -v`

Expected: PASS; the synthetic success candidate has `model.glb` plus a matching receipt, while a failing synthetic adapter is surfaced for Task 4 to record per selection.

- [ ] **Step 7: Commit conversion work.**

```bash
cd "$ASSET_WORKTREE"
git add scripts/asset_library_conversion.py scripts/obj_to_glb.py scripts/test_asset_library_workflow.py
git commit -m "feat: materialize validated asset candidates"
```

### Task 4: Provider batch transaction, CLI, receipts, resume, and verification

**Repository/session:** Continue the same `rpg-game-assets` Terra session and branch.

**Files:**
- Create: `scripts/asset_library.py`
- Modify: `scripts/test_asset_library_workflow.py`
- Modify: `scripts/asset_library_conversion.py`
- Modify: `scripts/asset_library_catalog.py`
- Modify: `library/source-catalog/README.md`
- Modify: `README.md`
- Reuse without modification: `scripts/promote_weapons.py` for sorted `tree_records()`/digest/seal design cues and `scripts/verify_web_asset_stage.py` for `canonical_tree_path()`/sorted inventory cues.

**Consumes:** Tasks 1–3 schemas, archive records, conversion adapters, and candidate receipt closure.

**Produces:** All collaborator commands, local batch state, structured events, deterministic private sealing, lock discipline, status, and verification.

- [ ] **Step 1: Extend tests for batch state, receipt reuse, interruption, and CLI JSON.**

```python
# scripts/test_asset_library_workflow.py
class WorkflowTests(unittest.TestCase):
    def test_resume_requires_a_complete_matching_receipt(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = make_complete_workflow_fixture(Path(temporary))
            first = materialize_batch(fixture.batch, fixture.catalog, fixture.workspace, blender=str(fixture.fake_blender))
            self.assertEqual("complete", first["selections"][0]["state"])
            (fixture.candidate_dir / "receipt.json").write_text("{}\n", encoding="utf-8")
            second = materialize_batch(fixture.batch, fixture.catalog, fixture.workspace, blender=str(fixture.fake_blender))
            self.assertIn("E_RECEIPT_INCOMPLETE", [event["code"] for event in second["events"]])
            self.assertEqual("complete", second["selections"][0]["state"])

    def test_verify_rejects_duplicate_converted_and_placement_inbox_state(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = make_complete_workflow_fixture(Path(temporary), materialized=True)
            shutil.copytree(fixture.candidate_dir, fixture.placement_inbox / fixture.candidate_dir.name)
            errors = verify_state(fixture.catalog_path, fixture.batch, fixture.workspace)
            self.assertIn("E_CACHE_CONFLICT: candidate appears in converted-candidates and placement-inbox", errors)

    def test_search_json_is_one_portable_object(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = make_complete_workflow_fixture(Path(temporary))
            completed = subprocess.run(fixture.cli + ["--workspace", str(fixture.workspace), "search", "sabre", "--json"], text=True, capture_output=True)
            self.assertEqual(0, completed.returncode)
            payload = json.loads(completed.stdout)
            self.assertNotIn(str(fixture.workspace), completed.stdout)
            self.assertEqual("search", payload["command"])
```

Implement `make_complete_workflow_fixture()` as a test-only helper returning a small dataclass with every path and fake executable used above. Add assertions that a second mutating subprocess receives a lock refusal, stale locator hash produces exit `4`, malformed arguments produce exit `2`, unsafe paths produce exit `3`, terminal partial batch results produce exit `5`, and `verify` conflict results produce exit `6`.

- [ ] **Step 2: Run workflow tests to demonstrate RED.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest scripts.test_asset_library_workflow -v`

Expected: FAIL because `scripts/asset_library.py` does not exist and the new batch/CLI symbols are unavailable.

- [ ] **Step 3: Implement lock and atomic local manifests.**

`mutation_lock()` opens `workspace/cache/asset-library.lock`, acquires `fcntl.flock(fd, LOCK_EX | LOCK_NB)`, yields, and always unlocks/closes. Return the stable `E_CACHE_CONFLICT` lock diagnostic on contention; never wait indefinitely. `create_batch()` validates a lowercase 1–64 character `name` matching `[a-z0-9][a-z0-9-]{0,63}`, creates `workbench/batches/name/`, and writes canonical `selection.json` with `schemaVersion: 1`, `batchId: name`, and empty sorted `selections`.

`select_batch()` validates 64-hex member IDs, preserves existing selections, rejects a duplicate model ID or more than 64 support IDs, and atomically writes a canonical selection entry with `memberId`, sorted `supportMemberIds`, and an optional portable `role`. Selection records identity only; it extracts and converts nothing.

- [ ] **Step 4: Implement independent materialization and deterministic events.**

For each sorted selection, rehash the locator’s archive immediately before opening it; a mismatch emits `E_LOCATOR_HASH_DRIFT` and leaves that selection terminally failed without reading member bytes. Resolve every declared support identity and every proven discovered dependency, reject a missing/ambiguous closure, then call the Task 3 adapter in a candidate-specific external stage. Continue to the next selection after any exception.

Append local ignored `events.jsonl` records with a monotonic `sequence`, `batchId`, `candidateId` when available, `stage`, `outcome`, `code`, `details`, and `recovery`. `details` is checked with the same portable-string walker before write. Candidate receipts use the sorted event IDs, not local timestamp/path text. A successful `batch materialize --seal-private` with every selection terminal writes canonical private `library/source-catalog/batches/` plus the validated batch ID plus `/batch-receipt.json` and `retrospective.json`; the latter aggregates sorted outcome-code counts and recovery actions without archive paths, extracted filenames, or operational logs.

- [ ] **Step 5: Implement status, verification, and command parsing.**

`status_batch()` derives each selection state from `selection.json`, exact matching receipts, `converted-candidates/`, `placement-inbox/`, `rejected/`, and events. It does not infer completion from a nonempty directory. `verify_state()` validates catalog schema/determinism, locator availability without writing it, all selected receipt closures, duplicate converted/inbox/rejected states, cache ancestor safety, and portable-artifact strings.

Build `argparse` subparsers for every command in the interface section. `scan --dry-run` reports discovery only; `scan --check` compares discovered locator facts without changing the locator; `index --check` generates expected catalog bytes in memory and compares them with `library/source-catalog/v1.json` without writing. `index` without `--check` deterministically rebuilds archive records from the locator plus `curation-v1.json`, binds the curation digest, and writes no timestamp; generated `v1.json` is never a curation input. `search`, `batch status`, and `verify` remain read-only. No command invokes Git, commits, pushes, opens a PR, or publishes an asset.

- [ ] **Step 6: Add CLI help and privacy checks to the documentation.**

Document every exit code, `--json`, `--check`, the local layout below, explicit `--support-member` selection, failure/recovery event behavior, and the candidate handoff rule. Include this exact layout, marking every `game-dev` path as ignored:

```text
assets/synty/asset-library/
  local-locations.json
  cache/
  workbench/batches/private-smoke/
    selection.json
    converted-candidates/
    placement-inbox/
    rejected/
    events.jsonl
```

Explain that moving a whole complete candidate directory from `converted-candidates/` to `placement-inbox/` is a human decision for a later placement process. This task neither opens Blender placement nor consumes the inbox.

- [ ] **Step 7: Run the complete public provider suite.**

Run: `cd "$ASSET_WORKTREE" && python3 -m unittest discover -s scripts -p 'test_*.py' && python3 scripts/build_synty_complete_inventory.py --check && python3 scripts/verify_web_asset_stage.py --verify-only`

Expected: unit tests PASS with `test_asset_library_private` skipped when its opt-in environment variable is absent; complete inventory reports verified; web-stage verification returns a deterministic JSON report and exit `0`.

- [ ] **Step 8: Commit the executable workflow.**

```bash
cd "$ASSET_WORKTREE"
git add scripts/asset_library.py scripts/asset_library_catalog.py scripts/asset_library_conversion.py \
  scripts/test_asset_library_workflow.py library/source-catalog/README.md README.md
git commit -m "feat: add resumable asset library workflow"
```

### Task 5: Private provider catalog acceptance, receipt sealing, and provider PR

**Repository/session:** Use one Luna xhigh session in the existing `rpg-game-assets` provider branch after Terra completes Tasks 1–4. This is the only task that reads authorized local licensed source bytes. Luna runs the deterministic CLI/tests, records the generated outputs, and commits without pushing; Terra resumes only for PR preparation or a genuine implementation defect.

**Files:**
- Create: `library/source-catalog/v1.json`
- Create: `library/source-catalog/batches/private-smoke/batch-receipt.json`
- Create: `library/source-catalog/batches/private-smoke/retrospective.json`
- Create: `scripts/test_asset_library_private.py`
- Modify: `README.md` only if the observed private acceptance identifies a portable runbook correction.

**Consumes:** Merged code from Tasks 1–4 and the authorized local root `$HOME/Downloads/synty`.

**Produces:** A deterministic private catalog accounting for the available source set, opt-in private checks, and a sealed smoke-batch provenance record. It does not create a runtime asset, public fixture, browser artifact, or placement scene.

- [ ] **Step 1: Write the opt-in private acceptance test before refreshing the catalog.**

```python
# scripts/test_asset_library_private.py
@unittest.skipUnless(os.environ.get("ASSET_LIBRARY_PRIVATE_ROOT"), "set ASSET_LIBRARY_PRIVATE_ROOT for licensed acceptance")
class PrivateCatalogAcceptanceTests(unittest.TestCase):
    def test_available_archives_are_accounted_for_and_portable(self) -> None:
        catalog = load_catalog(REPO_ROOT / "library/source-catalog/v1.json")
        discovered = discover_catalog_inputs(Path(os.environ["ASSET_LIBRARY_PRIVATE_ROOT"]))
        self.assertSetEqual(
            {row["sha256"] for row in discovered},
            {archive["sha256"] for archive in catalog["archives"]},
        )
        self.assertTrue(any("nature" in archive["packSlug"] and archive["aliases"] for archive in catalog["archives"]))
        self.assertEqual(len(catalog["archives"]), len({archive["sha256"] for archive in catalog["archives"]}))
        self.assert_portable_recursive(catalog)
```

`discover_catalog_inputs()` is the production pure discovery helper from `asset_library_archives.py`; it includes recognized archive containers and explicit unsupported/deferred archive candidates but excludes ordinary non-archive notes. Add test methods that compare a regenerated in-memory catalog with the tracked bytes, assert each indexed member belongs to one archive identity, assert a second unchanged scan/index performs no extraction/conversion, inspect the `private-smoke` receipts for complete closure, and assert `candidate.json`, `receipt.json`, batch receipt, and retrospective contain no source root, temporary directory, `file:`, or licensed payload bytes. Deferred Unreal behavior is proven by the public synthetic Sidekick fixture and is required privately only when a configured private root actually contains such a package.

- [ ] **Step 2: Run the private test before the generated catalog exists.**

Run: `cd "$ASSET_WORKTREE" && ASSET_LIBRARY_PRIVATE_ROOT="$HOME/Downloads/synty" python3 -m unittest scripts.test_asset_library_private -v`

Expected: FAIL because `library/source-catalog/v1.json` and the `private-smoke` sealed receipt files do not exist yet.

- [ ] **Step 3: Scan and index the authorized nested source root through the provider CLI.**

```bash
cd "$ASSET_WORKTREE"
WORKSPACE="${ASSET_LIBRARY_WORKSPACE:-$HOME/game-dev/assets/synty/asset-library}"
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json scan --root "$HOME/Downloads/synty" --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json index --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json index --check --json
```

Expected: the first command reports every discoverable archive candidate in the configured root, the catalog's archive-identity set exactly equals that discovery set, and `index --check` exits `0` with byte-identical catalog output. Confirm that the Nature alternate filename is represented by its canonical pack slug plus alias. If a configured root contains Sidekick/Unreal inputs, confirm they use `deferred-unreal-export`, not an error or omission.

- [ ] **Step 4: Seal an explicit non-promoting materialization smoke batch.**

Create exactly the `private-smoke` batch. From safe `search --json` results, use the stable member IDs for one authorized static FBX with one explicit texture support ID and one authorized static OBJ with its exact MTL/atlas closure. The asset owner chooses these smoke inputs for conversion coverage; this is not a visual-final-candidate decision. If an authorized static GLB is present, select it as a third input. If none is present, leave it unselected; Task 3’s synthetic GLB fixture remains the GLB adapter acceptance evidence.

```bash
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json batch create private-smoke --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json search fbx --extension fbx --capability ready --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json search obj --extension obj --capability ready --json
```

The two `batch select` invocations use the exact IDs returned by those preceding safe records and take this formal syntax: `batch select private-smoke MEMBER_ID --support-member SUPPORT_MEMBER_ID --role asset-library-smoke`. Do not use archive paths, filenames, a guessed atlas, or an unreviewed inferred visual selection. Then run:

```bash
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json batch materialize private-smoke --seal-private --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json batch status private-smoke --json
python3 scripts/asset_library.py --workspace "$WORKSPACE" --locator "$WORKSPACE/local-locations.json" --catalog library/source-catalog/v1.json verify --batch private-smoke --json
```

Expected: each independently valid selected input has a candidate directory with a complete hash-bound receipt; one deliberate corrupt-cache rerun is quarantined/rebuilt; a duplicate manual copy into `placement-inbox/` makes `verify` exit `6`; removing that duplicate restores exit `0`. No candidate is promoted to `harness/` or copied to a public repository.

- [ ] **Step 5: Run private, public, determinism, and boundary gates.**

Run: `cd "$ASSET_WORKTREE" && ASSET_LIBRARY_PRIVATE_ROOT="$HOME/Downloads/synty" python3 -m unittest scripts.test_asset_library_private -v && python3 -m unittest discover -s scripts -p 'test_*.py' && python3 scripts/build_synty_complete_inventory.py --check && python3 scripts/verify_web_asset_stage.py --verify-only`

Expected: private acceptance PASS; full public suite PASS; tracked catalog regeneration is byte-identical; existing provider inventory/stage gates stay green; no command reports source or temporary paths in portable output.

- [ ] **Step 6: Commit private metadata and private acceptance evidence.**

```bash
cd "$ASSET_WORKTREE"
git add library/source-catalog/v1.json library/source-catalog/batches/private-smoke \
  scripts/test_asset_library_private.py README.md
git commit -m "feat: catalog licensed asset sources"
```

- [ ] **Step 7: Prepare, review, and merge the one provider PR.**

Run the Task 4 complete provider gate again at the final head. Open one PR for the issue selected in the wave protocol, referencing `rpg-project#366`. A fresh read-only reviewer examines the current provider head, catalog schema/receipt privacy, archive-limit enforcement, direct-CLI behavior, synthetic tests, and private acceptance output. Publish the single final verdict on the PR with its exact SHA and verification evidence. Merge through the PR only after that verdict and required approval; do not locally merge or bypass the PR.

### Task 6: Dependent `game-dev` wrapper, local workflow, ignores, and documentation

**Repository/session:** Start one fresh Terra session only after Task 5’s provider PR is merged to `main`. Use one new board-backed `game-dev` Build issue and one `game-dev` branch for this entire task.

**Files:**
- Create: `scripts/asset-library`
- Create: `scripts/test_asset_library.sh`
- Modify: `scripts/ingest-assets.sh`
- Modify: `.gitignore`
- Modify: `CLAUDE.md`
- Modify: `README.md`

**Consumes:** The merged `rpg-game-assets/scripts/asset_library.py` CLI and `library/source-catalog/README.md` reference.

**Produces:** A harness-neutral command that makes the ignored local workspace explicit and defaults source discovery to `~/Downloads/synty/`, plus a deprecated non-duplicating legacy entry point.

- [ ] **Step 1: Write the wrapper and shim shell tests first.**

```bash
# scripts/test_asset_library.sh
set -euo pipefail
ROOT="$(mktemp -d)"
trap 'rm -rf "$ROOT"' EXIT
mkdir -p "$ROOT/game-dev/scripts" "$ROOT/private/scripts" "$ROOT/home/Downloads/synty"
cat > "$ROOT/private/scripts/asset_library.py" <<'PY'
import json, sys
print(json.dumps(sys.argv[1:]))
PY
cp scripts/asset-library "$ROOT/game-dev/scripts/asset-library"
chmod +x "$ROOT/game-dev/scripts/asset-library"
OUTPUT="$(HOME="$ROOT/home" RPG_GAME_ASSETS_DIR="$ROOT/private" "$ROOT/game-dev/scripts/asset-library" scan --json)"
printf '%s' "$OUTPUT" | grep -F -- 'Downloads/synty'
printf '%s' "$OUTPUT" | grep -F -- '--json'
```

Extend the test to assert an explicit `--root /safe/root` is forwarded unchanged, a missing provider CLI exits nonzero without creating a source/output tree, `git check-ignore` recognizes the four local asset-library paths, and `DOWNLOADS_DIR="$ROOT/home/Downloads/synty" scripts/ingest-assets.sh --dry-run` performs no extraction or conversion.

- [ ] **Step 2: Run the wrapper test to establish RED.**

Run: `cd "$GAME_DEV_WORKTREE" && bash scripts/test_asset_library.sh`

Expected: FAIL because `scripts/asset-library` does not exist.

- [ ] **Step 3: Implement the single wrapper command.**

```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSET_REPO="${RPG_GAME_ASSETS_DIR:-$ROOT/rpg-game-assets}"
WORKSPACE="$ROOT/assets/synty/asset-library"
LOCATOR="$WORKSPACE/local-locations.json"
CATALOG="$ASSET_REPO/library/source-catalog/v1.json"
[ -f "$ASSET_REPO/scripts/asset_library.py" ] || { printf '%s\n' 'asset-library: private rpg-game-assets CLI is unavailable' >&2; exit 4; }
exec python3 "$ASSET_REPO/scripts/asset_library.py" \
  --workspace "$WORKSPACE" --locator "$LOCATOR" --catalog "$CATALOG" "$@"
```

Before `exec`, when the command is `scan` and no `--root` occurs in argv, append `--root "${ASSET_LIBRARY_SOURCE_ROOT:-$HOME/Downloads/synty}"`. Do not expose the resolved workspace/root in `--json` output; the ignored local locator is the only place it may occur. Preserve explicit roots and `--json` exactly. The wrapper must not invoke Git, Blender, or an archive extractor itself.

- [ ] **Step 4: Replace old ingestion behavior with the compatibility shim.**

Keep `scripts/ingest-assets.sh --dry-run` non-mutating and source-root configurable through `DOWNLOADS_DIR`; have it print one deprecation line to stderr and delegate to `scripts/asset-library scan --root "$DOWNLOADS_DIR" --dry-run`. For any non-dry-run invocation, print the same deprecation line plus the explicit next command `scripts/asset-library scan --root "$DOWNLOADS_DIR"`; exit `2` rather than preserving the old extraction/conversion loop. Remove the hard-coded four-profile list, direct `unzip`/`tar`, nonempty-directory completion test, and direct `blender` call so there is no second catalog or conversion workflow.

- [ ] **Step 5: Add precise ignored paths and collaborator documentation.**

Add these explicit rules after `/assets/` in `.gitignore`:

```gitignore
/assets/synty/asset-library/local-locations.json
/assets/synty/asset-library/cache/
/assets/synty/asset-library/workbench/
/assets/synty/asset-library/.asset-library.lock
```

Update `CLAUDE.md` and `README.md` to replace the parent Downloads extraction story with the nested source root, the `scan → index → search → batch select → batch materialize → status → verify` flow, the visible `converted-candidates → placement-inbox` handoff, the location of private catalog authority, and the requirement to use repository commands from Claude Code, Codex, or Pi. Link to the private provider reference for schemas/error codes; do not duplicate them or add a web/runtime path.

- [ ] **Step 6: Run wrapper, legacy, and ignore gates.**

Run: `cd "$GAME_DEV_WORKTREE" && bash scripts/test_asset_library.sh && DOWNLOADS_DIR="$HOME/Downloads/synty" ./scripts/ingest-assets.sh --dry-run && ./scripts/asset-library --help && git check-ignore assets/synty/asset-library/local-locations.json assets/synty/asset-library/cache/logs/scan.log assets/synty/asset-library/workbench/batches/private-smoke/selection.json assets/synty/asset-library/.asset-library.lock`

Expected: wrapper test PASS; legacy dry-run does not extract/convert; help lists all eight command families; every named local artifact is ignored; no licensed file is staged or tracked by `game-dev`.

- [ ] **Step 7: Commit, review, and merge the one dependent wrapper PR.**

```bash
cd "$GAME_DEV_WORKTREE"
git add scripts/asset-library scripts/test_asset_library.sh scripts/ingest-assets.sh .gitignore CLAUDE.md README.md
git commit -m "feat: add asset library workspace workflow"
```

Open one PR for the selected `game-dev` issue, target `main`, and reference the merged provider PR plus `rpg-project#366`. A fresh independent reviewer reads the exact current head and checks wrapper forwarding, ignored local state, deprecated shim behavior, docs, and Task 6 gate output. Publish the final verdict on the PR and merge through the PR only after it is approved.

### Task 7: Reviewed shared Agent Skills-standard procedure on the existing design PR

**Repository/session:** Start one fresh Terra session in the existing `rpg-project#366` worktree only after the Task 5 provider PR and Task 6 wrapper PR are merged. Do not create a new issue, branch, or PR for this documentation addition.

**Files:**
- Create: `.agents/skills/asset-library-ingestion/SKILL.md`
- Modify: `.agents/skills/README.md`

**Consumes:** The merged `game-dev` README command flow, merged `rpg-game-assets` catalog reference, and the approved design at `ideas/assets/asset-library-ingestion/design.md`.

**Produces:** The first individually reviewed canonical skill in the shared catalog. It explains a stable procedure only; it contains no issue number other than its design link, active batch, asset selection, operator/model assignment, team policy, current catalog data, local path, or copied AGENTS rule.

- [ ] **Step 1: Write a failing standard/links validation command.**

```bash
cd "$PROJECT_WORKTREE"
python3 - <<'PY'
from pathlib import Path
path = Path('.agents/skills/asset-library-ingestion/SKILL.md')
assert path.is_file(), 'asset-library skill is missing'
text = path.read_text(encoding='utf-8')
assert text.startswith('---\nname: asset-library-ingestion\ndescription: ')
assert '\n---\n' in text
for required in ('scripts/asset-library', 'game-dev/README.md', 'rpg-game-assets/library/source-catalog/README.md'):
    assert required in text, f'missing reference: {required}'
for forbidden in ('Project 19', 'Terra', 'Luna', 'active.md', '/home/', 'file:', 'private-smoke'):
    assert forbidden not in text, f'non-stable skill content: {forbidden}'
PY
```

Expected: FAIL with `AssertionError: asset-library skill is missing`.

- [ ] **Step 2: Write the minimal progressive-disclosure skill.**

```markdown
---
name: asset-library-ingestion
description: Use when an authorized collaborator needs to index, search, select, materialize, inspect, or verify licensed asset-library candidates.
---

# Asset library ingestion

## Start here

Work from the `game-dev` workspace and read `game-dev/README.md` before running the portable command.

## Procedure

1. Run `scripts/asset-library --help`, then use its `scan`, `index`, `search`, `batch`, and `verify` subcommands for the requested operation.
2. Select only stable member IDs returned by `search --json`; do not treat a search result as runtime approval.
3. Inspect `batch status` and `verify --json` before relying on a candidate receipt or advancing a complete candidate directory to `placement-inbox/`.

## References

- `game-dev/README.md` — local setup, ignored workbench, commands, and folder handoff.
- `rpg-game-assets/library/source-catalog/README.md` — catalog/receipt schemas, adapter boundary, and error codes.
- `ideas/assets/asset-library-ingestion/design.md` — scope and downstream exclusions.
```

Use the exact frontmatter and headings above. Do not create a bundled helper, fixture, reference copy, or a repository-specific duplicate of the provider documentation.

- [ ] **Step 3: Update the catalog index after the skill exists.**

Replace the pilot-only wording in `.agents/skills/README.md` with a short canonical catalog table that names `asset-library-ingestion`, its `SKILL.md` path, and its two repository-owned references. Retain the rule that AGENTS invariants and live state do not belong in skills, and retain the warning that legacy runtime skill directories are not automatically canonical.

- [ ] **Step 4: Run the standard/links validation and repository checks.**

Run the Step 1 Python command again, then run: `cd "$PROJECT_WORKTREE" && git diff --check && git diff -- .agents/skills`

Expected: the Python validation exits `0`; `git diff --check` prints nothing; the diff contains only the catalog README update and the concise new `SKILL.md`.

- [ ] **Step 5: Commit to the existing design branch and publish its one review.**

```bash
cd "$PROJECT_WORKTREE"
git add .agents/skills/README.md .agents/skills/asset-library-ingestion/SKILL.md
git commit -m "docs: add asset library ingestion skill"
```

Update the existing `rpg-project#366` PR rather than creating another PR. A fresh read-only reviewer checks the final head against Agent Skills frontmatter/progressive disclosure, confirms both linked merged documents exist, confirms the skill contains no live/policy/private content, and publishes the required current-head verdict before the existing design PR merges.

## Verification matrix

| Layer | Command or evidence | Expected outcome |
|---|---|---|
| Catalog primitives | `python3 -m unittest scripts.test_asset_library_catalog -v` | Stable IDs, canonical JSON, privacy rejection, and deterministic safe search pass. |
| Archive safety | `python3 -m unittest scripts.test_asset_library_archives -v` | ZIP/Unity synthetic fixtures pass; traversal/link/bomb cases reject; Unreal is deferred and unknown containers remain visible. |
| Materializers | `python3 -m unittest scripts.test_asset_library_workflow -v` | FBX dispatch, static GLB validation, static OBJ fixture, independent failures, receipts, resume, lock, and duplicate-state checks pass. |
| Provider regression | `python3 -m unittest discover -s scripts -p 'test_*.py'` | All public tests pass; private test skips only without its opt-in root. |
| Existing provider boundary | `python3 scripts/build_synty_complete_inventory.py --check && python3 scripts/verify_web_asset_stage.py --verify-only` | Existing runtime inventory/stage contracts remain byte-valid and no new candidate is promoted. |
| Private acceptance | `ASSET_LIBRARY_PRIVATE_ROOT="$HOME/Downloads/synty" python3 -m unittest scripts.test_asset_library_private -v` | Every archive identity discovered in the configured root is accounted for, Nature alias is recognized, any present Sidekick input is deferred, catalog is deterministic, selected FBX/OBJ receipts close, and no portable artifact leaks a local path. |
| Wrapper | `cd "$GAME_DEV_WORKTREE" && bash scripts/test_asset_library.sh && ./scripts/asset-library --help` | Default nested root, explicit-root/JSON forwarding, safe missing-provider failure, and all command families work through the single wrapper. |
| Local hygiene | `git check-ignore` for the four Task 6 paths and `git ls-files assets` | Local locator/cache/workbench/lock are ignored; no licensed byte is tracked in `game-dev`. |
| Shared skill | Task 7 Python validation plus `git diff --check` | Correct Agent Skills frontmatter, stable references, no live/policy/private text, and whitespace-clean docs diff. |
| PR review | Published reviewer comment for each current substantive head | Exactly one independent current-head verdict per provider, wrapper, and design/skill PR, with scope, SHA, counts, evidence, and finding disposition. |

## Rollback and cleanup

1. If a catalog/index change is wrong before the provider PR merges, revert the focused commit on the same provider branch, rerun the full provider gate, and have the reviewer bind a new verdict to the new head. Do not alter source archives or delete a locator/cache to disguise the failure.
2. If materialization fails after staging, retain only the ignored stage/quarantine evidence needed to diagnose the recorded code, then remove that stage with the workflow’s explicit cleanup path after `verify` confirms it is not a complete candidate. The canonical candidate directory is changed only by the ancestor-safe atomic publish; a failed publish leaves the old complete candidate intact.
3. If a batch receipt is corrupt or a candidate is manually present in two incompatible folders, run `verify --batch`; quarantine/rebuild the cache from its immutable archive identity, or remove only the duplicate manual copy after preserving the complete receipt. Never repair by editing hashes, receipt fields, or catalog records by hand.
4. If the private catalog must be rolled back after merge, submit a new provider PR that restores the prior tracked catalog/receipt metadata and leaves the licensed source root untouched. Do not force-push, rewrite merged history, or use a public repository as a recovery store.
5. If the `game-dev` wrapper must be rolled back, submit a new `game-dev` PR restoring the previous shim/documentation behavior while retaining `/assets/` ignores; never use `git clean`, reset, or overwrite the known dirty/untracked workspace.
6. After all three PRs merge in order, remove only ignored temporary stages, test-created fake provider directories, and quarantined cache entries whose receipts/events are already sealed. Keep `local-locations.json`, workbench selections, private catalog metadata, private batch receipts, and retrospectives unless a later reviewed retention decision says otherwise.
