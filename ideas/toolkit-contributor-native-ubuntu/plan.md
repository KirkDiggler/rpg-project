# Native Ubuntu toolkit contributor sandbox implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (preferred) or `superpowers:executing-plans` to execute this plan one task at a time. Keep the checkbox state truthful; do not start a later task while its listed gate is unresolved.

**Goal:** Land one additive `game-dev` change that makes the existing six-command toolkit contributor facade explicitly support both native Ubuntu and Ubuntu WSL2, then independently prove the full real loop on clean native Ubuntu and clean WSL2 without changing any API, web, toolkit, proto, or deployment behavior.

**Architecture:** `scripts/toolkit-contributor.sh` remains the sole facade. Every valid command first reads the literal host files, passes their captured text to a pure classifier, and either gets `ubuntu-native` / `ubuntu-wsl2` or refuses before repository or runtime work. The existing checkout, one-module override, local image, three-compose-overlay, Envoy health, RPC seeding, and owned-cleanup paths remain a single shared path for both modes. Only diagnostics and `status`'s one additional host-mode line vary by classified mode.

**Tech Stack:** Bash with fixture executables, existing Bash contract tests, Docker-compatible daemon through the existing `docker` CLI, Docker Compose, Go sandbox seeder through Envoy at `localhost:8080`, Vite on `localhost:3001`, Markdown runbook/README, GitHub Project 19.

## Global constraints

- This plan is the approval artifact for [rpg-project#211](https://github.com/KirkDiggler/rpg-project/issues/211). This plan commit creates no delivery issue, branch, worktree, PR, source change, stack, browser run, or GitHub mutation. **Task 1 starts only after the rpg-project tracking PR containing this design and this plan is merged to `origin/main`.**
- The original sandbox is a dependency, not scope to revisit: [rpg-project PR #209](https://github.com/KirkDiggler/rpg-project/pull/209) merged at `22aee544a42906c2f8c01a0e1eb4935c252dcda2`; [rpg-api PR #792](https://github.com/KirkDiggler/rpg-api/pull/792) merged at `9099953f9bc86efbed9bf62209a96c54d9383d6b`; [game-dev PR #60](https://github.com/KirkDiggler/game-dev/pull/60) merged at `1df0212e5a04374ea83b9af1dd81d09a8a55831a`; [rpg-dnd5e-web PR #747](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/747) merged at `cfa63138a1f06de65991c31b006f29fc2af1ad74`.
- Work is one bounded `game-dev` issue/branch/PR. No rpg-api, rpg-dnd5e-web, rpg-toolkit, rpg-api-protos, rpg-deployment, compose, API, web, seed-RPC, or protocol change is authorized. The temporary marker is an acceptance-only uncommitted edit in a disposable toolkit checkout.
- The user-visible command set remains exactly `bootstrap`, `start`, `refresh`, `seed`, `status`, and `down`. Do not add a host command, mode flag, second native script, installer, daemon/vendor/context selector, watcher, retry/cleanup framework, port manager, alternate-port path, direct-storage path, or process killer.
- Production host observation is hard-coded: it reads **only** `/etc/os-release` and `/proc/sys/kernel/osrelease`, on every valid command. It does not honor `TOOLKIT_CONTRIBUTOR_OSRELEASE`, `TOOLKIT_CONTRIBUTOR_OS_RELEASE`, `OS_RELEASE_PATH`, `HOST_MODE`, a config file, a flag, Docker context, or any other host-selection input. Tests may feed captured strings to the pure classifier or replace `cat` in their fixture `PATH`; they must not add a production bypass.
- The host gate occurs after exact argument validation and before `git`, Docker, API-helper, Go, compose, seed, checkout, clone, build, or mutation activity. `bootstrap`, `start`, `refresh`, and `down` additionally run `command -v docker` and `docker info` before their first mutation. `seed` and `status` intentionally do not add a Docker probe.
- The existing contracts are preservation gates: four exact clone roots and origins; no checkout refresh/reset/clean/stash/switch; dirty preservation; exactly one D&D 5e override owned by the API helper; `Dockerfile.local-toolkit`; compose files in `local-dev → api → local-api-src` order; bounded Envoy health; one API RPC seeder; normal `GameView`; and `down` cleanup of only its owned tree.
- The facade never invokes `ss`, `kill`, `pkill`, `fuser`, `lsof`, a container sweep, or generic port remediation. `ss -ltnp` is a manual operator observation only. An occupied, hidden, or failed-to-bind listener blocks acceptance; it is never killed, reused, moved, or worked around.
- Every GitHub comment made by these future tasks ends exactly `— asset-pipeline agent, on behalf of KirkDiggler`. Issue/PR bodies may refer to dependencies but contain no closing keyword except the implementation PR's one `Closes #` line for its own game-dev issue.
- Preserve branches and worktrees after merge and after verification. Do not use `git worktree remove`, `git branch -D`, `git clean`, or `rm -rf` as cleanup. The clean acceptance directory is deliberately retained with its logs/screenshots until acceptance is recorded.

## Source map and observed seams

| Repository/ref or source                                                | Observed fact                                                                                                                                                                                  | Consequence for this plan                                                                                                                                                      |
| ----------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `rpg-project` PR #209, `22aee544a42906c2f8c01a0e1eb4935c252dcda2`       | Merged original design and plan define the six commands, API helper, Envoy seeder, marker, browser route, and acceptance sequence.                                                             | Native support extends the original loop; it does not recreate Units A–C.                                                                                                      |
| `game-dev` PR #60, `1df0212e5a04374ea83b9af1dd81d09a8a55831a`           | The original facade release is the required baseline for this change.                                                                                                                          | Task 1 refuses to create a worktree until freshly fetched `origin/main` contains this merge.                                                                                   |
| `game-dev/scripts/toolkit-contributor.sh`                               | Current `require_wsl2` reads only `/proc/sys/kernel/osrelease`; `require_tools` includes Docker; `bootstrap` runs `docker info`; runtime commands do not presently host- or daemon-gate first. | Replace the WSL-only gate with a literal-file reader plus pure classifier and move mode-aware Docker preflight before each mutating path.                                      |
| `game-dev/tests/toolkit-contributor-contract.sh`                        | A hermetic copied-script fixture already records fake Git/Docker/Go/compose order, health retries, origins, dirty preservation, status, and `down`.                                            | Extend this one contract file with direct classifier fixtures, literal-reader fixtures, refusal ordering, Docker pre-mutation, status-host-line, and documentation assertions. |
| `game-dev/docs/toolkit-contributor-sandbox.md` and `game-dev/README.md` | The runbook currently promises Ubuntu WSL2 plus Docker Desktop; README is a short pointer.                                                                                                     | One runbook describes both supported modes; README remains a concise pointer rather than a second bootstrap.                                                                   |
| `rpg-deployment/docker-compose.local-dev.yml`                           | Host publications are `80:80` (`nginx-local`), `3002:3000` (`dnd-api`), `6380:6379` (`redis`), and `8080:8080` (`envoy`).                                                                      | Acceptance manually observes 80, 3002, 6380, and 8080 before start. No compose change is made.                                                                                 |
| `rpg-dnd5e-web/vite.config.ts`                                          | The existing Vite server port is `3001`.                                                                                                                                                       | Acceptance also observes port 3001 before it starts the owned Vite process and rejects an alternate URL.                                                                       |
| `rpg-api` PR #792 and `rpg-dnd5e-web` PR #747                           | Their provider/sandbox interfaces are already merged at the SHAs above.                                                                                                                        | Focused checks and live proof consume those interfaces verbatim; no new provider work is created.                                                                              |

## File map

| Repository    | Path                                              | Change                              | Responsibility                                                                                                   |
| ------------- | ------------------------------------------------- | ----------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `rpg-project` | `ideas/toolkit-contributor-native-ubuntu/plan.md` | Create in this planning commit only | Approved execution record, delivery gates, and verification packets.                                             |
| `game-dev`    | `scripts/toolkit-contributor.sh`                  | Modify in Task 2                    | Literal host reader, pure classifier, mode-aware mutating-command Docker preflight, and status host-mode output. |
| `game-dev`    | `tests/toolkit-contributor-contract.sh`           | Modify in Task 2                    | Direct classifier and all-command fixture coverage while retaining prior facade assertions.                      |
| `game-dev`    | `docs/toolkit-contributor-sandbox.md`             | Modify in Task 2                    | One native-Ubuntu/WSL2 runbook, literal observations, mode-specific daemon guidance, and port/no-auto-kill rule. |
| `game-dev`    | `README.md`                                       | Modify in Task 2                    | Concise link that names both supported Ubuntu modes.                                                             |

No file is added in `game-dev`; no production file outside the four listed `game-dev` paths changes.

## Exact interfaces and invariants

### Host-classifier seam

The production script exposes this internal Bash seam for direct contract tests; it is not a seventh command and is not advertised by `usage`:

```bash
classify_toolkit_contributor_host "$os_release_text" "$kernel_release_text"
```

It writes exactly one successful mode token to stdout and returns zero:

```text
ubuntu-native
ubuntu-wsl2
```

On refusal it returns nonzero, writes no mode token, and emits an actionable diagnostic through the caller. It is pure: no file reads, subprocesses, Docker calls, environment selection, checkout inspection, or mutation. The script's final command dispatch is guarded by `[[ "${BASH_SOURCE[0]}" == "$0" ]]` so the contract may source only this seam; executing the script still accepts only the six listed commands.

The production-only reader has no arguments and sets the validated mode used by its caller:

```bash
read_toolkit_contributor_host
# sets TOOLKIT_CONTRIBUTOR_HOST_MODE to ubuntu-native or ubuntu-wsl2
```

It invokes literal reads of `/etc/os-release` and `/proc/sys/kernel/osrelease` every time it is called, then calls the pure seam with their text. It rejects a read failure or empty/whitespace-only kernel capture. It does not accept a path or a mode parameter.

`/etc/os-release` is parsed as text, never sourced or evaluated. Ignore blank/comment lines. Require exactly one `ID=` assignment whose whole value is exactly `ubuntu`, `'ubuntu'`, or `"ubuntu"`; reject missing, empty, duplicate, malformed, or another ID. An `ID =ubuntu`, trailing text, unmatched quote, or a second `ID=` is malformed. Other `os-release` keys do not matter.

Lowercase the captured kernel release only for marker comparison, then apply this exact precedence after the Ubuntu-ID and nonempty-kernel checks:

| Lowercased kernel content               | Classification     |
| --------------------------------------- | ------------------ |
| contains `wsl2` or `microsoft-standard` | `ubuntu-wsl2`      |
| otherwise contains `microsoft`          | refuse Ubuntu WSL1 |
| contains neither `microsoft` nor `wsl2` | `ubuntu-native`    |

Thus `4.19.128-microsoft-standard` is WSL2, `4.4.0-19041-Microsoft` is rejected WSL1, and a generic native Ubuntu release is native. No kernel-version, vendor, desktop, init-system, Docker-context, or environment allowlist exists.

### Gate and diagnostic contract

After argument count validation, each command calls `read_toolkit_contributor_host` before every existing action. `status` prints the exact successful line immediately after that gate and before helper-status or Envoy-health activity:

```text
host mode: ubuntu-native
```

or

```text
host mode: ubuntu-wsl2
```

For `bootstrap`, `start`, `refresh`, and `down`, the next common prelude is mode-aware Docker reachability:

```bash
require_toolkit_contributor_docker "$TOOLKIT_CONTRIBUTOR_HOST_MODE"
```

It first checks `command -v docker`, then runs `docker info >/dev/null`. Native missing-CLI/daemon diagnostics say that a Docker CLI and reachable Docker-compatible daemon are required and that Docker Engine, rootless Docker, or another compatible daemon is acceptable; they must not prescribe Docker Desktop. WSL2 diagnostics say that a Docker CLI and reachable compatible daemon are required and may direct the contributor to Docker Desktop WSL integration or another reachable compatible daemon; they must not change Windows or Docker Desktop. Neither diagnostic installs/configures anything.

The required successful ordering is:

| Command     | Required order after argument validation                                                                                                                                    |
| ----------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `bootstrap` | host gate → Docker CLI → `docker info` → remaining tool checks → GitHub SSH → four-root checks/clones                                                                       |
| `start`     | host gate → Docker CLI → `docker info` → existing runtime-layout checks → override `on` → image build → compose `up -d` → existing Envoy health loop                        |
| `refresh`   | host gate → Docker CLI → `docker info` → existing runtime-layout checks → override `refresh` → image build → compose `up -d --no-deps rpg-api` → existing Envoy health loop |
| `down`      | host gate → Docker CLI → `docker info` → existing runtime-layout checks → compose `down` → override `off`                                                                   |
| `seed`      | host gate → existing runtime-layout checks → one existing non-health seeder invocation                                                                                      |
| `status`    | host gate → print host-mode line → existing runtime-layout checks → one helper `status` → one Envoy health invocation                                                       |

`seed` adds no health poll. `status` adds no Docker probe or arbitrary container listing. Its existing helper and Envoy results remain authoritative; the host-mode line is not a health claim.

### Preserved facade calls

These calls and their order are unchanged after the new gates succeed:

```bash
"$ROOT/rpg-api/scripts/toolkit-local-override.sh" on --target rulebooks/dnd5e --src "$ROOT/rpg-toolkit"
docker build -f "$ROOT/rpg-api/Dockerfile.local-toolkit" -t rpg-api:local "$ROOT/rpg-api"
(
  cd "$ROOT/rpg-deployment"
  docker compose \
    -f docker-compose.local-dev.yml \
    -f docker-compose.api.yml \
    -f docker-compose.local-api-src.yml \
    up -d
)
(
  cd "$ROOT/rpg-api"
  go run ./cmd/sandboxseed --address localhost:8080 --health
)
(
  cd "$ROOT/rpg-api"
  go run ./cmd/sandboxseed --address localhost:8080
)
```

`refresh` retains `up -d --no-deps rpg-api`; `down` retains the three-file `docker compose ... down` followed by helper `off`. No command path selects a different module, image, compose file, endpoint, identity, seed sequence, or port by mode.

## Delivery control and records

Project 19 is `KirkDiggler`'s **The Dungeon Run** board. Its exact selection values for this delivery are:

| Record                | Repository                    | Exact title                                                     | Team                      | Feature              | Kind                  | Initial Status                     | Relationship                                                                     |
| --------------------- | ----------------------------- | --------------------------------------------------------------- | ------------------------- | -------------------- | --------------------- | ---------------------------------- | -------------------------------------------------------------------------------- |
| Native implementation | `KirkDiggler/game-dev`        | `Add native Ubuntu host support to toolkit contributor sandbox` | `Platform`                | `Infra`              | `Build`               | `Todo`                             | Child of #208; body links #208, #211, #209, #60, #792, and #747.                 |
| Native acceptance     | `KirkDiggler/rpg-project`     | `Verify toolkit contributor sandbox on clean native Ubuntu`     | `Platform`                | `Infra`              | `Verify`              | `Todo`                             | Child of #208; body links #208, #211, #209, #60, #792, and #747.                 |
| Existing WSL evidence | `KirkDiggler/rpg-project#210` | `Verify the toolkit contributor sandbox on clean WSL2`          | **preserve `Cross-team`** | **preserve `Infra`** | **preserve `Verify`** | `In Progress` until Task 4 success | Remains the one and only WSL evidence record; do not duplicate or reclassify it. |

The two new child records can have one GitHub parent only, so they are formal sub-issues of umbrella #208. Their bodies and #211 handoff comment carry the explicit #211 design-decision link; this is the required visible connection to both #208 and #211 without inventing a second parent relationship.

The future game-dev issue number is deliberately not guessed. Task 1 captures it in `game_dev_issue` and uses it everywhere thereafter:

```bash
game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
```

The implementation PR target is `main`, its title is `feat: support native Ubuntu toolkit contributor sandbox`, and it has exactly one closing line, `Closes #${game_dev_issue}`. The native-verification issue number is captured in `native_verify_issue`; it owns no branch or PR.

## Tasks

### Task 1: Gate the merged design/plan, create exact delivery records, and prepare the one isolated implementation worktree

**Files:** none changed.
**Produces:** the exact two delivery issue URLs/numbers, Project 19 memberships/fields, parent links, deterministic game-dev branch/worktree names, and a fresh worktree from `origin/main` that includes original game-dev PR #60.
**Stop point:** do not create either issue, branch, or worktree if the merged plan/design or original release ancestry checks fail.

- [ ] **1. Verify the plan merge and original release dependencies before mutation.** From the physical rpg-project primary checkout, fetch only remote refs, discover the actually merged plan/design commits from `origin/main`, and refuse if either is absent or if the original dependencies are not the recorded SHAs.

  ```bash
  set -euo pipefail
  project_root="$HOME/game-dev/rpg-project"
  git -C "$project_root" fetch origin
  native_plan_commit="$(git -C "$project_root" log -1 --format=%H origin/main -- ideas/toolkit-contributor-native-ubuntu/plan.md)"
  native_design_commit="$(git -C "$project_root" log -1 --format=%H origin/main -- ideas/toolkit-contributor-native-ubuntu/design.md)"
  test -n "$native_plan_commit"
  test -n "$native_design_commit"
  git -C "$project_root" merge-base --is-ancestor "$native_plan_commit" origin/main
  git -C "$project_root" merge-base --is-ancestor "$native_design_commit" origin/main
  git -C "$project_root" show "$native_plan_commit:ideas/toolkit-contributor-native-ubuntu/plan.md" | \
    grep -Fqx '> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (preferred) or `superpowers:executing-plans` to execute this plan one task at a time. Keep the checkbox state truthful; do not start a later task while its listed gate is unresolved.'
  git -C "$project_root" merge-base --is-ancestor 22aee544a42906c2f8c01a0e1eb4935c252dcda2 origin/main

  git -C "$HOME/game-dev" fetch origin
  git -C "$HOME/game-dev" merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a origin/main
  gh pr view 209 --repo KirkDiggler/rpg-project --json state,mergeCommit --jq '.state + " " + .mergeCommit.oid'
  gh pr view 60 --repo KirkDiggler/game-dev --json state,mergeCommit --jq '.state + " " + .mergeCommit.oid'
  gh pr view 792 --repo KirkDiggler/rpg-api --json state,mergeCommit --jq '.state + " " + .mergeCommit.oid'
  gh pr view 747 --repo KirkDiggler/rpg-dnd5e-web --json state,mergeCommit --jq '.state + " " + .mergeCommit.oid'
  ```

  **Expected:** every command exits `0`; the four PR reads print `MERGED` and exactly the SHAs in the dependency list. The plan/design are now in `origin/main`, and fresh game-dev `origin/main` contains PR #60. A missing SHA, non-merged tracking artifact, or stale `origin/main` is a hard stop, not a reason to branch from a local checkout or change another repository.

- [ ] **2. Create the two exact issue bodies, record their real numbers, make each a #208 child, and set Project 19 fields.** Write the following two `pih-dispatch:v1` bodies verbatim except for the shell-expanded links/numbers. They must contain no closing keyword.

  ```bash
  set -euo pipefail
  existing_game_dev="$(gh issue list --repo KirkDiggler/game-dev --state all --search 'in:title "Add native Ubuntu host support to toolkit contributor sandbox"' --json number,title --jq '.[] | select(.title == "Add native Ubuntu host support to toolkit contributor sandbox") | .number')"
  existing_native_verify="$(gh issue list --repo KirkDiggler/rpg-project --state all --search 'in:title "Verify toolkit contributor sandbox on clean native Ubuntu"' --json number,title --jq '.[] | select(.title == "Verify toolkit contributor sandbox on clean native Ubuntu") | .number')"
  test -z "$existing_game_dev"
  test -z "$existing_native_verify"
  dispatch_dir="$(mktemp -d)"
  trap 'rm -f "$dispatch_dir/game-dev.md" "$dispatch_dir/native-verify.md" "$dispatch_dir/project-19.json"; rmdir "$dispatch_dir" 2>/dev/null || true' EXIT

  cat > "$dispatch_dir/game-dev.md" <<'EOF'
  <!-- pih-dispatch:v1 -->
  ## Goal / symptom
  Extend the one landed toolkit contributor facade so native Ubuntu and Ubuntu WSL2 are explicit supported hosts without changing the established sandbox loop.

  ## Desired outcome
  `bootstrap`, `start`, `refresh`, `seed`, `status`, and `down` classify only `ubuntu-native` or `ubuntu-wsl2`; native Ubuntu accepts any working Docker-compatible daemon reached by `docker`, WSL2 retains actionable Docker Desktop WSL-integration guidance, and `status` reports the selected mode.

  ## Contract boundaries
  - Change only `scripts/toolkit-contributor.sh`, `tests/toolkit-contributor-contract.sh`, `docs/toolkit-contributor-sandbox.md`, and `README.md`.
  - Read literal `/etc/os-release` and `/proc/sys/kernel/osrelease` for every command. Parse only one data `ID=ubuntu`; WSL2 is a case-insensitive `wsl2` or `microsoft-standard` marker; Microsoft without either is unsupported WSL1; non-Microsoft Ubuntu is native. No host/path/mode environment override, flag, config, Docker-context/vendor policy, or second script is allowed.
  - The host gate precedes every command action. Docker CLI and `docker info` precede bootstrap/start/refresh/down mutations with mode-specific diagnostics. Seed remains one Envoy seeder call; status remains helper status plus one Envoy health call and adds only its host line.
  - Preserve the exact roots/origins/dirty behavior, D&D 5e override ownership, local Dockerfile, three compose overlays, health polling, RPC seed flow, normal GameView, and owned down cleanup.
  - Do not modify API, web, toolkit, proto, deployment, compose, ports, credentials, Docker/Windows setup, or global tools. Do not add a process killer, port manager, alternate port, watcher, installer, direct storage, or new command.

  ## Acceptance
  - Focused contract coverage proves native, normal and legacy WSL2, WSL1 refusal, Debian/non-Ubuntu refusal, malformed/missing observations, no environment bypass, every-command host ordering, mode-specific Docker fail-closed ordering, seed/status preservation, and documentation terms.
  - Existing contributor and bootstrap contracts remain green; the facade accepts both modes but changes no existing sandbox behavior after the new gates.
  - Required review reports no blocker before merge. The PR targets main and contains exactly one `Closes #` reference to this issue.

  ## Verification evidence
  Record TDD RED/GREEN command results, the fake-command ordering log, shell syntax, both contract suites, documentation assertions, `git diff --check`, the four-file diff, review comments, CI, and exact merge SHA. Native live acceptance is performed by its separate rpg-project verification issue only after this PR merges.

  ## Related / dependencies
  Umbrella https://github.com/KirkDiggler/rpg-project/issues/208
  Native design decision https://github.com/KirkDiggler/rpg-project/issues/211
  Original design/plan https://github.com/KirkDiggler/rpg-project/pull/209 at 22aee544a42906c2f8c01a0e1eb4935c252dcda2
  Original facade https://github.com/KirkDiggler/game-dev/pull/60 at 1df0212e5a04374ea83b9af1dd81d09a8a55831a
  Original API provider https://github.com/KirkDiggler/rpg-api/pull/792 at 9099953f9bc86efbed9bf62209a96c54d9383d6b
  Original web sandbox https://github.com/KirkDiggler/rpg-dnd5e-web/pull/747 at cfa63138a1f06de65991c31b006f29fc2af1ad74

  — asset-pipeline agent, on behalf of KirkDiggler
  EOF

  cat > "$dispatch_dir/native-verify.md" <<'EOF'
  <!-- pih-dispatch:v1 -->
  ## Goal / symptom
  Repository tests cannot prove the landed host gate and original contributor loop work together on clean native Ubuntu.

  ## Desired outcome
  On a clean supported native-Ubuntu host, independently execute the full existing loop, retain native evidence, and close this issue only after every required command, normal GameView artifact, negative, marker restoration, and owned cleanup succeeds.

  ## Contract boundaries
  - This is rpg-project evidence-only work: no implementation branch, PR, code edit, GitHub implementation issue, deployment change, host-mode bypass, direct storage, harness-only success, automatic listener kill, or alternate port.
  - Run only after the native game-dev PR is merged and its exact merge SHA, the original PR #60/#792/#747 SHAs, and the native `ubuntu-native` status mode are present in the clean clone.
  - The only source mutation is the temporary Human Strength marker in a disposable toolkit clone; restore it, refresh, reseed, and retain the restoration proof before evidence is accepted.
  - Before start, manually observe ports 80, 3001, 3002, 6380, and 8080 with `ss -ltnp`; occupancy or inability to observe blocks acceptance. Never terminate an owner.

  ## Acceptance
  - Run the merged focused API, game-dev, and web gates; bootstrap twice; start, status, seed twice, and prove the original 16 → 17 → 16 refresh marker.
  - Save `toolkit-contributor-sandbox` through PutDungeon; run Fighter, Barbarian, Fighter then Barbarian, and Barbarian then Fighter; retain all six order-qualified normal GameView screenshots.
  - Prove production Dev-header and production sandbox-route refusals, the existing wrong-owner Create/Join integration negative, explicit down, and that only `rpg-api/local-toolkit/rulebooks/dnd5e` is removed.
  - One signed evidence comment and retained attachment manifest are required. Close only after an independent reviewer accepts the complete package.

  ## Verification evidence
  Record exact commits, host files/mode, Docker/SSH/port preflight, command exit codes, Envoy/status/seed output, marker transcript, PutDungeon key, six screenshots, negatives, down/tree result, Vite log, SHA256 manifest, blocker handling, and reviewer decision.

  ## Related / dependencies
  Umbrella https://github.com/KirkDiggler/rpg-project/issues/208
  Native design decision https://github.com/KirkDiggler/rpg-project/issues/211
  Original design/plan https://github.com/KirkDiggler/rpg-project/pull/209 at 22aee544a42906c2f8c01a0e1eb4935c252dcda2
  Original facade https://github.com/KirkDiggler/game-dev/pull/60 at 1df0212e5a04374ea83b9af1dd81d09a8a55831a
  Original API provider https://github.com/KirkDiggler/rpg-api/pull/792 at 9099953f9bc86efbed9bf62209a96c54d9383d6b
  Original web sandbox https://github.com/KirkDiggler/rpg-dnd5e-web/pull/747 at cfa63138a1f06de65991c31b006f29fc2af1ad74

  — asset-pipeline agent, on behalf of KirkDiggler
  EOF

  game_dev_url="$(gh issue create --repo KirkDiggler/game-dev --title 'Add native Ubuntu host support to toolkit contributor sandbox' --body-file "$dispatch_dir/game-dev.md")"
  native_verify_url="$(gh issue create --repo KirkDiggler/rpg-project --title 'Verify toolkit contributor sandbox on clean native Ubuntu' --body-file "$dispatch_dir/native-verify.md")"
  game_dev_issue="${game_dev_url##*/}"
  native_verify_issue="${native_verify_url##*/}"
  case "$game_dev_issue:$native_verify_issue" in
    *[!0-9:]*|:*|*:) printf 'failed to capture created issue numbers\n' >&2; exit 1 ;;
  esac
  printf 'game-dev=%s\nnative-verify=%s\n' "$game_dev_url" "$native_verify_url"

  parent_id="$(gh issue view 208 --repo KirkDiggler/rpg-project --json id --jq .id)"
  game_dev_node_id="$(gh issue view "$game_dev_issue" --repo KirkDiggler/game-dev --json id --jq .id)"
  native_verify_node_id="$(gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json id --jq .id)"
  add_child='mutation($issueId:ID!, $parentIssueId:ID!) { addSubIssue(input:{issueId:$issueId,parentIssueId:$parentIssueId}) { issue { url } } }'
  gh api graphql -f query="$add_child" -F issueId="$game_dev_node_id" -F parentIssueId="$parent_id"
  gh api graphql -f query="$add_child" -F issueId="$native_verify_node_id" -F parentIssueId="$parent_id"

  game_dev_item="$(gh project item-add 19 --owner KirkDiggler --url "$game_dev_url" --format json --jq .id)"
  native_verify_item="$(gh project item-add 19 --owner KirkDiggler --url "$native_verify_url" --format json --jq .id)"
  project_id='PVT_kwHOAASbwc4Bcj4v'
  status_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM'
  feature_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s'
  kind_field='PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w'
  team_field='PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs'
  status_todo='397864df'
  feature_infra='c8ad5032'
  kind_build='ea162471'
  kind_verify='ab287333'
  team_platform='f9c87bc7'
  for item in "$game_dev_item" "$native_verify_item"; do
    gh project item-edit --project-id "$project_id" --id "$item" --field-id "$status_field" --single-select-option-id "$status_todo"
    gh project item-edit --project-id "$project_id" --id "$item" --field-id "$feature_field" --single-select-option-id "$feature_infra"
    gh project item-edit --project-id "$project_id" --id "$item" --field-id "$team_field" --single-select-option-id "$team_platform"
  done
  gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$kind_field" --single-select-option-id "$kind_build"
  gh project item-edit --project-id "$project_id" --id "$native_verify_item" --field-id "$kind_field" --single-select-option-id "$kind_verify"
  ```

  **Expected:** exactly one newly created URL/number is captured for each title; each is a child of #208; both issue bodies link #208 and #211; Project 19 reads `Platform / Infra / Build / Todo` for game-dev and `Platform / Infra / Verify / Todo` for native verification. If a title already exists, stop and reconcile it rather than creating a duplicate.

- [ ] **3. Read back records, preserve #210, update #211's design handoff, then create the deterministic isolated worktree.** First prove field values and parent link. Then leave #210 as `Cross-team / Infra / Verify / In Progress`, post #211's signed handoff, set #211 to `Done`, and close #211 as the completed design-decision record. It is not an implementation or acceptance closure.

  ```bash
  gh issue view "$game_dev_issue" --repo KirkDiggler/game-dev --json number,title,url,body
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json number,title,url,body
  gh issue view 210 --repo KirkDiggler/rpg-project --json state,title,projectItems
  gh project item-list 19 --owner KirkDiggler --limit 500 --format json > "$dispatch_dir/project-19.json"
  jq -e --arg game "$game_dev_url" --arg native "$native_verify_url" '
    [.items[] | select(.content.url == $game or .content.url == $native)] | length == 2
  ' "$dispatch_dir/project-19.json"

  gh issue comment 211 --repo KirkDiggler/rpg-project --body "Native design and plan are merged at ${native_design_commit} and ${native_plan_commit}. Delivery records are ${game_dev_url} and ${native_verify_url}; both are Project 19 Platform/Infra items and formal #208 children. #210 remains the sole WSL2 record with Cross-team/Infra/Verify ownership and is not duplicated. #211 closes as the completed design decision; #208 remains open until the native implementation, native acceptance, and amended #210 WSL2 acceptance are all complete.

  — asset-pipeline agent, on behalf of KirkDiggler"
  gh project item-edit --project-id "$project_id" --id 'PVTI_lAHOAASbwc4Bcj4vzg2H2C0' --field-id "$status_field" --single-select-option-id 'e4d8ce42'
  gh issue close 211 --repo KirkDiggler/rpg-project

  game_dev_branch="feat/${game_dev_issue}-native-ubuntu-toolkit-contributor"
  game_dev_worktree="$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
  test ! -e "$game_dev_worktree"
  git -C "$HOME/game-dev" fetch origin
  git -C "$HOME/game-dev" merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a origin/main
  git -C "$HOME/game-dev" worktree add -b "$game_dev_branch" "$game_dev_worktree" origin/main
  git -C "$game_dev_worktree" status --porcelain=v1
  test -z "$(git -C "$game_dev_worktree" status --porcelain=v1)"
  test "$(git -C "$game_dev_worktree" branch --show-current)" = "$game_dev_branch"
  git -C "$game_dev_worktree" merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a HEAD
  git -C "$game_dev_worktree" rev-parse HEAD
  ```

  **Expected:** #211 is closed only after the merged-plan and delivery-record comment; #208 remains open; #210 remains open and retains Cross-team/Infra/Verify. The new worktree is clean, is exactly `$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}`, branches from freshly fetched `origin/main`, and contains PR #60. No primary worktree status is changed.

- [ ] **4. Move only the implementation record into active work and record the baseline.**

  ```bash
  gh project item-edit --project-id "$project_id" --id "$game_dev_item" --field-id "$status_field" --single-select-option-id 'a434eab1'
  git -C "$game_dev_worktree" rev-parse HEAD | tee "$dispatch_dir/game-dev-origin-main.txt"
  git -C "$game_dev_worktree" log -1 --oneline
  ```

  **Expected:** the game-dev item is `Platform / Infra / Build / In Progress`; native verification remains `Todo`; the baseline file is retained for the implementation report.

### Task 2: TDD the two-mode host gate, documentation, and one game-dev PR

**Files:** modify `scripts/toolkit-contributor.sh`, `tests/toolkit-contributor-contract.sh`, `docs/toolkit-contributor-sandbox.md`, and `README.md` only.
**Consumes:** original PR #60 facade and the existing API/deployment contracts.
**Produces:** a pure host classifier plus literal production reader, mode-aware Docker diagnostics, status mode output, focused evidence, one commit, and one PR to `main`.
**Stop point:** any test that indicates changed clone, override, compose, Envoy, seed, or down semantics blocks merge.

- [ ] **1. Add failing fixture cases before changing production code.** In `tests/toolkit-contributor-contract.sh`, keep the existing fake-command log and add fixture files for the two literal production paths. The fake `cat` must return the selected fixture file only for `/etc/os-release` and `/proc/sys/kernel/osrelease`; it must not inspect a host-mode/path variable used by the production script. Add a direct seam test by sourcing the copied script and calling `classify_toolkit_contributor_host` after the eventual dispatch guard is present.

  Add these exact classifier fixtures/assertions:

  | Fixture              | `/etc/os-release`                              | `/proc/sys/kernel/osrelease`         | Expected                                      |
  | -------------------- | ---------------------------------------------- | ------------------------------------ | --------------------------------------------- |
  | native               | `ID=ubuntu`                                    | `6.8.0-31-generic`                   | stdout `ubuntu-native`, zero                  |
  | normal WSL2          | `ID="ubuntu"`                                  | `5.15.153.1-microsoft-standard-WSL2` | stdout `ubuntu-wsl2`, zero                    |
  | legacy WSL2          | `ID='ubuntu'`                                  | `4.19.128-microsoft-standard`        | stdout `ubuntu-wsl2`, zero                    |
  | WSL1                 | `ID=ubuntu`                                    | `4.4.0-19041-Microsoft`              | nonzero, names WSL1/WSL2/native remedy        |
  | Debian               | `ID=debian`                                    | `6.8.0-generic`                      | nonzero, names Ubuntu-only support            |
  | missing ID           | `NAME="Ubuntu"`                                | `6.8.0-generic`                      | nonzero                                       |
  | empty ID             | `ID=`                                          | `6.8.0-generic`                      | nonzero                                       |
  | duplicate ID         | `ID=ubuntu` followed by `ID=ubuntu`            | `6.8.0-generic`                      | nonzero                                       |
  | malformed ID         | `ID =ubuntu`                                   | `6.8.0-generic`                      | nonzero                                       |
  | empty kernel         | `ID=ubuntu`                                    | empty text                           | nonzero                                       |
  | unreadable OS/kernel | fake `cat` fails for the selected literal path | otherwise valid fixture              | nonzero and names the unreadable literal path |

  Add a no-bypass group that sets each of `TOOLKIT_CONTRIBUTOR_OSRELEASE`, `TOOLKIT_CONTRIBUTOR_OS_RELEASE`, `OS_RELEASE_PATH`, and `HOST_MODE` to values that would claim a good WSL2 host while the literal fake-file fixture is Debian or WSL1. It must still refuse. Assert a native fixture still succeeds while all four variables hold hostile values.

  For each command in `bootstrap start refresh seed status down`, run a WSL1 fixture and separately a Debian fixture. Assert refusal occurs before any logged `git`, `docker`, API helper, `go`, compose, clone, build, checkout, or seed call. Retain the existing unknown-argument/count cases.

  For **both** native and WSL2 fixtures, make fake `docker info` fail and prove:

  - `bootstrap` logs `docker info` but neither `git ls-remote` nor `git clone`;
  - `start` logs it but neither helper `on`, image build, compose, nor health;
  - `refresh` logs it but neither helper `refresh`, image build, compose, nor health;
  - `down` logs it but neither compose `down` nor helper `off`.

  Assert native refusal mentions a reachable Docker-compatible daemon and not Docker Desktop; assert WSL2 refusal names Docker Desktop WSL integration or a reachable compatible daemon. Add missing-`docker` CLI checks with the same mode-specific distinction. Keep the existing successful command-order, retry, clone, origin, dirty, and owned-down cases. Extend successful native and WSL2 status cases to assert one host line before helper status, exactly one health invocation, no Docker `info`, and no container listing; extend seed to assert one non-health seeder call and no health/Docker call.

  Finally make the contract check the documentation literally:

  ```bash
  grep -Fq 'ubuntu-native' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq 'ubuntu-wsl2' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq '/etc/os-release' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq '/proc/sys/kernel/osrelease' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq 'WSL1' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq 'Docker-compatible daemon' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq '80, 3001, 3002, 6380, 8080' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq 'never kills' "$ROOT/docs/toolkit-contributor-sandbox.md"
  grep -Fq 'native Ubuntu and Ubuntu WSL2' "$ROOT/README.md"
  ```

- [ ] **2. Run the RED suite.**

  ```bash
  cd "$game_dev_worktree"
  bash tests/toolkit-contributor-contract.sh
  ```

  **Expected RED result:** nonzero. The current PR #60 script lacks `classify_toolkit_contributor_host`, reads no `/etc/os-release`, treats generic Ubuntu as WSL failure, has no native diagnostics/status line, and does not Docker-preflight start/refresh/down before runtime work. Record the first failing assertion and fixture command log; do not weaken any assertion to make the old script pass.

- [ ] **3. Implement only the specified production seam and preserve the rest of the facade.** Make these bounded changes in `scripts/toolkit-contributor.sh`:

  1. Add pure `classify_toolkit_contributor_host(os_release_text, kernel_release_text)` and its data-only ID parser. Its success output is exactly one of the two tokens. It rejects malformed/missing/duplicate/non-Ubuntu IDs, unreadable/empty kernel captures, and WSL1 with the distinct remedies in the design.
  2. Add `read_toolkit_contributor_host` that performs the two literal reads and calls the seam. Do not parameterize either path. Store its successful result in `TOOLKIT_CONTRIBUTOR_HOST_MODE`; do not read a preexisting value.
  3. Add `require_toolkit_contributor_docker(mode)` with `command -v docker` then `docker info >/dev/null`, and the native/WSL2 diagnostic wording described in the interface section. Remove Docker from any later generic-tool loop so the daemon preflight occurs only once on mutating commands.
  4. Put `read_toolkit_contributor_host` at the beginning of every valid command function after argument validation. Put Docker preflight before `bootstrap` tool/SSH/clone work and before runtime-layout checks in `start`, `refresh`, and `down`. Do not add it to `seed` or `status`.
  5. In `status`, print `host mode: $TOOLKIT_CONTRIBUTOR_HOST_MODE` directly after a successful host gate, then retain the helper status and one Envoy health call. Keep every existing start/refresh/seed/down call, compose array, sleep bound, and URL unchanged after the new gates.
  6. Put the final `case` dispatch under the source-vs-execute guard needed by the direct pure-function test. `usage` still lists exactly six commands.

  Update `docs/toolkit-contributor-sandbox.md` as one loop, not two scripts: describe `ubuntu-native` and `ubuntu-wsl2`; show both literal observations; document strict Ubuntu ID/WSL1 boundary; list native prerequisites (Git/GitHub SSH, Go, Node/npm, rsync, jq, Docker CLI, any reachable Docker-compatible daemon) without Docker Desktop or Windows setup; retain WSL2 Docker Desktop integration guidance without making it exclusive; keep exact first-checkout/daily/refresh/marker/Vite/down commands; and state that the five-port `ss` observation is acceptance-only, has no scanner/alternate-port mode, and never kills an owner. Change `README.md` only to name native Ubuntu and Ubuntu WSL2 in its runbook pointer.

- [ ] **4. Run GREEN checks and inspect the narrow diff.**

  ```bash
  cd "$game_dev_worktree"
  bash -n scripts/toolkit-contributor.sh
  bash tests/toolkit-contributor-contract.sh
  bash tests/bootstrap-contract.sh
  git diff --check
  git diff -- scripts/toolkit-contributor.sh tests/toolkit-contributor-contract.sh \
    docs/toolkit-contributor-sandbox.md README.md
  git diff --name-only
  ```

  **Expected GREEN result:** all three Bash commands and `git diff --check` exit `0`; the contract ends `PASS: toolkit contributor facade contract verified`; the file list contains exactly the four listed paths. The fixture log proves direct legacy/new WSL2 classification, native classification, WSL1/Debian/bad-capture refusal, no environment bypass, every-command first gate, Docker fail-closed mutation prevention, status output/order, and every preexisting preservation assertion.

- [ ] **5. Commit only the four-file implementation and open one reviewable PR.**

  ```bash
  cd "$game_dev_worktree"
  git add scripts/toolkit-contributor.sh tests/toolkit-contributor-contract.sh \
    docs/toolkit-contributor-sandbox.md README.md
  test "$(git diff --cached --name-only | wc -l)" -eq 4
  git diff --cached --name-only | sort
  git diff --cached --check
  git commit -m 'feat: support native Ubuntu toolkit contributor sandbox'
  git status --short
  git diff origin/main...HEAD --check
  ```

  **Expected:** the staged list and commit contain exactly the four listed paths; post-commit status has no tracked/untracked implementation artifact; range diff check passes. Do not stage `.pi-subagents`, test logs, screenshots, Docker artifacts, worktrees, or unrelated primary-worktree files.

  Open one PR from `$game_dev_branch` to `main` titled `feat: support native Ubuntu toolkit contributor sandbox`. Its body contains: the two-mode interface table; exact literal file reads/classification precedence; no-bypass and gate-order proof; Docker diagnostic distinction; preservation matrix; four-path diff; all RED/GREEN commands; dependency SHAs/PRs; and one final closing line `Closes #${game_dev_issue}`. It contains no other `close`, `closes`, `fixes`, or `resolves` keyword.

- [ ] **6. Complete required review and merge gates before native acceptance.** Set the Project item to `In Review` only after CI is green. Obtain two independent signed review packages:

  1. **Specification/safety review:** checks every design row, literal path/no-bypass rule, pure seam, Ubuntu-ID parser, WSL precedence, all-command order, native/WSL diagnostics, scope/file boundary, docs, and no-auto-kill rule against this plan and #211.
  2. **Shell/TDD quality review:** runs the exact GREEN commands independently, inspects fake-command logs for forbidden later calls, checks regressions in clone/dirty/compose/override/RPC/down contracts, confirms test seams cannot select a production host, and reviews `git diff origin/main...HEAD`.

  Each reviewer posts a signed PR comment with `PASS` or concrete blockers. Any blocker returns the item to `In Progress` and is fixed with the same four-file scope and rerun evidence; no merge occurs on a conditional pass. After CI and both `PASS` comments, merge to `main`, capture `native_game_dev_merge_sha` with:

  ```bash
  native_game_dev_pr="$(gh pr list --repo KirkDiggler/game-dev --state merged --head "$game_dev_branch" --json number,mergeCommit --jq 'if length == 1 then .[0].number else empty end')"
  test -n "$native_game_dev_pr"
  native_game_dev_merge_sha="$(gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev --json mergeCommit --jq .mergeCommit.oid)"
  test -n "$native_game_dev_merge_sha"
  printf '%s\n' "$native_game_dev_merge_sha"
  ```

  **Expected:** exactly one merged PR is found; `native_game_dev_merge_sha` is recorded in both verification issues. The implementation issue closes through its one PR closing reference and its Project item becomes `Done`; the native verification item stays `Todo` until Task 3 begins.

### Task 3: Independently accept the landed facade on clean native Ubuntu

**Files:** no repository file is changed.
**Produces:** one native acceptance evidence package and one signed comment on `native_verify_issue`; issue closure only after independent acceptance.
**Consumes:** the merged native game-dev SHA, original merged SHAs, clean native Ubuntu, and the original sandbox's existing real API/web loop.
**Stop point:** a non-Ubuntu/WSL host, failed Docker/SSH check, missing dependency SHA, occupied/unobservable port, alternate Vite port, failed marker restoration, failed normal GameView, or failed `down` is a truthful blocker—not a code, port, Docker, or process-management workaround.

- [ ] **1. Establish the disposable path, evidence directory, native host identity, Docker/SSH readiness, and manual fixed-port observation before any clone or stack action.**

  ```bash
  set -euo pipefail
  native_clean_root="$HOME/toolkit-sandbox-native-${native_game_dev_merge_sha:0:12}"
  native_evidence="$native_clean_root/evidence"
  test ! -e "$native_clean_root"
  test -f /etc/os-release
  test -f /proc/sys/kernel/osrelease
  grep -Eq "^ID=(ubuntu|'ubuntu'|\"ubuntu\")$" /etc/os-release
  ! grep -Eqi 'wsl2|microsoft' /proc/sys/kernel/osrelease
  docker info
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD
  command -v git docker go node npm rsync jq ssh ss
  mkdir -p "$native_evidence"
  cat /etc/os-release | tee "$native_evidence/os-release.txt"
  cat /proc/sys/kernel/osrelease | tee "$native_evidence/kernel-osrelease.txt"
  docker info | tee "$native_evidence/docker-info.txt"
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD | tee "$native_evidence/github-ssh.txt"
  ss -ltnp '( sport = :80 or sport = :3001 or sport = :3002 or sport = :6380 or sport = :8080 )' \
    | tee "$native_evidence/ports-before.txt"
  ```

  **Expected:** Ubuntu ID is exactly accepted; kernel has neither Microsoft nor WSL2 signature; Docker and GitHub SSH work; `ports-before.txt` has no listener on 80, 3001, 3002, 6380, or 8080. The operator manually records any process detail `ss` exposes. If any target port is occupied or process observation is unavailable, stop before clone/start, retain these files, post a signed blocker comment, and do not kill/restart/reconfigure any listener.

- [ ] **2. Clone a fresh default-branch game-dev checkout, run the facade bootstrap twice, and prove it contains all merged dependencies.**

  ```bash
  git clone git@github.com:KirkDiggler/game-dev.git "$native_clean_root/game-dev"
  cd "$native_clean_root/game-dev"
  git merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a HEAD
  git merge-base --is-ancestor "$native_game_dev_merge_sha" HEAD
  ./scripts/toolkit-contributor.sh bootstrap | tee "$native_evidence/bootstrap-first.txt"
  ./scripts/toolkit-contributor.sh bootstrap | tee "$native_evidence/bootstrap-second.txt"
  test -d rpg-toolkit/.git
  test -d rpg-api/.git
  test -d rpg-dnd5e-web/.git
  test -d rpg-deployment/.git
  git -C rpg-api merge-base --is-ancestor 9099953f9bc86efbed9bf62209a96c54d9383d6b HEAD
  git -C rpg-dnd5e-web merge-base --is-ancestor cfa63138a1f06de65991c31b006f29fc2af1ad74 HEAD
  ```

  **Expected:** the clone and both bootstrap runs exit `0`; first run creates exactly the four facade roots; second reports them unchanged; all four release ancestry checks pass. A missing original provider/web SHA is an integration-baseline blocker—do not checkout, rebase, or alter the facade's clone behavior to mask it.

- [ ] **3. Run merged focused gates in the clean checkout before live mutation.**

  ```bash
  cd "$native_clean_root/game-dev/rpg-api"
  bash scripts/toolkit-local-override.test.sh | tee "$native_evidence/api-override-contract.txt"
  go test ./cmd/sandboxseed -count=1 | tee "$native_evidence/api-sandboxseed.txt"
  go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1 \
    | tee "$native_evidence/api-sandboxseed-integration.txt"
  make pre-commit | tee "$native_evidence/api-pre-commit.txt"

  cd "$native_clean_root/game-dev"
  bash tests/toolkit-contributor-contract.sh | tee "$native_evidence/game-dev-contributor-contract.txt"
  bash tests/bootstrap-contract.sh | tee "$native_evidence/game-dev-bootstrap-contract.txt"

  cd "$native_clean_root/game-dev/rpg-dnd5e-web"
  npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx \
    | tee "$native_evidence/web-sandbox-tests.txt"
  npm run ci-check | tee "$native_evidence/web-ci-check.txt"
  ```

  **Expected:** every command exits `0`. The integration test includes the existing wrong-owner Create/Join negative; retain its output. A failed focused test blocks live acceptance and is reported without an unrelated fix.

- [ ] **4. Start the exact stack, prove native status/Envoy and two seed resets, then arm restoration/down fail-safes before the marker.**

  ```bash
  cd "$native_clean_root/game-dev"
  marker_file="$native_clean_root/game-dev/rpg-toolkit/rulebooks/dnd5e/races/data.go"
  marker_changed=0
  vite_pid=''
  cleanup_native_acceptance() {
    status=$?
    if [ "$marker_changed" -eq 1 ]; then
      git -C "$native_clean_root/game-dev/rpg-toolkit" checkout -- rulebooks/dnd5e/races/data.go || true
      "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" refresh || true
      "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" seed || true
    fi
    if [ -n "$vite_pid" ] && kill -0 "$vite_pid" 2>/dev/null; then
      kill "$vite_pid" || true
      wait "$vite_pid" || true
    fi
    "$native_clean_root/game-dev/scripts/toolkit-contributor.sh" down || true
    exit "$status"
  }
  trap cleanup_native_acceptance EXIT INT TERM

  ./scripts/toolkit-contributor.sh start | tee "$native_evidence/start.txt"
  ./scripts/toolkit-contributor.sh status | tee "$native_evidence/status.txt"
  grep -Fqx 'host mode: ubuntu-native' "$native_evidence/status.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/seed-first.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/seed-second.txt"
  grep -Fq 'Strength 16' "$native_evidence/seed-first.txt"
  grep -Fq 'Strength 16' "$native_evidence/seed-second.txt"
  grep -Fq 'Protection' "$native_evidence/seed-second.txt"
  grep -Fq 'shield' "$native_evidence/seed-second.txt"
  ```

  **Expected:** start/status/seed commands exit `0`; status prints `host mode: ubuntu-native` and the normal helper/Envoy results; each seed proves the fixed characters, fighter Protection, shield, and Strength 16. The trap may terminate only the Vite PID this task itself later starts; it never identifies or stops an unrelated listener. It restores the marker and runs explicit `down` on failure. Do not disable the trap.

- [ ] **5. Execute and prove the original reversible Human Strength marker, including restoration before browser work.**

  ```bash
  cd "$native_clean_root/game-dev"
  perl -0pi -e 's/(Human: \{.*?abilities\.STR: )1,/${1}2,/s' rpg-toolkit/rulebooks/dnd5e/races/data.go
  marker_changed=1
  ./scripts/toolkit-contributor.sh refresh | tee "$native_evidence/marker-refresh-to-17.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/marker-seed-17.txt"
  grep -Fq 'Strength 17' "$native_evidence/marker-seed-17.txt"
  git -C rpg-toolkit checkout -- rulebooks/dnd5e/races/data.go
  marker_changed=0
  ./scripts/toolkit-contributor.sh refresh | tee "$native_evidence/marker-refresh-to-16.txt"
  ./scripts/toolkit-contributor.sh seed | tee "$native_evidence/marker-seed-restored-16.txt"
  grep -Fq 'Strength 16' "$native_evidence/marker-seed-restored-16.txt"
  git -C rpg-toolkit diff --exit-code -- rulebooks/dnd5e/races/data.go
  ```

  **Expected:** seed evidence is 16 before edit, 17 only after refresh/reseed, and 16 after source restoration/refresh/reseed. The final `git diff --exit-code` is zero. A failed restoration is a hard blocker; leave the trap active and do not continue to screenshots.

- [ ] **6. Start only the owned Vite process, retain its log, save the real template, exercise all literal party arrangements, and capture all normal routes.**

  ```bash
  cd "$native_clean_root/game-dev/tools/browser"
  npm ci | tee "$native_evidence/browser-npm-ci.txt"
  npx playwright install chromium | tee "$native_evidence/playwright-install.txt"

  cd "$native_clean_root/game-dev/rpg-dnd5e-web"
  npm ci | tee "$native_evidence/web-npm-ci.txt"
  npm run dev > "$native_evidence/vite.log" 2>&1 &
  vite_pid=$!
  for attempt in $(seq 1 30); do
    if curl -fsS 'http://localhost:3001/?toolkitSandbox=1' > "$native_evidence/sandbox-page.html"; then
      break
    fi
    sleep 2
  done
  test -s "$native_evidence/sandbox-page.html"
  grep -Fq 'http://localhost:3001' "$native_evidence/vite.log"
  ```

  **Expected:** Vite is the recorded `$vite_pid`, serves exactly port 3001, and its log is retained. A bind failure or a Vite-selected different port stops acceptance; do not change the port or kill its owner.

  In the browser at `http://localhost:3001/?toolkitSandbox=1`, save the populated `toolkit-contributor-sandbox` template through **PutDungeon** and retain the successful key. Then run exactly these UI actions in this order, waiting for each displayed normal link to resolve before moving on: **Fighter**; **Barbarian**; **Fighter then Barbarian**; **Barbarian then Fighter**. For every selected normal `?playerId=` link, capture the normal GameView—not a harness route—with these exact non-overwriting filenames:

  ```bash
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?toolkitSandbox=1' \
    "$native_evidence/toolkit-sandbox-put-dungeon.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
    "$native_evidence/toolkit-sandbox-fighter-only-fighter-gameview.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
    "$native_evidence/toolkit-sandbox-barbarian-only-barbarian-gameview.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
    "$native_evidence/toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
    "$native_evidence/toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
    "$native_evidence/toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png"
  node "$native_clean_root/game-dev/tools/browser/screenshot.mjs" \
    'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
    "$native_evidence/toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png"
  ```

  **Expected:** the PutDungeon screenshot/key and all six order-qualified files exist and show normal GameView/EncounterView for their declared identity/order. Harness URLs are not evidence. Retain browser-visible errors as failure evidence rather than retrying into another scenario.

- [ ] **7. Run production negatives, explicitly stop only the owned Vite, run owned `down`, retain evidence, and request independent acceptance.**

  ```bash
  cd "$native_clean_root/game-dev/rpg-api"
  go test ./internal/auth -run TestUnaryAuthInterceptor_DevScheme_NotAllowed -count=1 \
    | tee "$native_evidence/production-dev-header-negative.txt"

  cd "$native_clean_root/game-dev/rpg-dnd5e-web"
  npm run test:run -- src/toolkit-contributor-sandbox/route.test.ts \
    | tee "$native_evidence/production-sandbox-route-negative.txt"

  if kill -0 "$vite_pid" 2>/dev/null; then
    kill "$vite_pid"
    wait "$vite_pid" || true
  fi
  vite_pid=''

  cd "$native_clean_root/game-dev"
  ./scripts/toolkit-contributor.sh down | tee "$native_evidence/down.txt"
  test ! -e rpg-api/local-toolkit/rulebooks/dnd5e
  test -d rpg-toolkit/.git
  test -d rpg-api/.git
  test -d rpg-dnd5e-web/.git
  test -d rpg-deployment/.git
  trap - EXIT INT TERM
  sha256sum "$native_evidence"/* > "$native_evidence/SHA256SUMS.txt"
  find "$native_evidence" -maxdepth 1 -type f -printf '%f\n' | sort | tee "$native_evidence/manifest.txt"
  ```

  **Expected:** both negatives pass, Vite shutdown addresses only the PID started in Step 6, explicit `down` succeeds, only the owned D&D 5e local tree is gone, and all checkouts/marker edits remain intact. Do not delete the clean checkout or evidence directory.

  Post one signed comment to `native_verify_issue` with this evidence schema: exact original/native merge SHAs and PR URLs; `os-release`/kernel/mode; Docker/SSH exit results; `ports-before.txt` observation; every focused/live command with exit code; bootstrap first/second proof; start/status/Envoy; two seeds; 16→17→16 marker; PutDungeon key; the seven PNG filenames including six normal-route images; wrong-owner integration output; production negatives; Vite PID/log; explicit down/tree result; SHA256 manifest; and the independent-review request. Attach the seven PNGs and `SHA256SUMS.txt` through the issue attachment UI or the approved artifact store, then put their resulting links in the comment. Evidence must remain at `$native_evidence` until reviewer acceptance; if attachment publication is unavailable, leave the issue open and report that as an evidence-retention blocker.

  A reviewer independent of the executor verifies the command transcripts, image names/content, checksum manifest, no-bypass/no-auto-kill posture, exact closure conditions, and that failures were not hidden. Only a signed `PASS` review allows setting this Project item to `Done` and closing `native_verify_issue`. Any failed check or missing attachment leaves it `In Progress` with a signed blocker comment; it does not begin a new implementation wave.

### Task 4: Formally amend and execute existing WSL2 verification issue #210 on the landed native baseline

**Files:** no repository file is changed.
**Produces:** a signed superseding execution-baseline comment and latest `AGENT PICKUP — START HERE ON UBUNTU WSL2` packet on #210, then independent WSL2 evidence on the same issue.
**Consumes:** native PR merge SHA and accepted native verification from Task 3 plus original merged SHAs.
**Stop point:** do not amend/execute #210 before native acceptance succeeds; do not create another WSL issue, branch, or PR; do not change #210 Team/Feature/Kind from Cross-team/Infra/Verify.

- [ ] **1. Gate the amended baseline and set #210 active without changing its ownership fields.**

  ```bash
  set -euo pipefail
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state,projectItems --jq '.state'
  gh pr view "$native_game_dev_pr" --repo KirkDiggler/game-dev --json state,mergeCommit --jq '.state + " " + .mergeCommit.oid'
  gh issue view 210 --repo KirkDiggler/rpg-project --json state,title,projectItems
  ```

  **Expected:** native verification is closed after accepted evidence; native PR is `MERGED` at `native_game_dev_merge_sha`; #210 is open and still Cross-team/Infra/Verify. Change only #210 Status to `In Progress` if it was not already there.

- [ ] **2. Post the formal amendment and latest self-contained packet as the latest #210 comment.** Generate it only from real merged outputs, so it contains concrete SHA strings and a concrete clean path when posted—not an angle-bracket, future-number, or unexpanded token.

  ```bash
  set -euo pipefail
  wsl_clean_root="$HOME/toolkit-sandbox-wsl2-${native_game_dev_merge_sha:0:12}"
  wsl_packet="$(mktemp)"
  cat > "$wsl_packet" <<EOF
  ## FORMAL EXECUTION-BASELINE AMENDMENT

  The prior WSL2 execution baseline is superseded intentionally. Execute this issue against original API PR #792 at 9099953f9bc86efbed9bf62209a96c54d9383d6b, original game-dev PR #60 at 1df0212e5a04374ea83b9af1dd81d09a8a55831a, original web PR #747 at cfa63138a1f06de65991c31b006f29fc2af1ad74, original design/plan PR #209 at 22aee544a42906c2f8c01a0e1eb4935c252dcda2, and landed native game-dev PR #${native_game_dev_pr} at ${native_game_dev_merge_sha}. This is the same landed facade proving the WSL2 regression; it is not a new implementation wave.

  ## AGENT PICKUP — START HERE ON UBUNTU WSL2

  <!-- pih-dispatch:v1 -->

  ## Goal / symptom
  Independently prove that the landed two-mode facade preserves the original full contributor loop on clean Ubuntu WSL2 after native Ubuntu support landed.

  ## Desired outcome
  Record one accepted evidence package from ${wsl_clean_root}. Status must print host mode: ubuntu-wsl2; the original bootstrap, Envoy, seed, refresh marker, PutDungeon, four party orders, normal GameView, negatives, and owned cleanup must all succeed.

  ## Contract boundaries
  - Evidence only: no source edit except the temporary Human Strength marker in the disposable toolkit checkout; no branch, PR, implementation issue, deployment/API/web/toolkit/proto change, host override, direct storage, harness-only evidence, alternate port, or Docker/Windows configuration change.
  - Do not stop, restart, kill, or repurpose another listener. The facade does not scan or manage ports. A failed observation, occupied port, Docker bind failure, or Vite port other than 3001 is a blocker.
  - Preserve #210's Cross-team / Infra / Verify fields. Do not use a closing keyword for #208. Close #210 only after complete evidence and independent review pass.

  ## Preflight
  Run exactly:

  \`\`\`bash
  set -euo pipefail
  test ! -e "${wsl_clean_root}"
  test -f /etc/os-release
  test -f /proc/sys/kernel/osrelease
  grep -Eq "^ID=(ubuntu|'ubuntu'|\\\"ubuntu\\\")$" /etc/os-release
  grep -Eqi 'wsl2|microsoft-standard' /proc/sys/kernel/osrelease
  docker info
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD
  command -v git docker go node npm rsync jq ssh ss
  mkdir -p "${wsl_clean_root}/evidence"
  cat /etc/os-release | tee "${wsl_clean_root}/evidence/os-release.txt"
  cat /proc/sys/kernel/osrelease | tee "${wsl_clean_root}/evidence/kernel-osrelease.txt"
  docker info | tee "${wsl_clean_root}/evidence/docker-info.txt"
  git ls-remote git@github.com:KirkDiggler/game-dev.git HEAD | tee "${wsl_clean_root}/evidence/github-ssh.txt"
  ss -ltnp '( sport = :80 or sport = :3001 or sport = :3002 or sport = :6380 or sport = :8080 )' | tee "${wsl_clean_root}/evidence/ports-before.txt"
  \`\`\`

  Require a WSL2 signature, reachable Docker daemon, GitHub SSH, and clear/observable 80, 3001, 3002, 6380, 8080. Port 3001 is Vite; 80, 3002, 6380, and 8080 are the unchanged compose publications. On failure, retain the preflight files, post a signed blocker comment, leave #210 open, and stop; never kill a listener.

  ## Exact execution
  \`\`\`bash
  git clone git@github.com:KirkDiggler/game-dev.git "${wsl_clean_root}/game-dev"
  cd "${wsl_clean_root}/game-dev"
  git merge-base --is-ancestor 1df0212e5a04374ea83b9af1dd81d09a8a55831a HEAD
  git merge-base --is-ancestor ${native_game_dev_merge_sha} HEAD
  ./scripts/toolkit-contributor.sh bootstrap | tee "${wsl_clean_root}/evidence/bootstrap-first.txt"
  ./scripts/toolkit-contributor.sh bootstrap | tee "${wsl_clean_root}/evidence/bootstrap-second.txt"
  git -C rpg-api merge-base --is-ancestor 9099953f9bc86efbed9bf62209a96c54d9383d6b HEAD
  git -C rpg-dnd5e-web merge-base --is-ancestor cfa63138a1f06de65991c31b006f29fc2af1ad74 HEAD

  cd "${wsl_clean_root}/game-dev/rpg-api"
  bash scripts/toolkit-local-override.test.sh | tee "${wsl_clean_root}/evidence/api-override-contract.txt"
  go test ./cmd/sandboxseed -count=1 | tee "${wsl_clean_root}/evidence/api-sandboxseed.txt"
  go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1 | tee "${wsl_clean_root}/evidence/api-sandboxseed-integration.txt"
  make pre-commit | tee "${wsl_clean_root}/evidence/api-pre-commit.txt"

  cd "${wsl_clean_root}/game-dev"
  bash tests/toolkit-contributor-contract.sh | tee "${wsl_clean_root}/evidence/game-dev-contributor-contract.txt"
  bash tests/bootstrap-contract.sh | tee "${wsl_clean_root}/evidence/game-dev-bootstrap-contract.txt"

  cd "${wsl_clean_root}/game-dev/rpg-dnd5e-web"
  npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx | tee "${wsl_clean_root}/evidence/web-sandbox-tests.txt"
  npm run ci-check | tee "${wsl_clean_root}/evidence/web-ci-check.txt"

  cd "${wsl_clean_root}/game-dev"
  marker_changed=0
  vite_pid=''
  cleanup_wsl_acceptance() {
    status=\$?
    if [ "\$marker_changed" -eq 1 ]; then
      git -C "${wsl_clean_root}/game-dev/rpg-toolkit" checkout -- rulebooks/dnd5e/races/data.go || true
      "${wsl_clean_root}/game-dev/scripts/toolkit-contributor.sh" refresh || true
      "${wsl_clean_root}/game-dev/scripts/toolkit-contributor.sh" seed || true
    fi
    if [ -n "\$vite_pid" ] && kill -0 "\$vite_pid" 2>/dev/null; then
      kill "\$vite_pid" || true
      wait "\$vite_pid" || true
    fi
    "${wsl_clean_root}/game-dev/scripts/toolkit-contributor.sh" down || true
    exit "\$status"
  }
  trap cleanup_wsl_acceptance EXIT INT TERM
  ./scripts/toolkit-contributor.sh start | tee "${wsl_clean_root}/evidence/start.txt"
  ./scripts/toolkit-contributor.sh status | tee "${wsl_clean_root}/evidence/status.txt"
  grep -Fqx 'host mode: ubuntu-wsl2' "${wsl_clean_root}/evidence/status.txt"
  ./scripts/toolkit-contributor.sh seed | tee "${wsl_clean_root}/evidence/seed-first.txt"
  ./scripts/toolkit-contributor.sh seed | tee "${wsl_clean_root}/evidence/seed-second.txt"
  grep -Fq 'Strength 16' "${wsl_clean_root}/evidence/seed-first.txt"
  grep -Fq 'Protection' "${wsl_clean_root}/evidence/seed-second.txt"
  grep -Fq 'shield' "${wsl_clean_root}/evidence/seed-second.txt"

  perl -0pi -e 's/(Human: \\{.*?abilities\\.STR: )1,/\${1}2,/s' rpg-toolkit/rulebooks/dnd5e/races/data.go
  marker_changed=1
  ./scripts/toolkit-contributor.sh refresh | tee "${wsl_clean_root}/evidence/marker-refresh-to-17.txt"
  ./scripts/toolkit-contributor.sh seed | tee "${wsl_clean_root}/evidence/marker-seed-17.txt"
  grep -Fq 'Strength 17' "${wsl_clean_root}/evidence/marker-seed-17.txt"
  git -C rpg-toolkit checkout -- rulebooks/dnd5e/races/data.go
  marker_changed=0
  ./scripts/toolkit-contributor.sh refresh | tee "${wsl_clean_root}/evidence/marker-refresh-to-16.txt"
  ./scripts/toolkit-contributor.sh seed | tee "${wsl_clean_root}/evidence/marker-seed-restored-16.txt"
  grep -Fq 'Strength 16' "${wsl_clean_root}/evidence/marker-seed-restored-16.txt"
  git -C rpg-toolkit diff --exit-code -- rulebooks/dnd5e/races/data.go
  \`\`\`

  Start the browser runtime and Vite exactly:

  \`\`\`bash
  cd "${wsl_clean_root}/game-dev/tools/browser"
  npm ci | tee "${wsl_clean_root}/evidence/browser-npm-ci.txt"
  npx playwright install chromium | tee "${wsl_clean_root}/evidence/playwright-install.txt"
  cd "${wsl_clean_root}/game-dev/rpg-dnd5e-web"
  npm ci | tee "${wsl_clean_root}/evidence/web-npm-ci.txt"
  npm run dev > "${wsl_clean_root}/evidence/vite.log" 2>&1 &
  vite_pid=\$!
  for attempt in \$(seq 1 30); do
    if curl -fsS 'http://localhost:3001/?toolkitSandbox=1' > "${wsl_clean_root}/evidence/sandbox-page.html"; then break; fi
    sleep 2
  done
  test -s "${wsl_clean_root}/evidence/sandbox-page.html"
  grep -Fq 'http://localhost:3001' "${wsl_clean_root}/evidence/vite.log"
  \`\`\`

  At http://localhost:3001/?toolkitSandbox=1, save the populated template through PutDungeon and run exactly Fighter, Barbarian, Fighter then Barbarian, then Barbarian then Fighter. Capture the PutDungeon screen plus these six normal GameView URLs/files; no harness URL substitutes:

  \`\`\`bash
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?toolkitSandbox=1' "${wsl_clean_root}/evidence/toolkit-sandbox-put-dungeon.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-fighter' "${wsl_clean_root}/evidence/toolkit-sandbox-fighter-only-fighter-gameview.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' "${wsl_clean_root}/evidence/toolkit-sandbox-barbarian-only-barbarian-gameview.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-fighter' "${wsl_clean_root}/evidence/toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' "${wsl_clean_root}/evidence/toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' "${wsl_clean_root}/evidence/toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png"
  node "${wsl_clean_root}/game-dev/tools/browser/screenshot.mjs" 'http://localhost:3001/?playerId=toolkit-sandbox-fighter' "${wsl_clean_root}/evidence/toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png"
  \`\`\`

  Finish with exact negatives and cleanup:

  \`\`\`bash
  cd "${wsl_clean_root}/game-dev/rpg-api"
  go test ./internal/auth -run TestUnaryAuthInterceptor_DevScheme_NotAllowed -count=1 | tee "${wsl_clean_root}/evidence/production-dev-header-negative.txt"
  cd "${wsl_clean_root}/game-dev/rpg-dnd5e-web"
  npm run test:run -- src/toolkit-contributor-sandbox/route.test.ts | tee "${wsl_clean_root}/evidence/production-sandbox-route-negative.txt"
  if kill -0 "\$vite_pid" 2>/dev/null; then kill "\$vite_pid"; wait "\$vite_pid" || true; fi
  vite_pid=''
  cd "${wsl_clean_root}/game-dev"
  ./scripts/toolkit-contributor.sh down | tee "${wsl_clean_root}/evidence/down.txt"
  test ! -e rpg-api/local-toolkit/rulebooks/dnd5e
  test -d rpg-toolkit/.git
  test -d rpg-api/.git
  test -d rpg-dnd5e-web/.git
  test -d rpg-deployment/.git
  trap - EXIT INT TERM
  sha256sum "${wsl_clean_root}/evidence"/* > "${wsl_clean_root}/evidence/SHA256SUMS.txt"
  find "${wsl_clean_root}/evidence" -maxdepth 1 -type f -printf '%f\\n' | sort | tee "${wsl_clean_root}/evidence/manifest.txt"
  \`\`\`

  ## Verification evidence
  Post one signed evidence comment with concrete baseline SHAs; host-mode/status; Docker/SSH/ports; all exit codes; bootstrap rerun; two seeds; 16→17→16; PutDungeon key; existing wrong-owner Create/Join integration result; production negatives; down/tree check; Vite log/PID; the seven PNG names; SHA256 manifest; and attachment links. Retain files until review. On any failure, run the trap only for the marker, the Vite PID started here, and explicit facade down; post a signed blocker; leave #210 open; do not kill an unrelated process or start implementation.

  ## Copy-paste prompt
  Execute rpg-project#210 exactly from the latest AGENT PICKUP — START HERE ON UBUNTU WSL2 comment. Work evidence-only in its declared clean path, preserve Cross-team/Infra/Verify ownership, stop on any preflight or acceptance failure without changing code/ports/Docker/Windows, restore the marker and run owned down through the supplied trap, attach the complete evidence package, and close only after independent review passes.

  ## Related / dependencies
  Parent https://github.com/KirkDiggler/rpg-project/issues/208
  Native design decision https://github.com/KirkDiggler/rpg-project/issues/211
  Native verification ${native_verify_url}

  — asset-pipeline agent, on behalf of KirkDiggler
  EOF
  gh issue comment 210 --repo KirkDiggler/rpg-project --body-file "$wsl_packet"
  rm -f "$wsl_packet"
  ```

  **Expected:** this is the latest #210 comment; it is self-contained, has actual SHAs/path, includes all preflight/live/browser/negative/down commands and names all seven screenshot files, declares failure behavior, and has exactly one copy-paste prompt. It does not weaken #210's ownership or create an implementation deliverable.

- [ ] **3. Execute the packet on Ubuntu WSL2, publish its evidence, and close only after independent acceptance.** Follow every command in the posted packet, not a locally remembered variant. At preflight, `microsoft-standard` or `wsl2` is required; `4.4.0-19041-Microsoft` remains WSL1/unsupported. Keep the marker/Vite/down trap intact until restoration and explicit down pass. Attach the seven defined PNGs and checksum manifest before requesting review.

  **Expected success:** all packet commands exit zero; status reports `host mode: ubuntu-wsl2`; the two seed runs, 16→17→16 proof, PutDungeon, four party arrangements, six normal GameView screenshots, wrong-owner integration result, production negatives, and owned cleanup are present. An independent reviewer posts signed `PASS`. Only then set #210 Status `Done` and close #210. A failure leaves #210 `In Progress` with a signed blocker and does not close #208.

### Task 5: Reconcile final delivery state and close #208 only after all required evidence is accepted

**Files:** no repository file is changed.
**Produces:** exact signed closure comments and final Project 19 status only after all gates are real.
**Stop point:** do not close #208 for a merged implementation alone, a native-only result, a WSL-only result, an unreviewed artifact package, or a missing design-decision handoff.

- [ ] **1. Verify the closure order and all required records.**

  ```bash
  set -euo pipefail
  gh issue view 211 --repo KirkDiggler/rpg-project --json state,comments
  gh issue view "$game_dev_issue" --repo KirkDiggler/game-dev --json state,closedByPullRequestsReferences,projectItems
  gh issue view "$native_verify_issue" --repo KirkDiggler/rpg-project --json state,comments,projectItems
  gh issue view 210 --repo KirkDiggler/rpg-project --json state,comments,projectItems
  gh issue view 208 --repo KirkDiggler/rpg-project --json state,projectItems
  ```

  **Expected:** the closure sequence is: (a) #211 was closed as a design decision only after the merged design/plan and Task 1 delivery-record handoff; (b) the single game-dev issue closed through its merged PR; (c) native verification closed only after its independent acceptance; (d) #210 closed only after the amended WSL2 packet and independent acceptance; (e) #208 is still open. #210's retained classification is Cross-team/Infra/Verify; no duplicate WSL record exists.

- [ ] **2. Post the signed parent completion summary and set only truthful final board statuses.** The #208 comment lists the design/plan commit(s), PR #209/#60/#792/#747/native PR URL and SHAs, game-dev issue/PR, native verification issue/evidence links, #210 amended-packet/evidence links, review `PASS` links, and explicit statement that no API/web/toolkit/proto/deployment code changed in the native wave. It ends with the required signature and contains no closing reference to another issue.

  Set #208 Project 19 `Status=Done` only after the comment; retain its existing Team/Feature/Kind values unless the board owner explicitly changes them. Do not change #210 Team/Feature/Kind. Preserve completed game-dev and native verification fields as `Platform / Infra / Build / Done` and `Platform / Infra / Verify / Done`.

- [ ] **3. Close #208 and preserve all provenance.** Close #208 only after the read-back confirms all Task 5 Step 1 predicates and the signed summary is visible. Keep `$game_dev_worktree`, `$native_clean_root`, `$native_clean_root/evidence`, `$wsl_clean_root`, and `$wsl_clean_root/evidence` in place for audit/attachment recovery. Do not launch a follow-up implementation wave from a verification finding; record a separately triaged future issue only after the parent closure review decides it is needed.

## Review packages and evidence checklist

| Gate                        | Required evidence                                                                                                                                   | Independent decision                                                                                               |
| --------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| Task 1 process gate         | merged plan/design, exact issue URLs/numbers, #208 child links, Project field read-back, original SHA ancestry, clean worktree                      | Coordinator verifies before implementation starts.                                                                 |
| Task 2 TDD/safety gate      | RED failure, GREEN syntax/contracts, fake command logs, four-path diff, docs assertions, PR CI                                                      | Separate specification/safety and shell/TDD reviewers both post signed `PASS`.                                     |
| Task 3 native live gate     | host/port/Docker/SSH files, clean bootstrap, merged focused tests, Envoy/status, seeds, marker, PutDungeon, screenshots, negatives, down, checksums | A reviewer not running the acceptance verifies the complete attachment package before closing native verification. |
| Task 4 WSL2 regression gate | formal latest packet, actual merged SHA/path, all repeated WSL2 transcript/artifacts, attachments, preservation of #210 fields                      | A reviewer not running the WSL2 acceptance signs `PASS` before #210 closes.                                        |
| Task 5 parent gate          | closure-order reads, signed #208 summary, board fields, evidence/review links, retained worktrees                                                   | Coordinator confirms all dependencies before closing #208.                                                         |

## Plan self-review requirements

Before committing this plan and before any execution worker starts Task 1, verify:

```bash
grep -nE 'T''BD|TO''DO|fill'' in|to be'' determined|place''holder|similar'' to|as'' above' \
  ideas/toolkit-contributor-native-ubuntu/plan.md
npx prettier --check ideas/toolkit-contributor-native-ubuntu/plan.md
git diff --check
git diff -- ideas/toolkit-contributor-native-ubuntu/plan.md
git status --short
git diff --cached --name-only
```

**Expected:** the deferred-work scan exits `1` with no matches; Prettier and diff checks exit `0`; the plan describes four production file changes only, exact Task 1-derived issue/branch/worktree names, literal source paths, classification/parser/diagnostic rules, every command order, exact compose/port sources, full native and WSL2 browser command/file names, traps, evidence retention, reviews, and closure order. Before this planning commit, no files are staged. The commit for this plan is only:

```bash
git add ideas/toolkit-contributor-native-ubuntu/plan.md
git commit -m 'docs: plan native Ubuntu toolkit sandbox support'
```
