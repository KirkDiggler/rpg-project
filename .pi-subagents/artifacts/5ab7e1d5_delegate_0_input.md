# Task for delegate

Create the canonical rpg-project umbrella issue and Project 19 routing item for the already-approved toolkit contributor onboarding + live sandbox loop. This is coordination/GitHub mutation only; do not edit any repository files or disturb sessions/active.md.
Repository: KirkDiggler/rpg-project. No matching issue exists per the completed routing scout.
Title: Toolkit contributor onboarding and live sandbox loop.
The issue body must begin with exactly one top-level <!-- pih-dispatch:v1 --> marker and contain non-empty top-level semantic sections Goal / symptom, Desired outcome, Contract boundaries, Acceptance, Verification evidence, and Related / dependencies. Keep the body bounded and implementation-ready.
Approved fixed scope: WSL2 toolkit contributor; game-dev bootstrap/start/refresh/seed/status/down workflow; rpg-api one-module local override supports rulebooks/dnd5e; rpg-api cmd/sandboxseed calls the running real CharacterService APIs to create reusable Protection Fighter and Barbarian characters and uses the real equip API to equip the fighter shield; no direct Redis writes; web dev-only sandbox reuses Dungeon Builder with one preloaded canvas template, assembles one or two owned characters in either order, saves via PutDungeon, and uses real lobby create/join/ready/start APIs; normal GameView links plus optional harness links; one active scenario; no generic scenario framework.
Important verified constraint: GetRequirements and SubmitChoices proto methods are currently unimplemented handlers. The MVP must use currently implemented production section APIs (CreateDraft, UpdateName/Race/Class/Background/AbilityScores with class choices, GetDraft, FinalizeDraft, List/DeleteCharacter, and the existing equip API) rather than expanding into generic choice-handler completion.
Non-goals: native-Windows asset setup, rpg-toolkit code changes for the MVP, proto changes, deployment changes unless later proven necessary, automatic file watching, arbitrary scenario catalogs, weakened auth/ownership, direct Character.Data or Redis construction.
Acceptance must include clean WSL2 setup, explicit refresh after a local rulebook edit, idempotent API-based seeding of both characters, canvas template save, single-seat Fighter, single-seat Barbarian, Fighter→Barbarian two-seat, Barbarian→Fighter two-seat, normal GameView, and proof the local rulebooks/dnd5e override executed.
End the issue body with: — asset-pipeline agent, on behalf of KirkDiggler
Add it to user Project 19 and set Status=Todo, Team=Cross-team, Feature=Infra, Kind=Build. Verify the live issue body and project fields after mutation. Do not create implementation issues yet; those wait for design approval.
Return the issue URL/number, Project item ID and exact fields, commands/actions performed, and any blocker. Do not create a branch or PR.

## Acceptance Contract
Acceptance level: checked
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Implement the requested change without widening scope

Required evidence: changed-files, tests-added, commands-run, residual-risks, no-staged-files

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