# Isolated one-command local game environments

**Status:** Written design approved by Kirk on 2026-08-15
**Tracking:** [rpg-project#221](https://github.com/KirkDiggler/rpg-project/issues/221), follow-on under [rpg-project#208](https://github.com/KirkDiggler/rpg-project/issues/208)  
**Owners:** `rpg-deployment`, `rpg-dnd5e-web`, and `game-dev`

## Summary

A named local environment becomes the complete declaration of which pushed
sources or local worktrees make up one running game. One command resolves those
sources, starts an isolated backend, synchronizes the selected assets, starts
Vite on the host, and prints the URL Vite actually chose:

```bash
./scripts/dev-env.sh up dev
```

Only Envoy receives an explicit host port. Vite keeps its normal
next-available-port behavior. Redis, Mongo, the D&D API, nginx, Envoy admin, and
gRPC remain internal to the environment's Compose project.

The normal development launcher and the toolkit-contributor facade share one
runtime supervisor and ownership model. A committed manifest is desired state;
an ignored runtime receipt is observed state. Shutdown is always named and
stops only resources whose ownership can be proved.

## Context

The current normal loop starts the backend with `dev-env.sh`, then asks the
operator to synchronize assets and run Vite separately. The toolkit facade also
asks the operator to start Vite separately. Both backend paths inherit fixed
Compose `container_name` values and fixed publications for ports 80, 3002,
6380, and 8080. Consequently:

- a fresh contributor does not get a running game from one command;
- Vite's actual fallback port is not retained by the launcher;
- two branch games collide even when their source trees are independent;
- fixed names make Compose's project isolation ineffective;
- shutdown cannot identify one named branch game as its sole target; and
- a manifest says which branches are desired but not what is actually running.

The original native-Ubuntu and WSL2 toolkit design treated fixed ports as an
acceptance observation. That remains part of the historical verification record
for the already-merged facade, but it is superseded for permanent local use by
this design.

## Goals

1. Make `./scripts/dev-env.sh up dev` start a complete playable local game,
   including assets and background Vite.
2. Let multiple named branch games coexist when their manifests assign distinct
   Envoy host ports.
3. Let a manifest run pushed refs reproducibly or opt into explicit existing
   worktree paths for live local edits.
4. Never switch, clean, reset, stash, or remove a primary checkout or a
   developer-owned worktree.
5. Bind every running Compose project and Vite process to one named environment
   and one workspace.
6. Make `status` distinguish configured, running, stale, drifted, and unhealthy
   state.
7. Make named shutdown affect only its environment.
8. Preserve the toolkit facade's six commands and its literal native-Ubuntu /
   Ubuntu-WSL2 host classification.
9. Preserve host Vite and HMR rather than containerizing the web development
   loop.
10. Support Linux-native and Ubuntu WSL2 with a reachable Docker-compatible
    daemon.

## Non-goals

- Automatically choosing, reserving, or retrying an Envoy port.
- Killing a listener or container that belongs to another environment.
- Turning this into a generic process manager or deployment framework.
- Creating, switching, or deleting developer feature worktrees.
- Making unpushed API commits available through a toolkit override.
- Publishing Redis, Mongo, the D&D API, nginx, Envoy admin, or direct gRPC to the
  host.
- Supporting WSL1, native Windows shells, or non-Ubuntu hosts in the toolkit
  facade.
- Changing gameplay, RPC, authentication, lobby, or Dungeon Builder contracts.

## Chosen architecture

### Components

The system has five narrow components:

1. **Manifest parser** — reads committed or ignored `envs/**/*.env` files as
   data, validates their closed key set, and produces desired source and port
   configuration.
2. **Source resolver** — validates developer-owned paths or resolves pushed refs
   into launcher-owned detached runtime worktrees.
3. **Runtime supervisor** — derives identity, builds the API, invokes Compose,
   waits for Envoy, prepares web dependencies/assets, starts or reuses Vite, and
   writes receipts.
4. **Provider contracts** — deployment accepts project-scoped identity, image,
   and Envoy port; web asset sync accepts an explicit non-mutating asset path.
5. **Facades** — `dev-env.sh` and `toolkit-contributor.sh` retain their
   domain-specific behavior while delegating lifecycle ownership to the shared
   supervisor.

The shared code belongs in `game-dev/scripts/`; provider repositories expose
only the small inputs they own. The supervisor is not copied into provider
repositories.

### Why this approach

This keeps Vite on the host, where file watching and HMR work best on native
Ubuntu and WSL2, while restoring Compose's intended project isolation. It also
keeps desired state human-readable and observed state machine-readable.

Two alternatives were rejected:

- **Vite in Compose:** simpler process ownership, but worse HMR/file watching and
  a regression from the existing host-Vite workflow.
- **Generated ad hoc override files for unchanged deployment Compose:** avoids a
  provider change initially, but duplicates deployment structure in shell code
  and lets generated configuration drift from its owner.

## Named environments and manifests

### Location and name

Committed shared manifests live at `envs/<name>.env`. Machine-local feature
manifests live under ignored `envs/local/<name>.env` and are selected as
`local/<name>`.

A name consists of lowercase ASCII letters, digits, `_`, `.`, or `-` within one
or two path components. It must not contain `..`, an empty component, a leading
punctuation character, or a path separator outside the optional `local/`
prefix. This makes the name safe as a lookup key; it is never evaluated as a
shell fragment.

### Data syntax

A manifest is parsed, never sourced. Its syntax is deliberately smaller than a
shell `.env` file:

- blank lines and lines whose first non-whitespace character is `#` are ignored;
- every other line is exactly `KEY=VALUE`;
- keys are uppercase ASCII identifiers from the closed set below;
- the value is the literal remainder after the first `=`; shell quoting,
  expansion, substitution, and escapes have no meaning;
- leading/trailing whitespace around a key or value is invalid;
- duplicate keys and unknown keys fail the whole manifest; and
- required values must be non-empty and contain no control characters.

The closed key set is:

```text
RPG_API_REF                 RPG_API_PATH
RPG_DND5E_WEB_REF           RPG_DND5E_WEB_PATH
RPG_DEPLOYMENT_REF          RPG_DEPLOYMENT_PATH
RPG_GAME_ASSETS_REF         RPG_GAME_ASSETS_PATH
RPG_TOOLKIT_REF             RPG_TOOLKIT_PATH
RPG_API_HOST_PORT
SNAPSHOT
```

API, web, deployment, and game-assets selectors are required. Toolkit is
required only for toolkit mode. `SNAPSHOT` is optional. For each repository,
`*_PATH` takes precedence when both forms are present; an invalid path never
falls back to the ref.

`RPG_API_HOST_PORT` is a decimal integer from 1024 through 65535. It is the one
host port the backend owns. The launcher does not infer or persist an
alternative.

Old `*_BRANCH` keys fail with a migration message. They are not silently
translated because their old behavior switched primary checkouts.

### Default manifests

The committed normal environment is:

```bash
RPG_API_REF=dev
RPG_DND5E_WEB_REF=dev
RPG_DEPLOYMENT_REF=main
RPG_GAME_ASSETS_REF=main
RPG_API_HOST_PORT=8080
```

The committed toolkit environment is:

```bash
RPG_API_REF=dev
RPG_DND5E_WEB_REF=dev
RPG_DEPLOYMENT_REF=main
RPG_GAME_ASSETS_REF=main
RPG_TOOLKIT_REF=main
RPG_API_HOST_PORT=8081
```

A local toolkit edit replaces the final selector without changing the pushed
providers:

```bash
RPG_TOOLKIT_PATH=/home/kirk/game-dev/rpg-toolkit/.worktrees/my-feature
```

## Source resolution

### Developer-owned paths

A `*_PATH` may be absolute or relative to the `game-dev` workspace root. The
resolver canonicalizes it and requires:

- an existing Git worktree root, not a subdirectory;
- the expected canonical `KirkDiggler` repository origin;
- a readable `HEAD`; and
- the repository-specific files needed by the selected mode.

The launcher uses the path exactly as it stands, including tracked and
untracked local edits. It performs no fetch, checkout, switch, merge, reset,
clean, stash, commit, or worktree removal there. The expected origin is the
exact SSH origin used by bootstrap, not merely a repository with a matching
basename.

Expected generated web content is the only non-Git mutation permitted in a
path-backed web source: `node_modules` and the gitignored synced
`public/models/synty` tree. The launcher never changes its tracked
`package-lock.json`.

### Pushed refs

A `*_REF` names a pushed branch, pushed tag, or reachable full commit on the
canonical origin. Short names resolve against remote heads and tags; ambiguity
between a head and tag fails and asks for an explicit full ref. The resolver
fetches only the selected remote object into a launcher namespace, records the
immutable SHA, and checks it out detached at:

```text
.runtime/worktrees/<environment>/<repository>/
```

These are launcher-owned Git worktrees, not developer feature worktrees. Each
ref-backed repository must already have its canonical bootstrap clone in the
workspace; a missing clone fails with a bootstrap instruction rather than
causing an implicit runtime clone. The primary checkout's branch, `HEAD`, index,
and worktree bytes never change. Fetching the selected object may update only
launcher-owned refs and the shared object database.

On rerun, a moving branch may resolve to a new SHA. Before advancing the managed
worktree, the supervisor requires no unexplained tracked changes. Toolkit mode
first removes its owned API replacement. Any other tracked difference fails
closed. Ignored dependency and asset outputs may remain.

### Resolved source record

For every source, the receipt records:

- repository identity;
- requested ref, if any;
- resolved immutable SHA;
- canonical actual path;
- `managed` or `developer-owned`; and
- tracked cleanliness at resolution time.

All later actions consume these paths. No runtime command assumes that provider
repositories are siblings of `game-dev`.

## Runtime identity and isolation

The supervisor derives, rather than accepts, runtime identity from:

- the canonical physical `game-dev` root;
- the normalized environment name; and
- the receipt schema version.

A stable short hash of the workspace path plus the sanitized environment name
forms the Compose project and API image tag. This prevents two different
`game-dev` clones that both call an environment `dev` from sharing container,
network, or image identity.

Examples are illustrative, not caller-configurable:

```text
Compose project: rpg-dev-4f29a12c
API image:      rpg-api:local-dev-4f29a12c
```

Deployment removes local `container_name` declarations so Compose names all
resources under that project. The local source overlay accepts the derived API
image through `RPG_API_IMAGE`. The Envoy publication accepts
`RPG_API_HOST_PORT` and maps only:

```text
<manifest port>:8080
```

Redis 6379, Mongo, D&D API 3000, rpg-api 50051, Envoy admin 9901, nginx 80, and
all service-to-service traffic remain on the project network. nginx may remain
part of the local stack for internal compatibility, but receives no host
publication.

The normal facade preserves its current two-file Compose semantics. Toolkit
mode preserves its current API overlay in addition to the local base and local
API source overlay. Both use the same project, image, port, labeling, and
receipt contract.

## Runtime receipts and ownership

Desired state stays in the manifest. Observed state is written atomically below:

```text
.runtime/environments/<environment>/
  attempt.json
  receipt.json
  vite.log
```

`.runtime/` is ignored. `attempt.json` is an in-progress ownership journal used
for bounded rollback. `receipt.json` is written only after backend and web
readiness pass.

The versioned receipt contains at least:

- canonical workspace and manifest paths plus manifest digest;
- environment name and facade mode;
- derived Compose project, API image, and Envoy port/URL;
- resolved source records and repository SHAs;
- Compose input paths and started service/container IDs;
- Vite PID, process-group ID, Linux `/proc` start time, random ownership token,
  working directory, API URL, actual local URL, and log path;
- package-lock and Vite-config digests relevant to reuse; and
- created/updated timestamps.

Receipt parsing uses a closed JSON schema. Values that can be derived from the
workspace and environment must exactly match a fresh derivation. Compose
resources must carry matching Compose project labels. Vite receives the random
ownership token in a dedicated process environment variable. It is owned only
when PID, process group, `/proc` start time and environment, working directory,
command shape, and that token all agree. This prevents ordinary PID reuse or a
stale/edited receipt from targeting an unrelated server.

A receipt is evidence to verify, not authority to stop an arbitrary name or
PID.

## One-command startup

`dev-env.sh up <environment>` performs these stages in order:

1. Parse the manifest and validate all non-network inputs.
2. Derive runtime identity and inspect an existing receipt/journal.
3. Check the explicit Envoy port. A listener is accepted only when it is the
   already-owned Envoy for this exact environment; every other owner fails
   before source fetch, build, branch/worktree advancement, or process stop.
4. Resolve all path-backed and ref-backed sources.
5. Build the selected API source into the environment-specific image. A build
   failure leaves an already-running old container image untouched.
6. Start or reconcile the derived Compose project with the resolved deployment
   files, image, and Envoy port.
7. Wait for backend service health and the existing real Envoy health probe at
   the configured host port. No game URL is printed before this succeeds.
8. Prepare the selected web source.
9. Synchronize the selected assets.
10. Reuse a matching healthy owned Vite process or start one in a new process
    group with `VITE_API_HOST=http://localhost:<Envoy port>`.
11. Parse Vite's ANSI-normalized `Local:` URL from its log, verify the HTTP
    server responds, atomically promote the attempt journal to a receipt, and
    print the exact game and API URLs.

Normal mode prints a URL with a URL-encoded environment player identity:

```text
Game ready: http://localhost:<vite-port>/?playerId=<environment>
API: http://localhost:<envoy-port>
```

Toolkit mode prints the selected Vite origin with
`?toolkitSandbox=1` after toolkit-specific health succeeds.

### Vite behavior

Vite retains `port: 3001` and its default non-strict fallback. The supervisor
does not scan, reserve, or pass a web port. If 3001 is occupied, Vite chooses
its next available port and the supervisor records what Vite reports.

Vite runs from the resolved web path so path-backed web edits receive normal
HMR. It runs in a dedicated process group with output retained in `vite.log`.

An unchanged rerun reuses Vite when process ownership, web path, API URL,
package-lock digest, and Vite-config digest still match. Source-only changes in
the same path remain Vite/HMR's responsibility. A changed API URL, web path,
dependency lock, or Vite configuration causes an owned restart. No unowned
Vite process is reused or stopped.

### Web dependencies and assets

If `node_modules` is absent, no trusted installed-lock digest exists, or the
recorded package-lock digest differs, startup runs `npm ci` in the resolved web
source. `--clean-web` removes only `node_modules` and forces `npm ci`; it never
deletes or rewrites `package-lock.json`.

`rpg-dnd5e-web` extends `assets:sync` with
`RPG_GAME_ASSETS_PATH`. When supplied, the script validates the expected
`harness/models/synty` source and copies it with the existing exact
`rsync --delete` semantics. It performs no clone, fetch, pull, switch, or other
Git operation. The supervisor always supplies its resolved game-assets path, so
startup cannot accidentally update a sibling checkout.

A ref-backed game-assets source is already updated by source resolution. A
path-backed source is copied exactly as it stands, including local asset work.

## Reruns and failure recovery

`up` is reconciliation, not an instruction to create another copy of the same
environment.

On a healthy rerun it:

- resolves moving refs again;
- rebuilds/reconciles the backend;
- runs dependency preparation when needed;
- synchronizes assets every time; and
- reuses Vite when the reuse contract still matches.

For a first start, the attempt journal records each resource created by the
attempt. A failure stops only Vite and Compose resources whose derived identity
and ownership checks match that journal. Logs and the failed journal remain for
diagnosis; no readiness URL is printed.

For a rerun, build occurs before replacement, so build failure preserves the
previous backend. Later Compose or readiness failure may leave some owned
services reconciled; the supervisor must not destroy a previously healthy
owned environment merely to create the appearance of rollback. It retains the
prior receipt, records the failed attempt, and lets `status` report the exact
healthy, unhealthy, or drifted reality.

An interrupted process may leave an orphaned journal or Compose project.
`status` reports project-labeled resources even when the active receipt is
missing. It prints exact environment-scoped recovery guidance; it never adopts
or deletes them silently.

## Status

```bash
./scripts/dev-env.sh status dev
./scripts/dev-env.sh status
```

With a name, status compares manifest, receipt, and live observations and
reports:

- requested refs/paths, resolved SHAs, and actual paths;
- manifest digest and post-start manifest drift;
- Compose project and each service's existence/health;
- configured Envoy port, ownership, and health;
- Vite ownership, PID, actual URL, and HTTP readiness;
- API and Vite log locations; and
- one of `not started`, `starting/orphaned`, `running`, `drifted`, `unhealthy`,
  or `stale receipt`.

A receipt alone never produces `running`.

Without a name, status lists known committed/local manifests and runtime
receipt/journal directories. It may inspect Compose resources carrying the
launcher-derived project identity, but it does not scan for or classify
arbitrary system processes.

The toolkit facade's `status` first retains its literal `host mode:` report,
then adds selected manifest/source/runtime/override/Envoy/Vite observations.

## Named shutdown

Normal shutdown is explicit:

```bash
./scripts/dev-env.sh down dev
```

It performs no source-control operation. It freshly derives the expected
identity, validates the receipt or attempt journal belongs to this workspace and
environment, verifies Vite's complete process identity, stops only that process
group, and brings down only the matching Compose project. It then removes the
active receipt while retaining useful logs.

A missing, forged, mismatched, or stale ownership record fails closed with
specific recovery evidence. Shutdown never guesses from a PID, broad container
name, port, or process search.

The toolkit facade remains:

```bash
./scripts/toolkit-contributor.sh down
./scripts/toolkit-contributor.sh --env toolkit down
```

Its default environment is `toolkit`. After the shared supervisor removes that
environment's owned web/backend resources, the facade asks the existing API
override helper to remove only its validated replacement.

## Environment-aware snapshots

Snapshots identify the running environment explicitly:

```bash
./scripts/dev-env.sh snapshot save dev my-repro
./scripts/dev-env.sh snapshot restore dev my-repro
```

They are stored under `envs/snapshots/<environment>/<name>.rdb`. The command
validates the active receipt and targets the `redis` service in that exact
Compose project. It no longer relies on a fixed Redis container name. Restore
keeps its interactive confirmation or explicit `--yes` requirement and stops /
starts only that environment's Redis service as required.

A manifest's optional `SNAPSHOT` value retains the current `up --restore`
behavior, but resolves within that environment's snapshot namespace.

## Toolkit-contributor integration

The public command set remains exactly:

```text
bootstrap  start  refresh  seed  status  down
```

The optional global `--env <name>` selects a toolkit-capable manifest; omitting
it selects `toolkit`. `bootstrap` retains its literal `/etc/os-release` and
`/proc/sys/kernel/osrelease` classification, Docker capability check, and exact
origin validation. Because one-command startup now requires assets, its
missing-clone set grows from the original four providers to include
`rpg-game-assets` on `main`; valid existing roots remain untouched. General
`game-dev` bootstrap continues to provide all seven canonical clones.

`start` resolves sources, enables the existing `rulebooks/dnd5e` override from
the resolved toolkit path inside the launcher-owned API runtime worktree,
builds that environment's local-toolkit image, starts the shared isolated
runtime, waits through the configured Envoy port, synchronizes assets, starts
or reuses Vite, and prints the actual sandbox URL.

`refresh` resynchronizes the selected toolkit path into that same API runtime
worktree, rebuilds before replacing only that environment's `rpg-api` service,
waits on that environment's Envoy port, and reports the existing source revision
and sync timestamp. It does not restart a matching Vite process.

`seed` and health checks use the active receipt's Envoy address, never a literal
8080. A manifest/receipt port mismatch is reported as drift; commands do not
silently contact the newly configured port before `start` reconciles it.

Toolkit mode rejects `RPG_API_PATH`: the override helper intentionally changes
`go.mod` and writes an owned local-toolkit tree, which must not occur in a
developer-owned API worktree. API changes are pushed and selected through
`RPG_API_REF`. Toolkit and web may be path-backed for live edits.

Each named toolkit environment has a separate managed API runtime worktree,
image, Compose project, override state, and Envoy port. Therefore parallel
toolkit environments are supported in one `game-dev` workspace when each uses
a ref-backed API and a distinct port. The override helper's one-replacement
rule remains true per API worktree.

## Command surface

The resulting normal flow is:

```bash
./scripts/dev-env.sh up dev
./scripts/dev-env.sh status dev
./scripts/dev-env.sh down dev
```

`pin` remains available for authors who intentionally update and commit an API
module pin. It operates only on an explicit developer-owned API path or the
primary API checkout selected by the user; it is not part of runtime source
resolution and never edits a managed running API worktree implicitly.

The toolkit flow is:

```bash
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed
./scripts/toolkit-contributor.sh status
./scripts/toolkit-contributor.sh down
```

## Repository ownership and delivery order

### `rpg-deployment` provider

Owns the local Compose contract:

- remove fixed local `container_name` declarations from participating files;
- parameterize the local API image and Envoy host port;
- remove non-Envoy host publications;
- preserve internal service names and networking; and
- add Compose-config tests proving two project names produce isolated resources
  and only the selected Envoy port is published.

### `rpg-dnd5e-web` provider

Owns explicit asset synchronization:

- accept `RPG_GAME_ASSETS_PATH`;
- validate the expected asset source;
- retain exact `rsync --delete` behavior; and
- prove the explicit-path mode performs no Git mutation or network operation.

### `game-dev` consumer/orchestrator

Owns manifests, source resolution, lifecycle, receipts, process ownership,
facade integration, snapshots, migration errors, documentation, and contract /
live tests.

Provider PRs land first. The `game-dev` PR consumes only merged provider
contracts. The cross-repository idea PR stays open as the review/tracking
surface until implementation and acceptance complete.

## Security and safety invariants

1. Manifest files are data and are never shell-evaluated.
2. Environment names cannot escape `envs/` or `.runtime/`.
3. Every developer-owned source path is origin-validated and Git-untouched.
4. Managed worktrees may be advanced only when their tracked state is known and
   owned.
5. An occupied Envoy port never triggers eviction or automatic fallback.
6. Compose project/image identity is derived, not caller-injected.
7. A PID is never sufficient proof of Vite ownership.
8. `down` never uses broad process/container filters as a kill target.
9. No readiness URL is printed before backend and Vite readiness pass.
10. The selected asset source is explicit; startup never pulls an incidental
    sibling checkout.
11. Toolkit API mutations occur only in a launcher-owned managed worktree.
12. Primary checkout branch, `HEAD`, index, and worktree bytes, plus every
    unrelated worktree, remain unchanged by runtime lifecycle commands; only
    launcher-owned refs/object data and launcher-managed worktrees may change.

## Verification strategy

### Deterministic contract tests

`rpg-deployment` tests rendered Compose configuration for:

- absence of `container_name`;
- only one host publication, Envoy at the supplied port;
- distinct project-scoped resource names for two environments;
- selected API image propagation; and
- no regression in internal service discovery.

`rpg-dnd5e-web` tests explicit asset-path success and failures with fixture
repositories, including source validation, deletion of stale destination
files, exact copied bytes, and an executable-path trap proving no Git command
runs.

`game-dev` tests:

- strict manifest grammar, key closure, traversal refusal, and `*_BRANCH`
  migration errors;
- ref/path precedence, origin checks, detached managed worktrees, and no primary
  checkout mutation;
- derived identity stability and cross-workspace distinction;
- occupied API port refusal before mutation;
- first start, rerun/reuse, config-driven Vite restart, and exact URL parsing;
- atomic receipt/journal behavior and forged/stale ownership refusal;
- named status/down and environment-aware snapshots;
- all bounded failure cleanup paths;
- toolkit configured-port start/refresh/seed/status/down behavior; and
- the toolkit facade's exact six commands and literal host classification.

### Live native-Ubuntu and WSL2 acceptance

On each supported host, retain evidence that:

1. One `up dev` from a prepared `game-dev` workspace starts assets, the isolated
   API stack, and background Vite and prints a working normal-game URL.
2. Two manifests with distinct Envoy ports run simultaneously and receive
   different Vite-selected URLs.
3. Rerunning one environment rebuilds/reconciles its API and reuses its owned
   unchanged Vite.
4. Stopping one environment leaves the other's backend and browser URL healthy.
5. A path-backed web worktree reflects an edit through HMR.
6. A path-backed toolkit worktree changes seed-visible behavior only after
   toolkit `refresh`, through that environment's configured port.
7. Ref-backed sources record and execute the pushed immutable SHAs.
8. An occupied Envoy port, malformed manifest, wrong repository origin,
   stale/forged receipt, failed asset sync, failed Compose start, and failed
   Vite start leave primary checkouts and unowned processes/resources intact.
9. Final shutdown leaves no owned Vite process, Compose project, published API
   port, or active receipt.

Native and WSL2 results are separate attestations. Hosted CI does not substitute
for either live lifecycle proof.

## Migration and compatibility

- `envs/dev.env` moves from `*_BRANCH` to explicit `*_REF` selectors and gains
  deployment, assets, and Envoy port.
- `envs/toolkit.env` is added with port 8081.
- `down` becomes `down <environment>` for `dev-env.sh`; an omitted name fails
  with usage rather than stopping an ambiguous stack.
- `status` accepts an optional environment and lists the registry when omitted.
- Snapshot syntax gains the environment before snapshot name.
- `--clean-web` remains, with safer `npm ci` semantics that preserve the lock
  file.
- Existing manually started fixed-name stacks are not adopted or killed. The
  migration runbook asks the operator to stop them explicitly before the first
  isolated start.
- Toolkit keeps its six commands and default no-argument environment selection;
  only its global optional `--env` is additive.

## Resolved decisions

- The permanent workflow supports multiple simultaneous branch games.
- Envoy ports are explicit manifest data; Vite ports are Vite-selected observed
  data.
- Only Envoy is host-published.
- Desired configuration and observed runtime state are separate records.
- Shutdown is named and ownership-checked.
- Rerun reconciles the backend and reuses unchanged owned Vite.
- Normal and toolkit workflows share one runtime ownership contract.
- Pushed refs and explicit local worktree paths are both first-class; paths win.
- The launcher manages detached runtime worktrees only for ref-backed sources.
- Primary repositories are clean integration bases; feature edits belong in
  worktrees.
- Provider contracts land before the `game-dev` consumer.
