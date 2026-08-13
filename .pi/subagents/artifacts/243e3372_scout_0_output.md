## Files Retrieved

1. `ideas/attack-die-3d/design.md` (PR diff lines 1-663) — sole added file; no other changed paths.

## Key Code

No executable code is present. Critical publication controls are:

```text
Closes #216
```

```text
The conversational design sections are approved, but Kirk must review and approve this written specification before planning or implementation begins.
```

## Architecture

Issue #216 is the Project 19 tracking item. PR #217 closes that issue and provides the single-file written-design review surface from remote branch `idea/216-attack-die-3d`.

## Field Evidence

### Issue #216

- URL: https://github.com/KirkDiggler/rpg-project/issues/216
- Title: `Design staged 3D attack die presentation`
- State: `OPEN`
- Exact body:

```markdown
## Goal

Turn the Kirk-approved conversational design for a staged 3D attack die into the canonical cross-repository written specification and review surface.

The design must preserve the authoritative server `d20` result, use the promoted private lightning d20 GLB, exercise a production-intent shared web component through a development-only concept, and define calibration, verification, fallback, accessibility, performance, ownership, and promotion gates without implementing any runtime code.

## Scope

- Add only `ideas/attack-die-3d/design.md` in `rpg-project`.
- Cover the asset and web ownership boundaries; rules, proto, and API own no work in this scope.
- Keep the design PR open as the cross-repository review surface.
- Create owning-repository implementation issues only after Kirk approves the written design.
- Do not add `plan.md` or implement web/assets changes in this issue.

The conversational design sections are approved. Kirk must review and approve the written specification before planning or implementation begins.

— asset-pipeline agent, on behalf of KirkDiggler
```

- Signature is the final nonblank line and occurs exactly once:
  `— asset-pipeline agent, on behalf of KirkDiggler`

### Project 19

- Project: `The Dungeon Run`, https://github.com/users/KirkDiggler/projects/19
- 451 items inspected; exactly one matched `KirkDiggler/rpg-project#216`.
- Status: `Todo`
- Team: `Assets`
- Feature: `Game Screen` — reported without judgment.
- Project item also reports linked PR #217.

### PR #217

- URL: https://github.com/KirkDiggler/rpg-project/pull/217
- Title: `docs: design staged 3D attack die`
- State: `OPEN`
- Draft: `false`
- Base: `main`
- Head: `idea/216-attack-die-3d`
- Head SHA: `427c3da60db15b1a84ed45d2bb353076666e0ded`
- Merge state: `CLEAN`; mergeability: `MERGEABLE`
- Review decision: empty; review requests: 0
- Changed files: 1
  - `ideas/attack-die-3d/design.md`: added, +663/−0
- Commits, in order:
  1. `5bdec83ef2083980581ea38bd83163f9bd606148` — `docs: design staged 3D attack die (#216)`
  2. `427c3da60db15b1a84ed45d2bb353076666e0ded` — `docs: tighten attack die acceptance (#216)`
- Exact body:

```markdown
## Summary

- makes the Kirk-approved conversational design for the staged 3D attack die canonical at `ideas/attack-die-3d/design.md`
- defines authoritative result-driven settlement, hash-bound 1–20 face calibration, material treatment, concept stages, SVG fallback/a11y, performance measurement, and graduation evidence
- assigns asset/web ownership and explicitly excludes rules, proto, API, implementation code, `plan.md`, and production promotion

The conversational design sections are approved, but Kirk must review and approve this written specification before planning or implementation begins. This ready PR stays open as the cross-repository review surface; later implementation PRs receive owning-repository issues after written design approval.

## Cross-repository scope

- `rpg-project`: canonical design and review surface only
- `rpg-game-assets`: future provenance, stable asset/material/face-map contract, and visual QA
- `rpg-dnd5e-web`: future shared component, concept, presentation, fallback/a11y, tests, and performance evidence
- rules/proto/API: no work for the recommended result-driven settle

## Validation

- `npx --yes markdownlint-cli2@0.18.1 ideas/attack-die-3d/design.md`
- `git diff --check origin/main...HEAD`
- placeholder/required-section/scope self-review
- confirmed the commit changes only `ideas/attack-die-3d/design.md`

Closes #216

— asset-pipeline agent, on behalf of KirkDiggler
```

- Signature is the final nonblank line and occurs exactly once.
- `Closes #216` occurs exactly once; GitHub GraphQL resolves exactly one closing issue reference, issue #216.
- The written-spec approval sentence occurs exactly once.
- Remote compare reports two commits ahead, zero behind, and only the design file.
- No `plan.md` or implementation file exists in the PR changes.
- Both the GitHub ref API and `git ls-remote` resolve the remote branch to the required SHA.
- PR is open, non-draft, clean, and mergeable: ready for the stated Kirk review.

## Review Findings

- **No publication defects detected.**
- **Info — `ideas/attack-die-3d/design.md:1-663`:** sole PR change, matching approved documentation-only scope.

## Residual Risks

- GitHub state is a point-in-time snapshot and can change after this audit.
- Kirk’s written-spec approval remains outstanding; no review decision or review request is recorded.

## Start Here

Start with PR #217 because it contains the canonical written specification, issue-closing linkage, approval gate, and complete remote branch/diff evidence.