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

## How We Work — three lanes

> Kirk, 2026-07-24: *"the assets, ui/ux and platform is the new way ... DoD becomes
> built in and boundaries established ... how we work is as or more important than
> what we work on."*

**Three lanes own the work: Assets, UI/UX, Platform.** A lane owns an *outcome* end
to end, across whatever repos that outcome touches. Repos are territory; lanes are
ownership.

| Lane | Owns | Spans |
|------|------|-------|
| **Assets** | Licensed source → conversion → contract tree → manifests → what renders in-world: models, environment, animation playback, 3D evidence, shipped-asset budgets | rpg-game-assets + the 3D half of rpg-dnd5e-web |
| **UI/UX** | What the player reads and touches: screens, HUD, accessibility, presentation, sending intent | rpg-dnd5e-web |
| **Platform** | The rules and the wire: game rules, by-key orchestration, contract shape, delivery | rpg-toolkit, rpg-api, rpg-api-protos, rpg-deployment |

**Why lanes and not one agent per repo.** A repo-scoped split cuts *across* outcomes:
one crypt-monster slice touches rpg-game-assets **and** rpg-dnd5e-web, and the handoff
between two repo owners is exactly where intent leaked. Lane ownership keeps one owner
accountable for the whole outcome. The per-repo member charters
(`docs/teams/roles/rpg-*-member`) still exist and still carry each repo's architectural
boundary — they are now the *implementation detail* of a lane, not the unit of ownership.

**The seam between lanes is the thing being protected.** These docs were written when
the boundaries had to be taught from scratch. Newer models read this architecture
natively, so the docs no longer need to explain the obvious — they need to record the
**decisions** and defend the **seams**. Prefer deleting scaffolding that teaches what is
now assumed; keep anything that encodes a decision or protects a boundary.

### Definition of done is built in

Not a gate someone remembers to apply — it travels with the work. Nothing is done until:

- it is **boarded** (issue-first, one issue per PR), and the decision is visible on the
  board **and** the PR — narrative alone is not a decision;
- **CI is green — necessary, never sufficient**;
- layered review has run: Copilot (code repos), the implementer's own `/code-review`
  pass, and the director's review of the returned evidence;
- **evidence is captured and stated as viewed, not inferred** — the MCP playtest is the
  sign-off bar for anything a player sees;
- the PR is open **ready for review, never draft** — a draft is invisible work;
- Kirk decides and merges.

### The board is the living cross-lane document

It is what each lane **sees**, not a task tracker. Its real job is the cross-lane
question: *this matters for UI — can Platform help?* Lanes negotiate there in the open.
If a lane needs something from another lane, that need belongs on the board where the
other lane can see it, not in one lane's private narrative.

### All of this is revisable

Anything here can change if it improves — **we must improve to keep up.** If a
convention has stopped earning its place, say so and propose the replacement rather
than quietly working around it. How we work is as important as what we work on.

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

## Cross-Repo Design Workflow

Cross-repo designs and plans live under `ideas/<topic>/` **in this repo**, never scattered as
scratch docs in an implementing repo. Flow:

1. Start an idea branch from **latest `origin/main`**, add `ideas/<topic>/design.md`, open an
   `rpg-project` PR — this PR is Kirk's review surface.
2. Design is reviewed and approved first. `plan.md` is added to the **same PR**, after design
   approval — not before, not as a separate PR.
3. Implementation happens in the owning repo(s) as their own PRs, referencing the approved design.
4. Merge the `rpg-project` idea PR only **after** implementation is complete — it stays open as
   the tracking surface until then.

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
    howto/               <- Operational runbooks (run the game locally, etc.)
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
