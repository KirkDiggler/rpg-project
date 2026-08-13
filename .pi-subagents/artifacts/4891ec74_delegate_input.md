# Task for delegate

You are reviving a previous subagent conversation.

Original run: a0ba5923
Original agent: delegate
Original session file: /home/kirk/.pi/agent/sessions/--home-kirk-game-dev--/2026-08-05T15-00-12-202Z_019fd270-712a-7b7b-ae0b-5850bc2f084c/a0ba5923/run-0/session.jsonl

Use the stored session context as background. Answer the orchestrator's follow-up below. Do not assume the original child process is still alive.

Follow-up:
Resume and complete with approved Kind `Decide`. Verify/create only the signed #208 link once, fetch origin/main, create the absent isolated worktree/branch from fresh main, verify primary snapshot preserved, and return evidence. If any action already completed before interrupt, do not duplicate it.

## Acceptance Contract
Acceptance level: attested
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Return a concise result and residual risks when applicable

Required evidence: manual-notes, residual-risks

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