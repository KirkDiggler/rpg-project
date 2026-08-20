# Composable Attack Damage PR Re-cut Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace PR #1144 with independently green, provider-first pull requests for canonical composable attack damage.

**Architecture:** The `rulebooks/dnd5e` provider lands first without modifying any consumer module. CI tags that provider on `main`; `resolution` then adopts the released provider, and `session` adopts both released dependencies. The frozen top-level `encounter` module remains unchanged on its old pins and is deliberately outside the legacy guard.

**Tech Stack:** Go multi-module repository, Git/GitHub CLI, GitHub Actions module auto-tagging, Bash legacy guard, ADR Markdown.

**Spec:** `ideas/composable-attack-damage/design.md` (to be copied from the approved design in the source branch before toolkit cleanup)

## Global Constraints

- Every toolkit PR changes exactly one Go module, except repository-level ADR and guard files accompanying the provider PR.
- Every PR must have green standalone CI before merge.
- Do not commit `go.work`, sibling-module `replace` directives, unpublished pseudo-versions, SDD reports, or `docs/superpowers/**` artifacts to `rpg-toolkit`.
- Do not modify the top-level `encounter/` module; it remains frozen on its published dependency pending removal.
- Preserve one Strike, one damage roll per pool, one fold, and one target application.
- Preserve canonical per-component damage typing and aggregate-only session recording.
- Do not restore deleted legacy attack APIs, fields, adapters, or compatibility mirrors.
- Keep Net out of the damage weapon catalog; document its deferral to a future restraining action in PR A.
- CI owns module tags after merge to `main`; never create tags manually.

---

## Checkpoint 0: Preserve design context and establish clean source boundaries

### Task 1: Move canonical design artifacts to `rpg-project`

**Files:**
- Create: `rpg-project/ideas/composable-attack-damage/design.md`
- Modify: `rpg-project/ideas/composable-attack-damage/plan.md`
- Source only: `rpg-toolkit/docs/superpowers/specs/2026-08-14-composable-attack-damage-design.md`
- Source only: `rpg-toolkit/docs/superpowers/plans/2026-08-19-composable-attack-damage.md`

**Interfaces:**
- Consumes: approved design and original implementation plan from branch `feat/composable-attack-damage-canonical`.
- Produces: project-owned design history that toolkit ADR-0040 may reference conceptually without linking to a toolkit-local Superpowers path.

- [ ] **Step 1: Copy the approved design into the project idea directory**

Use `apply_patch` to create `ideas/composable-attack-damage/design.md` with the complete contents of the approved toolkit design. Do not edit the source file in place.

- [ ] **Step 2: Confirm the project repository contains only intentional idea changes**

Run: `git -C /home/dammitbilly/game-dev/rpg-project status --short`

Expected: only `ideas/composable-attack-damage/design.md` and `ideas/composable-attack-damage/plan.md` are new or modified.

- [ ] **Step 3: Commit the project artifacts separately**

```bash
git -C /home/dammitbilly/game-dev/rpg-project add ideas/composable-attack-damage/design.md ideas/composable-attack-damage/plan.md
git -C /home/dammitbilly/game-dev/rpg-project commit -m "docs: preserve composable attack damage design"
```

### Task 2: Record immutable source and base SHAs

**Files:**
- Modify: `rpg-project/ideas/composable-attack-damage/plan.md`

**Interfaces:**
- Consumes: source branch `feat/composable-attack-damage-canonical` and current `origin/main`.
- Produces: exact source/base SHAs used by all path-limited re-cuts.

- [ ] **Step 1: Refresh the base without modifying either repository**

Run: `git -C /home/dammitbilly/game-dev/rpg-toolkit fetch origin main`

Expected: `origin/main` resolves successfully.

- [ ] **Step 2: Record the source and base**

Run:

```bash
git -C /home/dammitbilly/game-dev/rpg-toolkit rev-parse feat/composable-attack-damage-canonical
git -C /home/dammitbilly/game-dev/rpg-toolkit rev-parse origin/main
```

Expected source at plan creation: `2e980b875e0842b449aab0de298914e7104da4b0`. Update this plan with the actual base SHA immediately before execution.

- [ ] **Step 3: Confirm PR #1144 remains an unchanged source snapshot**

Run: `gh pr view 1144 --repo KirkDiggler/rpg-toolkit --json state,headRefOid,headRefName`

Expected: open PR, head `feat/composable-attack-damage-canonical`, head SHA matching the recorded source.

---

## Checkpoint 1: PR A — canonical D&D provider

### Task 3: Create a provider-only worktree and apply the provider diff

**Files:**
- Modify: `rulebooks/dnd5e/**`
- Exclude: `rulebooks/dnd5e/resolution/**`
- Exclude: `rulebooks/dnd5e/session/**`
- Exclude: `rulebooks/dnd5e/encounter/**`
- Exclude: `encounter/**`
- Modify: `scripts/check-no-legacy-attack.sh`
- Modify: `docs/adr/0036-additional-damage-selective-crit.md`
- Modify: `docs/adr/DECISIONS.md`
- Create: `docs/adr/0040-composable-attack-damage.md`
- Modify: `docs/adr/README.md`

**Interfaces:**
- Consumes: canonical damage declarations, monster actions, feature subscribers, and legacy deletion from source SHA `2e980b8`.
- Produces: a green `rulebooks/dnd5e` provider release containing canonical damage APIs and no consumer-module edits.

- [ ] **Step 1: Create the clean provider branch**

```bash
git -C /home/dammitbilly/game-dev/rpg-toolkit worktree add /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-provider -b feat/composable-attack-damage-provider origin/main
```

- [ ] **Step 2: Generate a path-limited provider patch**

Run from the source worktree:

```bash
git diff --binary --output=/tmp/composable-damage-provider.patch origin/main...2e980b8 -- \
  rulebooks/dnd5e \
  ':!rulebooks/dnd5e/resolution/**' \
  ':!rulebooks/dnd5e/session/**' \
  ':!rulebooks/dnd5e/encounter/**' \
  scripts/check-no-legacy-attack.sh \
  docs/adr/0036-additional-damage-selective-crit.md \
  docs/adr/DECISIONS.md
```

- [ ] **Step 3: Apply the provider patch in the new worktree**

Run: `git -C /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-provider apply -3 /tmp/composable-damage-provider.patch`

Expected: provider files apply; no `encounter/`, resolution, session, `.superpowers/`, or `docs/superpowers/` paths appear in `git status`.

### Task 4: Add ADR-0040 and correct ADR-0036 supersession

**Files:**
- Create: `docs/adr/0040-composable-attack-damage.md`
- Modify: `docs/adr/0036-additional-damage-selective-crit.md`
- Modify: `docs/adr/DECISIONS.md`
- Modify: `docs/adr/README.md`

**Interfaces:**
- Consumes: SRD 5.1 critical-hit ruling and approved component model.
- Produces: ADR-to-ADR supersession with `0036` pointing to `0040`.

- [ ] **Step 1: Write ADR-0040**

ADR-0040 must state:

- Status: Accepted.
- Attack damage is an ordered collection of typed pools.
- Exactly one pool carries `AddsAttackAbilityModifier`.
- All eligible attack dice, including Sneak Attack, double on a critical.
- `DoesNotCrit` is the explicit exception.
- Flat modifiers, including a Lifedrinker-shaped feature bonus, do not double.
- Resolution owns one roll per pool, one fold, and one application.
- Legacy singular damage declarations and attack resolver APIs are deleted.

- [ ] **Step 2: Point ADR-0036 to ADR-0040**

Set its status line to `Superseded by [ADR-0040](0040-composable-attack-damage.md)` and retain the historical body.

- [ ] **Step 3: Update both ADR indexes**

Add ADR-0040 to `README.md` and `DECISIONS.md`; describe ADR-0036 only as the superseded selective-critical proposal.

- [ ] **Step 4: Run documentation checks**

Run: `make check-decisions`

Expected: PASS.

### Task 5: Narrow and strengthen the legacy guard

**Files:**
- Modify: `scripts/check-no-legacy-attack.sh`

**Interfaces:**
- Consumes: deleted API/field vocabulary.
- Produces: provider/resolution guard that intentionally ignores frozen top-level encounter.

- [ ] **Step 1: Set exact production roots**

Use:

```bash
production_roots=(
  rulebooks/dnd5e/combat
  rulebooks/dnd5e/events
  rulebooks/dnd5e/conditions
  rulebooks/dnd5e/gamectx
  rulebooks/dnd5e/monster
  rulebooks/dnd5e/resolution
)
```

Add a comment that top-level `encounter/` is frozen on an old provider pin pending removal and is deliberately excluded.

Filter comment-only matches before failing the symbol scan so historical prose such as the existing `combat.ResolveAttackHit` comment in resolution does not fail the guard. Executable declarations, method names, calls, field types, and inline references must still fail it.

- [ ] **Step 2: Remove encounter-specific persisted-data scanning**

Restrict the `damage_dice|damage_type|damage_bonus|KnockdownDC|Scimitar...` scan to provider production paths. Do not scan `encounter/seed_monsters.go` in PR A.

- [ ] **Step 3: Run the guard**

Run: `bash scripts/check-no-legacy-attack.sh`

Expected: PASS on the provider-only branch.

### Task 6: Verify and open PR A

**Files:**
- No additional implementation files.

**Interfaces:**
- Consumes: provider-only branch.
- Produces: independently green provider PR and CI-generated D&D tag.

- [ ] **Step 1: Assert scope mechanically**

Run:

```bash
git diff --name-only origin/main...HEAD | rg '^(encounter/|rulebooks/dnd5e/(resolution|session|encounter)/|docs/superpowers/|\.superpowers/)'
```

Expected: no output and exit status 1.

- [ ] **Step 2: Run provider verification**

Run:

```bash
cd rulebooks/dnd5e
go test ./...
cd ../..
bash scripts/check-no-legacy-attack.sh
git diff --check
```

Expected: all PASS.

- [ ] **Step 3: Commit and open PR A**

```bash
git add rulebooks/dnd5e docs/adr scripts/check-no-legacy-attack.sh
git commit -m "feat(dnd5e): compose canonical attack damage"
git push -u origin feat/composable-attack-damage-provider
gh pr create --base main --head feat/composable-attack-damage-provider --title "feat(dnd5e): compose canonical attack damage"
```

PR body must mention SRD critical behavior, Net removal, no encounter migration, and that resolution/session follow after CI tags the provider.

- [ ] **Step 4: Address every automated review comment in-thread**

For each Copilot comment, either push a verified fix or reply with concrete code/test evidence explaining why no change is required.

- [ ] **Step 5: Merge only after every required check is green**

Expected after merge: safe CI creates `rulebooks/dnd5e/v0.97.0` or the next actual semver derived from current tags.

---

## Checkpoint 2: PR B — resolution consumer

### Task 7: Re-cut resolution against the released provider

**Files:**
- Modify: `rulebooks/dnd5e/resolution/**`
- Modify: `rulebooks/dnd5e/resolution/go.mod`
- Modify: `rulebooks/dnd5e/resolution/go.sum`

**Interfaces:**
- Consumes: released D&D provider containing `damage.Damage`, `damage.Instance`, `damage.Validate`, and weapon `DamageForGrip`.
- Produces: canonical `AttackProfile` compiler and typed multi-pool `StrikeOutcome` in the resolution module.

- [ ] **Step 1: Wait for the provider tag to resolve**

Run: `go list -m github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@v0.97.0`

Expected: module metadata, not an unknown-revision error. If CI chose a different next version, substitute the actual tag in every following command.

- [ ] **Step 2: Create the clean resolution branch from updated main**

```bash
git fetch origin main
git worktree add /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-resolution -b feat/composable-attack-damage-resolution origin/main
```

- [ ] **Step 3: Apply only the resolution source diff**

```bash
git diff --binary --output=/tmp/composable-damage-resolution.patch origin/main...2e980b8 -- rulebooks/dnd5e/resolution
git -C /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-resolution apply -3 /tmp/composable-damage-resolution.patch
```

- [ ] **Step 4: Pin the released provider and tidy**

```bash
cd rulebooks/dnd5e/resolution
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@v0.97.0
go mod tidy
```

- [ ] **Step 5: Run resolution verification**

Run: `go test ./...`

Expected: PASS, including multi-pool rolling, exact primary-marker selection, `DoesNotCrit`, canceled advantage, and one application.

- [ ] **Step 6: Assert scope, commit, and open PR B**

```bash
git diff --name-only origin/main...HEAD | rg -v '^rulebooks/dnd5e/resolution/'
git diff --check
git add rulebooks/dnd5e/resolution
git commit -m "feat(resolution): resolve composable attack damage"
git push -u origin feat/composable-attack-damage-resolution
gh pr create --base main --head feat/composable-attack-damage-resolution --title "feat(resolution): resolve composable attack damage"
```

Expected scope command: no output. Merge only with green standalone resolution CI.

Expected after merge: CI creates `rulebooks/dnd5e/resolution/v0.11.0` or the actual next tag.

---

## Checkpoint 3: PR C — session consumer

### Task 8: Re-cut aggregate-only session recording against released dependencies

**Files:**
- Modify: `rulebooks/dnd5e/session/attack_internal_test.go`
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`
- Modify only if required by the released encounter API selected by Go MVS: `rulebooks/dnd5e/session/read.go`, `rulebooks/dnd5e/session/write.go`, and their focused tests.

**Interfaces:**
- Consumes: D&D `v0.97.0` and resolution `v0.11.0` (or actual CI-generated successors).
- Produces: session adapter that consumes typed `StrikeOutcome` while persisting only `encounter.ValueAmount = struck.Damage`.

- [ ] **Step 1: Wait for the resolution tag to resolve**

Run: `go list -m github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/resolution@v0.11.0`

Expected: module metadata.

- [ ] **Step 2: Create the clean session branch from updated main**

```bash
git fetch origin main
git worktree add /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-session -b feat/composable-attack-damage-session origin/main
```

- [ ] **Step 3: Apply only the aggregate-boundary test**

```bash
git diff --binary --output=/tmp/composable-damage-session.patch origin/main...2e980b8 -- rulebooks/dnd5e/session/attack_internal_test.go
git -C /home/dammitbilly/game-dev/rpg-toolkit/.worktrees/composable-damage-session apply -3 /tmp/composable-damage-session.patch
```

- [ ] **Step 4: Pin released provider and resolution versions**

```bash
cd rulebooks/dnd5e/session
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@v0.97.0
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/resolution@v0.11.0
go mod tidy
```

- [ ] **Step 5: Compile before changing session APIs**

Run: `go test ./... -run '^$'`

Expected: PASS. If it fails specifically because MVS selects an encounter version whose `Atlas`, `Locate`, or connection error API differs, stop this PR and open a separately scoped session/encounter-generation compatibility PR; do not add aliases or pin an older transitive version to conceal the mismatch.

- [ ] **Step 6: Prove aggregate-only recording**

Run: `go test ./... -run 'AggregateFromTyped|RecordFor' -count=1`

Expected: PASS; `DamageInstances` is accepted on input but no typed collection is added to session record persistence.

- [ ] **Step 7: Run the complete session suite**

Run: `go test ./... -count=1`

Expected: PASS.

- [ ] **Step 8: Assert scope, commit, and open PR C**

```bash
git diff --name-only origin/main...HEAD | rg -v '^rulebooks/dnd5e/session/'
git diff --check
git add rulebooks/dnd5e/session
git commit -m "test(session): pin aggregate strike recording"
git push -u origin feat/composable-attack-damage-session
gh pr create --base main --head feat/composable-attack-damage-session --title "test(session): pin aggregate strike recording"
```

Expected scope command: no output. Merge only with green standalone session CI.

---

## Checkpoint 4: Retire the superseded combined PR

### Task 9: Close PR #1144 after replacement PR A is visible

**Files:**
- No repository files.

**Interfaces:**
- Consumes: open provider-only PR A and preserved source branch.
- Produces: unambiguous review history with no red combined PR competing for merge.

- [ ] **Step 1: Post the re-cut map on PR #1144**

Comment with links to PR A, the planned resolution/session successors, confirmation that top-level encounter was dropped, and the ADR/docs corrections.

- [ ] **Step 2: Close PR #1144 without deleting its branch**

Run: `gh pr close 1144 --repo KirkDiggler/rpg-toolkit --comment "Superseded by provider-first module PRs; preserving this branch as the reviewed source snapshot."`

- [ ] **Step 3: Keep the source branch until PR C merges**

Do not delete `feat/composable-attack-damage-canonical` until all path-limited patches have landed and the final session suite is green.

---

## Final acceptance

- [ ] PR A changes only the D&D provider plus ADR/guard files and is green.
- [ ] CI tags the provider after PR A merges.
- [ ] PR B changes only resolution, pins the released provider, and is green.
- [ ] CI tags resolution after PR B merges.
- [ ] PR C changes only session, pins released dependencies, and is green.
- [ ] Top-level `encounter/` is unchanged from `main` throughout.
- [ ] ADR-0036 is superseded by ADR-0040.
- [ ] No toolkit PR contains `docs/superpowers/**`, `.superpowers/**`, `go.work`, or local `replace` directives.
- [ ] Net removal is explicitly documented in PR A.
- [ ] PR #1144 is closed as superseded only after PR A is open.
