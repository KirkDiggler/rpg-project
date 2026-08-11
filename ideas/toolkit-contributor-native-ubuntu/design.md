# Native Ubuntu support for the toolkit contributor sandbox

**Status:** Draft — Kirk review

**Parent:** [rpg-project#208](https://github.com/KirkDiggler/rpg-project/issues/208)

**Additive to:** merged [rpg-project PR #209](https://github.com/KirkDiggler/rpg-project/pull/209)

**Scope:** make native Ubuntu a permanent first-class host beside Ubuntu WSL2
for the already-approved six-command toolkit contributor sandbox. This is an
additive host-gate and documentation design, not a redesign of the sandbox.

## Decision and outcome

`game-dev/scripts/toolkit-contributor.sh` remains the one contributor facade
with exactly these six commands:

```text
bootstrap  start  refresh  seed  status  down
```

The facade supports exactly two explicit host modes:

| Mode            | Meaning                                                                         |
| --------------- | ------------------------------------------------------------------------------- |
| `ubuntu-native` | Ubuntu whose kernel has neither a Microsoft nor `WSL2` marker.                  |
| `ubuntu-wsl2`   | Ubuntu whose kernel case-insensitively contains `WSL2` or `microsoft-standard`. |

Both are first-class supported modes. Native Ubuntu accepts any working
Docker-compatible daemon reachable through the existing `docker` CLI; it does
not require, identify, configure, or install Docker Desktop. Ubuntu WSL2 keeps
the existing Docker Desktop WSL-integration guidance, while accepting any
reachable compatible daemon that makes the required Docker commands work.

This addition preserves the approved loop and its vocabulary:

- the contributor edits only `rpg-toolkit/rulebooks/dnd5e`;
- the API helper continues to own the single D&D 5e local override;
- `seed` remains the API's real Envoy client, never a storage or character-data
  path;
- the three existing compose overlays remain in this exact order:

  ```bash
  docker compose \
    -f docker-compose.local-dev.yml \
    -f docker-compose.api.yml \
    -f docker-compose.local-api-src.yml
  ```

- explicit `refresh`, the fixed seed identities, the one Dungeon Builder
  template, the real lobby flow, and normal `GameView` links remain unchanged.

No source of truth, toolkit behavior, API contract, web behavior, deployment
configuration, or local-override ownership changes as part of host support.

## Problem

The merged facade currently treats WSL2 as the only supported host. Kirk's
native Ubuntu environment can run the same local Docker/API/Vite loop, but
there is no approved contract that distinguishes supported native Ubuntu from
WSL1, other Linux distributions, or an arbitrary environment-variable bypass.

A permissive "Linux" check would make failures harder to diagnose and silently
widen operational support. A second native script would duplicate the exact
bootstrap, override, compose, health, and cleanup safety rules that the current
facade already centralizes. The correct extension is one classified host gate
at the existing facade boundary.

## Supported-host contract

### Production observations and classification

Production code reads these literal files on every valid facade-command invocation:

```text
/etc/os-release
/proc/sys/kernel/osrelease
```

The implementation must not make either path configurable. It must not read a
host mode from the environment, a configuration file, a CLI flag, or a Docker
context name. In particular, former or hypothetical variables such as
`TOOLKIT_CONTRIBUTOR_OSRELEASE`, `TOOLKIT_CONTRIBUTOR_OS_RELEASE`,
`OS_RELEASE_PATH`, and `HOST_MODE` have no effect.

The host gate has two layers:

1. A pure, side-effect-free classification seam receives already captured
   contents of both files and returns either one of the two mode tokens or a
   classified refusal. It performs no file access, subprocess call, Docker
   call, checkout inspection, or mutation.
2. The production reader captures the two literal files and passes those
   contents to the seam. It is the only path used by the six commands.

This makes fixture classification direct and deterministic without creating a
production bypass. Tests may pass strings to the pure seam; they must not
change production file paths or set an environment value that alters host
selection.

The `/etc/os-release` reader is deliberately data parsing, not shell execution:

- ignore blank lines and comment lines;
- accept exactly one `ID=` assignment with the literal value `ubuntu`, either
  bare or simply single/double quoted;
- reject a missing, empty, duplicate, or otherwise malformed `ID` assignment;
- reject every other `ID` value;
- never `source`, `eval`, or otherwise execute the file.

The kernel capture must be non-empty. Classification is case-insensitive for
its markers and follows this order exactly:

| `/etc/os-release` ID                           | `/proc/sys/kernel/osrelease` content                              | Result                                             |
| ---------------------------------------------- | ----------------------------------------------------------------- | -------------------------------------------------- |
| not `ubuntu`, missing, duplicate, or malformed | any                                                               | Refuse: unsupported or unreadable Ubuntu identity. |
| `ubuntu`                                       | contains `wsl2` or `microsoft-standard`                           | `ubuntu-wsl2`                                      |
| `ubuntu`                                       | contains `microsoft`, but neither `wsl2` nor `microsoft-standard` | Refuse: WSL1 is unsupported.                       |
| `ubuntu`                                       | contains no `microsoft` marker and no `wsl2` marker               | `ubuntu-native`                                    |
| `ubuntu`                                       | unreadable or empty                                               | Refuse: host cannot be classified.                 |

`wsl2` and `microsoft-standard` are both WSL2 signatures and take precedence
before the WSL1 rule. This includes the legacy WSL2 fixture
`4.19.128-microsoft-standard`; the WSL1 fixture `4.4.0-19041-Microsoft` has a
Microsoft marker but neither WSL2 signature and is refused. This is the
signature boundary: any Ubuntu release containing either WSL2 signature is
`ubuntu-wsl2`; a Microsoft-containing release with neither signature is
unsupported WSL1; a non-Microsoft Ubuntu release with no WSL2 marker is
`ubuntu-native`. No kernel version, vendor, desktop, or init-system allowlist
is required.

### Required failures

Every refusal is fail-closed and names both the observation and the supported
next step. The exact final wording may be concise, but the messages must
convey these distinctions:

| Condition                                             | Actionable failure meaning                                                                                                                                                                       |
| ----------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Cannot read `/etc/os-release`                         | The facade cannot determine the Ubuntu distribution; run it from supported Ubuntu native or Ubuntu WSL2.                                                                                         |
| Missing/malformed/non-Ubuntu `ID`                     | The facade supports Ubuntu native and Ubuntu WSL2 only; identify the observed `ID` when safe to do so.                                                                                           |
| Cannot read or has empty `/proc/sys/kernel/osrelease` | The facade cannot determine native versus WSL; use a supported Ubuntu host with that kernel file available.                                                                                      |
| Microsoft without `WSL2` or `microsoft-standard`      | Ubuntu WSL1 is unsupported; use Ubuntu WSL2 or boot native Ubuntu.                                                                                                                               |
| Docker CLI missing in `ubuntu-native`                 | Provide the Docker CLI and a reachable Docker-compatible daemon; the facade will not install or configure either.                                                                                |
| `docker info` fails in `ubuntu-native`                | Start or repair a Docker-compatible daemon reachable to `docker info`; Docker Engine, rootless Docker, or another compatible daemon is acceptable if the required build/compose operations work. |
| Docker CLI missing in `ubuntu-wsl2`                   | Provide the Docker CLI and a daemon reachable from this Ubuntu WSL2 distribution; the facade will not change Windows or Docker Desktop.                                                          |
| `docker info` fails in `ubuntu-wsl2`                  | Enable Docker Desktop WSL integration for this distro or otherwise make a compatible daemon reachable; the facade will not change Windows or Docker Desktop.                                     |

The native messages intentionally do not tell a contributor to install Docker
Desktop. The WSL2 messages preserve the existing Docker Desktop integration
remedy without asserting that it is the only acceptable working daemon.

## Components and data flow

```text
six-command facade
  |
  +--> literal host reader
  |      +--> /etc/os-release ------------------+
  |      +--> /proc/sys/kernel/osrelease -------+--> pure host classifier
  |                                                     |
  |                                                     +--> ubuntu-native
  |                                                     +--> ubuntu-wsl2
  |                                                     +--> fail closed
  |
  +--> mode-aware Docker preflight (mutating commands only)
  +--> existing checkout/layout and tool checks
  +--> existing API override / Docker build / compose / Envoy commands
  +--> existing API sandbox seeder and web loop
```

The classifier supplies a small mode token only for facade reporting and
mode-aware diagnostics. It does not select a separate command path, compose
file, module target, image, API endpoint, identity, or port. Both modes run
the same commands against the same workspace layout.

### Gate order and command behavior

Host classification is the first operational gate for **every** command. It
runs after argument validation but before any repository or runtime action:
no `git`, API-helper, `go`, Docker, compose, seed, or checkout action may occur
before a supported mode is established.

`bootstrap`, `start`, `refresh`, and `down` also require a reachable Docker
daemon before their first mutation. Their prelude is:

1. validate the literal Ubuntu host and obtain the mode;
2. confirm that the Docker CLI is available;
3. run `docker info` and issue the mode-specific refusal on failure;
4. run the command's existing non-mutating prerequisite checks; and
5. perform the existing command behavior only after those gates succeed.

Consequently, a failed `docker info` occurs before any of these mutations:

| Command     | Mutations that must not run after Docker preflight fails                             |
| ----------- | ------------------------------------------------------------------------------------ |
| `bootstrap` | GitHub SSH/clone work that could create a required root.                             |
| `start`     | API override `on`, local API image build, and compose `up -d`.                       |
| `refresh`   | API override `refresh`, local API image build, and compose replacement of `rpg-api`. |
| `down`      | compose `down` and API override `off`.                                               |

`seed` and `status` run the same host gate first, then preserve their approved
behavior. `seed` still delegates exactly once to the API seeder through Envoy;
it does not add a Docker probe, storage path, or health poll. `status` still
asks the API override helper for its owned D&D 5e state and makes one Envoy
health call, without listing arbitrary containers. It additionally prints:

```text
host mode: ubuntu-native
```

or:

```text
host mode: ubuntu-wsl2
```

The host-mode line is printed only after successful classification. It does not
claim that Docker, Envoy, or the stack is healthy; the existing status results
remain authoritative for those facts.

The existing per-command responsibilities remain intact:

| Command     | Preserved responsibility                                                                                                                                                                  |
| ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `bootstrap` | Verify tools and GitHub SSH, clone only missing `rpg-toolkit`, `rpg-api`, `rpg-dnd5e-web`, and `rpg-deployment` roots, and leave valid existing checkouts untouched.                      |
| `start`     | Turn on the owned `rulebooks/dnd5e` override, build `rpg-api:local`, compose the three overlays in their fixed order, wait for Envoy, then print the normal Vite command and sandbox URL. |
| `refresh`   | Resync the already-owned D&D 5e source, build before replacing only `rpg-api`, wait for Envoy, and report the existing source revision/sync time.                                         |
| `seed`      | Invoke `go run ./cmd/sandboxseed --address localhost:8080` in `rpg-api`.                                                                                                                  |
| `status`    | Report the owned override and one Envoy health result, now also naming the classified host mode.                                                                                          |
| `down`      | Use the same compose overlays and then ask the API helper to remove only its owned D&D 5e replacement/tree.                                                                               |

The existing clone-origin checks, dirty-checkout preservation, one-module
override ownership/state validation, dedicated local Dockerfile, release-pin
protection, compose order, bounded Envoy health behavior, RPC seed sequence,
and `down` cleanup refusal remain unchanged. Native support must not weaken any
of them. `status` prints its host-mode line immediately after the successful
host gate and before its existing helper-status/Envoy calls, so the selected
mode is visible even when a later status check fails.

### Bootstrap checkout roots and branch contract

The four sandbox roots have one explicit absent-root clone mapping. It is part
of the existing `bootstrap` contract, not a clean-acceptance workaround:

| Root             | `repo_branch` | Exact origin URL                                |
| ---------------- | ------------- | ----------------------------------------------- |
| `rpg-toolkit`    | `main`        | `git@github.com:KirkDiggler/rpg-toolkit.git`    |
| `rpg-api`        | `dev`         | `git@github.com:KirkDiggler/rpg-api.git`        |
| `rpg-dnd5e-web`  | `dev`         | `git@github.com:KirkDiggler/rpg-dnd5e-web.git`  |
| `rpg-deployment` | `main`        | `git@github.com:KirkDiggler/rpg-deployment.git` |

For a root which is absent, the facade selects the mapped branch before any
later helper/build operation:

```bash
repo_branch=dev
repo_url=git@github.com:KirkDiggler/rpg-api.git
repo_root="$ROOT/rpg-api"
git clone --branch "$repo_branch" "$repo_url" "$repo_root"
```

The same literal `repo_branch` / `git clone --branch` form is used for all four
rows. `--single-branch` is deliberately absent: this change needs an initial
checkout branch, not a narrower ref-retention policy, and the existing facade
has no reason to change ordinary clone breadth.

For an already-present root, `bootstrap` validates that it is a Git checkout
with exactly the table's `origin` URL, then leaves it alone. It preserves its
current branch, detached state if any, staged and unstaged edits, untracked
files, and other working state. In particular, the facade must never
`fetch`, `switch`, `checkout`, `pull`, `reset`, `clean`, or `stash` an existing
root. An origin mismatch or invalid checkout refuses before any mutation.

The mapping is required because the original provider work is not a `main`
baseline: rpg-api PR #792 (`9099953f9bc86efbed9bf62209a96c54d9383d6b`) merged
to `dev`, and rpg-dnd5e-web PR #747
(`cfa63138a1f06de65991c31b006f29fc2af1ad74`) merged to `dev`. Their commits
are not guaranteed to be ancestors of the corresponding `main` tips. A fresh
API/web `dev` clone makes the approved sandbox provider/web baseline directly
reachable without a release-to-`main` prerequisite, while toolkit and
deployment keep their approved `main` baselines. Clean acceptance therefore
asserts `main/dev/dev/main` checkout branches and verifies #792/#747 ancestry
against the API/web **dev clones**, respectively. This branch choice never
changes an existing checkout.

The API-helper seam is also unchanged. The facade continues to call only its
existing D&D 5e ownership commands: `on --target rulebooks/dnd5e --src`,
`refresh --src`, `status`, and `off`. It does not duplicate the helper's
one-replace validation, state parsing, sync, release-pin protection, or exact
cleanup rules.

## Docker, ports, and fail-closed operation

`docker info` is the universal daemon reachability check. It must not inspect a
vendor string or require Docker Desktop, a system service name, a package
manager, root access, or a particular Docker context. Passing `docker info`
does not promise that every later build or compose operation will succeed;
those existing operations remain their own truthful failure points.

The facade never kills a host process. It must not introduce `kill`, `pkill`,
`fuser -k`, `lsof -t | xargs kill`, a container sweep, or any other generic
port-remediation action. Its existing explicit `down` continues to stop only
the known composition and then clean only the API helper's owned replacement;
it is not a port-killer.

Port ownership is an **acceptance-operator observation for native and WSL2
live runs, not a facade feature**. Before a clean live run, the operator records
listening owners for the fixed local surface:

```text
80, 3001, 3002, 6380, 8080
```

Port `3001` is the normal Vite listener; `80`, `3002`, `6380`, and `8080` are
fixed Docker Compose host publications. Run `ss -ltnp` manually as
acceptance/operator evidence only and report the listener/process details
available to the operator. The facade does not invoke `ss` or any scanner. If
inspection is unavailable or any of those ports is already occupied before the
clean run, acceptance is blocked and reported as an environmental prerequisite;
it is not repaired by the implementation or silently treated as clean.

Docker Compose port-bind failures and Vite bind failures remain fail-closed.
The acceptance run stops and reports the bind error (or a Vite-selected port
other than the documented `:3001`) rather than killing the owner, accepting an
alternate URL, or building a port allocation framework. No scanner, port
manager, alternate-port behavior or configuration, automatic retry/cleanup
mechanism, or process kill is added to the facade.

## Documentation contract

The `game-dev` README link and the toolkit contributor sandbox runbook must be
updated together so they describe one loop, not two scripts.

The runbook will:

- say the loop supports **native Ubuntu and Ubuntu WSL2**, explicitly naming
  `ubuntu-native` and `ubuntu-wsl2` as the facade's status modes;
- show the two literal host observations (`/etc/os-release` and
  `/proc/sys/kernel/osrelease`) and explain the Ubuntu-only rule;
- give native Ubuntu prerequisites as Git/GitHub SSH, Go, Node/npm, `rsync`,
  `jq`, the Docker CLI, and any working Docker-compatible daemon, without
  Docker Desktop, Windows, installer, or daemon-configuration instructions;
- retain WSL2 guidance for Docker Desktop WSL integration and make clear that
  the facade diagnoses rather than changes Windows, Docker Desktop, global
  tools, credentials, branches, or checkouts;
- retain the exact first-checkout, daily loop, explicit refresh, marker proof,
  Vite URL, and owned shutdown instructions from the merged runbook;
- document the native/WSL2 Docker failure distinctions, the exact WSL
  signature boundary, and the unsupported WSL1/non-Ubuntu refusals; and
- state that manually observing `80, 3001, 3002, 6380, 8080` is an
  acceptance-only prerequisite (`3001` for Vite; the remainder for Compose),
  with no facade scanner, port manager, alternate-port behavior, or auto-kill,
  rather than presenting it as a new contributor command.

The README remains a concise pointer to the runbook. It must not duplicate a
second bootstrap or broaden the general seven-repository bootstrap.

## Test-first verification matrix

Implementation begins with focused failing shell-contract coverage, then makes
that coverage pass while retaining the merged facade contract. The host
classifier is unit-tested directly with captured file contents; facade tests
exercise the literal production paths and command ordering. No test may depend
on a host-mode environment override.

| Area                      | Required automated evidence                                                                                                                                                                                                         |
| ------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Native classification     | `ID=ubuntu` plus a generic non-Microsoft kernel returns `ubuntu-native`.                                                                                                                                                            |
| WSL2 classification       | `ID=ubuntu` plus case variants containing `wsl2` or `microsoft-standard` returns `ubuntu-wsl2`; fixtures include legacy WSL2 `4.19.128-microsoft-standard` and a normal Microsoft WSL2 release.                                     |
| WSL1 refusal              | `ID=ubuntu` plus Microsoft without either WSL2 signature fails before any external command; the exact WSL1 fixture is `4.4.0-19041-Microsoft`.                                                                                      |
| Other distributions       | Debian and another non-Ubuntu `ID` fail before any external command.                                                                                                                                                                |
| Bad observations          | Missing/unreadable file captures, empty kernel content, and missing, empty, duplicate, or malformed `ID` all fail closed.                                                                                                           |
| No bypass                 | Setting legacy/hypothetical path or host-mode environment variables cannot make a generic, non-Ubuntu, or WSL1 fixture pass. Production reads only the two literal paths.                                                           |
| Every facade command      | `bootstrap`, `start`, `refresh`, `seed`, `status`, and `down` each reject an unsupported fixture before Git, Docker, API-helper, Go, compose, seed, or checkout actions.                                                            |
| Docker pre-mutation order | For `bootstrap`, `start`, `refresh`, and `down`, a failing `docker info` logs before—and prevents—clone, override, build, compose, and cleanup mutations. The error is mode-specific for native and WSL2 fixtures.                  |
| Seed/status preservation  | `seed` retains its one seeder delegation without a health poll; `status` retains one helper-status call and one Envoy health call without arbitrary container listing, and includes the expected host mode.                         |
| Existing safety contract  | Four-root-only bootstrap, exact origin validation, dirty preservation, no checkout/global-tool mutation, override/build/compose order, bounded health retry/timeout, source-report ordering, and owned `down` all continue to pass. |
| Documentation             | Focused assertions verify both host modes, literal observations, WSL1/non-Ubuntu refusal, native daemon guidance, and no-auto-kill/port-preflight language in the runbook and README link.                                          |

The existing `tests/toolkit-contributor-contract.sh` remains the primary
fixture contract. Its fake command log must prove ordering and absence of
later calls, not only exit status. The implementation may factor the pure
classifier for direct tests, but it may not expose a seventh user command or a
runtime test switch.

## Browser evidence protocol for both clean acceptances

Native and WSL2 acceptance use the **`chrome_devtools_*` MCP tools** as the
primary agent interaction surface. The browser protocol is live UI evidence,
not URL-only navigation. `screenshot.mjs` may be used only as a secondary visual
corroboration after a successful MCP action sequence; it cannot click Save,
choose a party, establish an order, extract links, or substitute for any
required action or assertion.

Start one sandbox page with `chrome_devtools_new_page` at
`http://localhost:3001/?toolkitSandbox=1`, retain its page identity, and use
`chrome_devtools_take_snapshot` before every interaction. For **each**
arrangement in this exact order — **Fighter**, **Barbarian**, **Fighter then
Barbarian**, **Barbarian then Fighter** — re-save the template and complete this
entire sequence before beginning the next arrangement:

1. `chrome_devtools_select_page` the retained sandbox page,
   `chrome_devtools_take_snapshot` it, locate the enabled **Save** button, and
   `chrome_devtools_click` it.
2. `chrome_devtools_wait_for` the exact visible text `Saved as "toolkit-contributor-sandbox".`. Use `chrome_devtools_evaluate_script` on the authoring result to record the
   successful `toolkit-contributor-sandbox` PutDungeon key and the order name
   in the evidence transcript. This is the authoring proof; a route image is
   not a substitute.
3. `chrome_devtools_wait_for` the selected party button to be enabled,
   `chrome_devtools_take_snapshot` its enabled state, and
   `chrome_devtools_click` it.
4. `chrome_devtools_wait_for` the exact normal `?playerId=` link or links
   expected for that arrangement. `chrome_devtools_evaluate_script` the visible
   links and assert their displayed hrefs are the expected normal routes:
   Fighter = fighter; Barbarian = barbarian; Fighter then Barbarian = fighter
   followed by barbarian; Barbarian then Fighter = barbarian followed by
   fighter.
5. Immediately — before selecting the sandbox page again or taking any later
   sandbox action — use `chrome_devtools_new_page` for **each displayed href**.
   On each new tab, `chrome_devtools_wait_for` `[data-testid="encounter-view"]`,
   then use `chrome_devtools_evaluate_script` to assert that selector exists,
   `chrome_devtools_take_snapshot` the normal GameView, and
   `chrome_devtools_take_screenshot` it to its order-qualified file. Only then
   `chrome_devtools_select_page` the sandbox page and start the next re-save
   after every link in the current arrangement has its evidence.

The six required normal-GameView screenshot names are exactly:

```text
toolkit-sandbox-fighter-only-fighter-gameview.png
toolkit-sandbox-barbarian-only-barbarian-gameview.png
toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png
toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png
toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png
```

If Save, the exact saved text, the selected-party enablement, normal links,
new-tab encounter selector, evaluation, or screenshot fails, retain the
current MCP snapshot/evaluation/error evidence, stop that acceptance sequence,
and take **no later save, party action, link capture, or end-of-run catch-up
screenshot**. Run only the declared marker/Vite/owned-down recovery. Harness
routes remain supplemental and never satisfy this protocol.

## Clean native-Ubuntu acceptance

After the future `game-dev` implementation PR is merged, an independent clean
native-Ubuntu run is required before claiming this design delivered. The
operator records the exact merged commits and compact exit-code transcript.

1. Confirm a clean supported host: `/etc/os-release` has `ID=ubuntu`, the
   kernel release has no Microsoft marker and no `WSL2` marker, required tools
   are present, `docker info` succeeds, and GitHub SSH works. Record the
   selected `ubuntu-native` mode.
2. Manually run `ss -ltnp` as acceptance/operator evidence for the fixed
   host-port set `80, 3001, 3002, 6380, 8080` (`3001` is Vite; the others are
   Docker Compose publications). Record listener output and stop if any is
   occupied or cannot be observed; do not terminate anything. This is not a
   facade scanner or port-management command.
3. Clone a fresh `game-dev` checkout. Run `bootstrap` twice and show that the
   first run creates exactly the four sandbox roots on `main/dev/dev/main` and
   the second leaves valid roots unchanged. Assert API #792 ancestry in the
   `rpg-api` **dev** clone and web #747 ancestry in the `rpg-dnd5e-web` **dev**
   clone; do not switch, fetch, or otherwise repair a root to make either
   assertion pass.
4. Run `start`, `seed`, and `status`; retain the fixed compose order,
   `ubuntu-native` status line, Envoy-serving result, and two consecutive
   seed outcomes. On both seed files, require the exact regular-expression
   records `sandboxseed: identity=toolkit-sandbox-fighter
character_id=[^[:space:]]+ strength=16 off_hand=shield` and
   `sandboxseed: identity=toolkit-sandbox-barbarian
character_id=[^[:space:]]+ strength=16`. The focused sandbox-seed
   integration report, rather than seeder stdout, is the evidence for
   Protection, FightingStyles, and a real equipped shield.
5. From a disposable toolkit checkout only, execute the approved local Human
   Strength marker proof: baseline fighter `strength=16 off_hand=shield`;
   temporarily change the local Human STR contribution from 1 to 2; `refresh`
   then `seed` observes fighter `strength=17 off_hand=shield`; restore the
   source; `refresh` then `seed` returns the fighter to
   `strength=16 off_hand=shield`. This remains an existing API projection
   proof, not a debug API/proto change.
6. Start the normal Vite command only after the port preflight. Verify it
   serves the documented `http://localhost:3001/?toolkitSandbox=1` URL, then
   execute the browser evidence protocol above: four fresh template saves,
   exact PutDungeon-key evidence, immediate new-tab normal-GameView proof, and
   six order-qualified encounter screenshots. Harness URLs remain supplemental
   only.
7. Run the established production Dev-header, production sandbox-route, and
   wrong-owner Create/Join negatives. Run `down` and prove only the owned
   `rpg-api/local-toolkit/rulebooks/dnd5e` tree is removed while checkouts and
   local toolkit edits remain intact. After the manifest and SHA256SUMS are
   complete, create `task3-native-evidence.tar.gz` and
   `task3-native-evidence.tar.gz.sha256`. Publish the signed evidence comment
   through the GitHub issue UI using actual `chrome_devtools_new_page`,
   `chrome_devtools_take_snapshot`, `chrome_devtools_upload_file`,
   `chrome_devtools_wait_for`, `chrome_devtools_evaluate_script`, and
   `chrome_devtools_click` calls to attach that archive, its checksum, and all
   six screenshots. Read it back as KirkDiggler's comment, require its marker,
   exact merge SHA, signature, all eight filenames, and at least eight
   `https://github.com/user-attachments/` URLs, then explicitly open/reload
   every URL and record a viewed statement.

A current occupied native port, an alternate Vite port, a failed Docker bind,
or a missing observation is an acceptance blocker with recorded diagnosis. It
is not a reason to add process management or widen the feature.

## WSL2 verification pickup for #210

Native evidence does not replace [rpg-project#210](https://github.com/KirkDiggler/rpg-project/issues/210)'s
original purpose: the independent clean-WSL2 evidence record. Before #210 is
executed, formally amend its issue contract and latest AGENT PICKUP to use the
landed Units A-C plus the landed additive native-host PR as its execution
baseline. That is an intentional superseding execution baseline, not an
undocumented dependency: the same landed facade must prove the WSL regression.
#210 remains the sole evidence-only WSL2 acceptance record, with no branch or
PR; do not create a duplicate WSL verification issue.

The following is the self-contained amendment/pickup packet for the person
executing that later verification. It is intentionally evidence-only and must
not be used to open a new implementation wave.

```markdown
<!-- pih-dispatch:v1 -->

## Goal / symptom

The toolkit contributor sandbox now supports two explicit Ubuntu modes. The
original #210 purpose remains independent proof that Ubuntu WSL2 is a
first-class supported mode and that native support did not weaken the merged
WSL2 workflow.

## Superseding execution baseline

Before execution, formally amend #210's issue contract and latest AGENT PICKUP
so this replaces its prior execution baseline: run against the landed Units A-C
plus the landed additive native-host PR. This is an intentional, documented
supersession so the same landed facade proves the WSL regression; it is not an
undocumented dependency and does not create a second WSL verification issue.

## Desired outcome

On clean Ubuntu WSL2 with a Docker-compatible daemon reachable from the
Ubuntu distribution (Docker Desktop WSL integration is the documented remedy),
execute the complete existing contributor loop and post one concise evidence
comment. The facade must report `host mode: ubuntu-wsl2`.

## Contract boundaries

- Verification owner is `rpg-project`; this issue creates no branch, PR,
  source edit, deployment change, or GitHub implementation issue.
- Run only after the formally amended execution baseline is satisfied and its
  exact merged commits are recorded: landed Units A-C plus the landed additive
  native-host PR.
- The only source mutation is the temporary local Human Strength marker in a
  disposable checkout; restore it before final refresh/evidence.
- Do not use a host-mode/path environment bypass, direct storage, a harness
  URL as success evidence, automatic process killing, an alternate port, or a
  Docker/Windows configuration change.
- Do not close #208 or use a closing keyword. This issue is completed by its
  accepted evidence comment.

## Acceptance

- `/etc/os-release` identifies Ubuntu and `/proc/sys/kernel/osrelease`
  classifies as `ubuntu-wsl2`; WSL1 and other distributions remain unsupported
  in the automated contract.
- `bootstrap` twice creates only the four required roots using
  `main/dev/dev/main`, then leaves valid roots unchanged. API #792 and web
  #747 ancestry pass in the API/web `dev` clones; `start`, `status`, `refresh`,
  `seed`, and `down` retain the original exact compose/Envoy/owned-cleanup
  behavior.
- `docker info` succeeds before each mutating facade command. A failed WSL2
  daemon check would report Docker Desktop WSL-integration or reachable-daemon
  guidance without changing Windows or Docker Desktop.
- Before the live run, manually record `ss -ltnp` evidence for the fixed
  host-port set `80, 3001, 3002, 6380, 8080`; `3001` is Vite and the rest are
  Docker Compose host publications. Occupancy or an unavailable observation
  blocks acceptance; the facade neither scans nor manages ports.
- Both seed files contain the exact Fighter record
  `sandboxseed: identity=toolkit-sandbox-fighter character_id=[^[:space:]]+
strength=16 off_hand=shield` and the exact Barbarian record
  `sandboxseed: identity=toolkit-sandbox-barbarian character_id=[^[:space:]]+
strength=16`. The focused integration report is the evidence for
  Protection, FightingStyles, and the real equipped shield.
- The marker transcript proves fighter `strength=16 off_hand=shield` ->
  `strength=17 off_hand=shield` only after refresh/reseed, then back to
  `strength=16 off_hand=shield` after restoration/refresh/reseed.
- The populated `toolkit-contributor-sandbox` template uses the browser
  evidence protocol: each arrangement saves first, records the exact successful
  PutDungeon key, opens each displayed normal `?playerId=` href in a new tab,
  verifies `[data-testid="encounter-view"]`, and captures its six
  order-qualified screenshots before the next save.
- Evidence includes wrong-owner Create/Join refusal, production Dev-header
  refusal, production sandbox-route refusal, and post-`down` proof that only
  the owned D&D 5e local tree was removed.

## Verification evidence

Record the formal #210 issue-contract/latest-AGENT-PICKUP amendment and exact
merged baseline commits, command exit codes, host-mode/status output,
Docker/Envoy results, manual `ss -ltnp` output for `80, 3001, 3002, 6380,
8080`, bootstrap rerun output, both exact seed records on both seed files, the
fighter `strength=16 -> 17 -> 16` marker transcript with `off_hand=shield`,
successful `PutDungeon` key, six normal-route screenshots, negative-test
output, and final owned-tree check. After the manifest and SHA256SUMS, create
`task4-wsl-evidence.tar.gz` and `task4-wsl-evidence.tar.gz.sha256`; use the
GitHub issue UI's actual `chrome_devtools_new_page`,
`chrome_devtools_take_snapshot`, `chrome_devtools_upload_file`,
`chrome_devtools_wait_for`, `chrome_devtools_evaluate_script`, and
`chrome_devtools_click` calls to attach them and all six PNGs to the signed
#210 evidence comment. Read back KirkDiggler's comment and require the WSL
marker, exact merge SHA, signature, all eight names, and at least eight
`https://github.com/user-attachments/` URLs; explicitly open/reload each URL
and record every viewed attachment statement. Report any occupied expected
port as an environmental block; never kill its owner.

## Related / dependencies

- Parent: https://github.com/KirkDiggler/rpg-project/issues/208
- Original merged design: https://github.com/KirkDiggler/rpg-project/pull/209
- This native-Ubuntu design: https://github.com/KirkDiggler/rpg-project/issues/211
- Verification record: https://github.com/KirkDiggler/rpg-project/issues/210

— asset-pipeline agent, on behalf of KirkDiggler
```

## Post-approval GitHub AGENT PICKUP packet

This is the future, owning-issue brief for the single additive `game-dev`
implementation. It is not authorization to create that issue, branch, or PR
until this design and its subsequent plan are approved. It keeps the eventual
worker in the host-gate/doc/test lane and prevents accidental API, web, or
deployment work.

```markdown
<!-- pih-dispatch:v1 -->

## Goal / symptom

The merged toolkit contributor facade supports only Ubuntu WSL2 even though
Kirk's native Ubuntu environment can run the same local loop. Add permanent
native Ubuntu support without splitting the facade or changing the approved
sandbox behavior.

## Desired outcome

One `game-dev` facade continues to expose exactly `bootstrap`, `start`,
`refresh`, `seed`, `status`, and `down`, and classifies only `ubuntu-native` or
`ubuntu-wsl2`. Native Ubuntu accepts any working Docker-compatible daemon;
Ubuntu WSL2 retains actionable Docker Desktop WSL-integration guidance.
`status` reports the classified mode.

## Contract boundaries

- Change only the `game-dev` facade, its focused shell contract tests, the
  toolkit contributor runbook, and the concise README pointer if needed.
- Production reads literal `/etc/os-release` and
  `/proc/sys/kernel/osrelease` on every command. Require exactly `ID=ubuntu`;
  case-insensitive `WSL2` or `microsoft-standard` means `ubuntu-wsl2`, a
  Microsoft-containing release with neither signature rejects WSL1, and a
  non-Microsoft Ubuntu release with no `WSL2` marker means `ubuntu-native`.
  This signature boundary has no kernel-version allowlist.
- Keep a pure classifier that receives captured file contents for direct tests;
  production paths are not configurable. No environment variable, flag,
  config, Docker vendor/context, or test bypass selects a host mode.
- Host gate precedes every repository/runtime action for all six commands.
  `docker info` precedes bootstrap/start/refresh/down mutations. `seed` and
  `status` retain their existing Envoy behavior; status adds only the host-mode
  line.
- Preserve four-root clone restrictions, exact-origin/dirty preservation,
  D&D 5e override ownership, dedicated local image, exact three-overlay order,
  bounded Envoy polling, RPC seeding, and owned `down` cleanup.
- Do not change rpg-api, rpg-dnd5e-web, rpg-toolkit, rpg-api-protos, or
  rpg-deployment. Do not add a second script, a new user command, installer,
  Docker/vendor configuration, watcher, catalog, retry/cleanup system, direct
  storage path, process killer, port manager, alternate-port framework, API,
  web, proto, or deployment change.

## Acceptance

- Test-first coverage proves native Ubuntu, WSL2 (including exact legacy
  `4.19.128-microsoft-standard`), WSL1 refusal (including exact
  `4.4.0-19041-Microsoft`), Debian/non-Ubuntu refusal, malformed/missing
  observations, no environment bypass, host-gate ordering for every command,
  and Docker pre-mutation failures for bootstrap/start/refresh/down.
- The Docker failure text is actionable and mode-specific. Native accepts a
  working compatible daemon without a Docker Desktop requirement.
- Existing facade contract assertions continue to pass; `seed` remains one
  seeder delegation, and `status` remains override status plus one Envoy health
  check while including `host mode: ubuntu-native` or `host mode: ubuntu-wsl2`.
- Docs name both supported modes, literal observations, unsupported WSL1/non-
  Ubuntu, native daemon guidance, WSL2 guidance, and no-auto-kill port rule.
- A clean native-Ubuntu live acceptance manually records `ss -ltnp` evidence
  for `80, 3001, 3002, 6380, 8080` (`3001` for Vite; the remainder for
  Compose), bootstrap rerun, full start/refresh/seed/status/down loop, 16 ->
  17 -> 16 marker proof, all four normal GameView party outcomes, negatives,
  and owned shutdown. This adds no facade scanner, port manager,
  alternate-port behavior, or process kill.

## Verification evidence

Run shell syntax, the focused contributor facade contract, existing bootstrap
contract, markdown formatting/checks, and `git diff --check`. Retain test logs
that demonstrate no later action after host/Docker refusal. Record a clean
native live transcript and the exact merged commits. A required independent
review must report no blocker before merge.

## Related / dependencies

- Parent: https://github.com/KirkDiggler/rpg-project/issues/208
- Original design: https://github.com/KirkDiggler/rpg-project/pull/209
- Additive design: https://github.com/KirkDiggler/rpg-project/issues/211
- Later WSL2 evidence-only verification: https://github.com/KirkDiggler/rpg-project/issues/210

— asset-pipeline agent, on behalf of KirkDiggler
```

## Delivery, safety, evidence, and closure order

1. This document stays **Draft** until Kirk approves it. It creates no
   implementation issue, branch, PR, source change, push, or GitHub mutation.
2. Only after design approval is a detailed additive plan written and separately
   approved on the appropriate tracking surface. The plan must retain this
   document's exact host, Docker, port, and non-goal constraints.
3. Only after both approvals, create one owning `game-dev` issue from the AGENT
   pickup packet, base one branch on freshly fetched `origin/main`, and make
   one additive PR. Do not guess its issue number.
4. That PR contains exactly one closing reference, to its own `game-dev` issue.
   It must not close #208, #210, #211, or the already-merged #209. Required
   review is a merge gate, not a post-merge formality.
5. After merge, retain automated evidence and execute the clean native-Ubuntu
   acceptance above. Record blockers truthfully rather than fixing unrelated
   tooling or killing processes. A fresh reviewer distinct from the executor
   (even if both comments are signed by the KirkDiggler account) must post the
   unique native review marker with PASS, package COMPLETE, findings none, the
   exact reviewed merge SHA, evidence marker, archive/checksum names,
   user-attachment URL, and signature before native closure.
6. Before executing #210, formally amend its issue contract and latest AGENT
   PICKUP to the intentional superseding execution baseline of landed Units
   A-C plus the landed additive native-host PR. Then execute #210 as the sole
   independent evidence-only Ubuntu WSL2 acceptance; its evidence comment has
   no branch, PR, or closing keyword. A fresh reviewer distinct from the WSL
   executor, though the same KirkDiggler account may sign, must post the unique
   WSL review marker with PASS, package COMPLETE, findings none, exact reviewed
   merge SHA, evidence marker, archive/checksum names, user-attachment URL,
   and signature. Kirk may close it only after accepting that amended-baseline
   evidence; do not create a duplicate WSL issue.
7. Kirk may close #208 only after accepting the clean native acceptance and
   the amended #210 evidence. Until then it remains the parent tracking issue;
   no implementation PR closes the parent.

## Non-goals

This design does **not** add or authorize:

- other Linux distributions, macOS, Windows-native support, or WSL1;
- a second native script, host auto-detection bypass, "any Linux" posture, or
  runtime host configuration;
- Docker/tool installers, Docker Desktop/Windows changes, daemon/vendor/context
  policy, or deployment configuration;
- watcher, scenario catalog, generic fixture/choice framework, retry engine,
  cleanup framework, or process/port killer;
- automatic port remediation, a generic port manager, or an alternate-port
  framework;
- API, web, toolkit, proto, deployment, direct-storage, character-data, or
  seed-RPC behavior changes;
- weakening clone/origin restrictions, dirty preservation, one-module override
  ownership, compose order, normal `GameView`, production auth, or ownership
  checks; or
- using current native occupied ports as a feature request rather than an
  acceptance prerequisite.

## Alternatives rejected

### Separate native and WSL2 scripts

Rejected because both scripts would own the same checkout safety, override,
compose, health, and cleanup behavior and would inevitably drift. One facade
with an explicit mode token makes the host distinction visible while preserving
one operational contract.

### Support any Linux distribution

Rejected because the approved support promise is Ubuntu native plus Ubuntu
WSL2. A broad kernel-only test would accept unvalidated distro layouts and turn
an actionable preflight into an ambiguous later Docker or tool failure.

### Environment or test bypass for host detection

Rejected because a configurable OS-release path or forced host mode turns a
fixture seam into a production escape hatch. Direct pure classification tests
supply captured contents while production always reads literal operating-system
files.

### Require Docker Desktop everywhere or inspect a daemon vendor

Rejected because native Ubuntu only needs a working Docker-compatible daemon.
Vendor detection adds a false restriction without proving the required build or
compose commands will work.

### Automatically free ports or offer another port

Rejected because killing a listener can destroy unrelated work and a different
Vite port can make the documented URL point at the wrong checkout. Occupancy is
reported as an acceptance blocker; Docker/Vite failures remain truthful.

## Migration, rollout, and risks

The change has no persistent state or data migration. Existing Ubuntu WSL2
contributors retain the same six commands, override target, compose overlays,
Envoy endpoint, seed identities, web URL, and shutdown behavior. Their only
observable additive status output is the explicit `ubuntu-wsl2` host-mode line
and clearer daemon diagnostics. Native Ubuntu contributors gain the same loop
with `ubuntu-native` reported by status.

Roll out through the focused test contract and required review first, then the
clean native acceptance, then #210's amended-baseline independent WSL2 evidence.
Documentation lands with the facade so the stated support boundary and actual
gate cannot drift.

Residual operational risks are bounded rather than hidden:

| Risk                                                  | Treatment                                                                                                                                                                        |
| ----------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Future kernel-release wording differs                 | The signature boundary is isolated in the pure classifier and fixtures: `WSL2` or `microsoft-standard` is WSL2; Microsoft without either remains fail-closed rather than native. |
| `/etc/os-release` syntax is surprising or hostile     | Parse only the needed `ID` as data and never execute the file; malformed/ambiguous input refuses.                                                                                |
| `docker info` passes but build/compose later fails    | Subsequent Docker failures remain fail-closed and report their real operation; no vendor heuristic claims compatibility.                                                         |
| Existing listener owns a required port                | The acceptance preflight records and blocks on it; no process is killed and no alternate port is accepted.                                                                       |
| Native work accidentally changes the sandbox contract | The preserved-command assertions and narrow owning-issue packet keep API, web, toolkit, proto, deployment, and seed behavior out of scope.                                       |

## Design self-review

- The scope is additive: one existing facade, six existing commands, two Ubuntu
  modes, and no new runtime workflow.
- Production observations are literal, the classification seam is testable, and
  no environment bypass or unspecified host fallback exists.
- WSL2, WSL1, native Ubuntu, non-Ubuntu, malformed files, daemon failures,
  branch-selected absent roots, untouched existing roots, command ordering,
  ports, docs, and live browser evidence each have explicit behavior.
- Docker messages distinguish native from WSL2 without creating a Docker vendor
  requirement; later Docker/Vite failures remain fail-closed.
- Native and WSL2 acceptance use the same Save/link/new-tab protocol; issue
  ownership, evidence, review, and closure order are explicit. No deferred
  decision or chat-only assumption is needed to produce the subsequent
  implementation plan.
