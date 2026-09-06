# Bulk World-Asset Review and Promotion Design

Status: approved by Kirk in conversation on 2026-09-06

Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)

Design slice: [rpg-project#394](https://github.com/KirkDiggler/rpg-project/issues/394)

Related deployed composition design: [rpg-project#378](https://github.com/KirkDiggler/rpg-project/issues/378)

## Purpose

Turn the now-repeatable Synty pack ingestion pipeline into a scalable human selection workflow for world building.

Pack conversion and contact sheets are no longer the bottleneck. A collaborator can already build hundreds of cached GLBs and browse paired sheets, but calibration preparation still accepts one source at a time and the local Web form assumes every loaded row must become a provider prop. That makes inspecting, selecting, and calibrating a rich catalog unnecessarily slow.

This design adds one manifest-backed review queue across Dark Fortress props, items, weapons, and environment pieces. Kirk can visually classify candidates, mark interesting models Keep, return later to finish their metadata, explicitly mark valid selections Ready, and export only those Ready entries to a private provider batch. After promotion and normal asset sync, the selected visuals appear in the World Builder catalog.

The complete flow is:

```text
preserved Dark Fortress source
  -> palette-C material preflight
  -> trusted cached GLBs + neutral material-review GLBs
  -> paired browsing sheets
  -> source-glob review preparation
  -> loopback-only Asset Review Lab
  -> Undecided / Keep / Ready / Skip / Defer
  -> Ready-only provider recipe
  -> private atomic world-asset promotion
  -> generated license-safe Web catalog
  -> asset sync
  -> World Builder palette
```

This is the private **base visual asset** workflow. It does not replace the deployed composition publication architecture in #378. DMs compose these supplied assets through the deployed application; they do not run this licensed-source ingestion pipeline or publish their compositions through a developer workstation.

## User outcome

Kirk can:

1. process Dark Fortress under palette C;
2. prepare a review batch with readable source-relative globs rather than adding one file at a time;
3. browse the matched cached GLBs with previous/next navigation, search, filters, and keyboard decisions;
4. mark candidates Keep without finishing metadata immediately;
5. return to all Keep entries and finish classification and calibration;
6. mark complete, visually accepted entries Ready;
7. export one provider recipe containing only Ready entries; and
8. promote, sync, and select those visuals in the real World Builder.

A batch may contain hundreds of candidates. Review may happen in smaller source-glob batches when that is more practical. No script invocation is required between candidates.

## Scope and inventory

The first real pack is POLYGON Dark Fortress v3. The configured review inventory contains 825 top-level FBX sources:

| Source family | Count | Suggested visual category |
| --- | ---: | --- |
| `SM_Env_*` | 92 | `env` |
| `SM_Bld_*` | 329 | `env` |
| `SM_Veh_*` | 8 | `env` |
| `SM_Prop_*` | 352 | `props` |
| `SM_Wep_*` | 17 | `weapons` |
| `SM_Chr_Attach_*` | 21 | `items` |
| `FX_*` | 6 | `env`, review-only until static fidelity is supported |
| **Total** | **825** | |

The three combined character/cape FBXs are outside this world-building run. Characters, rigs, animation, and equipment attachment retain their existing workflows.

Every source remains discoverable even when its material or effect cannot yet become a trusted runtime asset.

## Decisions

### 1. Use a manifest-backed review queue

A provider-owned script selects candidates by source-path glob from the conversion manifests. The manifest resolves each `.fbx` source identity to one cached `.glb` path and SHA-256. The browser never receives unrestricted access to a local folder.

Conceptual command:

```bash
python3 scripts/prepare_asset_review.py \
  --pack-root library/polygon-dark-fortress/v3 \
  --source-match '**/SM_Prop_Brazier_*.fbx' \
  --source-match '**/SM_Prop_Candle*.fbx' \
  --web-root ../rpg-dnd5e-web \
  --reset \
  --port 5173
```

`--source-match` uses shell-style source-relative globs, not regular expressions. Globs are readable beside contact-sheet labels and do not turn arbitrary pattern execution into another parsing/security surface. The option is repeatable and results are deduplicated and sorted by canonical source path.

The script does not reconvert FBX files. It verifies and stages existing cached GLBs for local review.

### 2. Keep the browser local and read-only toward repositories

The Asset Review Lab is available only in a Vite development build on a loopback hostname with an explicit query parameter. It reads an ignored catalog and ignored model files. It has no repository-writing API, local daemon, production route, or arbitrary filesystem picker.

Browser local storage is a convenience draft. Portable JSON export is the handoff to provider tooling.

### 3. Separate review decisions from provider readiness

Each candidate has exactly one review state:

```text
Undecided -> Keep -> Ready
      |         |
      |         -> metadata edit -> Keep
      -> Skip
      -> Defer
```

- **Undecided** means not reviewed.
- **Keep** means visually selected. Required metadata may remain incomplete.
- **Ready** is explicit final human approval to publish the visual into the game. It is never inferred merely because fields happen to validate.
- **Skip** excludes the candidate from the current review.
- **Defer** retains it with a reason such as material, FX, revisit, or current placement limitation.

Changing category, display identity, calibration, or another provider field on a Ready entry demotes it to Keep. A changed source GLB hash creates a new candidate identity and cannot inherit Ready.

### 4. Publish visual categories, not gameplay behavior

The review category radio has four values:

- `props` — scenery candidates;
- `items` — future inventory candidates;
- `weapons` — future equipment candidates; and
- `env` — environment and structure candidates.

Every category is available to the World Builder as a visual component after promotion. Category does not grant pickup, equipment, collision, line-of-sight, lighting, animation, or Toolkit behavior.

A source-prefix mapping supplies an editable suggestion:

```text
SM_Prop_*        -> props
SM_Wep_*         -> weapons
SM_Chr_Attach_*  -> items
SM_Env_*         -> env
SM_Bld_*         -> env
SM_Veh_*         -> env
FX_*             -> env, forced non-Ready until supported
```

Kirk may reclassify a source. For example, `SM_Prop_Rope_01` can change from Props to Items because it is an inventory candidate. The category change updates the derived visual ref.

Toolkit refs remain a separate later mapping. For example:

```text
dnd5e:items:dark-fortress:rope_01       visual asset
dnd5e:item:rope                         future Toolkit gameplay concept
```

Any link between them must be explicit when inventory behavior is designed. This workflow never guesses it from a filename.

### 5. Derive exact visual refs mechanically

Dark Fortress world-asset refs use exactly four segments:

```text
dnd5e:<category>:dark-fortress:<source-suffix>
```

Examples:

```text
SM_Prop_Brazier_01.fbx
  -> dnd5e:props:dark-fortress:brazier_01

SM_Prop_Rope_01.fbx, reclassified as Items
  -> dnd5e:items:dark-fortress:rope_01

SM_Wep_Sword_02.fbx
  -> dnd5e:weapons:dark-fortress:sword_02

SM_Bld_Tower_03.fbx
  -> dnd5e:env:dark-fortress:tower_03
```

The pack reference key `dark-fortress` is reviewed configuration, separate from the source pack slug `polygon-dark-fortress`. Review identity fields live in `scripts/configs/world-asset-review/<pack-slug>.json`, not the conversion config: changing category/ref suggestions must not invalidate a manifest whose GLB bytes did not change.

The suffix algorithm:

1. takes the FBX basename without its extension;
2. removes exactly one configured source-family prefix;
3. lowercases the remainder;
4. preserves underscores and numeric suffixes; and
5. rejects empty or non-reference-safe output.

The generated ref is read-only in the browser. The category radio is its only ordinary variable. If two selected sources generate the same ref, preparation/provider validation fails and requires an exact per-source `refSuffixOverride` in reviewed pack configuration. It never silently adds a directory or counter.

These refs are exact visual identities. They introduce no family-default aliases and do not change existing legacy refs such as `dnd5e:props:brazier`, existing exact prop refs, or gameplay refs such as `dnd5e:item:longsword`.

## Dark Fortress palette and materials

### Palette C is the pack default

The Dark Fortress pack configuration selects `PolygonDarkFortress_Texture_01_C.png` as its canonical default atlas. Every compatible main-atlas slot uses C.

A and B remain named variants for narrowly scoped comparisons. Comparison output is namespaced by an explicit comparison ID so a brazier run and a later plushie run cannot overwrite each other's manifests or leave stale sheets in the same palette directory.

This design depends on the material-slot and palette-comparison foundation in `rpg-game-assets#160`. That PR is not ready until the comparison namespace issue recorded on it is fixed and its final reviewed head lands.

### Material preflight is mandatory

Before conversion, each selected source slot is classified as:

- `default-atlas` — compatible with the configured C atlas;
- `explicit` — bound to a reviewed auxiliary base-color/normal/wrapping configuration;
- `review-required` — a declared non-default material has no supported binding; or
- `unsupported` — source declaration, object, slot, texture, effect, or path cannot be verified.

A source is `ready-default` when every slot is default-atlas. It is `ready-explicit` when every non-default slot is explicitly mapped. The other two states are blocked from trusted conversion and Ready review state.

The preflight produces deterministic JSON plus a human-readable Markdown summary. It groups unresolved sources by material family so one reviewed mapping can unlock a family rather than requiring one-off scripts for every model.

No unknown custom slot may silently use C. The existing brazier proof remains the reference: its primary slot uses the pack atlas while the chain slot retains its own color texture, normal, wrapping, and face assignment.

### Trusted and review-only outputs

Material-ready sources enter the visible ignored canonical cache:

```text
library/polygon-dark-fortress/v3/glbs/
```

Material-blocked sources may receive neutral-material, geometry-only review GLBs under a separate ignored review cache. Neutral output exists only to reveal shape and scale. It carries a visible `MATERIAL REVIEW REQUIRED` status in its sheets and Asset Review Lab, and can be Keep, Skip, or Defer but never Ready.

A material-trusted GLB may still be provider-ineligible when its planned post-ceiling embedded textures exceed the runtime budget. It stays visually trusted and reviewable, but `readyEligible` is false and the Lab shows the exact provider reason. Review status describes material fidelity; Ready eligibility independently describes whether the current provider contract can publish the asset.

FX sources use the same review-only boundary until a static export is proven to represent the intended effect. A source that cannot produce even an honest neutral/static preview remains in the report without a fabricated image. Flat planes and particle review meshes with one or two zero-sized bounds axes remain browseable when they are already Ready-ineligible; their dimensions are preserved exactly rather than clamped or fabricated.

The C rebuild is stage-first. It computes required disk, builds trusted and review-only outputs in disposable sibling roots, validates manifests and expected counts, and only then replaces the corresponding cache/sheet surfaces. Failure preserves the previous canonical discovery library.

Tracked browsing output distinguishes normal trusted sheets, material-review sheets, and FX/static-review sheets. Wrongly textured fallback images never appear in the trusted surface.

## Review preparation contract

The preparation script consumes one or more trusted or review-only manifests. For every match it verifies:

- reviewed pack identity;
- normalized source path;
- exactly one manifest row;
- expected material/review status;
- cached output containment and regular-file status;
- recorded versus actual GLB SHA-256; and
- ignored destination containment in the intended Web checkout.

Review files use content-addressed names. When supported, preparation uses a copy-on-write filesystem clone/reflink to avoid duplicating large embedded textures. It falls back to an ordinary verified copy. It does **not** hard-link the destination to the canonical cache, because writes through either hard-link name would mutate the same inode and violate cache custody.

Preparation performs a disk-space preflight before copying and writes the catalog atomically. `--reset` removes only the ignored Asset Review Lab directory after proving containment. Re-running the same selection is idempotent.

## Asset Review Lab

### Catalog and suggestions

Each prepared candidate includes:

- pack slug and version;
- source path;
- GLB SHA-256;
- temporary local URL;
- trusted/material/FX review status and blocking reasons, including provider texture-budget ineligibility;
- source family;
- suggested category;
- derived suffix and visual ref;
- suggested display name;
- measured source bounds in metres (finite non-negative review bounds; Ready still requires all three axes positive); and
- browsing-family guess such as `brazier`.

Suggestions reduce typing but do not become gameplay facts. Display names are humanized from source names and remain editable. Category suggestions remain editable. Refs remain derived.

### Navigation

The Lab keeps the calibrated scene as the primary surface and adds a candidate drawer with:

- search across source filename/path, display name, ref, and browsing family;
- category filter;
- source-family/folder filter;
- review-state filter;
- trusted/material/FX status filter;
- direct candidate selection;
- previous/next within the filtered result set; and
- deterministic source-path order.

Progress shows totals for Undecided, Keep, Ready, Skip, and Defer. A dedicated Keep/Needs Details filter supports the second pass.

Keyboard controls:

```text
Left / Right  previous / next in current filter
K             Keep
S             Skip and advance
D             Defer and advance
```

Typing in a form field does not trigger review shortcuts. Ready is an explicit button rather than a one-key action, preventing an accidental keystroke from publishing a candidate.

### Metadata and calibration

A review entry contains:

- display name;
- category radio;
- read-only derived visual ref;
- scale;
- base yaw;
- fine X/Y/Z offset;
- browsing tags;
- an explicit `supportsDecoration` authoring hint for surfaces such as tables;
- notes; and
- review state plus optional defer reason.

The scene retains the real Synty floor, hex context, standard fighter, orbit/play cameras, facing inspection, and raw/calibrated overlay from the delivered Prop Calibration Lab. Large assets are allowed to overhang the current hex during review; the Lab reports measured dimensions rather than inferring occupancy.

Ready requires:

- material-ready source status;
- a pure provider preflight showing the planned post-ceiling embedded images fit the 4.5 MiB aggregate decoded-texture budget;
- successful browser GLB load;
- unique valid derived ref;
- non-empty display name;
- valid category;
- finite positive scale within provider bounds;
- normalized finite yaw;
- finite bounded fine offsets;
- positive scaled bounds with no axis above the provider's existing 20-metre static-asset ceiling; and
- explicit Mark Ready confirmation.

Tags and notes are useful but not required for Ready. `supportsDecoration` defaults false and is never inferred from a filename; it affects only World Builder surface attachment, not gameplay collision or physics. Source hashes and refs are read-only.

### Memory behavior

The current `useGLTF` global cache is unsuitable for walking hundreds of embedded-texture GLBs because every visited model can remain resident. The Lab uses a review-specific loader lifecycle that:

1. loads only the current candidate;
2. optionally prefetches only the next candidate;
3. releases the prior scene, geometries, materials, textures, and loader cache entry after navigation; and
4. keeps only shared floor/fighter resources resident.

Navigation remains usable after repeated candidate changes without accumulating every prior atlas or reproducing the earlier WebGL context-loss failure.

## Review state and export schemas

### Candidate identity and draft merge

The stable review identity is the tuple:

```text
(packSlug, packVersion, sourcePath, glbSha256)
```

A refreshed catalog merges exact matching entries with the local draft. New candidates begin Undecided. Removed candidates remain only in an exported older review file and are reported as stale on import. Changed GLB bytes produce a different identity and require a new decision.

### Review-progress export

The review-progress JSON contains:

- schema version;
- batch ID;
- all current candidates;
- all review states;
- metadata/calibration drafts;
- source identity and hash; and
- defer reasons.

It contains no absolute path, localhost URL, Blob URL, or licensed bytes. Import is strict and atomic: malformed/stale input reports errors without replacing the valid in-memory/local draft.

### Provider export

Provider export contains only Ready entries. Undecided, Keep, Skip, and Defer never appear. At least one Ready entry is required.

Conceptually:

```json
{
  "$schemaVersion": 1,
  "batchId": "dark-fortress-world-assets-v1",
  "entries": [
    {
      "source": {
        "packSlug": "polygon-dark-fortress",
        "packVersion": "v3",
        "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Brazier_01.fbx",
        "glbSha256": "<generated>"
      },
      "category": "props",
      "ref": "dnd5e:props:dark-fortress:brazier_01",
      "displayName": "Brazier 01",
      "calibration": {
        "scale": 1.0,
        "yawDegrees": 0.0,
        "fineOffsetMeters": [0.0, 0.0, 0.0]
      },
      "tags": ["dark-fortress"],
      "supportsDecoration": false,
      "notes": ""
    }
  ]
}
```

The browser omits its local URL and review-only fields. The provider recomputes the expected category/ref relationship and never trusts browser-derived strings alone.

## Private world-asset provider

A new generic provider consumes the Ready-only recipe. It does not overload the existing floor-prop, held-weapon, or semantic environment role providers.

Canonical layout:

```text
harness/models/synty/world-assets/
  props/dark-fortress/<suffix>.glb
  items/dark-fortress/<suffix>.glb
  weapons/dark-fortress/<suffix>.glb
  env/dark-fortress/<suffix>.glb

scripts/configs/world-asset-promotion/<batch-id>.json
harness/catalogs/synty-world-assets.json
evidence/world-asset-promotions/<batch-id>/receipt.json
```

The provider exposes stage, validate, apply, and check operations using the existing static release transaction pattern.

`stage`:

1. parses the strict recipe;
2. resolves every source through its exact trusted conversion manifest;
3. verifies source and GLB hashes plus material-ready status;
4. recomputes and validates category, pack key, suffix, ref, output path, and uniqueness;
5. normalizes scale, yaw, centering, grounding, and fine offset in factory Blender;
6. preserves all reviewed material-slot assignments and embedded texture provenance;
7. caps every embedded image at 1024×1024 without upscaling, preserves the existing 4.5 MiB aggregate decoded-texture ceiling, and enforces the current static geometry gates;
8. validates finite positive bounds, the existing 20-metre maximum runtime axis, and identity output transforms;
9. generates a complete candidate catalog, inventory updates, mesh statistics, and receipt; and
10. mutates no canonical path.

`apply` repeats exact-stage validation and atomically installs every target. A failure restores the previous provider tree. `check` recomputes expected metadata from tracked recipes and current promoted bytes and fails on drift.

A new batch may not silently redefine a ref already owned by another tracked recipe. Updating an existing visual happens by changing its owning tracked recipe and obtaining a new Ready review for the new source hash/transform.

The provider creates deterministic PR-ready repository changes. It does not open, approve, or merge a GitHub pull request.

## Consumer-safe catalog and World Builder

The private provider catalog projects only license-safe data:

- exact visual ref;
- display name;
- category;
- runtime-relative file;
- runtime GLB hash and size;
- measured bounds;
- browsing tags;
- the explicit `supportsDecoration` authoring hint; and
- provider recipe/tool identity required for verification.

It contains no raw source asset, machine path, review draft, rejected/deferred candidate, or gameplay behavior.

The existing asset sync copies the private runtime tree into the Web checkout's ignored `public/models/synty/` tree. A deterministic generator verifies provider versus synchronized hashes and emits committed TypeScript resolver metadata. The public repository tracks metadata, never Synty GLBs.

A generic `WorldAssetModel`/resolver consumes this generated catalog. Existing legacy `PropModel`, prop exact/default resolution, weapon attachments, and semantic environment role rendering continue unchanged. The World Builder palette can list both legacy placeables and new exact world assets without hand-maintaining a second list.

The palette groups new entries by pack, category, browsing family, and search terms. Selecting an entry places its exact visual ref. Unsupported refs render empty and report a diagnostic; they never substitute another asset.

A Ready world asset is therefore genuinely in the game as a selectable visual component after provider merge, asset sync, and consumer merge. It is not thereby a Toolkit item, equippable weapon, light source, blocker, pickup, or multi-hex object.

## Relationship to compositions and current hex limits

This workflow supplies the rich base vocabulary needed by the composition authoring work:

- candles, books, bottles, tables, chairs, chests, crates, barrels, tools, and clutter can be arranged into reusable scenes now;
- multiple visual parts may belong to one authored composition;
- a published composition may currently behave as one gameplay prop anchored to one hex; and
- large environment assets can be reviewed and supplied without claiming that current gameplay occupancy is correct for them.

This design records measured bounds but does not infer occupied hexes. Multi-hex occupancy, multiple independent gameplay objects in one hex, collision aggregation, and pickup/equipment behavior remain owned by their respective world/composition and Toolkit designs. Real reviewed assets can be used as forcing examples when those contracts advance.

## Failure behavior

- Unknown pack/group/source matches fail before output.
- A source absent from, duplicated in, or inconsistent with its manifest fails preparation.
- Missing, external, symlinked, path-traversing, or hash-mismatched files fail closed.
- Empty source globs fail unless an explicit dry-run/list mode was requested.
- Insufficient disk fails before review copies or rebuild output are written.
- Material preflight writes its report but cannot publish an unresolved source as trusted.
- Neutral/material/FX review candidates cannot become Ready.
- Browser load errors are candidate-local and preserve the rest of the review draft, but block Ready for that candidate.
- Ref collisions fail with both source paths named.
- Invalid imported review JSON cannot replace a valid draft.
- Provider validation reports failures by source/ref and mutates nothing.
- Provider apply is batch-atomic.
- Consumer generation fails before writing when provider metadata and synchronized bytes disagree.
- Unsupported exact refs never render a plausible substitute.

## Verification

### Assets repository

Synthetic tests cover:

- material-list parsing and slot classification;
- palette C selection and expected embedded atlas bytes;
- auxiliary texture/normal/wrapping preservation;
- safe-only versus neutral-review manifests;
- all source-family category suggestions;
- prefix stripping, lowercase suffix derivation, and explicit suffix overrides;
- repeatable source globs, deterministic ordering, and deduplication;
- traversal, symlink, stale hash, wrong pack, empty match, and disk-preflight failures;
- reflink/copy fallback without hard-link aliasing;
- strict provider recipe parsing and Ready-only entries;
- derived ref/output collision rejection;
- Blender normalization and material preservation;
- stage purity, atomic apply/rollback, and check drift detection; and
- deterministic provider catalogs, receipts, mesh stats, and complete inventory.

Private acceptance verifies the configured 825-source Dark Fortress inventory and reports exact trusted, material-review, unsupported, and static-FX counts rather than baking estimated counts into tests.

### Web repository

Tests cover:

- development/mode/loopback/query route gating;
- strict prepared-catalog and review-draft parsing;
- category suggestions and derived read-only refs;
- Undecided/Keep/Ready/Skip/Defer transitions;
- explicit Ready and automatic demotion after provider-field edits;
- material/FX Ready refusal;
- search and category/family/status/source filters;
- previous/next and shortcuts within filtered results;
- shortcut suppression while typing;
- autosave, safe import, and stale candidate reporting;
- Ready-only provider export with no local URLs;
- candidate-local load errors;
- prior-model resource disposal across repeated navigation;
- deterministic provider-to-TypeScript generation; and
- World Builder palette/resolver integration without changing legacy refs.

### Human acceptance

The first real acceptance run proves:

1. a multi-model Dark Fortress source glob prepares with one command;
2. Kirk moves through candidates without another script invocation;
3. a source-prefix suggestion can be changed, such as Props to Items;
4. the visual ref updates to the exact approved formula;
5. Keep survives reload/export/import with incomplete metadata;
6. Keep/Needs Details returns Kirk to selected candidates;
7. valid selected candidates become Ready only through explicit action;
8. Ready-only JSON stages and applies atomically in the private provider;
9. synced entries appear once in the real World Builder palette; and
10. placed visuals save/reopen through the composition authoring path without acquiring invented gameplay behavior.

Substantive implementation PRs receive one fresh independent final review round at their final heads. Copilot remains disabled. Final verdicts are published on the PRs with commands and evidence considered.

## Learning documentation

The implementation adds or expands human lessons under `rpg-game-assets/docs/human/asset-ingestion/`:

- `material-preflight.md` — slots, UVs, default versus auxiliary material families, reports, and safe recovery;
- `asset-review-lab.md` — source globs, review batches, navigation, state, calibration, drafts, and Ready export;
- `world-asset-promotion.md` — recipe custody, stage/validate/apply/check, provider PR, sync, and Builder verification; and
- `troubleshooting.md` — stale caches, hash drift, missing candidates, disk limits, Web load failures, draft recovery, and material blocks.

Each lesson includes:

- **What owns this stage**;
- **Make it work** commands and observable results;
- **Explain it** questions that test understanding;
- common failure meanings rather than bypasses; and
- how to resume without deleting licensed sources, canonical caches, or review decisions.

## Repository and delivery strategy

The design and implementation plan live together on the rpg-project #394 idea PR, which remains open until implementation completes.

Prerequisite:

1. finish the palette-comparison namespace fix, final review, and merge of `rpg-game-assets#160`.

Implementation uses one issue branch per owning repository for this wave:

2. one `rpg-game-assets` issue/branch for palette-C material preflight, trusted/review output, bulk preparation, generic provider, docs, and provider consumer catalog;
3. one `rpg-dnd5e-web` issue/branch from `origin/dev` for the Asset Review Lab, generated catalog consumption, and World Builder palette integration; and
4. the existing composition work remains on its separately owned branches/contracts.

Development follows the consumer need without merging an unavailable provider. The Web Lab can consume synthetic/prepared catalogs while the Assets provider is built. Provider promotion lands before the Web branch binds its committed generated catalog to merged provider authority.

All work remains PR-only. Bounded coordinator-controlled delegation is allowed when it preserves a useful context boundary, such as keeping Python provider work separate from TypeScript/Web work. The coordinator retains the plan, uses one writer per worktree, reviews results between bounded task batches, and reports back rather than launching an unattended autonomous review/fix loop. Unrelated local Dungeon library/config work, placed assets, active notes, and generated Web customization changes remain untouched.

## Out of scope

- Toolkit item, weapon, environment, or pickup mappings;
- gameplay behavior inferred from visual names or categories;
- inventory transfer or equipment attachment;
- automatic lighting/emission behavior;
- animation, particles, cloth, physics, destruction, or interaction;
- automatic custom-material guessing;
- automatic bulk promotion of every converted model;
- direct browser filesystem access;
- a local or production asset-review server;
- automatic GitHub PR creation or merging;
- deployed composition persistence/publication from #378;
- multi-hex occupancy and multiple independent gameplay objects per cell;
- changing existing legacy prop, weapon, or environment refs/providers; and
- public distribution of licensed GLBs or source archives.

## Completion criteria

The journey slice is complete when:

- Dark Fortress uses C as its configured canonical main atlas;
- all 825 configured world-building sources are accounted for by trusted, material-review, unsupported, or FX review status;
- the canonical trusted cache contains no unresolved custom-material fallback;
- a repeatable source glob prepares a verified review queue from existing cached GLBs;
- the Lab remains responsive while navigating repeated embedded-texture candidates;
- Kirk can Keep first, finish metadata later, and explicitly mark selected entries Ready only when material and current provider budgets both pass;
- category overrides generate exact refs using `dnd5e:<category>:dark-fortress:<suffix>`;
- review progress and Ready-only provider JSON round-trip without local paths or licensed bytes;
- provider stage/apply/check publishes a selected batch atomically;
- generated consumer metadata binds to the merged private provider revision and exact GLB hashes;
- Ready entries are selectable and render correctly in the real World Builder; and
- no Toolkit behavior, gameplay occupancy, or licensed public artifact is introduced by implication.
