# Task for worker

You are reviving a previous subagent conversation.

Original run: 2c3235d1
Original agent: worker
Original session file: /home/kirk/.pi/agent/sessions/--home-kirk-game-dev--/2026-08-12T15-46-25-420Z_019ff6a7-460c-7d0f-8eb4-d1538c0d6b43/2c3235d1/run-0/session.jsonl

Use the stored session context as background. Answer the orchestrator's follow-up below. Do not assume the original child process is still alive.

Follow-up:
Apply a focused written-spec correction to PR #217 based on independent review. Work only in `/home/kirk/game-dev/rpg-project/.worktrees/attack-die-3d`; before any edit, cd there and prove it is the linked worktree on `idea/216-attack-die-3d` with a clean tracked/index state. Never edit, stash, reset, clean, switch, or commit the dirty primary checkout.

Review evaluation: all four warning-class findings are technically valid and within the already-approved design; they clarify acceptance rather than expand product scope. Resolve them in `ideas/attack-die-3d/design.md` only:

1. Lifecycle ownership: define one production owner for queue/presentation completion (preserve existing CombatPresentation/DiceTray sequencing; AttackDie3D is visual and must not independently advance FIFO). Lock 3D-vs-SVG mode for the lifetime of one presentation token. If 3D is not ready at beat start or fails, use SVG for that entire beat; never wait for asset/shader/WebGL readiness. Cancel frame work/listeners on unmount/token change and ignore stale readiness/settle callbacks, preventing late appearance or duplicate completion. Concept-only settle telemetry may exist for evidence but is not production queue authority. Fit this to actual existing callback semantics after re-reading the relevant documented source facts; do not invent a conflicting API.

2. Settle tolerance: freeze a concrete angular acceptance threshold before implementation. Use quaternion angular distance in degrees and set the gate to `<= 0.25°` at the completion observation, followed by copying/holding the exact target quaternion for the settled frame. Explain why the epsilon is a measurement gate, not permission to leave the final face visibly off-target. Apply the same frozen threshold to all 20 results and reduced motion.

3. Evidence invalidation: expand the immutable evidence tuple to every readability-affecting input already in scope: exact GLB hash, face map/version, camera transforms/projection, material mode/shader revision, lighting/environment, exposure/tone mapping, die scale, viewport CSS size/output resolution, and device-pixel ratio. A change invalidates human confirmation. Avoid inventing a general visual-regression platform.

4. Performance/playtest pass-fail: replace subjective terms with a bounded before/after protocol on the same build/device/route and a named target matrix. At minimum cover desktop Chromium and the project’s supported Discord-iframe/mobile-or-low-GPU profile if available from repo docs. Use warm-load repeated samples and freeze explicit regression budgets before concept promotion. Prefer concrete relative budgets appropriate to a transient single die: no >10% regression in p95 frame time during the throw/verdict window versus the existing SVG baseline, no new long task >50ms attributable to the die, no sustained frame-time miss after the die unmounts, and asset/network/GPU-memory measurements recorded. If repository conventions establish better exact targets, use and cite them instead; do not fabricate unsupported device names. Make failure block promotion, not concept exploration.

Also add immutable rpg-game-assets source references for established facts: asset promotion PR #46 and merged commit/ref available locally/remotely, while making clear the facts were inspected and can go stale if the GLB hash changes. Do not claim provenance is resolved.

Run the same placeholder/contradiction/scope/ambiguity self-review again. Confirm only design.md changes relative to the current branch head, run `git diff --check`, any available Markdown/link check, commit with a concise conventional message referencing #216, push the same branch, and verify PR #217 points to the new commit. Do not add plan.md, implement, merge, close, comment, or change board fields.

Return exact changed sections, commit SHA, validation commands/exits, PR-head evidence, clean linked-worktree status including untracked files, and residual risks.

## Acceptance Contract
Acceptance level: checked
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Implement the requested change without widening scope
- criterion-2: Return evidence sufficient for an independent acceptance review

Required evidence: changed-files, tests-added, commands-run, residual-risks, no-staged-files

Review gate: required by reviewer.

Finish with a fenced JSON block tagged `acceptance-report` in this shape:
Use empty arrays when no items apply; array fields contain strings unless object entries are shown.
`criteriaSatisfied[].status` must be exactly one of: satisfied, not-satisfied, not-applicable.
`commandsRun[].result` must be exactly one of: passed, failed, not-run.
`manualNotes` and `notes` are optional strings; an empty string means no note and does not satisfy `manual-notes` evidence.
```acceptance-report
{
  "criteriaSatisfied": [
    {
      "id": "criterion-1",
      "status": "satisfied",
      "evidence": "specific proof"
    },
    {
      "id": "criterion-2",
      "status": "satisfied",
      "evidence": "specific proof"
    }
  ],
  "changedFiles": [
    "src/file.ts"
  ],
  "testsAddedOrUpdated": [
    "test/file.test.ts"
  ],
  "commandsRun": [
    {
      "command": "command",
      "result": "passed",
      "summary": "short result"
    }
  ],
  "validationOutput": [
    "validation output or concise summary"
  ],
  "residualRisks": [
    "none"
  ],
  "noStagedFiles": true,
  "diffSummary": "short description of the diff",
  "reviewFindings": [
    "blocker: file.ts:12 - issue found, or no blockers"
  ],
  "manualNotes": "anything else the parent should know"
}
```