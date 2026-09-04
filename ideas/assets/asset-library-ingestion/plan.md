# Pack-by-Pack Asset Ingestion Implementation Plan

**Goal:** Index licensed packs and provide a configured conversion, preview, and contact-sheet path one pack family at a time.

**Spec:** `ideas/assets/asset-library-ingestion/design.md`

## Execution rules

- No spawned subagents or autonomous review/fix loops.
- Work inline in one bounded repository session.
- Prefer real pack verification over a new generalized test framework.
- Run existing repository tests once before PR readiness.
- Integration is PR-only.
- Do not add a daemon, database, lock manager, event graph, or state machine.

## Slice 1 — Source index and Fantasy Kingdom reference (`rpg-game-assets#123`)

### Source catalog

Create `scripts/index_synty_archives.py` and generated private `library/source-catalog/v1.json`.

Verify:

```bash
python3 scripts/index_synty_archives.py \
  --archives-root "$HOME/Downloads/synty" \
  --output library/source-catalog/v1.json \
  --check
```

Expected current reference: every configured archive is represented and no absolute local path appears in the catalog.

### Pack configuration

Create `scripts/configs/synty-packs/polygon-fantasy-kingdom.json` with:

- Fantasy Kingdom v5 archive identity;
- `fantasy-kingdom` scale profile;
- `Texture_01_A` default atlas;
- character family include patterns;
- orthographic isometric preview settings; and
- 20-item, five-column sheet pagination.

### Pack conversion

Create `scripts/convert_synty_pack.py` as a small wrapper around existing `fbx_to_glb.py`.

It must:

- select only the requested configured group;
- support `--dry-run` and a simple `--resume` for missing files;
- run Blender with `--factory-startup`;
- preserve original source directories;
- refuse accidental overwrite unless resuming missing output; and
- write source/output paths, sizes, and hashes to `manifest-<group>.json`.

### Preview rendering

Create `scripts/render_glb_previews.py` for Blender background execution.

It renders one 512px orthographic isometric PNG per manifest GLB using the configured camera, neutral background, ground shadow, source/rest pose, and independent bounds-based framing. It writes `preview-manifest-<group>.json`.

### Contact sheets

Create `scripts/build_preview_sheets.py` using Pillow.

It groups previews by original source directory and then filename family, paginates them, labels filename/hash/dimensions, and writes `sheet-manifest-<group>.json`.

### Human documentation

Add:

- `library/source-catalog/README.md` with exact pack commands;
- `docs/human/asset-ingestion/fbx-to-glb.md` explaining manual Blender conversion; and
- links from the existing repository and human-doc indexes.

### Fantasy Kingdom acceptance

Run the `characters` group end to end.

Done when:

- 39 source FBXs produce 39 GLBs;
- original `Source_Files/Characters` and `Source_Files/FBX` paths remain distinct;
- all four duplicate-name pairs survive independently;
- 39 reusable 512px PNGs and five sheets exist;
- Kirk accepts the preview style;
- one factory-startup resume conversion reproduces an existing GLB hash without Auto-Rig Pro handler output;
- manifests match every output path, size, and hash;
- source catalog rebuild and runtime inventory checks pass; and
- portable manifests contain no local path.

### Fantasy Kingdom item acceptance

The merged item pass adds an ignored `.glb-cache/` convention, separates model-cache input from tracked preview output, and groups sheets by the first meaningful filename category.

Verified result:

- 272 item FBXs produced 272 ignored cached GLBs;
- 272 reusable 512px PNGs and 75 category sheets exist;
- 470 MiB of GLBs remain outside Git;
- no cached GLB is tracked;
- preview and sheet manifests match every path and hash;
- the source catalog and runtime inventory remain exact; and
- Kirk accepted the complete item set as **“they all look great.”**

## Later pack-family passes

Add one reviewed group/config decision at a time:

1. Fantasy Kingdom weapons.
2. Fantasy Kingdom props.
3. Fantasy Kingdom buildings and environment.
4. Other packs selected with Kirk.

A family needing another atlas receives a focused configuration override when observed. Do not guess all texture rules in advance.

## Later repositories

After the private tool PR merges:

- add a small `game-dev` wrapper/default local paths if it proves useful;
- add a concise Agent Skills-standard guide only after the commands stabilize; and
- keep Blender placement and held-item delivery as separate future slices.
