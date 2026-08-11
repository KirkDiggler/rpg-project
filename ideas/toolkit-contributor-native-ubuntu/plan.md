# Native Ubuntu toolkit contributor sandbox implementation plan

> **For execution workers:** execute one task at a time. A task starts only after
> its own rehydration block and every listed `jq -e` assertion passes. Do not
> inherit shell variables from a previous task.

**Goal:** Land one additive `game-dev` change that makes the existing six-command
contributor facade support native Ubuntu and Ubuntu WSL2, then independently
prove the original full loop on clean native Ubuntu and clean WSL2 without
changing API, web, toolkit, proto, deployment, compose, or seed behavior.

**Architecture:** `scripts/toolkit-contributor.sh` remains the only facade and
continues to expose exactly `bootstrap`, `start`, `refresh`, `seed`, `status`,
and `down`. A literal-file host gate classifies `ubuntu-native` or
`ubuntu-wsl2` before every command action. The two modes share the existing
checkout, override, Docker image, three-overlay compose, Envoy, seeding, and
owned-cleanup paths. The only implementation repository is `game-dev`, and the
only implementation files are the four already-approved files in Task 2.

## Fixed facts, scope, and branch contract

### Delivery facts to preserve

| Artifact           | Exact title / branch facts                                                                                        | Merge SHA                                  |
| ------------------ | ----------------------------------------------------------------------------------------------------------------- | ------------------------------------------ |
| rpg-project #209   | `docs: finalize toolkit contributor sandbox design`; base `main`, head `docs/208-toolkit-contributor-sandbox`     | `22aee544a42906c2f8c01a0e1eb4935c252dcda2` |
| game-dev #60       | `feat: add toolkit contributor sandbox facade`; base `main`, head `feat/59-toolkit-contributor-sandbox`           | `1df0212e5a04374ea83b9af1dd81d09a8a55831a` |
| rpg-api #792       | `feat: add toolkit contributor sandbox provider`; base `dev`, head `feat/791-dnd5e-sandbox`                       | `9099953f9bc86efbed9bf62209a96c54d9383d6b` |
| rpg-dnd5e-web #747 | `feat: add development-only toolkit contributor sandbox`; base `dev`, head `feat/746-toolkit-contributor-sandbox` | `cfa63138a1f06de65991c31b006f29fc2af1ad74` |

The original API and web provider commits are `dev`-branch facts, not assumed
ancestors of `main`. The unchanged deployment composition publishes `80`,
`3002`, `6380`, and `8080`; the web Vite server uses `3001`. The complete
acceptance port set is therefore exactly `80, 3001, 3002, 6380, 8080`.

### Exact bootstrap root mapping

The facade has one immutable absent-root mapping:

| Root             | `repo_branch` | `repo_url`                                      |
| ---------------- | ------------- | ----------------------------------------------- |
| `rpg-toolkit`    | `main`        | `git@github.com:KirkDiggler/rpg-toolkit.git`    |
| `rpg-api`        | `dev`         | `git@github.com:KirkDiggler/rpg-api.git`        |
| `rpg-dnd5e-web`  | `dev`         | `git@github.com:KirkDiggler/rpg-dnd5e-web.git`  |
| `rpg-deployment` | `main`        | `git@github.com:KirkDiggler/rpg-deployment.git` |

For every missing root, the implementation assigns the table value to
`repo_branch` and invokes this exact argument order:

```bash
repo_branch=dev
repo_url=git@github.com:KirkDiggler/rpg-api.git
repo_root="$ROOT/rpg-api"
git clone --branch "$repo_branch" "$repo_url" "$repo_root"
```

`--single-branch` is not used. The requirement is to choose the initial
checkout branch, not to change the existing normal clone ref-retention policy.
The API and web `dev` selections make #792 and #747 reachable in a clean clone
without forcing an unrelated release to `main`; toolkit and deployment retain
their approved `main` baselines.

For a root that already exists, `bootstrap` validates it is a Git checkout with
exactly the table's `origin` URL and then preserves all state: current branch or
detached state, staged changes, unstaged changes, and untracked files. It must
never run `fetch`, `switch`, `checkout`, `pull`, `reset`, `clean`, or `stash`
on an existing root. An invalid root or mismatched origin fails before a clone,
helper, build, or compose action. This is a facade constraint; a future
implementation worker may fetch the separate `game-dev` primary only to create
its own new worktree.

### Non-negotiable boundaries

- Change only `game-dev/scripts/toolkit-contributor.sh`,
  `game-dev/tests/toolkit-contributor-contract.sh`,
  `game-dev/docs/toolkit-contributor-sandbox.md`, and `game-dev/README.md` in
  Task 2. No file is added in `game-dev`.
- No API, web, toolkit, proto, deployment, compose, direct-storage, character,
  seed-RPC, port, daemon-configuration, installer, watcher, second script,
  seventh command, process killer, port manager, or alternate-port path is in
  scope.
- The host reader reads only `/etc/os-release` and
  `/proc/sys/kernel/osrelease`. No environment variable, flag, config, Docker
  context/vendor, or test path selects a host mode.
- `bootstrap`, `start`, `refresh`, and `down` require `command -v docker` and
  `docker info` after host classification and before their first mutation.
  `seed` gets no Docker/health probe; `status` gets no Docker probe and makes
  one helper-status call plus one Envoy health call after its host-mode line.
- The compose order remains `docker-compose.local-dev.yml`,
  `docker-compose.api.yml`, then `docker-compose.local-api-src.yml`. The
  existing override target, local Dockerfile, Envoy endpoint, fixed identities,
  seed order, normal GameView route, and owned `down` cleanup remain unchanged.
- The facade never runs `ss`, `kill`, `pkill`, `fuser`, `lsof`, a container
  sweep, or any port-remediation command. `ss -ltnp` is manual acceptance
  evidence only. An occupied, unobservable, or failed-to-bind required port is
  a stop condition.
- Every automation comment ends exactly
  `— asset-pipeline agent, on behalf of KirkDiggler`. The required independent
  tracking/specification/shell review packages are instead signed marker
  comments authored by the `KirkDiggler` account. Issue and PR bodies have no
  closing keyword except the implementation PR's one final closing reference to
  its own game-dev issue number.
- Preserve all branches, worktrees, clean clones, logs, and evidence after
  merge and verification. Do not use `git worktree remove`, `git branch -D`,
  `git clean`, or recursive deletion as cleanup.

## Host classifier and command invariants

### Pure classifier seam

The script exposes the internal, sourceable test seam below while keeping the
public usage at six commands:

```bash
classify_toolkit_contributor_host "$os_release_text" "$kernel_release_text"
```

It prints exactly `ubuntu-native` or `ubuntu-wsl2` on success and returns
nonzero without a mode token on refusal. It is pure: no file reads,
subprocesses, Docker calls, environment-mode selection, checkout inspection, or
mutation. The production reader has no path/mode arguments:

```bash
read_toolkit_contributor_host
# Sets TOOLKIT_CONTRIBUTOR_HOST_MODE only after literal reads and classification.
```

The data-only ID parser ignores blank/comment lines and must require exactly
one whole-line form. The implementation shape is intentionally strict:

```bash
if [[ $line =~ ^ID=(ubuntu|\"ubuntu\"|\'ubuntu\')$ ]]; then
  ((id_count += 1))
elif [[ $line == ID* ]]; then
  malformed_id=1
fi
```

After the scan, `id_count == 1`, `malformed_id == 0`, and a nonempty kernel are
required. The kernel is lowercased only for comparison: `wsl2` or
`microsoft-standard` means `ubuntu-wsl2`; otherwise `microsoft` means refused
WSL1; otherwise Ubuntu is `ubuntu-native`. No kernel-version, desktop, init,
Docker, or environment allowlist exists.

### Required TDD matrix

Task 2 adds direct pure-seam assertions and literal-reader fixture assertions
for every row before production code changes:

| Case                     | `/etc/os-release` fixture                           | Kernel fixture                       | Result                           |
| ------------------------ | --------------------------------------------------- | ------------------------------------ | -------------------------------- |
| Native bare ID           | `ID=ubuntu`                                         | `6.8.0-31-generic`                   | `ubuntu-native`                  |
| Native double quoted ID  | `ID="ubuntu"`                                       | `6.8.0-31-generic`                   | `ubuntu-native`                  |
| Native single quoted ID  | `ID='ubuntu'`                                       | `6.8.0-31-generic`                   | `ubuntu-native`                  |
| Normal mixed-case WSL2   | `ID=ubuntu`                                         | `5.15.153.1-MiCrOsOfT-StAnDaRd-wSl2` | `ubuntu-wsl2`                    |
| Mixed-case explicit WSL2 | `ID="ubuntu"`                                       | `6.1.21.2-mIcRoSoFt-WSL2`            | `ubuntu-wsl2`                    |
| Legacy WSL2              | `ID='ubuntu'`                                       | `4.19.128-microsoft-standard`        | `ubuntu-wsl2`                    |
| WSL1                     | `ID=ubuntu`                                         | `4.4.0-19041-Microsoft`              | nonzero WSL1 refusal             |
| Debian                   | `ID=debian`                                         | `6.8.0-generic`                      | nonzero Ubuntu-only refusal      |
| Missing ID               | `NAME="Ubuntu"`                                     | `6.8.0-generic`                      | nonzero                          |
| Empty ID                 | `ID=`                                               | `6.8.0-generic`                      | nonzero                          |
| Duplicate ID             | `ID=ubuntu` followed by `ID=ubuntu`                 | `6.8.0-generic`                      | nonzero                          |
| Whitespace form          | `ID =ubuntu`                                        | `6.8.0-generic`                      | nonzero                          |
| Unmatched quote          | `ID="ubuntu`                                        | `6.8.0-generic`                      | nonzero                          |
| Trailing text            | `ID=ubuntu trailing`                                | `6.8.0-generic`                      | nonzero                          |
| Empty kernel             | `ID=ubuntu`                                         | empty                                | nonzero                          |
| Unreadable OS/kernel     | fake `cat` fails only for its selected literal path | otherwise valid                      | nonzero naming that literal path |

The no-bypass group sets all of
`TOOLKIT_CONTRIBUTOR_OSRELEASE`, `TOOLKIT_CONTRIBUTOR_OS_RELEASE`,
`OS_RELEASE_PATH`, and `HOST_MODE` to hostile values which claim WSL2 while the
literal fixture is Debian or WSL1. Both must still refuse. A native literal
fixture must still pass while all four variables contain hostile values.

### Gate ordering and Docker diagnostics

After valid argument-count validation, each command calls the literal reader
before `git`, Docker, API-helper, Go, compose, checkout, build, clone, health,
or seed activity. `status` immediately prints the successful exact line
`host mode: ubuntu-native` or `host mode: ubuntu-wsl2` before helper/Envoy work.

| Command     | Required order after argument validation                                                                           |
| ----------- | ------------------------------------------------------------------------------------------------------------------ |
| `bootstrap` | host → Docker CLI → `docker info` → remaining tools → SSH → root checks/clones                                     |
| `start`     | host → Docker CLI → `docker info` → layout → helper `on` → build → compose → bounded health                        |
| `refresh`   | host → Docker CLI → `docker info` → layout → helper `refresh` → build → `up -d --no-deps rpg-api` → bounded health |
| `down`      | host → Docker CLI → `docker info` → layout → compose down → helper off                                             |
| `seed`      | host → layout → exactly one non-health seeder invocation                                                           |
| `status`    | host → exact host-mode line → layout → one helper status → one Envoy health invocation                             |

Native Docker diagnostic text requires a Docker CLI and a reachable
Docker-compatible daemon and may name Docker Engine/rootless/another compatible
daemon, but never Docker Desktop. WSL2 text may name Docker Desktop WSL
integration or another reachable compatible daemon and never changes Windows or
Docker Desktop. Docker failure prevents all later mutating calls.

## Implementation file map

| Repository | Path                                    | Task 2 responsibility                                                                                                                                 |
| ---------- | --------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| `game-dev` | `scripts/toolkit-contributor.sh`        | Pure classifier, literal reader, mapped absent-root clone, mode-aware Docker preflight, status host-mode line.                                        |
| `game-dev` | `tests/toolkit-contributor-contract.sh` | Classifier, literal-reader, no-bypass, all-command ordering, clone mapping, existing-root preservation, Docker, status, and documentation assertions. |
| `game-dev` | `docs/toolkit-contributor-sandbox.md`   | One native/WSL2 runbook, literal observations, branch mapping, daemon distinctions, manual five-port observation, and no-auto-kill rule.              |
| `game-dev` | `README.md`                             | Concise pointer naming native Ubuntu and Ubuntu WSL2.                                                                                                 |

## Durable SDD ledger and JSON assertion conventions

Task 1 creates this durable, ignored control directory in the standard native
worktree and uses it for issue bodies, JSON read-backs, test reports, and the
ledger. It is never dispatch state in a transient directory, and no task relies
on an inherited shell or transient temporary directory:

```bash
project_worktree="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox"
sdd_root="$project_worktree/.superpowers/sdd/plan"
ledger="$sdd_root/ledger.json"
sdd_gitignore="$project_worktree/.superpowers/sdd/.gitignore"
mkdir -p "$project_worktree/.superpowers/sdd"
printf '%s\n' '*' > "$sdd_gitignore"
mkdir -p "$sdd_root"
test "$(cat "$sdd_gitignore")" = '*'
git -C "$project_worktree" check-ignore -q .superpowers/sdd/plan/ledger.json
```

The ledger records exactly `tracking_pr`, `tracking_merge_sha`, design/plan
blob IDs, `game_dev_issue`, `game_dev_url`, `game_dev_project_item`,
`game_dev_branch`, `game_dev_worktree`, `native_verify_issue`,
`native_verify_url`, `native_verify_project_item`, `native_game_dev_pr`, and
`native_game_dev_merge_sha`. A write is atomic through the fixed ignored path
`$ledger.next`, then `mv "$ledger.next" "$ledger"`; the ledger is audit
material only. Every task re-queries GitHub instead of trusting a shell variable
or the ledger value.

Project 19 is `KirkDiggler`'s The Dungeon Run board. These field and option IDs
are fixed for this delivery:

```bash
project_id='PVT_kwHOAASbwc4Bcj4v'
status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
feature_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s'
kind_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w'
team_field='PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs'
status_todo='397864df'
status_in_progress='a434eab1'
status_done='e4d8ce42'
feature_infra='c8ad5032'
kind_build='ea162471'
kind_verify='ab287333'
team_platform='f9c87bc7'
```

Use this exact issue-scoped GraphQL read-back whenever a Project field is gated.
It never scans the board: each issue is read separately into its own JSON file,
and the Project 19 row is selected from that issue's `projectItems` result.

```bash
issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
  -F number="$game_dev_issue" > "$sdd_root/project-game-dev.json"
```

The assertions below use `jq -e`; a false predicate is a hard stop. For
example, this checks the single Project 19 row and all four values:

```bash
jq -e '
  def has_field($field; $value):
    [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ($rows[0] | has_field("Status"; "Todo"))
    and ($rows[0] | has_field("Team"; "Platform"))
    and ($rows[0] | has_field("Feature"; "Infra"))
    and ($rows[0] | has_field("Kind"; "Build"))
' "$sdd_root/project-game-dev.json"
```

All comment read-backs use a task marker and this exact signature test:

```bash
signature='— asset-pipeline agent, on behalf of KirkDiggler'
gh issue view "$issue_number" --repo "$issue_repo" --json comments \
  | jq -e --arg marker "$comment_marker" --arg signature "$signature" '
      [.comments[] | select((.body | contains($marker)) and (.body | endswith($signature))]
      | length == 1
    '
```

## Tasks

### Task 1: Gate the merged tracking artifact, create exact delivery records, and prepare the isolated game-dev worktree

**Files:** none changed.
**Produces:** authoritative tracking-PR evidence, two exact delivery issue
records, Project 19 and parent-link proofs, an ignored ledger, and one clean
worktree on a deterministic branch/path.
**Stop point:** a failed tracking PR, signed review package, merge-content,
original-provider, issue, parent, or Project assertion stops before later
mutation.

- [ ] **1. Rehydrate from GitHub and prove the exact #211 tracking PR is merged.** Do not use a prior task shell. The tracking identity is fixed here:

  ```bash
  set -euo pipefail
  project_repo='KirkDiggler/rpg-project'
  tracking_pr_title='docs: native Ubuntu toolkit contributor sandbox design and plan'
  tracking_pr_head='design/native-ubuntu-toolkit-sandbox'
  tracking_pr_base='main'
  tracking_files='["ideas/toolkit-contributor-native-ubuntu/design.md","ideas/toolkit-contributor-native-ubuntu/plan.md"]'
  project_worktree="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox"
  sdd_root="$project_worktree/.superpowers/sdd/plan"
  ledger="$sdd_root/ledger.json"
  signature='— asset-pipeline agent, on behalf of KirkDiggler'
  project_id='PVT_kwHOAASbwc4Bcj4v'
  status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
  feature_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s'
  kind_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w'
  team_field='PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs'
  status_todo='397864df'
  status_in_progress='a434eab1'
  status_done='e4d8ce42'
  feature_infra='c8ad5032'
  kind_build='ea162471'
  kind_verify='ab287333'
  team_platform='f9c87bc7'
  issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
  sdd_gitignore="$project_worktree/.superpowers/sdd/.gitignore"
  mkdir -p "$project_worktree/.superpowers/sdd"
  printf '%s\n' '*' > "$sdd_gitignore"
  mkdir -p "$sdd_root"
  test "$(cat "$sdd_gitignore")" = '*'
  git -C "$project_worktree" check-ignore -q .superpowers/sdd/plan/ledger.json

  gh pr list --repo "$project_repo" --state merged \
    --search "in:title \"$tracking_pr_title\"" \
    --json number,title,url,headRefName,headRefOid,baseRefName,state,mergeCommit \
    > "$sdd_root/tracking-pr-list.json"
  jq -e --arg title "$tracking_pr_title" --arg head "$tracking_pr_head" --arg base "$tracking_pr_base" '
    length == 1
    and .[0].title == $title
    and .[0].headRefName == $head
    and (.[] | .headRefOid | type == "string" and length == 40)
    and .[0].baseRefName == $base
    and .[0].state == "MERGED"
    and (.[] | .mergeCommit.oid | type == "string" and length == 40)
  ' "$sdd_root/tracking-pr-list.json"
  tracking_pr="$(jq -r '.[0].number' "$sdd_root/tracking-pr-list.json")"
  tracking_merge_sha="$(jq -r '.[0].mergeCommit.oid' "$sdd_root/tracking-pr-list.json")"
  tracking_head_sha="$(jq -r '.[0].headRefOid' "$sdd_root/tracking-pr-list.json")"

  gh pr view "$tracking_pr" --repo "$project_repo" \
    --json number,title,state,baseRefName,headRefName,headRefOid,mergeCommit,files,statusCheckRollup,comments \
    > "$sdd_root/tracking-pr.json"
  jq -e --argjson files "$tracking_files" --arg title "$tracking_pr_title" \
    --arg head "$tracking_pr_head" --arg base "$tracking_pr_base" \
    --arg head_sha "$tracking_head_sha" --arg signature "$signature" '
    def good_check:
      (.conclusion // .state // "") as $result
      | $result == "SUCCESS" or $result == "NEUTRAL" or $result == "SKIPPED";
    def kirk_tracking_review:
      [.comments[] | select(
        .author.login == "KirkDiggler"
        and (.body | contains("native-ubuntu-tracking-review: PASS"))
        and (.body | contains("native-ubuntu-tracking-review-package: COMPLETE"))
        and (.body | contains("native-ubuntu-tracking-review-findings: none"))
        and (.body | contains("native-ubuntu-tracking-reviewed-head: " + $head_sha))
        and (.body | endswith($signature))
      )] | length == 1;
    .title == $title
    and .state == "MERGED"
    and .baseRefName == $base
    and .headRefName == $head
    and .headRefOid == $head_sha
    and ([.files[].path] | sort) == ($files | sort)
    and ([.statusCheckRollup[]? | select(good_check | not)] | length == 0)
    and kirk_tracking_review
    and (.mergeCommit.oid | type == "string" and length == 40)
  ' "$sdd_root/tracking-pr.json"

  project_root="$HOME/game-dev/rpg-project"
  git -C "$project_root" fetch origin
  git -C "$project_root" merge-base --is-ancestor "$tracking_merge_sha" origin/main
  git -C "$project_root" diff-tree --no-commit-id --name-only -r "$tracking_merge_sha" \
    | sort > "$sdd_root/tracking-merge-files.txt"
  git -C "$project_root" show "$tracking_merge_sha:ideas/toolkit-contributor-native-ubuntu/design.md" \
    > "$sdd_root/tracking-design.md"
  git -C "$project_root" show "$tracking_merge_sha:ideas/toolkit-contributor-native-ubuntu/plan.md" \
    > "$sdd_root/tracking-plan.md"
  test -s "$sdd_root/tracking-design.md"
  test -s "$sdd_root/tracking-plan.md"
  design_blob="$(git -C "$project_root" rev-parse "$tracking_merge_sha:ideas/toolkit-contributor-native-ubuntu/design.md")"
  plan_blob="$(git -C "$project_root" rev-parse "$tracking_merge_sha:ideas/toolkit-contributor-native-ubuntu/plan.md")"
  test "$design_blob" = "$(git -C "$project_root" rev-parse "origin/main:ideas/toolkit-contributor-native-ubuntu/design.md")"
  test "$plan_blob" = "$(git -C "$project_root" rev-parse "origin/main:ideas/toolkit-contributor-native-ubuntu/plan.md")"
  ```

  The PR identity, exact two-file list, immutable head SHA, hosted-check
  conclusions, Kirk-authored complete/no-findings review marker, merge SHA
  ancestry, and merged blob IDs are all required.
  This is intentionally stronger than searching a document heading.

- [ ] **2. Validate original merged dependencies with exact GitHub facts.** Query each PR instead of trusting remembered branch facts:

  ```bash
  gh pr view 209 --repo KirkDiggler/rpg-project --json title,state,baseRefName,headRefName,mergeCommit \
    | jq -e '
        .title == "docs: finalize toolkit contributor sandbox design"
        and .state == "MERGED" and .baseRefName == "main"
        and .headRefName == "docs/208-toolkit-contributor-sandbox"
        and .mergeCommit.oid == "22aee544a42906c2f8c01a0e1eb4935c252dcda2"
      '
  gh pr view 60 --repo KirkDiggler/game-dev --json title,state,baseRefName,headRefName,mergeCommit \
    | jq -e '
        .title == "feat: add toolkit contributor sandbox facade"
        and .state == "MERGED" and .baseRefName == "main"
        and .headRefName == "feat/59-toolkit-contributor-sandbox"
        and .mergeCommit.oid == "1df0212e5a04374ea83b9af1dd81d09a8a55831a"
      '
  gh pr view 792 --repo KirkDiggler/rpg-api --json title,state,baseRefName,headRefName,mergeCommit \
    | jq -e '
        .title == "feat: add toolkit contributor sandbox provider"
        and .state == "MERGED" and .baseRefName == "dev"
        and .headRefName == "feat/791-dnd5e-sandbox"
        and .mergeCommit.oid == "9099953f9bc86efbed9bf62209a96c54d9383d6b"
      '
  gh pr view 747 --repo KirkDiggler/rpg-dnd5e-web --json title,state,baseRefName,headRefName,mergeCommit \
    | jq -e '
        .title == "feat: add development-only toolkit contributor sandbox"
        and .state == "MERGED" and .baseRefName == "dev"
        and .headRefName == "feat/746-toolkit-contributor-sandbox"
        and .mergeCommit.oid == "cfa63138a1f06de65991c31b006f29fc2af1ad74"
      '
  ```

- [ ] **3. Create only the two deterministic delivery records through durable body files.** The future issue numbers are Task 1 outputs, never guessed values. First require no existing exact-title record; after creation, query the exact title again and require one result.

  ```bash
  game_dev_issue_title='Add native Ubuntu host support to toolkit contributor sandbox'
  native_verify_issue_title='Verify toolkit contributor sandbox on clean native Ubuntu'
  signature='— asset-pipeline agent, on behalf of KirkDiggler'

  gh issue list --repo KirkDiggler/game-dev --state all \
    --search "in:title \"$game_dev_issue_title\"" --json number,title,url \
    > "$sdd_root/game-dev-issues-before.json"
  gh issue list --repo KirkDiggler/rpg-project --state all \
    --search "in:title \"$native_verify_issue_title\"" --json number,title,url \
    > "$sdd_root/native-verify-issues-before.json"
  jq -e 'length == 0' "$sdd_root/game-dev-issues-before.json"
  jq -e 'length == 0' "$sdd_root/native-verify-issues-before.json"

  {
    printf '%s\n' '<!-- pih-dispatch:v1 -->' '' '## Goal / symptom' '' \
      'Add native Ubuntu host support to the one landed toolkit contributor facade while preserving the original sandbox loop.' \
      '' '## Contract boundaries' '' \
      '- Change only scripts/toolkit-contributor.sh, tests/toolkit-contributor-contract.sh, docs/toolkit-contributor-sandbox.md, and README.md.' \
      '- Missing roots clone toolkit/deployment from main and API/web from dev with git clone --branch; valid existing roots keep exact origin, branch, and dirty state with no fetch/switch/pull/reset/clean/stash.' \
      '- Keep literal host observation, no bypass, six commands, mode-aware Docker failures, exact compose order, API override ownership, RPC seeding, normal GameView, and owned down cleanup.' \
      '- Do not change API, web, toolkit, proto, deployment, compose, ports, Docker/Windows configuration, global tools, or credentials.' \
      '' '## Acceptance' '' \
      'TDD proves strict Ubuntu parsing, native/WSL2/WSL1 behavior, no bypass, all-command ordering, all four clone mappings, existing-root preservation, Docker fail-closed order, and preserved sandbox contracts. The PR has exactly one closing reference to this issue and exactly four changed paths.' \
      '' '## Related / dependencies' '' \
      'Umbrella: rpg-project#208' 'Native design decision: rpg-project#211' 'Original design: rpg-project#209' 'Original facade: game-dev#60' 'API provider: rpg-api#792 on dev' 'Web sandbox: rpg-dnd5e-web#747' '' \
      '— asset-pipeline agent, on behalf of KirkDiggler'
  } > "$sdd_root/game-dev-issue.md"

  {
    printf '%s\n' '<!-- pih-dispatch:v1 -->' '' '## Goal / symptom' '' \
      'Independently prove the landed facade and original contributor loop on clean native Ubuntu.' \
      '' '## Contract boundaries' '' \
      '- Evidence only: no implementation branch, PR, source edit, direct storage, host bypass, harness-only success, port workaround, or automatic process kill.' \
      '- Bootstrap must create new roots on main/dev/dev/main, preserve valid existing roots, and prove API #792/web #747 ancestry in the API/web dev clones.' \
      '- The only source mutation is the reversible Human Strength marker in the disposable toolkit clone. It stays armed until checkout, restored refresh, restored fighter strength=16/off_hand=shield record, and clean diff all pass.' \
      '- Browser actions use chrome_devtools_new_page/select_page/take_snapshot/click/wait_for/evaluate_script/take_screenshot: re-save each party order first, record PutDungeon, open displayed normal links in new tabs, verify encounter view, and capture each screenshot immediately.' \
      '' '## Acceptance' '' \
      'Record host/Docker/SSH/ports, focused gates, bootstrap rerun, seeds, marker proof, PutDungeon key evidence, six normal GameView screenshots, negatives, owned down result, checksums, attachments, and independent signed review before this issue closes.' \
      '' '## Related / dependencies' '' \
      'Umbrella: rpg-project#208' 'Native design decision: rpg-project#211' 'Original design: rpg-project#209' 'Original facade: game-dev#60' 'API provider: rpg-api#792 on dev' 'Web sandbox: rpg-dnd5e-web#747' '' \
      '— asset-pipeline agent, on behalf of KirkDiggler'
  } > "$sdd_root/native-verify-issue.md"

  game_dev_url="$(gh issue create --repo KirkDiggler/game-dev --title "$game_dev_issue_title" --body-file "$sdd_root/game-dev-issue.md")"
  native_verify_url="$(gh issue create --repo KirkDiggler/rpg-project --title "$native_verify_issue_title" --body-file "$sdd_root/native-verify-issue.md")"
  gh issue list --repo KirkDiggler/game-dev --state all \
    --search "in:title \"$game_dev_issue_title\"" --json number,title,url,id \
    > "$sdd_root/game-dev-issues-after.json"
  gh issue list --repo KirkDiggler/rpg-project --state all \
    --search "in:title \"$native_verify_issue_title\"" --json number,title,url,id \
    > "$sdd_root/native-verify-issues-after.json"
  jq -e --arg title "$game_dev_issue_title" --arg url "$game_dev_url" '
    length == 1 and .[0].title == $title and .[0].url == $url and (.[] | .number | type == "number")
  ' "$sdd_root/game-dev-issues-after.json"
  jq -e --arg title "$native_verify_issue_title" --arg url "$native_verify_url" '
    length == 1 and .[0].title == $title and .[0].url == $url and (.[] | .number | type == "number")
  ' "$sdd_root/native-verify-issues-after.json"
  game_dev_issue="$(jq -r '.[0].number' "$sdd_root/game-dev-issues-after.json")"
  native_verify_issue="$(jq -r '.[0].number' "$sdd_root/native-verify-issues-after.json")"
  game_dev_node_id="$(jq -r '.[0].id' "$sdd_root/game-dev-issues-after.json")"
  native_verify_node_id="$(jq -r '.[0].id' "$sdd_root/native-verify-issues-after.json")"
  ```

- [ ] **4. Make both records #208 sub-issues, set exact Project 19 values, and prove the reads.**

```bash
parent_node_id="$(gh issue view 208 --repo KirkDiggler/rpg-project --json id --jq .id)"
parent_url="$(gh issue view 208 --repo KirkDiggler/rpg-project --json url --jq .url)"
parent_number=208
add_child='mutation($parentId:ID!, $childId:ID!) { addSubIssue(input:{issueId:$parentId,subIssueId:$childId}) { issue { number url } subIssue { number url } } }'
gh api graphql -f query="$add_child" -F parentId="$parent_node_id" -F childId="$game_dev_node_id" \
  > "$sdd_root/add-game-dev-child.json"
gh api graphql -f query="$add_child" -F parentId="$parent_node_id" -F childId="$native_verify_node_id" \
  > "$sdd_root/add-native-verify-child.json"
jq -e --arg parent "$parent_url" --argjson parent_number "$parent_number" \
  --arg child "$game_dev_url" --argjson child_number "$game_dev_issue" '
  .data.addSubIssue.issue.url == $parent
  and .data.addSubIssue.issue.number == $parent_number
  and .data.addSubIssue.subIssue.url == $child
  and .data.addSubIssue.subIssue.number == $child_number
' "$sdd_root/add-game-dev-child.json"
jq -e --arg parent "$parent_url" --argjson parent_number "$parent_number" \
  --arg child "$native_verify_url" --argjson child_number "$native_verify_issue" '
  .data.addSubIssue.issue.url == $parent
  and .data.addSubIssue.issue.number == $parent_number
  and .data.addSubIssue.subIssue.url == $child
  and .data.addSubIssue.subIssue.number == $child_number
' "$sdd_root/add-native-verify-child.json"

game_dev_item="$(gh project item-add 19 --owner KirkDiggler --url "$game_dev_url" --format json --jq .id)"
native_verify_item="$(gh project item-add 19 --owner KirkDiggler --url "$native_verify_url" --format json --jq .id)"
for item in "$game_dev_item" "$native_verify_item"; do
  gh project item-edit --project-id "$project_id" --id "$item" --field-id "$status_field" --single-select-option-id "$status_todo"
  gh project item-edit --project-id "$project_id" --id "$item" --field-id "$feature_field" --single-select-option-id "$feature_infra"
  gh project item-edit --project-id "$project_id" --id "$item" --field-id "$team_field" --single-select-option-id "$team_platform"
done
gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$kind_field" --single-select-option-id "$kind_build"
gh project item-edit --project-id "$project_id" --id "$native_verify_item" --field-id "$kind_field" --single-select-option-id "$kind_verify"

parent_query='query($owner:String!, $repo:String!, $number:Int!) {
  repository(owner:$owner,name:$repo) {
    issue(number:$number) {
      number url parent { number repository { nameWithOwner } }
      subIssues(first:100) { nodes { number url repository { nameWithOwner } } }
    }
  }
}'
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=rpg-project -F number=208 \
  > "$sdd_root/parent-208.json"
jq -e --arg game "$game_dev_url" --arg native "$native_verify_url" '
  [.data.repository.issue.subIssues.nodes[] | .url] as $children
  | ($children | index($game) != null)
    and ($children | index($native) != null)
' "$sdd_root/parent-208.json"
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=game-dev -F number="$game_dev_issue" \
  > "$sdd_root/game-dev-parent.json"
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=rpg-project -F number="$native_verify_issue" \
  > "$sdd_root/native-verify-parent.json"
jq -e '.data.repository.issue.parent.number == 208 and .data.repository.issue.parent.repository.nameWithOwner == "KirkDiggler/rpg-project"' "$sdd_root/game-dev-parent.json"
jq -e '.data.repository.issue.parent.number == 208 and .data.repository.issue.parent.repository.nameWithOwner == "KirkDiggler/rpg-project"' "$sdd_root/native-verify-parent.json"

gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
  -F number="$game_dev_issue" > "$sdd_root/task1-game-dev-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project \
  -F number="$native_verify_issue" > "$sdd_root/task1-native-verify-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project \
  -F number=210 > "$sdd_root/task1-wsl-project.json"
jq -e --arg item "$game_dev_item" '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ($rows[0] | has_field("Status"; "Todo"))
    and ($rows[0] | has_field("Team"; "Platform")) and ($rows[0] | has_field("Feature"; "Infra"))
    and ($rows[0] | has_field("Kind"; "Build"))
' "$sdd_root/task1-game-dev-project.json"
jq -e --arg item "$native_verify_item" '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ($rows[0] | has_field("Status"; "Todo"))
    and ($rows[0] | has_field("Team"; "Platform")) and ($rows[0] | has_field("Feature"; "Infra"))
    and ($rows[0] | has_field("Kind"; "Verify"))
' "$sdd_root/task1-native-verify-project.json"
jq -e '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ($rows[0] | has_field("Status"; "In Progress"))
    and ($rows[0] | has_field("Team"; "Cross-team")) and ($rows[0] | has_field("Feature"; "Infra"))
    and ($rows[0] | has_field("Kind"; "Verify"))
' "$sdd_root/task1-wsl-project.json"
```

- [ ] **5. Record the ledger, complete #211 only as a design decision, and make the fresh worktree.** #211 is not closed by a PR keyword. Its Team/Feature/Kind are read before the status update and must remain unchanged.

  ```bash
  game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
  game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
  jq -n \
    --arg tracking_pr "$tracking_pr" --arg tracking_merge_sha "$tracking_merge_sha" --arg tracking_head_sha "$tracking_head_sha" \
    --arg design_blob "$design_blob" --arg plan_blob "$plan_blob" \
    --argjson game_dev_issue "$game_dev_issue" --arg game_dev_url "$game_dev_url" --arg game_dev_item "$game_dev_item" \
    --arg game_dev_branch "$game_dev_branch" --arg game_dev_worktree "$game_dev_worktree" \
    --argjson native_verify_issue "$native_verify_issue" --arg native_verify_url "$native_verify_url" --arg native_verify_item "$native_verify_item" \
    '{schema:1,tracking_pr:($tracking_pr|tonumber),tracking_merge_sha:$tracking_merge_sha,tracking_head_sha:$tracking_head_sha,design_blob:$design_blob,plan_blob:$plan_blob,game_dev_issue:$game_dev_issue,game_dev_url:$game_dev_url,game_dev_project_item:$game_dev_item,game_dev_branch:$game_dev_branch,game_dev_worktree:$game_dev_worktree,native_verify_issue:$native_verify_issue,native_verify_url:$native_verify_url,native_verify_project_item:$native_verify_item,native_game_dev_pr:null,native_game_dev_head_sha:null,native_game_dev_merge_sha:null}' \
    > "$ledger.next"
  mv "$ledger.next" "$ledger"
  jq -e '
    .schema == 1 and (.game_dev_issue|type == "number") and (.native_verify_issue|type == "number")
    and (.game_dev_branch|startswith("feat/")) and (.game_dev_worktree|startswith("/"))
    and (.tracking_merge_sha|length == 40) and (.tracking_head_sha|length == 40)
  ' "$ledger"

  handoff_marker='<!-- native-ubuntu-delivery:task1-handoff -->'
  printf '%s\n%s\n\n%s\n' "$handoff_marker" \
    "Tracking PR #$tracking_pr merged at $tracking_merge_sha with design blob $design_blob and plan blob $plan_blob. Delivery records are $game_dev_url and $native_verify_url; both are #208 sub-issues and Project 19 Platform / Infra records. #210 remains the sole WSL2 record and retains Cross-team / Infra / Verify ownership. #211 closes only as the completed design decision; #208 remains open through implementation plus both accepted live verifications." \
    "$signature" > "$sdd_root/task1-handoff.md"
  task211_item='PVTI_lAHOAASbwc4Bcj4vzg2H2C0'
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=211 \
    > "$sdd_root/task1-211-project-before.json"
  jq -e --arg item "$task211_item" '
    [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
    | ($rows | length == 1) and $rows[0].id == $item
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] | length == 1)
  ' "$sdd_root/task1-211-project-before.json"
  gh issue comment 211 --repo KirkDiggler/rpg-project --body-file "$sdd_root/task1-handoff.md"
  gh issue view 211 --repo KirkDiggler/rpg-project --json comments \
    | jq -e --arg marker "$handoff_marker" --arg signature "$signature" '[.comments[] | select((.body|contains($marker)) and (.body|endswith($signature))] | length == 1'
  gh project item-edit --project-id "$project_id" --id "$task211_item" --field-id "$status_field" --single-select-option-id "$status_done"
  gh issue close 211 --repo KirkDiggler/rpg-project
  gh issue view 211 --repo KirkDiggler/rpg-project --json state | jq -e '.state == "CLOSED"'
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=211 \
    > "$sdd_root/task1-211-project-after.json"
  jq -s -e --arg item "$task211_item" '
    def rows: [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)];
    def non_status: [.fieldValues.nodes[] | select(.field.name != "Status") | {field:.field.name,name:.name}] | sort_by(.field);
    (.[0] | rows) as $before
    | (.[1] | rows) as $after
    | ($before | length == 1) and ($after | length == 1)
      and $before[0].id == $item and $after[0].id == $item
      and ($before[0] | non_status) == ($after[0] | non_status)
      and ([ $after[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["Done"])
  ' "$sdd_root/task1-211-project-before.json" "$sdd_root/task1-211-project-after.json"

  test ! -e "$game_dev_worktree"
  git -C "$HOME/game-dev" fetch origin
  git -C "$HOME/game-dev" merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a origin/main
  git -C "$HOME/game-dev" worktree add -b "$game_dev_branch" "$game_dev_worktree" origin/main
  git -C "$game_dev_worktree" status --porcelain=v1 > "$sdd_root/game-dev-worktree-status.txt"
  test ! -s "$sdd_root/game-dev-worktree-status.txt"
  test "$(git -C "$game_dev_worktree" branch --show-current)" = "$game_dev_branch"
  git -C "$game_dev_worktree" merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a HEAD
  gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$status_field" --single-select-option-id "$status_in_progress"













  ```

### Task 2: TDD the host gate, exact clone mapping, documentation, and one game-dev PR

**Files:** modify only `scripts/toolkit-contributor.sh`,
`tests/toolkit-contributor-contract.sh`, `docs/toolkit-contributor-sandbox.md`,
and `README.md`.
**Produces:** the bounded four-file change, TDD evidence, one commit, one PR to
`main`, exact PR/review/check evidence, and one merge SHA.
**Stop point:** a regression in origin/dirty preservation, root mapping,
override, compose, health, seed, status, or down behavior blocks the PR.

- [ ] **1. Rehydrate this task from exact titles and GitHub facts.** The ledger is checked for audit but is not the source of truth.

```bash
set -euo pipefail
sdd_root="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox/.superpowers/sdd/plan"
ledger="$sdd_root/ledger.json"
game_dev_issue_title='Add native Ubuntu host support to toolkit contributor sandbox'
native_verify_issue_title='Verify toolkit contributor sandbox on clean native Ubuntu'
implementation_pr_title='feat: support native Ubuntu toolkit contributor sandbox'
signature='— asset-pipeline agent, on behalf of KirkDiggler'
project_id='PVT_kwHOAASbwc4Bcj4v'
status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
status_in_progress='a434eab1'
status_done='e4d8ce42'
issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
test -s "$ledger"
jq -e '.schema == 1' "$ledger"
gh issue list --repo KirkDiggler/game-dev --state all --search "in:title \"$game_dev_issue_title\"" --json number,title,url \
> "$sdd_root/task2-game-dev-issue.json"
gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$native_verify_issue_title\"" --json number,title,url \
> "$sdd_root/task2-native-verify-issue.json"
jq -e --arg title "$game_dev_issue_title" 'length == 1 and .[0].title == $title and (.[]|.number|type == "number")' "$sdd_root/task2-game-dev-issue.json"
jq -e --arg title "$native_verify_issue_title" 'length == 1 and .[0].title == $title and (.[]|.number|type == "number")' "$sdd_root/task2-native-verify-issue.json"
game_dev_issue="$(jq -r '.[0].number' "$sdd_root/task2-game-dev-issue.json")"
game_dev_url="$(jq -r '.[0].url' "$sdd_root/task2-game-dev-issue.json")"
native_verify_issue="$(jq -r '.[0].number' "$sdd_root/task2-native-verify-issue.json")"
game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
test -d "$game_dev_worktree"
test "$(git -C "$game_dev_worktree" branch --show-current)" = "$game_dev_branch"
gh issue view "$game_dev_issue" --repo KirkDiggler/game-dev --json state,title \
| jq -e --arg title "$game_dev_issue_title" '.state == "OPEN" and .title == $title'
gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state,title \
| jq -e --arg title "$native_verify_issue_title" '.state == "OPEN" and .title == $title'

```

- [ ] **2. Make RED tests first, including every root/branch assertion.** Extend the existing hermetic copied-script fixture; the fake `cat` returns selected contents only for the two literal paths and the fake command log remains the proof of ordering and absence of later calls. Add the classifier matrix above, no-bypass group, WSL1/Debian refusal for all six commands, native/WSL2 Docker-CLI and `docker info` failures, seed/status preservation, and documentation text assertions.

  Add a four-row clone group whose absent-root fake Git log asserts these exact invocations and no `--single-branch` or follow-up checkout:

  ```text
  git clone --branch main git@github.com:KirkDiggler/rpg-toolkit.git $ROOT/rpg-toolkit
  git clone --branch dev git@github.com:KirkDiggler/rpg-api.git $ROOT/rpg-api
  git clone --branch dev git@github.com:KirkDiggler/rpg-dnd5e-web.git $ROOT/rpg-dnd5e-web
  git clone --branch main git@github.com:KirkDiggler/rpg-deployment.git $ROOT/rpg-deployment
  ```

In the fixture, build each expected line from the exact contract:

```bash
assert_log_contains "git clone --branch $repo_branch $repo_url $repo_root"
assert_log_not_contains '--single-branch'
assert_log_not_contains 'git checkout'
assert_log_not_contains 'git switch'
```

For every existing root, create a valid exact-origin checkout on a distinct
branch with staged, unstaged, and untracked content. Capture
`git branch --show-current` and `git status --porcelain=v1` before and after
`bootstrap`; assert byte-for-byte equality and assert the log contains none
of `git fetch`, `git switch`, `git checkout`, `git pull`, `git reset`,
`git clean`, or `git stash`. Repeat a wrong-origin fixture for all four roots
and assert refusal before clone/helper/build/compose activity.

Documentation assertions must require the four branch mappings,
`git clone --branch`, both mode tokens, literal host paths, WSL1,
Docker-compatible daemon, exact `80, 3001, 3002, 6380, 8080`, and the
no-auto-kill language. Then run and retain the expected first failure:

```bash
cd "$game_dev_worktree"
set +e
bash tests/toolkit-contributor-contract.sh > "$sdd_root/task2-red.txt" 2>&1
red_status=$?
set -e
test "$red_status" -ne 0
grep -E 'classify_toolkit_contributor_host|clone --branch|ubuntu-native|literal' "$sdd_root/task2-red.txt"
```

- [ ] **3. Implement only the four-file contract.** In the script, encode the mapping as data, use `repo_branch` only for absent roots, and use the exact clone call shown above. Existing-root validation is read-only and exact-origin only. Do not add `--single-branch`, a fetch/switch/pull/reset/clean/stash path, or an auto-repair path.

  Add the pure classifier and literal reader from the invariant section. Put
  `read_toolkit_contributor_host` first in every valid command after argument
  validation. Add `require_toolkit_contributor_docker` only before mutating
  commands and remove Docker from a later generic tool loop so it is checked
  once at the required point. Put `host mode: $TOOLKIT_CONTRIBUTOR_HOST_MODE`
  directly after successful status classification. Guard final dispatch so the
  direct classifier test can source the script; usage still lists six commands.

  Update the runbook as one loop: show the two literal observations, strict ID
  parser, WSL2 precedence/WSL1 refusal, native versus WSL2 daemon remedies,
  the absent-root `main/dev/dev/main` mapping and untouched-existing-root rule,
  normal daily/marker/Vite/down flow, and manual acceptance-only five-port
  observation. Update the README only as a concise native-Ubuntu-and-WSL2
  runbook pointer.

- [ ] **4. Run GREEN checks and prove scope.**

  ```bash
  cd "$game_dev_worktree"
  bash -n scripts/toolkit-contributor.sh | tee "$sdd_root/task2-bash-n.txt"
  bash tests/toolkit-contributor-contract.sh | tee "$sdd_root/task2-green-contract.txt"
  bash tests/bootstrap-contract.sh | tee "$sdd_root/task2-bootstrap-contract.txt"
  git diff --check | tee "$sdd_root/task2-diff-check.txt"
  git diff --name-only | sort | tee "$sdd_root/task2-files.txt"
  test "$(cat "$sdd_root/task2-files.txt")" = $'README.md\ndocs/toolkit-contributor-sandbox.md\nscripts/toolkit-contributor.sh\ntests/toolkit-contributor-contract.sh'
  ```

  The focused contract must prove every classifier row, no bypass, all-command
  host ordering, native/WSL2 Docker refusal ordering, the four exact clone
  commands, all four existing-root preservation cases, and all pre-existing
  facade behavior.

- [ ] **5. Commit exactly four paths, open the bounded PR, and rehydrate the PR by exact title/head.**

  ```bash
  cd "$game_dev_worktree"
  git add scripts/toolkit-contributor.sh tests/toolkit-contributor-contract.sh \
    docs/toolkit-contributor-sandbox.md README.md
  git diff --cached --name-only | sort | tee "$sdd_root/task2-staged-files.txt"
  test "$(cat "$sdd_root/task2-staged-files.txt")" = $'README.md\ndocs/toolkit-contributor-sandbox.md\nscripts/toolkit-contributor.sh\ntests/toolkit-contributor-contract.sh'
  git diff --cached --check
  git commit -m 'feat: support native Ubuntu toolkit contributor sandbox'
  git status --porcelain=v1 > "$sdd_root/task2-post-commit-status.txt"
  test ! -s "$sdd_root/task2-post-commit-status.txt"
  git diff origin/main...HEAD --check
  native_game_dev_head_sha="$(git -C "$game_dev_worktree" rev-parse HEAD)"
  test "$(git -C "$game_dev_worktree" rev-parse HEAD)" = "$native_game_dev_head_sha"
  git -C "$game_dev_worktree" push --set-upstream origin "$game_dev_branch"
  remote_game_dev_head_sha="$(git -C "$game_dev_worktree" ls-remote origin "refs/heads/$game_dev_branch" | awk '{print $1}')"
  test "$remote_game_dev_head_sha" = "$native_game_dev_head_sha"

  {
    printf '%s\n' '## Summary' '' \
      '- Adds the literal native/WSL2 host gate and mode-specific Docker diagnostics while preserving six commands.' \
      '- Clones missing roots with main/dev/dev/main through exact git clone --branch calls and preserves valid existing roots without fetch/switch/pull/reset/clean/stash.' \
      '- Retains override, Dockerfile, compose, Envoy, seed, normal GameView, and owned cleanup behavior.' \
      '' '## Verification' '' \
      '- RED: bash tests/toolkit-contributor-contract.sh' \
      '- GREEN: bash -n scripts/toolkit-contributor.sh; bash tests/toolkit-contributor-contract.sh; bash tests/bootstrap-contract.sh; git diff --check.' \
      '- The fake-command log proves all host/Docker, clone, and preservation ordering.' \
      '' '## Dependencies' '' \
      'rpg-project#209; rpg-project#211; game-dev#60; rpg-api#792 on dev at 9099953f9bc86efbed9bf62209a96c54d9383d6b; rpg-dnd5e-web#747 on dev at cfa63138a1f06de65991c31b006f29fc2af1ad74.' ''
    printf 'Closes #%s\n' "$game_dev_issue"
  } > "$sdd_root/implementation-pr.md"
  gh pr create --repo KirkDiggler/game-dev --base main --head "$game_dev_branch" \
    --title "$implementation_pr_title" \
    --body-file "$sdd_root/implementation-pr.md" \
    > "$sdd_root/implementation-pr-url.txt"
  gh pr list --repo KirkDiggler/game-dev --state open --head "$game_dev_branch" \
    --search "in:title \"$implementation_pr_title\"" \
    --json number,title,url,headRefName,headRefOid,baseRefName,state \
    > "$sdd_root/task2-pr-list.json"
  jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" --arg head_sha "$native_game_dev_head_sha" '
    length == 1 and .[0].title == $title and .[0].headRefName == $head
    and .[0].headRefOid == $head_sha
    and .[0].baseRefName == "main" and .[0].state == "OPEN"
  ' "$sdd_root/task2-pr-list.json"
  native_game_dev_pr="$(jq -r '.[0].number' "$sdd_root/task2-pr-list.json")"













  ```

- [ ] **6. Validate the immutable PR before changing board status, then merge only that re-read head.** First validate the immutable head, exact four-file list, both Kirk-authored signed review comments, and every _present_ hosted check. An empty hosted-check list is not itself a failure; any present non-success/neutral/skipped result is. Only after those assertions pass may the item move to `In Review`. A fresh read of the same PR immediately precedes the merge. A conditional pass, changed head, failed/pending present check, non-Kirk author, or finding blocks merge.

```bash
test -s "$sdd_root/task2-green-contract.txt"
test -s "$sdd_root/task2-bootstrap-contract.txt"
gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev \
  --json number,title,state,baseRefName,headRefName,headRefOid,files,statusCheckRollup,comments \
  > "$sdd_root/task2-pr-before-in-review.json"
jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" \
  --arg head_sha "$native_game_dev_head_sha" --arg signature "$signature" '
def good_check:
  (.conclusion // .state // "") as $result
  | $result == "SUCCESS" or $result == "NEUTRAL" or $result == "SKIPPED";
def kirk_review($pass; $package; $findings; $head_marker):
  [.comments[] | select(
    .author.login == "KirkDiggler"
    and (.body | contains($pass))
    and (.body | contains($package))
    and (.body | contains($findings))
    and (.body | contains($head_marker + $head_sha))
    and (.body | endswith($signature))
  )] | length == 1;
.title == $title and .state == "OPEN" and .baseRefName == "main" and .headRefName == $head
and .headRefOid == $head_sha
and ([.files[].path] | sort == ["README.md","docs/toolkit-contributor-sandbox.md","scripts/toolkit-contributor.sh","tests/toolkit-contributor-contract.sh"])
and ([.statusCheckRollup[]? | select(good_check | not)] | length == 0)
and kirk_review("native-ubuntu-spec-review: PASS"; "native-ubuntu-spec-review-package: COMPLETE"; "native-ubuntu-spec-review-findings: none"; "native-ubuntu-spec-reviewed-head: ")
and kirk_review("native-ubuntu-shell-review: PASS"; "native-ubuntu-shell-review-package: COMPLETE"; "native-ubuntu-shell-review-findings: none"; "native-ubuntu-shell-reviewed-head: ")
' "$sdd_root/task2-pr-before-in-review.json"
closing_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){pullRequest(number:$number){closingIssuesReferences(first:20){nodes{number repository{nameWithOwner}}}}}}'
gh api graphql -f query="$closing_query" -F owner=KirkDiggler -F repo=game-dev -F number="$native_game_dev_pr" \
  > "$sdd_root/task2-closing-refs.json"
jq -e --argjson issue "$game_dev_issue" '
.data.repository.pullRequest.closingIssuesReferences.nodes as $refs
| ($refs | length == 1)
  and $refs[0].number == $issue
  and $refs[0].repository.nameWithOwner == "KirkDiggler/game-dev"
' "$sdd_root/task2-closing-refs.json"

gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
  -F number="$game_dev_issue" > "$sdd_root/task2-game-dev-project-before-review.json"
game_dev_item="$(jq -er '[.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] | if length == 1 then .[0].id else error("expected one Project 19 item") end' "$sdd_root/task2-game-dev-project-before-review.json")"
jq -e --arg item "$game_dev_item" '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["In Progress"])
' "$sdd_root/task2-game-dev-project-before-review.json"
status_options_query='query($id:ID!){node(id:$id){... on ProjectV2{field(name:"Status"){... on ProjectV2SingleSelectField{options{id name}}}}}}'
gh api graphql -f query="$status_options_query" -F id="$project_id" > "$sdd_root/task2-status-options.json"
jq -e '[.data.node.field.options[] | select(.name == "In Review")] | length == 1' "$sdd_root/task2-status-options.json"
status_in_review="$(jq -r '.data.node.field.options[] | select(.name == "In Review") | .id' "$sdd_root/task2-status-options.json")"
gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$status_field" --single-select-option-id "$status_in_review"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
  -F number="$game_dev_issue" > "$sdd_root/task2-game-dev-project-in-review.json"
jq -e --arg item "$game_dev_item" '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["In Review"])
' "$sdd_root/task2-game-dev-project-in-review.json"

# This is intentionally the last PR read before the head-locked merge.
gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev \
  --json number,title,state,baseRefName,headRefName,headRefOid,files,statusCheckRollup,comments \
  > "$sdd_root/task2-pr-immediately-before-merge.json"
jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" \
  --arg head_sha "$native_game_dev_head_sha" --arg signature "$signature" '
def good_check:
  (.conclusion // .state // "") as $result
  | $result == "SUCCESS" or $result == "NEUTRAL" or $result == "SKIPPED";
def kirk_review($pass; $package; $findings; $head_marker):
  [.comments[] | select(
    .author.login == "KirkDiggler"
    and (.body | contains($pass))
    and (.body | contains($package))
    and (.body | contains($findings))
    and (.body | contains($head_marker + $head_sha))
    and (.body | endswith($signature))
  )] | length == 1;
.title == $title and .state == "OPEN" and .baseRefName == "main" and .headRefName == $head
and .headRefOid == $head_sha
and ([.files[].path] | sort == ["README.md","docs/toolkit-contributor-sandbox.md","scripts/toolkit-contributor.sh","tests/toolkit-contributor-contract.sh"])
and ([.statusCheckRollup[]? | select(good_check | not)] | length == 0)
and kirk_review("native-ubuntu-spec-review: PASS"; "native-ubuntu-spec-review-package: COMPLETE"; "native-ubuntu-spec-review-findings: none"; "native-ubuntu-spec-reviewed-head: ")
and kirk_review("native-ubuntu-shell-review: PASS"; "native-ubuntu-shell-review-package: COMPLETE"; "native-ubuntu-shell-review-findings: none"; "native-ubuntu-shell-reviewed-head: ")
' "$sdd_root/task2-pr-immediately-before-merge.json"

set +e
gh pr merge "$native_game_dev_pr" --repo KirkDiggler/game-dev --squash --delete-branch=false \
  --match-head-commit "$native_game_dev_head_sha"
merge_status=$?
set -e
if [ "$merge_status" -ne 0 ]; then
  gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$status_field" --single-select-option-id "$status_in_progress"
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
    -F number="$game_dev_issue" > "$sdd_root/task2-game-dev-project-merge-failed.json"
  jq -e --arg item "$game_dev_item" '
    [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
    | ($rows | length == 1) and $rows[0].id == $item
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["In Progress"])
  ' "$sdd_root/task2-game-dev-project-merge-failed.json"
  exit "$merge_status"
fi

gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev --json state,mergeCommit,baseRefName,headRefName,headRefOid \
  > "$sdd_root/task2-pr-merged.json"
jq -e --arg head "$game_dev_branch" --arg head_sha "$native_game_dev_head_sha" '
.state == "MERGED" and .baseRefName == "main" and .headRefName == $head and .headRefOid == $head_sha
and (.mergeCommit.oid|type == "string" and length == 40)
' "$sdd_root/task2-pr-merged.json"
native_game_dev_merge_sha="$(jq -r '.mergeCommit.oid' "$sdd_root/task2-pr-merged.json")"
git -C "$HOME/game-dev" fetch origin
git -C "$HOME/game-dev" merge-base --is-ancestor "$native_game_dev_merge_sha" origin/main
issue_closure_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){state closedByPullRequestsReferences(first:20){nodes{number state mergedAt repository{nameWithOwner}}}}}}'
gh api graphql -f query="$issue_closure_query" -F owner=KirkDiggler -F repo=game-dev -F number="$game_dev_issue" \
  > "$sdd_root/task2-game-dev-closure.json"
jq -e --argjson pr "$native_game_dev_pr" '
  .data.repository.issue as $issue
  | $issue.state == "CLOSED"
    and ($issue.closedByPullRequestsReferences.nodes | length == 1)
    and $issue.closedByPullRequestsReferences.nodes[0].number == $pr
    and $issue.closedByPullRequestsReferences.nodes[0].state == "MERGED"
    and ($issue.closedByPullRequestsReferences.nodes[0].mergedAt | type == "string")
    and $issue.closedByPullRequestsReferences.nodes[0].repository.nameWithOwner == "KirkDiggler/game-dev"
' "$sdd_root/task2-game-dev-closure.json"
gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$status_field" --single-select-option-id "$status_done"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev \
  -F number="$game_dev_issue" > "$sdd_root/task2-game-dev-project-done.json"
jq -e --arg item "$game_dev_item" '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["Done"])
' "$sdd_root/task2-game-dev-project-done.json"
jq --argjson pr "$native_game_dev_pr" --arg head_sha "$native_game_dev_head_sha" --arg sha "$native_game_dev_merge_sha" '.native_game_dev_pr=$pr | .native_game_dev_head_sha=$head_sha | .native_game_dev_merge_sha=$sha' "$ledger" > "$ledger.next"
mv "$ledger.next" "$ledger"
jq -e --argjson pr "$native_game_dev_pr" --arg head_sha "$native_game_dev_head_sha" --arg sha "$native_game_dev_merge_sha" '.native_game_dev_pr == $pr and .native_game_dev_head_sha == $head_sha and .native_game_dev_merge_sha == $sha' "$ledger"

```

### Task 3: Independently accept the landed facade on clean native Ubuntu

**Files:** no repository file is changed.
**Produces:** a retained native evidence package, a signed evidence comment, an
independent review decision, and only then a native-verification closure.
**Stop point:** an unsupported host, Docker/SSH/port failure, absent branch/merge
ancestry, marker-restoration failure, browser-protocol failure, or owned-down
failure stops without source/port/daemon workaround.

- [ ] **1. Rehydrate by exact issue/PR titles and merge facts before cloning.** This task derives the fixed implementation-worktree path from the real issue number but does not depend on its filesystem state.

  ```bash
  set -euo pipefail
  sdd_root="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox/.superpowers/sdd/plan"
  game_dev_issue_title='Add native Ubuntu host support to toolkit contributor sandbox'
  native_verify_issue_title='Verify toolkit contributor sandbox on clean native Ubuntu'
  implementation_pr_title='feat: support native Ubuntu toolkit contributor sandbox'
  signature='— asset-pipeline agent, on behalf of KirkDiggler'
  project_id='PVT_kwHOAASbwc4Bcj4v'
  status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
  status_in_progress='a434eab1'
  status_done='e4d8ce42'
  issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
  gh issue list --repo KirkDiggler/game-dev --state all --search "in:title \"$game_dev_issue_title\"" --json number,title,url \
    > "$sdd_root/task3-game-dev-issue.json"
  gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$native_verify_issue_title\"" --json number,title,url \
    > "$sdd_root/task3-native-verify-issue.json"
  jq -e --arg title "$game_dev_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task3-game-dev-issue.json"
  jq -e --arg title "$native_verify_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task3-native-verify-issue.json"
  game_dev_issue="$(jq -r '.[0].number' "$sdd_root/task3-game-dev-issue.json")"
  native_verify_issue="$(jq -r '.[0].number' "$sdd_root/task3-native-verify-issue.json")"
  native_verify_url="$(jq -r '.[0].url' "$sdd_root/task3-native-verify-issue.json")"
  game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
  game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
  gh pr list --repo KirkDiggler/game-dev --state merged --head "$game_dev_branch" \
    --search "in:title \"$implementation_pr_title\"" \
    --json number,title,state,baseRefName,headRefName,headRefOid,mergeCommit \
    > "$sdd_root/task3-implementation-pr.json"
  jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" '
    length == 1 and .[0].title == $title and .[0].state == "MERGED"
    and .[0].baseRefName == "main" and .[0].headRefName == $head
    and (.[]|.headRefOid|type == "string" and length == 40)
    and (.[]|.mergeCommit.oid|type == "string" and length == 40)
  ' "$sdd_root/task3-implementation-pr.json"
  native_game_dev_pr="$(jq -r '.[0].number' "$sdd_root/task3-implementation-pr.json")"
  native_game_dev_head_sha="$(jq -r '.[0].headRefOid' "$sdd_root/task3-implementation-pr.json")"
  native_game_dev_merge_sha="$(jq -r '.[0].mergeCommit.oid' "$sdd_root/task3-implementation-pr.json")"
  native_clean_root="$HOME/toolkit-sandbox-native-${native_game_dev_merge_sha:0:12}"
  native_evidence="$native_clean_root/evidence"
  test ! -e "$native_clean_root"
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state \
    | jq -e '.state == "OPEN"'
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project \
    -F number="$native_verify_issue" > "$sdd_root/task3-native-verify-project-before.json"
  native_verify_item="$(jq -er '[.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] | if length == 1 then .[0].id else error("expected one Project 19 item") end' "$sdd_root/task3-native-verify-project-before.json")"
  jq -e --arg item "$native_verify_item" '
    [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
    | ($rows | length == 1) and $rows[0].id == $item
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name] == ["Todo"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Team") | .name] == ["Platform"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Feature") | .name] == ["Infra"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Kind") | .name] == ["Verify"])
  ' "$sdd_root/task3-native-verify-project-before.json"
  gh project item-edit --project-id "$project_id" --id "$native_verify_item" --field-id "$status_field" --single-select-option-id "$status_in_progress"
  ```

- [ ] **2. Preflight native host, tools, SSH, and the exact five-port set before any clone or stack action.**

  ```bash
  test -f /etc/os-release
  test -f /proc/sys/kernel/osrelease
  grep -Eq "^ID=(ubuntu|'ubuntu'|\"ubuntu\")$" /etc/os-release
  ! grep -Eqi 'wsl2|microsoft' /proc/sys/kernel/osrelease
  docker info
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD
  command -v git docker go node npm rsync jq ssh ss curl
  mkdir -p "$native_evidence"
  cat /etc/os-release | tee "$native_evidence/os-release.txt"
  cat /proc/sys/kernel/osrelease | tee "$native_evidence/kernel-osrelease.txt"
  docker info | tee "$native_evidence/docker-info.txt"
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD | tee "$native_evidence/github-ssh.txt"
  ss -ltnp '( sport = :80 or sport = :3001 or sport = :3002 or sport = :6380 or sport = :8080 )' \
    | tee "$native_evidence/ports-before.txt"
  test ! -s "$native_evidence/ports-before.txt" || ! grep -Eq ':(80|3001|3002|6380|8080)[[:space:]]' "$native_evidence/ports-before.txt"
  ```

If port output cannot show ownership or any listed port is occupied, retain
the preflight evidence, post a signed blocker, leave the verification open,
and stop. Do not kill, reuse, move, or configure a listener.

- [ ] **3. Clone the fresh facade, prove mapped root branches/origins and provider ancestry, then run focused gates.**

  ```bash
  git clone git@github.com:KirkDiggler/game-dev.git "$native_clean_root/game-dev"
  cd "$native_clean_root/game-dev"
  git merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a HEAD
  git merge-base --is-ancestor "$native_game_dev_merge_sha" HEAD
  ./scripts/toolkit-contributor.sh bootstrap | tee "$native_evidence/bootstrap-first.txt"
  ./scripts/toolkit-contributor.sh bootstrap | tee "$native_evidence/bootstrap-second.txt"
  test "$(git -C rpg-toolkit branch --show-current)" = main
  test "$(git -C rpg-api branch --show-current)" = dev
  test "$(git -C rpg-dnd5e-web branch --show-current)" = dev
  test "$(git -C rpg-deployment branch --show-current)" = main
  test "$(git -C rpg-toolkit remote get-url origin)" = git@github.com:KirkDiggler/rpg-toolkit.git
  test "$(git -C rpg-api remote get-url origin)" = git@github.com:KirkDiggler/rpg-api.git
  test "$(git -C rpg-dnd5e-web remote get-url origin)" = git@github.com:KirkDiggler/rpg-dnd5e-web.git
  test "$(git -C rpg-deployment remote get-url origin)" = git@github.com:KirkDiggler/rpg-deployment.git
  git -C rpg-api merge-base --is-ancestor 9099953f9bc86efbed9bf62209a96c54d9383d6b HEAD
  git -C rpg-dnd5e-web merge-base --is-ancestor cfa63138a1f06de65991c31b006f29fc2af1ad74 HEAD

  cd rpg-api
  bash scripts/toolkit-local-override.test.sh | tee "$native_evidence/api-override-contract.txt"
  go test ./cmd/sandboxseed -count=1 | tee "$native_evidence/api-sandboxseed.txt"
  go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1 \
    | tee "$native_evidence/api-sandboxseed-integration.txt"
  make pre-commit | tee "$native_evidence/api-pre-commit.txt"
  cd ..
  bash tests/toolkit-contributor-contract.sh | tee "$native_evidence/game-dev-contributor-contract.txt"
  bash tests/bootstrap-contract.sh | tee "$native_evidence/game-dev-bootstrap-contract.txt"
  cd rpg-dnd5e-web
  npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx \
    | tee "$native_evidence/web-sandbox-tests.txt"
  npm run ci-check | tee "$native_evidence/web-ci-check.txt"
  ```

- [ ] **4. Start, seed, and arm a recovery trap that never masks the original exit status.** Only the task-owned Vite PID may be stopped; no unrelated cleanup runs.

  ```bash
  cd "$native_clean_root/game-dev"
  marker_file='rulebooks/dnd5e/races/data.go'
  marker_applied=0
  vite_pid=''
  cleanup_native_acceptance() {
    original_status=$?
    trap - EXIT INT TERM
    if [ "$marker_applied" -eq 1 ]; then
      git -C "$native_clean_root/game-dev/rpg-toolkit" checkout -- "$marker_file" || printf '%s\n' 'marker checkout recovery failed' >&2
      "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" refresh || printf '%s\n' 'marker refresh recovery failed' >&2
      "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" seed || printf '%s\n' 'marker reseed recovery failed' >&2
      printf '%s\n' 'recovery attempted checkout, refresh, and reseed without replacing original status' >&2
    fi
    if [ -n "$vite_pid" ] && kill -0 "$vite_pid" 2>/dev/null; then
      kill "$vite_pid" || true
      wait "$vite_pid" || true
    fi
    "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" down || printf '%s\n' 'owned down recovery failed' >&2
    exit "$original_status"
  }
  trap cleanup_native_acceptance EXIT INT TERM

  ./scripts/toolkit-contributor.sh start | tee "$native_evidence/start.txt"
  ./scripts/toolkit-contributor.sh status | tee "$native_evidence/status.txt"
  grep -Fqx 'host mode: ubuntu-native' "$native_evidence/status.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/seed-first.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/seed-second.txt"
  for seed_file in "$native_evidence/seed-first.txt" "$native_evidence/seed-second.txt"; do
    grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=16 off_hand=shield' "$seed_file"
    grep -Ex 'sandboxseed: identity=toolkit-sandbox-barbarian character_id=[^[:space:]]+ strength=16' "$seed_file"
  done
  ```

- [ ] **5. Prove the reversible marker and keep its flag set through all restoration proof.** The only command which clears `marker_applied` is the final line after every required restoration assertion succeeds.

  ```bash
  cd "$native_clean_root/game-dev"
  perl -0pi -e 's/(Human: \{.*?abilities\.STR: )1,/${1}2,/s' rpg-toolkit/$marker_file
  marker_applied=1
  ./scripts/toolkit-contributor.sh refresh | tee "$native_evidence/marker-refresh-to-17.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/marker-seed-17.txt"
  grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=17 off_hand=shield' "$native_evidence/marker-seed-17.txt"
  git -C rpg-toolkit checkout -- "$marker_file"
  ./scripts/toolkit-contributor.sh refresh | tee "$native_evidence/marker-refresh-to-16.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/marker-seed-restored-16.txt"
  grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=16 off_hand=shield' "$native_evidence/marker-seed-restored-16.txt"
  git -C rpg-toolkit diff --exit-code -- "$marker_file"
  marker_applied=0
  ```

  If checkout, restored refresh, restored reseed, the restored `strength=16`
  record, or clean diff fails, the flag remains `1`; the trap re-checks out the
  marker source and attempts refresh, reseed, and owned down while preserving
  the failing command's original status. The focused integration report
  `api-sandboxseed-integration.txt`, not seeder stdout, is the evidence for
  Protection, FightingStyles, and a real shield.

- [ ] **6. Start only task-owned Vite and execute live browser evidence through the `chrome_devtools_*` MCP tools.** `screenshot.mjs` is optional secondary corroboration only after the MCP sequence; it never substitutes for Save/click/link/order actions.

  ```bash
  cd "$native_clean_root/game-dev/rpg-dnd5e-web"
  npm ci | tee "$native_evidence/web-npm-ci.txt"
  npm run dev > "$native_evidence/vite.log" 2>&1 &
  vite_pid=$!
  for attempt in $(seq 1 30); do
    if curl -fsS 'http://localhost:3001/?toolkitSandbox=1' > "$native_evidence/sandbox-page.html"; then break; fi
    sleep 2
  done
  test -s "$native_evidence/sandbox-page.html"
  grep -Fq 'http://localhost:3001' "$native_evidence/vite.log"
  ```

  Use these MCP actions, in this order; record snapshots/evaluations and append
  the observed successful PutDungeon key to
  `$native_evidence/put-dungeon-key-evidence.txt`. The exact expected normal
  href sequence is:

  | Arrangement            | Displayed hrefs, in order                                   |
  | ---------------------- | ----------------------------------------------------------- |
  | Fighter                | `http://localhost:3001/?playerId=toolkit-sandbox-fighter`   |
  | Barbarian              | `http://localhost:3001/?playerId=toolkit-sandbox-barbarian` |
  | Fighter then Barbarian | fighter href, then barbarian href                           |
  | Barbarian then Fighter | barbarian href, then fighter href                           |
  1. `chrome_devtools_new_page` the sandbox URL and retain `sandbox_page`;
     `chrome_devtools_take_snapshot` it.
  2. For **Fighter**, `chrome_devtools_select_page(sandbox_page)`,
     `chrome_devtools_take_snapshot`, and `chrome_devtools_click` the enabled
     Save button. `chrome_devtools_wait_for` exactly `Saved as "toolkit-contributor-sandbox".`,
     then `chrome_devtools_evaluate_script` and record that exact text/key.
     `chrome_devtools_wait_for` Fighter enabled,
     `chrome_devtools_take_snapshot`, and `chrome_devtools_click` Fighter.
     `chrome_devtools_wait_for` the displayed normal fighter href and
     `chrome_devtools_evaluate_script` the visible link array to assert only
     `/?playerId=toolkit-sandbox-fighter`. Immediately
     `chrome_devtools_new_page` that href,
     `chrome_devtools_wait_for` `[data-testid="encounter-view"]`,
     `chrome_devtools_evaluate_script` selector existence,
     `chrome_devtools_take_snapshot`, and
     `chrome_devtools_take_screenshot`
     `$native_evidence/toolkit-sandbox-fighter-only-fighter-gameview.png`.
     Only then `chrome_devtools_select_page(sandbox_page)`.
  3. Repeat the complete Save/
     `chrome_devtools_wait_for`/`chrome_devtools_evaluate_script` cycle for
     **Barbarian**, then immediately run
     `chrome_devtools_new_page`, `chrome_devtools_wait_for`,
     `chrome_devtools_evaluate_script`, `chrome_devtools_take_snapshot`, and
     `chrome_devtools_take_screenshot`
     `$native_evidence/toolkit-sandbox-barbarian-only-barbarian-gameview.png`
     from the displayed barbarian href before returning to the sandbox.
  4. Repeat the complete cycle for **Fighter then Barbarian**.
     `chrome_devtools_evaluate_script` must return fighter then barbarian hrefs
     in that order. Immediately `chrome_devtools_new_page`,
     `chrome_devtools_wait_for`, `chrome_devtools_evaluate_script`,
     `chrome_devtools_take_snapshot`, and `chrome_devtools_take_screenshot`
     the fighter tab as
     `$native_evidence/toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png`,
     then do the same for the barbarian tab as
     `$native_evidence/toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png`.
     Return to the sandbox only after both images exist.
  5. Repeat the complete cycle for **Barbarian then Fighter**. Require
     barbarian then fighter hrefs, then immediately use
     `chrome_devtools_new_page`, `chrome_devtools_wait_for`,
     `chrome_devtools_evaluate_script`, `chrome_devtools_take_snapshot`, and
     `chrome_devtools_take_screenshot` for
     `$native_evidence/toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png`
     and
     `$native_evidence/toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png`
     before `chrome_devtools_select_page` returns to the sandbox.

  A failed Save, exact saved text, key evaluation, selected-party enablement,
  link evaluation, new-page load, encounter selector, or screenshot records the
  current MCP error/snapshot and stops immediately. Do not begin a later order,
  perform another Save, or capture all routes at the end. Run only the armed
  marker/Vite/owned-down recovery. Harness URLs do not count.

- [ ] **7. Run negatives, perform owned shutdown, publish the archive through the issue UI, and close only after an independent PASS.**

  ```bash
  cd "$native_clean_root/game-dev/rpg-api"
  go test ./internal/auth -run TestUnaryAuthInterceptor_DevScheme_NotAllowed -count=1 \
    | tee "$native_evidence/production-dev-header-negative.txt"
  cd "$native_clean_root/game-dev/rpg-dnd5e-web"
  npm run test:run -- src/toolkit-contributor-sandbox/route.test.ts \
    | tee "$native_evidence/production-sandbox-route-negative.txt"
  if kill -0 "$vite_pid" 2>/dev/null; then kill "$vite_pid"; wait "$vite_pid" || true; fi
  vite_pid=''
  cd "$native_clean_root/game-dev"
  ./scripts/toolkit-contributor.sh down | tee "$native_evidence/down.txt"
  test ! -e rpg-api/local-toolkit/rulebooks/dnd5e
  test -d rpg-toolkit/.git
  test -d rpg-api/.git
  test -d rpg-dnd5e-web/.git
  test -d rpg-deployment/.git
  trap - EXIT INT TERM
  find "$native_evidence" -maxdepth 1 -type f -printf '%f\n' | sort | tee "$native_evidence/manifest.txt"
  sha256sum "$native_evidence"/* > "$native_evidence/SHA256SUMS.txt"
  native_archive="$native_clean_root/task3-native-evidence.tar.gz"
  native_archive_sha256="$native_clean_root/task3-native-evidence.tar.gz.sha256"
  tar -C "$native_clean_root" -czf "$native_archive" evidence
  (
    cd "$native_clean_root"
    sha256sum "$(basename "$native_archive")" > "$(basename "$native_archive_sha256")"
  )
  test -s "$native_archive"
  test -s "$native_archive_sha256"
  evidence_marker='<!-- native-ubuntu-delivery:task3-evidence -->'
  printf '%s\n%s\n%s\n%s\n\n%s\n' "$evidence_marker" \
    "Native clean acceptance for game-dev PR #$native_game_dev_pr at merge $native_game_dev_merge_sha is complete." \
    'Attachments: task3-native-evidence.tar.gz, task3-native-evidence.tar.gz.sha256, and the six required order-qualified PNGs.' \
    'The archive contains host/Docker/SSH/ports, focused reports including Protection/FightingStyles evidence, exact seed records, marker transcript, PutDungeon transcript, negatives, and owned-down proof.' \
    "$signature" > "$sdd_root/task3-evidence-comment.md"
  ```

  Publish that signed comment **through the GitHub issue UI**, never with `gh
issue comment` and never by claiming that a CLI comment upload attached a
  file. Use the actual MCP calls in this order against `$native_verify_url`:

  1. `chrome_devtools_new_page` the issue and retain `native_issue_page`, then
     `chrome_devtools_take_snapshot` it.
  2. `chrome_devtools_click` **Add a comment**, take another
     `chrome_devtools_take_snapshot`, and use
     `chrome_devtools_evaluate_script` to put the exact contents of
     `$sdd_root/task3-evidence-comment.md` into the comment editor and dispatch
     its input event. Snapshot the marker, merge SHA, and signature in the
     editor before publishing.
  3. Use `chrome_devtools_upload_file` on that comment's attachment control for
     `task3-native-evidence.tar.gz`,
     `task3-native-evidence.tar.gz.sha256`, and these six files:

     ```text
     toolkit-sandbox-fighter-only-fighter-gameview.png
     toolkit-sandbox-barbarian-only-barbarian-gameview.png
     toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png
     toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png
     toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png
     toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png
     ```

     After every upload, use `chrome_devtools_wait_for` its filename in the
     composer and `chrome_devtools_evaluate_script` the attachment list to
     record its visible filename. Do not substitute a local path or a `gh`
     command for an upload.

  4. `chrome_devtools_click` **Comment**, then `chrome_devtools_wait_for` the
     evidence marker and `chrome_devtools_take_snapshot` the published signed
     comment. Use `chrome_devtools_evaluate_script` to record its displayed
     attachment links.

  Read back the published UI comment and require its author, content, names,
  and attachment URLs. This is a readback only; it does not create or upload a
  comment.

  ```bash
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json comments \
    > "$sdd_root/task3-evidence-readback.json"
  jq -e --arg marker "$evidence_marker" --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
    [.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $comments
    | ($comments | length == 1)
      and ($comments[0].body | contains($marker))
      and ($comments[0].body | contains($merge))
      and ($comments[0].body | endswith($signature))
      and ([
        "task3-native-evidence.tar.gz",
        "task3-native-evidence.tar.gz.sha256",
        "toolkit-sandbox-fighter-only-fighter-gameview.png",
        "toolkit-sandbox-barbarian-only-barbarian-gameview.png",
        "toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png",
        "toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png",
        "toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png",
        "toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png"
      ] | all(. as $filename | ($comments[0].body | contains($filename))))
      and ([ $comments[0].body | scan("https://github\\.com/user-attachments/[^[:space:]]+") ] | unique | length >= 8)
  ' "$sdd_root/task3-evidence-readback.json"
  jq -r --arg marker "$evidence_marker" '
    [.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))]
    | .[0].body | scan("https://github\\.com/user-attachments/[^[:space:]]+")
  ' "$sdd_root/task3-evidence-readback.json" | sort -u > "$sdd_root/task3-attachment-urls.txt"
  test "$(wc -l < "$sdd_root/task3-attachment-urls.txt")" -ge 8
  ```

  For **each** URL in `task3-attachment-urls.txt`, explicitly use
  `chrome_devtools_new_page` (or reload the existing attachment page),
  `chrome_devtools_wait_for`, `chrome_devtools_take_snapshot`, and
  `chrome_devtools_evaluate_script` to confirm the URL opens. Record the
  resulting statement `viewed <filename> at <URL>` in
  `$native_evidence/task3-attachment-viewed.txt`; the statement must name all
  eight files. A missing/unopenable attachment stops before review.

  A fresh reviewer, distinct from the executor, reviews the retained package,
  command output, image order/content, no-bypass/no-auto-kill posture, and
  failure handling. The reviewer may sign through the same `KirkDiggler`
  account and signature, but is still a fresh reviewer rather than the
  executor. Require this unique signed review marker and full package before
  marking the issue done:

  ```bash
  native_review_marker='<!-- native-ubuntu-delivery:task3-native-review -->'
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json comments \
    > "$sdd_root/task3-native-review-readback.json"
  jq -e --arg marker "$native_review_marker" --arg evidence "$evidence_marker" \
    --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
    [.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $reviews
    | ($reviews | length == 1)
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review: PASS"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review-package: COMPLETE"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review-findings: none"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-reviewed-merge: " + $merge))
      and ($reviews[0].body | contains($evidence))
      and ($reviews[0].body | contains("task3-native-evidence.tar.gz"))
      and ($reviews[0].body | contains("task3-native-evidence.tar.gz.sha256"))
      and ($reviews[0].body | test("https://github\\.com/user-attachments/"))
      and ($reviews[0].body | endswith($signature))
  ' "$sdd_root/task3-native-review-readback.json"
  gh project item-edit --project-id "$project_id" --id "$native_verify_item" --field-id "$status_field" --single-select-option-id "$status_done"
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project \
    -F number="$native_verify_issue" > "$sdd_root/task3-native-verify-project-done.json"
  jq -e --arg item "$native_verify_item" '
    def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
    [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
    | ($rows | length == 1) and $rows[0].id == $item
      and ($rows[0] | has_field("Status"; "Done"))
      and ($rows[0] | has_field("Team"; "Platform"))
      and ($rows[0] | has_field("Feature"; "Infra"))
      and ($rows[0] | has_field("Kind"; "Verify"))
  ' "$sdd_root/task3-native-verify-project-done.json"
  gh issue close "$native_verify_issue" --repo KirkDiggler/rpg-project
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state | jq -e '.state == "CLOSED"'
  ```

### Task 4: Formally amend and execute #210 on the landed baseline

**Files:** no repository file is changed.
**Produces:** the latest self-contained GitHub AGENT PICKUP on #210, independent
WSL2 evidence, and only then #210 closure.
**Stop point:** Task 4 does not read `$native_clean_root`, native logs, native
ledger facts, or any native filesystem. It rehydrates only from GitHub and the
published AGENT PICKUP; it creates no new WSL issue, branch, or PR.

- [ ] **1. Rehydrate exactly from GitHub and prove native acceptance is accepted.**

  ```bash
  set -euo pipefail
  sdd_root="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox/.superpowers/sdd/plan"
  game_dev_issue_title='Add native Ubuntu host support to toolkit contributor sandbox'
  native_verify_issue_title='Verify toolkit contributor sandbox on clean native Ubuntu'
  wsl_issue_title='Verify the toolkit contributor sandbox on clean WSL2'
  implementation_pr_title='feat: support native Ubuntu toolkit contributor sandbox'
  signature='— asset-pipeline agent, on behalf of KirkDiggler'
  project_id='PVT_kwHOAASbwc4Bcj4v'
  status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
  status_done='e4d8ce42'
  issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
  gh issue list --repo KirkDiggler/game-dev --state all --search "in:title \"$game_dev_issue_title\"" --json number,title \
    > "$sdd_root/task4-game-dev-issue.json"
  gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$native_verify_issue_title\"" --json number,title,url \
    > "$sdd_root/task4-native-verify-issue.json"
  gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$wsl_issue_title\"" --json number,title,url \
    > "$sdd_root/task4-wsl-issue.json"
  jq -e --arg title "$game_dev_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task4-game-dev-issue.json"
  jq -e --arg title "$native_verify_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task4-native-verify-issue.json"
  jq -e --arg title "$wsl_issue_title" 'length == 1 and .[0].title == $title and .[0].number == 210' "$sdd_root/task4-wsl-issue.json"
  game_dev_issue="$(jq -r '.[0].number' "$sdd_root/task4-game-dev-issue.json")"
  native_verify_issue="$(jq -r '.[0].number' "$sdd_root/task4-native-verify-issue.json")"
  native_verify_url="$(jq -r '.[0].url' "$sdd_root/task4-native-verify-issue.json")"
  wsl_issue=210
  game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
  game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
  gh pr list --repo KirkDiggler/game-dev --state merged --head "$game_dev_branch" \
    --search "in:title \"$implementation_pr_title\"" --json number,title,baseRefName,headRefName,headRefOid,state,mergeCommit \
    > "$sdd_root/task4-implementation-pr.json"
  jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" '
    length == 1 and .[0].title == $title and .[0].state == "MERGED"
    and .[0].baseRefName == "main" and .[0].headRefName == $head
    and (.[]|.headRefOid|type == "string" and length == 40)
    and (.[]|.mergeCommit.oid|type == "string" and length == 40)
  ' "$sdd_root/task4-implementation-pr.json"
  native_game_dev_pr="$(jq -r '.[0].number' "$sdd_root/task4-implementation-pr.json")"
  native_game_dev_head_sha="$(jq -r '.[0].headRefOid' "$sdd_root/task4-implementation-pr.json")"
  native_game_dev_merge_sha="$(jq -r '.[0].mergeCommit.oid' "$sdd_root/task4-implementation-pr.json")"
  native_evidence_marker='<!-- native-ubuntu-delivery:task3-evidence -->'
  native_review_marker='<!-- native-ubuntu-delivery:task3-native-review -->'
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state,comments \
    > "$sdd_root/task4-native-review-readback.json"
  jq -e --arg marker "$native_review_marker" --arg evidence "$native_evidence_marker" \
    --arg merge "$native_game_dev_merge_sha" --arg sig "$signature" '
      .state == "CLOSED"
      and ([.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $reviews
        | ($reviews | length == 1)
          and ($reviews[0].body | contains("native-ubuntu-acceptance-review: PASS"))
          and ($reviews[0].body | contains("native-ubuntu-acceptance-review-package: COMPLETE"))
          and ($reviews[0].body | contains("native-ubuntu-acceptance-review-findings: none"))
          and ($reviews[0].body | contains("native-ubuntu-acceptance-reviewed-merge: " + $merge))
          and ($reviews[0].body | contains($evidence))
          and ($reviews[0].body | contains("task3-native-evidence.tar.gz"))
          and ($reviews[0].body | contains("task3-native-evidence.tar.gz.sha256"))
          and ($reviews[0].body | test("https://github\\.com/user-attachments/"))
          and ($reviews[0].body | endswith($sig)))
    '
  gh issue view 210 --repo KirkDiggler/rpg-project --json state,title \
    | jq -e --arg title "$wsl_issue_title" '.state == "OPEN" and .title == $title'
  gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=210 \
    > "$sdd_root/task4-wsl-project-before.json"
  wsl_project_item="$(jq -er '[.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] | if length == 1 then .[0].id else error("expected one Project 19 item") end' "$sdd_root/task4-wsl-project-before.json")"
  jq -e --arg item "$wsl_project_item" '
    [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
    | ($rows | length == 1) and $rows[0].id == $item
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name] == ["In Progress"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Team") | .name] == ["Cross-team"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Feature") | .name] == ["Infra"])
      and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Kind") | .name] == ["Verify"])
  ' "$sdd_root/task4-wsl-project-before.json"
  ```

- [ ] **2. Preserve #210 ownership, post the formal superseding packet, and make GitHub the only executor handoff.** Generate the durable local packet only for upload; the WSL executor uses the resulting latest GitHub comment, never the native filesystem or local ledger.

  The packet must state actual SHA values, the clean WSL root
  `$HOME/toolkit-sandbox-wsl2-${native_game_dev_merge_sha:0:12}`, original #209/#60/#792/#747 facts, `main/dev/dev/main` clone checks, the exact port set, marker trap semantics, negatives, owned down, six filenames, and this self-contained MCP browser protocol:

  ```text
  chrome_devtools_new_page sandbox → chrome_devtools_take_snapshot → for each
  Fighter, Barbarian, Fighter then Barbarian, Barbarian then Fighter:
  chrome_devtools_select_page sandbox; chrome_devtools_take_snapshot;
  re-save with chrome_devtools_click on enabled Save;
  chrome_devtools_wait_for exact `Saved as "toolkit-contributor-sandbox".`;
  chrome_devtools_evaluate_script/save PutDungeon key;
  chrome_devtools_wait_for selected party enabled; chrome_devtools_click it;
  chrome_devtools_wait_for/chrome_devtools_evaluate_script exact normal hrefs;
  immediately chrome_devtools_new_page every href;
  chrome_devtools_wait_for/chrome_devtools_evaluate_script asserts
  [data-testid="encounter-view"]; chrome_devtools_take_snapshot/
  chrome_devtools_take_screenshot each required order-qualified PNG; only then
  chrome_devtools_select_page sandbox. A failure records current MCP evidence
  and takes no later action.
  ```

Create `$packet_path` in the standard native SDD directory and post it as
the latest #210 comment. It is a rendered GitHub-only AGENT PICKUP, not a
reference to this plan, a native acceptance directory, a ledger, or a local
template. It begins exactly `<!-- native-ubuntu-delivery:task4-wsl-pickup -->`,
contains headings `FORMAL EXECUTION-BASELINE AMENDMENT` and `AGENT PICKUP —
START HERE ON UBUNTU WSL2`, ends with the required signature, and has no
closing keyword for #208.

Before posting, the packet must contain actual Task 4 GitHub-derived values,
never unresolved variables: original #209/#60/#792/#747 SHAs, the merged
implementation PR number/head/merge SHA, and its clean WSL root. Its first
shell block queries the game-dev issue by the exact title, requires one result,
derives the `feat/${game_dev_issue}-native-ubuntu-toolkit-contributor` branch, queries the merged
PR by exact title/head, requires one result, validates base `main`, immutable
head SHA, merge SHA, and #210's exact title/state. It then performs the
complete clean WSL preflight, clone, `main/dev/dev/main` root/origin checks,
#792/#747 dev-clone ancestry, focused API/game-dev/web reports, start/status/
two-seed flow, negative tests, owned down, checksum manifest, and attachment
instructions contained in this Task 4 design.

The packet's marker trap sets `marker_applied=1` before refresh/reseed to the
fighter `strength=17 off_hand=shield` record and clears it only after source
checkout, restored refresh, restored fighter `strength=16 off_hand=shield`,
and clean diff. Its trap always re-checks out the marker source and attempts
refresh/reseed/down without replacing the original status; it stops only its
task-owned Vite PID and performs no unrelated cleanup.

The packet's browser section names all actual MCP operations:
`chrome_devtools_new_page`, `chrome_devtools_select_page`,
`chrome_devtools_take_snapshot`, `chrome_devtools_click`,
`chrome_devtools_wait_for`, `chrome_devtools_evaluate_script`, and
`chrome_devtools_take_screenshot`. For each Fighter, Barbarian, Fighter then
Barbarian, and Barbarian then Fighter it re-saves first, waits exact `Saved as "toolkit-contributor-sandbox".`, records the PutDungeon key, waits the selected party enabled, evaluates normal hrefs, immediately opens each href in a new tab, uses `chrome_devtools_evaluate_script` to assert `[data-testid="encounter-view"]`, and captures before returning to sandbox:

```text
toolkit-sandbox-fighter-only-fighter-gameview.png
toolkit-sandbox-barbarian-only-barbarian-gameview.png
toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png
toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png
```

The packet must also contain these literal WSL evidence requirements; do not
summarize or weaken them. It must run the two seed files through these exact
regular expressions, with both expressions required for **both** files:

```bash
wsl_evidence="$wsl_clean_root/evidence"
for seed_file in "$wsl_evidence/seed-first.txt" "$wsl_evidence/seed-second.txt"; do
  grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=16 off_hand=shield' "$seed_file"
  grep -Ex 'sandboxseed: identity=toolkit-sandbox-barbarian character_id=[^[:space:]]+ strength=16' "$seed_file"
done
grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=17 off_hand=shield' "$wsl_evidence/marker-seed-17.txt"
grep -Ex 'sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+ strength=16 off_hand=shield' "$wsl_evidence/marker-seed-restored-16.txt"
```

The packet names the focused API integration report as the evidence for
Protection, FightingStyles, and the real shield; it must not infer that from
runtime seeder-output text. After its manifest and SHA256SUMS are complete,
the WSL executor creates the archive and checksum exactly as follows:

```bash
wsl_archive="$wsl_clean_root/task4-wsl-evidence.tar.gz"
wsl_archive_sha256="$wsl_clean_root/task4-wsl-evidence.tar.gz.sha256"
tar -C "$wsl_clean_root" -czf "$wsl_archive" evidence
(
  cd "$wsl_clean_root"
  sha256sum "$(basename "$wsl_archive")" > "$(basename "$wsl_archive_sha256")"
)
test -s "$wsl_archive"
test -s "$wsl_archive_sha256"
```

It publishes the signed evidence comment
in the #210 GitHub issue UI with marker
`<!-- native-ubuntu-delivery:task4-wsl-evidence -->`, the exact merged
`$native_game_dev_merge_sha`, and the required signature. It uses actual
`chrome_devtools_new_page`, `chrome_devtools_take_snapshot`,
`chrome_devtools_click`, `chrome_devtools_evaluate_script`,
`chrome_devtools_upload_file`, and `chrome_devtools_wait_for` calls to put the
archive, checksum, and these six PNGs in that same issue comment:

```text
toolkit-sandbox-fighter-only-fighter-gameview.png
toolkit-sandbox-barbarian-only-barbarian-gameview.png
toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png
toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png
```

The pickup explicitly requires `chrome_devtools_wait_for` each filename in the
composer, a UI click to publish, and a comment readback that asserts author
`KirkDiggler`, the WSL evidence marker, exact merge SHA, signature, all eight
filenames, and at least eight distinct
`https://github.com/user-attachments/` URLs. It then explicitly reloads or
opens every returned attachment URL with `chrome_devtools_new_page`, waits,
takes a snapshot, evaluates the loaded attachment, and records a
`viewed <filename> at <URL>` statement. It does not say that `gh issue comment`
uploads an attachment.

`screenshot.mjs` is only secondary corroboration. A browser failure retains
the current MCP artifact and takes no later save, party action, or catch-up
screenshot. The packet has exactly one copy-paste prompt and tells the WSL
executor to use only this GitHub comment.

```bash
packet_marker='<!-- native-ubuntu-delivery:task4-wsl-pickup -->'
packet_path="$sdd_root/task4-wsl-pickup.md"
test -s "$packet_path"
gh issue comment 210 --repo KirkDiggler/rpg-project --body-file "$packet_path"
gh issue view 210 --repo KirkDiggler/rpg-project --json comments \
  | jq -e --arg marker "$packet_marker" --arg sig '— asset-pipeline agent, on behalf of KirkDiggler' '
      [.comments[] | select((.body|contains($marker)) and (.body|contains("FORMAL EXECUTION-BASELINE AMENDMENT")) and (.body|contains("AGENT PICKUP — START HERE ON UBUNTU WSL2")) and (.body|endswith($sig))] | length == 1
    '
```

- [ ] **3. Execute only the posted GitHub packet and close #210 only after independent review.** The executor starts with the packet's own GitHub-derived baseline, not an inherited shell. It must prove `host mode: ubuntu-wsl2`, all clean clone/root/ancestry assertions, the exact two-file seed records, the `strength=16 → 17 → 16` marker evidence, PutDungeon evidence, six immediate MCP screenshots, negatives, archive/checksum attachment readback and URL views, and owned cleanup. Any failure retains `In Progress` and does not close #208.

A fresh reviewer distinct from the executor verifies the WSL package, even if
both use the `KirkDiggler` account and signature. The reviewer comment uses its
own unique marker and cites the evidence marker and exact merge SHA.

```bash
wsl_evidence_marker='<!-- native-ubuntu-delivery:task4-wsl-evidence -->'
wsl_review_marker='<!-- native-ubuntu-delivery:task4-wsl-review -->'
gh issue view 210 --repo KirkDiggler/rpg-project --json comments \
  > "$sdd_root/task4-wsl-evidence-readback.json"
jq -e --arg marker "$wsl_evidence_marker" --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
  [.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $comments
  | ($comments | length == 1)
    and ($comments[0].body | contains($merge))
    and ($comments[0].body | endswith($signature))
    and ([
      "task4-wsl-evidence.tar.gz",
      "task4-wsl-evidence.tar.gz.sha256",
      "toolkit-sandbox-fighter-only-fighter-gameview.png",
      "toolkit-sandbox-barbarian-only-barbarian-gameview.png",
      "toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png",
      "toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png",
      "toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png",
      "toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png"
    ] | all(. as $filename | ($comments[0].body | contains($filename))))
    and ([ $comments[0].body | scan("https://github\\.com/user-attachments/[^[:space:]]+") ] | unique | length >= 8)
' "$sdd_root/task4-wsl-evidence-readback.json"
gh issue view 210 --repo KirkDiggler/rpg-project --json comments \
  > "$sdd_root/task4-wsl-review-readback.json"
jq -e --arg marker "$wsl_review_marker" --arg evidence "$wsl_evidence_marker" \
  --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
  [.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $reviews
  | ($reviews | length == 1)
    and ($reviews[0].body | contains("wsl2-acceptance-review: PASS"))
    and ($reviews[0].body | contains("wsl2-acceptance-review-package: COMPLETE"))
    and ($reviews[0].body | contains("wsl2-acceptance-review-findings: none"))
    and ($reviews[0].body | contains("wsl2-acceptance-reviewed-merge: " + $merge))
    and ($reviews[0].body | contains($evidence))
    and ($reviews[0].body | contains("task4-wsl-evidence.tar.gz"))
    and ($reviews[0].body | contains("task4-wsl-evidence.tar.gz.sha256"))
    and ($reviews[0].body | test("https://github\\.com/user-attachments/"))
    and ($reviews[0].body | endswith($signature))
' "$sdd_root/task4-wsl-review-readback.json"
gh project item-edit --project-id "$project_id" --id "$wsl_project_item" --field-id "$status_field" --single-select-option-id "$status_done"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=210 \
  > "$sdd_root/task4-wsl-project-done.json"
jq -e --arg item "$wsl_project_item" '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ($rows[0] | has_field("Status"; "Done"))
    and ($rows[0] | has_field("Team"; "Cross-team"))
    and ($rows[0] | has_field("Feature"; "Infra"))
    and ($rows[0] | has_field("Kind"; "Verify"))
' "$sdd_root/task4-wsl-project-done.json"
gh issue close 210 --repo KirkDiggler/rpg-project
gh issue view 210 --repo KirkDiggler/rpg-project --json state | jq -e '.state == "CLOSED"'
```

### Task 5: Reconcile exact delivery state and close #208 only after every required record is accepted

**Files:** no repository file is changed.
**Produces:** signed parent summary, truthful board status, and final parent closure.
**Stop point:** no merged PR, one-host-only result, unsigned/missing evidence,
wrong board ownership, wrong closure mechanism, missing merge ancestry, or
missing exact comment can close #208.

- [ ] **1. Rehydrate every record by exact title, PR head, merge SHA, and issue state.**

```bash
set -euo pipefail
sdd_root="$HOME/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox/.superpowers/sdd/plan"
game_dev_issue_title='Add native Ubuntu host support to toolkit contributor sandbox'
native_verify_issue_title='Verify toolkit contributor sandbox on clean native Ubuntu'
wsl_issue_title='Verify the toolkit contributor sandbox on clean WSL2'
implementation_pr_title='feat: support native Ubuntu toolkit contributor sandbox'
tracking_pr_title='docs: native Ubuntu toolkit contributor sandbox design and plan'
tracking_pr_head='design/native-ubuntu-toolkit-sandbox'
signature='— asset-pipeline agent, on behalf of KirkDiggler'
project_id='PVT_kwHOAASbwc4Bcj4v'
status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
status_done='e4d8ce42'
issue_project_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){number url projectItems(first:20){nodes{id project{id number title} fieldValues(first:30){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2SingleSelectField{name}}}}}}}}}}'
parent_query='query($owner:String!, $repo:String!, $number:Int!) { repository(owner:$owner,name:$repo) { issue(number:$number) { number url parent { number repository { nameWithOwner } } subIssues(first:100) { nodes { number url repository { nameWithOwner } } } } } }'
gh pr list --repo KirkDiggler/rpg-project --state merged --head "$tracking_pr_head" --search "in:title \"$tracking_pr_title\"" --json number,title,state,baseRefName,headRefName,mergeCommit > "$sdd_root/task5-tracking-pr.json"
jq -e --arg title "$tracking_pr_title" --arg head "$tracking_pr_head" 'length == 1 and .[0].title == $title and .[0].state == "MERGED" and .[0].baseRefName == "main" and .[0].headRefName == $head and (.[]|.mergeCommit.oid|type == "string" and length == 40)' "$sdd_root/task5-tracking-pr.json"
tracking_pr="$(jq -r '.[0].number' "$sdd_root/task5-tracking-pr.json")"
gh issue list --repo KirkDiggler/game-dev --state all --search "in:title \"$game_dev_issue_title\"" --json number,title,url \
  > "$sdd_root/task5-game-dev-issue.json"
gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$native_verify_issue_title\"" --json number,title,url \
  > "$sdd_root/task5-native-verify-issue.json"
gh issue list --repo KirkDiggler/rpg-project --state all --search "in:title \"$wsl_issue_title\"" --json number,title,url \
  > "$sdd_root/task5-wsl-issue.json"
jq -e --arg title "$game_dev_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task5-game-dev-issue.json"
jq -e --arg title "$native_verify_issue_title" 'length == 1 and .[0].title == $title' "$sdd_root/task5-native-verify-issue.json"
jq -e --arg title "$wsl_issue_title" 'length == 1 and .[0].title == $title and .[0].number == 210' "$sdd_root/task5-wsl-issue.json"
game_dev_issue="$(jq -r '.[0].number' "$sdd_root/task5-game-dev-issue.json")"
native_verify_issue="$(jq -r '.[0].number' "$sdd_root/task5-native-verify-issue.json")"
native_verify_url="$(jq -r '.[0].url' "$sdd_root/task5-native-verify-issue.json")"
game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
gh pr list --repo KirkDiggler/game-dev --state merged --head "$game_dev_branch" --search "in:title \"$implementation_pr_title\"" \
  --json number,title,state,baseRefName,headRefName,headRefOid,mergeCommit \
  > "$sdd_root/task5-implementation-pr.json"
jq -e --arg title "$implementation_pr_title" --arg head "$game_dev_branch" '
  length == 1 and .[0].title == $title and .[0].state == "MERGED"
  and .[0].baseRefName == "main" and .[0].headRefName == $head
  and (.[]|.headRefOid|type == "string" and length == 40)
  and (.[]|.mergeCommit.oid|type == "string" and length == 40)
' "$sdd_root/task5-implementation-pr.json"
native_game_dev_pr="$(jq -r '.[0].number' "$sdd_root/task5-implementation-pr.json")"
native_game_dev_head_sha="$(jq -r '.[0].headRefOid' "$sdd_root/task5-implementation-pr.json")"
native_game_dev_merge_sha="$(jq -r '.[0].mergeCommit.oid' "$sdd_root/task5-implementation-pr.json")"
```

- [ ] **2. Assert every closure predicate with `jq -e`, including states, Project fields, parent links, PR files/Kirk review packages/hosted checks/closing count, ancestry, and signatures.**

```bash
signature='— asset-pipeline agent, on behalf of KirkDiggler'
native_evidence_marker='<!-- native-ubuntu-delivery:task3-evidence -->'
native_review_marker='<!-- native-ubuntu-delivery:task3-native-review -->'
wsl_evidence_marker='<!-- native-ubuntu-delivery:task4-wsl-evidence -->'
wsl_review_marker='<!-- native-ubuntu-delivery:task4-wsl-review -->'
gh issue view 211 --repo KirkDiggler/rpg-project --json state,comments \
  | jq -e --arg sig "$signature" '.state == "CLOSED" and ([.comments[] | select((.body|contains("native-ubuntu-delivery:task1-handoff")) and (.body|endswith($sig))] | length == 1)'
issue_closure_query='query($owner:String!,$repo:String!,$number:Int!){repository(owner:$owner,name:$repo){issue(number:$number){state closedByPullRequestsReferences(first:20){nodes{number state mergedAt repository{nameWithOwner}}}}}}'
gh api graphql -f query="$issue_closure_query" -F owner=KirkDiggler -F repo=game-dev -F number="$game_dev_issue" \
  > "$sdd_root/task5-game-dev-closure.json"
jq -e --argjson pr "$native_game_dev_pr" '
  .data.repository.issue as $issue
  | $issue.state == "CLOSED"
    and ($issue.closedByPullRequestsReferences.nodes | length == 1)
    and $issue.closedByPullRequestsReferences.nodes[0].number == $pr
    and $issue.closedByPullRequestsReferences.nodes[0].state == "MERGED"
    and ($issue.closedByPullRequestsReferences.nodes[0].mergedAt | type == "string")
    and $issue.closedByPullRequestsReferences.nodes[0].repository.nameWithOwner == "KirkDiggler/game-dev"
' "$sdd_root/task5-game-dev-closure.json"
gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state,comments \
  > "$sdd_root/task5-native-review-readback.json"
jq -e --arg marker "$native_review_marker" --arg evidence "$native_evidence_marker" \
  --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
  .state == "CLOSED"
  and ([.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $reviews
    | ($reviews | length == 1)
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review: PASS"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review-package: COMPLETE"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-review-findings: none"))
      and ($reviews[0].body | contains("native-ubuntu-acceptance-reviewed-merge: " + $merge))
      and ($reviews[0].body | contains($evidence))
      and ($reviews[0].body | contains("task3-native-evidence.tar.gz"))
      and ($reviews[0].body | contains("task3-native-evidence.tar.gz.sha256"))
      and ($reviews[0].body | test("https://github\\.com/user-attachments/"))
      and ($reviews[0].body | endswith($signature)))
' "$sdd_root/task5-native-review-readback.json"
gh issue view 210 --repo KirkDiggler/rpg-project --json state,comments \
  > "$sdd_root/task5-wsl-review-readback.json"
jq -e --arg marker "$wsl_review_marker" --arg evidence "$wsl_evidence_marker" \
  --arg merge "$native_game_dev_merge_sha" --arg signature "$signature" '
  .state == "CLOSED"
  and ([.comments[] | select(.author.login == "KirkDiggler" and (.body | contains($marker)))] as $reviews
    | ($reviews | length == 1)
      and ($reviews[0].body | contains("wsl2-acceptance-review: PASS"))
      and ($reviews[0].body | contains("wsl2-acceptance-review-package: COMPLETE"))
      and ($reviews[0].body | contains("wsl2-acceptance-review-findings: none"))
      and ($reviews[0].body | contains("wsl2-acceptance-reviewed-merge: " + $merge))
      and ($reviews[0].body | contains($evidence))
      and ($reviews[0].body | contains("task4-wsl-evidence.tar.gz"))
      and ($reviews[0].body | contains("task4-wsl-evidence.tar.gz.sha256"))
      and ($reviews[0].body | test("https://github\\.com/user-attachments/"))
      and ($reviews[0].body | endswith($signature)))
' "$sdd_root/task5-wsl-review-readback.json"
gh issue view 208 --repo KirkDiggler/rpg-project --json state | jq -e '.state == "OPEN"'

gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev \
  --json title,state,baseRefName,headRefName,headRefOid,mergeCommit,files,statusCheckRollup,comments \
  > "$sdd_root/task5-pr-readback.json"
jq -e --arg head "$game_dev_branch" --arg head_sha "$native_game_dev_head_sha" --arg signature "$signature" '
  def good_check:
    (.conclusion // .state // "") as $result
    | $result == "SUCCESS" or $result == "NEUTRAL" or $result == "SKIPPED";
  def kirk_review($pass; $package; $findings; $head_marker):
    [.comments[] | select(
      .author.login == "KirkDiggler"
      and (.body | contains($pass))
      and (.body | contains($package))
      and (.body | contains($findings))
      and (.body | contains($head_marker + $head_sha))
      and (.body | endswith($signature))
    )] | length == 1;
  .title == "feat: support native Ubuntu toolkit contributor sandbox"
  and .state == "MERGED" and .baseRefName == "main" and .headRefName == $head
  and .headRefOid == $head_sha
  and (.mergeCommit.oid|type == "string" and length == 40)
  and ([.files[].path] | sort == ["README.md","docs/toolkit-contributor-sandbox.md","scripts/toolkit-contributor.sh","tests/toolkit-contributor-contract.sh"])
  and ([.statusCheckRollup[]? | select(good_check | not)] | length == 0)
  and kirk_review("native-ubuntu-spec-review: PASS"; "native-ubuntu-spec-review-package: COMPLETE"; "native-ubuntu-spec-review-findings: none"; "native-ubuntu-spec-reviewed-head: ")
  and kirk_review("native-ubuntu-shell-review: PASS"; "native-ubuntu-shell-review-package: COMPLETE"; "native-ubuntu-shell-review-findings: none"; "native-ubuntu-shell-reviewed-head: ")
' "$sdd_root/task5-pr-readback.json"
closing_query='query($owner:String!, $repo:String!, $number:Int!) { repository(owner:$owner,name:$repo) { pullRequest(number:$number) { closingIssuesReferences(first:20) { nodes { number repository { nameWithOwner } } } } } }'
gh api graphql -f query="$closing_query" -F owner=KirkDiggler -F repo=game-dev -F number="$native_game_dev_pr" \
  | jq -e --argjson issue "$game_dev_issue" '
      .data.repository.pullRequest.closingIssuesReferences.nodes as $refs
      | ($refs|length == 1) and $refs[0].number == $issue and $refs[0].repository.nameWithOwner == "KirkDiggler/game-dev"
    '
git -C "$HOME/game-dev" fetch origin
git -C "$HOME/game-dev" merge-base --is-ancestor "$native_game_dev_merge_sha" origin/main

gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=game-dev -F number="$game_dev_issue" \
  > "$sdd_root/task5-game-dev-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number="$native_verify_issue" \
  > "$sdd_root/task5-native-verify-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=210 \
  > "$sdd_root/task5-wsl-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=211 \
  > "$sdd_root/task5-211-project.json"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=208 \
  > "$sdd_root/task5-208-project.json"
jq -e '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ($rows[0] | has_field("Status"; "Done") and has_field("Team"; "Platform") and has_field("Feature"; "Infra") and has_field("Kind"; "Build"))
' "$sdd_root/task5-game-dev-project.json"
jq -e '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ($rows[0] | has_field("Status"; "Done") and has_field("Team"; "Platform") and has_field("Feature"; "Infra") and has_field("Kind"; "Verify"))
' "$sdd_root/task5-native-verify-project.json"
jq -e '
  def has_field($field; $value): [.fieldValues.nodes[] | select(.field.name == $field) | .name] == [$value];
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ($rows[0] | has_field("Status"; "Done") and has_field("Team"; "Cross-team") and has_field("Feature"; "Infra") and has_field("Kind"; "Verify"))
' "$sdd_root/task5-wsl-project.json"
jq -e '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] == ["Done"])
' "$sdd_root/task5-211-project.json"
jq -e '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1)
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] | length == 1)
' "$sdd_root/task5-208-project.json"
```

Re-run parent/sub-issue reads instead of relying on Task 1 output:

```bash
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=rpg-project -F number=208 > "$sdd_root/task5-parent-208.json"
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=game-dev -F number="$game_dev_issue" > "$sdd_root/task5-game-parent.json"
gh api graphql -f query="$parent_query" -F owner=KirkDiggler -F repo=rpg-project -F number="$native_verify_issue" > "$sdd_root/task5-native-parent.json"
jq -e --arg game_url "$(jq -r '.[0].url' "$sdd_root/task5-game-dev-issue.json")" --arg native_url "$native_verify_url" '
  [.data.repository.issue.subIssues.nodes[] | .url] as $children
  | ($children | index($game_url) != null) and ($children | index($native_url) != null)
' "$sdd_root/task5-parent-208.json"
jq -e '.data.repository.issue.parent.number == 208 and .data.repository.issue.parent.repository.nameWithOwner == "KirkDiggler/rpg-project"' "$sdd_root/task5-game-parent.json"
jq -e '.data.repository.issue.parent.number == 208 and .data.repository.issue.parent.repository.nameWithOwner == "KirkDiggler/rpg-project"' "$sdd_root/task5-native-parent.json"
```

#210 is intentionally not replaced by a new child record.

- [ ] **3. Post the signed parent summary, edit only #208 Status, read it back, and close #208 explicitly.** This command sequence preserves the non-default closure semantics: #211 is a design decision closure, the game-dev issue closed solely through its one PR reference, native/#210 closed only after independent evidence reviews, and #208 has no closing keyword in its summary.

```bash
parent_marker='<!-- native-ubuntu-delivery:task5-parent-summary -->'
printf '%s\n%s\n\n%s\n' "$parent_marker" \
  "Native Ubuntu delivery is complete: tracking design/plan PR #$tracking_pr, original rpg-project#209 at 22aee544a42906c2f8c01a0e1eb4935c252dcda2, game-dev#60 at 1df0212e5a04374ea83b9af1dd81d09a8a55831a, rpg-api#792 at 9099953f9bc86efbed9bf62209a96c54d9383d6b on dev, rpg-dnd5e-web#747 at cfa63138a1f06de65991c31b006f29fc2af1ad74 on dev, and native game-dev#$native_game_dev_pr at $native_game_dev_merge_sha. The game-dev issue, native verification $native_verify_url, and #210 have accepted signed reviews/evidence. No API, web, toolkit, proto, deployment, or compose code changed in the native wave. Both clean runs proved main/dev/dev/main bootstrap, API/web dev-clone ancestry, marker restoration, immediate MCP browser evidence, negatives, and owned cleanup." \
  "$signature" > "$sdd_root/task5-parent-summary.md"
gh issue comment 208 --repo KirkDiggler/rpg-project --body-file "$sdd_root/task5-parent-summary.md"
gh issue view 208 --repo KirkDiggler/rpg-project --json comments \
| jq -e --arg marker "$parent_marker" --arg sig "$signature" '[.comments[] | select((.body|contains($marker)) and (.body|endswith($sig))] | length == 1'

gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=208 \
  > "$sdd_root/task5-parent-project-before.json"
parent_item="$(jq -er '[.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] | if length == 1 then .[0].id else error("expected one Project 19 item") end' "$sdd_root/task5-parent-project-before.json")"
jq -e --arg item "$parent_item" '
  [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)] as $rows
  | ($rows | length == 1) and $rows[0].id == $item
    and ([ $rows[0].fieldValues.nodes[] | select(.field.name == "Status") | .name ] | length == 1)
' "$sdd_root/task5-parent-project-before.json"
gh project item-edit --project-id "$project_id" --id "$parent_item" --field-id "$status_field" --single-select-option-id "$status_done"
gh api graphql -f query="$issue_project_query" -F owner=KirkDiggler -F repo=rpg-project -F number=208 \
  > "$sdd_root/task5-parent-project-after.json"
jq -s -e --arg item "$parent_item" '
  def rows: [.data.repository.issue.projectItems.nodes[] | select(.project.number == 19)];
  def non_status: [.fieldValues.nodes[] | select(.field.name != "Status") | {field:.field.name,name:.name}] | sort_by(.field);
  (.[0] | rows) as $before
  | (.[1] | rows) as $after
  | ($before | length == 1) and ($after | length == 1)
    and $before[0].id == $item and $after[0].id == $item
    and ($before[0] | non_status) == ($after[0] | non_status)
    and ([ $after[0].fieldValues.nodes[] | select(.field.name == "Status") | .name] == ["Done"])
' "$sdd_root/task5-parent-project-before.json" "$sdd_root/task5-parent-project-after.json"
gh issue close 208 --repo KirkDiggler/rpg-project
gh issue view 208 --repo KirkDiggler/rpg-project --json state \
| jq -e '.state == "CLOSED"'

```

## Review packages and retained evidence

| Gate               | Required retained evidence                                                                                                                                                                                                                      | Independent decision                         |
| ------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| Tracking/process   | exact #211 PR identity, two-file list, review verdict/check conclusions, merged blobs, original PR branch/SHA reads, issue/Project/parent JSON                                                                                                  | Coordinator before Task 2                    |
| TDD/implementation | RED result, GREEN commands, fake logs, four-file diff, clone/preservation tests, PR metadata/reviews/checks/closing count                                                                                                                       | Specification/safety and shell/TDD reviewers |
| Native live        | host/Docker/SSH/ports, clean branch/origin/ancestry proof, focused reports/packages, exact seed and marker records, PutDungeon transcript, six MCP screenshots, negatives, down, archive/checksum, eight attachment URLs, and viewed statements | Fresh reviewer other than executor           |
| WSL2 live          | latest GitHub packet, GitHub-derived baseline, same clean clone/marker/MCP evidence, exact seed records, archive/checksum, eight attachment URLs, viewed statements, and field preservation                                                     | Fresh reviewer other than executor           |
| Parent closure     | exact states/fields/links/PR facts/merge ancestry/comment signatures and retained evidence URLs                                                                                                                                                 | Coordinator before #208 close                |

## Plan self-review requirements

Before committing this plan and before Task 1 begins, run these commands from
`rpg-project`. They validate the two documents without reading another checkout.
The scanner permits intentional HTML comment markers, rejects unresolved work
markers, checks fence pairing, and extracts every fenced Bash block for a real
`bash -n` pass.

```bash
python3 - <<'PY'
from pathlib import Path
import re
import subprocess
import tempfile

paths = [
    Path('ideas/toolkit-contributor-native-ubuntu/design.md'),
    Path('ideas/toolkit-contributor-native-ubuntu/plan.md'),
]
tokens = ('T' + 'BD', 'TO' + 'DO', 'FIX' + 'ME', 'PLACE' + 'HOLDER')
opening = re.compile(r'^\s*(?P<fence>`{3,}|~{3,})(?P<info>[^\n]*)$')
heading_re = re.compile(r'^#{1,6}\s+(.+)$')
all_bash_blocks = []
for path in paths:
    lines = path.read_text().splitlines()
    headings = set()
    task_headings = []
    active = None
    bash_lines = []
    for line_number, line in enumerate(lines, 1):
        if active is None:
            match = opening.match(line)
            if match:
                active = (match.group('fence')[0], len(match.group('fence')), match.group('info').strip())
                bash_lines = []
                continue
            match = heading_re.match(line)
            if match:
                heading = match.group(1)
                if heading in headings:
                    raise SystemExit(f'{path}:{line_number}: duplicate heading: {heading}')
                headings.add(heading)
                if re.fullmatch(r'Task [1-5]:.*', heading):
                    task_headings.append(heading)
            continue
        fence_char, fence_len, info = active
        if re.match(rf'^\s*{re.escape(fence_char)}{{{fence_len},}}\s*$', line):
            if info.split(maxsplit=1)[0:1] == ['bash']:
                all_bash_blocks.append((path, line_number, '\n'.join(bash_lines) + '\n'))
            active = None
            continue
        bash_lines.append(line)
    if active is not None:
        raise SystemExit(f'{path}: unbalanced fenced code block')
    if path.name == 'plan.md':
        expected = {1, 2, 3, 4, 5}
        found = {int(re.match(r'Task ([1-5]):', heading).group(1)) for heading in task_headings}
        if len(task_headings) != 5 or found != expected:
            raise SystemExit(f'{path}: require exactly five unique Task 1-5 headings, found {task_headings}')
    text = path.read_text()
    bad_tokens = [token for token in tokens if token in text]
    if bad_tokens:
        raise SystemExit(f'{path}: unresolved marker(s): {bad_tokens}')
    legacy_sdd = '.pi-agents/' + 'sdd'
    legacy_gate = 'APPRO' + 'VED'
    legacy_items_scan = 'items(first:' + '100)'
    legacy_query = 'project_' + 'items_query'
    if legacy_sdd in text or legacy_gate in text:
        raise SystemExit(f'{path}: prohibited legacy gate/path')
    if legacy_items_scan in text or legacy_query in text:
        raise SystemExit(f'{path}: legacy Project scan/query remains')
    if re.search(r'gh\s+issue\s+view[^\n]*closedByPullRequestsReferences', text):
        raise SystemExit(f'{path}: unsupported gh issue closedBy field remains')
with tempfile.TemporaryDirectory() as tmp:
    for index, (path, line_number, body) in enumerate(all_bash_blocks, 1):
        script = Path(tmp) / f'{path.stem}-{index}.sh'
        script.write_text(body)
        subprocess.run(['bash', '-n', str(script)], check=True)
print(f'checked {len(all_bash_blocks)} fenced Bash blocks')
PY
npx prettier --write ideas/toolkit-contributor-native-ubuntu/design.md \
  ideas/toolkit-contributor-native-ubuntu/plan.md
first_hashes="$(sha256sum ideas/toolkit-contributor-native-ubuntu/design.md ideas/toolkit-contributor-native-ubuntu/plan.md)"
npx prettier --write ideas/toolkit-contributor-native-ubuntu/design.md \
  ideas/toolkit-contributor-native-ubuntu/plan.md
test "$first_hashes" = "$(sha256sum ideas/toolkit-contributor-native-ubuntu/design.md ideas/toolkit-contributor-native-ubuntu/plan.md)"
npx prettier --check ideas/toolkit-contributor-native-ubuntu/design.md \
  ideas/toolkit-contributor-native-ubuntu/plan.md
git diff --check
test "$(git diff --name-only | sort)" = $'ideas/toolkit-contributor-native-ubuntu/design.md\nideas/toolkit-contributor-native-ubuntu/plan.md'
test -z "$(git diff --cached --name-only)"
```

Expected: the scanner and every extracted Bash block pass; the first Prettier
write may normalize the documents, the second is idempotent, and the Prettier
check and whitespace check pass. The only changed paths are
`ideas/toolkit-contributor-native-ubuntu/design.md` and
`ideas/toolkit-contributor-native-ubuntu/plan.md`; no paths are staged before
the documentation commit. Commit exactly those two files, without an amend:

```bash
git add ideas/toolkit-contributor-native-ubuntu/design.md \
  ideas/toolkit-contributor-native-ubuntu/plan.md
git diff --cached --check
test "$(git diff --cached --name-only | sort)" = $'ideas/toolkit-contributor-native-ubuntu/design.md\nideas/toolkit-contributor-native-ubuntu/plan.md'
git commit -m 'docs: make native Ubuntu delivery plan executable'
git diff --cached --name-only
git status --short
```
