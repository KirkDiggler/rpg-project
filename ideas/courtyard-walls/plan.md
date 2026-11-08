# Courtyard S3 — published wall kit and configuration

**Goal:** make the already-published stone/timber wall kit available through existing Studio wall controls and demonstrate a courtyard-facing treatment that survives native save/play.

**Authority:** [story #867](https://github.com/KirkDiggler/rpg-project/issues/867), [Courtyard S3 #895](https://github.com/KirkDiggler/rpg-project/issues/895); KirkDiggler approved “walls it is then.” This issue-linked brief records the agreed design; no new storage, renderer or wire architecture is introduced.

**Architecture:** private published world assets → generated public-safe metadata and pinned ignored runtime → existing Studio wall appearance controls → native `wall.appearance` → existing captured atlas/play rendering. Geometry capability remains explicit; only the existing enrolled plain wall supports miter fitting. The independent material catalog supplies the existing Studio-only finish controls, not new play-side finish support.

**Execution:** direct, task-by-task. No delegation required for implementation. One applicable independent review after the early look; no repeated floor approval.

**Baseline:** Assets `fe6b5ee` (merged #317/#324 resources/models); Web `c3af7c0d` (world and material pins `44ade03`), API `9a3896e`, encounter `241e4263`. Web worktree `.worktrees/courtyard-895`, branch `feat/895-courtyard-walls`, stacked on ground #1395 to retain the approved baseline. Initial focused wall helper/control baseline: 71 passing tests. Ground/provider release prerequisites remain #874, not permission to merge this child early.

## Constraints and finish line

- Preserve exact earlier asset identities/defaults and ground provider pin. No licensed bytes in public Git, no hand-edited generated metadata, no duplicate ingestion/Ready approvals.
- Existing native wall appearance has `assetRef`, `height`, `thickness`, `elevation`; appearance edits do not change lines, blockers, door openings/bindings, regions, ground or placed items.
- More wall **models/default looks**, not a universal material editor. The five already-published masonry materials may be adopted for existing Studio-only controls. General atlas/door scopes and finish Save & Play remain Web #1268; do not claim or silently implement them here.
- Do not use visual windows/archways to imply new holes in authoritative opaque barriers. Moving leaves remain excluded from repeated wall bodies. Detailed timber geometry is not a plain miter body.
- No operator :3030 or Lab :3180 changes, no merges without authorization. Reuse the owned isolated castle stack for early view rather than build another custom asset viewer.
- Required artifacts: generated consumer pins, distinguishing regression tests, normal package checks, isolated Studio/play visual evidence, appearance-only castle fold after visual agreement, applicable gates/review and published PRs. Release readiness follows #874 and explicit merge authorization.
- Actual canonical castle source currently has no tree item. Preserve all existing items in any edited document; do not invent the operator's approved tree placement. Obtain their current document before claiming a fold preserves that specific composition.

## Task 1 — Adopt published provider catalogs

**Owner:** Web sync/generation; Assets is an existing read-only provider.
**Files:** `src/generated/worldAssetCatalog.ts`, `src/generated/worldMaterialCatalog.ts`; new `src/concepts/encounter-studio/courtyardWallAssets.test.ts`; run record `knowledge/runs/2026-11-08-courtyard-walls.md`.
**Interfaces:** existing `GENERATED_WORLD_ASSETS`, `GENERATED_WORLD_MATERIALS`, `WORLD_BUILDING_CATALOG_BY_REF`, `repeatableWallAssetRefs`, `isRepeatableWallAsset`. No shape changes. Source is exact merged Assets main revision, resolved to full SHA before generation.
**Behavior:** expose published House Wall01–09 and their thumbnails/measured bounds; reject moving leaves for repeating; retain existing Castle Wall Fit01 declarations; expose the five schema1 wall finish rows with complete paired resources. Ground pin/bytes are independent and unchanged. Failure to consume the published schema is a real seam finding, not a reason to drop provider fields.

- [ ] Add `published solid house walls are selectable wall bodies` covering the missing 01/05–09 refs individually and existing 02–04; show absence failure on the old pin.
- [ ] Add `moving leaves are not repeating wall bodies` using an actual published leaf; `only declared fit bodies claim corner capability`; `published masonry finishes retain paired resources` for each named ref, not a historical catalog-count assertion.
- [ ] Run existing `scripts/sync-game-assets.sh --world-assets` and `--world-materials` against clean exact provider with `RPG_GAME_ASSETS_PATH` and `ASSETS_SYNC_SKIP_UPDATE=1`; no generated file editing.
- [ ] Run new tests, generation tests, wall controls/edit helpers; verify semantic preservation of every prior generated asset/material row and unchanged ground metadata.
- [ ] Run normal `assets:sync:pinned`; check actual delivered model/thumbnail/image hashes against generated metadata/provider. No broad unrelated Assets suite.

**Verification:** from Web worktree: `npx vitest run src/concepts/encounter-studio/courtyardWallAssets.test.ts scripts/generateWorldAssetCatalog.test.ts scripts/generateWorldMaterialCatalog.test.ts src/concepts/world-building/structuralWallEditing.test.ts src/concepts/encounter-studio/StudioWallControls.test.tsx`; sync `--world-assets --check`, `--world-materials --check`, and `ground:check` with its actual provider checkout. Expected: named new choices work, old rows/ground are exact, honest capability eligibility. At PR boundary only, `npm run ci-check`.

## Task 2 — Native appearance-only authoring regression

**Owner:** Web authoring contracts.
**Files:** new `src/concepts/encounter-studio/courtyardWalls.test.ts` and verbatim `fixtures/the-mad-king-walls-source.yaml` from API `9a3896e`; existing `draftFile.ts`, `structuralWallEditing.ts::setWallAppearance`, `roomDocument.ts` are consumed, not rewritten by default.
**Inputs/outputs:** current served native document; model choices from Task 1; existing pure appearance edit and document/native export helpers. A proposed stone hall edge and timber kitchen-facing edge are initial builder choices for the early view, not final artistic acceptance.
**Behavior:** edit model ref and visual dimensions through production helpers, preserving all other source facts; reset and history restore exact prior appearance. Short/long spans with an opening derive finite positive pieces and leave that opening clear; reject a moving-leaf ref under existing validation. No fabricated miter capability for detailed timber.

- [ ] Add `current castle changes only selected wall appearances`: import and byte-roundtrip actual source, apply two wall model edits, persist/reopen/export; parsed result equals original after restoring just the edited appearance fields.
- [ ] Assert ground and all scene items, walls' lines/blockers/openings and door bindings stay exact; verify original/default restoration.
- [ ] Use existing `layoutWallPieces` and current structural geometry contracts to check finite tiling and no opening overlap at short and longer lengths. Reuse existing undo/redo controls tests; add only missing changed-path assertions.
- [ ] Run focused test and types; no production behavior patch unless the tests expose a genuine gap within the agreed contracts.

**Verification:** `npx vitest run src/concepts/encounter-studio/courtyardWalls.test.ts src/concepts/encounter-studio/castleGround.test.ts src/concepts/encounter-studio/StudioWallControls.test.tsx`; `npm run typecheck`. Expected exact non-appearance preservation and same model identity after native reload.

## Task 3 — Early visual integration, then accepted fold

**Owner:** Web Studio/play and the owning content copies. Reuse isolated `local/courtyard-874-castle` (:3182/:8125), only after syncing normal assets and preserving existing saved runs. Do not modify the running operator stack or Lab.
**Files:** private ignored wall-preview artifacts and native export; after operator appearance agreement only, API `content/the-mad-king.yaml`, toolkit `rulebooks/dnd5e/encounter/dungeonspec/testdata/world-builder-v4-castle.yaml`, their existing atlas/compiled fixtures and scoped tests/run records. Separate content worktrees/branches based on #874 checkpoints when that task begins.
**Lifecycle:** boot/check the new Web head, open a native copy in the existing Studio, configure the two wall appearances through real controls/helpers, save a copy and show Studio/play. A local walk copy may start in the courtyard so seeing walls does not require a fight; that spawn is never folded into canonical content. Do not auto-use alternate finishes that the UI labels Studio-only. Artist reviews wall look, proportions, seams and visibility from the actual camera, not an engine checklist.

- [ ] Verify normal delivery and no browser exceptions, exact runtime head; inspect screenshots before sending the link.
- [ ] Record exact selected refs/appearance values and source hash. Keep prior runs intact. Clearly distinguish a proposed composition from the operator's tree-containing document.
- [ ] Request the bounded visual look, then do applicable review/walk only after the early-view rule is met.
- [ ] Fold only accepted appearance fields; structural comparison and existing `TestCastleCopiesAgree` prove no unrelated content changes. Re-capture changed goldens with trusted unchanged projection code and compare allowed fields explicitly.
- [ ] Run owning gates (`GOWORK=off go test ./...` and lint in encounter; actual SDK/API pins if needed; API `make test` / `make ci-check`; one Web `npm run ci-check` at PR boundary). Publish linked PRs and exact evidence; retain drafts where ground-wave release dependencies are outstanding.

**Completion:** actual changed walls visible through the native saved path, operator wall feedback recorded, non-appearance facts unchanged, applicable technical gates/review complete. This is not fulfilled by exposing a catalog alone.

## Requirement coverage

| Requirement | Task | Concrete proof |
|---|---|---|
| Additional usable wall models, not re-ingestion | 1 | Actual missing refs become selectable after exact merged-provider adoption |
| Correct calibration/repetition and truthful capabilities | 1,2,3 | measured bounds/leaf exclusions; opening-fit tests and real camera inspection |
| Same choice in Studio/save/play | 2,3 | native roundtrip test + real saved-copy launch |
| Preserve approved floor/tree and authoritative structure | 2,3 | entire parsed document equality except selected appearances; exact operator composition before its fold |
| Small visible improvement before broad testing | 3 | existing Studio/game early look, no new standalone viewer or general gameplay demand |
| Governed delivery and release | 1,3 | pins/hash checks, applicable gates/review, authorization before merge/deploy |

## Provider/consumer seams and checks

| Provider | Consumer | Contract and availability | Proof |
|---|---|---|---|
| Assets merged world catalog/runtime | existing Web generator/sync | measured metadata, role/capability declarations, immutable model/thumbnail identities; already published | Task 1 real generation and byte verification |
| Assets merged material catalog v1 | existing Studio-only finish reader | five complete paired resources; no new schema or play claim | Task 1 named-ref checks and resource validation |
| Studio appearance helper | native document / API compiler | existing appearance fields; no new wire or persistence shape | Task 2 native roundtrip, Task 3 actual launch |
| Native captured atlas | existing wall renderer | explicit model refs/dimensions + unchanged openings/blocks | real saved-copy play and opening-clearance view |
| S2 floor branch | S3 branch and isolated view | stacked branch dependency; released provider tags still pending | preserve exact ground pin, explicit PR base/release prerequisite |

**Plan check findings:** the assets already exist, so new conversion/promotion would duplicate work. World geometry and materials are independently pinned; updating one does not update the other. Finish controls explicitly say Studio-only, so model/default appearance parity must not be confused with general surface editing. Canonical source lacks the operator's tree placement; do not pretend a fixture is their current composition. These findings narrow implementation mechanism, not the approved visible finish line. No protocol, event, authorization or rules changes are needed for model choices. No replaced path or schema migration to clean up.
