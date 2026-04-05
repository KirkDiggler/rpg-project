# Round 2 Briefing: Multi-Room Dungeon

**Start here.** This document provides full context for continuing from Round 1.

## What Was Accomplished (Round 1)

Round 1 validated the scenario-based development rounds system. A Level 1 Monk can:
- Select and use unarmed strike with correct damage (1d4 + DEX)
- See monster HP change (bloodied → dead with tilted grey model)
- See correct AC in combat log (Unarmored Defense WIS included)
- Fight across multiple turns with action economy resetting correctly
- See own HP update when hit by monsters

All PRs merged to main across all repos. See `docs/teams/playtests/round-1-monk-room1.md` for full results.

**Known intermittent:** "Insufficient action" after EndTurn sometimes. Not reproducible.

## Round 2 Scenario

**"Player clears room 1, walks through door, enters room 2 with new encounter"**

This is the multi-room dungeon experience. The design exists at `ideas/multi-room-dungeons/design.md` with a 5-phase plan. The critical blocker is **Phase 1 (API)** — room origin coordinates aren't being threaded through to the client.

## Current State by Layer

### Toolkit (READY)
- Absolute cube coordinates calculated via BFS in `GraphGenerator`
- `ToAbsolute()` / `ToLocal()` coordinate conversion helpers exist
- Room positions tracked in `BasicEnvironment`
- Multi-room spatial infrastructure is done

### API (BLOCKED — Phase 1 needed)
- `convertRoomDataToProto()` doesn't set `Origin` field
- `OpenDoor()` returns `RoomOffset: nil` with TODO
- Room positions not stored on dungeon entity
- Existing design: `ideas/multi-room-dungeons/design.md` Phase 1
- Key issues: #426 (plan), #397, #393, #399, #407

### Web (BLOCKED on API)
- `useDungeonMap` hook exists with room accumulation logic
- `InstancedHexTiles` renders from (0,0) single room only
- Needs room origin from API to place rooms correctly
- Key issues: #158, #311, #335

### Protos (READY)
- `Room.origin` field exists (field 9, type `Position`)

## Suggested Approach

**Phase 1 first (API):** Wire room origins through the API — this unblocks everything else.
1. Store room positions during dungeon generation
2. Populate `Room.origin` in proto conversion
3. Thread origins through `StartCombat` and `OpenDoor` handlers

**Then parallel (Web + API):**
- Web: Accumulate revealed rooms with origins, render multi-room hex grid
- API: Handle room transitions, entity movement across rooms

## Key Design Decisions (Already Made)
- Toolkit owns positioning (not client math)
- Cube coordinates (q,r,s) for all hex grids
- BFS room origin calculation during dungeon generation
- Phased approach (learned from PR #400 being too large)

## Files to Read
- `rpg-project/ideas/multi-room-dungeons/design.md` — Full architecture
- `rpg-project/ideas/multi-room-dungeons/CLAUDE.md` — Scope and key files
- `rpg-project/docs/teams/roles/` — Team role prompts with context directories
- `rpg-project/ideas/scenario-rounds/design.md` — How rounds work

## How to Start

1. Read this briefing and the multi-room design doc
2. Co-plan acceptance criteria with Kirk
3. Create Round 2 tracking issue
4. Dispatch workers (API fixer first — critical path)
