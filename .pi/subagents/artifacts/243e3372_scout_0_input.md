# Task for scout

Independently audit GitHub publication state for the approved attack-die design. Strictly read-only: do not edit files, commit, push, comment, create/close/merge issues or PRs, or alter Project fields.

Use `gh`/GitHub read APIs to inspect rpg-project issue #216, PR #217, and Project 19. Verify:
- issue #216 URL/title/state/body and exact signature footnote;
- issue is present exactly once on Project 19 with Status=Todo, Team=Assets, and report Feature value without judging unless clearly wrong;
- PR #217 URL/title/state/isDraft/base/head/head SHA/merge state/changed files/commit list/body and exact signature footnote;
- PR links/closes issue #216 appropriately and says written-spec approval is required before plan/implementation;
- PR changes only `ideas/attack-die-3d/design.md`, has no plan.md or implementation, and head SHA is `427c3da60db15b1a84ed45d2bb353076666e0ded`;
- branch/commit is reachable remotely and PR remains open/ready for Kirk review.

Return concise field evidence and any exact publication defect. Do not change anything.

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