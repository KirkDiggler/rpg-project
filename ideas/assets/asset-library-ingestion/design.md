# Pack-by-Pack Asset Ingestion and Preview Design

Status: approved by Kirk

Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)

Slice: [rpg-project#366](https://github.com/KirkDiggler/rpg-project/issues/366)

## Purpose

Give asset collaborators a small, understandable toolchain for turning licensed Synty source packs into private browsable GLB libraries.

The useful result of ingestion is:

```text
preserved source archive
  -> configured pack conversion
  -> GLBs retaining source directories
  -> one PNG preview per GLB
  -> paginated sheets per source directory and filename family
```

Collaborators can identify assets from PNG sheets without loading every model in Blender. Blender is used only after someone chooses an asset or wants to learn the manual process.

## Pack-by-pack decisions

Each pack is handled separately with Kirk. Before conversion, record:

- whether the pack contains models, animation, 2D art, or deferred Unreal content;
- the correct atlas texture;
- the approved static scale profile;
- source model families to include or exclude; and
- preview camera settings.

These facts live in one reviewed JSON configuration under `scripts/configs/synty-packs/`.

After configuration, conversion is mechanical. New decisions are added only when a family genuinely needs a different atlas or treatment.

## Tools

### `index_synty_archives.py`

Indexes every ZIP and Unity package in `~/Downloads/synty/`. It records archive/member names, source-relative paths, formats, sizes, and hashes in the private catalog. It never modifies or deletes source archives.

### `convert_synty_pack.py`

Runs one configured model family, such as `characters` or `props`. It delegates FBX conversion to the existing `fbx_to_glb.py`, launches Blender with factory settings so personal add-ons cannot affect the run, preserves original source directories, and writes a simple manifest of source and output hashes.

### `fbx_to_glb.py`

The existing converter imports FBX, assigns the selected Synty atlas, preserves rigged characters, applies the pack scale and floor alignment to static objects, and exports GLB.

### `render_glb_previews.py`

Imports converted GLBs in one background Blender process and renders one independently auto-framed orthographic isometric PNG per model. The preview uses a neutral background, soft ground shadow, consistent pack orientation, and the source/rest pose.

### `build_preview_sheets.py`

Builds sheets from reusable preview PNGs. Sheets retain the original source directory, split large directories by filename family such as `SK_Chr`, `SM_Prop`, or `SM_Bld`, paginate at 20 items, and label each tile with source filename, short GLB hash, and measured dimensions.

## Output ownership

Raw archives remain local and are never deleted. Temporary extracted sources may be removed after conversion is verified.

Bulk converted GLBs are reproducible local cache, not Git content. Starting with the Fantasy Kingdom item pass, they live under the ignored path:

```text
library/<pack>/v<version>/.glb-cache/
```

Tracked private discovery output contains only previews, sheets, configurations, and portable manifests:

```text
library/<pack>/v<version>/
  previews/<original source directories>/
  sheets/<original source directories>/
  preview-manifest-<group>.json
  sheet-manifest-<group>.json
```

The 39 already-merged Fantasy Kingdom reference character GLBs remain at their documented paths. Future bulk families use the ignored cache. Licensed collaborators regenerate the same cache from their own source archive and approved pack config. `library/` is for discovery; moving a selected asset into `harness/` remains a separate reviewed promotion.

## Family order

A pack may be processed in useful passes:

1. character models;
2. weapons and items;
3. props;
4. buildings and environment;
5. vehicles and effects.

This avoids pretending one atlas is correct for every asset in a large pack.

## Reference pack

POLYGON Fantasy Kingdom v5 is the first reference:

- profile: `fantasy-kingdom`;
- initial atlas: `PolygonFantasyKingdom_Texture_01_A.png`;
- character reference: 39 `SK_Chr` and `SM_Chr` FBXs produced 39 directory-preserving GLBs, 39 accepted previews, and five sheets;
- item pass: 272 `SM_Item` FBXs produced 272 local cached GLBs, 272 accepted previews, and 75 category sheets;
- item cache: 470 MiB remained ignored while approximately 65 MiB of previews and sheets became shared private discovery output.

The directory-preserving character output fixes four distinct same-named FBX pairs that the old flattened conversion collapsed. Kirk accepted the character previews as **“looks great”** and the complete item pass as **“they all look great.”**

## Human documentation

Verified hands-on recipes live under `docs/human/asset-ingestion/`. The first lesson explains manual FBX-to-GLB conversion in Blender so collaborators can understand what the batch script automates.

## Verification

This workshop tool is verified through real runs rather than a large new test framework:

- rebuild the source catalog and compare exact bytes;
- dry-run a configured group and confirm its selected count;
- run conversion and verify every manifest path, size, and hash;
- reconvert one missing output with `--resume` and confirm byte-identical output;
- render every preview and verify image size/hash;
- build sheets and inspect them visually; and
- confirm no portable manifest contains a local absolute path.

Existing repository tests and runtime inventory checks remain unchanged.

## Execution guardrail

Journey #365 does not use spawned subagents or autonomous review/fix loops. Work is inline and bounded. Independent review, if requested, is a separately initiated top-level session at the final PR head.

If ingestion starts requiring a daemon, database, generalized state machine, concurrency protocol, or production deployment framework, stop and return to Kirk before adding it.

## Out of scope

- selecting the merchant or other final game assets;
- Blender hand placement and posing;
- provider/harness promotion;
- runtime exact-ref mapping;
- animation authoring and Auto-Rig Pro workflows; and
- exporting Sidekick assets from Unreal.
