---
name: Multi-contributor workspace
tracking: https://github.com/KirkDiggler/rpg-project/issues/237
journey: https://github.com/KirkDiggler/rpg-project/issues/236
status: approved in conversation 2026-08-21; written review pending
---

# Multi-contributor workspace

## 1. Purpose

Every bootstrapped collaborator MUST be able to direct their assigned Project
19 journeys with their own agents, resume machine-local work across agent
runtimes, and load the owning repository and module rules before editing.

The system MUST remain human-directed. Startup MAY orient and detect conflicts;
it MUST NOT choose or assign work on the human's behalf.

## 2. Terms

- **Operator:** the human authenticated by GitHub CLI in the current workspace.
- **Director:** an operator directing their own journeys and agents.
- **Shared state:** collaboration facts another contributor must be able to read.
- **Local continuity:** machine-local intent and observations used to resume a
  conversation or unpublished implementation.
- **Module:** for rpg-toolkit, the source tree owned by one nearest enclosing
  `go.mod` and released at its own version.

The Project 19 definitions of Initiative, Journey, Slice, and Baton remain
canonical in `ideas/team-workflow/project-19-journeys/design.md` after that open
idea is moved into the domain taxonomy.

## 3. Identity and authority

The operator login MUST be derived from the authenticated `gh` session. Runtime
configuration MUST NOT require a separately maintained collaborator identity.

Project 19 remains the Project owned by `KirkDiggler`. Project ownership MUST
NOT be confused with the current operator. Board queries MUST filter or present
assignments using the authenticated operator's login.

Each collaborator is a director of their assigned journeys with their own
coordinating and role agents. Canonical charters MUST NOT reserve all decisions,
reviews, or merges to Kirk. Repository permissions, protection rules, assigned
ownership, and required review remain binding.

Agent-authored GitHub comments MUST identify the active role and authenticated
operator:

```text
— <role> agent, on behalf of <github-login>
```

An agent MUST NOT claim to speak for another collaborator. Shared boundary
changes affecting another journey MUST be recorded on the affected issues or
PRs rather than resolved only in local context.

## 4. State ownership

| State | Authority | Required home |
|---|---|---|
| Initiative, journey, slice, baton, status, readiness | Project 19 | Project fields and hierarchy |
| Requirements, accepted decisions, blockers, handoffs | GitHub | Owning issue or PR |
| Linked implementation and review phase | GitHub | Slice issue and PR |
| User's latest local direction | Local continuity | Ignored `rpg-project/active.md` |
| Local worktree, branch, commits, dirty files | Git | Verified from the repositories |
| Repository and module laws | Versioned docs | `AGENTS.md`/`CLAUDE.md`, README, ADR/design |
| Personal preferences or runtime recollection | Optional private memory | Runtime/user-owned storage |

No tracked global progress file or role context file MAY override Project 19.
No local continuity claim MAY override Git.

## 5. Local continuity file

The actual `rpg-project/active.md` MUST be ignored by Git. The tracked
`rpg-project/docs/templates/local-active.md` MUST define this minimum shape:

```md
# Local active context

Operator: <github-login>
Updated: <timestamp>

## User direction
> <verbatim future-facing direction, or an explicit statement that none was recorded>

## Current focus
- Journey: <URL>
- Slice: <URL, if selected>
- PR: <URL, if one exists>

## Local work
- Repository/worktree: <path>
- Branch: <branch>
- Observed HEAD: <sha>
- Upstream state: <observed ahead/behind/unpublished state>
- Working tree: <observed clean/dirty summary>

## Last observed
<agent-generated facts and evidence>

## Next
<immediate local continuation>

## Open questions
<unresolved thoughts, explicitly not decisions>
```

The user's direction MUST be preserved separately from agent-generated summary.
The file MAY list several local worktrees, but SHOULD identify the last focused
journey and slice. It MUST NOT contain credentials, tokens, secrets, or licensed
asset contents.

The coordinating agent SHOULD rewrite the file when the user gives future
direction, focus changes, or meaningful local Git state changes. This is
best-effort continuity, not a shutdown guarantee. The file points to local work;
it does not back up that work.

## 6. Session startup

A coordinating session MUST:

1. load the workspace instructions;
2. derive the operator through `gh`;
3. read `rpg-project/active.md` when present;
4. query Project 19 for the operator's assigned non-Done journeys and current
   slices, including linked PRs;
5. inspect referenced branches and worktrees without mutation;
6. report local/GitHub disagreements; and
7. ask the human director what to focus on.

The human's current direction supersedes an older local note. Several assigned
or In Progress items MUST be shown without imposing a work-in-progress limit or
silently choosing one.

If `active.md` is missing or stale, a bounded workspace inventory SHOULD surface
dirty worktrees, branches without upstreams, and commits ahead of origin. The
agent MUST NOT reset, clean, stash, switch, or otherwise reconcile those facts
without explicit direction.

### 6.1 Failure behavior

- If `gh` is unavailable or unauthenticated, the agent MAY continue from local
  continuity and Git but MUST label Project 19 state unavailable.
- If the file's Operator differs from the authenticated login, the agent MUST
  preserve the file and stop for human direction before writing it.
- If a recorded path, branch, or SHA is stale, Git wins and the mismatch MUST be
  reported.
- If Project 19 is unavailable, a cached or local projection MUST NOT be called
  current shared truth.
- If no local file exists, startup from Project 19 and Git remains valid.

## 7. Shared checkpoint boundary

The pilot MUST NOT define an automatic rule that publishes every local
checkpoint. A collaborator or agent MAY publish when a fact must become shared,
including an intentional handoff, blocker, review transition, or accepted
decision.

The pilot retro MUST evaluate which local facts repeatedly needed promotion.
Automation MAY be designed only after that repeated boundary is observed.

## 8. Instruction hierarchy

Where `CLAUDE.md` is the maintained instruction source, `AGENTS.md` MUST be a
tracked symlink to `CLAUDE.md`.

The required hierarchy is:

1. **game-dev root:** operator identity, Project 19 orientation, local
   continuity, and workspace-wide safety;
2. **rpg-project root:** shared architecture, journey workflow, and canonical
   process pointers;
3. **owning repository root:** repository boundary, branches, tests, release,
   and review mechanics; and
4. **nearest scoped module:** module-specific contract and verification.

Every workspace repository is considered migrated only when a root
`CLAUDE.md` exposes the root symlink. Every nested `CLAUDE.md` intended to govern
scoped work SHOULD expose a sibling symlink. The pilot MUST migrate game-dev,
rpg-project, and rpg-toolkit; the retro decides propagation to the remaining
repositories. A deterministic workspace check MUST verify that required links
exist, are symlinks, and resolve to the intended tracked file.

Instructions MUST use progressive disclosure. Load-bearing rules MUST appear
before historical examples and status. Repository instructions MUST NOT rely on
personal absolute paths.

## 9. Role system

A standing role charter MUST define only:

- identity and owning boundary;
- responsibilities;
- refusal and escalation conditions;
- required completion evidence; and
- pointers to shared working agreements and owning repository instructions.

A runtime adapter MAY select model and permissions and point at the charter. It
MUST NOT copy or override policy.

Role context files MUST be classified during migration:

| Existing content | Destination |
|---|---|
| Live progress, PRs, blockers | Project 19 / issue / PR, then remove |
| Machine-local unpublished state | `active.md` and Git, then remove |
| Current reusable repository law | Owning repository/module instructions |
| Architectural decision | ADR or normative design |
| Historical rationale | Journey or brainstorm document |
| Actionable debt | Owning issue on Project 19 |
| Personal preference | Operator-owned private memory |
| Stale duplication | Remove |

The migration MUST preserve useful knowledge before deleting its old copy. Role
invocation MUST NOT require reading an accumulated `active-work.json` or one
shared session narrative.

The tracked `sessions/active.md` MUST be retired only after a reconciliation
sweep. Shared current facts move to their owning Project 19 item, issue, or PR;
machine-local facts move to the operator's ignored `active.md`; historical prose
remains available through Git history. Startup references to the tracked session
file MUST be removed in the same slice.

## 10. rpg-toolkit module contract

The nearest enclosing `go.mod` defines an independently versioned module. Nested
module roots are separate even when they share the rpg-toolkit repository.
Packages beneath the same nearest `go.mod` belong to one module.

An implementation PR MUST change at most one versioned module. It MAY also
change directly supporting repository-wide ADRs, decision indexes, or checks.
A docs-only PR MAY change no module. A diff spanning two module roots MUST fail
scope verification and be split.

Provider-first sequencing MUST occur between mergeable PRs:

1. develop against a local provider override when useful;
2. remove uncommitted `replace`/`go.work` state from the proposed diff;
3. merge the provider module with green CI;
4. verify the provider's published tag;
5. create or update the consumer module PR to pin that tag; and
6. prove the consumer independently green.

Only one PR may be in flight for a module at a time. Before starting a slice,
the responsible agent MUST inspect current open PRs for that module. Work in a
second journey that needs the occupied module waits, joins the coherent existing
slice when ownership agrees, or records an explicit dependency.

A deterministic scope check MUST prove at least these cases:

- red: `rulebooks/dnd5e` and `rulebooks/dnd5e/resolution` both change;
- green: only `rulebooks/dnd5e/resolution` and its dependency files change;
- green: one module plus directly related root ADR/check files; and
- green: repository-only documentation changes no module.

The root toolkit instructions MUST state this contract near the top and remove
obsolete issue inventories, historical completion claims, and personal-path
examples that obscure it.

## 11. Ideas taxonomy

Cross-repository idea artifacts MUST use:

```text
ideas/<stable-domain>/<specific-idea>/brainstorm.md
ideas/<stable-domain>/<specific-idea>/design.md
ideas/<stable-domain>/<specific-idea>/plan.md
```

The stable domain MUST describe the enduring problem space, not a repository,
temporary team, PR, or speculative phase. A domain README SHOULD be added only
when multiple sibling ideas need orientation. Empty taxonomy directories MUST
NOT be created speculatively.

Examples:

- `ideas/character-progression/level-up-screen/`;
- `ideas/dungeon-authoring/composable-builder/`;
- `ideas/team-workflow/project-19-journeys/`; and
- `ideas/team-workflow/multi-contributor-workspace/`.

Existing merged flat ideas SHOULD move only when touched, with links updated in
the same change. The open Project 19 journey idea SHOULD move under
`ideas/team-workflow/project-19-journeys/` before ratification.

## 12. Runtime memory and bootstrap

Correct contributor operation MUST NOT depend on Pi, Codex, Claude, or another
runtime's private memory system. Personal memory MAY provide convenience but
MUST NOT be the sole home of team policy, work state, or repository law.

General bootstrap MUST NOT silently install another collaborator's personal
memory as shared policy. Existing game-dev memory MUST be audited: load-bearing
team laws move to canonical docs before memory installation becomes optional or
operator-scoped. The pilot does not require deleting useful personal memory.

## 13. Runtime adapters and board schema

Adapters MUST use the live Project 19 schema. The product classification field
is `Area`, not the former `Feature` name. Immutable Project owner/field IDs MAY
remain adapter configuration, but field names and types MUST match live readback
or fail closed.

A contributor-facing assigned-work projection MUST retain assignee information
and filter against the authenticated operator. Ephemeral Pi status, inbox, and
focus views MAY project GitHub state; they MUST NOT become a second durable task
store. Codex and other runtimes MUST be able to follow the same workflow directly
from AGENTS and GitHub without Pi.

## 14. Rollout

The first rollout MUST remain bounded to:

1. **rpg-project:** ignored local continuity contract; retirement of global live
   progress authority; dynamic contributor authority; role-context
   reconciliation; and canonical idea taxonomy;
2. **game-dev:** root AGENTS link; runtime-neutral startup; optional rather than
   canonical personal memory; and workspace link verification;
3. **rpg-toolkit:** root/scoped AGENTS links; concise instruction ordering;
   module scope/release guard; and stale personal-path cleanup; and
4. **real contributor proof:** a fresh Codex session continuing
   `dammitbilly0ne`'s Composable Attack Damage journey.

The rollout MUST use Project 19 journey/slice issues and one owning-repository PR
per slice. It MUST NOT mass-edit all repositories before the pilot retro. After
the proof, the retro decides whether and how to propagate the AGENTS/link and
continuity pattern to remaining repositories.

## 15. Acceptance

The pilot passes only when a fresh runtime-neutral session can:

1. load workspace guidance through AGENTS without private Claude memory;
2. derive the authenticated login;
3. find that user's assigned journey, current slice, and linked PR;
4. read local human direction from ignored `active.md`;
5. identify dirty, ahead, or unpublished Git work without mutation;
6. load the owning repository and nearest module contract;
7. state rpg-toolkit's one-versioned-module-per-PR and provider-first rules
   before editing; and
8. leave updated local continuity that another agent runtime can read.

Deterministic verification MUST include:

- AGENTS symlink target and tracked-file checks;
- `git check-ignore` proof for the real `active.md`;
- active-template heading and secret-placeholder checks;
- fixture tests for authenticated assignment filtering and failure behavior;
- toolkit multi-module red and single-module green scope fixtures;
- scans for retired global-progress authority and stale `Feature` adapter use;
  and
- a documented fresh Codex-path walkthrough on the assigned journey.

## 16. Non-goals

This design does not:

- replace Project 19;
- impose a work-in-progress limit on contributors;
- automatically select or assign a journey;
- define a complete rule for publishing local checkpoints;
- synchronize local continuity across machines;
- claim to preserve unpushed code when the machine is lost;
- standardize private runtime memory locations;
- migrate every historical role/context entry without classification;
- mass-add AGENTS links to every repository before the pilot; or
- change gameplay, rules, wire contracts, or product behavior.
