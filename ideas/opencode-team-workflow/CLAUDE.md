# Idea: OpenCode Team Workflow

Port the established human-centered UI/UX, Platform, and Assets team workflow
to OpenCode as a provider-neutral control plane — without replacing Claude
Code, without duplicating Project 19 as task state, and without copying the
existing role charters instead of adapting them.

## Status

**Design snapshot merged (2026-07-21, PR #102).** `design.md` records the
complete, Kirk-approved design. The next state is a separate implementation
plan snapshot: `plan.md` is authored on its own branch and reviewed in its own
ready Plan Review PR.

Review of this idea's own artifacts follows the **design/plan artifact
lifecycle** the design itself specifies (`design.md` §2, "Design/plan artifact
lifecycle — never local-only"): each committed design or plan snapshot is
pushed and opened as a ready (non-draft) PR for GitHub-surface review. A
worktree path is never the review surface. Implementation discoveries update
the canonical docs in the implementation PR or a linked docs follow-up.

## Tracking

- Issue: `rpg-project#101` — https://github.com/KirkDiggler/rpg-project/issues/101
- Merged design snapshot PR: `rpg-project#102` —
  https://github.com/KirkDiggler/rpg-project/pull/102
- Plan review PR: created from `docs/101-opencode-team-workflow-plan` after
  `plan.md` passes self-review
- Board: Project 19 ("The Dungeon Run"), Team **Cross-team**, Feature **Infra**, Kind **Build**
- Pilot chain this design's first live proof rides on: `rpg-api-protos#187`
  (equipment contract), coordinating with `rpg-project#94` and
  `rpg-dnd5e-web#531`/`#557`

## Purpose

OpenCode becomes a second, provider-neutral control plane standing side-by-side
with Claude Code during rollout. It ports the *workflow semantics* already
proven here — human-centered team pods, standing expert ownership, an
independent adversarial gate, Project 19 as durable task state — rather than
Claude Code's literal team/mailbox internals. `rpg-project` remains the
canonical cross-repo coordination root; `game-dev` remains portable
workstation bootstrap and does not fork team policy.

## Approved boundaries (the non-negotiables)

- Project 19 / GitHub (issues, PRs, board fields) is the durable, authoritative
  task state. OpenCode sessions are replaceable workers, not the source of
  truth — a worker can be killed and replaced from GitHub state alone.
- No executable dispatch without a backing issue. No second database, daemon,
  or orchestration store.
- Existing `docs/teams/roles/**` charters stay canonical. OpenCode agent files
  are thin adapters (model + permission bindings) that point to them.
- Role policy (who does what, what a role refuses) and model profile (which
  GPT model/variant backs a role) are separate axes — the model profile can
  change later without touching role policy.
- Human alone makes final product decisions and merges. No role gains merge
  authority.
- `game-dev/bootstrap.sh` adds `rpg-project`, `rpg-toolkit`, `rpg-api`, and
  `rpg-api-protos` to its clone/verify set (alongside the existing
  `rpg-dnd5e-web`, `rpg-game-assets`, `rpg-deployment`) — the full seven-repo
  workspace all three pods and the pilot chain need — and may verify OpenCode
  availability; it never overwrites global
  `~/.config/opencode/opencode.jsonc` or provider credentials. `game-dev`
  itself stays bootstrap/tooling, not a fourth standing team pod.

## Pointers

- Full design (complete, no open questions): [`design.md`](design.md)
- Artifact review process: `design.md` §2, "Design/plan artifact lifecycle —
  never local-only" — the ready PR is the review surface, never a worktree
  path; design and plan may be separate merged snapshots.
- Implementation plan: `plan.md` — authored after the merged design snapshot,
  reviewed in its own ready Plan Review PR, then merged before implementation.
