# Composable Attack Damage Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Publish the reconciled composable-damage record, close toolkit PR #1126 as superseded, and close toolkit issue #979 with evidence while leaving journey rpg-project#232 and all deferred technical work unchanged.

**Architecture:** The project-owned design is the canonical cross-repository record. Publish that record first, then use immutable merged PRs #1146, #1148, and #1156 as the evidence for closing the stale toolkit design surface and its Decide issue. Finish with read-only verification of Project 19 and both repositories.

**Tech Stack:** Markdown, Git, GitHub CLI (`gh`), GitHub Projects V2 GraphQL API.

**Spec:** `ideas/composable-attack-damage/design.md`

## Global Constraints

- Close rpg-toolkit PR #1126 as superseded; do not merge, rebase, or edit its branch.
- Close rpg-toolkit issue #979 only after the supersession comment is visible on #1126.
- Preserve rpg-project#232 as an open journey unless its own Done-when has been independently demonstrated; this plan does not demonstrate it.
- Do not create or implement top-level `encounter/` migration, strike-notification replacement, or off-hand/TWF gating work.
- Do not modify rpg-toolkit files, refs, branches, commits, or tags.
- GitHub comments and PR bodies use: `— asset-pipeline agent, on behalf of KirkDiggler`.
- If GitHub state differs from the preconditions below, stop before mutation and report the difference instead of guessing.

---

### Task 1: Publish the canonical reconciliation record

**Files:**
- Modify: `ideas/composable-attack-damage/design.md`
- Create: `ideas/composable-attack-damage/closure-plan.md`

**Interfaces:**
- Consumes: approved reconciliation design commit `a6d6239` on branch `docs/232-composable-damage-reconciliation`.
- Produces: a merged rpg-project documentation PR whose URL can be cited in toolkit closure comments.

- [ ] **Step 1: Verify the branch and approved design commit**

Run:

```bash
git -C /home/dammitbilly/game-dev/rpg-project status --short --branch
git -C /home/dammitbilly/game-dev/rpg-project log -2 --oneline
git -C /home/dammitbilly/game-dev/rpg-project diff --check
```

Expected: branch `docs/232-composable-damage-reconciliation`; commit `a6d6239 docs: define composable damage closure`; only this plan is uncommitted; `git diff --check` exits 0.

- [ ] **Step 2: Confirm the documented implementation evidence still exists**

Run:

```bash
gh pr view 1146 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,mergeCommit,url
gh pr view 1148 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,mergeCommit,url
gh pr view 1156 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,mergeCommit,url
```

Expected: all three report `state: MERGED` with non-null `mergedAt` and `mergeCommit` values. Stop if any result differs.

- [ ] **Step 3: Commit the closure plan**

Run:

```bash
git -C /home/dammitbilly/game-dev/rpg-project add ideas/composable-attack-damage/closure-plan.md
git -C /home/dammitbilly/game-dev/rpg-project commit -m "docs: plan composable damage closure"
```

Expected: one commit containing only `closure-plan.md`.

- [ ] **Step 4: Push the documentation branch and open its review surface**

Run:

```bash
git -C /home/dammitbilly/game-dev/rpg-project push -u origin docs/232-composable-damage-reconciliation
gh pr create --repo KirkDiggler/rpg-project --base main --head docs/232-composable-damage-reconciliation --title "docs: reconcile composable attack damage delivery" --body-file /tmp/composable-damage-reconciliation-pr.md
```

Before the command, create `/tmp/composable-damage-reconciliation-pr.md` with `apply_patch` using exactly:

```markdown
## Summary

- record #1146, #1148, and #1156 as the shipped composable-damage path
- define toolkit PR #1126 as a superseded historical design surface
- keep encounter migration, strike notifications, and TWF gating explicitly deferred
- add the evidence-first closure plan for toolkit issue #979

Parent journey: #232

— asset-pipeline agent, on behalf of KirkDiggler
```

Expected: GitHub returns a new rpg-project PR URL targeting `main`.

- [ ] **Step 5: Review and merge the documentation PR**

Run:

```bash
reconciliation_pr_number=$(gh pr view docs/232-composable-damage-reconciliation --repo KirkDiggler/rpg-project --json number --jq .number)
gh pr checks "$reconciliation_pr_number" --repo KirkDiggler/rpg-project --watch
gh pr view "$reconciliation_pr_number" --repo KirkDiggler/rpg-project --json mergeable,mergeStateStatus,reviewDecision,statusCheckRollup
```

Expected: required checks succeed and the PR reports a mergeable state. Obtain human approval if repository policy requires it, then squash-merge:

```bash
gh pr merge "$reconciliation_pr_number" --repo KirkDiggler/rpg-project --squash --delete-branch
```

Expected: the PR reports `MERGED`. The stable canonical record used by Tasks 2 and 3 is `https://github.com/KirkDiggler/rpg-project/blob/main/ideas/composable-attack-damage/design.md`.

---

### Task 2: Close toolkit PR #1126 as superseded

**Files:**
- No repository files change.

**Interfaces:**
- Consumes: the merged canonical design at `https://github.com/KirkDiggler/rpg-project/blob/main/ideas/composable-attack-damage/design.md` and immutable implementation PRs #1146, #1148, and #1156.
- Produces: a visible supersession record on closed, unmerged toolkit PR #1126.

- [ ] **Step 1: Re-check PR #1126 before mutation**

Run:

```bash
gh pr view 1126 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,headRefOid,title,url,comments
```

Expected: `state: OPEN`, `mergedAt: null`, head `205980d9eaa78eb5eeb3e0ecd35f477adc921ad5`, and no existing comment containing `closed as superseded by the shipped provider/resolution/session sequence`. If it is already closed with equivalent evidence, treat this task as complete without adding a duplicate comment. Stop on any other difference.

- [ ] **Step 2: Add the supersession comment**

Create `/tmp/rpg-toolkit-1126-superseded.md` with `apply_patch` using exactly:

```markdown
Closing this design PR as superseded by the shipped provider/resolution/session sequence:

- #1146 — canonical provider damage pools and rules;
- #1148 — `AttackProfile`, `Strike`, one fold/application, and typed outcomes;
- #1156 — aggregate-only session recording from the typed outcome.

The durable cross-repository rationale and closure record live in https://github.com/KirkDiggler/rpg-project/blob/main/ideas/composable-attack-damage/design.md. This branch is intentionally not merged because current `main` already contains the landed ADR, decision digest, and combat architecture, while this branch predates those reconciliations.

Deferred and not claimed complete here: the frozen top-level `encounter/` migration, strike-path notification replacement, and the off-hand/TWF gate tracked by #1155.

— asset-pipeline agent, on behalf of KirkDiggler
```

```bash
gh pr comment 1126 --repo KirkDiggler/rpg-toolkit --body-file /tmp/rpg-toolkit-1126-superseded.md
```

Expected: GitHub returns the new comment URL.

- [ ] **Step 3: Close without merging**

Run:

```bash
gh pr close 1126 --repo KirkDiggler/rpg-toolkit
gh pr view 1126 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,comments,url
```

Expected: `state: CLOSED`, `mergedAt: null`, and the supersession comment is present.

- [ ] **Step 4: Prove toolkit `main` was untouched**

Run:

```bash
gh api repos/KirkDiggler/rpg-toolkit/git/ref/heads/main --jq .object.sha
```

Compare the returned SHA with the SHA recorded immediately before Step 2 using the same command. Expected: identical SHAs.

---

### Task 3: Reconcile and close toolkit issue #979

**Files:**
- No repository files change.

**Interfaces:**
- Consumes: closed, unmerged PR #1126; the merged canonical project design; shipped PR evidence.
- Produces: closed Decide issue #979 and verified Project 19 state without closing journey #232.

- [ ] **Step 1: Verify issue and journey preconditions**

Run:

```bash
gh issue view 979 --repo KirkDiggler/rpg-toolkit --json state,title,url,comments
gh issue view 232 --repo KirkDiggler/rpg-project --json state,title,url
```

Expected: #979 is `OPEN`; #232 is `OPEN`. If #979 already contains an equivalent reconciliation comment and is closed, skip Steps 2–3. Stop if #232 is closed because that changes the approved scope.

- [ ] **Step 2: Add the reconciliation comment to #979**

Create `/tmp/rpg-toolkit-979-reconciled.md` with `apply_patch` using exactly:

```markdown
The composable attack damage decision is reconciled against the implementation that shipped after this Decide slice was opened:

- #1146 delivered the canonical provider and ADR;
- #1148 delivered `AttackProfile`, `Strike`, typed evidence, and the one-fold/one-application boundary;
- #1156 delivered the aggregate-only session boundary;
- #1126 is closed unmerged as the superseded toolkit-local design surface.

The canonical cross-repository rationale and closure record are preserved in https://github.com/KirkDiggler/rpg-project/blob/main/ideas/composable-attack-damage/design.md. The frozen top-level `encounter/` migration, strike-path notifications, and #1155 TWF gating remain outside this closure.

Closing this Decide issue as reconciled. Parent journey KirkDiggler/rpg-project#232 remains open pending its own Done-when proof.

— asset-pipeline agent, on behalf of KirkDiggler
```

```bash
gh issue comment 979 --repo KirkDiggler/rpg-toolkit --body-file /tmp/rpg-toolkit-979-reconciled.md
```

Expected: GitHub returns the new comment URL.

- [ ] **Step 3: Close #979**

Run:

```bash
gh issue close 979 --repo KirkDiggler/rpg-toolkit --reason completed
gh issue view 979 --repo KirkDiggler/rpg-toolkit --json state,closedAt,comments,url
```

Expected: `state: CLOSED`, non-null `closedAt`, and the reconciliation comment is present.

- [ ] **Step 4: Verify Project 19 and journey preservation**

Run:

```bash
gh api graphql --paginate -f query='query($endCursor:String) { user(login:"KirkDiggler") { projectV2(number:19) { items(first:100, after:$endCursor) { pageInfo { hasNextPage endCursor } nodes { content { ... on Issue { number state repository { nameWithOwner } } } fieldValues(first:20) { nodes { ... on ProjectV2ItemFieldSingleSelectValue { name field { ... on ProjectV2FieldCommon { name } } } } } } } } } }' --jq '.data.user.projectV2.items.nodes[] | select((.content.repository.nameWithOwner == "KirkDiggler/rpg-toolkit" and .content.number == 979) or (.content.repository.nameWithOwner == "KirkDiggler/rpg-project" and .content.number == 232)) | [ .content.repository.nameWithOwner, (.content.number|tostring), .content.state, ([.fieldValues.nodes[] | select(.field.name == "Status") | .name][0] // "") ] | @tsv'
```

Expected final state:

```text
KirkDiggler/rpg-toolkit  979  CLOSED  Done
KirkDiggler/rpg-project  232  OPEN    In Progress
```

If #979 remains `In Review` after issue closure, derive the exact Project 19 IDs and update only that item:

```bash
project_id=$(gh api graphql -f query='query { user(login:"KirkDiggler") { projectV2(number:19) { id } } }' --jq .data.user.projectV2.id)
status_field_id=$(gh api graphql -f query='query { user(login:"KirkDiggler") { projectV2(number:19) { fields(first:50) { nodes { ... on ProjectV2SingleSelectField { id name options { id name } } } } } } }' --jq '.data.user.projectV2.fields.nodes[] | select(.name == "Status") | .id')
done_option_id=$(gh api graphql -f query='query { user(login:"KirkDiggler") { projectV2(number:19) { fields(first:50) { nodes { ... on ProjectV2SingleSelectField { name options { id name } } } } } } }' --jq '.data.user.projectV2.fields.nodes[] | select(.name == "Status") | .options[] | select(.name == "Done") | .id')
item_979_id=$(gh api graphql --paginate -f query='query($endCursor:String) { user(login:"KirkDiggler") { projectV2(number:19) { items(first:100, after:$endCursor) { pageInfo { hasNextPage endCursor } nodes { id content { ... on Issue { number repository { nameWithOwner } } } } } } } }' --jq '.data.user.projectV2.items.nodes[] | select(.content.repository.nameWithOwner == "KirkDiggler/rpg-toolkit" and .content.number == 979) | .id')
gh project item-edit --id "$item_979_id" --project-id "$project_id" --field-id "$status_field_id" --single-select-option-id "$done_option_id"
```

Require all four IDs to be non-empty before `item-edit`; otherwise stop without mutation. Do not change any field on #232. Re-run the paginated query and require the expected state.

- [ ] **Step 5: Run the final closure audit**

Run:

```bash
gh pr view 1126 --repo KirkDiggler/rpg-toolkit --json state,mergedAt,url
gh issue view 979 --repo KirkDiggler/rpg-toolkit --json state,closedAt,url
gh issue view 232 --repo KirkDiggler/rpg-project --json state,url
git -C /home/dammitbilly/game-dev/rpg-project status --short --branch
git -C /home/dammitbilly/game-dev/rpg-toolkit status --short --branch
```

Expected: #1126 closed and unmerged; #979 closed; #232 open; both local worktrees clean; no toolkit commit or branch mutation occurred during Tasks 2–3.
