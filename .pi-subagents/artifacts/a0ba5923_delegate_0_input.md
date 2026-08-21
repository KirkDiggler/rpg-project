# Task for delegate

Prepare additive native-Ubuntu design work only; no design content yet, no implementation, push, PR, or primary mutation.
Verify rpg-project PR #209 MERGED to main as 22aee544a42906c2f8c01a0e1eb4935c252dcda2 and umbrella #208 plus WSL verification #210 remain open. Capture dirty primary branch/head/status byte-for-byte.
Create one rpg-project issue titled `Design permanent native Ubuntu support for the toolkit contributor sandbox`. Body: parent #208; context original WSL design now merged; approved decisions native Ubuntu first-class alongside Ubuntu WSL2, any working Docker-compatible daemon, one facade/two explicit host modes, no bypass/other distro/auto-kill; deliverables design+plan, later game-dev issue/PR, native acceptance and self-contained #210 WSL pickup. No closing keyword. Signature required.
Add issue exactly once to Project 19 and set In Progress / Platform / Infra / Design if those exact options exist; if Kind Design absent ask. Add signed #208 comment linking the additive design issue and stating #208 remains open.
Fetch only origin/main. After issue exists, verify target branch/path absent and create isolated worktree `/home/kirk/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox` on branch `design/native-ubuntu-toolkit-sandbox` from fresh origin/main. Refuse overwrite. Verify exact clean base and primary snapshot preserved.
Return issue/board fields/comment URL, base SHA, branch/worktree, primary preservation, blockers.

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