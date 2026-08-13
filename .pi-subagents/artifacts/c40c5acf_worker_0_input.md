# Task for worker

Create the design-only rpg-project change for the newly created umbrella issue.
Issue creation result:
Created the canonical umbrella issue and routed it to Project 19.

- **Issue:** [rpg-project #208](https://github.com/KirkDiggler/rpg-project/issues/208)
- **Project item ID:** `PVTI_lAHOAASbwc4Bcj4vzg2BUBI`
- **Fields:**
  - Status: `Todo`
  - Team: `Cross-team`
  - Feature: `Infra`
  - Kind: `Build`

### Actions performed

- Confirmed GitHub authentication and repository access.
- Confirmed no matching issue existed.
- Validated Project 19 field and option IDs.
- Created issue #208 with the required marker, sections, bounded scope, acceptance criteria, and signature.
- Added it to Project 19 and set all requested fields.
- Re-read the live issue and project item to verify content and routing.
- Confirmed no staged files and no repository files changed.
- Created no implementation issues, branch, or PR.

**Blockers:** None.
Target repository: /home/kirk/game-dev/rpg-project. The primary checkout is dirty with unrelated user work. Never modify, clean, stash, reset, stage, or switch the primary checkout.
Fetch origin and create an isolated manual git worktree at /home/kirk/game-dev/.pi-worktrees/rpg-project-toolkit-contributor-sandbox-design from latest origin/main. Create a docs/<issue-number>-toolkit-contributor-sandbox branch. Refuse rather than overwrite if the branch/path already exists unexpectedly.
Create only ideas/toolkit-contributor-sandbox/design.md. Do not create plan.md, implementation issues, or implementation code. Do not modify sessions/active.md or role policy.
Use the approved conversation and these scout artifacts as evidence: /home/kirk/game-dev/rpg-project/.pi-subagents/artifacts/dd419acb_scout_0_output.md, /home/kirk/game-dev/rpg-api/.pi-subagents/artifacts/68c441b2_scout_0_output.md, /home/kirk/game-dev/rpg-dnd5e-web/.pi-subagents/artifacts/bb8e6771_scout_0_output.md, /home/kirk/game-dev/.pi-subagents/artifacts/ab50db25_scout_0_output.md, /home/kirk/game-dev/rpg-toolkit/.pi-subagents/artifacts/5985ec3a_rpg-toolkit-member_0_output.md.
The design must state: user problem and Windows/WSL2 context; exact daily UX; repository ownership; data/control flow; one-module override safety; API-based idempotent seeding under dedicated Dev identities; use only currently implemented section RPCs because GetRequirements/SubmitChoices are unimplemented; Protection class choice plus post-finalize real shield equip; no direct Redis/Data construction; dev-auth client isolation; Dungeon Builder canvas template and authoring lifecycle; 1/2-seat arbitrary order; normal GameView versus optional harness; error/cleanup behavior; security and production gating; tests and clean-WSL verification matrix; phased delivery and explicit non-goals. Preserve the Boundary Rule.
Keep design simple-first: one template, one active scenario, explicit refresh, no watcher, no generic scenario schema/catalog/editor.
Run markdown/diff hygiene checks available in the repo. Commit the design locally with a clear docs message. Do not push or open a PR yet.
Return worktree path, branch, commit, changed files, checks with exit codes, residual questions, and confirm the primary checkout was untouched.

## Acceptance Contract
Acceptance level: checked
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Implement the requested change without widening scope
- criterion-2: Return evidence sufficient for an independent acceptance review

Required evidence: changed-files, tests-added, commands-run, residual-risks, no-staged-files, diff-summary

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