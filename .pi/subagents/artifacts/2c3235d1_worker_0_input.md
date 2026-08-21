# Task for worker

Goal: turn Kirk-approved brainstorming into the canonical cross-repository design and GitHub review surface for a staged 3D attack-die concept.

Isolation context: native managed worktree creation was attempted and failed because the primary checkout contains pre-existing unrelated changes. A read-only safety audit verified `/home/kirk/game-dev/rpg-project/.worktrees` exists and is ignored by `.git/info/exclude`, `/home/kirk/game-dev/rpg-project/.worktrees/attack-die-3d` is absent, no related branch exists, and the primary checkout must remain untouched. Use the manual linked-worktree fallback. Do not stash, clean, reset, switch, commit, or edit anything in the primary checkout.

Authority: you are the sole writer. You may fetch, search GitHub, create one tracking issue in KirkDiggler/rpg-project if no duplicate exists, add it to Project 19, create one linked worktree under the verified ignored `.worktrees` directory, edit only the canonical design artifact inside that worktree, commit, push, and open one ready (not draft) PR targeting main. Do not merge, close anything, implement web/assets code, write plan.md, modify sessions/active.md, publish/release assets, or touch implementation repositories.

Required isolation sequence:
1. Read rpg-project instructions from the primary checkout.
2. Search open/closed rpg-project, rpg-dnd5e-web, and rpg-game-assets issues/PRs for an exact attack-die/d20 concept effort. Reuse an exact tracking issue if appropriate; otherwise create one rpg-project design issue because this PR belongs to rpg-project.
3. New work must be tracked on Project 19. Set Status=Todo and Team=Assets when those exact options exist. Select an existing Feature only if clearly appropriate; never invent one. Report limitations.
4. Fetch origin without altering working files. Prove latest `refs/remotes/origin/main`.
5. Create `/home/kirk/game-dev/rpg-project/.worktrees/attack-die-3d` from latest `refs/remotes/origin/main` on a new branch `idea/<issue-number>-attack-die-3d` using `git worktree add --no-track`. Work exclusively inside that linked worktree afterward.
6. Before editing, prove the linked worktree is clean and HEAD equals fetched origin/main. If any collision or unexpected state exists, stop safely.

Canonical artifact: `ideas/attack-die-3d/design.md` in the linked rpg-project worktree. Do not use docs/superpowers/specs and do not add plan.md.

Approved design contract:
- Goal: improve the existing attack-roll presentation with the new private lightning d20 GLB. Ultimate acceptance: every authoritative server d20 result 1–20 tumbles decoratively and settles with that engraved numeral visibly on top.
- Existing facts: production web already passes authoritative `AttackResolved.attackRoll` through `CombatPresentation` to `DiceTray`; presentation randomness must never generate or alter the outcome; no proto/API/toolkit work is needed for the recommended result-driven settle.
- Asset: `/models/synty/props/SM_Prop_D20_Lightning_01.glb`, source/promoted under rpg-game-assets. Runtime is about 204 KB, 1,906 triangles, two material slots, no textures, animations, or face metadata. Source look is cloudy white/gray marble with gold numerals. Advertised lightning is not embedded and needs a narrow web material treatment. Runtime names carry Blender suffixes (`D20_Lightning_Material.010`, `Paint_Material.010`), so use a stable normalized-prefix or corrected-export contract.
- Provenance/licensing confirmation is a shipping precondition; the private repo/path is correct but current metadata does not establish an explicit ownership/license grant.
- Web concept URL/id: `?concept=attack-die-3d` in the existing development-only concept registry.
- Build a production-intent shared `AttackDie3D` component exercised by the concept, not a throwaway concept-only renderer. Keep existing SVG `DiceTray` as accessible/load/WebGL/unmapped fallback. Do not add d20 to the generic dungeon prop palette merely to resolve its URL.
- Concept stages: Appearance (raw versus magical material); Calibrate (select 1–20, top and 3/4 cameras, save normalized quaternion); Roll (authoritative input, decorative path, kinematic slerp/damped settle, exact final orientation); Verify (run all 20 and evidence grid).
- Graduation gate: 20/20 mapped, 20/20 final orientation within a defined small angular tolerance, and 20/20 human-confirmed numeral visible/readable from top and canonical 3/4 settlement cameras. Evidence binds to exact GLB hash, mapping, camera contract, and material mode; changes invalidate stale confirmations.
- Reduced motion: no tumble or animated lightning, brief static settle to the same result. Visual die remains aria-hidden; authoritative result is announced exactly once by existing accessible presentation.
- Failure behavior: load/WebGL/shader errors and missing mappings must never show a wrong physical face; fall back visibly to the authoritative SVG result.
- Performance: prefer isolated DOM-overlay presentation because it matches the current theater, but treat a second WebGL context as a measurable Discord/mobile GPU risk. Measure before production promotion. Preserve FIFO timing, movement wait, result/damage release, keyboard control, and completion callback semantics.
- Ownership: rpg-game-assets owns provenance, stable material/face-map asset contract tied to GLB hash, and visual QA; web owns loader, shader, deterministic presentation animation, concept, fallback/a11y, tests, and performance playtest; rules/proto/API own nothing for this scope.
- Explicit non-goals: physics/collision, damage dice, discarded advantage/disadvantage dice, server seed/trajectory reproduction, universal dice platform, production promotion during the concept PR.
- Testing: pure face-map completeness/normalization and target-orientation math; final angular tolerance; invalidation keys; loader/error/unmapped fallback; reduced motion; existing sequencing regression; visual all-face evidence; hit/miss/NAT-1/crit, keyboard, narrow viewport, Discord iframe, and low-GPU playtests.
- Incremental path: asset/material proof -> face calibration -> authoritative tumble/settle -> all-face verification -> separate approved promotion work. Physics remains deferred.

Write a concise but implementation-ready design with: summary, context, established facts, decisions, goals/non-goals, product experience, architecture/ownership, component boundaries, data flow/authority, face-map and asset contract, material approach, concept stages/controls, failure/a11y/reduced-motion/performance behavior, validation/evidence, testing/playtest, graduation/promotion criteria, and risks/preconditions. Avoid TBD/TODO/placeholders and never imply unverified face orientations or provenance are resolved.

Self-review before publication: scan for placeholders, contradictions, ambiguous authority, scope creep, and mismatch with the cross-repo workflow; fix findings inline. Confirm only design.md changed. Validate Markdown with best repo-available checks and `git diff --check`.

GitHub conventions:
- Issue and PR explain that conversational design sections were approved, but Kirk must review the written spec before plan/implementation.
- One issue for this design PR; later implementation PRs get owning-repo issues after written design approval.
- Ready PR, not draft; leave it open. Include issue linkage and cross-repo scope.
- End any issue/PR body or comments authored through Kirk’s account with `— asset-pipeline agent, on behalf of KirkDiggler`.

Return: issue URL/number, Project 19 field evidence, linked worktree path, design path, commit SHA, pushed branch, PR URL/number/base/head/state, changed-file list, validation commands and exit codes, self-review outcome, primary-checkout preservation evidence, and residual risks. If auth/board/PR actions fail, stop safely and report exact state; do not improvise routing.

## Acceptance Contract
Acceptance level: checked
Completion is not accepted from prose alone. End with a structured acceptance report.

Criteria:
- criterion-1: Implement the requested change without widening scope
- criterion-2: Return evidence sufficient for an independent acceptance review

Required evidence: changed-files, tests-added, commands-run, residual-risks, no-staged-files, validation-output, diff-summary

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