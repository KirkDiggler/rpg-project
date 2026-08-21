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
- **Team role:** the Project 19 Team perspective responsible for the work's
  outcome across repository boundaries.
- **Skill:** an approved, on-demand procedure for a repeatable way of working;
  never an authority or live-state store.
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

Agent-authored GitHub comments MUST identify the active Team role and
authenticated operator:

```text
— <team-role> agent, on behalf of <github-login>
```

Each Team charter MUST define its stable signature label. The Assets charter MAY
use `asset-pipeline` so the established `asset-pipeline agent` signature remains
intact. An agent MUST NOT claim to speak for another collaborator. Shared boundary
changes affecting another journey MUST be recorded on the affected issues or
PRs rather than resolved only in local context.

## 4. State ownership

| State | Authority | Required home |
|---|---|---|
| Initiative, journey, slice, baton, status, readiness | Project 19 | Project fields and hierarchy |
| Requirements, accepted decisions, blockers, handoffs | GitHub | Owning issue or PR |
| Linked implementation and review phase | GitHub | Slice issue and PR |
| Outcome perspective and signature | Project 19 Team | Team charter in `rpg-project/docs/teams/roles/` |
| User's latest local direction | Local continuity | Ignored `rpg-project/active.md` |
| Local worktree, branch, commits, dirty files | Git | Verified from the repositories |
| Repository and module laws | Versioned docs | `AGENTS.md`/`CLAUDE.md`, README, ADR/design |
| Repeatable optional procedure | Approved skill | Canonical `.agents/skills/` location |
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

The only automatic instruction load this workspace assumes is
`game-dev/AGENTS.md`, because the coordinating session starts in game-dev. That
file MUST act as a concise bootloader and explicitly require this sequence:

1. read `rpg-project/AGENTS.md`, the canonical pointer to shared architecture,
   Project 19, Team charters, cross-repository designs, and approved skills;
2. read local `rpg-project/active.md` and query the authenticated user's Project
   19 assignments;
3. after the human selects work, read the owning repository's `AGENTS.md`;
4. read the nearest scoped module's `AGENTS.md` or README before editing; and
5. load a matching approved skill only when the task calls for one.

A session started directly in an owning repository MUST still follow its root
AGENTS pointer back to rpg-project for shared work. A dispatched worker brief
MUST name the same load chain; inheritance from a parent session MUST NOT be
assumed.

Every workspace repository is considered migrated only when a root
`CLAUDE.md` exposes the root symlink. Every nested `CLAUDE.md` intended to govern
scoped work SHOULD expose a sibling symlink. The pilot MUST migrate game-dev,
rpg-project, and rpg-toolkit; the retro decides propagation to the remaining
repositories. A deterministic workspace check MUST verify that required links
exist, are symlinks, and resolve to the intended tracked file.

Instructions MUST use progressive disclosure. Load-bearing rules MUST appear
before historical examples and status. Repository instructions MUST NOT rely on
personal absolute paths.

## 9. Team role system

The standing role set MUST align with Project 19 Team values and live at
`rpg-project/docs/teams/roles/<team>/prompt.md`:

- Platform;
- UI/UX;
- Assets, with `asset-pipeline` as an allowed signature label;
- Monster AI; and
- Cross-team.

The authenticated human is the director. Director MUST NOT be a persistent agent
identity with shared progress. The selected Slice's Team is active; when no Slice
has been selected, the Journey's Team is active. Conflicting explicit Team values
MUST be surfaced to the human rather than guessed. The active Team selects the
agent's perspective; the owning repository selects technical law; Kind and an
optional skill select the execution method.

| Question | Canonical answer |
|---|---|
| Who directs? | Authenticated `gh` operator |
| What is being advanced? | Assigned Journey and selected Slice |
| Which outcome perspective owns it? | Project 19 Team charter |
| Which technical laws apply? | Repository and nearest-module AGENTS |
| Which procedure applies? | Project 19 Kind plus an approved skill when one exists |

A Team charter MUST define only:

- the Team's outcome lens and cross-repository responsibilities;
- refusal and escalation conditions;
- required completion evidence;
- its stable GitHub signature label; and
- pointers to shared agreements and repository instructions.

Repository boundaries MUST NOT be modeled as standing roles. The API's ban on
rulebook logic belongs in `rpg-api/AGENTS.md`; toolkit module/release mechanics
belong in `rpg-toolkit/AGENTS.md`; asset tooling and license rules belong in the
owning asset repository instructions. Fixer, explorer, reviewer, independent
gate, and janitor MAY remain runtime execution profiles or become reviewed
skills, but MUST NOT own durable progress or duplicate a Team charter.

A runtime adapter MAY select model and permissions and point at the Team charter.
It MUST NOT copy or override policy.

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

## 10. Skills

The canonical shared skill location MUST be:

```text
rpg-project/.agents/skills/<skill-name>/SKILL.md
```

A procedure that applies only inside one repository MUST live at:

```text
<owning-repository>/.agents/skills/<skill-name>/SKILL.md
```

Skills MUST follow the Agent Skills standard and use progressive disclosure.
They MAY include relative scripts, references, fixtures, and assets. Executable
helpers MUST have deterministic verification and receive the same security
review as code.

A skill is accepted only after the team has observed and chosen a repeatable way
of working. It MUST NOT contain live progress, collaborator identity, board
state, repository invariants, or copied Team policy. AGENTS states what must
always be true; a skill explains how to perform a matching procedure.

The canonical skill catalog MUST start clean: the pilot catalog contains no
approved `SKILL.md`. Existing `.opencode/skills`, `.claude/skills`, and other
runtime-specific skills MUST NOT be bulk-migrated or presented as canonical.
Each legacy skill MAY be reviewed individually; an accepted workflow is
rewritten or moved deliberately into the canonical location with current
terminology and tests. Until then, new runtime adapters MUST NOT depend on it.

`rpg-project/.agents/skills/README.md` MAY describe this contract without
creating a discoverable skill. The game-dev root AGENTS MUST point to the shared
catalog. Runtime adapters MAY expose canonical skills through settings or
explicit paths, but MUST NOT copy their content.

## 11. rpg-toolkit module contract

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

## 12. Ideas taxonomy

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

## 13. Runtime memory and bootstrap

Correct contributor operation MUST NOT depend on Pi, Codex, Claude, or another
runtime's private memory system. Personal memory MAY provide convenience but
MUST NOT be the sole home of team policy, work state, or repository law.

General bootstrap MUST NOT silently install another collaborator's personal
memory as shared policy. Existing game-dev memory MUST be audited: load-bearing
team laws move to canonical docs before memory installation becomes optional or
operator-scoped. The pilot does not require deleting useful personal memory.

## 14. Runtime adapters and board schema

Adapters MUST use the live Project 19 schema. The product classification field
is `Area`, not the former `Feature` name. Immutable Project owner/field IDs MAY
remain adapter configuration, but field names and types MUST match live readback
or fail closed.

A contributor-facing assigned-work projection MUST retain assignee information
and filter against the authenticated operator. Ephemeral Pi status, inbox, and
focus views MAY project GitHub state; they MUST NOT become a second durable task
store. Codex and other runtimes MUST be able to follow the same workflow directly
from AGENTS and GitHub without Pi.

## 15. Rollout

The first rollout MUST remain bounded to:

1. **rpg-project:** ignored local continuity contract; retirement of global live
   progress authority; Team charters replacing repository roles; role-context
   reconciliation; clean `.agents/skills/` contract with no imported skills;
   and canonical idea taxonomy;
2. **game-dev:** root AGENTS bootloader with the explicit rpg-project/repository/
   module load chain; runtime-neutral startup; optional rather than canonical
   personal memory; and workspace link verification;
3. **rpg-toolkit:** root/scoped AGENTS links; concise instruction ordering;
   module scope/release guard; and stale personal-path cleanup; and
4. **real contributor proof:** a fresh Codex session continuing
   `dammitbilly0ne`'s Composable Attack Damage journey without relying on or
   importing a legacy runtime-specific skill.

The rollout MUST use Project 19 journey/slice issues and one owning-repository PR
per slice. It MUST NOT mass-edit all repositories before the pilot retro. After
the proof, the retro decides whether and how to propagate the AGENTS/link and
continuity pattern to remaining repositories.

## 16. Acceptance

The pilot passes only when a fresh runtime-neutral session can:

1. auto-load only game-dev AGENTS and follow its explicit pointer into
   rpg-project, the selected repository, and the nearest module;
2. derive the authenticated login;
3. find that user's assigned journey, current slice, and linked PR;
4. read local human direction from ignored `active.md`;
5. identify dirty, ahead, or unpublished Git work without mutation;
6. load the owning repository and nearest module contract;
7. state rpg-toolkit's one-versioned-module-per-PR and provider-first rules
   before editing; and
8. leave updated local continuity that another agent runtime can read; and
9. locate the clean canonical skill catalog without treating a legacy skill as
   approved.

Deterministic verification MUST include:

- game-dev bootloader pointer and AGENTS symlink target/tracked-file checks;
- `git check-ignore` proof for the real `active.md`;
- active-template heading and secret-placeholder checks;
- fixture tests for authenticated assignment filtering and failure behavior;
- toolkit multi-module red and single-module green scope fixtures;
- scans for retired repository-role progress, stale `Feature` adapter use, and
  accidental canonical exposure of legacy skills; and
- a documented fresh Codex-path walkthrough on the assigned journey.

## 17. Non-goals

This design does not:

- replace Project 19;
- impose a work-in-progress limit on contributors;
- automatically select or assign a journey;
- define a complete rule for publishing local checkpoints;
- synchronize local continuity across machines;
- claim to preserve unpushed code when the machine is lost;
- standardize private runtime memory locations;
- bulk-migrate or automatically trust existing runtime-specific skills;
- migrate every historical role/context entry without classification;
- mass-add AGENTS links to every repository before the pilot; or
- change gameplay, rules, wire contracts, or product behavior.
