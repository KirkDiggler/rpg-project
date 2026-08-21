---
name: Multi-contributor workspace — Brainstorm
tracking: https://github.com/KirkDiggler/rpg-project/issues/237
journey: https://github.com/KirkDiggler/rpg-project/issues/236
status: conversation-approved 2026-08-21; written review pending
---

# Multi-contributor workspace — Brainstorm

## Why this exists

The workspace was built around one human, one coordinating session, and one
preferred agent runtime. That worked while Kirk carried every initiative. It no
longer matches the team:

- Project 19 now assigns multi-PR journeys to individual collaborators;
- each collaborator directs their own agents and initiatives;
- `dammitbilly0ne` is carrying the Composable Attack Damage journey with Codex;
- the tracked `sessions/active.md` and role `active-work*.json` files present one
  shared narrative as if there were one director;
- most repositories expose `CLAUDE.md` but not the `AGENTS.md` that Pi and Codex
  discover consistently; and
- `game-dev` installs Claude-specific memory containing Kirk-specific process
  and authority assumptions.

The practical failure was rpg-toolkit PR #1144. Its implementation crossed the
independently released `rulebooks/dnd5e`, `resolution`, `session`, and legacy
`encounter` modules. The consumer modules could not pass CI because their
committed `go.mod` files could only consume a provider version published after
merge. Review split the work into provider PR #1146 and resolution PR #1148,
with session following after another published tag. The technical law existed
inside a large toolkit `CLAUDE.md`, but the collaborator's Codex agent did not
load it automatically.

The missing system is not automatic task assignment. It is a runtime-neutral
way for each human director and their agents to discover shared work, preserve
local intent, inspect unpublished work, and load the owning repository's laws.

## Facts already established by Project 19

The Project 19 pilot already supplies the shared hierarchy:

```text
Initiative #231
└── Journey #232 — assigned to dammitbilly0ne
    └── Slice rpg-toolkit#979
        └── PR rpg-toolkit#1126
```

The journey assignee holds the baton. Slice assignment and linked PRs expose the
current merge unit. Status, Team, Area, Kind, Initiative, Readiness, parent
issues, and sub-issue progress are board facts. A special issue representing a
collaborator or session would duplicate this model.

GitHub CLI authentication supplies collaborator identity. The Project remains
owned by `KirkDiggler`; the authenticated actor is the result of `gh api user`,
not the Project owner hardcoded into an adapter.

## What the board cannot know

Project 19 cannot truthfully describe:

- an uncommitted working tree;
- an unpublished local branch or commit;
- the machine-local path to a worktree;
- which assigned journey the human wants to discuss today; or
- a conversational direction such as, “Tomorrow let's wrap up our monster
  behavior implementation.”

An issue comment could repeat those facts, but publishing every local turn would
turn collaboration records into session logs without preserving the code. Git
is the only authority for whether local implementation exists.

## Approaches considered

### Rejected: Project 19 and issues only

This is portable and shared, but it cannot inspect local Git and would pressure
agents to publish noisy local checkpoints. It also cannot preserve unpublished
code; at best it can say that the code once existed.

### Rejected: Project 19 plus runtime-specific memory

Pi, Codex, and Claude have different memory locations and loading behavior. A
workflow that depends on one of them gives collaborators different project
facts. Private memory may enrich an agent, but it cannot carry team policy or
required continuity.

### Chosen: Project 19 + ignored local continuity + Git

Three layers have distinct authority:

1. Project 19, issues, and PRs hold shared work and collaboration state.
2. Ignored `rpg-project/active.md` holds local human intent and pointers.
3. Git proves local branches, commits, worktrees, and dirty state.

The local file is a portable handoff between runtimes, not a database, shared
task store, or backup. It is verified against Git and GitHub at startup. If it
is stale or missing, the agent reports that honestly and reconstructs only what
the board and Git prove.

## Human-directed startup

The coordinating session belongs to the authenticated human director. Startup
should orient rather than assign:

1. derive the GitHub login;
2. read local `active.md` if it exists;
3. query assigned Project 19 journeys and slices;
4. inspect referenced Git work without mutation;
5. report discrepancies; and
6. let the human choose the focus.

The user's current words override an older local note. The note should preserve
the user's future direction verbatim and keep agent-generated observations in
separate sections.

There is deliberately no fixed rule yet for when a local observation must be
promoted to an issue checkpoint. Handoffs, blockers, review, and decisions are
obvious collaboration boundaries, but the pilot should teach the finer rule
rather than pretending it is already known.

## Runtime-neutral instruction hierarchy

A fresh agent should receive policy through ordinary repository files:

1. workspace `AGENTS.md` for identity, board, and local continuity;
2. `rpg-project/AGENTS.md` for shared architecture and workflow;
3. owning repository `AGENTS.md` for repository mechanics; and
4. nearest module `AGENTS.md` or README for scoped contracts.

Where `CLAUDE.md` is the maintained source, `AGENTS.md` should be a symlink to
it. This gives Claude, Pi, Codex, and future runtimes the same bytes without
copying policy.

Role charters should state identity, ownership, responsibilities, and refusal
boundaries. They should not duplicate repository mechanics or maintain task
progress. A role invocation reads the repository/module instructions for the
work it actually owns.

## Toolkit rule made explicit

A directory rooted by `go.mod` is an independently released toolkit module. A
mergeable implementation PR changes one such module, plus directly supporting
repository-wide docs or checks. Provider-first sequencing happens between PRs:

1. provider module merges green and receives its tag;
2. consumer module bumps to that published version;
3. consumer PR then proves green independently.

Local `replace` directives and `go.work` remain useful during development but
never reach a commit. Only one PR may be in flight for a module at a time.
Several packages under one `go.mod` are one module; nested `go.mod` roots are
separate modules even when they live in the same repository.

The rule should be early in the repository instructions and enforced by a
scope check. It should not be buried beneath historical status, obsolete issue
lists, or personal filesystem examples.

## Role and memory cleanup

Existing role context needs reconciliation, not blind deletion:

- live progress moves to Project 19 or local `active.md`;
- current reusable laws move to owning repository/module docs;
- historical reasoning moves to ADRs or journey documents;
- actionable debt becomes an issue;
- runtime bindings remain thin adapters; and
- stale duplication is removed.

Personal agent memory remains optional. General bootstrap must not make another
collaborator's personal memory a prerequisite for correct work. Team policy
counts only when it is in the shared board or versioned canonical docs.

## Ideas directory taxonomy

Cross-repository ideas should be grouped by stable problem domain:

```text
ideas/
└── <stable-domain>/
    └── <specific-idea>/
        ├── brainstorm.md
        ├── design.md
        └── plan.md
```

Examples include:

- `ideas/character-progression/level-up-screen/`;
- `ideas/dungeon-authoring/composable-builder/`;
- `ideas/team-workflow/project-19-journeys/`; and
- `ideas/team-workflow/multi-contributor-workspace/`.

Domains are not repositories, temporary teams, or speculative roadmap phases.
Existing flat ideas move when touched; an open unratified idea may move before
merge with its links updated in the same change.

## Pilot

The first rollout is intentionally narrow:

1. rpg-project establishes local continuity and cleans the role/state contract;
2. game-dev exposes runtime-neutral startup and stops relying on shared personal
   memory;
3. rpg-toolkit exposes scoped AGENTS files and enforces module isolation; and
4. a fresh Codex session on the Composable Attack Damage journey validates the
   real contributor path.

The retro decides what to propagate to the remaining repositories and whether a
repeatable checkpoint-promotion rule has emerged. It does not pre-automate that
judgment.
