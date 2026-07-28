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
| **rpg-dnd5e-web** | **`origin/dev`** — work lands there, then `dev` → `main` as one batch (rpg-dnd5e-web#630). `dev` can be cut to prod at any point; there is no deployment pipeline yet. |
| **rpg-api** | **`origin/dev`** — same integration-branch model as web, above. |
| everything else | `origin/main` |

`dev` replaced `development` (web) on 2026-07-28, when rpg-api grew a matching `dev`
branch — see [Merging a `dev` branch into `main`](#merging-a-dev-branch-into-main)
below for the rule that came with it.

**Always `git fetch` first and cut from the `origin/` ref, never from a local branch.**

```bash
git fetch origin
git checkout -b feat/123-thing origin/main          # or origin/dev for web / rpg-api
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

### Merging a `dev` branch into `main`

Squash in, merge out — mixing the two breaks a release.

- **Into `dev`: squash.** Every feature PR squash-merges into `dev` — one clean
  commit per feature. This is right and unchanged.
- **Out of `dev` into `main`: a real merge commit. Never squash.** Squashing the
  release cut severs ancestry between the two long-lived branches — GitHub records
  the release as a single commit with `main` as its only parent, so `dev` is never
  recorded as an ancestor of the result. The next release then has to replay `dev`'s
  whole history against a `main` that already contains that content, and it
  conflicts with itself.

Worked example, 2026-07-28, rpg-dnd5e-web: release PR #653 (`development` → `main`)
was squash-merged. Consequences, all verified:

- The merge-base between the two branches stayed pinned at `0e92197`, from before
  the whole wave that PR shipped.
- A trial merge (`git merge-tree`) of the *next* cut produced conflicts in 4 files —
  git was replaying `development`'s entire history onto a `main` that already held
  that content, just not as a recorded ancestor.
- An attempted `-X ours` back-merge **silently reintroduced a dead `Position` type
  import** in 2 files: `development` had added the import and then removed it as
  dead code in a later commit (#651), so relative to the stale merge-base
  `development` showed zero net change while `main` showed a net add, and git took
  `main`'s side. No conflict markers — a silent clean-merge artifact. It would have
  failed lint.
- Fixed by **cherry-picking** the two new commits onto `main` instead (web #654,
  merged) — cherry-pick replays each commit's own diff, which included the import
  removal, so the artifact can't occur.

**Kirk's call:** recut the branch as `dev`, give rpg-api a matching `dev` branch,
and write the rule down now instead of waiting for it to bite again.

**The general principle:** the merge-style rule has to be set the moment a
*second* long-lived branch is created, not discovered at its first release. A
single long-lived `main` never hits this — squash is fine there forever. It's
specifically the relationship between two long-lived branches that needs a real
merge to stay intact.

**Detector, run any time:** `git merge-base --is-ancestor origin/main origin/dev` —
fails the instant the branches diverge, instead of surfacing as conflicts at the
next release cut.

**What actually prevents it:** branch protection on `main`, "Require branches to be
up to date before merging." A `dev` → `main` PR then can't merge unless `dev`
already contains `main`'s tip — which a squash-merged predecessor would violate by
construction.

**What you cannot enforce:** GitHub's merge-method toggles (allow squash / allow
merge commit / allow rebase) are repo-wide settings, not per-target-branch — you
cannot allow squash for feature→`dev` PRs while forcing merge-commit for
`dev`→`main` PRs. This rule cannot live as a repo setting; it lives in this doc plus
the up-to-date check above. Worth stating so nobody goes hunting for a setting that
doesn't exist.

**rpg-toolkit is exempt, deliberately.** It has no long-lived integration branch to
protect: the board rule above already gives it a fresh branch per issue merged
straight to `main`, and it coordinates downstream consumers by version tag, not by
branch (`docs/teams/roles/rpg-toolkit-member/prompt.md`: `main` → `feat/...`, CI
tags packages on merge, consumers bump to the tag when ready). Giving it a
long-lived `dev` too would reintroduce, from the branch side, the same "three
versions of one thing" problem proto versioning (above) already had to solve on the
schema side.

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
