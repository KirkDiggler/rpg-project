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

| Mode            | Meaning                                              |
| --------------- | ---------------------------------------------------- |
| `ubuntu-native` | Ubuntu running on a kernel with no Microsoft marker. |
| `ubuntu-wsl2`   | Ubuntu whose kernel release contains a WSL2 marker.  |

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

| `/etc/os-release` ID                           | `/proc/sys/kernel/osrelease` content | Result                                             |
| ---------------------------------------------- | ------------------------------------ | -------------------------------------------------- |
| not `ubuntu`, missing, duplicate, or malformed | any                                  | Refuse: unsupported or unreadable Ubuntu identity. |
| `ubuntu`                                       | contains `wsl2`                      | `ubuntu-wsl2`                                      |
| `ubuntu`                                       | contains `microsoft` but not `wsl2`  | Refuse: WSL1 is unsupported.                       |
| `ubuntu`                                       | contains neither marker              | `ubuntu-native`                                    |
| `ubuntu`                                       | unreadable or empty                  | Refuse: host cannot be classified.                 |

The WSL2 marker takes precedence when both markers appear, as in ordinary WSL2
kernel releases. A Microsoft marker without WSL2 is specifically treated as
WSL1 rather than native Linux. A generic Ubuntu kernel is native; the facade
does not require a vendor, desktop, init system, or kernel-version allowlist.

### Required failures

Every refusal is fail-closed and names both the observation and the supported
next step. The exact final wording may be concise, but the messages must
convey these distinctions:

| Condition                                             | Actionable failure meaning                                                                                                                                                                       |
| ----------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Cannot read `/etc/os-release`                         | The facade cannot determine the Ubuntu distribution; run it from supported Ubuntu native or Ubuntu WSL2.                                                                                         |
| Missing/malformed/non-Ubuntu `ID`                     | The facade supports Ubuntu native and Ubuntu WSL2 only; identify the observed `ID` when safe to do so.                                                                                           |
| Cannot read or has empty `/proc/sys/kernel/osrelease` | The facade cannot determine native versus WSL; use a supported Ubuntu host with that kernel file available.                                                                                      |
| Microsoft without WSL2                                | Ubuntu WSL1 is unsupported; use Ubuntu WSL2 or boot native Ubuntu.                                                                                                                               |
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

Port ownership is a **native live-acceptance prerequisite, not a facade
feature**. Before a clean native live run, the acceptance operator records
listening owners for the current local surface:

```text
3001  3002  3003  8080
```

Use a non-mutating listener observation such as `ss -ltnp` and report the
listener/process details available to the operator. If inspection is unavailable
or any of those ports is already occupied before the clean run, the native
acceptance run is blocked and reported as an environmental prerequisite; it is
not repaired by the implementation or silently treated as clean.

Docker compose port-bind failures and Vite bind failures remain fail-closed.
The acceptance run stops and reports the bind error (or a Vite-selected port
other than the documented `:3001`) rather than killing the owner, accepting an
alternate URL, or building a port allocation framework. No generic port
manager, port scanner command, alternate-port configuration, or automatic
retry/cleanup mechanism is in scope.

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
- document the native/WSL2 Docker failure distinctions and the unsupported
  WSL1/non-Ubuntu refusals; and
- state the occupied-port acceptance prerequisite and no-auto-kill rule rather
  than presenting it as a new contributor command.

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
| WSL2 classification       | `ID=ubuntu` plus representative case variants containing `wsl2` returns `ubuntu-wsl2`, including a normal Microsoft WSL2 release.                                                                                                   |
| WSL1 refusal              | `ID=ubuntu` plus `microsoft` without `wsl2` fails before any external command.                                                                                                                                                      |
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

## Clean native-Ubuntu acceptance

After the future `game-dev` implementation PR is merged, an independent clean
native-Ubuntu run is required before claiming this design delivered. The
operator records the exact merged commits and compact exit-code transcript.

1. Confirm a clean supported host: `/etc/os-release` has `ID=ubuntu`, the
   kernel release has neither Microsoft nor WSL2, required tools are present,
   `docker info` succeeds, and GitHub SSH works. Record the selected
   `ubuntu-native` mode.
2. Perform the non-mutating preflight for ports `3001`, `3002`, `3003`, and
   `8080`. Record listener output and stop if any is occupied or cannot be
   observed; do not terminate anything.
3. Clone a fresh `game-dev` checkout. Run `bootstrap` twice and show that the
   first run creates exactly the four sandbox roots and the second leaves valid
   roots unchanged.
4. Run `start`, `seed`, and `status`; retain the fixed compose order,
   `ubuntu-native` status line, Envoy-serving result, and two consecutive
   seed outcomes. The Fighter must still show Protection, Strength 16, and the
   real equipped shield.
5. From a disposable toolkit checkout only, execute the approved local Human
   Strength marker proof: baseline 16; temporarily change the local Human STR
   contribution from 1 to 2; `refresh` then `seed` observes 17; restore the
   source; `refresh` then `seed` returns to 16. This remains an existing API
   projection proof, not a debug API/proto change.
6. Start the normal Vite command only after the port preflight. Verify it
   serves the documented `http://localhost:3001/?toolkitSandbox=1` URL, save
   the fixed template through `PutDungeon`, and capture normal `GameView`
   evidence for Fighter, Barbarian, Fighter then Barbarian, and Barbarian then
   Fighter. Harness URLs remain supplemental only.
7. Run the established production Dev-header, production sandbox-route, and
   wrong-owner Create/Join negatives. Run `down` and prove only the owned
   `rpg-api/local-toolkit/rulebooks/dnd5e` tree is removed while checkouts and
   local toolkit edits remain intact.

A current occupied native port, an alternate Vite port, a failed Docker bind,
or a missing observation is an acceptance blocker with recorded diagnosis. It
is not a reason to add process management or widen the feature.

## WSL2 verification pickup for #210

Native evidence does not replace the existing clean-WSL2 verification. Once
the original provider, facade, and web units are merged and the additive host
change is merged, [rpg-project#210](https://github.com/KirkDiggler/rpg-project/issues/210)
remains the evidence-only WSL2 acceptance record. It has no branch or PR.

The following is a self-contained pickup packet for the person executing that
later verification. It is intentionally evidence-only and must not be used to
open a new implementation wave.

```markdown
<!-- pih-dispatch:v1 -->

## Goal / symptom

The toolkit contributor sandbox now supports two explicit Ubuntu modes. It
still needs independent proof that Ubuntu WSL2 remains a supported first-class
mode and that native support did not weaken the merged WSL2 workflow.

## Desired outcome

On clean Ubuntu WSL2 with a Docker-compatible daemon reachable from the
Ubuntu distribution (Docker Desktop WSL integration is the documented remedy),
execute the complete existing contributor loop and post one concise evidence
comment. The facade must report `host mode: ubuntu-wsl2`.

## Contract boundaries

- Verification owner is `rpg-project`; this issue creates no branch, PR,
  source edit, deployment change, or GitHub implementation issue.
- Run only after the merged API provider, game-dev facade (including native
  host support), and web sandbox commits are recorded.
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
- `bootstrap` twice creates only the four required roots then leaves valid
  roots unchanged. `start`, `status`, `refresh`, `seed`, and `down` retain the
  original exact compose/Envoy/owned-cleanup behavior.
- `docker info` succeeds before each mutating facade command. A failed WSL2
  daemon check would report Docker Desktop WSL-integration or reachable-daemon
  guidance without changing Windows or Docker Desktop.
- Two seed runs produce exactly the fixed Fighter and Barbarian; the Fighter
  has Protection, Strength 16, and a real equipped shield.
- The marker transcript proves Strength 16 -> 17 only after refresh/reseed,
  then 17 -> 16 after restoration/refresh/reseed.
- The populated `toolkit-contributor-sandbox` template saves through
  `PutDungeon`; Fighter, Barbarian, Fighter then Barbarian, and Barbarian then
  Fighter reach normal `?playerId=` `GameView` routes with order-qualified
  screenshots.
- Evidence includes wrong-owner Create/Join refusal, production Dev-header
  refusal, production sandbox-route refusal, and post-`down` proof that only
  the owned D&D 5e local tree was removed.

## Verification evidence

Record exact merged commits, command exit codes, host-mode/status output,
Docker/Envoy results, bootstrap rerun output, two seed results, the 16 -> 17
-> 16 marker transcript, successful `PutDungeon` key, six normal-route
screenshots, negative-test output, and final owned-tree check. Report any
occupied expected port as an environmental block; never kill its owner.

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
  WSL2 marker means `ubuntu-wsl2`, Microsoft without WSL2 rejects WSL1, and no
  Microsoft marker means `ubuntu-native`.
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

- Test-first coverage proves native Ubuntu, WSL2, WSL1 refusal, Debian/non-
  Ubuntu refusal, malformed/missing observations, no environment bypass,
  host-gate ordering for every command, and Docker pre-mutation failures for
  bootstrap/start/refresh/down.
- The Docker failure text is actionable and mode-specific. Native accepts a
  working compatible daemon without a Docker Desktop requirement.
- Existing facade contract assertions continue to pass; `seed` remains one
  seeder delegation, and `status` remains override status plus one Envoy health
  check while including `host mode: ubuntu-native` or `host mode: ubuntu-wsl2`.
- Docs name both supported modes, literal observations, unsupported WSL1/non-
  Ubuntu, native daemon guidance, WSL2 guidance, and no-auto-kill port rule.
- A clean native-Ubuntu live acceptance records free-port observation,
  bootstrap rerun, full start/refresh/seed/status/down loop, 16 -> 17 -> 16
  marker proof, all four normal GameView party outcomes, negatives, and owned
  shutdown.

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
   tooling or killing processes.
6. Execute #210 later as its independent evidence-only Ubuntu WSL2 acceptance.
   Its evidence comment has no branch, PR, or closing keyword. Kirk may close
   it only after accepting that evidence.
7. #208 remains the parent tracking issue until Kirk accepts all required
   original and additive evidence. No implementation PR closes the parent.

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
clean native acceptance, then the independent #210 WSL2 evidence. Documentation
lands with the facade so the stated support boundary and actual gate cannot
drift.

Residual operational risks are bounded rather than hidden:

| Risk                                                  | Treatment                                                                                                                                                                   |
| ----------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Future kernel-release wording differs                 | The documented marker rules are isolated in the pure classifier and fixture matrix; unknown Microsoft-without-WSL2 remains fail-closed rather than misclassified as native. |
| `/etc/os-release` syntax is surprising or hostile     | Parse only the needed `ID` as data and never execute the file; malformed/ambiguous input refuses.                                                                           |
| `docker info` passes but build/compose later fails    | Subsequent Docker failures remain fail-closed and report their real operation; no vendor heuristic claims compatibility.                                                    |
| Existing listener owns a required port                | The acceptance preflight records and blocks on it; no process is killed and no alternate port is accepted.                                                                  |
| Native work accidentally changes the sandbox contract | The preserved-command assertions and narrow owning-issue packet keep API, web, toolkit, proto, deployment, and seed behavior out of scope.                                  |

## Design self-review

- The scope is additive: one existing facade, six existing commands, two Ubuntu
  modes, and no new runtime workflow.
- Production observations are literal, the classification seam is testable, and
  no environment bypass or unspecified host fallback exists.
- WSL2, WSL1, native Ubuntu, non-Ubuntu, malformed files, daemon failures,
  command ordering, ports, docs, and live evidence each have explicit behavior.
- Docker messages distinguish native from WSL2 without creating a Docker vendor
  requirement; later Docker/Vite failures remain fail-closed.
- Native and WSL2 acceptance, issue ownership, evidence, review, and closure
  order are explicit. No placeholder, deferred decision, or chat-only
  assumption is needed to produce the subsequent implementation plan.
