# RPG Project - Session Context

This is the brain for the RPG platform. Read this first in any session.

## What We're Building

A multiplayer D&D 5e dungeon crawler playable as a Discord Activity. Players create characters, join lobbies, and fight through procedurally generated multi-room dungeons together.

## The Repos

| Name | Role | Key Phrase |
|------|------|-----------|
| **rpg-toolkit** | Rules engine | "Knows what Rage does" |
| **rpg-api** | Game server / data orchestrator | "Knows feature keys, not behavior" |
| **rpg-dnd5e-web** | React UI in Discord Activity | "Renders protos, sends intent" |
| **rpg-api-protos** | Contract definitions | "Source of truth for API shape" |

## Vocabulary

Use these terms consistently:

- **Game server** = rpg-api
- **Toolkit** / **Rulebook** = rpg-toolkit (specifically rulebooks/dnd5e for D&D rules)
- **The web** = rpg-dnd5e-web
- **Activate** = Player-initiated action with resource cost (Rage.Activate consumes a use)
- **Apply** = Subscribe to event bus for passive/ongoing effects (RagingCondition.Apply listens for damage)
- **Chain** = Staged modifier pipeline (base -> features -> conditions -> equipment -> final)
- **Feature** = A core.Action that can be activated (costs resources, does something)
- **Condition** = An applied effect that listens on the event bus (modifies chains passively)
- **Reference** = A key like "dnd5e:features:rage" - client sends these, never behavior

## The Boundary Rule

```
Client sends REFERENCES (keys, IDs) -> never calculations
API orchestrates by KEY            -> never knows what "rage" does
Toolkit implements RULES           -> returns rich breakdowns for rendering
```

If you see game logic in the API, say something. If you see calculations in the client, say something.

## Project Board

**ALWAYS check before starting work:**
https://github.com/users/KirkDiggler/projects/10

Rules:
- One issue per PR
- No branch without an issue
- No issue without a board entry
- New work = new issue on board -> fresh branch from main

## Current Milestone

**4-Class Multiplayer Multi-Room Dungeon**

See `progress/` for current state.

## Documentation Structure

```
rpg-project/
  CLAUDE.md          <- You are here. Read this first.
  docs/
    architecture.md  <- 3-layer model, trust boundaries, data flow
    vocabulary.md    <- Extended glossary with examples
    boundaries.md    <- What each layer knows and doesn't know
  ideas/             <- Active design explorations (cross-repo)
  progress/          <- Milestone tracking, what's merged/open
```

## Per-Repo CLAUDE.md Files

Each repo has its own CLAUDE.md for repo-specific patterns:
- rpg-toolkit: Module workflow, testing, JSON serialization, spawn phases
- rpg-api: Outside-in development, Input/Output types, mock organization
- rpg-dnd5e-web: Proto hooks, ci-check, Discord integration
- rpg-api-protos: buf workflow, branch strategy, naming conventions

## Key Architecture Decisions

- **Input/Output types on every function** - Non-negotiable
- **Never return (nil, nil)** - Always valid object or error
- **Events for multiplayer** - Only one player sees the RPC response; events broadcast to all
- **Absolute positioning in toolkit** - Rooms have absolute coords, not UI-offset
- **Toolkit types are canonical** - API stores them directly, one conversion point at handler/proto boundary
- **Outside-in development** - Start at handler, work inward
