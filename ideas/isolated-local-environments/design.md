# Simple named local game stacks

**Issue:** rpg-project#221 / game-dev#63  
**Status:** Proposed replacement for the over-engineered game-dev#70 runtime

## Goal

A developer names the API branch, web branch, API port, web port, and optional local toolkit source in one small manifest. One command starts that exact local game; one command stops it.

This wraps the manual local workflow Kirk already uses. It is not a deployment system and does not attempt production-grade crash recovery.

## User contract

Machine-local manifests live under ignored `envs/local/`:

```dotenv
# envs/local/combat.env
RPG_API_REF=feat/combat-api
RPG_DND5E_WEB_REF=feat/combat-web
RPG_API_HOST_PORT=8082
RPG_WEB_HOST_PORT=3002

# Optional: iterate on one local toolkit module through the API build.
RPG_TOOLKIT_PATH=/home/kirk/game-dev/rpg-toolkit/.worktrees/combat
RPG_TOOLKIT_TARGET=rulebooks/dnd5e
```

Shared manifests may live at `envs/<name>.env`. The commands are:

```bash
./scripts/dev-env.sh up local/combat
./scripts/dev-env.sh status local/combat
./scripts/dev-env.sh down local/combat
```

A manifest may select `RPG_API_PATH` or `RPG_DND5E_WEB_PATH` instead of the corresponding pushed ref when the developer wants uncommitted local source. An optional toolkit source may likewise use `RPG_TOOLKIT_PATH` or `RPG_TOOLKIT_REF`.

## What `up` does

1. Read the named manifest as trusted local shell-style `KEY=value` configuration.
2. If this name has prior local state, stop that name's recorded Vite group and Compose project.
3. Refuse if either requested host port is still occupied.
4. For each `*_REF`, fetch that pushed ref from the existing workspace clone and create a detached worktree under `.runtime/<environment>/sources/`. For each `*_PATH`, use that checkout directly without changing its Git state.
5. When a toolkit source is selected, require a ref-backed managed API worktree and call the API's existing `scripts/toolkit-local-override.sh` for `rulebooks/dnd5e` (default) or `encounter`.
6. Build one environment-tagged API image from the selected API source.
7. Start the existing deployment Compose files with a project name derived from the environment, `RPG_API_HOST_PORT`, and the environment-tagged API image.
8. Sync game assets into the selected web source, run `npm ci` only when `node_modules` is absent, and start host Vite on exactly `RPG_WEB_HOST_PORT` with `--strictPort` and `VITE_API_HOST` pointed at the selected API port.
9. Record only the Vite process-group ID and log path needed by `status` and `down`, then print both URLs.

Running `up` again for the same name first stops that name's prior Vite/Compose processes and recreates its managed source worktrees. This is restart-and-rebuild, not state reconciliation.

## What `status` and `down` do

`status` prints the manifest, selected source commits/paths, Compose `ps`, configured URLs, and whether the recorded Vite process is alive. It has no drift taxonomy.

`down` stops the recorded Vite process group and runs Compose down for the exact environment-derived project name. It leaves source worktrees and logs available for inspection; the next `up` replaces them.

If local runtime state is confusing, the supported recovery is:

```bash
./scripts/dev-env.sh down local/combat
rm -rf .runtime/local/combat
./scripts/dev-env.sh up local/combat
```

## Safety retained

- Ref-backed sources never switch primary checkouts.
- Path-backed sources are never checked out, reset, cleaned, or removed.
- Requested API and web ports must be free before startup.
- Each environment uses a distinct Compose project, image tag, worktree directory, Vite process, and log.
- `down <name>` targets only that name's Compose project and recorded Vite group.

These are ordinary local-development guardrails, not hostile-state guarantees.

## Explicit non-goals

The launcher will not implement:

- write-ahead journals or transactional recovery;
- cryptographic receipts or immutable process/container fingerprints;
- automatic adoption or forensic classification of stale resources;
- schema-closed JSON lifecycle state;
- snapshot management;
- branch pinning or release automation;
- platform installation or Docker Desktop configuration;
- a separate toolkit lifecycle facade.

Toolkit refresh is simply another `up` for the same environment. Web edits continue through normal Vite HMR.

## Verification

A small shell contract will prove:

- manifest refs/paths and both ports reach the expected Git, Docker, Compose, and Vite commands;
- optional toolkit configuration invokes the existing API override helper before the API build;
- occupied ports fail before build/start;
- `down` targets only the named Compose project and recorded Vite group;
- primary checkout HEAD and tracked files are unchanged.

The final manual check is the workflow itself: create one local manifest, run `up`, open the printed URL, edit web code and observe HMR, optionally edit toolkit code and rerun `up`, then run `down`.
