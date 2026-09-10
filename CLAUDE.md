# RPG Project - Session Context

This is the shared brain for the RPG platform. Read this first in any session.

## Startup — shared board first, local continuity second

1. Derive the operator identity with `gh api user --jq .login`; use that login in Team signatures.
2. Treat Project 19 assignment as shared work. The board and issue hierarchy are the shared state; a local note is never a claim on shared focus.
3. If ignored `rpg-project/active.md` exists, read it as this operator's local continuity only. To create one, copy `docs/templates/local-active.md` to `active.md`; it is ignored by git.
4. Let the human choose the current focus before turning local continuity into action.
5. Read the selected owning repository's AGENTS.md and nearest scoped instructions before touching that repo.
6. Check `.agents/skills/` only for a matching approved skill. The canonical catalog may be empty; do not treat legacy runtime skills as automatically approved.

> **Fresh shared state:** read [`sessions/active.md`](sessions/active.md) when you need the current cross-team handoff. If the board or open PRs look like they disagree with it, trust the handoff's direction long enough to reconcile the board — it can lag a fresh decision (e.g. a just-superseded PR may still read "open").
>
> **Picking up as the technical director?** Read [`docs/teams/roles/director/prompt.md`](docs/teams/roles/director/prompt.md) + [`field-notes.md`](docs/teams/roles/director/field-notes.md) **first** — that's who you are and how you operate (thin: orchestrate + verify, never hands-on). Your `feedback_*` memories auto-load. That plus the handoff is enough to start directing — do **not** pull the whole world into your context.

> **New to working with Gary?** [`Billy + Gary — ChatGPT 101 Cheat Sheet`](docs/working-procedures/Gary_ChatGPT_101_Cheat_Sheet.md) is an optional human-facing collaboration primer. It does not replace project policy, repository instructions, code, tests, or Project 19 as authoritative sources.

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
- https://github.com/users/KirkDiggler/projects/19 — **Project 19**, the durable board for the evolving game. Boards #11–#13 are historical chapter records, not the current work source.

The current initiative is **Four-player Level-3 Dungeon**. Adopted work is
organized as **Initiative → Journey → Slice**; an unadopted capability may remain
a top-level Shaping journey until an initiative adopts it. Initiative is a
priority lens, not a permission boundary.

**Ready Journeys** is the global contributor entry point. Use **Current
Initiative** for prioritized outcomes, **Shelf → Shaping** for rough journey
starters, and the **Active Journeys** Parent issue rail for work already sliced
beneath a journey. Todo alone does not make a journey Ready.

Team and Area are shared assignment and product filters on Project 19. Assignment
means the Team is accountable for the outcome lens, not that the Team charter owns
technical commands. For implementation, read `docs/teams/roles/` for the Team
lens, then read the owning repository's AGENTS.md and nearest scoped instructions
for commands and invariants. Toolkit tests can prove a slice; a gameplay journey
closes only when its Done-when behavior is observed through the named local dev
path. Full contract:
[`ideas/team-workflow/project-19-journeys/design.md`](ideas/team-workflow/project-19-journeys/design.md).

Rules:
- One issue per PR
- No branch without an issue
- No issue without a board entry
- New work = new slice issue on Project 19 -> fresh branch from that repo's **base branch** (below)
- Local `active.md` is operator continuity only; shared work remains on Project 19 and linked issues/PRs.

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

**What actually prevents it — now live:** a repository ruleset targeting `main`
("Protect The President," both repos) sets `strict_required_status_checks_policy:
true` — the ruleset's form of "require branches to be up to date before merging."
A `dev` → `main` PR can't merge unless `dev` already contains `main`'s tip — which
a squash-merged predecessor would violate by construction. Zero bypass actors on
either ruleset, so this binds repo admins too.

**Merge method is enforceable too, and also now live.** The original version of
this section claimed GitHub's merge-method toggles (allow squash / allow merge
commit / allow rebase) are repo-wide, not per-target-branch, and concluded the
`dev`→`main` merge-commit rule could only ever live in this doc. That was wrong.
It's true of *classic* branch protection, which is exactly why it was easy to
conclude the setting doesn't exist — but repository **rulesets** are a separate,
newer system: a ruleset targets specific branches (`conditions.ref_name`), and its
`pull_request` rule type carries its own `allowed_merge_methods`. Merge method
genuinely can differ by target branch. The same "Protect The President" ruleset
sets `allowed_merge_methods: ["merge"]` on `main` in both repos
(rpg-dnd5e-web ruleset 6782045, rpg-api ruleset 6668107), with `dev` deliberately
left unprotected so feature→`dev` PRs keep squashing normally. That split is safe
because the only thing that ever targets `main` is the release cut, which should
be a merge commit anyway. Together the two rules close both halves of the
2026-07-28 failure this section documents: require-up-to-date blocks a *stale*
`dev`; merge-method-`merge`-only blocks the *severed-ancestry* squash. That
incident needed both to happen — either guard alone would have prevented it.

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

### Reviews — visible drafts, then proportionate review

Publish the draft on the first working push and show checkpoints while building.
**Ready for review** means the declared scope is implemented and applicable checks
are green; **merge-ready** additionally requires completed review and release
prerequisites. Neither label authorizes an automatic merge. See
[`working agreements §§9–12`](docs/teams/roles/working-agreements.md#9-match-the-model-and-context-to-the-task)
for model/context budgets, module-sized PRs, branch naming, and evolving role prompts.

**Not every PR needs a review round.** Feature PRs and rules/engine changes do; doc-only PRs,
pin bumps, and small mechanical fixes may skip it (Kirk, 2026-08-22, on a converter null→{}
change: *"i dont need copilot review for that change"*).

A substantive PR gets **one independent final review round** through one of two mechanisms:

- an enabled hosted reviewer such as Copilot; or
- a fresh read-only review session that did not implement the change.

Kirk may disable a hosted reviewer temporarily. While it is disabled, **do not request it**:
use a fresh independent session until Kirk explicitly re-enables it. Fresh-session review is
not a lesser fallback; use the reviewer that produces the stronger project-aware result.

**A local review file is not the review record.** Before calling a PR merge-ready, publish the final
verdict as a PR comment when an independent review round is required. The comment must name the reviewed head, readiness verdict,
Critical/Important/Minor counts, review scope, verification evidence considered, and the
disposition of any findings or fix rereviews. If the head changes materially, rerun or rebind
the review and publish an updated final verdict. `.superpowers` and other scratch reports may
support the work, but reviewers should not need access to the operator's filesystem to know
what was reviewed.

When Copilot is the selected reviewer, request it once for the substantive review
checkpoint, normally when the draft becomes ready for review—not automatically on
its first working push. The quota is monthly and shared, so a round spent on a typo
fix or an unfinished placeholder is a round some engine PR does not get.

**Requesting it.** GraphQL `requestReviews` with `botIds: ["BOT_kgDOCnlnWA"]`, then verify
by reading the PR node back — `gh pr view` does not show bot reviewers, so it will tell you
nothing landed when it did, or nothing when it didn't. Verify by read-back, never by the
mutation's own response. Same rule as board writes.

**Answering it.** A review is advice from something that has read the diff and not the
conversation. It is often right, sometimes right about the wrong reason, and occasionally
wrong. All three deserve a reply.

1. **Read every finding before fixing any of them.** Findings cluster; two comments are
   often one mistake seen twice, and fixing them separately produces two different fixes.
2. **Apply what is right, on the same branch.** No follow-up PR, no "address in a later
   slice" for something a reviewer could see from the diff.
3. **Look past the finding to its shape.** If a bug is real in one place, check whether the
   same shape exists somewhere the reviewer did not look. That is the finding's real value.
4. **Decline in writing when you disagree**, with the reason, in the thread. A finding you
   silently ignored is indistinguishable from one you missed.
5. **Reply in every thread**, naming the commit that addressed it and stating what changed.
   The thread is the record of why the code looks the way it does.
6. **Never re-request.** Copilot's footer invites another round; decline the invitation.
   Re-run the gates instead — build, vet, test, lint — and let Kirk's review be the second
   pass. Kirk merges.

**Requesting a review does not close it.** This is the failure mode, and writing the rule
down does not prevent it: you request the review, report the PR in the same breath, and move
on — and the review lands four minutes later, addressed to nobody. It has happened twice
after the rule was already written, once on the very next PR after writing it.

**Report the PR link immediately, with review explicitly pending.** Return for the
review when it arrives; visibility must not be withheld while a reviewer runs.
Before claiming merge readiness, check each finding for a recorded disposition and
publish the current-head verdict. Raw comment/reply counts do not prove closure:
one thread can have several replies, and a reply can leave the finding unresolved.
A review nobody answers wastes its cost. A fresh-session review may have no inline
threads; its closure mechanism is the published current-head verdict comment,
including every finding's disposition.

**What a good round looks like** — rpg-toolkit#1254, 2026-08-26, three findings, all valid:

- One was a real bug the author had not seen: a condition marked its owner dirty whether or
  not its flag had actually changed, which would have flagged every rogue dirty at the end
  of every round once turn boundaries become interactions. Fixed, **and the same guard
  applied to a second condition the reviewer had not flagged** — point 3 earning its place.
- Two said the package documentation asserted an invariant that did not hold yet: it claimed
  resolution installs the cast on every path, when no call site existed. Both correct, and
  pointedly so — **documenting an invariant before it holds is precisely how the registry
  that PR was deleting came to have five installers and one install.** A reviewer that has
  not read the conversation can still see that.

The cost of the alternative is the bug that PR fixed. A test asserted the broken behaviour
and a comment above it called that behaviour an "API WIRING REQUIREMENT" — a requirement
nothing in the codebase had ever met. Nobody was lying; the note was written when it was
almost true and never revisited. Review comments are one of the few places that gets caught.

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
- **We run exclusively on hex** - square and gridless stay supported, nothing ships on them

## The grid is hex

`encounter/compilefield.go` compiles an `AxialHexGrid` and nothing else, and `dungeonspec`
validates against one. **That is the only grid the product ever builds.** `tools/spatial`
also implements square (Chebyshev) and gridless (Euclidean) and they should keep working —
the toolkit is a library and somebody else's game may want them — but nothing of ours does.

**So a positional rule proven on a square grid is proven on a configuration we do not ship.**
Build the grid the encounter builds:

```go
spatial.NewAxialHexGrid(spatial.AxialHexGridConfig{SpanWidth: 1e6, SpanHeight: 1e6})
```

Why it matters more than it sounds: hex and square both report **integral** distances, so a
whole class of mistake is invisible on either. rpg-toolkit#1255 shipped a "within 5 feet"
radius of 1.5 cells — "widened to include diagonals", a correction hex and square do not need
— and nothing on either grid ever falls between 1 and 1.5, so it read as correct everywhere
anyone looked. On a gridless room it was 7.5 feet. The constant had been wrong in
`SneakAttackCondition` long enough that `ProneCondition`'s comment cited it as the shared
convention while using a different value itself.

Hex has already cost us twice for related reasons — rpg-toolkit#1141 (offset schemes) and
#1150 (axial basis) — both cases where a conversion applied identically in both directions
passed every round-trip test. Prove positional rules on the grid we run, and prefer a test
that computes an expected value over one that echoes the code's own arithmetic back.

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
