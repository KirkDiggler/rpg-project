# Task for scout

Goal: determine the canonical issue/design/board routing for the approved cross-repository toolkit-contributor onboarding and sandbox work.
Target: /home/kirk/game-dev/rpg-project. Read-only; do not edit, commit, push, comment, or create issues/PRs.
Inspect current canonical policy, relevant role charters, Project 19 conventions, existing related issues/designs, and the cross-repo idea workflow. Treat sessions/active.md as unrelated in-flight work that must not be disturbed.
Approved product direction: WSL2 toolkit contributor; game-dev bootstrap/start/refresh/seed commands; local override of rpg-toolkit/rulebooks/dnd5e into rpg-api; rpg-api seed command calls the running real CharacterService APIs to create reusable Protection Fighter and Barbarian sandbox characters; web dev-only sandbox reuses Dungeon Builder with a template, lets users assemble 1-2 party seats in any order, and uses real PutDungeon + lobby create/join/ready/start APIs; one active scenario; no generic scenario framework or direct Redis writes.
Return a concise routing brief: required umbrella/implementation issues by owning repo, Team/Feature/Kind/Status recommendations, canonical design path, role ownership, ordering, and any process blockers. Include evidence paths/URLs. No file dumps.

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