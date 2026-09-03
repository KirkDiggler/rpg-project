# Content-Addressed Asset Library Ingestion and Catalog Design

Status: approved in conversation; pending repository review  
Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)  
Slice: [rpg-project#366](https://github.com/KirkDiggler/rpg-project/issues/366)

## Purpose

Create a repeatable, agent-first asset-library foundation that indexes every licensed source archive and materializes only selected candidates. The same catalog must support held items, NPCs, props, environments, and later asset workflows without requiring an agent to reconstruct prior conversations.

This is the first focused subsystem beneath the broader repeatable asset-delivery Journey. It ends at provenance-bound converted candidates. Blender placement, held-item provider promotion, runtime exact-ref mapping, and animation authoring are separate downstream processes.

## Why this comes first

The current workflow has strong one-wave provider validation but weak source-library ingestion:

- the canonical source drop contains 34 archives, while prior weapon-wave indexes cover only 21;
- the current ingest entry point defaults to the parent Downloads directory rather than the nested Synty drop;
- a nonempty extraction or conversion directory is treated as complete without binding it to an archive hash;
- source indexes are weapon-wave-specific rather than a shared catalog;
- extraction and conversion state is inferred from folders instead of receipts; and
- agents repeatedly spend reasoning tokens rediscovering source structure, coordinating scripts, and repairing stale metadata.

The durable correction is not a larger prompt. It is a content-addressed catalog, selective materialization, deterministic receipts, and a small shared command surface.

## Goals

1. Account for every archive in configured licensed-source roots without silently skipping unsupported formats.
2. Make pack and member metadata searchable by any authorized collaborator's agent.
3. Keep source archives immutable and identify them by content hash.
4. Extract and convert only explicitly selected candidates.
5. Make every candidate reproducible from a stable member identity and processing recipe.
6. Resume safely after interruption without repeating completed hashing or conversion.
7. Preserve rejections, surprises, failures, and recoveries as structured retrospective evidence.
8. Work through the same repository commands from Claude Code, Codex, and Pi.
9. Keep licensed bytes, private source contents, and machine-specific absolute paths out of public repositories.
10. Reduce premium-model usage by assigning deterministic work to scripts and routine catalog operation to economical agents.

## Non-goals

This subsystem does not:

- create or manipulate Blender placement scenes;
- require Blender MCP or any agent-specific remote-control protocol;
- choose final visual candidates on Kirk's behalf;
- capture hand placement or pose authority;
- promote provider assets or edit runtime manifests;
- generate web exact-ref catalogs;
- author, retarget, or validate gameplay animations;
- implement Auto-Rig Pro workflows;
- export Sidekick content from Unreal; or
- build speculative adapters for formats no selected candidate requires.

## Relationship to downstream work

The complete Journey is intentionally decomposed:

1. **Asset Library Ingestion and Catalog** — this design: index, search, select, materialize.
2. **Placement authoring** — a minimal character-and-item Blender scene; the human places or poses it and saves the result.
3. **Placement capture and held-item delivery** — extract approved hand-local authority, bake static provider assets, validate supported sockets, and publish exact-ref mappings.
4. **Animation delivery** — a separate future workflow for animation packs, Auto-Rig Pro, use actions, flexible items, projectiles, and support-hand behavior.

A converted candidate advances to placement through an explicit folder transition. The ingestion subsystem remains useful even when a candidate is destined for an NPC, prop, environment, or another workflow.

## Core decisions

### Immutable source, reproducible cache

Licensed source archives are immutable inputs. The system never deletes, renames, or modifies files in a configured source root.

Archive identity is the SHA-256 of the archive bytes. A changed archive is a new archive version rather than an in-place mutation. Extraction trees, conversions, thumbnails, and search accelerators are reproducible caches. They may be verified, repaired, or rebuilt from the immutable archive plus tracked processing authority.

### Index everything, convert selections

Indexing and conversion are independent.

The indexer inventories every archive and every member it can safely inspect. Unsupported or deferred formats remain visible. Conversion occurs only after an explicit batch selection references stable member IDs.

Optional background previews may be added later, but complete eager conversion is not an entry requirement.

### Agent-first, harness-neutral

Every collaborator is expected to work through an agent, but correctness must not depend on a particular harness or model. Claude Code, Codex, and Pi invoke the same command-line interface and consume the same JSON status.

The commands, schemas, and receipts are authoritative. Agent prose is not workflow state.

### Human-readable folder handoff

Candidate advancement is physically visible in the local workbench:

```text
catalog -> converted-candidates -> placement-inbox -> downstream placement process
```

Moving a candidate folder into `placement-inbox/` is the explicit human decision to advance it. The next subsystem validates that handoff before constructing or opening any Blender scene.

## Repository ownership

### `game-dev`

`game-dev` owns local licensed-source and conversion orchestration:

- the portable `scripts/asset-library` entry point;
- configured source-root discovery, initially `~/Downloads/synty/`;
- ignored machine-local archive locators;
- ignored extraction and conversion caches;
- ignored batch workbenches; and
- workspace/dependency checks.

The local layout is gitignored:

```text
assets/synty/asset-library/
  local-locations.json
  cache/
  workbench/
    batches/<batch-id>/
      selection.json
      converted-candidates/
      placement-inbox/
      rejected/
      events.jsonl
```

### Private `rpg-game-assets`

`rpg-game-assets` owns durable asset authority:

- catalog and receipt schemas;
- archive/member indexing and validation;
- format-specific safe extraction adapters;
- deterministic candidate conversion and validation;
- tracked private source metadata;
- sanitized batch receipts and retrospectives; and
- public synthetic and private licensed verification.

The canonical metadata catalog lives at:

```text
library/source-catalog/v1.json
```

It contains metadata and hashes, never archive bytes or absolute local paths.

### `rpg-project`

`rpg-project` owns:

- this cross-repository design and its implementation plan;
- Journey and Slice truth on Project 19; and
- a concise, reviewed Agent Skills-standard procedure after the executable workflow exists.

The skill points to repository-owned commands and documentation. It does not copy AGENTS policy, catalog data, current batch state, or implementation details.

## Data model

### Archive identity

A catalog archive record contains:

- schema version;
- archive SHA-256 identity;
- filename and canonical pack slug;
- known aliases;
- byte size and container format;
- acquisition category, such as owned or subscription;
- deterministic capability status;
- member count and extension summary;
- predecessor/successor version relationships when known; and
- a deterministic digest of its sorted member records.

Capability statuses include:

- `ready` — members can be inspected and supported members can be materialized;
- `deferred-unreal-export` — retained for a future Unreal export workflow;
- `unsupported` — safely recognized but no current adapter exists; and
- `invalid` — malformed or unsafe input with a recorded reason.

Sidekick Unreal packages are not failures. They are indexed as `deferred-unreal-export`. A future Unreal export will produce a derivative whose receipt points back to the original archive identity and records export-tool authority.

### Member identity

A stable member identity is derived from:

```text
archive SHA-256 + normalized archive-relative member path + member SHA-256
```

A member record contains:

- stable member ID;
- normalized archive-relative path;
- basename and extension;
- decompressed byte size and SHA-256;
- deterministic search tokens;
- technical capability status;
- discoverable support relationships, such as texture or material companions; and
- optional curated semantic tags stored separately from raw scan facts.

Agent guesses never rewrite source facts. An agent may propose that a bottle model is a potion candidate, but that classification remains a curated overlay until accepted.

### Local locator

An ignored local locator maps archive identities to machine paths and last-observed filesystem facts. Before reading member bytes, the operation verifies that the local archive matches its catalog identity.

Absolute paths never enter the tracked catalog, candidate receipts, evidence, or agent-facing portable output.

### Batch selection

A batch has a stable ID and a selection manifest. Selection entries reference member IDs rather than absolute paths or mutable filenames. They may include an intended downstream role, but the catalog itself is not restricted to weapons or held items.

### Candidate receipt

Each successful candidate folder contains:

```text
<candidate-id>/
  candidate.json
  model.glb
  support/
  preview.png       # optional
  receipt.json
```

The receipt binds:

- archive and member identities;
- every extracted support member;
- converter adapter and version;
- deterministic processing options;
- output paths, sizes, and hashes;
- validation results; and
- sanitized warnings or surprises.

A complete matching receipt is the only evidence that a cache entry is reusable.

## Command surface

The collaborator-facing command is:

```text
scripts/asset-library
```

V1 commands are:

```text
asset-library scan
asset-library index
asset-library search <terms>
asset-library batch create <name>
asset-library batch select <batch> <member-id>...
asset-library batch materialize <batch>
asset-library batch status <batch>
asset-library verify
```

Every command:

- is idempotent;
- supports concise human output and `--json` output;
- has stable documented exit codes;
- writes detailed diagnostics to local logs rather than standard output;
- refuses ambiguous or unsafe state;
- acquires an exclusive mutation lock where required; and
- never commits, pushes, opens a PR, or publishes automatically.

### Scan

`scan` discovers files under configured roots, computes or reuses verified archive identities, and updates the ignored local locator. It reports new, changed, missing, recognized-deferred, and invalid archives explicitly.

### Index

`index` safely inventories new archive identities. It reads container metadata and hashes member bytes without extracting a persistent source tree. Existing archive identities are reused. Tracked catalog output is deterministically sorted and contains no generated timestamps.

### Search

`search` queries names, paths, extensions, pack slugs, deterministic tokens, capability, and curated tags. It returns stable IDs and safe metadata only. A generated local search accelerator may be used, but the tracked JSON catalog remains canonical.

### Select

`batch select` records explicit stable member IDs. It does not extract or convert anything.

### Materialize

For each selected member independently, `batch materialize`:

1. verifies the archive identity;
2. resolves explicitly declared or deterministically discovered dependencies;
3. extracts only those members into an external temporary stage;
4. rejects traversal paths, unsafe links, ambiguous dependencies, and unsupported conversion requests;
5. runs the selected format adapter;
6. validates the candidate output;
7. emits a complete candidate receipt; and
8. atomically moves the validated candidate folder into `converted-candidates/`.

One candidate failure does not prevent independent selections from completing.

### Status and verify

`batch status` derives progress from manifests, receipts, and folder state. It does not trust folder existence alone.

`verify` checks catalog determinism, local availability, candidate receipt closure, duplicate or conflicting workflow states, and privacy boundaries without changing state.

## Format boundary

Indexing is broader than conversion.

V1 indexing recognizes:

- ZIP containers and all safely enumerable members;
- Unity package metadata and logical asset paths where reconstructable;
- Sidekick Unreal archives as explicit deferred inputs; and
- unknown containers as visible unsupported records rather than silent omissions.

V1 materialization supports:

1. FBX plus identified texture/material support members;
2. existing GLB through strict validation rather than needless reconversion; and
3. OBJ as a static fallback when no preferable FBX exists.

Maya, Unreal, Unity-specific binary assets, and other formats receive focused adapters only when an actual selected candidate requires them.

## Resumability and concurrency

- Archive/member hashing is reused only when bound to the same archive identity.
- Candidate conversion is reused only when the complete receipt matches current input and processing authority.
- A filesystem lock prevents concurrent catalog or batch mutation.
- Read-only searches may run concurrently.
- Interrupted temporary stages are never mistaken for complete candidates.
- Restart begins at the first incomplete or invalid selection.
- Manual folder movement is permitted, but duplicate presence across incompatible states is an explicit validation error.

## Failures and retrospective records

Every rejection, surprise, failure, and recovery is recorded as a structured event with:

- batch and candidate identity;
- lifecycle stage;
- outcome category;
- portable reason code;
- sanitized details;
- recovery action when applicable; and
- related receipt or evidence locator.

Local `events.jsonl` may contain operational detail but must remain ignored. Closing a batch emits sanitized private `batch-receipt.json` and `retrospective.json` records suitable for future search.

Examples include alternate archive naming, missing atlases, unexpected object structure, failed conversion, rejected candidates, and corrected processing profiles.

## Security and licensing

The implementation must:

- never modify or delete source archives;
- reject archive traversal and unsafe links before extraction;
- stage outside canonical output paths;
- reject path escape through aliases or normalization;
- never serialize home directories, temporary paths, drive-qualified paths, or `file://` URLs into portable artifacts;
- avoid printing licensed member contents;
- keep source and converted candidate bytes ignored in `game-dev`;
- commit source metadata only to the private asset repository; and
- preserve the existing rule that public web repositories track no licensed GLBs.

Catalog metadata is not a license grant. A collaborator must possess authorized local source bytes whose hash matches the catalog before materialization can run.

## Determinism

Given identical archive bytes, schemas, and processing authority:

- catalog generation is byte-stable;
- record and token ordering is stable;
- no timestamps or absolute paths affect tracked output;
- member and dependency closure is exact;
- candidate conversion is byte-stable where the selected adapter promises determinism; and
- receipts identify any adapter that cannot yet meet that guarantee rather than claiming false reproducibility.

## Verification strategy

Public tests use small generated archives and synthetic geometry. Licensed assets are never test fixtures in a public repository.

### Public tests

Cover:

- nested configured source roots;
- archive aliases and alternate source filename patterns;
- changed archive identities;
- deterministic member ordering and tokenization;
- ZIP traversal, links, malformed containers, and decompression limits;
- local-path privacy rejection;
- complete and partial candidate receipts;
- idempotent unchanged reruns;
- independent candidate failure;
- FBX/GLB/OBJ adapter dispatch using synthetic fixtures; and
- duplicate folder-state rejection.

### Private verification

Cover:

- all currently available archives are accounted for;
- the Nature archive's alternate source filename is recognized;
- Sidekick archives are explicit deferred records;
- each indexed member belongs to exactly one archive identity;
- tracked catalog regeneration is byte-identical;
- an unchanged refresh performs no extraction or conversion;
- selected real FBX and OBJ candidates materialize with complete provenance;
- existing archive and candidate caches detect corruption or partial state; and
- portable artifacts contain no private paths or licensed bytes.

## Completion criteria

The subsystem is complete when:

1. all available source archives are represented by explicit catalog records;
2. catalog regeneration and verification pass from authorized local source bytes;
3. an economical read-only agent can search the shared catalog without opening archives itself;
4. selected stable member IDs materialize into validated candidate folders with complete receipts;
5. unchanged work resumes without redundant extraction or conversion;
6. failures and rejections remain visible without blocking independent candidates;
7. the folder handoff to `placement-inbox/` is validated and documented;
8. public synthetic and private licensed checks pass;
9. collaborators can operate the workflow from Claude Code, Codex, or Pi through repository-owned commands; and
10. one independent current-head review is published on each substantive implementation PR.

## Agent guidance and cost controls

Canonical documentation describes capabilities, not a permanent vendor model list. In the current Pi environment:

- Luna xhigh performs scanning, catalog refresh, search, inventory reconciliation, and status work;
- Terra performs ordinary implementation, known converter-adapter work, and PR preparation; and
- Sol is reserved for novel coordinate, format, or security problems, or an explicitly requested final review.

Normal per-candidate processing runs commands rather than spawning one agent per item. Sessions are scoped to one repository slice. Machine-readable status and receipts replace conversation-history handoffs. Focused tests run while implementing; a full suite runs once before PR readiness unless a broad fix invalidates it.

## Documentation deliverables

Implementation provides:

- concise collaborator setup and folder-flow documentation in `game-dev`;
- catalog, receipt, adapter, and error-code references in `rpg-game-assets`;
- complete `--help` for normal commands; and
- a reviewed Agent Skills-standard `asset-library-ingestion` skill in `rpg-project/.agents/skills/` that points to those commands and references.

The skill becomes canonical only after individual review and approval, as required by the project skills catalog.

## Future slices

After this foundation is working:

1. build the minimal placement-inbox to saved-Blender-scene workflow;
2. capture placed static items relative to declared hand sockets;
3. generalize weapon/off-hand provider authority into held-item authority;
4. generate exact-ref consumer catalogs from merged provider manifests;
5. migrate the paused final-weapon remainder through the new workflow; and
6. separately learn NPC source/rig release contracts and Unreal Sidekick export.
