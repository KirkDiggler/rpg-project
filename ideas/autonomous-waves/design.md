---
name: Autonomous Wave Implementation
description: A phased rollout for letting agents implement entire chapter waves end-to-end with progressively less human-in-the-loop
status: vision — Phase 1 starts with toolkit doc re-grounding (2026-05-04)
---

# Autonomous Wave Implementation

## The vision

A wave on the chapter board contains N issues. The autonomous loop:

```
foreach issue in wave:
  dispatch fresh team-member with worktree isolation
  team-member implements
  reviewer (subagent or me) verifies the work against filesystem/git
  open PR
  Copilot reviews
  team-member addresses Copilot (fix or threaded why-not)
  CI green
  merge
  next issue

at wave end:
  playable gate verified (the playtest milestone)
  human review of wave outcome
  promote to next wave
```

Goal: Kirk dispatches a wave, walks away, returns to a merged wave with playable gate satisfied.

## Why this matters

Kirk's day-job at platform-mcp is building exactly this kind of agent dispatch system. The rpg-project work is a high-fidelity test bed — same agent infrastructure, same trust questions, same failure modes — without the production-customer pressure. Lessons here transfer directly.

The "Architecture Honesty" chapter framing also fits: each PR documents what the agent did; what's broken-but-deferred vs broken-and-fixed; we surface real architectural drift rather than papering over it.

## The architectural principle: gates, artifacts, consensus, learning

Phases below describe *who does what*. The deeper structure is *what
verifies what*.

Each step in a wave produces an **artifact** (a thing that can be looked at)
and is examined by a **second opinion** (a different perspective looking at
that same artifact). When the artifact and the second opinion agree
(**consensus**), the step proceeds automatically. When they disagree, the
disagreement is the signal — we **learn from what prevented consensus** and
either add a new gate, deepen an existing gate, or sharpen the brief that
fed the work.

| Gate | Artifact | Second opinion |
|---|---|---|
| Pre-implementation grounding | The wave plan + inner-issue brief; the relevant ADRs and shipped patterns the planned shape will compose with | A grounding pass (orchestrator or a fresh exploration agent) that confirms the planned shape doesn't duplicate an existing toolkit / api primitive |
| Agent dispatch | The diff, the PR description, the brief-vs-actual delta | Verifier-pass (today: orchestrator; future: dedicated agent before PR opens) |
| Verifier-pass | Pass/fail report on symbols, citations, internal coherence | Copilot review |
| Copilot review | Inline comments | Orchestrator decides each: fix, threaded why-not, or escalate to Kirk |
| CI | Build + test + lint status | Self-evident; pass/fail |
| Premise (e.g., a TODO's stated condition) | The condition named in the marker (e.g., "swap back after wall UI testing") | Project-state evidence: closed PR/issue, merged branch, design doc — *not* in scope today, named gap |

**Consensus** = all gates converge on green. The cost of human-in-the-loop
is exactly proportional to how often gates *disagree*.

**The smoothing loop:** each disagreement names either a missing gate or a
too-shallow check. Wave 1 evidence:

- Verifier missed an inherited audit-doc bug on PR #620 (the "monster/actions imported zero times" claim) → next time, verifier reads the audit doc end-to-end against rpg-api itself, not just spot-checks.
- Verifier missed prose-coherence on PR #619 (5 of 5 issues caught only by Copilot) → next time, verifier does a depth-pass before the PR opens, not after.
- Copilot caught replace-directive misstatements on PR #618 → next time, brief tells the agent to actually run `go mod tidy` and report what the directive does, instead of asserting.
- Label-on-after-open failed CI on PR #147 → workflow trigger types fixed in #149.
- Wave 2.11's original plan (`09-wave-2.11-combat-depth.md`) invented `ReactionPromptEvent` + a parallel "PromptKindReaction" machinery in the encounter SDK; toolkit ADR-0027 (chain-as-reaction-window) + ADR-0025 (gamectx) already covered the same problem (conditions subscribe to chains, query gamectx; `sneak_attack.go`, `fighting_style_protection.go`, `disengaging.go` are the shipped exemplars). Two exploration agents grounded against shipped toolkit code on 2026-05-10 and surfaced the duplication; the plan was replaced with `11-wave-2.11-condition-driven-reactions.md`. → **new gate: pre-implementation grounding** (added above to the gates table). The brief writer's job, before any wave plan ships, is to ground the planned shape against the shipped substrate — relevant ADRs, existing patterns in the affected role's `patterns.json`, grep results for the verbs and nouns the plan introduces. If the substrate already provides the abstraction, the plan composes with it instead of paralleling it. This gate runs before agent dispatch and applies whether the implementer is the orchestrator, a team-member, or an external contributor.

**The "self-manageable" candidate.** PR #483 (single-constant TODO swap)
hit consensus immediately: verifier-pass green, Copilot 0 comments, CI
green. Five criteria distinguish this shape:
1. Single-file or very narrow surface
2. An existing marker in the code that names the change (TODO, deprecated comment, etc.)
3. Pre-existing test that validates the change
4. Zero prose / architecture decisions
5. Copilot historically silent on this shape

When all five hold, the orchestrator's role on the PR collapses to
"premise check + push + auto-merge on consensus." The remaining gap —
verifying the premise (the TODO's condition is actually met) — is named
above as a future gate, not currently automated.

**Phases as gate density.** The phased rollout below isn't really about
*how much the agent does*. It's about *how dense and reliable the gates
are*. Phase 2 ships when our gates are reliable enough that the
team-member can self-merge on consensus. Phase 3 ships when consensus
across an entire wave is a near-certain outcome of the wave's brief.

## Phased rollout

### Phase 1 — Verifier in the loop (current)

**Who does what:**
- Kirk: picks the next issue (or directs the orchestrator to pick), reviews PR, merges
- Orchestrator (me): dispatches team-member, verifies output, opens PR, addresses Copilot, surfaces ready-to-merge
- Team-member (fresh agent per task): implements

**Why a verifier:** the "agent claims X, didn't really X" failure mode is real. The doc-bootstrap rollout hit it twice (rpg-toolkit-member and rpg-api-member both reported "archived 60+ docs" when they actually copied them — left duplicates in place). Until we trust agents to self-verify, a verifier in the loop catches this.

**Phase 1 success looks like:** verifier (me) catches zero issues across an entire wave. That's the signal Phase 2 is safe.

### Phase 2 — Team-member self-merges per PR

**Who does what:**
- Kirk: picks the wave, reviews wave outcome at the playable gate
- Orchestrator (me): dispatches, monitors, intervenes only on real failures
- Team-member: implements, opens PR, addresses Copilot, merges when CI green and Copilot satisfied

**Removed checkpoint:** Kirk doesn't merge each PR. The team-member does it.

**Phase 2 success looks like:** zero "had to revert" across a wave. Playable gate works without Kirk re-checking PRs individually.

### Phase 3 — Team-member runs the wave

**Who does what:**
- Kirk: declares "run Wave N", reviews wave outcome
- Team-member: picks next issue from the wave column, implements, opens PR, merges, picks next, until wave is empty
- Orchestrator (me): falls back to "wave-level health check" — surface issues only at wave boundaries

**Removed checkpoint:** orchestrator no longer dispatches per-issue.

**Phase 3 success looks like:** one Kirk command at wave start, one Kirk decision at wave end (promote/rerun/intervene).

## What stays human-only forever

- **Architectural decisions** (e.g. "delete `dnd5e.DiceRoll` vs rename to `DiceNotation`") — judgment calls about scope, not patterns. Surfaced to Kirk; Kirk decides.
- **Wave promotion** — does the playable gate count as "playable" for the playtest? Subjective.
- **Chapter promotion** — is Chapter 1 done; what's Chapter 2's theme?
- **Deferring work** — the call to file a follow-up issue rather than expand the current one (e.g. issue #617 was the right deferral over expanding #613 to migrate effects/conditions/spells).

## Trust signals to escalate phases

Move Phase 1 → Phase 2 when:
- A full wave passes with the verifier (orchestrator) catching nothing the team-member didn't already catch
- Copilot's catch-rate on team-member PRs is consistently low (most comments are nitpicks, not real issues)
- CI catches no regressions that local testing missed

Move Phase 2 → Phase 3 when:
- A full wave passes with the team-member self-merging with no Kirk interventions
- Doc / status / quality scorecards stay coherent without orchestrator intervention

If a phase regresses (verifier/orchestrator catches a real issue mid-wave), drop back to the prior phase for the next wave.

## Parallel infrastructure to invest in

- **Reliable verification scripts**: `ls`, `git diff --stat`, grep for claimed changes — the verifier's primary tooling. Should be fast and quoted in PR descriptions. Already in use; formalize.
- **Per-repo Copilot enablement**: rpg-toolkit has it; rpg-api-protos does not. For autonomous mode, all 4 repos need it.
- **CI grep guards** (e.g. issue #617's "no replace directives"): each architectural rule that's a "never do X" should have a CI check, so an agent that drifts gets caught at PR time.
- **Status/quality docs as living state**: every PR that changes the status quo updates `docs/status.md` and `docs/quality.md` in the same PR. Already established as a pattern; lean into it.
- **Issue-as-task**: each issue body needs to be self-contained enough that a fresh team-member with no conversation history can pick it up. Issue #142 (C++ removal) was a good template — clear acceptance criteria, file:line references, cross-references to related issues. Bad examples: vague "fix X service" with no concrete checklist.

## Day-1 dispatch task (Phase 1 demo, 2026-05-04)

The first concrete dispatch:

> Walk rpg-api's actual toolkit imports. Map each import path to the toolkit components rpg-api uses. Rewrite `rpg-toolkit/docs/architecture/components/` so the documented components reflect what rpg-api actually consumes. Clean cruft: Conditions and Features are Actions under the hood (typed interface); document the unified Action abstraction, not as separate concepts. Reinforce the load-bearing architecture: domain models with `ToData()` per type, event bus + chains for combat resolution. Verifier (me) checks: did the docs reflect grep-able usage, or did the agent describe what they thought was true.

This is the first PR where I dispatch rather than implement directly.

## Open questions

- Worktree isolation: is `Agent({isolation: "worktree"})` reliable for Go-modules-aware work? (test on dispatch task)
- Auto-merge mechanics: GitHub auto-merge on green-CI exists; is it preferable to "team-member runs `gh pr merge`"? Probably yes — auto-merge respects branch protection rules and Copilot.
- Wave-level health checks: what does "wave outcome review" actually look like? Probably: rerun the integration test for the playable gate; eyeball the wave's PRs in aggregate; confirm board status reflects reality.
- Cross-repo waves: Wave 2 spans rpg-api-protos #143, rpg-api #471, rpg-api #478. Can one team-member work across repos in one worktree session, or do we dispatch separately and coordinate?

## Status

- 2026-05-04 morning: vision recorded. Phase 1 demo dispatch in flight (toolkit doc re-grounding).
- 2026-05-04 afternoon: Wave 1 complete (5 PRs merged across protos + toolkit). Retro at `wave-1-retro.md`. Wave 2 in flight (protos #147, ci #149, rpg-api #483 merged; rpg-api #471 dispatched to background agent).
- 2026-05-04 afternoon: gates+consensus+learning principle added (above the phased rollout) — phases re-framed as "gate density," not "how much the agent does."
