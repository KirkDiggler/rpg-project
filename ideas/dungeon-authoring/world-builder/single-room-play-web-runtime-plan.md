# World Builder Play — M2 runtime presentation substep

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans. This is one bounded implementation substep on the existing web#1112 branch, not a new feature/PR.

**Goal:** Render the complete canonical room on the existing session/authoring atlas scene path, with strict decoding and legacy compatibility, before wiring the Play button.

**Architecture:** Add an optional typed room presentation to Scene3D at its existing atlas conversion seam. A canonical environment reuses current prop, floor and light leaves; actor rendering and all game decisions stay on existing member-scoped paths.

**Tech Stack:** React/TypeScript/R3F, current generated proto v0.1.199 and shared WorldScene renderer/validation.

**Spec:** [single-room-play.md](single-room-play.md). Existing Draft [web#1116](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1116), issue1112.

## Global constraints and corrections to scout notes

- Writable source ONLY `.worktrees/1112-world-builder-play`, current head5b3c2b49. The detached `.worktrees/1112-setup-preview` serves3031 and MUST stay unchanged;3030/API8110/Redis/assets also protected. No worker Git/config/index/ref/commit/push/reset/stash, servers, browser profiles, asset sync/copy, global configuration or other-repo writes.
- **Proto empty string means ABSENT/legacy.** Generated proto3 string defaults to `""`; the scout suggestion to reject present-empty would break every legacy atlas. Accept missing/undefined or empty string as absent. Whitespace, `"null"`, malformed/unsupported/nonempty data must fail closed — never legacy fallback.
- Canonical RoomScenePresentation has EXACTLY `version`, `coordinateFrame`, `workspace`, `scene`. It has NO painted cells, gameplay declarations, monster/start markers. Do not require or invent them in this JSON. Mechanical cells/boundaries/occupancy/sight remain atlas/session answers.
- No copied rules/geometry or fake room-as-prop anchor. Already-world-posed item/group records are not additional transforms. Source/render both use HEX_SIZE1; do not multiply visuals by feet conversion or discrete facing helpers.
- This substep does NOT implement key UI, Save/Play callbacks or any new authoring preview pane. No hardcoded `workshop-room` default: that was a scout invention, not a product decision. Key handling belongs to the following launch substep.
- One full ci-check only at the next parent-owned changed-head PR boundary, no standalone full suite. One independent complete-feature review after all M2 work and local proof, not per substep.

## Task 1 — strict atlas presentation boundary

**Create:** `src/components/session/roomSceneJson.ts` and focused tests.
**Modify:** `src/components/session/atlasToScene3D.ts`, its tests, `SessionEncounterView.tsx`, `src/author/preview3d/previewScene.ts` and affected tests.

**Proposed interface:**

```ts
export interface RoomScenePresentation {
  version: 1;
  coordinateFrame: RoomDraft['coordinateFrame'];
  workspace: RoomWorkspace;
  scene: WorldScene;
}
export function decodeRoomSceneJSON(value: unknown): RoomScenePresentation | null;
// Scene3D gains optional roomScene?: RoomScenePresentation
```

Use existing visual types; no second pose DTO. Read `roomDraft.ts` and `serialization.ts` for reusable structural validation. Validate exact root/frame/workspace/version and full scene graph, preserving doubles/empty arrays/optional values. Call the existing scene validator with the declared workspace horizontal limit. If helpers need extracting to avoid duplication, make a small shared structural helper; do not manufacture a synthetic RoomDraft or fake scene prop merely to call a validator. No declaration/actor validation here.

- [ ] Red tests: undefined/empty→absent; nonempty full presentation→exact expected graph/poses/light/height/workspace; unsupported version/frame, malformed graph/number/root fields, whitespace and JSON null→explicit refusal; no mutation of source input.
- [ ] Extend buildScene3D's atlas input with optional roomSceneJson. Decode once inside scene construction; retain its existing caller useMemo by atlas identity. Attach the typed presentation to Scene3D; no JSON parsing per prop/per frame. Preserve mechanical floorTiles and actor-related data. If a canonical input cannot be faithfully rendered at a nonstandard requested hex size, refuse explicitly rather than guess a scaling conversion; actual game callers use HEX_SIZE1.
- [ ] `SessionEncounterView` currently invokes buildScene3D inside useMemo, then retains lastGoodSceneRef. Catch presentation refusal into an explicit visible scene-error outcome. **An invalid CURRENT nonempty presentation may not draw the cached prior scene as if valid.** Keep ordinary transient refresh behavior for valid legacy/current data. Disable only unusable scene interaction while this integrity error is shown, not new game-rule eligibility logic.
- [ ] `previewScene` currently returns resolveSceneLayout refusal or directly calls buildScene3D. Catch canonical decode refusal into its existing `{ok:false,message}` result, so existing preview consumers don't crash or silently revert. No second renderer path.

## Task 2 — canonical environment using shared leaves

**Modify:** `src/components/session/DungeonEnvironment.tsx`; add a focused `RoomSceneEnvironment.tsx` if it keeps the canonical/legacy branches clear, and corresponding render tests. `SessionCanvas` already passes Scene3D to DungeonEnvironment; do not rewrite actor rendering.

Actual reusable pieces verified by parent:
- `WorldPropModel` in `src/concepts/world-building/WorldPropModel.tsx`: accepts catalog entry, source position, continuous rotationY and optional heightScale; dispatches generated WorldAssetModel vs legacy grounded PropModel.
- `WORLD_BUILDING_CATALOG_BY_REF` in `catalog.ts`: known visual lookup. Missing entries produce explicit errors, never a substitute model.
- `WorkspaceFloorSurface`/`WorkspaceFloorUnderlay`: existing Crypt floor geometry/UV. Authoring uses radius `workspace.horizontalLimit + 1`, surface Y=`DUNGEON_SURFACE_Y-0.006`. Reuse the geometry/material contract; runtime must present load/error status explicitly rather than silently lose a floor. Do not add paint tint, anchor ring, composition bounds or grid guides.
- `projectCompositionPointLights(scene, placement)`: shared pure projection; item local light offset rotates by item yaw, surface lift once; groups/supports are relations. Use identity outer placement for this already-world-posed inline scene solely to supply stable light IDs — no synthetic AtlasProp, external composition lookup or double group transform.
- `resolveDungeonLighting` and `DungeonSceneLights`: keep current region/focus/light budget behavior. Canonical lights come from the canonical scene exactly once; do not also resolve duplicate legacy composition/prop sources.

- [ ] For valid scene.roomScene, render the full workspace floor and every visual item once through WorldPropModel, using the same source position/surface-lift convention as WorldPropVisual. Keep group/support records intact but do not nest/apply their transforms again. Height remains visual only. Show named loading/error states, no hidden substituted assets.
- [ ] Suppress duplicate legacy AtlasPropModel placements and DungeonShell/AtlasWalls/perimeter generation ONLY in the canonical branch. Leave original legacy branch unchanged when roomScene is absent. Keep raw atlas cells/boundaries and member actions/visibility authoritative; no mesh-derived walkability.
- [ ] Keep actors entirely in SessionCanvas's existing roster/sightings path. Never render author monster/start markers from a presentation (those fields do not belong in it).
- [ ] Focused render tests: rich source with legacy+generated visual entries, negative/fractional poses and yaw, raised grouped item and supported lit decor reaches shared leaves once with exact expected values; whole workspace floor radius/UV path; lights project once; no guides/legacy proxy walls; existing member actor/visibility branches remain mounted and legacy v2 scene tests unchanged. Decoder memoization/current-invalid-vs-cached-good behavior must be tested at the actual caller boundary.

## Gates and handoff

Run targeted decoder/scene/environment/preview/session component tests and typecheck, plus changed-file ESLint/Prettier. Do NOT run full suite/ci-check or start any server. Parent will test the integrated browser route after the launch substep and publish once at its boundary.

Return source-only implementation with exact files, actual targeted commands/results, semantic assertions and remaining launch/UI boundary. No Git publication obligation in worker criteria. If the legacy/canonical rendering boundary cannot be implemented without a new product/rules decision, report the concrete code evidence; do not invent defaults or reopen the approved room design.
