---
name: External contributor bootstrap
description: Read this first if your session is not Kirk's primary one. Drops you into the autonomous-waves loop at the implementer slot.
---

# External Contributor Bootstrap

This doc is for any Claude session whose human is not Kirk operating from his primary machine — a contributor picking up work from board #11 or its successor chapters.

Your session does not have Kirk's memory bank or `sessions/active.md`. That's fine. The artifacts in this repo are the source of truth; this doc walks you to them in the right order.

## Who this is for

- Contributors with a GitHub account and an interest in claiming work from the chapter board.
- Specialists (e.g. FadedPez owns spatial in rpg-toolkit) who want to pick up issues in their domain.
- Kirk on a different machine without his usual memory + session state.

You are entering an existing autonomous-wave loop (see `ideas/autonomous-waves/design.md`). The loop has well-defined slots. Your human is the director of their own issue; you are the producer + implementer.

## Bootstrap reading order

1. **This doc** (you are here).
2. `rpg-project/CLAUDE.md` — workspace map, the Boundary Rule, vocabulary, per-repo conventions.
3. `rpg-project/ideas/autonomous-waves/design.md` — the autonomous-waves vision and the gates + consensus + learning principle.
4. `rpg-project/ideas/encounter/v1alpha2/roadmap.md` — current wave sequence; identify the active wave.
5. **The active wave's plan doc** (`rpg-project/ideas/encounter/v1alpha2/plans/0X-wave-X.Y.md`). Architectural calls for that wave are already made under "Architectural calls already made (do not relitigate)." Do not relitigate them.
6. **The role context dir** for the repo your issue touches: `docs/teams/roles/<role>/{prompt.md, context/{patterns.json, discoveries.json, dependencies.json}}`. Patterns are validated; discoveries are wave-by-wave observations; dependencies are cross-repo coupling notes. Read all three before writing code.
7. **Your issue body** on board #11, plus the per-repo `CLAUDE.md` of the repo you're touching.

If after reading these you still don't have enough to start, surface that gap to your human. The wave package was incomplete. Don't guess.

## The loop you will run

For each inner issue:

1. **Pre-implementation grounding** (see next section — required).
2. **Plan**: write a short brief in the issue or a comment. Files to touch, tests to extend, new types.
3. **Implement**: branch from main (`gcm && gl && gcb feat/<name>`); implement; run `make pre-commit` (Go) or `npm run ci-check` (web). Never `git commit --no-verify` — CI auto-deploys.
4. **Open PR**: push with `-u`, open PR, link the issue.
5. **Address Copilot**: it reviews PRs in ~5–10 minutes. Reply to or fix every comment. Never ignore.
6. **Confirm CI green**.
7. **Surface for human merge**: your human merges. You don't.

When in doubt, surface and wait. The loop has a director slot for a reason.

## Pre-implementation grounding (required gate)

Before writing any code on an inner issue:

1. **Grep for the pattern your issue describes.** Search the repo (and the toolkit if you're in rpg-api or rpg-dnd5e-web) for the verbs and nouns in your issue. Frequently the substrate already has the abstraction.
2. **List the relevant ADRs.** Each repo has `docs/adr/`. If your issue touches conditions, chains, gamectx, proto contracts, or persistence — read the ADRs in that area before planning.
3. **Read the role context dir's `patterns.json`.** Validated patterns are the shape the next change should compose with.
4. **Surface overlaps.** If your issue's described approach overlaps with an existing pattern, raise it before implementing. Building a parallel system to an existing one is the most common failure mode this gate prevents.

**Worked example — the OA-condition near-miss.** Wave 2.11's original plan invented a `ReactionPromptEvent` and a "PromptKindReaction" machinery in the encounter SDK. Toolkit ADR-0027 (chain-as-reaction-window) and ADR-0025 (gamectx) already covered the problem — conditions subscribe to chains and query gamectx; the OA trigger ports to a condition every melee combatant Apply()s. A grounding pass against shipped toolkit code (`sneak_attack.go`, `fighting_style_protection.go`, `disengaging.go`) on 2026-05-10 surfaced the duplication and the plan was replaced with `11-wave-2.11-condition-driven-reactions.md`. **The lesson is structural**: a clear brief can still tell you to build something the substrate provides if grounding isn't explicit.

## Claiming work from board #11

The chapter board (https://github.com/users/KirkDiggler/projects/11) has wave columns. The current wave is the only one with inner issues fully filed — future waves are sketched in the roadmap but not yet broken into issues.

- Your human points you at specific issues that are appropriate to claim. Don't free-pick.
- Read the wave plan doc for the wave your issue is in before touching anything.
- Default to Phase 1 of the autonomous-waves design (your human reviews each PR before merge) until you've completed a few cycles cleanly.

## What stays in your human's hands

- **Architectural decisions** (scope calls about renaming vs deleting, deferring vs absorbing).
- **Wave promotion** (does the playable gate count as playable?).
- **Filing followup issues vs expanding the current one.**
- **Merging PRs.**

If any of these come up mid-implementation, stop and surface.

## Conventions you must follow

- **Outside-in development**: start at the handler/interface layer, return `codes.Unimplemented`, work inward. Per `rpg-project/CLAUDE.md`.
- **Input/Output types on every function** (non-negotiable). Never return `(nil, nil)`.
- **Test suites with gomock for Go, mocks under `mock/` subdir.**
- **Never use local replace directives in commits.** They break CI. Replaces during local dev are fine if stripped before commit.
- **Never rebase feature branches.** Merge only. See `rpg-project/CLAUDE.md` § Git Workflow.
- **One issue per PR. No branch without an issue.**
