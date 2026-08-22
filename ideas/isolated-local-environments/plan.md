# Simple Named Local Game Stacks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace checkout-switching `scripts/dev-env.sh` with a small named-manifest wrapper that starts selected API/web refs or paths, exact API/web ports, and an optional toolkit override.

**Architecture:** Start from fresh `game-dev/origin/main`, not game-dev PR #70. Keep one Bash entry point and one shell contract: ref selectors create detached runtime worktrees, path selectors are used directly, Compose receives a name-specific project/image/API port, and host Vite receives the exact web port and API URL. Runtime state is a trusted local `state.env` containing only resolved paths, URLs, the Compose project, and Vite process-group/log details.

**Tech Stack:** Bash, Git worktrees, Docker/Compose, npm/Vite, the existing rpg-api toolkit override helper.

**Spec:** `ideas/isolated-local-environments/design.md`

## Global Constraints

- Begin from current `game-dev/origin/main`; preserve PR #70 at `e89c69e26cde98e14a8724e889d9f0d3296383f8` unchanged.
- Never switch, reset, clean, or remove a primary checkout or a manifest-selected `*_PATH` checkout.
- `RPG_API_HOST_PORT` and `RPG_WEB_HOST_PORT` are required and exact; occupied ports fail before build/start.
- Ref-backed sources use detached worktrees under `.runtime/<environment>/sources/`.
- A toolkit selector requires a ref-backed managed API source and uses `scripts/toolkit-local-override.sh`; no new toolkit facade is added.
- Do not add receipts, JSON schemas, recovery journals, snapshots, pinning, provider identity checks, or automatic resource adoption.
- RED-GREEN TDD is required for production behavior.

---

## File map

- `scripts/dev-env.sh` — the complete trusted-local manifest parser, source resolver, and `up|status|down` lifecycle.
- `tests/dev-env-contract.sh` — one end-to-end shell contract using real temporary Git repositories and fake Docker/npm/port commands.
- `envs/dev.env` — committed example selecting API/web `dev` and exact ports 8080/3001.
- `.gitignore` — ignores `/.runtime/` and `/envs/local/`.
- `docs/dev-environments.md` — short user walkthrough, manifest reference, toolkit example, and reset instructions.

---

### Task 1: Minimal named launcher

**Files:**
- Replace: `scripts/dev-env.sh`
- Create: `tests/dev-env-contract.sh`
- Modify: `envs/dev.env`
- Modify: `.gitignore`

**Interfaces:**
- Consumes manifests at `envs/<name>.env` and `envs/local/<name>.env`.
- Produces CLI `scripts/dev-env.sh {up|status|down} <name>`.
- Produces trusted runtime state at `.runtime/<name>/state.env` with `COMPOSE_PROJECT`, `API_SOURCE`, `WEB_SOURCE`, `API_URL`, `WEB_URL`, `VITE_PGID`, and `VITE_LOG`.

- [ ] **Step 1: Write the failing end-to-end shell contract**

Create `tests/dev-env-contract.sh`. Its fixture must initialize local bare origins and clones for `rpg-api`, `rpg-dnd5e-web`, and `rpg-toolkit`; put executable `scripts/toolkit-local-override.sh` and both Dockerfiles in the API commit; put `package.json`, `package-lock.json`, and `node_modules/` in the web commit; and create placeholder deployment Compose files and asset roots.

Put fake `docker`, `npm`, and `ss` executables first on `PATH`. Each fake appends shell-escaped arguments to `$COMMAND_LOG`. The Docker fake prefixes its line with `RPG_API_HOST_PORT=${RPG_API_HOST_PORT:-} RPG_API_IMAGE=${RPG_API_IMAGE:-}`; the npm fake prefixes its line with `VITE_API_HOST=${VITE_API_HOST:-}`. Fake `docker compose ps` also prints `fixture compose running`, fake `ss` prints `${OCCUPIED_PORT_OUTPUT:-}`, and fake `npm run dev` sleeps until killed. The fixture override helper writes `toolkit-local-override.sh` plus its shell-escaped arguments to `$COMMAND_LOG` and exits 0. The assertions must be literal:

```bash
cat >"$ROOT/envs/local/combat.env" <<'EOF'
RPG_API_REF=feature-api
RPG_DND5E_WEB_REF=feature-web
RPG_API_HOST_PORT=18082
RPG_WEB_HOST_PORT=13002
RPG_TOOLKIT_PATH=TOOLKIT_PATH
RPG_TOOLKIT_TARGET=rulebooks/dnd5e
EOF
sed -i "s#TOOLKIT_PATH#$ROOT/rpg-toolkit#" "$ROOT/envs/local/combat.env"

api_head_before=$(git -C "$ROOT/rpg-api" rev-parse HEAD)
web_head_before=$(git -C "$ROOT/rpg-dnd5e-web" rev-parse HEAD)
"$ROOT/scripts/dev-env.sh" up local/combat

grep -F 'toolkit-local-override.sh on --target rulebooks/dnd5e --src' "$COMMAND_LOG"
grep -F 'docker build -f' "$COMMAND_LOG"
grep -F 'Dockerfile.local-toolkit' "$COMMAND_LOG"
grep -F 'docker compose -p rpg-local--combat' "$COMMAND_LOG"
grep -F 'RPG_API_HOST_PORT=18082' "$COMMAND_LOG"
grep -F 'RPG_API_IMAGE=rpg-api:local--combat' "$COMMAND_LOG"
grep -F -- '--port 13002 --strictPort' "$COMMAND_LOG"
grep -F 'VITE_API_HOST=http://localhost:18082' "$COMMAND_LOG"
test "$(git -C "$ROOT/rpg-api" rev-parse HEAD)" = "$api_head_before"
test "$(git -C "$ROOT/rpg-dnd5e-web" rev-parse HEAD)" = "$web_head_before"
test -f "$ROOT/.runtime/local/combat/state.env"

"$ROOT/scripts/dev-env.sh" status local/combat | grep -F 'fixture compose running'
"$ROOT/scripts/dev-env.sh" down local/combat
grep -F 'docker compose -p rpg-local--combat' "$COMMAND_LOG"
grep -F 'down --remove-orphans' "$COMMAND_LOG"

collision_log="$TMP/commands-after-collision.log"
: >"$collision_log"
if OCCUPIED_PORT_OUTPUT='LISTEN fixture' COMMAND_LOG="$collision_log" \
  "$ROOT/scripts/dev-env.sh" up local/combat; then
  echo 'expected occupied-port failure' >&2
  exit 1
fi
! grep -F 'docker build' "$collision_log"
```

Add a second path-backed manifest and assert its API/web paths appear in state while their HEADs and tracked status remain byte-identical. Assert a toolkit selector plus `RPG_API_PATH` fails with a direct message. Assert malformed names, missing selectors, equal API/web ports, and non-numeric/out-of-range ports fail.

- [ ] **Step 2: Run the contract to verify RED**

Run:

```bash
bash tests/dev-env-contract.sh
```

Expected: FAIL because current `dev-env.sh` expects `*_BRANCH`, mutates primary checkouts, has no exact web-port/Vite lifecycle, and `down` is unnamed.

- [ ] **Step 3: Implement the minimal launcher**

Replace `scripts/dev-env.sh` with `set -euo pipefail` and these focused functions:

```bash
usage() { printf 'Usage: %s {up|status|down} <environment>\n' "$0" >&2; }
fail() { printf 'dev-env: %s\n' "$*" >&2; exit 1; }

environment_paths() {
  case "$ENV_NAME" in
    local/[a-z0-9]*([a-z0-9-])) MANIFEST="$ROOT/envs/$ENV_NAME.env" ;;
    [a-z0-9]*([a-z0-9-]))       MANIFEST="$ROOT/envs/$ENV_NAME.env" ;;
    *) fail "invalid environment name: $ENV_NAME" ;;
  esac
  SLUG=${ENV_NAME//\//--}
  RUNTIME="$ROOT/.runtime/$ENV_NAME"
  STATE="$RUNTIME/state.env"
  PROJECT="rpg-$SLUG"
}

resolve_source() {
  local repo=$1 ref=$2 selected_path=$3 destination=$4
  if [[ -n "$selected_path" ]]; then
    git -C "$selected_path" rev-parse --show-toplevel
    return
  fi
  [[ -n "$ref" ]] || fail "$repo requires a REF or PATH selector"
  mkdir -p "$(dirname "$destination")"
  if [[ -e "$destination" ]]; then
    git -C "$ROOT/$repo" worktree remove --force "$destination"
  fi
  git -C "$ROOT/$repo" fetch origin "$ref"
  git -C "$ROOT/$repo" worktree add --detach "$destination" FETCH_HEAD
  printf '%s\n' "$destination"
}

port_in_use() {
  ss -H -ltn "sport = :$1" 2>/dev/null | grep -q .
}
```

Enable `extglob` for the name patterns, source the trusted manifest, reject simultaneous `*_REF`/`*_PATH`, validate each port as decimal `1..65535`, and reject equal API/web ports.

Implement `down_internal` to source `state.env` when present, kill only a numeric recorded process group with `kill -- "-$VITE_PGID"` (ignore already-dead), and run:

```bash
RPG_API_HOST_PORT="$RPG_API_HOST_PORT" RPG_API_IMAGE="$API_IMAGE" \
  docker compose -p "$PROJECT" \
    -f "$DEPLOYMENT/docker-compose.local-dev.yml" \
    -f "$DEPLOYMENT/docker-compose.local-api-src.yml" \
    down --remove-orphans
```

Implement `up` in this order: load/validate manifest; call `down_internal`; fail if either port remains occupied; recreate ref-backed worktrees; apply optional toolkit override; build `rpg-api:$SLUG` with normal `Dockerfile` or `Dockerfile.local-toolkit`; start named Compose with `RPG_API_HOST_PORT` and `RPG_API_IMAGE`; run asset sync with explicit `$ROOT/rpg-game-assets`; run `npm ci` only when `node_modules` is absent; start Vite in a new process group with exact arguments:

```bash
setsid env VITE_API_HOST="http://localhost:$RPG_API_HOST_PORT" \
  npm --prefix "$WEB_SOURCE" run dev -- --port "$RPG_WEB_HOST_PORT" --strictPort \
  >"$VITE_LOG" 2>&1 &
VITE_PGID=$!
sleep 1
kill -0 "$VITE_PGID" || fail "Vite exited; see $VITE_LOG"
```

Write `state.env` using `printf '%q'` for every value, print `Game ready: http://localhost:<web-port>/?playerId=<slug>` and `API: http://localhost:<api-port>`, and make `status` source state then run named Compose `ps` plus `kill -0` for Vite.

Update `envs/dev.env` to:

```dotenv
RPG_API_REF=dev
RPG_DND5E_WEB_REF=dev
RPG_API_HOST_PORT=8080
RPG_WEB_HOST_PORT=3001
```

Add to `.gitignore`:

```gitignore
/.runtime/
/envs/local/
```

- [ ] **Step 4: Run the contract to verify GREEN**

Run:

```bash
bash -n scripts/dev-env.sh tests/dev-env-contract.sh
bash tests/dev-env-contract.sh
```

Expected: syntax checks exit 0 and the contract prints `dev-env contract: PASS`.

- [ ] **Step 5: Commit the launcher**

```bash
git add .gitignore envs/dev.env scripts/dev-env.sh tests/dev-env-contract.sh
git commit -m "feat: add simple named local game stacks"
```

---

### Task 2: User walkthrough and real provider check

**Files:**
- Replace: `docs/dev-environments.md`

**Interfaces:**
- Documents only the Task 1 CLI and manifest keys.
- Does not document PR #70 lifecycle concepts or removed commands.

- [ ] **Step 1: Rewrite the walkthrough**

Keep the document under 180 lines. Lead with this complete example and command sequence:

```dotenv
# envs/local/combat.env
RPG_API_REF=feat/combat-api
RPG_DND5E_WEB_REF=feat/combat-web
RPG_API_HOST_PORT=8082
RPG_WEB_HOST_PORT=3002
```

```bash
./scripts/dev-env.sh up local/combat
./scripts/dev-env.sh status local/combat
./scripts/dev-env.sh down local/combat
```

Document `*_PATH`, pushed-ref detached worktrees, exact ports, printed URLs, optional `RPG_TOOLKIT_REF|PATH` plus `RPG_TOOLKIT_TARGET`, automatic rebuild-on-rerun, ordinary Vite HMR, log/state locations, and the supported reset (`down`, remove that environment's runtime directory, `up`). Explicitly say the deployment and game-assets roots are the existing workspace checkouts and are not selected by this manifest.

- [ ] **Step 2: Validate the real merged Compose provider**

Use a detached deployment worktree at merged provider commit `bbbf04f6f7d425ea06aa215c7ca11db1c85e390b`, then run:

```bash
RPG_API_HOST_PORT=18082 RPG_API_IMAGE=rpg-api:local--contract \
  docker compose -p rpg-local--contract \
    -f <detached-deployment>/docker-compose.local-dev.yml \
    -f <detached-deployment>/docker-compose.local-api-src.yml \
    config > /tmp/simple-local-stack-compose.yml

grep -F 'published: "18082"' /tmp/simple-local-stack-compose.yml
grep -F 'rpg-api:local--contract' /tmp/simple-local-stack-compose.yml
```

Expected: Compose config exits 0 and both selected values appear. Remove only the detached verification worktree and temporary config file.

- [ ] **Step 3: Run the complete gate**

Run:

```bash
bash -n scripts/dev-env.sh tests/dev-env-contract.sh
bash tests/dev-env-contract.sh
git diff --check origin/main...HEAD
```

Expected: all commands exit 0.

- [ ] **Step 4: Commit documentation**

```bash
git add docs/dev-environments.md
git commit -m "docs: explain named local game stacks"
```

---

### Task 3: Focused review and publication

**Files:**
- Review only; no planned production files.

**Interfaces:**
- Produces one game-dev PR linked to game-dev#63 from a fresh branch.
- Preserves old game-dev PR #70 and rpg-project PR #222 until Kirk decides how to close/supersede them.

- [ ] **Step 1: Review the complete branch against the minimal spec**

Inspect:

```bash
git diff --stat origin/main...HEAD
git diff origin/main...HEAD
git log --oneline origin/main..HEAD
```

Reject additions involving JSON receipts, journals, recovery phases, snapshots, provider fingerprints, or toolkit lifecycle commands.

- [ ] **Step 2: Re-run verification immediately before push**

```bash
bash -n scripts/dev-env.sh tests/dev-env-contract.sh
bash tests/dev-env-contract.sh
git diff --check origin/main...HEAD
git status --short
```

Expected: all gates exit 0 and status is clean.

- [ ] **Step 3: Push and open the replacement PR without merging**

Push the fresh branch and open one game-dev PR referencing game-dev#63 and rpg-project#221. In the PR body, lead with the manifest and the three commands before rationale. State that PR #70 remains unmerged historical reference and that Kirk alone merges.
