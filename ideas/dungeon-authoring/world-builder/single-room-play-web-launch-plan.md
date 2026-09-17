# World Builder Play — M2 save/launch substep

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans. Finish this one source-only substep on the existing web1112 branch.

**Goal:** Validate/save the complete current room and enter it through the existing selected-character/lobby/session flow, without losing drafts or acting on stale results.

**Architecture:** Reuse the authoring RPC client/hooks and extract the existing AuthorView launch sequence into a shared hook. World Builder supplies canonical v3 YAML and receives the existing App navigation callback. No new RPC, fake encounter or client rules.

**Tech Stack:** React/TypeScript, existing Connect hooks, current RoomDraft v3 and canonical YAML adapter.

**Spec:** [single-room-play.md](single-room-play.md). Issue1112 / Draft PR1116. Runtime substep is implemented in the same writer worktree but uncommitted; preserve it.

## Global constraints

- Writer `.worktrees/1112-world-builder-play` only. Current Git head5b3c2b49 plus intentional runtime-scene patch. Parent checked six runtime suites174tests and full typecheck. Do not reset/revert that patch.
- Detached `.worktrees/1112-setup-preview` on3031, original3030, API8110, Redis, assets and user profiles are protected. No server/browser/tooling/other-repo operations by worker. NO Git config/index/ref/stash/reset/commit/push/branch mutations. Parent owns publication and environment handoff.
- No new gameplay, model palette, character creation, lobby system, doors, loadout/AI/theme features or client legality checks.
- Empty roomSceneJson is legacy absence; the runtime decoder now deliberately rejects non-string objects too. Do not undo the parent's narrowing/regression or otherwise redesign the completed runtime substep.
- Only focused tests/types/changed lint/format here. No full suite/ci-check; parent runs one full gate at the next changed-head PR boundary and one complete independent feature review after integrated proof.

## 1. Share the existing launch flow

**Read/modify:** `src/author/AuthorView.tsx`, `src/App.tsx`, `src/concepts/world-building/WorldBuilderWorkspace.tsx` and tests. Add a small shared hook near AuthorView, e.g. `usePlayAuthoredDungeon.ts`, with focused deferred-promise tests.

Actual existing sequence in AuthorView:

```ts
const lobby = await createLobby({ campaignId: 'default-campaign', characterId });
await setReady({ lobbyId: lobby.lobbyId, ready: true });
const started = await startEncounter({ lobbyId: lobby.lobbyId, dungeonKey });
onPlay(started.encounterId, characterId);
```

App.handlePlayAuthored already clears resumeLobbyId, sets lobbyCharacterId/resumeEncounterId and routes to `lobby`. App's existing AuthorView receives `selectedType === 'character' ? selectedId : null`. Supply that SAME identity and callback to WorldBuilderWorkspace; do not invent a pending route or bypass the SDK/lobby.

- [ ] Extract/reuse this sequence for both legacy AuthorView and World Builder. Keep legacy AuthorView behavior/tests working. No character means Play is disabled with the existing “Pick a character on Home to play” explanation; do not create a replacement character or lobby.
- [ ] Track request generation/mount/character identity and check between asynchronous steps and before navigation. A stale/unmounted request must not initiate its next step or redirect the current editor. A failure names its phase, remains in the editor and preserves the draft.
- [ ] Block duplicate launch, Back, room/prop mode switching and document replacement/edit actions during the actual save-and-launch transaction. Preserve existing leave confirmation when idle. Do not rely on disabled HTML buttons alone while canvas/global keyboard handlers still mutate the source: guard the actual commit/undo/import/new-room paths or apply a cohesive busy interaction boundary. Ordinary background validation must not freeze editing.

## 2. Root key, server validation and save

**Modify/add:** a focused room publishing panel/hook under `src/concepts/world-building/`, wired from WorldBuildingConcept/Workspace; `src/author/authoringRpc.ts` only for narrow generation/identity improvements with legacy regression coverage.

Actual APIs:
- `encodeSingleRoomDungeon({key,draft})` and `decodeSingleRoomDungeon(source)` already exist.
- `defaultAuthoringClient` provides `putDungeon`, `getDungeon`, `listScenarios` (there is NO new authoring list endpoint to invent).
- `usePutDungeonPreview(key,yaml,options)` debounces validateOnly=true and keeps last good atlas.
- `useSaveDungeon(client).save(key,yaml)` sends validateOnly=false and returns boolean; the server validates before persistence, so a final successful save proves the exact submitted YAML, not a stale preview.

- [ ] Provide a clearly labelled root `Dungeon key` control. Default is a stable value derived from the existing room ID only when it forms a valid API key, e.g. `room-${draft.id}` for `[a-z0-9-]+` IDs. For an unsuitable ID leave the key empty and request an explicit key; do not silently slug/collapse identities or use the scout's arbitrary shared `workshop-room` default. Room-name edits never rename a key automatically.
- [ ] Key is editor publication state, NOT a field added to strict RoomDraft or room-document envelopes. Keep it stable across ordinary edits/renders; reset intentionally for a different room identity. Canonical YAML export/import retains its root key through the existing adapter. Local room JSON/snapshots remain room-only: explain that distinction rather than invent a second persisted scene/schema or silently add a key field.
- [ ] Before first saving to an existing key in this editor context, read with existing getDungeon and require explicit overwrite confirmation. NotFound permits a new save; other read errors refuse visibly. A key previously saved by this same current document may update normally. A new room/key/source identity invalidates pending confirmation. Cancel changes no stored bytes. No silent overwrite, auto-renaming or fallback dungeon.
- [ ] Reuse existing preview/save hooks or narrowly extend their request-identity support, rather than duplicating an RPC system. Existing useSaveDungeon lacks generation fencing; preview cleanup does not currently invalidate an in-flight response on unmount. Ensure old responses cannot replace current error/saved/busy state or initiate Play. Capture key + exact emitted YAML + room identity + request generation; never validate A and save/launch B accidentally.
- [ ] Show provider field paths/messages and transport failures without changing room history or local/world ownership. Keep known draft bytes and in-memory work intact on all refusals. No local floor/footprint/occupancy/LOS validation. Preserve a last-good validation result with truthful stale status, not as permission to launch stale source.
- [ ] Save & Play performs a successful save of the exact captured current document before invoking the shared launch flow. A failed/invalid save cannot create a lobby. The final save already validates server-side; do not add redundant compile RPC loops just for ceremony.
- [ ] Room mode's explicit name edit must make the published RoomDraft.name match the user's edited room title (and keep the editor's scene-name presentation coherent). Do this only on the explicit rename transaction, with one Undo; do not normalize distinct imported names on every unrelated edit. The current commit helper retains the old draft.name, so inspect that seam rather than silently publishing every newly named room as “Untitled room”.
- [ ] Keep publishing controls/RPC effects out of prop-only mode and the existing local-only concept mount. Inject capability/callbacks from the World Builder route. An unavailable/disabled authoring service is an explicit unavailable state, never a fixture fallback.

## Required focused proof

Use real component interactions and deferred mocked RPC promises; no tests that only assert hook names:
- App/Workspace passes the selected character and uses the existing onPlay route with returned encounter ID; legacy AuthorView still uses the same shared sequence.
- Exact rich v3 YAML/key submitted (full graph, height/light/false flags), server failure preserves draft/history/local bytes; successful save precedes createLobby→ready→start→route with the same key/character.
- Missing character disables Play; Save remains available. Duplicate clicks produce one transaction. Failure at each real asynchronous seam surfaces without navigation.
- Stale validation/save/Get-for-overwrite/launch results after key/room/character change or unmount do not alter current state or trigger later requests/navigation. In-flight transaction blocks actual source-changing controls/keyboard/canvas and Back/mode switch.
- Existing-key confirmation is explicit and fenced; cancel sends no write. New key saves without overwriting another key.
- Stable default key, explicit custom key, canonical YAML key round-trip, no key in RoomDraft/room envelope. Explicit name edit persists to provider-facing name with one Undo; unrelated edits retain imported names.
- Run the existing room draft/document/ownership/marker suites as focused regression coverage. Keep M1 preview and completed runtime decoder/render tests green.

Stop if an actual provider capability is missing. API1004's unmerged status is not itself a local-development blocker; no runtime operation is requested from this worker. Return source-only completion, exact files and focused evidence, remaining parent-owned integrated browser/CI/publication/review tasks. Do not claim the whole playable-room feature accepted until that proof is done.
