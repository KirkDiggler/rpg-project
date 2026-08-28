# Dev-Default Integration Branch Rollout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Before creating either implementation worktree, use superpowers:using-git-worktrees.

**Goal:** Make `dev` the safe GitHub default integration branch for `rpg-api` and `rpg-dnd5e-web` while keeping production image publication and deployment explicitly tied to `main`.

**Architecture:** Land the default-independent Docker tag gate first, replace default-relative protection with explicit `main` and `dev` rulesets, then promote the safety changes through real merge commits before changing either default branch. Every GitHub mutation is snapshotted and read back, every production merge is a Kirk-controlled checkpoint, and the two repositories are promoted sequentially so each image and downstream deployment can be attributed exactly.

**Tech Stack:** GitHub Actions YAML, Git/Git worktrees, GitHub REST and GraphQL APIs through `gh`, `jq`, Python 3, Go repository CI, npm/Vitest repository CI, Docker/GHCR read-back

**Spec:** `ideas/team-workflow/dev-default-branches/design.md`

## Global Constraints

- This rollout owns only rpg-project#312, rpg-project#313/PR #314, rpg-api#855/#856, and rpg-dnd5e-web#843/#844.
- `rpg-api` and `rpg-dnd5e-web` use `dev`; every other repository retains its existing base/default policy.
- Feature PRs target `dev` and squash. Production promotions use head `dev`, base `main`, and a real merge commit.
- Never rebase either long-lived branch and never force-push.
- `main` remains the only branch that publishes `latest` or dispatches production deployment.
- Keep automatic head-branch deletion enabled, but protect both `main` and `dev` from deletion.
- Keep strict required status checks and zero bypass actors on both explicit rulesets.
- Disable repository-level rebase merging only after both production promotions succeed.
- A future production hotfix uses two issues and two reviewed PRs: merge the fix from `main`, then replay that merge's first-parent diff with `git cherry-pick -m 1` on a branch from `dev` and squash the sync PR into `dev`. Never weaken the final merge-method rules for a hotfix.
- Do not merge any PR without a fresh, explicit instruction from Kirk. In particular, agents never merge the two promotion PRs.
- Treat rpg-api PR #853 as another active lane. Do not retarget it while its owner is writing; stop for a Kirk/owner checkpoint first.
- Do not touch existing unrelated worktrees or untracked files.
- Use the Platform Team signature on every agent-authored issue/PR comment: `— platform agent, on behalf of KirkDiggler`.
- Skip Copilot review for the two one-line workflow PRs, the two promotion PRs, and the documentation-only tracking PR; project policy reserves the quota for substantive features and engine work.
- Stop on a mutation error, a mismatched read-back, a missing required check, a non-content-neutral ancestry update, or a production/deployment failure. Preserve evidence and do not continue into the next repository.

## File and live-resource map

| Slice | Files/resources | Responsibility |
|---|---|---|
| rpg-api#856 | `rpg-api/.github/workflows/docker.yml` | Make `latest` explicitly main-only while retaining `:dev` publication |
| rpg-dnd5e-web#844 | `rpg-dnd5e-web/.github/workflows/docker.yml` | Make `latest` explicitly main-only while retaining main-only Docker publication |
| rpg-api settings | rulesets 6668107 plus one new dev ruleset | Explicit merge-only `main` and squash-only `dev` protection |
| web settings | ruleset 6782045 plus one new dev ruleset | Explicit merge-only `main` and squash-only `dev` protection |
| rpg-api#855 | PR with head `dev`, base `main` | Merge-commit promotion, ancestry repair, image/deployment proof |
| rpg-dnd5e-web#843 | PR with head `dev`, base `main` | Merge-commit promotion, ancestry repair, image/deployment proof |
| rpg-project#313 / PR #314 | `ideas/team-workflow/dev-default-branches/plan.md`, `CLAUDE.md` | Plan, final living policy, rollout ledger |
| local workspace | `/home/kirk/personal/CLAUDE.md` | Remove the stale all-repositories-start-from-main instruction |

## Stable interfaces and IDs

```text
rpg-api production ruleset:       6668107
rpg-api required checks:          test, build
rpg-dnd5e-web production ruleset: 6782045
web dev required checks:          Test, Lint and Type Check, Security Audit
web main required checks:         Test, Lint and Type Check, Security Audit, build
rpg-deployment workflow:          Deploy RPG Platform (175201148)
```

The existing disabled rpg-api ruleset `No Force` (7235062) remains disabled and unchanged.

---

### Task 1: Snapshot live state and prove the rollout is still safe to start

**Files:**
- Create locally: `$HOME/.cache/pi/dev-default-branches-312/**`
- Modify tracked files: none

**Interfaces:**
- Consumes: approved design, current GitHub repositories, Project 19 items, and open PRs.
- Produces: immutable baseline JSON and a content-neutral merge-tree proof used by Tasks 5–10.

- [ ] **Step 1: Verify identity, board state, and the tracking PR**

```bash
set -euo pipefail
test "$(gh api user --jq .login)" = "KirkDiggler"

gh project item-list 19 --owner KirkDiggler --limit 1000 --format json \
  > /tmp/project19-dev-default.json
jq -e '[.items[] | select(
  .content.url? == "https://github.com/KirkDiggler/rpg-project/issues/312" or
  .content.url? == "https://github.com/KirkDiggler/rpg-project/issues/313" or
  .content.url? == "https://github.com/KirkDiggler/rpg-api/issues/855" or
  .content.url? == "https://github.com/KirkDiggler/rpg-api/issues/856" or
  .content.url? == "https://github.com/KirkDiggler/rpg-dnd5e-web/issues/843" or
  .content.url? == "https://github.com/KirkDiggler/rpg-dnd5e-web/issues/844"
)] | length == 6' /tmp/project19-dev-default.json >/dev/null

gh pr view 314 --repo KirkDiggler/rpg-project \
  --json state,isDraft,headRefName,baseRefName,headRefOid,url \
  > /tmp/rpg-project-314.json
jq -e '.state=="OPEN" and .isDraft==true and
  .headRefName=="docs/313-dev-default-branch-workflow" and
  .baseRefName=="main"' /tmp/rpg-project-314.json >/dev/null
```

Expected: all six items are boarded and PR #314 is the open draft tracking surface.

- [ ] **Step 2: Capture the immutable GitHub baseline**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
test ! -e "$cache/baseline.sha256"
mkdir -p "$cache"

for repo in rpg-api rpg-dnd5e-web; do
  gh api "repos/KirkDiggler/$repo" > "$cache/$repo-repository-before.json"
  gh api "repos/KirkDiggler/$repo/branches/main" > "$cache/$repo-main-before.json"
  gh api "repos/KirkDiggler/$repo/branches/dev" > "$cache/$repo-dev-before.json"
  gh api "repos/KirkDiggler/$repo/rulesets" > "$cache/$repo-rulesets-before.json"
  for id in $(jq -r '.[].id' "$cache/$repo-rulesets-before.json"); do
    gh api "repos/KirkDiggler/$repo/rulesets/$id" \
      > "$cache/$repo-ruleset-$id-before.json"
  done
  gh api "repos/KirkDiggler/$repo/compare/main...dev" \
    > "$cache/$repo-main-dev-before.json"
  gh api "repos/KirkDiggler/$repo/contents/.github/workflows/docker.yml?ref=dev" \
    --jq '.content' | base64 -d > "$cache/$repo-docker-dev-before.yml"
done

gh api repos/KirkDiggler/rpg-api/pulls/853 > "$cache/rpg-api-pr-853-before.json"
gh api repos/KirkDiggler/rpg-project/pulls/314 > "$cache/rpg-project-pr-314-before.json"
gh run list --repo KirkDiggler/rpg-deployment --workflow 175201148 \
  --event workflow_dispatch --limit 20 \
  --json databaseId,status,conclusion,createdAt,url \
  > "$cache/deployment-runs-before.json"
sha256sum "$cache"/*-before.* > "$cache/baseline.sha256"
```

Expected: every baseline file is non-empty and the checksum file is written once.

- [ ] **Step 3: Assert the verified starting configuration has not drifted**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
for repo in rpg-api rpg-dnd5e-web; do
  jq -e '.default_branch=="main" and .delete_branch_on_merge==true and
    .allow_merge_commit==true and .allow_squash_merge==true and
    .allow_rebase_merge==true' \
    "$cache/$repo-repository-before.json" >/dev/null
  jq -e '.protected==true' "$cache/$repo-main-before.json" >/dev/null
  jq -e '.protected==false' "$cache/$repo-dev-before.json" >/dev/null
  test "$(rg -c 'type=raw,value=latest,enable=\{\{is_default_branch\}\}' \
    "$cache/$repo-docker-dev-before.yml")" = 1
  test "$(rg -c "github.ref == 'refs/heads/main'" \
    "$cache/$repo-docker-dev-before.yml")" = 1
done

jq -e '.name=="Protect The President" and .enforcement=="active" and
  .conditions.ref_name.include==["~DEFAULT_BRANCH"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]]) and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["build","test"]) and
  .bypass_actors==[]' \
  "$cache/rpg-api-ruleset-6668107-before.json" >/dev/null

jq -e '.name=="Protect The President" and .enforcement=="active" and
  .conditions.ref_name.include==["~DEFAULT_BRANCH"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]]) and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["Lint and Type Check","Security Audit","Test","build"]) and
  .bypass_actors==[]' \
  "$cache/rpg-dnd5e-web-ruleset-6782045-before.json" >/dev/null
```

Expected: exit 0. Any mismatch means the approved design must be reconciled before a mutation.

- [ ] **Step 4: Prove merging current `main` into `dev` is content-neutral**

```bash
set -euo pipefail
for repo in rpg-api rpg-dnd5e-web; do
  root="/home/kirk/game-dev/$repo"
  git -C "$root" fetch origin
  out="$HOME/.cache/pi/dev-default-branches-312/$repo-merge-tree-before.txt"
  git -C "$root" merge-tree --write-tree origin/dev origin/main > "$out"
  prospective="$(head -n 1 "$out")"
  dev_tree="$(git -C "$root" rev-parse 'origin/dev^{tree}')"
  test "$prospective" = "$dev_tree"
done
```

Expected: both prospective merge trees equal the current `dev` tree. A conflict or different tree is a stop condition because an ancestry repair must not silently alter code.

---

### Task 2: Land the rpg-api production-tag safety PR

**Issue/PR:** rpg-api#856; PR base `dev`.

**Files:**
- Modify: `.github/workflows/docker.yml:65`

**Interfaces:**
- Consumes: current Docker metadata-action configuration on `origin/dev`.
- Produces: `dev`/SHA image tags unchanged, but `latest` enabled only when `github.ref` is `refs/heads/main`.

- [ ] **Step 1: Create the isolated issue branch from fresh `origin/dev`**

Invoke superpowers:using-git-worktrees, then run:

```bash
set -euo pipefail
repo=/home/kirk/game-dev/rpg-api
wt=/home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
git -C "$repo" fetch origin
test ! -e "$wt"
test -z "$(git -C "$repo" branch --list 'ci/856-main-only-latest')"
git -C "$repo" worktree add -b ci/856-main-only-latest "$wt" origin/dev
git -C "$wt" status --short --branch
```

Expected: a clean worktree based exactly on `origin/dev`.

- [ ] **Step 2: Run the desired-state assertion and observe it fail**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
! rg -n -F 'type=raw,value=latest,enable=${{ github.ref == '\''refs/heads/main'\'' }}' \
  .github/workflows/docker.yml
```

Expected: exit 0 from the negated search, proving the main-only gate is absent.

- [ ] **Step 3: Replace exactly the default-relative gate**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
python3 - <<'PY'
from pathlib import Path
path = Path('.github/workflows/docker.yml')
text = path.read_text()
old = 'type=raw,value=latest,enable={{is_default_branch}}'
new = "type=raw,value=latest,enable=${{ github.ref == 'refs/heads/main' }}"
assert text.count(old) == 1
assert new not in text
path.write_text(text.replace(old, new))
PY
```

Expected: only the metadata tag line changes.

- [ ] **Step 4: Run focused structural verification**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
test "$(rg -c -F "type=raw,value=latest,enable=\${{ github.ref == 'refs/heads/main' }}" \
  .github/workflows/docker.yml)" = 1
! rg -n 'is_default_branch' .github/workflows/docker.yml
test "$(rg -c -F "github.ref == 'refs/heads/main'" .github/workflows/docker.yml)" = 2
git diff --check
test "$(git diff --name-only | paste -sd, -)" = '.github/workflows/docker.yml'
```

Expected: one tag gate and one deployment gate explicitly name `main`; no other file changed.

- [ ] **Step 5: Run the repository gates and reassert one-file scope**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
make pre-commit
make ci-check
git diff --check
test "$(git diff --name-only | paste -sd, -)" = '.github/workflows/docker.yml'
```

Expected: both gates pass without generating unrelated tracked changes.

- [ ] **Step 6: Commit, push, and open the PR against `dev`**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/856-main-only-latest
git add .github/workflows/docker.yml
git commit -m 'ci: keep latest image on main (#856)'
git push -u origin ci/856-main-only-latest
cat > /tmp/rpg-api-856-pr.md <<'EOF'
## Summary

- makes the production `latest` tag explicit to `refs/heads/main`
- preserves the existing `dev`, PR, and SHA image tags
- leaves the existing main-only deployment dispatch unchanged

## Verification

- focused workflow assertions
- `make pre-commit`
- `make ci-check`

Parent: KirkDiggler/rpg-project#312

Closes #856

— platform agent, on behalf of KirkDiggler
EOF
gh pr create --repo KirkDiggler/rpg-api \
  --base dev --head ci/856-main-only-latest \
  --title 'ci: make latest image explicitly main-only' \
  --body-file /tmp/rpg-api-856-pr.md
```

Expected: the PR base is `dev`; no review bot is requested.

- [ ] **Step 7: Read back scope and required checks**

```bash
pr="$(gh pr list --repo KirkDiggler/rpg-api --head ci/856-main-only-latest \
  --json number --jq '.[0].number')"
gh pr view "$pr" --repo KirkDiggler/rpg-api \
  --json state,isDraft,baseRefName,headRefName,files,url \
  > /tmp/rpg-api-856-readback.json
jq -e '.state=="OPEN" and .baseRefName=="dev" and
  .headRefName=="ci/856-main-only-latest" and
  (.files|length)==1 and .files[0].path==".github/workflows/docker.yml"' \
  /tmp/rpg-api-856-readback.json >/dev/null
gh pr checks "$pr" --repo KirkDiggler/rpg-api --watch --interval 10
```

Expected: `test` and `build` pass. Do not merge yet.

---

### Task 3: Land the web production-tag safety PR

**Issue/PR:** rpg-dnd5e-web#844; PR base `dev`.

**Files:**
- Modify: `.github/workflows/docker.yml:66`

**Interfaces:**
- Consumes: current web Docker metadata-action configuration on `origin/dev`.
- Produces: `latest` enabled only for `refs/heads/main`; normal `dev` CI remains independent of the main-only Docker workflow.

- [ ] **Step 1: Create the isolated issue branch from fresh `origin/dev`**

Invoke superpowers:using-git-worktrees, then run:

```bash
set -euo pipefail
repo=/home/kirk/game-dev/rpg-dnd5e-web
wt=/home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
git -C "$repo" fetch origin
test ! -e "$wt"
test -z "$(git -C "$repo" branch --list 'ci/844-main-only-latest')"
git -C "$repo" worktree add -b ci/844-main-only-latest "$wt" origin/dev
git -C "$wt" status --short --branch
```

Expected: a clean worktree based exactly on `origin/dev`.

- [ ] **Step 2: Run the desired-state assertion and observe it fail**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
! rg -n -F 'type=raw,value=latest,enable=${{ github.ref == '\''refs/heads/main'\'' }}' \
  .github/workflows/docker.yml
```

Expected: the desired explicit gate is absent.

- [ ] **Step 3: Replace exactly the default-relative gate**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
python3 - <<'PY'
from pathlib import Path
path = Path('.github/workflows/docker.yml')
text = path.read_text()
old = 'type=raw,value=latest,enable={{is_default_branch}}'
new = "type=raw,value=latest,enable=${{ github.ref == 'refs/heads/main' }}"
assert text.count(old) == 1
assert new not in text
path.write_text(text.replace(old, new))
PY
```

Expected: only the metadata tag line changes.

- [ ] **Step 4: Run focused structural verification**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
test "$(rg -c -F "type=raw,value=latest,enable=\${{ github.ref == 'refs/heads/main' }}" \
  .github/workflows/docker.yml)" = 1
! rg -n 'is_default_branch' .github/workflows/docker.yml
test "$(rg -c -F "github.ref == 'refs/heads/main'" .github/workflows/docker.yml)" = 2
git diff --check
test "$(git diff --name-only | paste -sd, -)" = '.github/workflows/docker.yml'
```

Expected: one main-only tag gate and one main-only deployment gate.

- [ ] **Step 5: Install dependencies and run the repository gate**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
npm ci
npm run ci-check
git diff --check
test "$(git diff --name-only | paste -sd, -)" = '.github/workflows/docker.yml'
```

Expected: the full web gate passes without lockfile or source changes.

- [ ] **Step 6: Commit, push, and open the PR against `dev`**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/844-main-only-latest
git add .github/workflows/docker.yml
git commit -m 'ci: keep latest image on main (#844)'
git push -u origin ci/844-main-only-latest
cat > /tmp/rpg-web-844-pr.md <<'EOF'
## Summary

- makes the production `latest` tag explicit to `refs/heads/main`
- preserves main-only Docker publication and deployment dispatch
- leaves normal `dev` CI unchanged

## Verification

- focused workflow assertions
- `npm run ci-check`

Parent: KirkDiggler/rpg-project#312

Closes #844

— platform agent, on behalf of KirkDiggler
EOF
gh pr create --repo KirkDiggler/rpg-dnd5e-web \
  --base dev --head ci/844-main-only-latest \
  --title 'ci: make latest image explicitly main-only' \
  --body-file /tmp/rpg-web-844-pr.md
```

Expected: the PR base is `dev`; no review bot is requested.

- [ ] **Step 7: Read back scope and required checks**

```bash
pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --head ci/844-main-only-latest \
  --json number --jq '.[0].number')"
gh pr view "$pr" --repo KirkDiggler/rpg-dnd5e-web \
  --json state,isDraft,baseRefName,headRefName,files,url \
  > /tmp/rpg-web-844-readback.json
jq -e '.state=="OPEN" and .baseRefName=="dev" and
  .headRefName=="ci/844-main-only-latest" and
  (.files|length)==1 and .files[0].path==".github/workflows/docker.yml"' \
  /tmp/rpg-web-844-readback.json >/dev/null
gh pr checks "$pr" --repo KirkDiggler/rpg-dnd5e-web --watch --interval 10
```

Expected: `Test`, `Lint and Type Check`, and `Security Audit` pass. A Docker `build` check is not expected on a PR to `dev`. Do not merge yet.

---

### Task 4: Reconcile the active API feature PR before protecting `dev`

**Live resource:** rpg-api PR #853.

**Files:**
- Modify tracked files: none
- Modify through GitHub API: PR #853 base branch only

**Interfaces:**
- Consumes: the active dice lane's stable head and owner checkpoint.
- Produces: an active feature PR targeting integration `dev`, or an explicit stop record if retargeting is not yet safe.

- [ ] **Step 1: Obtain the active-lane checkpoint**

Ask Kirk or the PR #853 owner to confirm that no push/base mutation is in progress. Do not proceed while another agent is writing the branch.

Expected: an explicit safe-to-retarget confirmation. Silence is not approval.

- [ ] **Step 2: Snapshot the PR head, files, and patch**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
gh pr view 853 --repo KirkDiggler/rpg-api \
  --json state,isDraft,headRefName,headRefOid,baseRefName,mergeable,mergeStateStatus,files,url \
  > "$cache/rpg-api-pr-853-pre-retarget.json"
gh pr diff 853 --repo KirkDiggler/rpg-api --patch \
  > "$cache/rpg-api-pr-853-pre-retarget.patch"
jq -e '.state=="OPEN" and .headRefName=="feat/852-shared-dice-presentation" and
  .baseRefName=="main"' "$cache/rpg-api-pr-853-pre-retarget.json" >/dev/null
```

Expected: the head matches the owner-approved SHA and the current base is still `main`.

- [ ] **Step 3: Change only the PR base to `dev` and read it back**

```bash
gh api --method PATCH repos/KirkDiggler/rpg-api/pulls/853 -f base=dev \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-api-pr-853-retarget-response.json"
gh pr view 853 --repo KirkDiggler/rpg-api \
  --json state,headRefName,headRefOid,baseRefName,mergeable,mergeStateStatus,files,url \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-api-pr-853-post-retarget.json"
jq -e '.state=="OPEN" and .headRefName=="feat/852-shared-dice-presentation" and
  .baseRefName=="dev"' \
  "$HOME/.cache/pi/dev-default-branches-312/rpg-api-pr-853-post-retarget.json" >/dev/null
```

Expected: only `baseRefName` changes immediately; the head SHA is unchanged.

- [ ] **Step 4: Return the retargeted diff to its owner for validation**

```bash
gh pr diff 853 --repo KirkDiggler/rpg-api --patch \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-api-pr-853-post-retarget.patch"
gh pr checks 853 --repo KirkDiggler/rpg-api --watch --interval 10
```

Expected: `test` and `build` rerun against `dev`. The dice-lane owner confirms the resulting file set and behavior; this rollout does not edit that branch. If the new base exposes an unexpected conflict or loses intended changes, restore `base=main`, record the evidence, and stop before Task 5.

---

### Task 5: Replace the API default-relative ruleset with explicit main/dev protection

**Live resources:** rpg-api ruleset 6668107 and one newly created dev ruleset.

**Files:**
- Create locally: `$HOME/.cache/pi/dev-default-branches-312/rpg-api-*-payload.json`
- Modify through GitHub API: repository rulesets only

**Interfaces:**
- Consumes: the exact baseline ruleset and the passing API safety PR.
- Produces: merge-only protected `main`, squash-only protected `dev`, strict `test`/`build`, no bypass actors.

- [ ] **Step 1: Re-read and assert the mutation preconditions**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api repos/KirkDiggler/rpg-api > "$cache/rpg-api-repository-pre-rules.json"
gh api repos/KirkDiggler/rpg-api/rulesets/6668107 \
  > "$cache/rpg-api-ruleset-6668107-pre-update.json"
gh api repos/KirkDiggler/rpg-api/rulesets > "$cache/rpg-api-rulesets-pre-update.json"
jq -e '.default_branch=="main"' "$cache/rpg-api-repository-pre-rules.json" >/dev/null
jq -e '[.[]|select(.name=="Protect dev (integration)")] | length==0' \
  "$cache/rpg-api-rulesets-pre-update.json" >/dev/null
cmp "$cache/rpg-api-ruleset-6668107-before.json" \
  "$cache/rpg-api-ruleset-6668107-pre-update.json"
```

Expected: default remains `main`, no dev ruleset exists, and the existing production ruleset has not drifted since Task 1.

- [ ] **Step 2: Generate explicit payloads from the verified live object**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
source="$cache/rpg-api-ruleset-6668107-pre-update.json"

jq '{
  name:"Protect main (production)",
  target:"branch",
  enforcement:"active",
  bypass_actors:.bypass_actors,
  conditions:{ref_name:{exclude:[],include:["refs/heads/main"]}},
  rules:(.rules | map(
    if .type=="pull_request"
    then (.parameters.allowed_merge_methods=["merge"])
    else . end
  ))
}' "$source" > "$cache/rpg-api-main-ruleset-payload.json"

jq '{
  name:"Protect dev (integration)",
  target:"branch",
  enforcement:"active",
  bypass_actors:.bypass_actors,
  conditions:{ref_name:{exclude:[],include:["refs/heads/dev"]}},
  rules:(.rules | map(
    if .type=="pull_request"
    then (.parameters.allowed_merge_methods=["squash"])
    else . end
  ))
}' "$source" > "$cache/rpg-api-dev-ruleset-payload.json"

jq -e '.bypass_actors==[] and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["build","test"]) and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]])' \
  "$cache/rpg-api-main-ruleset-payload.json" >/dev/null
jq -e '.bypass_actors==[] and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["build","test"]) and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]])' \
  "$cache/rpg-api-dev-ruleset-payload.json" >/dev/null
```

Expected: payloads differ only in name, explicit target, and allowed PR merge method.

- [ ] **Step 3: Update production protection before creating dev protection**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api --method PUT repos/KirkDiggler/rpg-api/rulesets/6668107 \
  --input "$cache/rpg-api-main-ruleset-payload.json" \
  > "$cache/rpg-api-main-ruleset-update-response.json"
gh api repos/KirkDiggler/rpg-api/rulesets/6668107 \
  > "$cache/rpg-api-main-ruleset-readback.json"
jq -e '.name=="Protect main (production)" and
  .conditions.ref_name.include==["refs/heads/main"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]]) and
  .bypass_actors==[]' "$cache/rpg-api-main-ruleset-readback.json" >/dev/null
```

Expected: `main` remains protected while still the default, now by explicit ref and merge-only PR policy.

- [ ] **Step 4: Create and verify dev protection**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api --method POST repos/KirkDiggler/rpg-api/rulesets \
  --input "$cache/rpg-api-dev-ruleset-payload.json" \
  > "$cache/rpg-api-dev-ruleset-create-response.json"
api_dev_ruleset="$(jq -r .id "$cache/rpg-api-dev-ruleset-create-response.json")"
test "$api_dev_ruleset" != null
echo "$api_dev_ruleset" > "$cache/rpg-api-dev-ruleset-id.txt"
gh api "repos/KirkDiggler/rpg-api/rulesets/$api_dev_ruleset" \
  > "$cache/rpg-api-dev-ruleset-readback.json"
jq -e '.name=="Protect dev (integration)" and
  .conditions.ref_name.include==["refs/heads/dev"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]]) and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.strict_required_status_checks_policy] == [true]) and
  .bypass_actors==[]' "$cache/rpg-api-dev-ruleset-readback.json" >/dev/null
gh api repos/KirkDiggler/rpg-api/branches/main | jq -e '.protected==true' >/dev/null
gh api repos/KirkDiggler/rpg-api/branches/dev | jq -e '.protected==true' >/dev/null
```

Expected: both long-lived branches report protected.

- [ ] **Step 5: Record the read-back on rpg-api#856**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
api_dev_ruleset="$(cat "$cache/rpg-api-dev-ruleset-id.txt")"
cat > /tmp/rpg-api-856-rules-comment.md <<EOF
Ruleset installation read back successfully before the safety PR merge:

- production: ruleset 6668107, explicit \`refs/heads/main\`, merge-only
- integration: ruleset $api_dev_ruleset, explicit \`refs/heads/dev\`, squash-only
- required checks on both: \`test\`, \`build\`, strict
- bypass actors: none
- both \`main\` and \`dev\` report protected

The repository default remains \`main\`; the Docker safety PR is not merged yet.

— platform agent, on behalf of KirkDiggler
EOF
gh issue comment 856 --repo KirkDiggler/rpg-api \
  --body-file /tmp/rpg-api-856-rules-comment.md
```

Expected: the issue records the exact created ruleset ID and verified pre-merge state.

---

### Task 6: Replace the web default-relative ruleset with explicit main/dev protection

**Live resources:** web ruleset 6782045 and one newly created dev ruleset.

**Files:**
- Create locally: `$HOME/.cache/pi/dev-default-branches-312/rpg-dnd5e-web-*-payload.json`
- Modify through GitHub API: repository rulesets only

**Interfaces:**
- Consumes: the exact baseline ruleset and the passing web safety PR.
- Produces: merge-only `main` with four checks; squash-only `dev` with the three checks that actually run there.

- [ ] **Step 1: Re-read and assert mutation preconditions**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api repos/KirkDiggler/rpg-dnd5e-web > "$cache/rpg-dnd5e-web-repository-pre-rules.json"
gh api repos/KirkDiggler/rpg-dnd5e-web/rulesets/6782045 \
  > "$cache/rpg-dnd5e-web-ruleset-6782045-pre-update.json"
gh api repos/KirkDiggler/rpg-dnd5e-web/rulesets \
  > "$cache/rpg-dnd5e-web-rulesets-pre-update.json"
jq -e '.default_branch=="main"' "$cache/rpg-dnd5e-web-repository-pre-rules.json" >/dev/null
jq -e '[.[]|select(.name=="Protect dev (integration)")] | length==0' \
  "$cache/rpg-dnd5e-web-rulesets-pre-update.json" >/dev/null
cmp "$cache/rpg-dnd5e-web-ruleset-6782045-before.json" \
  "$cache/rpg-dnd5e-web-ruleset-6782045-pre-update.json"
```

Expected: the baseline still matches and no dev ruleset exists.

- [ ] **Step 2: Generate target-specific payloads**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
source="$cache/rpg-dnd5e-web-ruleset-6782045-pre-update.json"

jq '{
  name:"Protect main (production)",
  target:"branch",
  enforcement:"active",
  bypass_actors:.bypass_actors,
  conditions:{ref_name:{exclude:[],include:["refs/heads/main"]}},
  rules:(.rules | map(
    if .type=="pull_request"
    then (.parameters.allowed_merge_methods=["merge"])
    else . end
  ))
}' "$source" > "$cache/rpg-dnd5e-web-main-ruleset-payload.json"

jq '{
  name:"Protect dev (integration)",
  target:"branch",
  enforcement:"active",
  bypass_actors:.bypass_actors,
  conditions:{ref_name:{exclude:[],include:["refs/heads/dev"]}},
  rules:(.rules | map(
    if .type=="pull_request" then
      (.parameters.allowed_merge_methods=["squash"])
    elif .type=="required_status_checks" then
      (.parameters.required_status_checks |= map(select(.context!="build")))
    else . end
  ))
}' "$source" > "$cache/rpg-dnd5e-web-dev-ruleset-payload.json"

jq -e '([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["Lint and Type Check","Security Audit","Test","build"]) and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]]) and .bypass_actors==[]' \
  "$cache/rpg-dnd5e-web-main-ruleset-payload.json" >/dev/null
jq -e '([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["Lint and Type Check","Security Audit","Test"]) and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]]) and .bypass_actors==[]' \
  "$cache/rpg-dnd5e-web-dev-ruleset-payload.json" >/dev/null
```

Expected: only the dev payload removes Docker `build` because that workflow does not run for `dev`.

- [ ] **Step 3: Update explicit production protection**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api --method PUT repos/KirkDiggler/rpg-dnd5e-web/rulesets/6782045 \
  --input "$cache/rpg-dnd5e-web-main-ruleset-payload.json" \
  > "$cache/rpg-dnd5e-web-main-ruleset-update-response.json"
gh api repos/KirkDiggler/rpg-dnd5e-web/rulesets/6782045 \
  > "$cache/rpg-dnd5e-web-main-ruleset-readback.json"
jq -e '.name=="Protect main (production)" and
  .conditions.ref_name.include==["refs/heads/main"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]]) and
  .bypass_actors==[]' "$cache/rpg-dnd5e-web-main-ruleset-readback.json" >/dev/null
```

Expected: explicit main protection remains active before any default change.

- [ ] **Step 4: Create and verify dev protection**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
gh api --method POST repos/KirkDiggler/rpg-dnd5e-web/rulesets \
  --input "$cache/rpg-dnd5e-web-dev-ruleset-payload.json" \
  > "$cache/rpg-dnd5e-web-dev-ruleset-create-response.json"
web_dev_ruleset="$(jq -r .id "$cache/rpg-dnd5e-web-dev-ruleset-create-response.json")"
test "$web_dev_ruleset" != null
echo "$web_dev_ruleset" > "$cache/rpg-dnd5e-web-dev-ruleset-id.txt"
gh api "repos/KirkDiggler/rpg-dnd5e-web/rulesets/$web_dev_ruleset" \
  > "$cache/rpg-dnd5e-web-dev-ruleset-readback.json"
jq -e '.name=="Protect dev (integration)" and
  .conditions.ref_name.include==["refs/heads/dev"] and
  ([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["squash"]]) and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.strict_required_status_checks_policy] == [true]) and
  ([.rules[]|select(.type=="required_status_checks")|.parameters.required_status_checks[].context] | sort == ["Lint and Type Check","Security Audit","Test"]) and
  .bypass_actors==[]' "$cache/rpg-dnd5e-web-dev-ruleset-readback.json" >/dev/null
gh api repos/KirkDiggler/rpg-dnd5e-web/branches/main | jq -e '.protected==true' >/dev/null
gh api repos/KirkDiggler/rpg-dnd5e-web/branches/dev | jq -e '.protected==true' >/dev/null
```

Expected: both branches report protected and the dev rule does not wait for a nonexistent Docker check.

- [ ] **Step 5: Record the read-back on web#844**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
web_dev_ruleset="$(cat "$cache/rpg-dnd5e-web-dev-ruleset-id.txt")"
cat > /tmp/rpg-web-844-rules-comment.md <<EOF
Ruleset installation read back successfully before the safety PR merge:

- production: ruleset 6782045, explicit \`refs/heads/main\`, merge-only
- integration: ruleset $web_dev_ruleset, explicit \`refs/heads/dev\`, squash-only
- dev checks: \`Test\`, \`Lint and Type Check\`, \`Security Audit\`
- main adds Docker \`build\`; both are strict and have no bypass actors
- both \`main\` and \`dev\` report protected

The repository default remains \`main\`; the Docker safety PR is not merged yet.

— platform agent, on behalf of KirkDiggler
EOF
gh issue comment 844 --repo KirkDiggler/rpg-dnd5e-web \
  --body-file /tmp/rpg-web-844-rules-comment.md
```

Expected: the issue records the exact dev ruleset ID and read-back.

---

### Task 7: Have Kirk squash-merge both safety PRs into protected `dev`

**Live resources:** the PRs created in Tasks 2 and 3.

**Files:**
- Modify through reviewed PRs: each repository's `.github/workflows/docker.yml`

**Interfaces:**
- Consumes: passing safety PRs and active explicit dev protection.
- Produces: each `dev` contains its main-only `latest` gate; short-lived heads are deleted; long-lived `dev` survives.

- [ ] **Step 1: Re-read both PRs immediately before the human merge gate**

```bash
api_pr="$(gh pr list --repo KirkDiggler/rpg-api --head ci/856-main-only-latest --json number --jq '.[0].number')"
web_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --head ci/844-main-only-latest --json number --jq '.[0].number')"
for spec in "KirkDiggler/rpg-api:$api_pr" "KirkDiggler/rpg-dnd5e-web:$web_pr"; do
  repo="${spec%:*}"; pr="${spec#*:}"
  gh pr view "$pr" --repo "$repo" \
    --json state,mergeable,mergeStateStatus,baseRefName,headRefName,headRefOid,files,url
  gh pr checks "$pr" --repo "$repo"
done
```

Expected: both are open, mergeable, target `dev`, contain one workflow file, and all expected checks pass.

- [ ] **Step 2: Human merge checkpoint**

Present both exact PR URLs and stop. Kirk squash-merges each PR into `dev`; do not merge on his behalf.

Expected: explicit approval and human merges, one PR at a time.

- [ ] **Step 3: Verify merge style, branch survival, and workflow content**

```bash
set -euo pipefail
api_pr="$(gh pr list --repo KirkDiggler/rpg-api --state merged --head ci/856-main-only-latest --json number --jq '.[0].number')"
web_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --state merged --head ci/844-main-only-latest --json number --jq '.[0].number')"
for spec in "rpg-api:$api_pr" "rpg-dnd5e-web:$web_pr"; do
  repo="${spec%:*}"; pr="${spec#*:}"
  gh pr view "$pr" --repo "KirkDiggler/$repo" \
    --json state,mergedAt,mergeCommit,baseRefName,headRefName \
    | jq -e '.state=="MERGED" and .baseRefName=="dev"'
  gh api "repos/KirkDiggler/$repo/branches/dev" | jq -e '.protected==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/contents/.github/workflows/docker.yml?ref=dev" \
    --jq '.content' | base64 -d > "/tmp/$repo-docker-dev-after.yml"
  rg -n -F "type=raw,value=latest,enable=\${{ github.ref == 'refs/heads/main' }}" \
    "/tmp/$repo-docker-dev-after.yml"
  ! rg -n 'is_default_branch' "/tmp/$repo-docker-dev-after.yml"
done
```

Expected: `dev` still exists and is protected in both repositories; each workflow has the explicit gate.

- [ ] **Step 4: Re-prove the upcoming ancestry update is content-neutral**

```bash
set -euo pipefail
for repo in rpg-api rpg-dnd5e-web; do
  root="/home/kirk/game-dev/$repo"
  git -C "$root" fetch origin
  out="$HOME/.cache/pi/dev-default-branches-312/$repo-merge-tree-after-safety.txt"
  git -C "$root" merge-tree --write-tree origin/dev origin/main > "$out"
  test "$(head -n 1 "$out")" = "$(git -C "$root" rev-parse 'origin/dev^{tree}')"
done
```

Expected: merging current production history into safety-updated `dev` changes ancestry only, not the `dev` tree.

---

### Task 8: Promote and verify rpg-api through a real merge commit

**Issue/PR:** rpg-api#855; promotion PR head `dev`, base `main`.

**Files:**
- No new source edit; the PR promotes the reviewed workflow change.

**Interfaces:**
- Consumes: protected safety-updated `dev`, explicit merge-only `main`, and a content-neutral merge-tree proof.
- Produces: a main merge commit that contains the `dev` parent, a `latest` API image labeled with that main SHA, and a successful downstream deployment.

- [ ] **Step 1: Create the promotion PR without changing either branch**

```bash
set -euo pipefail
git -C /home/kirk/game-dev/rpg-api fetch origin
cat > /tmp/rpg-api-855-pr.md <<'EOF'
## Summary

- promotes the reviewed main-only `latest` tag gate from `dev` to production `main`
- restores the supported merge-commit ancestry between the two long-lived branches
- keeps production deployment tied to the resulting `main` commit

## Merge contract

This PR must be updated from `main`, pass `test` and `build`, and be merged with a real merge commit. It must never be squashed or rebased.

Parent: KirkDiggler/rpg-project#312

Closes #855

— platform agent, on behalf of KirkDiggler
EOF
gh pr create --repo KirkDiggler/rpg-api --base main --head dev \
  --title 'release: promote dev-default branch safety' \
  --body-file /tmp/rpg-api-855-pr.md
api_release_pr="$(gh pr list --repo KirkDiggler/rpg-api --head dev --base main \
  --json number --jq '.[0].number')"
gh pr view "$api_release_pr" --repo KirkDiggler/rpg-api \
  --json state,baseRefName,headRefName,headRefOid,mergeable,mergeStateStatus,url
```

Expected: one open PR with base `main` and head `dev`.

- [ ] **Step 2: Update `dev` from `main` through GitHub's promotion-PR update endpoint**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
api_release_pr="$(gh pr list --repo KirkDiggler/rpg-api --head dev --base main \
  --json number --jq '.[0].number')"
head_before="$(gh pr view "$api_release_pr" --repo KirkDiggler/rpg-api --json headRefOid --jq .headRefOid)"
git -C /home/kirk/game-dev/rpg-api fetch origin
tree_before="$(git -C /home/kirk/game-dev/rpg-api rev-parse 'origin/dev^{tree}')"
git -C /home/kirk/game-dev/rpg-api merge-tree --write-tree origin/dev origin/main \
  > "$cache/rpg-api-release-update-merge-tree.txt"
test "$(head -n 1 "$cache/rpg-api-release-update-merge-tree.txt")" = "$tree_before"

gh api --method PUT "repos/KirkDiggler/rpg-api/pulls/$api_release_pr/update-branch" \
  -f expected_head_sha="$head_before" \
  > "$cache/rpg-api-release-update-response.json"
jq -e '.message|test("updat";"i")' "$cache/rpg-api-release-update-response.json" >/dev/null
```

Expected: GitHub accepts the update. If protected `dev` rejects this operation, do not loosen rules or push directly; stop and amend the design with the observed API response.

- [ ] **Step 3: Verify the update changed ancestry and no content**

```bash
set -euo pipefail
git -C /home/kirk/game-dev/rpg-api fetch origin
head_after="$(git -C /home/kirk/game-dev/rpg-api rev-parse origin/dev)"
test "$head_after" != "$head_before"
test "$(git -C /home/kirk/game-dev/rpg-api rev-parse 'origin/dev^{tree}')" = "$tree_before"
git -C /home/kirk/game-dev/rpg-api merge-base --is-ancestor origin/main origin/dev
test "$(git -C /home/kirk/game-dev/rpg-api cat-file -p origin/dev | rg -c '^parent ')" = 2
```

Expected: `dev` now has a two-parent merge commit, contains `main`, and retains exactly its pre-update tree.

- [ ] **Step 4: Wait for strict required checks and verify merge-only policy**

```bash
api_release_pr="$(gh pr list --repo KirkDiggler/rpg-api --head dev --base main \
  --json number --jq '.[0].number')"
gh pr checks "$api_release_pr" --repo KirkDiggler/rpg-api --watch --interval 10
gh api repos/KirkDiggler/rpg-api/rulesets/6668107 \
  | jq -e '([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]])' >/dev/null
gh pr view "$api_release_pr" --repo KirkDiggler/rpg-api \
  --json mergeable,mergeStateStatus,headRefOid,url
```

Expected: `test` and `build` pass and the PR is mergeable under merge-only production policy.

- [ ] **Step 5: Human production merge checkpoint**

Present the PR URL, exact reviewed head SHA, passing checks, and merge-only read-back. Stop until Kirk explicitly merges it with **Create a merge commit**.

Expected: no agent-issued merge command.

- [ ] **Step 6: Verify the main merge commit records `dev` and Docker succeeds**

```bash
set -euo pipefail
api_release_pr="$(gh pr list --repo KirkDiggler/rpg-api --state merged --head dev --base main \
  --json number --jq '.[0].number')"
api_merge_sha="$(gh pr view "$api_release_pr" --repo KirkDiggler/rpg-api \
  --json state,mergeCommit --jq 'select(.state=="MERGED")|.mergeCommit.oid')"
test -n "$api_merge_sha"
git -C /home/kirk/game-dev/rpg-api fetch origin
test "$(git -C /home/kirk/game-dev/rpg-api cat-file -p "$api_merge_sha" | rg -c '^parent ')" = 2
git -C /home/kirk/game-dev/rpg-api merge-base --is-ancestor origin/dev "$api_merge_sha"
gh api repos/KirkDiggler/rpg-api/branches/dev | jq -e '.protected==true' >/dev/null

api_run="$(gh run list --repo KirkDiggler/rpg-api --workflow docker.yml \
  --commit "$api_merge_sha" --limit 1 --json databaseId --jq '.[0].databaseId')"
test -n "$api_run"
gh run watch "$api_run" --repo KirkDiggler/rpg-api --exit-status
```

Expected: production `main` is a two-parent merge commit, `dev` survives, and the main Docker workflow succeeds.

- [ ] **Step 7: Verify GHCR `latest` labels and downstream deployment**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
api_merge_sha="$(gh pr view "$(gh pr list --repo KirkDiggler/rpg-api --state merged --head dev --base main --json number --jq '.[0].number')" \
  --repo KirkDiggler/rpg-api --json mergeCommit --jq .mergeCommit.oid)"
docker_config="$(mktemp -d)"
trap 'rm -rf "$docker_config"' EXIT
gh auth token | DOCKER_CONFIG="$docker_config" docker login ghcr.io \
  --username KirkDiggler --password-stdin >/dev/null
DOCKER_CONFIG="$docker_config" docker pull ghcr.io/kirkdiggler/rpg-api:latest
revision="$(DOCKER_CONFIG="$docker_config" docker image inspect \
  ghcr.io/kirkdiggler/rpg-api:latest \
  --format '{{ index .Config.Labels "org.opencontainers.image.revision" }}')"
test "$revision" = "$api_merge_sha"

before_id="$(jq '[.[].databaseId]|max' "$cache/deployment-runs-before.json")"
for attempt in $(seq 1 30); do
  deployment_id="$(gh run list --repo KirkDiggler/rpg-deployment --workflow 175201148 \
    --event workflow_dispatch --limit 1 --json databaseId --jq '.[0].databaseId')"
  if [ "$deployment_id" -gt "$before_id" ]; then break; fi
  sleep 10
done
test "$deployment_id" -gt "$before_id"
gh run watch "$deployment_id" --repo KirkDiggler/rpg-deployment --exit-status
gh run view "$deployment_id" --repo KirkDiggler/rpg-deployment --log \
  > "$cache/rpg-api-deployment-$deployment_id.log"
rg -n 'rpg-api|source.*api' "$cache/rpg-api-deployment-$deployment_id.log"
```

Expected: the `latest` image label equals the exact main merge SHA and a new API-attributable deployment run succeeds. The temporary Docker config is deleted; global Docker credentials are untouched.

---

### Task 9: Promote and verify the web through a real merge commit

**Issue/PR:** rpg-dnd5e-web#843; promotion PR head `dev`, base `main`.

**Files:**
- No new source edit; the PR promotes the reviewed workflow change.

**Interfaces:**
- Consumes: successful API promotion/deployment and protected safety-updated web `dev`.
- Produces: a merge-commit web production release, matching `latest` label, and separately attributable deployment success.

- [ ] **Step 1: Refresh the deployment baseline after the API release**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
gh run list --repo KirkDiggler/rpg-deployment --workflow 175201148 \
  --event workflow_dispatch --limit 20 \
  --json databaseId,status,conclusion,createdAt,url \
  > "$cache/deployment-runs-before-web.json"
```

Expected: the successful API deployment is included; the maximum run ID becomes the web-release baseline.

- [ ] **Step 2: Create the web promotion PR**

```bash
set -euo pipefail
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
cat > /tmp/rpg-web-843-pr.md <<'EOF'
## Summary

- promotes the reviewed main-only `latest` tag gate from `dev` to production `main`
- restores the supported merge-commit ancestry between the two long-lived branches
- keeps production deployment tied to the resulting `main` commit

## Merge contract

This PR must be updated from `main`, pass web CI plus Docker `build`, and be merged with a real merge commit. It must never be squashed or rebased.

Parent: KirkDiggler/rpg-project#312

Closes #843

— platform agent, on behalf of KirkDiggler
EOF
gh pr create --repo KirkDiggler/rpg-dnd5e-web --base main --head dev \
  --title 'release: promote dev-default branch safety' \
  --body-file /tmp/rpg-web-843-pr.md
web_release_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --head dev --base main \
  --json number --jq '.[0].number')"
gh pr view "$web_release_pr" --repo KirkDiggler/rpg-dnd5e-web \
  --json state,baseRefName,headRefName,headRefOid,mergeable,mergeStateStatus,url
```

Expected: one open PR with base `main` and head `dev`.

- [ ] **Step 3: Update `dev` from `main` through the promotion-PR endpoint**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
web_release_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --head dev --base main \
  --json number --jq '.[0].number')"
head_before="$(gh pr view "$web_release_pr" --repo KirkDiggler/rpg-dnd5e-web --json headRefOid --jq .headRefOid)"
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
tree_before="$(git -C /home/kirk/game-dev/rpg-dnd5e-web rev-parse 'origin/dev^{tree}')"
git -C /home/kirk/game-dev/rpg-dnd5e-web merge-tree --write-tree origin/dev origin/main \
  > "$cache/rpg-dnd5e-web-release-update-merge-tree.txt"
test "$(head -n 1 "$cache/rpg-dnd5e-web-release-update-merge-tree.txt")" = "$tree_before"

gh api --method PUT "repos/KirkDiggler/rpg-dnd5e-web/pulls/$web_release_pr/update-branch" \
  -f expected_head_sha="$head_before" \
  > "$cache/rpg-dnd5e-web-release-update-response.json"
jq -e '.message|test("updat";"i")' "$cache/rpg-dnd5e-web-release-update-response.json" >/dev/null
```

Expected: GitHub accepts the update without loosening dev protection.

- [ ] **Step 4: Verify ancestry-only update and all main checks**

```bash
set -euo pipefail
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
head_after="$(git -C /home/kirk/game-dev/rpg-dnd5e-web rev-parse origin/dev)"
test "$head_after" != "$head_before"
test "$(git -C /home/kirk/game-dev/rpg-dnd5e-web rev-parse 'origin/dev^{tree}')" = "$tree_before"
git -C /home/kirk/game-dev/rpg-dnd5e-web merge-base --is-ancestor origin/main origin/dev
test "$(git -C /home/kirk/game-dev/rpg-dnd5e-web cat-file -p origin/dev | rg -c '^parent ')" = 2

web_release_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --head dev --base main --json number --jq '.[0].number')"
gh pr checks "$web_release_pr" --repo KirkDiggler/rpg-dnd5e-web --watch --interval 10
gh api repos/KirkDiggler/rpg-dnd5e-web/rulesets/6782045 \
  | jq -e '([.rules[]|select(.type=="pull_request")|.parameters.allowed_merge_methods] == [["merge"]])' >/dev/null
```

Expected: `Test`, `Lint and Type Check`, `Security Audit`, and Docker `build` pass under merge-only production policy.

- [ ] **Step 5: Human production merge checkpoint**

Present the PR URL, exact head SHA, passing checks, and merge-only read-back. Stop until Kirk explicitly merges it with **Create a merge commit**.

- [ ] **Step 6: Verify the web main merge commit and Docker workflow**

```bash
set -euo pipefail
web_release_pr="$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --state merged --head dev --base main \
  --json number --jq '.[0].number')"
web_merge_sha="$(gh pr view "$web_release_pr" --repo KirkDiggler/rpg-dnd5e-web \
  --json state,mergeCommit --jq 'select(.state=="MERGED")|.mergeCommit.oid')"
test -n "$web_merge_sha"
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
test "$(git -C /home/kirk/game-dev/rpg-dnd5e-web cat-file -p "$web_merge_sha" | rg -c '^parent ')" = 2
git -C /home/kirk/game-dev/rpg-dnd5e-web merge-base --is-ancestor origin/dev "$web_merge_sha"
gh api repos/KirkDiggler/rpg-dnd5e-web/branches/dev | jq -e '.protected==true' >/dev/null

web_run="$(gh run list --repo KirkDiggler/rpg-dnd5e-web --workflow docker.yml \
  --commit "$web_merge_sha" --limit 1 --json databaseId --jq '.[0].databaseId')"
test -n "$web_run"
gh run watch "$web_run" --repo KirkDiggler/rpg-dnd5e-web --exit-status
```

Expected: the release is a two-parent merge commit, `dev` survives, and the Docker workflow succeeds.

- [ ] **Step 7: Verify web GHCR `latest` and the new downstream deployment**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/dev-default-branches-312"
web_merge_sha="$(gh pr view "$(gh pr list --repo KirkDiggler/rpg-dnd5e-web --state merged --head dev --base main --json number --jq '.[0].number')" \
  --repo KirkDiggler/rpg-dnd5e-web --json mergeCommit --jq .mergeCommit.oid)"
docker_config="$(mktemp -d)"
trap 'rm -rf "$docker_config"' EXIT
gh auth token | DOCKER_CONFIG="$docker_config" docker login ghcr.io \
  --username KirkDiggler --password-stdin >/dev/null
DOCKER_CONFIG="$docker_config" docker pull ghcr.io/kirkdiggler/rpg-dnd5e-web:latest
revision="$(DOCKER_CONFIG="$docker_config" docker image inspect \
  ghcr.io/kirkdiggler/rpg-dnd5e-web:latest \
  --format '{{ index .Config.Labels "org.opencontainers.image.revision" }}')"
test "$revision" = "$web_merge_sha"

before_id="$(jq '[.[].databaseId]|max' "$cache/deployment-runs-before-web.json")"
for attempt in $(seq 1 30); do
  deployment_id="$(gh run list --repo KirkDiggler/rpg-deployment --workflow 175201148 \
    --event workflow_dispatch --limit 1 --json databaseId --jq '.[0].databaseId')"
  if [ "$deployment_id" -gt "$before_id" ]; then break; fi
  sleep 10
done
test "$deployment_id" -gt "$before_id"
gh run watch "$deployment_id" --repo KirkDiggler/rpg-deployment --exit-status
gh run view "$deployment_id" --repo KirkDiggler/rpg-deployment --log \
  > "$cache/rpg-dnd5e-web-deployment-$deployment_id.log"
rg -n 'rpg-dnd5e-web|source.*web' "$cache/rpg-dnd5e-web-deployment-$deployment_id.log"
```

Expected: the web `latest` label equals the web main merge SHA and a distinct web-attributable deployment succeeds.

---

### Task 10: Change both repository defaults to protected `dev`

**Live resources:** repository settings for rpg-api and rpg-dnd5e-web.

**Files:**
- Modify tracked files: none
- Modify through GitHub API: `default_branch` and `allow_rebase_merge`

**Interfaces:**
- Consumes: both successful production promotions and explicit rulesets/workflows already live on `main`.
- Produces: default `dev`, rebase disabled, automatic short-lived branch cleanup retained.

- [ ] **Step 1: Prove all prerequisites from production refs**

```bash
set -euo pipefail
for repo in rpg-api rpg-dnd5e-web; do
  gh api "repos/KirkDiggler/$repo" | jq -e '.default_branch=="main"' >/dev/null
  gh api "repos/KirkDiggler/$repo/branches/main" | jq -e '.protected==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/branches/dev" | jq -e '.protected==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/contents/.github/workflows/docker.yml?ref=main" \
    --jq '.content' | base64 -d > "/tmp/$repo-docker-main-final.yml"
  rg -n -F "type=raw,value=latest,enable=\${{ github.ref == 'refs/heads/main' }}" \
    "/tmp/$repo-docker-main-final.yml"
  ! rg -n 'is_default_branch' "/tmp/$repo-docker-main-final.yml"
done
```

Expected: both production branches contain the safe tag gate and both long-lived branches are protected.

- [ ] **Step 2: Change rpg-api and immediately read it back**

```bash
gh api --method PATCH repos/KirkDiggler/rpg-api \
  -F default_branch=dev -F allow_rebase_merge=false \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-api-repository-default-update.json"
gh api repos/KirkDiggler/rpg-api \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-api-repository-final.json"
jq -e '.default_branch=="dev" and .allow_rebase_merge==false and
  .allow_merge_commit==true and .allow_squash_merge==true and
  .delete_branch_on_merge==true' \
  "$HOME/.cache/pi/dev-default-branches-312/rpg-api-repository-final.json" >/dev/null
gh api repos/KirkDiggler/rpg-api/branches/main | jq -e '.protected==true' >/dev/null
gh api repos/KirkDiggler/rpg-api/branches/dev | jq -e '.protected==true' >/dev/null
```

Expected: API default is `dev`; all other required merge/cleanup behavior remains.

- [ ] **Step 3: Change the web and immediately read it back**

```bash
gh api --method PATCH repos/KirkDiggler/rpg-dnd5e-web \
  -F default_branch=dev -F allow_rebase_merge=false \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-dnd5e-web-repository-default-update.json"
gh api repos/KirkDiggler/rpg-dnd5e-web \
  > "$HOME/.cache/pi/dev-default-branches-312/rpg-dnd5e-web-repository-final.json"
jq -e '.default_branch=="dev" and .allow_rebase_merge==false and
  .allow_merge_commit==true and .allow_squash_merge==true and
  .delete_branch_on_merge==true' \
  "$HOME/.cache/pi/dev-default-branches-312/rpg-dnd5e-web-repository-final.json" >/dev/null
gh api repos/KirkDiggler/rpg-dnd5e-web/branches/main | jq -e '.protected==true' >/dev/null
gh api repos/KirkDiggler/rpg-dnd5e-web/branches/dev | jq -e '.protected==true' >/dev/null
```

Expected: web default is `dev` with the same safe repository-level settings.

- [ ] **Step 4: Verify explicit rules did not follow the default**

```bash
cache="$HOME/.cache/pi/dev-default-branches-312"
for repo in rpg-api rpg-dnd5e-web; do
  gh api "repos/KirkDiggler/$repo/rulesets" > "$cache/$repo-rulesets-final.json"
  test "$(jq '[.[]|select(.enforcement=="active")]|length' "$cache/$repo-rulesets-final.json")" = 2
  for id in $(jq -r '.[]|select(.enforcement=="active")|.id' "$cache/$repo-rulesets-final.json"); do
    gh api "repos/KirkDiggler/$repo/rulesets/$id"
  done > "$cache/$repo-active-rulesets-final.jsonl"
  test "$(jq -s '[.[].conditions.ref_name.include[0]]|sort|join(",")' \
    "$cache/$repo-active-rulesets-final.jsonl")" = 'refs/heads/dev,refs/heads/main'
done
```

Expected: exactly two active explicit rulesets remain per repository; neither uses `~DEFAULT_BRANCH`.

- [ ] **Step 5: Verify clone/remote default resolution**

```bash
for repo in rpg-api rpg-dnd5e-web; do
  test "$(git ls-remote --symref "git@github.com:KirkDiggler/$repo.git" HEAD \
    | awk '/^ref:/{sub("refs/heads/", "", $2); print $2}')" = dev
  gh api graphql -f owner=KirkDiggler -f name="$repo" \
    -f query='query($owner:String!,$name:String!){repository(owner:$owner,name:$name){defaultBranchRef{name}}}' \
    | jq -e '.data.repository.defaultBranchRef.name=="dev"' >/dev/null
done
```

Expected: both Git and GraphQL resolve the new repository default to `dev`; no probe branch or throwaway PR is created.

- [ ] **Step 6: Record final settings on the journey**

```bash
cat > /tmp/rpg-project-312-defaults-comment.md <<'EOF'
Both repository defaults now read back as `dev` after the production safety promotions:

- explicit protected `main`: merge-only, strict production checks
- explicit protected `dev`: squash-only, strict integration checks
- repository rebase merge: disabled
- automatic merged-head deletion: still enabled
- Docker `latest` and deployment: still explicit to `refs/heads/main`
- Git remote HEAD and GraphQL defaultBranchRef: `dev`

Production promotions and downstream deployment evidence are recorded on rpg-api#855 and rpg-dnd5e-web#843.

— platform agent, on behalf of KirkDiggler
EOF
gh issue comment 312 --repo KirkDiggler/rpg-project \
  --body-file /tmp/rpg-project-312-defaults-comment.md
```

Expected: the journey records the post-mutation read-back, not merely the intended settings.

---

### Task 11: Reconcile living instructions and close the tracking surface

**Issue/PR:** rpg-project#313 / PR #314.

**Files:**
- Modify: `CLAUDE.md` (and therefore symlinked `AGENTS.md`)
- Modify: `ideas/team-workflow/dev-default-branches/plan.md` only for implementation corrections/ledger entries
- Modify locally, outside the repository: `/home/kirk/personal/CLAUDE.md`

**Interfaces:**
- Consumes: final live GitHub settings and successful production evidence.
- Produces: durable policy matching reality, local workspace guidance that defers to the repo-aware base table, and a completed journey.

- [ ] **Step 1: Replace the stale live-ruleset claims in rpg-project**

In `/home/kirk/personal/rpg-project/CLAUDE.md`, preserve the existing base-branch table and replace the two paragraphs beginning `What actually prevents it — now live` and `Merge method is enforceable too` with this final contract:

```markdown
**What prevents it — live and explicit:** rpg-api and rpg-dnd5e-web each have two
active repository rulesets with zero bypass actors. `Protect main (production)`
targets `refs/heads/main`, requires strict target-specific checks, blocks deletion
and force pushes, and allows merge commits only. `Protect dev (integration)`
targets `refs/heads/dev`, requires its target-specific checks, blocks deletion and
force pushes, and allows squash merges only. The explicit refs are load-bearing:
both repositories default to `dev`, but production protection remains attached to
`main`.

Docker image publication is independent of the default branch. In both repositories,
`latest` is enabled only when `github.ref == 'refs/heads/main'`; deployment dispatch
has the same explicit main gate. A push to default `dev` therefore cannot move the
production image or deploy.

Before every `dev` → `main` promotion, update the promotion PR branch from current
`main` and verify that the update changes ancestry without changing the `dev` tree.
Strict required checks enforce that update. Kirk then uses a real merge commit. The
result records the reviewed `dev` head as a parent; the next promotion repeats the
main-into-dev update because the new production merge commit is then ahead of `dev`.

A production hotfix starts from `origin/main` and merge-commits into `main`. After
production verification, create a second issue and branch from `origin/dev`, replay
the production merge with `git cherry-pick -m 1 <main-merge-sha>`, and squash that
second PR into `dev`. This keeps one integration commit per PR and makes the fix
available to new feature branches without weakening `dev` protection. The next
promotion's content-neutral update records the shared ancestry.
```

Expected: the living doc no longer claims a default-relative ruleset or says the final default is `main`.

- [ ] **Step 2: Write the failing documentation assertion, then make it pass**

Run before the edit:

```bash
cd /home/kirk/personal/rpg-project
! rg -n -F 'both repositories default to `dev`' CLAUDE.md
```

Expected before edit: exit 0. After applying Step 1, run:

```bash
cd /home/kirk/personal/rpg-project
rg -n -F 'both repositories default to `dev`' CLAUDE.md
rg -n -F '`latest` is enabled only when `github.ref == '\''refs/heads/main'\''`' CLAUDE.md
rg -n -F 'Protect main (production)' CLAUDE.md
rg -n -F 'Protect dev (integration)' CLAUDE.md
rg -n -F 'git cherry-pick -m 1 <main-merge-sha>' CLAUDE.md
! rg -n 'ruleset targeting `main` \("Protect The President"|target `~DEFAULT_BRANCH`' CLAUDE.md
git diff --check
```

Expected after edit: all assertions pass.

- [ ] **Step 3: Reconcile the local workspace instruction without broad cleanup**

Back up the local file and replace only its `Always Start Fresh` block:

```bash
cp /home/kirk/personal/CLAUDE.md \
  "$HOME/.cache/pi/dev-default-branches-312/personal-CLAUDE-before.md"
python3 - <<'PY'
from pathlib import Path
path = Path('/home/kirk/personal/CLAUDE.md')
text = path.read_text()
old = '''### Always Start Fresh
```bash
gcm                    # git checkout main
gl                     # git pull
gcb feature-name       # git checkout -b feature-name
```

**NEVER reuse old branches.** ALWAYS start from fresh main.
'''
new = '''### Always Start Fresh From the Owning Repository's Base
Read `rpg-project/AGENTS.md` and use its repo-aware base-branch table. Fetch first
and branch from the remote ref, never from a stale local branch:

```bash
git fetch origin
git checkout -b feature-name origin/dev   # rpg-api and rpg-dnd5e-web
git checkout -b feature-name origin/main  # repositories whose policy names main
```

**NEVER reuse old branches.** The owning repository policy decides the base.
'''
assert text.count(old) == 1
path.write_text(text.replace(old, new))
PY
rg -n -F 'origin/dev   # rpg-api and rpg-dnd5e-web' /home/kirk/personal/CLAUDE.md
! rg -n -F 'ALWAYS start from fresh main' /home/kirk/personal/CLAUDE.md
```

Expected: only the stale generic branch block changes; unrelated personal instructions remain untouched.

- [ ] **Step 4: Add implementation corrections to the plan ledger only if evidence changed the design**

If Tasks 1–10 exactly matched the plan, leave this file unchanged. If GitHub returned a different durable mechanism or identifier, append a dated `## Implementation ledger` entry naming the observed command/result and amend `design.md` in the same commit when the accepted architecture changed. Do not record transient run IDs as policy.

Expected: durable documents describe the mechanism that actually ran.

- [ ] **Step 5: Verify and commit the final rpg-project documentation**

```bash
cd /home/kirk/personal/rpg-project
git status --short
# .claude/ is pre-existing local state and remains untracked.
git diff --check
! rg -n 'T[B]D|T[O]DO|FIXM[E]' ideas/team-workflow/dev-default-branches CLAUDE.md
git diff -- CLAUDE.md ideas/team-workflow/dev-default-branches/
git add CLAUDE.md ideas/team-workflow/dev-default-branches/plan.md
if ! git diff --cached --quiet -- ideas/team-workflow/dev-default-branches/design.md; then
  git add ideas/team-workflow/dev-default-branches/design.md
fi
git commit -m 'docs: plan and record dev-default branch rollout (#313)'
git push origin docs/313-dev-default-branch-workflow
```

Expected: the commit contains `plan.md`, final living policy, and only evidence-earned design amendments. The untracked `.claude/` directory is untouched.

- [ ] **Step 6: Read back PR #314 and mark it ready for Kirk's final review**

```bash
gh pr view 314 --repo KirkDiggler/rpg-project \
  --json state,isDraft,headRefName,baseRefName,files,mergeable,mergeStateStatus,url
gh pr ready 314 --repo KirkDiggler/rpg-project
gh pr view 314 --repo KirkDiggler/rpg-project \
  --json state,isDraft,headRefOid,files,url \
  | jq -e '.state=="OPEN" and .isDraft==false and
    ([.files[].path]|contains(["ideas/team-workflow/dev-default-branches/design.md","ideas/team-workflow/dev-default-branches/plan.md","CLAUDE.md"]))' >/dev/null
```

Expected: PR #314 remains open, is no longer draft, and carries design, plan, and final living policy.

- [ ] **Step 7: Run the final cross-repository read-back**

```bash
set -euo pipefail
for repo in rpg-api rpg-dnd5e-web; do
  gh api "repos/KirkDiggler/$repo" \
    | jq -e '.default_branch=="dev" and .allow_rebase_merge==false and
      .allow_merge_commit==true and .allow_squash_merge==true and
      .delete_branch_on_merge==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/branches/main" | jq -e '.protected==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/branches/dev" | jq -e '.protected==true' >/dev/null
  gh api "repos/KirkDiggler/$repo/contents/.github/workflows/docker.yml?ref=main" \
    --jq '.content' | base64 -d | \
    rg -F "type=raw,value=latest,enable=\${{ github.ref == 'refs/heads/main' }}"
done

gh pr view 853 --repo KirkDiggler/rpg-api --json state,baseRefName,url

gh project item-list 19 --owner KirkDiggler --limit 1000 --format json \
  | jq '[.items[] | select(
      .content.url? == "https://github.com/KirkDiggler/rpg-project/issues/312" or
      .content.url? == "https://github.com/KirkDiggler/rpg-project/issues/313" or
      .content.url? == "https://github.com/KirkDiggler/rpg-api/issues/855" or
      .content.url? == "https://github.com/KirkDiggler/rpg-api/issues/856" or
      .content.url? == "https://github.com/KirkDiggler/rpg-dnd5e-web/issues/843" or
      .content.url? == "https://github.com/KirkDiggler/rpg-dnd5e-web/issues/844"
    ) | {title,status,team,area,kind}]'
```

Expected: live settings and workflow content match the design; PR #853 targets `dev` if still open; implementation slices are complete or in final review.

- [ ] **Step 8: Human merge and journey close gate**

Present PR #314 and the final evidence links. Kirk alone merges it. After the merge:

```bash
gh pr view 314 --repo KirkDiggler/rpg-project \
  --json state,mergedAt,mergeCommit,url | jq -e '.state=="MERGED"' >/dev/null
gh issue view 313 --repo KirkDiggler/rpg-project --json state \
  | jq -e '.state=="CLOSED"' >/dev/null
gh issue close 312 --repo KirkDiggler/rpg-project \
  --comment 'All five slices are merged and live settings, image tags, deployment runs, defaults, rulesets, and living instructions were read back successfully. — platform agent, on behalf of KirkDiggler'
```

Expected: PR #314 and #313 close first; #312 closes only after all four repository child issues are also closed and all deployment evidence is terminal success.

## Rollback boundaries

- Before either safety PR merges, ruleset mutations can be restored from the baseline payloads without changing code. Stop and ask Kirk before rollback; do not delete evidence.
- After a safety PR merges to `dev`, repair it through a new issue/PR rather than rewriting `dev`.
- After a production promotion, a failed deployment is a deployment incident; do not change defaults or describe the release as shipped.
- If a default change itself causes unexpected repository behavior, PATCH that repository's `default_branch` back to `main`. Keep explicit rulesets and explicit Docker tagging; they are safe under either default.
- Never solve an ancestry/update failure by rebasing, force-pushing, temporarily adding a bypass actor, or allowing direct pushes to a long-lived branch.
