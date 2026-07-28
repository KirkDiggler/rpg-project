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
- New work = new issue on board -> fresh branch from that repo's **base branch** (below)

### Base branches — check before you cut a branch

Not every repo bases off `main`. Getting this wrong sends a PR at the wrong target
and is not obvious from the code.

| Repo | Base for new work |
|------|-------------------|
| **rpg-dnd5e-web** | **`origin/development`** — work lands there, then `development` → `main` as one batch (rpg-dnd5e-web#630). `development` can be cut to prod at any point; there is no deployment pipeline yet. |
| everything else | `origin/main` |

**Always `git fetch` first and cut from the `origin/` ref, never from a local branch.**

```bash
git fetch origin
git checkout -b feat/123-thing origin/main          # or origin/development for web
```

A local `main` is only as fresh as your last pull, and in this workspace it is
routinely behind — the checkouts are long-lived and several worktrees share them.
Branching from a stale local base silently puts you a commit or two back, which
surfaces later as a phantom conflict or as "that fix isn't in my branch" (both hit
this workspace on 2026-07-26). Cutting from `origin/<base>` costs nothing and removes
the class. It also sidesteps the worktree rule that a branch checked out in one
worktree cannot be checked out in another.

This is deliberately recorded here rather than left in an issue. It lived only in
web#630, so a session that correctly searched rpg-project for the branch process
found nothing and briefed a teammate onto the wrong base (2026-07-26). **When a
process fact turns up in an issue or in conversation with Kirk, write it here** —
teammates and role agents come to rpg-project for the current word, and a fact that
isn't here does not exist as far as they are concerned.

We are pre-pre-alpha: nobody is playing the game. Breaking it is not a gate. Prefer
momentum and capture the learning over protecting runtime behavior.

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

## How a wave is shaped

**Develop outside-in. Merge inside-out.**

```
develop:  web concept  ->  protos  ->  api  ->  toolkit
merge:                     toolkit ->  api  ->  web
```

Develop from the edge, because the consumer is what discovers the requirement: a playable
concept proves the shape, protos transcribe what it proved, the api names the interface it
needs, and the toolkit implements that stated requirement. Nobody guesses at a layer they
don't consume.

Merge from the middle out, because a consumer can't honestly pin a provider that hasn't
shipped. The toolkit lands and tags; the api bumps to that real version; the web follows.

### One wave, one branch per repo

**A repo gets ONE branch for a wave, not one per bug found along the way.** Integration will
keep revealing things the provider must do — that is the method working, not new features.
They belong on the same branch.

This limits how a **single** wave is cut, not how many waves run at once. Parallel efforts
legitimately produce many versions of a module — that is a studio working, not churn, and
the rulebook will spread widest of all. What we did wrong on Fog of War wasn't producing
three versions. **It was producing three versions of one thing.**

Learned expensively on Fog of War (2026-07-27). The toolkit side shipped as four separate
PRs — #857, #860, #861, #863 — for what was one feature: fog-of-war support. The cost was
not theoretical:

- **#860 and #861 conflicted with each other**, in the same function, being two halves of
  the same change. On one branch that conflict is just typing. It had to be resolved twice,
  because the first resolution went stale the moment one of them merged.
- **#863 then conflicted with the result** and needed the same treatment.
- Four merges, four CI runs, four chances to land in the wrong order.
- Worst of all: the state became **unfollowable**. Kirk's own words — "I am so lost... it
  never even dawned on me to break up the toolkit work." The merge conflict was the alarm
  that something was wrong with the shape.

The rpg-api side made the mirror mistake — six stacked PRs, each fix layered on the last
because playtesting ran from the tip of the stack — and had to be collapsed into one PR
before it could ship.

**When is the provider's branch done?** When the consumer driving it stops asking for
changes. The consumer defines done. So don't open four small PRs as you go; keep one branch
that accumulates, and land it once. Same principle as an idea PR staying open as the
tracking surface until implementation completes — one layer down.

### Unmerged provider work

A Go consumer can't merge an unpublished toolkit change — there's nothing to merge, only a
version to pin. Two ways through:

- **Pre-release tags** (`encounter/v0.47.0-rc1`) for anything that must survive past one
  laptop. Committable, CI-buildable, honest.
- **A local override**, per module, for tight iteration only — never committed. See
  `rpg-api/docs/how-to/local-toolkit-override.md`. Only ever override ONE module; needing
  several at once is the signal that the wave was sliced too thin.

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
- **Breaking proto changes get a new version, not a break** - see below

## Proto versioning — when to bump instead of break

The migration marker we want is the **version**: add `v1alpha3`, move consumers one
at a time, and **the migration is done when we drop `v1alpha2`.** A break forces
every consumer to move in the same merge; a bump lets each move when it is ready.

**But that is not free, and right now it is usually not worth it.** Carrying two
packages means dual maintenance, duplicated messages, and a migration you still have
to finish. At this stage — pre-pre-alpha, nobody playing, a handful of consumers all
in this workspace — an in-place break is genuinely cheaper than the ceremony.

Worked example, 2026-07-26, Fog of War (rpg-api-protos#197/#198): removing
`GeometryRevealed`, `Space.walls`, `Hex` and `Entity.position` in place broke rpg-api
and rpg-dnd5e-web at once and made a scoped change into a three-repo migration.
**Kirk's call, with hindsight: do it again.** The coordination cost was real but
smaller than versioning it properly would have been.

So the rule is a trigger, not a default:

- **Break in place** while consumers are few, in this workspace, and can all be moved
  in one sitting. Use `buf breaking` + the `breaking-change-approved` label, and fix
  the consumers in the same push.
- **Bump the version** once that stops being true — an outside consumer, a deployed
  client we cannot update in lockstep, or a migration too large for one sitting.
  When the answer to "can I move every consumer right now?" is no, that is the signal.

**If switching a consumer between proto versions is painful, fix the pain** —
codemods, an import alias, a version constant. That tooling is what makes the bump
cheap enough to reach for when the trigger does fire, and it is cheaper than the
migrations it saves.
