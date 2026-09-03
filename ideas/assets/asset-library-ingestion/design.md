# Simple Asset Source Tools Design

Status: approved by Kirk

Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)

Slice: [rpg-project#366](https://github.com/KirkDiggler/rpg-project/issues/366)

## Purpose

Give asset collaborators a few small tools for finding and preparing licensed source assets. This is an internal workshop workflow, not a production service.

The tools index source archives, search safe metadata, extract explicitly selected files, and convert selected FBX candidates with the converter we already use.

## Human workflow

```text
source archives
  -> index
  -> search
  -> select model and texture IDs
  -> extract a candidate folder
  -> convert the selected candidate
  -> move useful candidates to the next human workflow
```

The next workflow is Blender placement. Kirk can open a character and item, place or pose them, and save the scene. Blender placement, hand-socket capture, provider promotion, runtime mapping, and animation are not part of these source tools.

## Source boundary

- The default licensed drop folder is `~/Downloads/synty/`.
- Source archives are never modified or deleted.
- Every ZIP or Unity package in the configured folder is indexed.
- Sidekick or Unreal packages remain visible as `deferred-unreal-export` when encountered.
- Unsupported model formats remain searchable; converters are added only when a selected asset needs one.
- The tracked catalog contains names, sizes, formats, and SHA-256 identities, but no source bytes or absolute machine paths.

## Tools

### Build the catalog

`build_asset_source_catalog.py` reads ZIP and Unity package metadata and writes deterministic JSON. Archive and member identities are content hashes. Unsafe archives are recorded as invalid without hiding their siblings.

### Search the catalog

`search_asset_source_catalog.py` performs simple all-term matching with an optional extension filter. It returns stable archive and member IDs.

### Extract candidates

`extract_asset_candidates.py` consumes a small human/agent-authored selection file. It rechecks archive and member hashes, extracts only the selected model and support files, and creates a fresh candidate folder. It refuses to overwrite an existing candidate.

### Convert candidates

`convert_asset_candidates.py` delegates FBX conversion to the existing `fbx_to_glb.py`, using one selected texture and an existing pack scale profile. Existing GLBs may pass through. Other formats return a clear unsupported error until a real candidate justifies another adapter.

## Candidate folder

```text
candidate-name/
  candidate.json
  source/
  support/
  model.glb          # after conversion
  conversion.json   # after conversion
```

The folder and its receipts are the workflow state. V1 has no database, daemon, lock manager, event graph, release transaction, rollback service, or resumable orchestration engine.

## Repository ownership

- `rpg-game-assets` owns the scripts, generated private catalog, tests, and source-tool reference.
- `game-dev` later receives a small wrapper and ignored candidate-workspace convention.
- `rpg-project` owns this design and, after the commands exist, a short Agent Skills-standard guide that links to repository documentation.

## Testing

Public tests use generated tiny ZIP, Unity package, FBX, texture, and fake-Blender fixtures. They prove:

- deterministic complete indexing;
- deferred Unreal visibility;
- Unity logical paths;
- simple metadata search;
- archive/member hash verification;
- selective extraction into a fresh folder;
- refusal to overwrite candidates;
- invocation of the existing FBX converter; and
- portable receipts without local paths.

A private check rebuilds the catalog from the configured Synty folder and compares exact bytes. Existing asset-provider tests remain unchanged.

## Execution guardrail

Journey #365 does not use spawned subagents or autonomous review/fix loops. Work happens inline in one bounded session. An independent review, when requested, is a separately initiated top-level session at the final PR head.

If a tool needs a state machine, concurrency protocol, transaction framework, or more than a few focused scripts, implementation stops and returns to Kirk before adding that machinery.

## Completion

This slice is complete when an agent can rebuild the shared private catalog, search it, extract a selected candidate and texture, and convert that candidate without reading prior conversation history or processing every model in every pack.
