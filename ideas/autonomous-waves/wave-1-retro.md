---
name: wave 1 retrospective
description: What Wave 1 actually shipped, what the numbers tell us about the autonomous slice, and what's applicable to Wave 2 and beyond
date: 2026-05-04
status: complete — Wave 1 fully merged, retro authored at the boundary before Wave 2 starts
---

# Wave 1 retrospective

Wave 1 was the autonomous-waves design's Phase 1 running for real: dispatch
specialist agents to do PR-class work, with the orchestrator (Claude in
session) gluing the dispatch to the review/merge cycle.

This retro captures what shipped, the numbers behind it, where autonomy
actually lived, and what to carry into Wave 2.

## Ship list

| PR | Repo | Driver | Wall time | Commits | LOC | Copilot inline |
|---|---|---|---|---|---|---|
| #144 | rpg-api-protos | orchestrator | 2 min | 1 | +77/-51 | 0 |
| #145 | rpg-api-protos | orchestrator | 15 min | 1 | +20/-439 | 0 |
| #146 | rpg-api-protos | orchestrator | ~10 min | 1 | small | 0 |
| #616 | rpg-toolkit | orchestrator | 24 min | 2 | +33/-91 | 0 |
| #618 | rpg-toolkit | orchestrator | open all day | 2 | +85/-111 | **8** |
| #619 | rpg-toolkit | agent (audit) | open all day | 2 | +715/0 | **5** |
| #620 | rpg-toolkit | agent (components rewrite) | 16 min | 3 | +1268/-177 | **5** |

"Wall time" is open-to-merge clock time. "Open all day" PRs sat waiting for
Kirk's merge approval, not for active work.

Two agent-driven PRs (#619, #620) carried ~75% of the LOC delivered in Wave 1.

## What the numbers say

**Copilot signal scales with surface, not author.** Tight mechanical PRs
(#144/#145/#146/#616) drew zero Copilot comments — orchestrator or agent
doesn't matter. The "narrative" PRs (#618 hand-written, #619/#620
agent-written) all drew 5–8 comments. The Copilot-fix loop will always be
busiest on the PR class we most want to grow autonomy on.

**Verifier-pass and Copilot catch different classes.** On PR #620,
verifier-pass caught 1 of 6 bugs (an inherited "monster/actions imported
zero times" claim from the audit). Copilot caught the other 5 — frontmatter
vs body mismatches, technically-false claims about exported symbols,
misleading explanations of what a `replace` directive does. Verifier is
**breadth** ("do the symbols resolve?"). Copilot is **depth** ("is the prose
self-consistent and accurate?"). Both are necessary; one doesn't replace
the other.

**First-time-correct rates are close.** Hand PRs averaged 1.4 commits, agent
PRs 2.5. Most of the gap is the Copilot-fix commit, which both classes need
when the surface is large. Agents aren't markedly worse at first-pass
quality — they're operating on bigger surfaces where Copilot has more to
catch.

## Where the autonomous slice actually lived

For PR #620, the 16-minute wall time broke down as:

- **Agent work (autonomous):** one shot — dispatch returned with PR open. Span was minutes.
- **Verifier-pass (orchestrator):** ~5 min — caught 1 inherited bug, pushed fix.
- **Copilot wait (passive poll):** ~5 min via `ScheduleWakeup`.
- **Copilot-fix loop (orchestrator):** ~10 min — read 5 comments, decide fix vs threaded why-not, write fixes, push, post 5 replies (2 hit GitHub 502s and got a fallback PR-level comment).
- **Surface as ready → user merge:** ~1 min.

So the autonomous slice was the dispatch itself. About 15 of 16 minutes of
human-mediated time was orchestrator glue. **The autonomous span is the
one-shot writing; everything around it is glue.**

## What grows the autonomous slice

Ranked by minutes-of-orchestrator-time-saved-per-agent-PR:

### 1. Copilot-responder agent (~10 min/PR)

The pattern is repetitive across all five Copilot rounds in Wave 1:
read comment → decide fix or threaded why-not → write fix → reply. Failure
mode is benign (fall back to a PR-level comment, which the orchestrator
already had to do twice when the threaded-reply API 502'd).

This is the highest-leverage single addition. Worth building before Wave 4,
where 8+ agent PRs would multiply this cost.

**Constraint:** agent lifetime is one-shot. The dispatching agent has
usually exited by the time Copilot's review lands (2–5 min after push). So
the responder needs to be either a fresh agent woken on review-event, or a
sub-agent the worker spawns and waits on. Today this falls back to the
orchestrator (Claude in session), woken by `ScheduleWakeup`.

### 2. Verifier-pass as a sub-agent before PR opens (~5 min/PR + 1 caught bug)

Today verifier runs after PR open, as the orchestrator. Run it as a dedicated
agent before the worker opens the PR and the bug fix happens in the original
commit instead of a follow-up. Bonus: structures the verifier-vs-Copilot
complement explicitly. Verifier owns breadth (symbols resolve, counts match,
citations land); Copilot owns depth (prose self-consistency).

### 3. Structured agent handoff

Agents return prose summaries. If they returned structured output —
`{facts_verified: [], open_questions: [], blocked_on: []}` — the orchestrator
(or a successor agent) wouldn't have to re-derive what was checked. Especially
important across compactions, where prose summaries lose nuance and structured
fields don't.

If 1 + 2 land before Wave 4, the orchestrator role on an agent PR shrinks to
"approve dispatch" + "approve merge." That's the meaningful jump.

## Constraints to plan around

- **One-shot agent lifetime.** See above. Until agents wake on review events,
  Copilot response can't live with the original worker.

- **Compaction is reliable but isn't free.** Wave 1 had two compactions over
  ~12 hours. Survived both via Claude Code's auto-summary, but each is a
  small risk and the failure mode (forgetting non-obvious context) is silent.
  Longer waves either need shorter agent spans, or durable state in
  issues/design-docs/retros so compaction-recovery is "re-read the doc."
  This retro is part of that durability layer.

- **Verifier discipline needs depth, not just breadth.** Wave 1 verifier-pass
  on PR #620 was breadth-first ("do these symbols exist?"). Reading the diff
  for self-consistency is a different skill. Adopt the depth checks Copilot
  uses, and the Copilot-responder has less to do.

## Lessons applicable to Wave 2

Wave 2 is coord types: rpg-api-protos #143 (proto contract), rpg-api #471
(orchestrator/handler refactor), rpg-api #478 (storage refactor),
rpg-project #11 (playable-gate updates). Per Kirk: still manual orchestration.

Carry forward:

1. **Wait for Copilot before declaring ready.** Both #618 and #619 in Wave 1
   drew substantive Copilot finds *after* I'd declared things sound. The
   "wait + address" loop pays off: in Wave 1 it caught 18 issues across the
   three narrative PRs, several of which would have been embarrassing to ship.

2. **Verifier-pass before push, not after.** On PR #620 the
   monster/actions/monsters import-count bug was inherited from PR #619
   (the audit). Verifier-pass caught it but only after PR #620 was open. For
   Wave 2 PRs that depend on prior Wave 2 PRs (the protos→api chain), run
   verifier before pushing the dependent — saves a fix-up commit.

3. **Reserve "mock" for gomock-only.** Hand-written test doubles get names
   like `testItem`, `fakeRepo`. Lesson surfaced mid-Wave-1 and is now in
   memory; carry into Wave 2's repository test code.

4. **Code-specific knowledge belongs in repo docs, not agent memory.** The
   Wave 1 dispatch for PR #620 worked because the agent could read the
   audit and `docs/architecture/components/` directly. Wave 2 dispatches
   for orchestrator/handler refactors should read existing
   `internal/orchestrators/` patterns the same way.

5. **One PR per logical unit.** Wave 1 stuck to this; PR #618 bundled "items
   replace-dir + mechanics/proficiency replace-dir" because both were the
   same operation. Don't mix unrelated changes.

6. **Always push and create PR — never merge locally.** Wave 1 followed this
   uniformly. Continue.

7. **Issue-first, type: labels, project board #10.** Filed #621 to backfill
   the audit on the same day as PR #620. Wave 2 should keep filing
   follow-up issues rather than letting drift accumulate.

## Open questions / deferred risks

- **The ~1700 LOC of components docs hasn't been prose-read end-to-end.**
  Verifier-pass and Copilot caught technical errors; nobody has read the
  whole thing for clarity. Risk: medium. Mitigation: catch on next touch.

- **Phase 2 (team-member self-merge) requires solving "who reviews the
  agent's PR before Copilot."** Currently the orchestrator does the
  pre-Copilot verifier-pass. Phase 2 needs that to live elsewhere — either
  a peer reviewer agent, or accepting that Copilot is the review. Open
  design question.

- **Phase 3 (team-member runs full wave) requires durable cross-PR state.**
  Wave 1's inter-PR sequencing (hold #620 until #619 merged) lived in
  orchestrator memory. Phase 3 needs that on disk — a wave-state file,
  ordered checklist on the issue, or similar.

## Process notes for retro authorship

This retro was written at the wave boundary, before Wave 2 starts.
Pre-Wave-2 is the right time:
- The numbers are still fresh
- The pattern (verifier-pass yields, Copilot finds per PR) is grounded
  in artifacts you can still inspect
- Compaction risk on the *next* wave is reduced because applicable
  lessons are now durable

Future waves should retro at the same boundary. The retro doc is the
artifact a future orchestrator (or team-member, in Phase 2/3) can read to
understand what worked, what didn't, and why specific patterns are in place.
