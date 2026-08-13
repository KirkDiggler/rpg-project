# Task for delegate

Record Kirk’s explicit plan approval and create the four owning-repository delivery issues required by the approved plan on rpg-project PR #209.
Read first: /home/kirk/game-dev/.pi-worktrees/rpg-project-toolkit-contributor-sandbox-design/ideas/toolkit-contributor-sandbox/plan.md and design.md. GitHub coordination only; do not edit local repository files, branches, commits, or PR code.
First comment on PR #209 and issue #208: Kirk approved the plan in the coordinating session; the plan gate is satisfied; implementation issues may be created and the rpg-api provider wave may begin. End each comment with — asset-pipeline agent, on behalf of KirkDiggler. Verify live comments.
Create exactly four issues from the plan’s delivery-record task, one in each owning repo: rpg-api provider, game-dev facade, rpg-dnd5e-web sandbox, and rpg-project clean-WSL verification. Use the exact plan titles/scope/acceptance where specified.
Every issue body must begin with exactly one top-level <!-- pih-dispatch:v1 --> marker and contain non-empty top-level Goal / symptom, Desired outcome, Contract boundaries, Acceptance, Verification evidence, and Related / dependencies sections. Include umbrella rpg-project#208, design/plan PR #209, owning base/PR target, and no out-of-owner work. End each issue body with — asset-pipeline agent, on behalf of KirkDiggler.
Add each issue to user Project 19 and set exact fields: rpg-api = Status Todo, Team Platform, Feature Infra, Kind Build; game-dev = Status Todo, Team Platform, Feature Infra, Kind Build (required for exact game-dev route); rpg-dnd5e-web = Status Todo, Team UI/UX, Feature Infra, Kind Build; rpg-project verification = Status Todo, Team Cross-team, Feature Infra, Kind Verify.
Link all four as sub-issues of rpg-project#208 using GitHub’s supported sub-issue API if available. If the API is unavailable, add one signed umbrella comment listing all four; do not rewrite or damage the approved issue body.
Re-read all four live issue bodies, repository identities, Project 19 item counts/fields, and umbrella linkage. Refuse duplicates rather than creating a second issue if an exact title already appeared during this run.
Do not create branches, implementation code, PRs, or merge anything. Return the four issue URLs/numbers/titles, item IDs/fields, approval comment URLs, linkage evidence, and any blocker.

## Acceptance Contract
Acceptance level: attested
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Return concrete findings with file paths and severity when applicable

Required evidence: review-findings, residual-risks

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