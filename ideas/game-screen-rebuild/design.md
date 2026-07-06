# Game Screen Rebuild: the real front end on the encounter stack

## Status: Design — approved direction (Kirk, 2026-07-06), slices not yet scheduled

## The Problem

The event-based UI architecture that Beat 2 proved out lives only in the `/playtest`
harness. The live game view (`LobbyView.tsx`, 2,425 lines, 42 legacy markers) still runs
the old v1alpha1 request/response architecture. We cannot run a true end-to-end test of
the game because one of the two ends — the actual game screen — doesn't exist on the
proven stack.

rpg-dnd5e-web#432 called the direction: clean-slate rebuild, no migration.
This design says what that rebuild is.

## Decisions (Kirk, 2026-07-06)

1. **Clean slate, no shim.** All old-style code is removed — including the v1alpha1
   lobby flow. We do not run a mixed v1-lobby/v2-combat interim. With a clean slate we
   build something better.
2. **`/playtest` is permanent.** It is where we go to verify things — never scheduled
   for deletion. rpg-dnd5e-web#391 must be rescoped: delete the v1alpha1 movement
   callbacks, keep the harness.
3. **No version-suffixed names.** We haven't launched v1; nothing internal ships named
   `v2`/`v3`. Because the old code is deleted, the rebuilt pieces take the real names
   (`useEncounterStream`, `useMoveEntity`, `ActionPanel`, …). Suffixes like `Stream2`
   existed only to coexist with legacy — coexistence ends, so the names do too.
4. **rpg-dnd5e-web#432 supersedes rpg-dnd5e-web#381.** De-duplicating legacy markers in
   a file we are deleting is wasted motion. #381 closes.

## What Exists (Survey 2026-07-06, rpg-dnd5e-web @ 70889cf)

The gap is smaller than the stale architecture docs suggest in the stream/state layer,
and larger in the combat-UI layer.

### Already shared or generic — the rebuilt game view adopts these
- `useEncounterStream2` + `encounterStream2Dispatch.ts` — v1alpha2
  `StreamEncounter` subscription + pure event dispatcher (21 event cases). Zero
  harness assumptions.
- `useEncounterState` (1,231 lines, ~30 reducers) — **already consumed by both
  LobbyView and the harness today.** The one truly shared piece; carries hybrid
  v1+v2 fields, so the rebuild trims the v1 half.
- The action-dispatch hook family: `useMoveEntityV2`, `useEndTurnV2`, `useInteractV2`,
  `useTakeActionV2`, `useSubmitCheckV2`, `useSetReactionReady`, `useActivateFeatureV2`.
- `ActionMenu` + `EconomyBar` — render the server-authored `TurnState.available_actions`
  and `ActionEconomy` verbatim. Zero client legality logic (the boundary rule, honored).
- `HexGrid` — already the single renderer behind both `PlaytestMap` and `BattleMapPanel`.

### Harness-specific — generalize, don't lift
- `PlaytestMap`'s floor-tile synthesis assumes one room (`roomId: ''`). The game needs
  room/door accumulation on the event stream (see Multi-room note below).
- Harness identity is URL-params-only (`?encounterId=&playerId=`) — the game needs the
  real lobby flow in front (see The Lobby Gap).
- Dev scaffolding (coordinate spinners, raw target-id inputs, entities table, event log)
  stays in the harness. It is the harness.

### Incompatible — rebuild, no bridge exists
The game's combat panels (`combat-v2/`, `usePlayerTurn`) get available actions from
**RPC response payloads**; the proven stack gets them **pushed on the stream** as
`TurnStateChanged`. Different delivery mechanisms, not different shapes — there is no
incremental merge. The new combat surface starts from `ActionMenu`/`EconomyBar` and adds
game-grade treatment (icons, tooltips, layout), it does not port `ActionPanel`/`ActionPanelV2`.

### Game-only capabilities to rebuild on the new stack
- Lobby: create / join-code / character select / ready-up party display
  (`LobbyScreen`, `WaitingRoom`, `PartyMemberCard`, `JoinCodeDisplay` — all v1alpha1 today).
- Equipment modal, dungeon result overlay, dungeon config selection — pure UI,
  re-plumb to the new state shape.
- Discord Activity integration (`src/discord/`) — orthogonal to the stream question,
  ports unchanged.

## The Lobby Gap (cross-repo, sequences everything)

**No lobby surface exists on the v1alpha2 encounter stack.** `CreateEncounter` /
`JoinEncounter` / `SetReady` are v1alpha1-only. Because we ship no shim, the boarded
trailblazer "4-player join/lobby on the v2 encounter stack" (board #19, Party Assembles)
**folds into this effort as slice 1** — it is the prerequisite, not a sibling. This
matches journey order: Party Assembles before The Dungeon.

Boundary shape (stated, per the boundary rule — not open for re-derivation): the API
owns join codes, membership, ready state, and encounter lifecycle; the toolkit owns
nothing lobby-shaped (a lobby is orchestration, not rules); the web renders membership
and dispatches join/ready by reference.

## Target Shape

```
App
└── GameView                      (successor to LobbyView — new file, new name)
    ├── LobbyFlow                 (create/join/ready on the new RPC surface)
    │   ├── CharacterSelect
    │   └── PartyRoster           (join code display, ready states)
    └── EncounterView             (mounts when encounter starts)
        ├── useEncounterStream    (renamed from useEncounterStream2)
        ├── useEncounterState     (trimmed: v1 fields deleted, single shape)
        ├── EncounterMap          (HexGrid + room/door accumulation from stream events)
        ├── ActionPanel           (game-grade surface built on ActionMenu/EconomyBar)
        ├── Equipment / ResultOverlay (re-plumbed)
        └── prompts               (skill check, reaction — lifted from harness modals)
```

`/playtest` keeps rendering through the same hooks and shared components — that is what
makes MCP harness verification a proof of the game path, permanently.

## Deletion list (the point of clean slate)

`LobbyView.tsx`, `LobbyScreen`/`WaitingRoom` v1 wiring, `useEncounterStream` (v1),
`useCreateEncounter`/`useJoinEncounter`/`useSetReady` (v1alpha1), `usePlayerTurn`,
`combat-v2/ActionPanel.tsx` + `ActionPanelV2.tsx`, `useDungeonMap` (v1), the v1 half of
`useEncounterState`, and the five inline transform functions in `LobbyView.tsx`.
Rename pass after deletion: every `*2`/`*V2` identifier loses its suffix.

## Slices

1. **Lobby surface** (protos + rpg-api + toolkit-none): join/ready/lifecycle RPCs on the
   encounter stack. Design detail in its own plan doc before implementation.
   Verify: 4 real clients join one encounter (the boarded Party Assembles story).
2. **EncounterView core** (web): GameView renders a live encounter through the shared
   hooks — map, ActionPanel on the push model, prompts. Single room is fine here.
   Verify: MCP playtest drives the *game route*, not `/playtest`, through a full fight.
3. **Delete + rename** (web): legacy path removed, suffixes dropped, `useEncounterState`
   trimmed to one shape. Verify: grep gates (`v1alpha1`, `V2`, `legacy` markers) + full
   playtest re-run on both `/playtest` and the game route.
4. **Multi-room accumulation** (web, later): room/door accumulation on stream events —
   scheduled with The Dungeon leg's multi-room trailblazer, not before.

Slices 2 and 3 could land as one wave if the diff stays reviewable; 1 blocks 2.

## Docs close-the-loop (in the wave, not deferred)

rpg-dnd5e-web `docs/architecture/*` (except `playtest-harness.md`) is dated 2026-05-02
and materially wrong — `use-encounter-state.md` documents 148 lines/3 functions vs the
actual 1,231/~30; the stream-2 layer and the ActionMenu subsystem have no architecture
docs at all. The rebuild waves refresh these pages as they touch each area.

## Issue disposition

- rpg-dnd5e-web#432 — this design is its design phase; stays open as the umbrella.
- rpg-dnd5e-web#381 — closed, superseded (Decision 4).
- rpg-dnd5e-web#391 — rescoped: movement-callback cleanup only; the harness lives.
- Board #19: "Trailblazer: 4-player join/lobby" and the Game Screen leg items now
  sequence under this design (slice 1 and slices 2–4 respectively).

## Load-bearing digest

- The stream/state plumbing is already shared between harness and game; the rebuild's
  real work is the lobby RPC surface (doesn't exist) and the combat UI cutover
  (push-model vs response-model — no bridge).
- No shim: the v1alpha1 lobby dies with the rest, so slice 1 is cross-repo and gates
  the web work.
- `/playtest` is a permanent verification surface sharing the game's hooks — never a
  deletion target again.
- Names: the rebuild takes the unsuffixed names; `v2` suffixes end with the legacy code.
