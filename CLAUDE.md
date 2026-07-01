# RPG Project - Session Context

This is the shared brain for the RPG platform. Read this first in any session.

> **Start every session here:** read [`sessions/active.md`](sessions/active.md) — the living handoff: current state, decisions, and next steps. It's the freshest narrative. If the board or open PRs look like they disagree with it, trust the handoff's direction and reconcile the board — it can lag a fresh decision (e.g. a just-superseded PR may still read "open").
>
> **Picking up as the technical director?** Read [`docs/teams/roles/director/prompt.md`](docs/teams/roles/director/prompt.md) + [`field-notes.md`](docs/teams/roles/director/field-notes.md) **first** — that's who you are and how you operate (thin: orchestrate + verify, never hands-on). Your `feedback_*` memories auto-load. That plus the handoff is enough to start directing — do **not** pull the whole world into your context.

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
- https://github.com/users/KirkDiggler/projects/13 — **Chapter 2: Combat Verbs** (current; verb-shaped waves on the v1alpha2 route; umbrella issue rpg-project #54, first wave = TakeAction)
- https://github.com/users/KirkDiggler/projects/11 — **Chapter 1: Architecture Honesty** (the clean rails Chapter 2 rides on; umbrella rpg-api #574)

_Board #12 "Chapter 2: The 4 Brothers" closed 2026-06-01 — superseded by #13; the 4 brothers are now the cast that verifies each verb wave._

Rules:
- One issue per PR
- No branch without an issue
- No issue without a board entry
- New work = new issue on board -> fresh branch from main

## Current Chapter

**Chapter 2: Combat Verbs** (board #13) — bring the v1alpha2 `EncounterService` verbs to life one
verb-shaped wave at a time, each validated against the **North-Star Invariants** in
`ideas/encounter/v1alpha2/design.md`. First wave = **TakeAction end-to-end** (umbrella rpg-project
#54): unify the toolkit's encounter verb path with the character action-menu/economy, add the
resolved-action event + correlation id + timestamp to the event spine, project menu/economy
verbatim, push the economy delta. Wave doc: `ideas/encounter/v1alpha2/take-action/design.md`
(validated 2026-06-01).

**Foundation — Chapter 1: Architecture Honesty** — the clean rails Chapter 2 rides on: one private
`load(id)` per orchestrator method (killed the #684 double-apply class), handlers as ~20-line pure
translation with zero rulebook imports (depguard-enforced), toolkit owns all rules and drives the events.

- **Live state + next steps:** `sessions/active.md` (read first — freshest narrative)
- **Design (don't re-derive):** `ideas/encounter/v1alpha2/{design,orchestrator-design,plan,roadmap}.md`
- **The 4 brothers** (L1 Barbarian/Fighter/Monk/Rogue) are the **cast** that verifies each verb wave on #13 — not a board axis of their own (the old #12 board is closed).

_Prior milestone (4-Class Multiplayer Multi-Room Dungeon, `milestones/4class-dungeon/`) is largely
delivered; its remaining gaps fold into the chapters above._

## Status docs

CLAUDE.md is the **table of contents**; the **status docs hold the live state**. They follow a fixed shape so a cold session reads a known structure instead of mining dense prose — and so the solid-vs-open discipline is built into the format.

**`sessions/active.md`** — the living handoff. ONE file, rewritten (not endlessly appended) each session:

| Section | Holds |
|---------|-------|
| **Now** | chapter / wave / what this session is driving (1–2 lines) |
| **Solid** | verified — keep, don't re-derive |
| **Open questions** | verify before acting; NEVER logged as findings |
| **Next** | the immediate gated step |
| **Decision log** | this wave's decisions, dated, + where each is visible (board / PR) |
| **Pointers** | where the deep design / code lives |

Narrative history lives in git, not in the doc — the handoff stays thin and current.

**Per-repo `docs/status.md`** (all 5 repos) — same idea, repo-scoped: Now / Health / In flight / Known rough edges / Pointers. (`docs/quality.md` keeps its A–D scorecard.)

## Structure

```
rpg-project/
  CLAUDE.md              <- You are here. Read this first.
  schemas/               <- Memory type definitions (the contract)
  docs/
    architecture.md      <- 3-layer model, trust boundaries, data flow
    vocabulary.md        <- Extended glossary with examples
    boundaries.md        <- What each layer knows and doesn't know
  milestones/
    4class-dungeon/
      CLAUDE.md          <- Milestone scope and context
      memories.json      <- Structured progress, decisions, blockers
  ideas/
    <idea-name>/
      CLAUDE.md          <- Idea scope and context
      design.md          <- Full design exploration
      memories.json      <- Structured progress, decisions, test criteria
    archive/             <- Superseded designs
  assets/
    CLAUDE.md            <- Asset pipeline: what's here, how it maps to web
    models/              <- OBJ/MTL files (body, heads, weapons, equipment, hair)
    textures/            <- PNG textures with marker colors for shader swapping
    coords/              <- JSON position/rotation data for attachments
    shaders/             <- JS shader implementations from teammate
    docs/                <- Integration guides and proposals
```

## Memory System

This repo uses typed memories at every scope. See `schemas/` for type definitions.
Each scope (project, milestone, idea) has a `memories.json` with structured entries.
Types: task-progress, decision, blocker, test-criteria, known-issue, note.

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
