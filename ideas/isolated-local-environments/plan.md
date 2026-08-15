# Isolated One-Command Local Game Environments Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make one named command start, observe, and safely stop a complete local game while allowing independent branch environments to coexist on native Ubuntu and Ubuntu WSL2.

**Architecture:** Two small provider PRs first expose project-scoped Compose and explicit asset-path contracts. A `game-dev` consumer then resolves pushed refs or developer-owned worktree paths, supervises isolated Compose plus host Vite, and records ownership in versioned JSON receipts shared by the normal and toolkit facades.

**Tech Stack:** Bash 4+, Git worktrees, Docker Compose v2.16+, `jq`, `ss`, `curl`, `setsid`, `flock`, `timeout`, `sha256sum`, `od`, Node/npm/Vite, Vitest, `rsync`, Go sandbox-seed health client, GitHub CLI.

**Spec:** `ideas/isolated-local-environments/design.md`

## Global Constraints

- Implementation must match the approved design at commit `3e6bcec5c33b15080e6cba507fb7eb3616fa26ce` and tracking issue `rpg-project#221`.
- Provider order is mandatory at the dependency boundary: `rpg-deployment/main` and `rpg-dnd5e-web/dev` may implement in parallel, but both must merge before `game-dev/main`; the consumer may not duplicate an unmerged provider contract.
- Use one issue, one branch, one worktree, and one PR per owning repository. Kirk alone merges PRs.
- Start every implementation branch from a freshly fetched `origin/main`, except `rpg-dnd5e-web`, which starts from `origin/dev` and targets `dev`.
- Primary checkouts contain unrelated work and must not be switched, reset, cleaned, stashed, or edited. All writes happen in isolated worktrees.
- Only Envoy is host-published. Its port is explicit manifest data; Vite retains normal next-available-port behavior and its reported URL is observed data.
- Manifests and receipts are parsed as data. Never use `source`, `eval`, command substitution from manifest values, or caller-supplied Compose/image identity.
- A path-backed source is never changed by Git operations. A ref-backed source may change only launcher-owned refs and launcher-managed detached worktrees.
- An occupied Envoy port never triggers fallback or eviction. No command may kill or stop a process/container without proving environment ownership.
- `toolkit-contributor.sh` retains exactly `bootstrap`, `start`, `refresh`, `seed`, `status`, and `down`, plus optional global `--env`; literal host classification remains non-overridable.
- `game-dev` runtime commands never delete `package-lock.json`; missing or stale dependencies use `HUSKY=0 npm ci` so managed worktrees cannot rewrite shared Git hooks.
- Ref-backed toolkit mode requires a launcher-owned API worktree and rejects `RPG_API_PATH`.
- `.pi-subagents/`, `.runtime/`, local manifests, credentials, generated assets, node modules, and evidence are never staged.
- GitHub issue/PR comments end with `— asset-pipeline agent, on behalf of KirkDiggler`.
- Live native-Ubuntu and Ubuntu-WSL2 attestations are separate. Hosted CI does not substitute for either.

## Delivery Units and Merge Gates

| Unit | Repository / base | Produces | Consumer gate |
|---|---|---|---|
| A | `rpg-deployment` / `origin/main` | Project-scoped local Compose, explicit API image/Envoy port, Compose contract tests | Merge SHA must be an ancestor of `origin/main` |
| B | `rpg-dnd5e-web` / `origin/dev` | Non-mutating `RPG_GAME_ASSETS_PATH` mode and tests | Merge SHA must be an ancestor of `origin/dev` |
| C | `game-dev` / `origin/main` | Shared supervisor, both facades, manifests, docs, deterministic tests | Units A and B merged first |
| D | Native Ubuntu + Ubuntu WSL2 | Live two-environment and failure-safety attestations | Exact Unit C PR head on both hosts |

Units A and B are independent and should use parallel isolated writer subagents. Unit C is one serial writer lane: use a fresh implementer and two-stage review per task, but never parallel writers in the same worktree. Use fresh read-only reviewers for correctness and test quality after each unit.

## Locked File Structure and Interfaces

### Unit A — `rpg-deployment`

- Create `tests/local-compose-contract.sh` — render and assert normal/toolkit Compose contracts without starting containers.
- Create `.github/workflows/local-compose-contract.yml` — install/use Compose and run the contract on pull requests.
- Modify `docker-compose.local-dev.yml` — remove participating `container_name` and non-Envoy publications; parameterize Envoy host port.
- Modify `docker-compose.api.yml` — remove toolkit overlay fixed names.
- Modify `docker-compose.local-api-src.yml` — parameterize local API image with a backward-compatible default.
- Modify `Makefile` — include the contract in `make test`.
- Modify `LOCAL_DEV.md` and `LOCAL_DEV_WORKFLOW.md` — document project names, Envoy-only publication, service-scoped logs, and legacy `local-src` exclusion.

Only `docker-compose.local-dev.yml`, `docker-compose.api.yml`, and `docker-compose.local-api-src.yml` participate in the named supervisor. Production Compose, modular production files, fixed toolkit labs, and the containerized-web `docker-compose.local-src.yml` are not migrated by this wave.

### Unit B — `rpg-dnd5e-web`

- Create `scripts/sync-synty-assets.test.ts` — node-environment fixture tests for explicit and legacy modes.
- Modify `scripts/sync-synty-assets.sh` — explicit non-mutating source selection.
- Modify `vite.config.ts` — add only `scripts/sync-synty-assets.test.ts` to Vitest discovery while preserving the existing `src/**` include and Vite server configuration.
- Modify `.github/workflows/ci.yml` — replace the no-test placeholder with the repository test command.

Vite server behavior remains unchanged: port 3001 has no `strictPort` and the proxy honors `VITE_API_HOST`.

### Unit C — `game-dev`

Create focused sourced modules:

| File | Responsibility | Public shell interface |
|---|---|---|
| `scripts/local-env/common.sh` | logging, canonical paths, env-name grammar, runtime identity, atomic files, dry-run command wrapper | `local_env_context ROOT NAME MODE` emits context JSON |
| `scripts/local-env/manifest.sh` | strict known-key parser and digest | `local_env_manifest_read CONTEXT_JSON` emits desired-state JSON |
| `scripts/local-env/sources.sh` | exact-origin path validation, ref resolution, managed detached worktrees | `local_env_sources_preflight CONTEXT DESIRED`; `local_env_sources_resolve CONTEXT PREFLIGHT` emit JSON |
| `scripts/local-env/receipt.sh` | attempt/receipt v1 schemas, closed validation, atomic promotion, derivation checks | `local_env_attempt_write CONTEXT DESIRED RESOLVED PHASE RESOURCES PREVIOUS_SHA`; `local_env_receipt_read CONTEXT`; `local_env_receipt_promote ATTEMPT COMPOSE VITE`; `local_env_receipt_update_api_container RECEIPT CONTAINER_ID UPDATED_AT` |
| `scripts/local-env/lock.sh` | one lifecycle lock per derived environment | `local_env_lock_acquire CONTEXT`; `local_env_lock_release`; lock FD remains held by caller |
| `scripts/local-env/process.sh` | `/proc` identity and safe process-group stop | `local_env_process_observe PID`; `local_env_process_owned RECEIPT`; `local_env_process_stop RECEIPT` |
| `scripts/local-env/compose.sh` | fixed file ordering and scoped Docker invocation | `local_env_compose CONTEXT RESOLVED -- COMPOSE_ARGS...` |
| `scripts/local-env/web.sh` | lock-aware `npm ci` and explicit asset synchronization | `local_env_web_prepare CONTEXT RESOLVED CLEAN_FLAG` |
| `scripts/local-env/vite.sh` | owned Vite start/reuse/restart/readiness and URL parsing | `local_env_vite_ensure CONTEXT RESOLVED PRIOR_RECEIPT` emits Vite JSON |
| `scripts/local-env/toolkit.sh` | toolkit override on/refresh/status/off in managed API source | `local_env_toolkit_on`; `local_env_toolkit_refresh`; `local_env_toolkit_off` |
| `scripts/local-env/snapshot.sh` | environment-qualified Redis save/restore | `local_env_snapshot_save`; `local_env_snapshot_restore` |
| `scripts/local-env/supervisor.sh` | ordered prepare/up/status/down and bounded cleanup | `local_env_prepare`; `local_env_up_prepared`; `local_env_up`; `local_env_status`; `local_env_down` |
| `scripts/local-env/index.sh` | source the modules in dependency order; no behavior | sourced by both facades and tests |

All functions use the `local_env_` prefix, print diagnostics to stderr, and reserve stdout for JSON or documented command output. JSON is constructed and validated with `jq`; no shell code consumes JSON through `eval`.

Desired and resolved JSON are locked before task execution:

```json
{
  "schema": "local-env-desired/v1",
  "contextSha256": "64hex",
  "manifest": {"path": "/workspace/envs/dev.env", "sha256": "64hex"},
  "apiPort": 8080,
  "snapshot": null,
  "selectors": {
    "api": {"kind": "ref", "value": "dev"},
    "web": {"kind": "ref", "value": "dev"},
    "deployment": {"kind": "ref", "value": "main"},
    "assets": {"kind": "ref", "value": "main"},
    "toolkit": null
  }
}
```

```json
{
  "schema": "local-env-resolved/v1",
  "contextSha256": "64hex",
  "desiredSha256": "64hex",
  "sources": {
    "api": {"repository": "rpg-api", "request": {"kind": "ref", "value": "dev"}, "sha": "40hex", "path": "/runtime/api", "ownership": "managed", "trackedClean": true},
    "web": {"repository": "rpg-dnd5e-web", "request": {"kind": "ref", "value": "dev"}, "sha": "40hex", "path": "/runtime/web", "ownership": "managed", "trackedClean": true},
    "deployment": {"repository": "rpg-deployment", "request": {"kind": "ref", "value": "main"}, "sha": "40hex", "path": "/runtime/deployment", "ownership": "managed", "trackedClean": true},
    "assets": {"repository": "rpg-game-assets", "request": {"kind": "ref", "value": "main"}, "sha": "40hex", "path": "/runtime/assets", "ownership": "managed", "trackedClean": true},
    "toolkit": null
  }
}
```

Every selector/source object has exactly those keys; toolkit is either `null` or the same closed shapes. `contextSha256` and `desiredSha256` are SHA256 of canonical compact JSON.

Receipt v1 is locked to this exact shape:

```json
{
  "schema": "local-env-receipt/v1",
  "workspace": "/canonical/game-dev",
  "environment": "dev",
  "mode": "dev",
  "manifest": {"path": "/canonical/game-dev/envs/dev.env", "sha256": "64hex"},
  "runtime": {"project": "rpg-dev-8hex", "image": "rpg-api:local-dev-8hex", "apiPort": 8080, "apiUrl": "http://localhost:8080"},
  "sources": {
    "api": {"repository": "rpg-api", "request": {"kind": "ref", "value": "dev"}, "sha": "40hex", "path": "/runtime/api", "ownership": "managed", "trackedClean": true},
    "web": {"repository": "rpg-dnd5e-web", "request": {"kind": "ref", "value": "dev"}, "sha": "40hex", "path": "/runtime/web", "ownership": "managed", "trackedClean": true},
    "deployment": {"repository": "rpg-deployment", "request": {"kind": "ref", "value": "main"}, "sha": "40hex", "path": "/runtime/deployment", "ownership": "managed", "trackedClean": true},
    "assets": {"repository": "rpg-game-assets", "request": {"kind": "ref", "value": "main"}, "sha": "40hex", "path": "/runtime/assets", "ownership": "managed", "trackedClean": true},
    "toolkit": null
  },
  "compose": {"files": [], "containerIds": {"redis": "hex", "dnd-database": "hex", "dnd-api": "hex", "rpg-api": "hex", "envoy": "hex", "nginx-local": "hex"}},
  "vite": {"pid": 1, "pgid": 1, "startTicks": "1", "token": "uuid", "cwd": "/web", "command": "npm run dev", "apiUrl": "http://localhost:8080", "localUrl": "http://localhost:3001/", "lockSha256": "64hex", "configSha256": "64hex", "log": "/runtime/vite.log"},
  "createdAt": "RFC3339",
  "updatedAt": "RFC3339"
}
```

Receipt and attempt `sources` are exact copies of resolved `.sources`, with the same keys and closed child shapes; they never rename `api` to `rpg-api`. Attempt journals have exact top-level keys `schema`, `workspace`, `environment`, `mode`, `manifest`, `runtime`, `sources`, `phase`, `createdResources`, `previousReceiptSha256`, `createdAt`, and `updatedAt`. `schema` is `local-env-attempt/v1`; `phase` is one of `prepared`, `override-off`, `sources-resolved`, `override-on`, `image-built`, `compose-started`, `snapshot-restored`, `api-ready`, `web-prepared`, `vite-started`, or `ready`. `createdResources` is exactly `{"composeStarted":false,"vite":null,"override":null}` where `vite` is either null or the closed Vite child object and `override` is either null or `{"apiPath":"...","toolkitPath":"...","active":true}`. `previousReceiptSha256` is null or 64 lowercase hex. Container ID syntax is checked here; live existence, labels, and port ownership are checked later through Docker inspection. Unknown keys or a derived-field mismatch fail closed.

Every mutating lifecycle command acquires the environment lock before reading/writing attempts, advancing managed worktrees, reconciling Compose, or touching Vite. Lock contention fails before mutation. Different derived environments use different lock files and may proceed concurrently. Lock FD 9 is held by the launcher only; every long-lived child explicitly closes FD 9 before `exec`.

Status exit codes are fixed:

- `0`: requested environment is verified running, or registry listing succeeded.
- `1`: valid environment is absent, starting/orphaned, drifted, unhealthy, or stale.
- `2`: unsafe/malformed input, receipt, manifest, source, or ownership evidence.

Snapshot names and optional `SNAPSHOT` values match `^[A-Za-z0-9][A-Za-z0-9._-]*$`, must not equal `.` or `..`, and resolve under `envs/snapshots/<environment>/`.

---

### Task 1: Establish Owning Issues and Isolated Worktrees

**Files:**
- No source files
- Record issue/branch/worktree facts in `rpg-project#221` and PR `#222`

**Interfaces:**
- Consumes: approved design and this plan
- Produces: one issue number and clean worktree per provider/consumer; two verification issues

- [ ] **Step 1: Create exact owning issues only when absent**

Use exact-title lookup before creation:

```bash
ensure_issue() {
  local repo="$1" title="$2" body_file="$3" number
  number="$(gh issue list -R "$repo" --state all --search "in:title \"$title\"" \
    --limit 100 --json number,title | jq -r --arg title "$title" \
    '[.[]|select(.title==$title)] | if length==0 then empty elif length==1 then .[0].number else error("duplicate exact issues") end')"
  if [ -z "$number" ]; then
    gh issue create -R "$repo" --title "$title" --body-file "$body_file" >/dev/null
    number="$(gh issue list -R "$repo" --state all --search "in:title \"$title\"" \
      --limit 100 --json number,title | jq -r --arg title "$title" \
      '[.[]|select(.title==$title)] | if length==1 then .[0].number else error("issue creation did not converge") end')"
  fi
  printf '%s\n' "$number"
}
```

Create these exact records with `<!-- pih-dispatch:v1 -->` bodies, acceptance from the relevant plan unit, parent `rpg-project#221`, Project 19 routing, and the signature footnote:

```text
KirkDiggler/rpg-deployment  Isolate named local Compose environments
KirkDiggler/rpg-dnd5e-web  Accept an explicit non-mutating local asset source
KirkDiggler/game-dev        Start isolated named local games with one command
KirkDiggler/rpg-project     Verify isolated local games on native Ubuntu
KirkDiggler/rpg-project     Verify isolated local games on Ubuntu WSL2
```

Generate each body from this closed case table; do not improvise scope:

```bash
write_body() {
  local key="$1" out="$2" goal outcome acceptance
  case "$key" in
    deployment)
      goal='Fixed local Compose names and host ports prevent named branch stacks from coexisting.'
      outcome='Expose project-scoped local Compose with an explicit API image and Envoy host port; publish no other service.'
      acceptance='Rendered normal and toolkit stacks have no container_name, exactly one Envoy publication, distinct project networks, selected API image, and unchanged internal DNS.' ;;
    web)
      goal='Asset synchronization pulls an incidental sibling checkout and cannot consume a manifest-resolved asset worktree safely.'
      outcome='Accept RPG_GAME_ASSETS_PATH and copy it exactly without Git or network operations while retaining the unset fallback.'
      acceptance='Explicit mode validates harness/models/synty, preserves rsync --delete, deletes stale destination bytes, and invokes no git/ssh/curl/wget.' ;;
    game-dev)
      goal='Starting the real local game requires multiple commands and fixed runtime ownership cannot isolate branch games.'
      outcome='Resolve named manifests, start isolated Compose plus owned background Vite, record receipts, and share lifecycle with the six-command toolkit facade.'
      acceptance='Deterministic contracts and native/WSL evidence prove one-command startup, two simultaneous environments, Vite reuse, named down, ref/path sources, and fail-closed ownership.' ;;
    native)
      goal='The isolated runtime requires independent live proof on native Ubuntu.'
      outcome='Attest the immutable final head through positive two-environment, toolkit/HMR, negative safety, and cleanup matrices.'
      acceptance='All Task 14 evidence is attached and independently reviewed; hosted CI is not substituted.' ;;
    wsl)
      goal='The isolated runtime requires independent live proof on Ubuntu WSL2 with a reachable Docker-compatible daemon.'
      outcome='Execute the self-contained immutable pickup inside the WSL filesystem and attest the same contract independently.'
      acceptance='All Task 15 evidence is attached and independently reviewed; no checkout under /mnt/c is used.' ;;
    *) return 64 ;;
  esac
  cat >"$out" <<EOF
<!-- pih-dispatch:v1 -->
## Goal / symptom
$goal

## Desired outcome
$outcome

## Contract boundaries
Implement or verify only the owning lane in rpg-project#221 and its approved design/plan. Preserve primary checkouts and unowned processes/resources. Kirk alone merges.

## Acceptance
- [ ] $acceptance

## Verification evidence
Report exact refs/SHAs, commands with exit codes, changed files or live artifacts, independent review, cleanup, and residual risks.

## Related / dependencies
Parent: https://github.com/KirkDiggler/rpg-project/issues/221

— asset-pipeline agent, on behalf of KirkDiggler
EOF
}

body_dir="$(mktemp -d)"
write_body deployment "$body_dir/deployment.md"
write_body web "$body_dir/web.md"
write_body game-dev "$body_dir/game-dev.md"
write_body native "$body_dir/native.md"
write_body wsl "$body_dir/wsl.md"

deployment_issue="$(ensure_issue KirkDiggler/rpg-deployment 'Isolate named local Compose environments' "$body_dir/deployment.md")"
web_issue="$(ensure_issue KirkDiggler/rpg-dnd5e-web 'Accept an explicit non-mutating local asset source' "$body_dir/web.md")"
game_dev_issue="$(ensure_issue KirkDiggler/game-dev 'Start isolated named local games with one command' "$body_dir/game-dev.md")"
native_issue="$(ensure_issue KirkDiggler/rpg-project 'Verify isolated local games on native Ubuntu' "$body_dir/native.md")"
wsl_issue="$(ensure_issue KirkDiggler/rpg-project 'Verify isolated local games on Ubuntu WSL2' "$body_dir/wsl.md")"
```

Set Project 19 fields through exact issue `projectItems(first:20)` readback:

```text
Provider/consumer: Status=Todo, Feature=Infra, Kind=Build, Team=Platform
Verification:      Status=Todo, Feature=Infra, Kind=Verify, Team=Cross-team
```

- [ ] **Step 2: Link the five issues beneath `rpg-project#221` and verify parentage**

For each repository/number, read the numeric REST issue ID and run:

```bash
sub_issue_id="$(gh api "repos/$owner/$repo/issues/$number" --jq .id)"
gh api --method POST repos/KirkDiggler/rpg-project/issues/221/sub_issues \
  -F sub_issue_id="$sub_issue_id" >/dev/null
```

Add each exact issue URL with `gh project item-add 19 --owner KirkDiggler --format json`, derive Project/field/option IDs by exact names from `gh project field-list 19 --owner KirkDiggler --format json`, and set all four single-select values with `gh project item-edit`. Verify each exact issue through GraphQL `projectItems(first:20)`, select `.project.number==19`, and require one item with the expected Status/Feature/Kind/Team. Query parent `#221` sub-issues and require the five repository-qualified numbers exactly once.

Move Unit A/B items from Todo to In Progress immediately before their worktree agents start. Unit C moves only after Task 4's merge gate. Verification items move when their live task starts.

- [ ] **Step 3: Create clean implementation worktrees from live bases**

```bash
git -C ~/game-dev/rpg-deployment fetch origin main
git -C ~/game-dev/rpg-dnd5e-web fetch origin dev
git -C ~/game-dev fetch origin main

git -C ~/game-dev/rpg-deployment worktree add \
  -b "feat/${deployment_issue}-isolated-local-compose" \
  "$HOME/game-dev/.pi-worktrees/rpg-deployment-${deployment_issue}" origin/main

git -C ~/game-dev/rpg-dnd5e-web worktree add \
  -b "feat/${web_issue}-explicit-local-assets" \
  "$HOME/game-dev/.pi-worktrees/rpg-dnd5e-web-${web_issue}" origin/dev
```

Do not create the `game-dev` implementation worktree until Units A and B are merged, because the approved design requires the consumer to start from merged provider contracts.

- [ ] **Step 4: Snapshot preservation evidence**

For every primary checkout, record `HEAD`, branch/detached state, `git status --porcelain=v1`, staged/unstaged binary diffs, worktree list, and untracked path hashes before any agent launch. Store evidence outside repositories. Repeat and byte-compare the appropriate primary snapshot after each provider task/PR so preservation is an explicit provider gate, not only a final assumption.

### Task 2: Unit A — Add the Deployment Provider Contract

**Files:**
- Create: `tests/local-compose-contract.sh`
- Create: `.github/workflows/local-compose-contract.yml`
- Modify: `docker-compose.local-dev.yml:4-105`
- Modify: `docker-compose.api.yml:4-50`
- Modify: `docker-compose.local-api-src.yml:1-24`
- Modify: `Makefile:1-19`
- Modify: `LOCAL_DEV.md:147-224,278-318`
- Modify: `LOCAL_DEV_WORKFLOW.md:55-89`
- Modify: `QUICKSTART.md:7-212`
- Modify: `MODULAR_DEPLOYMENT.md:7-132`

**Interfaces:**
- Consumes: `RPG_API_HOST_PORT`, `RPG_API_IMAGE`, Docker Compose project name
- Produces: normal and toolkit rendered stacks with no fixed container names, exactly one host publication, and unchanged service DNS

- [ ] **Step 1: Write the failing Compose contract**

Create `tests/local-compose-contract.sh` with two render helpers and assertions:

```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

render() {
  local project="$1" image="$2" port="$3" mode="$4"
  local files=(-f "$ROOT/docker-compose.local-dev.yml")
  [ "$mode" = toolkit ] && files+=(-f "$ROOT/docker-compose.api.yml")
  files+=(-f "$ROOT/docker-compose.local-api-src.yml")
  RPG_API_IMAGE="$image" RPG_API_HOST_PORT="$port" \
    docker compose -p "$project" "${files[@]}" config --format json
}

assert_config() {
  local json="$1" project="$2" image="$3" port="$4"
  jq -e --arg project "$project" --arg image "$image" --arg port "$port" '
    .name == $project and
    .services["rpg-api"].image == $image and
    ([.services | to_entries[] | select(.value.container_name? != null)] | length) == 0 and
    ([.services | to_entries[] | .value.ports[]?] | length) == 1 and
    .services.envoy.ports[0].target == 8080 and
    .services.envoy.ports[0].published == $port and
    .networks["rpg-network"].name == ($project + "_rpg-network") and
    .services["rpg-api"].environment.REDIS_ADDR == "redis:6379" and
    (.services["rpg-api"].environment.DND5E_API_URL | startswith("http://dnd-api:3000/"))
  ' <<<"$json" >/dev/null
}

a="$(render rpg-contract-a rpg-api:test-a 18080 normal)"
b="$(render rpg-contract-b rpg-api:test-b 18081 toolkit)"
assert_config "$a" rpg-contract-a rpg-api:test-a 18080
assert_config "$b" rpg-contract-b rpg-api:test-b 18081
[ "$(jq -r '.networks["rpg-network"].name' <<<"$a")" != \
  "$(jq -r '.networks["rpg-network"].name' <<<"$b")" ]
printf 'PASS: isolated local Compose contract\n'
```

- [ ] **Step 2: Run RED and retain the failure**

```bash
bash tests/local-compose-contract.sh
```

Expected: nonzero because fixed `container_name`, extra host ports, literal Envoy 8080, and literal `rpg-api:local` violate the assertions.

- [ ] **Step 3: Implement the minimal provider seam**

In `docker-compose.local-dev.yml`:

```yaml
services:
  redis:
    image: redis:7-alpine
    # no container_name and no ports

  dnd-api:
    image: ghcr.io/5e-bits/5e-srd-api:4.2.1
    # no container_name and no ports

  rpg-api:
    # retain current image/tag, environment, health, depends_on, and network
    # remove container_name only

  envoy:
    ports:
      - "${RPG_API_HOST_PORT:-8080}:8080"

  nginx-local:
    # retain service/internal route; remove container_name and ports
```

Remove `container_name` from `dnd-database`, `rpg-api`, and `envoy` too. In `docker-compose.api.yml`, remove the two fixed names. In `docker-compose.local-api-src.yml`:

```yaml
services:
  rpg-api:
    image: ${RPG_API_IMAGE:-rpg-api:local}
    pull_policy: never
```

Defaults preserve existing single-stack manual commands while named consumers supply explicit values.

- [ ] **Step 4: Run GREEN and syntax/config checks**

```bash
bash tests/local-compose-contract.sh
docker compose -f docker-compose.local-dev.yml config --quiet
RPG_API_IMAGE=rpg-api:test RPG_API_HOST_PORT=18080 \
  docker compose -p rpg-contract \
  -f docker-compose.local-dev.yml \
  -f docker-compose.api.yml \
  -f docker-compose.local-api-src.yml config --quiet
```

Expected: all zero; no containers start.

- [ ] **Step 5: Wire CI and docs**

Add to `Makefile`:

```make
.PHONY: local-prod local-prod-down local-prod-logs test

test:
	./scripts/test-toolkit-override-lab.sh
	./tests/local-compose-contract.sh
```

Add a pull-request workflow that checks out the repository and runs:

```yaml
- name: Verify isolated local Compose contract
  run: make test
```

Update `LOCAL_DEV.md`, `LOCAL_DEV_WORKFLOW.md`, `QUICKSTART.md`, and `MODULAR_DEPLOYMENT.md` to use `docker compose -p NAME ... logs SERVICE`, state that only Envoy is published by this local contract, remove fixed-name/direct Redis/D&D/nginx instructions for this path, and explicitly mark `docker-compose.local-src.yml` as a legacy containerized-web path outside the named supervisor.

- [ ] **Step 6: Run the provider gate and commit**

```bash
make test
git diff --check
git status --short
git add docker-compose.local-dev.yml docker-compose.api.yml \
  docker-compose.local-api-src.yml tests/local-compose-contract.sh \
  .github/workflows/local-compose-contract.yml Makefile \
  LOCAL_DEV.md LOCAL_DEV_WORKFLOW.md QUICKSTART.md MODULAR_DEPLOYMENT.md
git commit -m "feat: isolate named local compose environments"
```

- [ ] **Step 7: Independent review and PR**

Commission fresh read-only reviewers for Compose correctness/compatibility and contract-test quality. Apply accepted findings with one writer, rerun `make test`, push, and open the one issue-linked PR to `main`. Do not merge.

### Task 3: Unit B — Add Explicit Non-Mutating Web Asset Sync

**Files:**
- Create: `scripts/sync-synty-assets.test.ts`
- Modify: `scripts/sync-synty-assets.sh:1-47`
- Modify: `vite.config.ts:52-67`
- Modify: `.github/workflows/ci.yml:37-54`

**Interfaces:**
- Consumes: optional `RPG_GAME_ASSETS_PATH`
- Produces: exact `rsync -a --delete` from the supplied path with zero Git/network operations; unset variable retains legacy behavior

- [ ] **Step 1: Install clean dependencies in the isolated web worktree**

```bash
HUSKY=0 npm ci
```

Expected: zero and no tracked diff.

- [ ] **Step 2: Write RED fixture tests**

Create `scripts/sync-synty-assets.test.ts` using node Vitest. The test copies the production shell script into a temporary `web/scripts/` tree so its path-derived destination is isolated:

```ts
// @vitest-environment node
import { afterEach, describe, expect, it } from 'vitest';
import { chmod, cpFile, mkdtemp, mkdir, readFile, rm, writeFile } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { join } from 'node:path';
import { tmpdir } from 'node:os';

const roots: string[] = [];
afterEach(async () => Promise.all(roots.splice(0).map((p) => rm(p, { recursive: true, force: true }))));

async function fixture() {
  const root = await mkdtemp(join(tmpdir(), 'asset-sync-'));
  roots.push(root);
  const web = join(root, 'web');
  const assets = join(root, 'assets');
  const bin = join(root, 'bin');
  await mkdir(join(web, 'scripts'), { recursive: true });
  await mkdir(join(web, 'public/models/synty'), { recursive: true });
  await mkdir(join(assets, 'harness/models/synty'), { recursive: true });
  await mkdir(bin);
  await cpFile(join(process.cwd(), 'scripts/sync-synty-assets.sh'), join(web, 'scripts/sync-synty-assets.sh'));
  const forbiddenLog = join(root, 'forbidden.log');
  for (const command of ['git', 'ssh', 'curl', 'wget']) {
    await writeFile(join(bin, command), `#!/bin/sh\nprintf '%s %s\\n' "${command}" "$*" >> "${forbiddenLog}"\nexit 97\n`);
    await chmod(join(bin, command), 0o755);
  }
  return { root, web, assets, bin, forbiddenLog };
}
```

Add cases that:

```ts
it('copies exact explicit assets, deletes stale files, and never invokes git', async () => {
  const f = await fixture();
  await writeFile(join(f.assets, 'harness/models/synty/keep.glb'), 'new-bytes');
  await writeFile(join(f.web, 'public/models/synty/stale.glb'), 'stale');
  const run = spawnSync('sh', [join(f.web, 'scripts/sync-synty-assets.sh')], {
    env: { ...process.env, RPG_GAME_ASSETS_PATH: f.assets, PATH: `${f.bin}:${process.env.PATH}` },
    encoding: 'utf8',
  });
  expect(run.status).toBe(0);
  expect(await readFile(join(f.web, 'public/models/synty/keep.glb'), 'utf8')).toBe('new-bytes');
  expect(existsSync(join(f.web, 'public/models/synty/stale.glb'))).toBe(false);
  expect(existsSync(f.forbiddenLog)).toBe(false);
});
```

Also assert missing `harness/models/synty` fails before destination mutation, and unset `RPG_GAME_ASSETS_PATH` still enters the legacy sibling path. Use the strict fixture `PATH` to prove explicit mode invokes none of `git`, `ssh`, `curl`, or `wget`.

- [ ] **Step 3: Run RED**

```bash
npm exec vitest run scripts/sync-synty-assets.test.ts
```

Expected: explicit-path tests fail because the script ignores the variable and attempts sibling clone/pull.

- [ ] **Step 4: Implement explicit mode without changing fallback**

Use this branch before the existing sibling clone/pull block:

```sh
if [ -n "${RPG_GAME_ASSETS_PATH:-}" ]; then
  ASSETS_DIR=$RPG_GAME_ASSETS_PATH
  echo "Using explicit rpg-game-assets source at $ASSETS_DIR"
else
  ASSETS_DIR="$PARENT_DIR/rpg-game-assets"
  if [ -d "$ASSETS_DIR/.git" ]; then
    echo "Found existing rpg-game-assets checkout at $ASSETS_DIR — pulling latest..."
    git -C "$ASSETS_DIR" pull
  else
    echo "Cloning rpg-game-assets into $ASSETS_DIR..."
    if ! git clone "$ASSETS_REPO_URL" "$ASSETS_DIR"; then
      echo "SSH clone failed, retrying over HTTPS..."
      git clone "$ASSETS_REPO_URL_HTTPS" "$ASSETS_DIR"
    fi
  fi
fi
```

Keep source validation and `rsync -a --delete` after the branch. Explicit mode contains no Git or network command. Add `scripts/sync-synty-assets.test.ts` to the existing Vitest include without broadening discovery to unrelated script tests:

```ts
include: ['src/**/*.test.{ts,tsx}', 'scripts/sync-synty-assets.test.ts'],
```

- [ ] **Step 5: Run focused and full gates**

```bash
npm exec vitest run scripts/sync-synty-assets.test.ts
npm run format:check
npm run lint
npm run typecheck
npm run ci-check
```

Expected: all zero. Modify the CI test job to run `npm run test:run` instead of echoing a placeholder, then re-run the focused test.

- [ ] **Step 6: Commit, independently review, and open the provider PR**

```bash
git diff --check
git add scripts/sync-synty-assets.sh scripts/sync-synty-assets.test.ts \
  vite.config.ts .github/workflows/ci.yml
git commit -m "feat: accept an explicit local asset source"
```

Use fresh reviewers for explicit-mode non-mutation and test quality, apply accepted findings with one writer, rerun gates, push, and open the one issue-linked PR to `dev`. Do not merge.

### Task 4: Provider Merge and Immutable Consumer Gate

**Files:**
- No consumer source files yet
- Update signed evidence on `rpg-project#221` / PR `#222`

**Interfaces:**
- Consumes: Kirk-merged Unit A and B PRs
- Produces: immutable deployment/web provider SHAs available from public base refs

- [ ] **Step 1: Verify both PRs are reviewed, green, and merge-ready**

Read their immutable heads, rerun provider tests from clean worktrees, and publish independent signed reviews. Ask Kirk to merge; do not merge for him.

- [ ] **Step 2: Record and verify merge SHAs**

```bash
git -C ~/game-dev/rpg-deployment fetch origin main
git -C ~/game-dev/rpg-dnd5e-web fetch origin dev

deployment_merge_sha="$(gh pr view "$deployment_pr" -R KirkDiggler/rpg-deployment --json mergeCommit --jq .mergeCommit.oid)"
web_merge_sha="$(gh pr view "$web_pr" -R KirkDiggler/rpg-dnd5e-web --json mergeCommit --jq .mergeCommit.oid)"

git -C ~/game-dev/rpg-deployment merge-base --is-ancestor "$deployment_merge_sha" origin/main
git -C ~/game-dev/rpg-dnd5e-web merge-base --is-ancestor "$web_merge_sha" origin/dev
```

- [ ] **Step 3: Re-run provider contracts from merged commits**

Use clean detached worktrees at the merge SHAs, run `make test` for deployment and `HUSKY=0 npm ci` plus the web focused/full gates, then remove only those owned detached verification worktrees.

- [ ] **Step 4: Create the `game-dev` implementation worktree**

```bash
git -C ~/game-dev fetch origin main
git -C ~/game-dev worktree add \
  -b "feat/${game_dev_issue}-isolated-local-environments" \
  "$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}" origin/main
```

Record provider merge SHAs in the game-dev issue and branch handoff. Unit C starts only now.

### Task 5: Unit C1 — Common Identity and Strict Manifests

**Files:**
- Create: `scripts/local-env/common.sh`
- Create: `scripts/local-env/manifest.sh`
- Create: `scripts/local-env/index.sh`
- Create: `tests/local-env-manifest-contract.sh`
- Modify: `.gitignore`

**Interfaces:**
- Produces: validated context JSON and desired-state JSON consumed by every later Unit C task

- [ ] **Step 1: Write manifest/name RED tests**

`tests/local-env-manifest-contract.sh` must source `index.sh`, create a temporary workspace, and assert:

```bash
assert_ok_name dev
assert_ok_name feature-42
assert_ok_name local/toolkit-42
assert_ok_name feature_name
assert_ok_name feature.name
assert_bad_name '../escape'
assert_bad_name 'local/../escape'
assert_bad_name 'Upper'
assert_bad_name '.hidden'
assert_bad_name '_hidden'
assert_bad_name '-hidden'
assert_bad_name 'other/feature'
assert_bad_name 'local/'
assert_bad_name 'local/one/two'
```

Write a valid manifest with all required refs/port, tab-only blank lines, and a tab-indented comment, then assert exact JSON values/digest. Assert ports 1024 and 65535 pass; 1023, 65536, `+8080`, `-1`, `8x80`, and empty fail. Add refusals for unknown/duplicate keys, leading/trailing value whitespace, NUL or any other control byte, empty required values, unknown mode, missing toolkit selector in toolkit mode, unsafe `SNAPSHOT`, and every legacy `*_BRANCH` key. Add a hostile literal value `$(touch marker)` and assert no marker is created.

Before any lifecycle mutation, characterize prerequisites: missing `git`, `docker`, `go`, `node`, `npm`, `rsync`, `jq`, `ssh`, `ss`, `curl`, `setsid`, `flock`, `timeout`, `sha256sum`, or `od`; unreachable `docker info`; and Docker Compose below 2.16 all fail. The test uses fake commands and proves no fetch/build/Compose/npm/process/state action follows a prerequisite failure.

- [ ] **Step 2: Run RED**

```bash
bash tests/local-env-manifest-contract.sh
```

Expected: nonzero because modules/functions do not exist.

- [ ] **Step 3: Implement context identity**

`local_env_context ROOT NAME MODE` canonicalizes `ROOT`, validates `NAME`/`MODE`, hashes the unsanitized canonical root, full normalized name, and schema version, and emits:

```json
{"schema":"local-env-context/v1","root":"/canonical/root","environment":"dev","mode":"dev","slug":"dev","hash":"8hex","runtimeDir":"/canonical/root/.runtime/environments/dev","worktreeDir":"/canonical/root/.runtime/worktrees/dev","composeProject":"rpg-dev-8hex","apiImage":"rpg-api:local-dev-8hex"}
```

Sanitize `.` and `/` to `-` only for Docker names; include the original name in the hash. Limit the final Compose project to 63 characters and validate it against `^[a-z0-9][a-z0-9_-]*$`.

- [ ] **Step 4: Implement strict manifest data parsing**

Use a Bash associative array and line parser, never `source`. Bash cannot retain NUL in a variable, so reject byte 0 before line parsing using `od`; then ignore whitespace-only/comment lines before rejecting control bytes in data lines. This preserves tab-indented comments and tab-only blank lines required by the design:

```bash
if od -An -t u1 "$manifest_path" | tr -s ' ' '\n' | grep -qx '0'; then
  local_env_fail "manifest contains NUL"
fi
while IFS= read -r line || [ -n "$line" ]; do
  [[ "$line" =~ ^[[:space:]]*$ ]] && continue
  [[ "$line" =~ ^[[:space:]]*# ]] && continue
  if printf '%s' "$line" | LC_ALL=C grep -q '[[:cntrl:]]'; then
    local_env_fail "manifest data contains a control byte"
  fi
  [[ "$line" == *=* ]] || local_env_fail "invalid manifest line"
  key="${line%%=*}"
  value="${line#*=}"
  [[ "$key" =~ ^[A-Z][A-Z0-9_]*$ ]] || local_env_fail "invalid manifest key"
  [[ "$key" != *[[:space:]]* ]] || local_env_fail "manifest key whitespace is not allowed"
  [[ "$value" != [[:space:]]* && "$value" != *[[:space:]] ]] || local_env_fail "manifest value edge whitespace is not allowed"
  local_env_manifest_key_allowed "$key" || local_env_fail "unknown manifest key: $key"
  [[ -z "${seen[$key]+x}" ]] || local_env_fail "duplicate manifest key: $key"
  seen[$key]=1
  values[$key]="$value"
done <"$manifest_path"
```

Emit the exact `local-env-desired/v1` JSON contract above with selector objects `{kind:"path"|"ref",value:"..."}`; path wins when both keys exist, but validation of the selected path belongs to Task 6. `local_env_require_prerequisites` validates the complete tool set, `docker info`, and parsed Compose version before supervisor mutation.

- [ ] **Step 5: Run GREEN and commit**

```bash
bash tests/local-env-manifest-contract.sh
bash -n scripts/local-env/*.sh
git diff --check
git add scripts/local-env/common.sh scripts/local-env/manifest.sh \
  scripts/local-env/index.sh tests/local-env-manifest-contract.sh .gitignore
git commit -m "feat: parse isolated environment manifests safely"
```

`.gitignore` must add only root-anchored `/.runtime/` and `/envs/local/`.

### Task 6: Unit C2 — Resolve Paths and Pushed Refs Without Touching Primary Worktrees

**Files:**
- Create: `scripts/local-env/sources.sh`
- Create: `tests/local-env-sources-contract.sh`
- Modify: `scripts/local-env/index.sh`

**Interfaces:**
- Consumes: context and desired JSON
- Produces: resolved source JSON keyed by API, web, deployment, assets, and optional toolkit

- [ ] **Step 1: Build RED Git fixtures**

The test creates canonical bare repositories and primary clones. Give clones exact SSH origins while routing fetches locally through a fixture-only Git config:

```bash
git config --file "$fixture_gitconfig" \
  "url.file://$bare_root/.insteadOf" 'git@github.com:KirkDiggler/'
export GIT_CONFIG_GLOBAL="$fixture_gitconfig"
```

Snapshot each ref-backed primary clone's `HEAD`, branch, index hash, tracked binary diff, untracked list/bytes, worktree list, literal origin config, `FETCH_HEAD`, tags, and remote-tracking refs. Assert:

- exact path root accepted without a sibling primary clone, including tracked/untracked local edits; subdirectory/wrong origin/missing root rejected;
- selected `PATH` wins and an invalid selected path never falls back to `REF`;
- short branch and tag refs resolve detached to exact immutable SHA;
- same short head/tag name is ambiguous and rejected, while explicit `refs/heads/name` and `refs/tags/name` each succeed;
- full 40-hex reachable commit accepted and unreachable 40-hex commit rejected;
- managed worktree with unexplained tracked dirt is rejected;
- primary snapshots remain identical except launcher ref/object/worktree metadata explicitly allowed by the design.

- [ ] **Step 2: Run RED**

```bash
bash tests/local-env-sources-contract.sh
```

Expected: missing source resolver.

- [ ] **Step 3: Implement source preflight before mutation**

`local_env_sources_preflight` validates a canonical bootstrap clone only for each selected ref. Path-only selectors need no sibling primary clone; they validate their own worktree root, literal exact SSH origin from `git config --get remote.origin.url`, and repository-specific layout. All selected sources preflight before any fetch or worktree creation. Toolkit mode rejects API selector kind `path` here.

Repository layout pins:

```text
rpg-api:          Dockerfile, cmd/sandboxseed; toolkit mode also Dockerfile.local-toolkit and scripts/toolkit-local-override.sh
rpg-dnd5e-web:    package.json, package-lock.json, scripts/sync-synty-assets.sh, vite.config.ts
rpg-deployment:   docker-compose.local-dev.yml, docker-compose.local-api-src.yml; toolkit mode also docker-compose.api.yml
rpg-game-assets:  harness/models/synty
rpg-toolkit:      rulebooks/dnd5e/go.mod
```

- [ ] **Step 4: Implement ref resolution and managed worktrees**

Resolve short head/tag names with `git ls-remote`, reject ambiguity, and honor explicit `refs/heads/...` / `refs/tags/...`. Fetch with `--no-write-fetch-head --no-tags` and a forced refspec into a hash-only namespace such as `refs/local-env/<context-hash>/<repo-hash>`; verify the fetched commit equals the remote observation. This must not alter `FETCH_HEAD`, tags, normal remote-tracking refs, config, branch, index, or worktree bytes. Then create/update:

```text
.runtime/worktrees/<environment>/<repository>/
```

Use `git worktree add --detach PATH SHA` for first use. On update, require launcher ownership and no unexplained tracked changes before `git checkout --detach SHA`; never run `git clean`. Emit for each source:

```json
{"repository":"rpg-api","request":{"kind":"ref","value":"dev"},"sha":"40hex","path":"/runtime/worktree","ownership":"managed","trackedClean":true}
```

- [ ] **Step 5: Run GREEN and commit**

```bash
bash tests/local-env-sources-contract.sh
bash tests/local-env-manifest-contract.sh
git diff --check
git add scripts/local-env/sources.sh scripts/local-env/index.sh \
  tests/local-env-sources-contract.sh
git commit -m "feat: resolve isolated environment sources"
```

### Task 7: Unit C3 — Versioned Receipts and Process Ownership

**Files:**
- Create: `scripts/local-env/receipt.sh`
- Create: `scripts/local-env/lock.sh`
- Create: `scripts/local-env/process.sh`
- Create: `tests/local-env-state-contract.sh`
- Modify: `scripts/local-env/index.sh`

**Interfaces:**
- Consumes: context, desired, resolved sources
- Produces: validated attempt/receipt state and owned-process operations

- [ ] **Step 1: Write RED schema and ownership tests**

Create exact valid desired/resolved/receipt/attempt fixtures from the locked schemas. Assert unknown/missing keys, wrong schema, workspace/env/project/image derivation mismatch, bad manifest digest, path escape, malformed source ownership, and malformed container-ID syntax all return status 2. Live/nonexistent IDs and Docker-label ownership belong to Task 9.

Create a persistent fixture executable named `npm` and launch it from a fixture web directory using the production argv shape:

```bash
cat >"$bin/npm" <<'EOF'
#!/bin/sh
[ "$1" = run ] && [ "$2" = dev ] || exit 64
trap 'exit 0' TERM INT
while :; do sleep 1; done
EOF
chmod +x "$bin/npm"
setsid sh -c 'cd "$1" && exec env RPG_LOCAL_ENV_TOKEN="$2" "$3/npm" run dev' \
  sh "$web_dir" "$token" "$bin" &
pid=$!
```

A test trap terminates both owned and unrelated fixture groups on every exit path. Record `/proc/$pid/stat` field 22, PGID, cwd, NUL-separated argv, and `/proc/$pid/environ`. Assert exact evidence is owned; wrong PID start ticks, token, cwd, argv ending in `npm`, `run`, `dev`, or PGID is rejected; stopping exact owned evidence terminates only that group and leaves an unrelated fixture process alive.

Acquire one environment lock on FD 9, assert a second acquisition for that environment fails without state/process mutation, and assert a different environment lock succeeds concurrently. Add an interrupted-launcher case: after the launcher dies, its Vite fixture remains alive but has FD 9 closed, so a new lifecycle command can acquire the lock.

- [ ] **Step 2: Run RED**

```bash
bash tests/local-env-state-contract.sh
```

- [ ] **Step 3: Implement closed JSON state and atomic writes**

Use `jq -e` to assert exact top-level and child key sets. Write temporary JSON in the target directory, `chmod 600`, validate it, then `mv` atomically. Attempt writes update `phase`/owned resources without deleting the prior receipt. Promotion writes receipt first, then removes only the matching attempt.

- [ ] **Step 4: Implement complete Linux process identity**

`local_env_lock_acquire` uses fixed launcher FD 9 (`exec 9>LOCK; flock -n 9`); the caller holds it through the complete up/down/snapshot operation and releases it in a trap. Every spawned long-lived Vite shell starts with `exec 9>&-` so it cannot inherit the lifecycle lock.

`local_env_process_owned` requires all of:

```text
/proc/PID exists
/proc/PID/stat start ticks match
observed PGID matches and equals receipt PGID
readlink /proc/PID/cwd matches resolved web path
NUL-separated /proc/PID/cmdline ends with executable basename npm plus argv run and dev
NUL-separated /proc/PID/environ contains exact RPG_LOCAL_ENV_TOKEN
```

`local_env_process_stop` sends TERM to the negative PGID only after the full check, waits a bounded interval, rechecks identity, and may send KILL only to that still-owned group.

- [ ] **Step 5: Run GREEN and commit**

```bash
bash tests/local-env-state-contract.sh
bash tests/local-env-manifest-contract.sh
bash tests/local-env-sources-contract.sh
git diff --check
git add scripts/local-env/receipt.sh scripts/local-env/lock.sh \
  scripts/local-env/process.sh scripts/local-env/index.sh \
  tests/local-env-state-contract.sh
git commit -m "feat: bind local runtime ownership receipts"
```

### Task 8: Unit C4 — Scoped Compose, Web Preparation, and Vite

**Files:**
- Create: `scripts/local-env/compose.sh`
- Create: `scripts/local-env/web.sh`
- Create: `scripts/local-env/vite.sh`
- Create: `tests/local-env-compose-contract.sh`
- Create: `tests/local-env-web-vite-contract.sh`
- Modify: `scripts/local-env/index.sh`

**Interfaces:**
- Consumes: merged provider contracts, context, resolved sources, optional prior receipt
- Produces: scoped backend commands and verified Vite JSON

- [ ] **Step 1: Write RED Compose adapter tests**

Place fake `docker` first in `PATH` and assert exact invocation/log order for normal and toolkit modes. The adapter must execute from the resolved deployment path and supply:

```bash
env RPG_API_IMAGE="$image" RPG_API_HOST_PORT="$port" \
  docker compose --project-name "$project" \
  -f "$deployment/docker-compose.local-dev.yml" \
  -f "$deployment/docker-compose.local-api-src.yml" COMMAND...
```

Toolkit inserts `-f "$deployment/docker-compose.api.yml"` before the local-source overlay. Add a real `docker compose config --format json` provider gate against merged deployment files and assert one port/selected image.

- [ ] **Step 2: Write RED web/Vite tests**

Use fixture package-lock/config files and fake `npm`, `curl`, and asset script. Assert:

- missing/unknown dependency digest runs `HUSKY=0 npm ci` and leaves shared Git config/hooks unchanged;
- matching digest skips install;
- `--clean-web` removes only `node_modules`, never `package-lock.json`;
- asset sync receives exact `RPG_GAME_ASSETS_PATH` every up;
- ANSI-colored `Local: http://localhost:3007/` is parsed exactly;
- Vite starts via `setsid` with `VITE_API_HOST` and ownership token;
- exact unchanged metadata reuses PID/URL;
- changed API URL, web path, lock digest, or Vite config digest restarts only owned Vite;
- failed URL parse or HTTP readiness returns nonzero and removes only the test-owned process group.

- [ ] **Step 3: Run both RED suites**

```bash
bash tests/local-env-compose-contract.sh
bash tests/local-env-web-vite-contract.sh
```

- [ ] **Step 4: Implement fixed Compose file ordering**

`local_env_compose` reads project/image/port from derived JSON only. It never accepts caller project/image overrides. It supports `config`, `up -d`, `up -d --no-deps rpg-api`, `ps -q SERVICE`, `logs`, `stop`, `start`, and `down` through a guarded command tail.

- [ ] **Step 5: Implement lock-aware web preparation**

Compute SHA256 of `package-lock.json`; treat missing recorded digest as stale. Run:

```bash
env HUSKY=0 npm ci --prefix "$web_path"
env RPG_GAME_ASSETS_PATH="$assets_path" npm --prefix "$web_path" run assets:sync
```

Asset sync runs on every up, even when Vite is reused.

- [ ] **Step 6: Implement owned Vite ensure**

Launch with an isolated process group and retained log:

```bash
setsid sh -c 'exec 9>&-; cd "$1" && exec env RPG_LOCAL_ENV_TOKEN="$2" VITE_API_HOST="$3" npm run dev' \
  sh "$web_path" "$token" "$api_url" >"$vite_log" 2>&1 &
```

Poll the log for a bounded time, strip ANSI control sequences, extract only a localhost/127.0.0.1 HTTP URL, confirm `curl --fail --silent --show-error "$local_url"`, observe process identity, and emit the Vite receipt child object. Do not pass a Vite port.

- [ ] **Step 7: Run GREEN and commit**

```bash
bash tests/local-env-compose-contract.sh
bash tests/local-env-web-vite-contract.sh
bash tests/local-env-state-contract.sh
git diff --check
git add scripts/local-env/compose.sh scripts/local-env/web.sh \
  scripts/local-env/vite.sh scripts/local-env/index.sh \
  tests/local-env-compose-contract.sh tests/local-env-web-vite-contract.sh
git commit -m "feat: supervise scoped backend and vite runtimes"
```

### Task 9: Unit C5 — Ordered Supervisor, Port Safety, Status, and Down

**Files:**
- Create: `scripts/local-env/snapshot.sh`
- Create: `scripts/local-env/toolkit.sh`
- Create: `scripts/local-env/supervisor.sh`
- Create: `tests/local-env-snapshot-contract.sh`
- Create: `tests/local-env-supervisor-contract.sh`
- Modify: `scripts/local-env/index.sh`

**Interfaces:**
- Consumes: all Unit C modules from Tasks 5–8
- Produces: `local_env_prepare`, `local_env_up_prepared`, `local_env_up`, `local_env_status`, `local_env_down`

- [ ] **Step 1: Write RED lifecycle-order tests**

Fake commands log every mutation. Assert the exact first-start order:

```text
manifest -> identity and environment lock -> receipt/attempt inspection -> Envoy port ownership
-> all-source preflight -> owned old toolkit override off when present -> source resolution
-> toolkit override on when selected -> API build -> Compose up -> selected snapshot restore when requested
-> Envoy health -> npm ci when required -> assets sync -> Vite readiness
-> receipt promotion -> exact URL print
```

Port occupancy tests must show no override/fetch/worktree/build/Compose/npm/process/state mutation. Cover a Docker-published binding, an `ss` listener, and an externally reachable localhost TCP listener hidden from fake `ss` (the WSL2/Docker Desktop case). Accept an occupied port only when a validated receipt plus derived Compose project, exact Envoy container ID/labels, and published binding prove it is this environment's Envoy.

Add failure cases for override on/off, build, Compose, snapshot restore, health, asset sync, Vite, and receipt promotion. First-start cleanup uses attempt-owned resources only, including an active override; rerun failures retain the prior receipt and never destroy a previously healthy environment merely to simulate rollback. Add a moving API-ref toolkit rerun: receipt-validated old override `off` occurs before managed-worktree cleanliness/update, the new source gets `on`, and the successful receipt records the new SHA.

- [ ] **Step 2: Write RED status/down tests**

Assert status classifications/exit codes and that a receipt alone never reports running. Named down must handle both a ready receipt and a valid interruption attempt with no receipt: verify/stop exact journal-owned Vite, down only the journal project when `composeStarted`, disable only its recorded override, remove active state only after cleanup, retain logs, and never run Git. Missing/malformed/forged/stale ownership returns nonzero with zero stop/kill/down/off calls. Validate live container existence, exact Compose project/service labels, and Envoy published-port ownership here rather than in receipt syntax tests.

Write snapshot tests before supervisor implementation: safe namespace, exact project Redis service, interactive/`--yes` restore, manifest `SNAPSHOT` restored after Compose start and before API health only when `up --restore`, and no restore without the flag.

- [ ] **Step 3: Run RED**

```bash
bash tests/local-env-supervisor-contract.sh
```

- [ ] **Step 4: Implement specific-port ownership and health**

Require Docker inspection, `ss`, and a bounded localhost TCP-connect probe (`timeout 1 bash -c '</dev/tcp/127.0.0.1/PORT'`) for the configured port. Any one can establish occupancy. If occupied, validate the active receipt and use the derived Compose adapter plus exact Envoy container ID, Compose project/service labels, and Docker published-port binding to prove ownership. Every other listener/binding fails before source or override mutation.

Use the existing real health client parameterized by receipt port:

```bash
(
  cd "$api_path"
  go run ./cmd/sandboxseed --address "localhost:$api_port" --health
)
```

Retain the bounded 30 attempts / 2-second interval for toolkit compatibility. When `up --restore` and a safe manifest snapshot are selected, restore that environment's Redis after Compose is available and before this health gate. No success URL before optional restore, health, and Vite readiness.

- [ ] **Step 5: Implement attempts, rollback bounds, status, and down**

Acquire and hold the environment lock across the entire lifecycle. Write phase transitions before each external mutation. Toolkit rerun uses prior receipt/journal source data to validate and disable the owned override before managed API source advancement, records `override-off`, then records a new owned override immediately after `on`; failed first starts disable only that recorded override. On rerun failure, retain prior receipt and failed attempt. `local_env_status` verifies manifest digest, source paths/SHAs, Compose services/labels/health, API health, Vite identity/HTTP, and returns the locked status code. `local_env_down` captures source/override evidence before stopping Vite/Compose, disables the owned override before deleting state, handles valid attempt-only journals, and performs no source-resolution Git operation.

Normal readiness output is exact and URL-encoded using `jq -rn --arg value "$environment" '$value|@uri'`:

```text
Game ready: http://localhost:3001/?playerId=local%2Ffeature
API: http://localhost:8082
```

Tests use `local/feature` so an unencoded slash cannot pass.

- [ ] **Step 6: Run GREEN and commit**

```bash
bash tests/local-env-supervisor-contract.sh
bash tests/local-env-snapshot-contract.sh
for test in tests/local-env-{manifest,sources,state,compose,web-vite}-contract.sh; do bash "$test"; done
git diff --check
git add scripts/local-env/snapshot.sh scripts/local-env/toolkit.sh \
  scripts/local-env/supervisor.sh scripts/local-env/index.sh \
  tests/local-env-snapshot-contract.sh tests/local-env-supervisor-contract.sh
git commit -m "feat: reconcile owned local game environments"
```

### Task 10: Unit C6 — Adapt `dev-env.sh` and Environment-Aware Snapshots

**Files:**
- Create: `tests/dev-env-contract.sh`
- Modify: `scripts/local-env/snapshot.sh`
- Modify: `scripts/local-env/supervisor.sh`
- Modify: `scripts/dev-env.sh:1-540`

**Interfaces:**
- Produces normal CLI: `up ENV`, `down ENV`, `status [ENV]`, environment-qualified snapshots; preserves `pin`, `--root`, `--dry-run`

- [ ] **Step 1: Write RED CLI and snapshot tests**

Assert usage and dispatch exactly:

```text
up dev [--restore] [--clean-web]
down dev
status
status dev
snapshot save dev my-repro
snapshot restore dev my-repro [--yes]
pin ...
```

Omitted `down` environment fails without Docker. `status` with no env lists committed/local manifests plus receipts/journals. Invalid snapshot names/path traversal fail. Snapshot commands obtain Redis via `local_env_compose ... ps -q redis`, not `rpg-redis-dev`.

Dry-run parses, derives, validates non-mutating inputs, and prints mutation commands; it performs no fetch, managed-worktree update, build, Compose mutation, npm, process start/stop, receipt write, health success, or readiness URL.

Before thinning the legacy script, add characterization cases that already pass for `pin`: explicit modules, inferred toolkit branch, missing remote branch refusal, ahead-of-origin warning, `go get` plus `go mod tidy`, pin dry-run with no file change, and `--root` selecting a fixture workspace. Those cases must remain green after the lifecycle rewrite.

- [ ] **Step 2: Run RED**

```bash
bash tests/dev-env-contract.sh
bash tests/local-env-snapshot-contract.sh
```

- [ ] **Step 3: Wire the already-tested snapshot service into the CLI**

Save namespace remains `envs/snapshots/<environment>/<name>.rdb`. Dispatch standalone save/restore to Task 9's exact-project Redis implementation and pass `up --restore` into the supervisor before readiness. Preserve destructive confirmation and `--yes` for noninteractive standalone restore.

- [ ] **Step 4: Replace checkout-switching runtime code with thin dispatch**

Keep the current `pin_cmd` source-control workflow outside managed runtime resolution. Delete `env_var_for_repo`, sourced manifests, branch checkout/merge, fixed image/Compose paths, manual Vite next-steps, fixed Redis name, and broad status. Source `scripts/local-env/index.sh` and delegate lifecycle commands.

- [ ] **Step 5: Run GREEN and commit**

```bash
bash tests/dev-env-contract.sh
bash tests/local-env-snapshot-contract.sh
bash tests/local-env-supervisor-contract.sh
bash -n scripts/dev-env.sh scripts/local-env/*.sh
git diff --check
git add scripts/dev-env.sh scripts/local-env/snapshot.sh \
  scripts/local-env/supervisor.sh tests/dev-env-contract.sh
git commit -m "feat: make dev environments one-command runtimes"
```

### Task 11: Unit C7 — Adapt the Six-Command Toolkit Facade

**Files:**
- Modify: `scripts/local-env/toolkit.sh`
- Modify: `scripts/local-env/supervisor.sh`
- Modify: `scripts/local-env/index.sh`
- Modify: `scripts/toolkit-contributor.sh:1-332`
- Modify: `tests/toolkit-contributor-contract.sh:1-870`

**Interfaces:**
- Produces toolkit CLI `[--env NAME] bootstrap|start|refresh|seed|status|down`
- Consumes default `toolkit` manifest, configured Envoy receipt, managed API worktree, selected toolkit path/ref

- [ ] **Step 1: Extend RED contract fixtures without weakening host gates**

Preserve existing tests for literal host files, ID parsing, WSL1/non-Ubuntu refusal, Docker capability, wrong origin, existing checkout preservation, bounded health, and exactly six command cases.

Add RED cases for:

- optional `--env local/feature` before the command and default `toolkit`;
- bootstrap's exact five providers including `rpg-game-assets` on `main`;
- API path selector refusal before override/build;
- start configured port 8081, environment image/project, assets, background Vite, and actual sandbox URL;
- refresh only selected `rpg-api`, then health through receipt port, without Vite restart; inspect the replacement container's exact project/service labels and atomically update only `compose.containerIds["rpg-api"]` plus `updatedAt` while preserving Vite and `createdAt`;
- seed/status through receipt port, status beginning with exact `host mode:`;
- down exact Vite/project before owned override `off`;
- two toolkit manifests using separate managed API worktrees/ports.

- [ ] **Step 2: Run RED**

```bash
bash tests/toolkit-contributor-contract.sh
```

- [ ] **Step 3: Integrate the already-tested toolkit helpers with the facade**

Task 9's `local_env_toolkit_on` requires `.sources.api.ownership == "managed"` and runs the existing helper inside that path:

```bash
"$api_path/scripts/toolkit-local-override.sh" on \
  --target rulebooks/dnd5e --src "$toolkit_path"
```

Refresh/status/off use the same managed API path and existing ownership state. Image build uses `Dockerfile.local-toolkit` and the derived environment image. After `up -d --no-deps rpg-api` and configured-port health, refresh resolves the new `rpg-api` container ID through the exact Compose project, validates its project/service labels, and calls `local_env_receipt_update_api_container`; that atomic update preserves Vite, sources, manifest/runtime, and `createdAt`, changes only the API ID and `updatedAt`, and is revalidated before return. The facade must not bypass Task 9's environment lock, old-override-off-before-ref-advance ordering, attempt-owned override cleanup, or receipt-preserving down sequence.

- [ ] **Step 4: Thin the facade while retaining classification**

Keep `classify_toolkit_contributor_host`, literal readers, mode-aware Docker errors, exact-origin bootstrap preservation, and the six-case dispatch. Parse only optional `--env` globally. Runtime commands resolve selected paths from manifest/receipt instead of `$ROOT/rpg-*` literals and never hard-code 8080/3001.

- [ ] **Step 5: Run GREEN and commit**

```bash
bash tests/toolkit-contributor-contract.sh
bash tests/local-env-supervisor-contract.sh
bash -n scripts/toolkit-contributor.sh scripts/local-env/*.sh
git diff --check
git add scripts/toolkit-contributor.sh scripts/local-env/toolkit.sh \
  scripts/local-env/supervisor.sh scripts/local-env/index.sh \
  tests/toolkit-contributor-contract.sh
git commit -m "feat: isolate toolkit contributor runtimes"
```

### Task 12: Unit C8 — Defaults, Migration Docs, and Complete Deterministic Gate

**Files:**
- Create: `envs/toolkit.env`
- Modify: `envs/dev.env`
- Modify: `tests/bootstrap-contract.sh`
- Modify: `docs/dev-environments.md`
- Modify: `docs/toolkit-contributor-sandbox.md`
- Modify: `README.md`
- Modify: `.gitignore` if Task 5 did not complete both root-anchored rules

**Interfaces:**
- Produces documented one-command defaults and migration/recovery instructions

- [ ] **Step 1: Write exact committed manifests**

`envs/dev.env`:

```text
RPG_API_REF=dev
RPG_DND5E_WEB_REF=dev
RPG_DEPLOYMENT_REF=main
RPG_GAME_ASSETS_REF=main
RPG_API_HOST_PORT=8080
```

`envs/toolkit.env`:

```text
RPG_API_REF=dev
RPG_DND5E_WEB_REF=dev
RPG_DEPLOYMENT_REF=main
RPG_GAME_ASSETS_REF=main
RPG_TOOLKIT_REF=main
RPG_API_HOST_PORT=8081
```

- [ ] **Step 2: Update bootstrap/ignore contract tests**

Assert `/.runtime/` and `/envs/local/` are ignored by root rules while committed manifests remain trackable. Assert toolkit bootstrap documentation names five providers and general bootstrap remains seven.

- [ ] **Step 3: Rewrite runbooks around one command**

Normal quickstart:

```bash
./bootstrap.sh
./scripts/dev-env.sh up dev
# open the printed Game ready URL
./scripts/dev-env.sh status dev
./scripts/dev-env.sh down dev
```

Toolkit quickstart retains six commands and default `toolkit`. Document local feature manifest selectors, pushed refs, explicit API ports, Vite-selected ports, receipt states/logs, named snapshots, old fixed-stack manual shutdown, no adoption/kill behavior, provider prerequisites, WSL Docker integration, native compatible daemon, and exact recovery guidance.

- [ ] **Step 4: Run every deterministic gate fresh**

```bash
bash -n scripts/dev-env.sh scripts/toolkit-contributor.sh scripts/local-env/*.sh
bash tests/bootstrap-contract.sh
for test in \
  tests/local-env-manifest-contract.sh \
  tests/local-env-sources-contract.sh \
  tests/local-env-state-contract.sh \
  tests/local-env-compose-contract.sh \
  tests/local-env-web-vite-contract.sh \
  tests/local-env-supervisor-contract.sh \
  tests/local-env-snapshot-contract.sh \
  tests/dev-env-contract.sh \
  tests/toolkit-contributor-contract.sh; do
  bash "$test"
done
git diff --check
git status --short
```

Expected: all zero; only intended tracked files differ; no `.runtime`, local manifests, evidence, or subagent residue staged.

- [ ] **Step 5: Commit docs/defaults**

```bash
git add envs/dev.env envs/toolkit.env tests/bootstrap-contract.sh \
  docs/dev-environments.md docs/toolkit-contributor-sandbox.md README.md .gitignore
git commit -m "docs: document isolated local game environments"
```

### Task 13: Unit C Review Loop and PR

**Files:**
- Review complete `BASE..HEAD`; only one writer applies accepted fixes

**Interfaces:**
- Produces reviewed immutable game-dev PR head ready for live acceptance

- [ ] **Step 1: Self-review against every design section**

Build a coverage table mapping manifest/source/identity/receipt/up/rerun/status/down/snapshot/toolkit/security/migration/verification requirements to tests and code. Fix omissions before external review.

- [ ] **Step 2: Parallel fresh-context review**

Commission distinct read-only reviewers for:

1. lifecycle/process/receipt safety and failure cleanup;
2. Git source-resolution and primary-worktree immutability;
3. Compose/web/provider compatibility and test quality;
4. CLI/toolkit six-command/host-classification migration.

Every finding needs severity, file/line evidence, smallest safe fix, and focused rerun command. Parent synthesizes blockers/fixes/optional feedback; one writer applies accepted fixes.

- [ ] **Step 3: Re-run affected and full gates**

Repeat Task 12 Step 4 after every fix round. Run another focused review if fixes materially change ownership, source resolution, process termination, or rollback.

- [ ] **Step 4: Commit final fixes and open PR**

```bash
git status --short
git log --oneline origin/main..HEAD
git diff --check origin/main...HEAD
git push -u origin "feat/${game_dev_issue}-isolated-local-environments"
```

Open the issue-linked PR to `main` with provider merge SHAs, deterministic gates, residual risks, and explicit native/WSL acceptance still pending. Do not merge.

### Task 14: Unit D1 — Native Ubuntu Live Acceptance

**Files:**
- Evidence only in a clean native-Ubuntu verification root and its rpg-project issue

**Interfaces:**
- Consumes: exact immutable game-dev PR head plus merged provider refs
- Produces: signed native attestation and artifacts

- [ ] **Step 1: Prove host and clean baseline**

Record literal `/etc/os-release`, `/proc/sys/kernel/osrelease`, `docker info`, Compose/Go/Node/npm/Git versions, GitHub SSH, exact provider merge SHAs, game-dev PR head, primary checkout state/worktree lists, and configured/listening ports. Do not stop unrelated resources.

- [ ] **Step 2: One-command normal environment**

From a clean game-dev clone at the PR head:

```bash
./bootstrap.sh
./scripts/dev-env.sh up dev
./scripts/dev-env.sh status dev
```

Capture the printed normal Game URL, API URL, receipt, Vite log/PID/start ticks/token, Compose project/services/labels, and browser evidence that normal `GameView` loads with synced licensed assets.

- [ ] **Step 3: Start a second simultaneous environment**

Create ignored `envs/local/parallel.env` with pushed dev refs/assets/deployment and API port 8082. Run `up local/parallel`, verify a different Compose project/image/API port and Vite-selected URL, then rerun it and prove unchanged Vite PID/start ticks/token/URL.

- [ ] **Step 4: Isolation shutdown proof**

Run `down dev`; immediately verify the parallel API health, Vite HTTP URL, process identity, and browser view remain healthy. Then restart dev for toolkit coexistence if needed.

- [ ] **Step 5: Path-backed web/toolkit proofs**

Use disposable exact-origin developer worktrees named in ignored manifests. Confirm a web edit appears through HMR without Git mutation. Confirm a reversible toolkit Human Strength marker is invisible before `refresh`, visible after refresh/reseed, and restored after source restoration plus refresh/reseed through the environment's configured port.

- [ ] **Step 6: Negative matrix with owned fixtures**

Exercise occupied API port, malformed/hostile manifest, wrong origin, ref ambiguity, forged/stale receipt, asset-sync failure, Compose failure, and Vite readiness failure. Snapshot primary/unrelated fixture states before/after and prove no unowned stop/kill/down or source mutation.

- [ ] **Step 7: Final cleanup and signed evidence**

Named-down every owned environment. Prove no owned Vite group, Compose project, port, active receipt, or toolkit override remains. Publish commands/results, URLs/screenshots, exact SHAs, diffs, and residual risks to the native verification issue with signature.

### Task 15: Unit D2 — Ubuntu WSL2 Live Acceptance

**Files:**
- Evidence only in the user's Ubuntu WSL2 filesystem and its rpg-project issue

**Interfaces:**
- Produces: independent signed WSL2 attestation for the same immutable head

- [ ] **Step 1: Publish a self-contained WSL2 pickup**

The issue comment includes prerequisites, exact refs/SHAs, clean clone path inside WSL (not `/mnt/c`), Docker Desktop integration/compatible-daemon check, commands from Task 14, expected outputs, negative matrix, evidence names, cleanup, and failure recovery. It requires no chat history or native filesystem.

- [ ] **Step 2: Execute the same positive matrix**

Prove literal `ubuntu-wsl2` classification, one-command dev startup, two simultaneous environments, Vite reuse, isolated down, path-backed web HMR, toolkit refresh/seed, and final cleanup.

- [ ] **Step 3: Execute the same safety matrix**

Use only WSL-owned fixtures/listeners/worktrees. Include a Docker Desktop published-port fixture that is reachable on localhost while absent from the distro's filtered fake/observed `ss` output; prove the combined Docker/TCP preflight refuses before source mutation. Confirm Windows/other WSL/native resources are untouched and no checkout is shared through `/mnt/c`.

- [ ] **Step 4: Publish independent evidence**

Attach exact command transcript, receipts, Vite logs/URLs, Compose evidence, screenshots, marker proof, cleanup proof, provider/game-dev SHAs, and signed independent review. Any game-dev PR-head change invalidates both Task 14 and Task 15 attestations.

### Task 16: Final Delivery Gate

**Files:**
- `ideas/isolated-local-environments/design.md`
- `ideas/isolated-local-environments/plan.md`
- GitHub issue/PR/project records

**Interfaces:**
- Produces: merge-ready game-dev and tracking PRs; no automatic merge authority

- [ ] **Step 1: Cross-check all issue/PR closure relationships**

Use GraphQL `closedByPullRequestsReferences(first:20)`, exact `projectItems(first:20)`, remote head equality, provider ancestry, and immutable package/digest evidence. Do not use unsupported `gh issue view --json closedByPullRequestsReferences`.

- [ ] **Step 2: Restore failure statuses before review transitions**

Only after all deterministic and both live gates pass, move build/verification items to In Review. Any later failure restores In Progress while preserving the original failure evidence.

- [ ] **Step 3: Ask Kirk to merge the game-dev PR**

Kirk alone merges. After merge, verify merge SHA ancestry, implementation issue closure, no staged files, and provider/live evidence attachment integrity.

- [ ] **Step 4: Complete the tracking surface**

Add final results and exact merges to rpg-project PR `#222`, update the living handoff, and close/Done the new verification and implementation sub-issues only after assertions pass. Add signed supersession comments to historical fixed-port verification issues `#210` and `#213`, preserving their evidence and blockers verbatim, then close/Done them as superseded by the new native/WSL attestations. Ask Kirk to merge the rpg-project idea PR. Close umbrella `#208` only when no accepted onboarding/runtime verification remains open.
