# Simple Asset Source Tools Implementation Plan

**Goal:** Index all licensed source archives, search their metadata, extract selected candidates, and convert selected FBX files with small repository-owned scripts.

**Spec:** `ideas/assets/asset-library-ingestion/design.md`

## Execution rules

- No spawned subagents.
- No autonomous implementation/review/fix loops.
- Work inline in one bounded session per repository.
- Use test-first development for behavior changes.
- Run focused tests while working and one full repository suite before PR readiness.
- Integration is PR-only.
- Stop before adding a daemon, database, lock manager, transaction framework, event graph, or generalized workflow engine.

## Slice 1 — Private source tools (`rpg-game-assets#123`)

Create:

- `scripts/build_asset_source_catalog.py`
- `scripts/search_asset_source_catalog.py`
- `scripts/extract_asset_candidates.py`
- `scripts/convert_asset_candidates.py`
- `scripts/test_asset_source_tools.py`
- `library/source-catalog/v1.json`
- `library/source-catalog/README.md`

Update `README.md` with one link to the source-tool reference.

### Required behavior

1. Build deterministic catalog metadata for every ZIP and Unity package under a configured root.
2. Record archive/member names, sizes, formats, source-relative paths, and SHA-256 identities without serializing absolute paths.
3. Record unsafe archives as invalid and Unreal/Sidekick archives as deferred when present.
4. Search by all supplied terms and optional extension.
5. Extract only selected member IDs and support IDs after rechecking archive/member hashes.
6. Write each selection into a new candidate folder and refuse overwrite.
7. Convert FBX through the existing `fbx_to_glb.py` with one explicit selected texture and known pack profile.
8. Copy an existing GLB candidate; return a clear unsupported error for other formats.
9. Write `candidate.json` and `conversion.json` receipts containing portable paths and hashes.

### Verification

```bash
python3 -m unittest \
  scripts.test_asset_source_tools \
  scripts.test_index_weapon_source_archives \
  scripts.test_synty_pack_profiles -v

python3 scripts/build_asset_source_catalog.py \
  --archives-root "$HOME/Downloads/synty" \
  --output library/source-catalog/v1.json \
  --check

python3 scripts/search_asset_source_catalog.py \
  --catalog library/source-catalog/v1.json \
  --extension fbx \
  bottle
```

Before PR readiness, run the repository's complete public test suite once and verify existing runtime inventory/stage checks. Confirm the diff contains no runtime promotion or local absolute path.

## Slice 2 — Local wrapper (`game-dev`, after Slice 1 merges)

Create one small `scripts/asset-library` wrapper that supplies:

- the sibling `rpg-game-assets` paths;
- default source root `~/Downloads/synty/`; and
- ignored candidate workspace under `assets/synty/`.

Document the visible flow:

```text
catalog -> converted-candidates -> placement-inbox
```

Do not add orchestration state. Folder movement remains the human handoff.

## Slice 3 — Shared agent guide (`rpg-project`, after both command repositories merge)

Create `.agents/skills/asset-library-ingestion/SKILL.md` using the Agent Skills standard and the `writing-skills` procedure. Keep it short and link to the merged command documentation. It must not contain current batch state, model names, source inventory, local paths, or copied AGENTS policy.

## Merge order

1. `rpg-game-assets` source-tool PR.
2. `game-dev` wrapper PR.
3. Existing `rpg-project#367` design/skill PR.

Each merge requires Kirk's approval. No direct or local merges.
