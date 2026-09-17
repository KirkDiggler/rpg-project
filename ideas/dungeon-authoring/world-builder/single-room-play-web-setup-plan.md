# World Builder playable-room web setup — milestone 1

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans. Execute this bounded milestone, not the downstream Play/render milestone.

**Goal:** Safe v3 room authoring with visibly hex-snapped existing monsters and party start, retaining the complete freely composed scene.

**Architecture:** Extend the existing RoomDraft/editor/history, not a second editor or scene schema. Draft codecs validate representation only; encounter owns legal placement, occupancy, movement and sight. Produce canonical v3 source via a narrow YAML adapter for the later existing authoring RPC flow.

**Tech Stack:** React/TypeScript/R3F, shared HEX_SIZE=1 math, existing yaml dependency, Vitest.

**Spec:** [single-room-play.md](single-room-play.md). Issue: [web#1112](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1112). One branch/PR spans this milestone and subsequent real Save/Play/runtime rendering; no complete-feature review until both are done.

## Global constraints

- Worktree `rpg-dnd5e-web/.worktrees/1112-world-builder-play`, branch `feat/1112-world-builder-play`, base3a0ad05c (dev). Parent installed dependencies and verified normal Husky hook, typecheck and four focused baseline suites.
- Source changes only in this web worktree. No real Git metadata/index/ref/config/commit/push operations by the worker; parent intentionally owns publication. Read-only status/diff/log is allowed. No other repo, runtime, API/Redis, asset pipeline, global tool/config or browser-user-profile changes.
- Do not add client standing/occupancy/footprint/LOS legality checks. A structurally valid draft may be invalid for Play (off painted floor, overlapping monsters, covered start, missing start). Keep such work editable and persistable; server validation is later, non-destructive.
- Actors are authoring metadata, not scene props. Scenery stays free X/Z/Y/yaw/height; existing palette drag behavior, repeat placement, groups/supports/lights and selection/gizmos must not change.
- No door/loadout/AI/theme work. Fixed Crypt style stays. Reuse the four existing `PALETTE_MONSTERS` entries which resolve promoted models; do not invent refs/models or silently substitute missing assets.
- Old local bytes remain untouched. Invalid present v3 (including empty string) must not fall back or autosave over itself. Preserve world/local ownership and async identity fencing already delivered.
- Early preview is this editor with setup markers, NOT a second compiled-preview pane. No RPC or live environment handoff in this milestone.

## Formats (separate version axes)

1. Local/exported RoomDraft envelope (`kind: rpg-room-authoring-draft`): version3 containing RoomDraft.version3. Key `rpg.concepts.world-building.room-draft.v3`.
2. Saved world room document (`kind: room-authoring-draft`): envelope version2 containing RoomDraft.version3. Read old envelope1/draft2 by explicit upgrade.
3. Canonical dungeon YAML: root `version:3`, `key`, `play:{void:transparent,lighting:bright,standing:centre-covered}`, `room:<complete RoomDraft v3>`.

Scene/library versions remain1. Draft gameplay adds optional `partyStart:{q,r}` and required `monsters: Array<{id,ref,cell:{q,r}}>` (empty by default). Omit an absent start; never invent origin or emit null/empty object.

## Task A — safe model and codecs

**Modify:** `src/concepts/world-building/roomDraft.ts`, `src/compositions/roomDocument.ts` and their tests.
**Create:** `src/concepts/world-building/singleRoomDungeon.ts` and `.test.ts` for the thin canonical YAML adapter.

**Interfaces:** retain existing create/parse/stringify/save/load/reconcile functions. Add exported `RoomMonsterPlacement` as the typed `{id,ref,cell}` value. Proposed canonical adapter signatures:

```ts
export interface EncodeSingleRoomDungeonInput { key: string; draft: RoomDraft }
export function encodeSingleRoomDungeon(input: EncodeSingleRoomDungeonInput): string
export interface DecodeSingleRoomDungeonResult { key: string; draft: RoomDraft }
export function decodeSingleRoomDungeon(source: string): DecodeSingleRoomDungeonResult
```

These adapter names are proposed additions, not alleged existing APIs. Key belongs to the root source, NOT a new persisted RoomDraft field; milestone2 supplies it through authoring UI.

- [ ] Add red tests using a FIXED legacy v2 fixture, not createRoomDraft (which will become v3). Include fractional/negative world poses, nested group/support relations, lights, nondefault height, false flags, workspace and arrangement declarations.
- [ ] Upgrade v2 losslessly by cloning all supported data, setting draft.version3 and adding monsters[]. Chain existing v1 conversion through its established workspace default to v3. Preserve source bytes in legacy local keys; never remove or rewrite them during load.
- [ ] For storage loading, inspect keys in priority order v3→v2→v1, choosing an older key only when the newer key is ABSENT (`=== null`), not falsy. Present malformed/unsupported data yields the existing error-shaped result and known in-memory fallback, no fallback to older bytes. Connect that error to existing protected-local/autosave fencing. Explicit valid Save/Reload/Import may transfer ownership as the current UI does.
- [ ] Validate structure: exact supported versions/frame/workspace, integral bounded axial cells, stable nonempty unique monster IDs, monster ref grammar and complete valid scene graphs/declaration shapes. Ref syntax is not a local rules-catalog or model-availability gate for preserving imported work. Unknown/missing-model values must be explicit UI errors, never dropped. Do not reject a draft just because actors are off painted floor, overlap or lack a playable start.
- [ ] Update saved room envelope to version2. Only valid old envelope1/draft2 upgrades; reject invalid/mismatched/newer combinations. Preserve scene/room document segregation and size symmetry. Use the existing max JSON size consistently on reading AND writing, and retain prior bytes on error.
- [ ] Emit the full canonical source without alias/reference-only reduction:

```ts
// Use the existing yaml library; parse/validate the representation on input.
const document = {
  version: 3,
  key: input.key,
  play: { void: 'transparent', lighting: 'bright', standing: 'centre-covered' },
  room: validatedDraft,
};
```

Decode this exact root/play contract back to key+draft; reject unsupported/malformed shape without using the legacy v2 dungeon parser. No source→engine coordinate conversion here. No new normalization that rounds source doubles, recomputes groups or drops unknown-invalid fields. Add round-trip and refusal tests.

## Task B — snapped actor authoring in the existing editor

**Modify:** `WorldBuildingConcept.tsx`, `WorldBuildingViewport.tsx`, existing room/editor tests and a small scoped styles file only if needed. A focused room-setup controls/marker component may be added rather than bloating the existing components.

**Existing seams:** `WorldBuildingConcept.commit(nextScene, selection, nextRoom, nextWorkspace)` already commits one whole room history snapshot; `WorldBuildingViewport.roomAuthoring` owns room tools and floor gesture callbacks. `reconcileRoomDraft` spreads gameplay data and removes stale prop declarations: preserve actors/start through every scenery edit.

- [ ] Add room-only setup controls: existing monster choices, place/move/remove monster, place/move/clear party start. Stable IDs are minted once using existing identity conventions and retained on moves/undo/export/reload. One user placement/move/removal is one history transaction.
- [ ] Add visibly snapped markers and a snapped hover/placement preview using the SAME existing conversion as floor paint:

```ts
const cube = worldToCube({ x: event.point.x, z: event.point.z }, HEX_SIZE);
const cell = { q: cube.x, r: cube.z };
const center = cubeToWorld({ x: cell.q, y: -cell.q-cell.r, z: cell.r }, HEX_SIZE);
```

`worldToCube` already rounds; HEX_SIZE=1. This is authoring coordinate selection, not game legality. Keep workspace bounds, cancellation/camera ownership and one-gesture history. Use existing promoted monster render leaves where possible, with unmistakable authoring ring/label and explicit loading/error state; no default replacement monster. Party start is a clear setup marker.

- [ ] Keep actor selection separate from scene selectedIds. Placing/deleting an actor must not delete a selected prop, remap support links, insert fake scene items or snap scenery. Switching back to prop editing retains its normal interaction contract. Expose accessible labels for setup controls so unit/browser checks can drive real UI.
- [ ] Do not route actor operations through library arrangements or prop repeat generators. Prop-only mode mounts must remain unchanged. New/duplicate/reopen room paths must preserve or deliberately reset actors through the existing whole-draft operation, never accidentally carry another room's start.
- [ ] Keep UI truthful: this milestone is setup/source authoring, not engine-validated or playable yet. Do not add a fake Play button/session route or compiled preview pane.

## Required milestone proof

Extend existing suites, with explicit tests named/described for each boundary:
- fixed v2 and v1 upgrade retains every authored field; old storage bytes byte-identical; new saves go only to v3;
- invalid/empty current v3 does not recover v2/v1 and does not get overwritten by mount/autosave; recovery requires explicit valid action;
- old room envelope1 upgrades to2; wrong/newer envelopes refuse; full rich source survives both room snapshot and canonical YAML round trip;
- actor add/move/remove/start/clear uses integral snapped cells and one Undo/Redo; generated IDs survive moves and export/reopen;
- scenery/group/support/height edits retain actors, and actor edits leave all scenery free poses/declarations untouched;
- invalid-for-play but structurally valid actor arrangements are retained (no client legality gate);
- mode/Back confirmation and asynchronous world snapshot identity/local-ownership regressions stay green.

```sh
npm run test:run -- src/concepts/world-building/roomDraft.test.ts src/compositions/roomDocument.test.ts src/concepts/world-building/singleRoomDungeon.test.ts src/concepts/world-building/WorldBuildingConcept.test.tsx src/concepts/world-building/WorldBuildingViewport.test.tsx
npm run typecheck
```

Run targeted lint/Prettier checks on changed files. Do not run a standalone full suite or ci-check yet: PARENT performs one full ci-check at the first Draft-PR boundary after inspecting the source and early browser proof. Worker returns the source-only milestone with focused evidence, remaining limits and all changed files; no Git publication commands or false complete-feature claim.

Parent supplies any local licensed asset mount/copy and starts a checked-free temporary Vite with the API binding on Vite, fresh browser context and exact process teardown. Worker does not start servers or touch :3030. The full independent feature review is AFTER milestone2, not another reviewer per setup task.

## Following milestone (not implemented by this task)

Shared authoring RPC validation/save, root dungeon-key UI, Home-selected character→existing create/ready/start lobby→App.handlePlayAuthored, strict RoomSceneJSON adapter and full runtime scene rendering. Actual AuthorView already owns this launch sequence; there is no existing magical pending-key route to assume. Its no-character case says to choose on Home. App and WorldBuilderWorkspace need deliberate callback/character plumbing. Runtime uses source scene units and shared HEX_SIZE=1, not feet; do not multiply visual transforms by the engine k factor or apply group transforms twice. Preserve actual member-scoped actors/visibility and suppress only duplicate legacy scenery/wall rendering when valid canonical presentation exists. Ground detailed milestone2 code/tests at its dependency barrier.
