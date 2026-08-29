# Team Roles

This directory holds provider-neutral Team charters for the RPG platform. A Team
charter is an **outcome lens**: it says what kind of result the Team protects,
what boundaries it refuses to cross, what evidence it owes, and how it signs
shared GitHub activity.

**Repository/module AGENTS files own technical commands and invariants.** A Team
charter does not replace a repository's AGENTS.md, scoped CLAUDE.md, Makefile,
package scripts, CI, or domain rules. For any slice, load the Team charter and
then load the owning repository's instructions before touching files.

**[`working-agreements.md`](working-agreements.md) applies to every Team and
runtime adapter.** A charter says what outcome lens you carry; the working
agreements say how seams, evidence, dispatches, shell usage, and test claims stay
honest.

## Current Team charters

| Team | Charter | Outcome lens |
|---|---|---|
| Platform | [`platform/prompt.md`](platform/prompt.md) | toolkit/API/proto/deployment/workspace architecture that keeps rules in toolkit and hosts thin |
| UI/UX | [`ui-ux/prompt.md`](ui-ux/prompt.md) | screens, HUD, interaction, accessibility, and presentation |
| Assets | [`assets/prompt.md`](assets/prompt.md) | licensed ingestion, manifests, model loading, rendering, animation, and visual evidence |
| Monster AI | [`monster-ai/prompt.md`](monster-ai/prompt.md) | intentional monster decisions expressed through toolkit-owned behavior contracts |
| Cross-team | [`cross-team/prompt.md`](cross-team/prompt.md) | initiative seams, integration, coordination, and end-to-end verification |

These five Team charters are the stable paths for new multi-contributor work.
Project 19's Team field selects the outcome lens; the owning repository still
selects the technical law.

## Required load order for new work

1. `rpg-project/AGENTS.md` / `CLAUDE.md` for shared vocabulary, Project 19 rules,
   and startup procedure.
2. The Project 19 item, including its parent journey/initiative when present.
3. The selected Team charter above.
4. The owning repository's AGENTS.md and nearest scoped instructions.
5. A matching approved skill in `.agents/skills/`, if one exists.

If any source disagrees, stop and reconcile before implementing. Do not silently
let a Team absorb another Team's outcome or let a repository violate its boundary.

## Compatibility inputs during runtime migration

The older repository-member, fixer, director overlay, support, and runtime-adapter
paths remain load-bearing for existing game-dev/OpenCode/Pi adapters until that
runtime migration is complete. Keep them in place:

- `director/`, including overlays;
- `rpg-*-member/`, `game-dev-member/`, and `rpg-dnd5e-web-member/`;
- `*-fixer/`;
- `independent-gate/`, `explore/`, `janitor/`;
- historical support directories such as `project-manager/`,
  `bug-fix-coordinator/`, and `platform-simplifier/`.

Those directories are **temporary compatibility inputs**, not the standing
ownership model for new Project 19 work. Do not delete, rename, or migrate them
until the game-dev runtime migration that consumes the five Team charters has
landed and been verified.

## Signature contract

Derive the operator with `gh api user --jq .login`. GitHub comments are made
through Kirk's account or the current operator's account, so Team reports and PR
comments use the Team-specific signature form:

```text
— <team> agent, on behalf of <github-login>
```

Each Team charter spells out its exact signature line.
