# Wave 2.7 — OpenDoor (Player opens a door, room 2 contents render)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** Player opens a door, room 2 contents render via v2 events on both browsers — the same per-viewer projection that 2.5 movement validated, applied to a new verb shape (geometry events).

**Verified by playtest:** alice clicks an open-door button in `/playtest`; v2 server runs `encounter.OpenDoor()`; broker emits `DoorOpenedEvent` + `HexRevealedEvent` per viewer; both browsers' v2 streams render the new geometry. Alice (door-opener) sees the door change state and revealed hexes appear; bob sees only what his PerceptionView permits.

**Depends on:**
- Wave 2.5 (movement, per-viewer projection, v2 service registration, broker, translator) — shipped in PR rpg-api#496 + rpg-dnd5e-web#389.
- Wave 2.6 (CreateEncounter, GetEncounter, snapshot replay) — shipping under rpg-project#19. By the time 2.7 implementers pick up, the harness creates encounters via RPC and snapshot replay populates initial state. 2.7 builds on that lifecycle, doesn't reinvent it.

**Architecturally clean — no toolkit work:**
The rpg-toolkit `encounter` package already implements `Encounter.OpenDoor(playerID, doorID)` (encounter.go:244), `Encounter.AddDoor(id, position, open)` (encounter.go:69), `events.DoorOpenedEvent` (events/door_opened.go), and the per-viewer projection helper `perception.ProjectDoorOpen` (perception/project.go:138). Slice 1's PR description (rpg-api#496) already noted this verb exists. **Wave 2.7 is purely the rpg-api Interact handler + rpg-dnd5e-web event consumer + harness button — no toolkit changes needed.**

---

## Inner-work shape

### rpg-api — implement `Interact` for door interactions

The proto `Interact` RPC is already registered with `codes.Unimplemented` (rpg-api#496 slice 1). This wave implements it for the door-open case:

- `target_entity_id` resolves to a `DoorData` entry via `*encounter.Data.Doors`. If absent → `NotFound`.
- `interaction_kind` is optional; for a door, default behavior is "open". Future kinds (`examine`, `loot`, `disarm`) are out of scope.
- Authority: caller must be a player in the encounter. The toolkit's `OpenDoor(playerID, doorID)` already gates by player membership (returns error if `playerID` not in `data.Players`); the handler maps that to `PermissionDenied` or `FailedPrecondition` per `pat-v2-status-code-mapping`.
- "Already open" → `FailedPrecondition` (toolkit returns an error in this case).
- The handler load → call → save loop matches `MoveEntity`:

```go
func (h *Handler) Interact(ctx context.Context, req *encounterv2pb.InteractRequest) (*encounterv2pb.InteractResponse, error) {
    playerID := auth.GetPlayerID(ctx)
    // ... validation ...
    data, err := h.encRepo.Get(ctx, req.GetEncounterId())
    // ... NotFound mapping ...
    enc, err := encounter.LoadFromData(data, h.broker)
    // ... Internal mapping ...
    if err := enc.OpenDoor(core.PlayerID(playerID), core.EntityID(req.GetTargetEntityId())); err != nil {
        // toolkit-level errors → FailedPrecondition (per pat-v2-status-code-mapping)
        return nil, status.Errorf(codes.FailedPrecondition, "open door: %v", err)
    }
    if err := h.encRepo.Save(ctx, enc.ToData()); err != nil { /* Internal */ }
    return &encounterv2pb.InteractResponse{}, nil
}
```

Translator additions in `internal/handlers/dnd5e/v2/encounter/translate.go`:

- `*events.DoorOpenedEvent` → `EncounterEvent_DoorOpened`. Per-viewer slice's `Visible` flag gates whether to send (`Visible: false` → `ErrViewerSawNothing`). Only `door_entity_id` is on the proto from the toolkit event itself; revealed hexes / walls ride on the parallel `HexRevealedEvent` (already mapped).
- The `revealed_hexes` field in proto `DoorOpened` could be populated by buffering the parallel `HexRevealedEvent` — but **don't** combine the events at the handler level. The two-event split (cause + effect) is the toolkit's deliberate decoupling (`events/hex_revealed.go:9-12`); the web reducer composes them. Translator emits two proto events; web glues them visually.

The `HexRevealedEvent → GeometryRevealed` translation already exists from slice 1 — verify it still applies cleanly here (it should; same shape).

### rpg-dnd5e-web — consume `DoorOpened`, add open-door harness button

- New callback `onDoorOpened` on `EncounterStream2Options`. Add the typed case to `dispatchEncounterStream2Event` in `src/api/encounterStream2Dispatch.ts`. Test extension matches the existing per-event-case pattern.
- New hook `src/api/useInteractV2.ts` mirroring `useMoveEntityV2.ts` shape (one file per verb per `v2-rpc-hook-pattern`).
- Reducer for door-state in `useEncounterState`. When a door opens, the door's wall kind transitions from `DOOR_CLOSED` to `DOOR_OPEN` in the local model. The `GeometryRevealed` reducer (already exists) handles new hexes. Discovery: walls revealed by the door open are part of `revealedWalls` in `GeometryRevealed` payload — confirm shape.
- `PlaytestHarness` gains an "Open door" section: text input for `targetEntityId`, button that calls `useInteractV2`. Same shape as the move section. The harness's seeded encounter (Wave 2.6 shipped CreateEncounter) gets a door added — pick a known id like `door-east` matching the toolkit integration test fixture.
- The harness must seed a door. After Wave 2.6, the harness calls `CreateEncounter` to spawn a fresh encounter; Wave 2.7 needs that encounter to contain a door for the verb to act on. Two options:
  1. Extend `CreateEncounter` to seed a starter scenario (door + room 2 hexes hidden behind it). Likely the cleanest if Wave 2.6 already did similar fixture work.
  2. Add a separate seed step (toolkit `enc.AddDoor(...)` via the harness, similar to slice 1's direct-construct path).

  **Decision deferred to dispatch** — depends on what shape Wave 2.6's CreateEncounter ended up taking. Pick at execution time; document the choice in PR description.

### rpg-toolkit — out of scope

`Encounter.OpenDoor()` exists. `events.DoorOpenedEvent` exists. `perception.ProjectDoorOpen` exists. Wave 2.7 consumes; does not extend.

If during execution a toolkit gap surfaces (e.g., `DoorClosedEvent` needed for closing doors, or door state not snapshot-replayed because it's not in `SnapshotFor`), file a toolkit issue, place on board with Wave 2.7 (or future wave if not blocking the goal), and continue. Wave goal is OpenDoor — door-close is post-wave.

---

## Inner-issue list (filed on board #11 with Wave 2.7 + Status: Todo)

- 🎮 **Tracker** (rpg-project): wave goal sentence + sign-off + contents checklist
- **rpg-api**: implement `Interact` RPC for door interactions (handler + translator extension + integration test)
- **rpg-dnd5e-web**: consume `DoorOpened` event in v2 stream + add open-door harness button (`useInteractV2` hook + dispatch + harness UI)
- **chore: Wave 2.7 close-the-loop** (rpg-project): file followups, update board, draft Wave 2.8 plan, update role context with new patterns

---

## Out of scope for Wave 2.7

- `DoorClosed` (closing doors). Door close is a separate verb path; file as Wave 2.7+1 candidate if the wave goal needs it (it doesn't — verifying open is sufficient).
- Locked doors / `InputRequired{skill_check}` for unlock. The InputRequired plumbing is Wave 2.10 (Interact + SubmitCheck wave per roadmap). Wave 2.7 only handles unlocked doors.
- `Interact` for other targets (chest, lever, NPC, trap). Same RPC; different consequences; out of scope. File as Wave 2.10 fodder if not absorbed elsewhere.
- LobbyView migration to v2. Wave 5.
- Door state in snapshot replay. If Wave 2.6's snapshot replay doesn't include door state, file a followup (probably already in scope of #497). Wave 2.7's playtest might lean on the assumption that the harness's freshly-created encounter has all doors closed (no replay needed because nothing has happened yet).

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.7's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality.

**rpg-api-member additions:**

- `pat-v2-interact-handler-shape` *(anticipated)* — Interact handler is structurally similar to MoveEntity (load → call toolkit verb → save) but uses `target_entity_id` instead of `path`. The verb dispatched depends on the target's type (door → `OpenDoor`). For Wave 2.7, only door handling is wired — future Interact wires (chest, lever, trap) extend with their own dispatch arms in the same handler method.
- `pat-v2-cause-effect-event-split` *(anticipated)* — The toolkit emits cause + effect events as separate publish calls (`DoorOpenedEvent` + `HexRevealedEvent` for OpenDoor). The translator emits two proto events on the wire; the web composes the visual response. Do NOT collapse the two events into one at the rpg-api layer — the split is deliberate (`rpg-toolkit/encounter/events/hex_revealed.go:9-12`).

**rpg-dnd5e-web-member additions:**

- `pat-v2-door-state-reducer` *(anticipated)* — When a `DoorOpened` event arrives, the reducer transitions the door's wall kind from `DOOR_CLOSED` → `DOOR_OPEN` in local state, AND the `GeometryRevealed` reducer (separate event) adds the newly-visible hexes. Two events, two reducer calls — the visual coalescing is render-time.
- `pat-v2-harness-verb-button-shape` *(anticipated)* — Wave 2.5 established move-button pattern; Wave 2.7 establishes the verb-button-with-target pattern (target entity id input + verb button). Same overall shape, target-input pattern reusable for future verbs.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

---

## Execution sequence

1. **Wave 2.6 close-the-loop** must complete first. That step writes the verified Wave 2.6 patterns into role context, which Wave 2.7 dispatches read.
2. **rpg-api Interact issue** dispatches; merges PR; verified by integration test extending `EncounterV2IntegrationSuite` with a two-player door-open scenario (alice opens door-east; both alice and bob receive `DoorOpened` and `HexRevealedEvent`-derived `GeometryRevealed` consistent with their PerceptionView).
3. **rpg-dnd5e-web Interact issue** dispatches; merges PR; harness has open-door button that drives a real door interaction.
4. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`): alice clicks open-door, both alice and bob see the door open + hexes reveal in their respective viewports.
5. **chore: Wave 2.7 close-the-loop** dispatches: files followups, updates board, drafts Wave 2.8 plan, captures verified patterns into role context, retros on tracker.

---

## Reference

- Wave 2.7 tracker: rpg-project#<filled in by close-the-loop dispatch>
- rpg-api Interact handler: rpg-api#<filled in by close-the-loop dispatch>
- rpg-dnd5e-web Interact consumer: rpg-dnd5e-web#<filled in by close-the-loop dispatch>
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md` §4 (RPC shapes)
- Wave 2.6 plan / artifacts (template): `rpg-project/ideas/encounter/v1alpha2/plans/03-rpgapi-walking-skeleton.md` and rpg-project#19, rpg-api#499/#500/#497
- Slice 1 PR (translator + handler shape): rpg-api#496
- Toolkit OpenDoor: `rpg-toolkit/encounter/encounter.go:244`, `rpg-toolkit/encounter/events/door_opened.go`
- Board: https://github.com/users/KirkDiggler/projects/11
