# Toolkit Contributor Onboarding and Live Sandbox Loop Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give an Ubuntu WSL2 contributor one explicit, API-backed loop for iterating on a local D&D 5e rulebook and reaching a real one- or two-seat encounter through Dungeon Builder, LobbyService, and normal `GameView`.

**Architecture:** The API remains the provider boundary: it temporarily replaces only `github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e` inside an ignored Docker build context and exposes a narrow RPC client seeder. `game-dev` composes that landed contract with the already-tracked local compose overlays. The web adds a development-query-gated sandbox whose immutable, identity-bound clients author one in-memory template, drive the existing lobby RPCs in four fixed seat orders, and then link to the unchanged normal player route.

**Tech Stack:** Bash, Go 1.25, gRPC/Envoy, Docker Compose, Redis through existing repositories only, React 19, TypeScript, Connect RPC, Vitest, React Testing Library, Vite, Docker Desktop WSL2.

## Global Constraints

- The design was approved on 2026-08-10; this plan itself must be approved before any implementation issue, branch, worktree, code change, or implementation PR is created.
- Work only in the owning repository and only after its owning issue exists: `rpg-api`, then `game-dev`, then `rpg-dnd5e-web`; the clean-WSL2 verification issue is independent and is opened after plan approval.
- The merge/provider order is API → game-dev → web. Web work may begin against the explicit API contracts in this plan, but its PR cannot merge before the API provider is landed.
- API and web branches start from freshly fetched `origin/dev` and target `dev`; game-dev and rpg-project branches start from freshly fetched `origin/main` and target `main`.
- Do not change `rpg-toolkit`, `rpg-api-protos`, or `rpg-deployment`; do not add a proto field, deployment overlay, debug endpoint, or API-only version value.
- Do not write Redis directly, construct `character.Data` or `MonsterData` directly, or mutate equipment slots directly. The sandbox seeder uses only existing RPCs.
- Do not implement or call `GetRequirements`, `SubmitChoices`, or `ValidateDraft`; use the fixed implemented section RPC sequence below.
- Keep the sandbox fixed: two Dev labels, one stable dungeon key, one checked-in canvas template, two fixed presets, and four fixed party choices. Do not add catalog discovery, arbitrary scenarios, a generic choice interpreter, a generic client factory, watcher/retry/cleanup state machine, importer/exporter, or deployment work.
- `ListCharacters` may identify deletions only for the authenticated identity that made that call. The seeder must delete every ID returned by that identity’s bounded list and must never accept an arbitrary deletion ID.
- The rulebook override is the only local source refresh. Browser refresh and Vite HMR do not reload Go source.
- The exact compose overlay order is `docker-compose.local-dev.yml`, `docker-compose.api.yml`, then `docker-compose.local-api-src.yml`; do not reorder or copy their definitions.
- `AUTH_DEV_MODE=true`, `RPG_AUTHORING_ENABLED=1`, and `RPG_CONTENT_DIR=/content` come from those existing compose files. Existing production authoring posture remains unchanged.
- Dev labels are not credentials. Production-mode servers must continue rejecting `authorization: Dev <player-id>` and production builds must not route to or render the sandbox.
- `CreateLobby` and `JoinLobby` independently enforce character ownership. Do not describe the existing character RPC surface as a general ownership authority beyond the authenticated-list discipline used by the seeder.
- Every implementation PR body contains exactly one closing keyword: one `Closes #` reference to its own repository issue, with no closing keyword for #208, another delivery issue, or a dependency. The independent verification issue is completed by its evidence comment and does not create a branch or PR.
- Preserve the existing normal `GameView` route. Harness URLs are diagnostic-only evidence and never satisfy the playable-route acceptance check.

---

## Delivery Control, Repository Ownership, and Issue Map

Create these issues only after this plan is approved. Add each to Project 19 before making its branch. The project has `Status`, `Team`, `Feature`, and `Kind` single-select fields; set the literal values shown here.

| Unit | Repository owner | Exact issue title                                      | Project 19 fields                                                | Branch base → PR target                                                            | Delivery/merge gate                                                                                            |
| ---- | ---------------- | ------------------------------------------------------ | ---------------------------------------------------------------- | ---------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| A    | `rpg-api`        | `Add D&D 5e local override and API sandbox seeder`     | Team `Platform`; Feature `Infra`; Kind `Build`; Status `Todo`    | fresh `origin/dev` → `dev`                                                         | Provider PR lands first. It has one branch and one PR for the complete override plus seeder unit.              |
| B    | `game-dev`       | `Add WSL2 toolkit contributor sandbox facade`          | Team `Platform`; Feature `Infra`; Kind `Build`; Status `Todo`    | fresh `origin/main` → `main`                                                       | Starts only after Unit A is merged. It consumes A’s script and binary contracts without copying them.          |
| C    | `rpg-dnd5e-web`  | `Add development-only toolkit contributor sandbox`     | Team `UI/UX`; Feature `The Dungeon`; Kind `Build`; Status `Todo` | fresh `origin/dev` → `dev`                                                         | May develop against the interface table below; merge only after A and B are landed and its API pin resolves A. |
| D    | `rpg-project`    | `Verify the toolkit contributor sandbox on clean WSL2` | Team `Cross-team`; Feature `Infra`; Kind `Verify`; Status `Todo` | no branch and no PR; evidence comment on this issue and final #208 tracking update | Open after plan approval; execute only when A–C are merged. It does not close #208.                            |

For each implementation unit, fetch before branching and derive its issue number from the one exact title GitHub returned; never guess or reuse a number:

```bash
api_issue="$(gh issue list -R KirkDiggler/rpg-api --state open --search 'in:title "Add D&D 5e local override and API sandbox seeder"' --json number,title | jq -r '.[] | select(.title == "Add D&D 5e local override and API sandbox seeder") | .number')"
test "$(printf '%s\n' "$api_issue" | sed '/^$/d' | wc -l)" -eq 1
git -C ~/game-dev/rpg-api fetch origin
git -C ~/game-dev/rpg-api worktree add -b "feat/${api_issue}-dnd5e-sandbox" \
  "$HOME/game-dev/rpg-api/.worktrees/${api_issue}-dnd5e-sandbox" origin/dev

game_dev_issue="$(gh issue list -R KirkDiggler/game-dev --state open --search 'in:title "Add WSL2 toolkit contributor sandbox facade"' --json number,title | jq -r '.[] | select(.title == "Add WSL2 toolkit contributor sandbox facade") | .number')"
test "$(printf '%s\n' "$game_dev_issue" | sed '/^$/d' | wc -l)" -eq 1
git -C ~/game-dev fetch origin
git -C ~/game-dev worktree add -b "feat/${game_dev_issue}-toolkit-contributor-sandbox" \
  "$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}" origin/main

web_issue="$(gh issue list -R KirkDiggler/rpg-dnd5e-web --state open --search 'in:title "Add development-only toolkit contributor sandbox"' --json number,title | jq -r '.[] | select(.title == "Add development-only toolkit contributor sandbox") | .number')"
test "$(printf '%s\n' "$web_issue" | sed '/^$/d' | wc -l)" -eq 1
git -C ~/game-dev/rpg-dnd5e-web fetch origin
git -C ~/game-dev/rpg-dnd5e-web worktree add -b "feat/${web_issue}-toolkit-contributor-sandbox" \
  "$HOME/game-dev/rpg-dnd5e-web/.worktrees/${web_issue}-toolkit-contributor-sandbox" origin/dev
```

Do not create a branch for Unit D.

## Contract Inventory

### API local-override command contract

Unit A evolves `scripts/toolkit-local-override.sh` without weakening its existing encounter loop:

```text
scripts/toolkit-local-override.sh on --target encounter|rulebooks/dnd5e --src "$TOOLKIT_SOURCE"
scripts/toolkit-local-override.sh refresh --src "$TOOLKIT_SOURCE"
scripts/toolkit-local-override.sh status
scripts/toolkit-local-override.sh off
```

- `on` defaults to `encounter` when `--target` is omitted, preserving the existing documented command; Unit B always supplies `--target rulebooks/dnd5e`.
- The two allowlisted targets are exactly `github.com/KirkDiggler/rpg-toolkit/encounter` and `github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e`; their source subdirectories are exactly `encounter/` and `rulebooks/dnd5e/`.
- `on` and `refresh` require the selected source subdirectory’s first `go.mod` line to equal the module mapped to its target, sync only that subdirectory to `local-toolkit/$target/`, and permit one active `go.mod` replacement total.
- The helper owns a replacement only when all of these exact values agree before it mutates: the one allowlisted module path, `go.mod` replacement RHS `./local-toolkit/$target`, and an ignored `local-toolkit/.toolkit-local-override-state` record with `target=$target`, `module=$module`, and `replace=./local-toolkit/$target`. The record also holds the source absolute path, revision, and UTC sync timestamp for reporting; parse it as data, never by `source`-ing it.
- `on` permits no active replacement or the exact owned replacement for its selected target. It refuses an unknown replacement, more than one replacement, a mismatched RHS/state record, or switching from one valid target to the other without `off`, before `rsync`, `go mod edit`, or state writes.
- `refresh`, `status`, and `off` first require exactly one owned replacement with that exact RHS and matching state. They refuse a missing state, mismatched RHS/state, unknown replacement, or a second replacement before any mutation. `refresh` syncs only that selected target.
- `off` removes only that prevalidated selected `replace`, its `local-toolkit/$target/`, and its state record; it removes the `local-toolkit/` parent only if it is empty. It never removes a user-owned replacement or sibling tree.
- `Dockerfile.local-toolkit` remains the local build path because it copies `local-toolkit/` before `go mod download`; normal `Dockerfile`, normal/release pins, and CI remain published-module paths. `scripts/verify-release-pin.sh` continues rejecting `go.mod` replacements and any `local-toolkit/` residue for a commit gate.

### API sandbox-seeder contract

`cmd/sandboxseed` is a real gRPC client, not a storage utility:

```text
go run ./cmd/sandboxseed --address localhost:8080
go run ./cmd/sandboxseed --address localhost:8080 --health
```

- `--address` defaults to `localhost:8080`, the Envoy listener; it is never a Redis address.
- `--health` calls `grpc.health.v1.Health/Check` through Envoy and exits zero only for `SERVING`; it makes no character mutation. Unit B polls this command after `start` and `refresh`, retrying at most 30 times with a 2-second interval (60 seconds total) and failing with the final health error on timeout.
- Normal execution calls the existing `dnd5e.api.v1alpha1.CharacterService` with outgoing `authorization: Dev toolkit-sandbox-fighter` or `authorization: Dev toolkit-sandbox-barbarian` metadata on every RPC.
- For each identity, the deterministic reset is: `ListCharacters(page_size=100)` → fail if a continuation token is present → `DeleteCharacter` once for every returned character ID → `CreateDraft` → `UpdateName` → `UpdateRace` → `UpdateClass` → `UpdateBackground` → `UpdateAbilityScores` → `GetDraft` → `FinalizeDraft` → `GetCharacter` → `ListCharacters(page_size=100)`.
- The final list must have no continuation token and exactly one character with the fixed expected name. A draft or section failure stops immediately with the identity and RPC method in the error; it does not retry, choose a substitute, or continue to the next preset.
- The fighter’s `GetCharacter` response supplies the inventory ID `shield`. The command then calls the existing v1alpha1 `EquipItem` with that returned ID and `EQUIPMENT_SLOT_OFF_HAND`, verifies `EquipItemResponse.Character.EquipmentSlots.OffHand.ItemId == "shield"`, then performs the final authenticated list.

The fixed request values are deliberately literal rather than discovered:

| Identity                    | Name and fixed requests                                                                                                                                                                                                                                                                                                                                                                                                                                      | Required postcondition                                                                                                                                                     |
| --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `toolkit-sandbox-fighter`   | Name `Toolkit Sandbox Fighter`; `RACE_HUMAN` with `LANGUAGE_DWARVISH`; `CLASS_FIGHTER`; class skills `SKILL_ATHLETICS`, `SKILL_PERCEPTION`; `FIGHTING_STYLE_PROTECTION`; equipment `fighter-armor`/`fighter-armor-a`, `fighter-weapons-primary`/`fighter-weapon-a` with `WEAPON_LONGSWORD`, `fighter-weapons-secondary`/`fighter-ranged-a`, and `fighter-pack`/`fighter-pack-a`; `BACKGROUND_SOLDIER`; scores STR 15, DEX 13, CON 14, INT 10, WIS 12, CHA 8. | Exactly one level-1 Human Fighter named `Toolkit Sandbox Fighter`; Fighting Style Protection; returned `shield` is equipped in off hand. Baseline returned Strength is 16. |
| `toolkit-sandbox-barbarian` | Name `Toolkit Sandbox Barbarian`; `RACE_HUMAN` with `LANGUAGE_ORC`; `CLASS_BARBARIAN`; class skills `SKILL_ATHLETICS`, `SKILL_INTIMIDATION`; equipment `barbarian-weapons-primary`/`barbarian-weapon-a`, `barbarian-weapons-secondary`/`barbarian-secondary-a`, and `barbarian-pack`/`barbarian-pack-a`; `BACKGROUND_OUTLANDER`; scores STR 15, DEX 13, CON 14, INT 8, WIS 12, CHA 10.                                                                       | Exactly one level-1 Human Barbarian named `Toolkit Sandbox Barbarian`; Rage remains the toolkit class grant.                                                               |

### Game-dev facade contract

```text
./scripts/toolkit-contributor.sh bootstrap
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed
./scripts/toolkit-contributor.sh status
./scripts/toolkit-contributor.sh down
```

- `bootstrap` verifies Ubuntu WSL2, Docker Desktop’s WSL-connected daemon, Git, GitHub SSH read access, Go, Node/npm, `rsync`, and `jq`; it changes none of Windows, Docker Desktop, global tools, credentials, configuration, branches, or existing checkout files.
- `bootstrap` clones only absent valid roots `rpg-toolkit`, `rpg-api`, `rpg-dnd5e-web`, and `rpg-deployment`. Existing roots must be Git repositories with respectively exact `git@github.com:KirkDiggler/rpg-toolkit.git`, `git@github.com:KirkDiggler/rpg-api.git`, `git@github.com:KirkDiggler/rpg-dnd5e-web.git`, or `git@github.com:KirkDiggler/rpg-deployment.git` origins; valid roots are left untouched and invalid existing paths fail.
- `start` runs API override `on --target rulebooks/dnd5e --src "$ROOT/rpg-toolkit"`, builds `rpg-api:local` using `rpg-api/Dockerfile.local-toolkit`, starts the exact three compose files in the stated order, then polls `cmd/sandboxseed --health` through Envoy at most 30 times with a 2-second interval. It prints `npm run dev` plus `http://localhost:3001/?toolkitSandbox=1` only after a serving response.
- `refresh` runs API override `refresh --src "$ROOT/rpg-toolkit"`, builds before replacement, runs the same three-file compose command with `up -d --no-deps rpg-api` only after a successful build, then uses the same bounded Envoy health poll and prints source revision and sync timestamp only after it serves.
- `seed` delegates exactly to `go run ./cmd/sandboxseed --address localhost:8080`; it neither contains seeding RPC definitions nor accesses storage.
- `status` delegates API override status and health, and reports only the D&D 5e source revision/sync timestamp plus compose/API health.
- `down` runs the same compose set with `down`, then calls API override `off`; it touches only stack containers and the selected local D&D 5e replacement/tree.

### Web sandbox contract

The only sandbox URL is `http://localhost:3001/?toolkitSandbox=1` and it is true only when `import.meta.env.MODE === 'development'`.

```ts
export const TOOLKIT_SANDBOX_KEY = "toolkit-contributor-sandbox" as const;
export const TOOLKIT_SANDBOX_FIGHTER = "toolkit-sandbox-fighter" as const;
export const TOOLKIT_SANDBOX_BARBARIAN = "toolkit-sandbox-barbarian" as const;
export type ToolkitSandboxPlayer =
  | typeof TOOLKIT_SANDBOX_FIGHTER
  | typeof TOOLKIT_SANDBOX_BARBARIAN;

export interface ToolkitSandboxClients {
  readonly fighter: SandboxUnaryClients;
  readonly barbarian: SandboxUnaryClients;
}

export interface SandboxUnaryClients {
  readonly authoring: Pick<typeof authoringClient, "putDungeon">;
  readonly character: Pick<typeof characterClient, "listCharacters">;
  readonly lobby: Pick<
    typeof lobbyClient,
    "createLobby" | "joinLobby" | "setReady" | "startEncounter"
  >;
}
```

- The feature-local client helper returns only those fixed unary clients. It constructs the two literal identity sets once with a closure interceptor that always writes its own `Dev <identity>` header and never imports or reads `setAuth`, `getPlayerId`, Discord state, URL `playerId`, or `VITE_DEV_PLAYER_ID`.
- It does not expose a client creator for arbitrary identities. Focused tests interleave fighter and barbarian unary interceptor calls and prove the captured headers remain `Dev toolkit-sandbox-fighter` and `Dev toolkit-sandbox-barbarian` respectively.
- The checked-in YAML is an in-memory, editable `version: 1` canvas with `key: toolkit-contributor-sandbox`, `theme: crypt`, `canvas: { width: 12, height: 6 }`, `start: [1, 3]`, and two `dnd5e:monsters:skeleton` placements at `[7, 2]` and `[9, 4]`. A page reload starts from this literal template; it never reads or writes a sandbox draft in local storage.
- The existing Dungeon Builder is reused through narrowly injected authoring client, initial YAML, and draft-persistence/file-import-export controls. Normal AuthorView keeps its existing defaults. The supplied client is threaded through the builder mount liveness probe, capability suite, live compile/creation preview, and non-validate save: a sandbox mount makes zero calls through the mutable global `authoringClient`. The sandbox performs `PutDungeon(validate_only: true)` for preview and `PutDungeon(validate_only: false)` under the stable key for save; after an RPC failure it renders the returned error and stops.
- `DungeonBuilderConcept` exposes optional `onSaveSucceeded(key: string): void`, invoked once for each successful non-validate save and never for validation rejection or transport failure. Sandbox mode suppresses the builder’s ordinary generic lobby link, handles this callback, then calls the two identity-bound `ListCharacters` clients and offers only `Fighter`, `Barbarian`, `Fighter then Barbarian`, and `Barbarian then Fighter`.
- For a selected fixed order, first identity calls `CreateLobby(campaign_id="toolkit-contributor-sandbox", character_id=the Character.Id returned for that identity)`; second identity, if selected, calls `JoinLobby(join_ref=the CreateLobby response JoinRef, character_id=the second identity’s returned Character.Id)`; every selected identity calls `SetReady(lobby_id, true)` with its own bound client; only first identity calls `StartEncounter(lobby_id, dungeon_key="toolkit-contributor-sandbox")`.
- On success, show normal links only for selected identities: `/?playerId=toolkit-sandbox-fighter` and/or `/?playerId=toolkit-sandbox-barbarian`. Do not render a harness as the sandbox result.

## File Structure

### Unit A — `rpg-api`

| File                                                 | Change | Responsibility                                                                                                                 |
| ---------------------------------------------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------ |
| `scripts/toolkit-local-override.sh:1-229`            | Modify | Retain encounter default and add the allowlisted D&D 5e target, exact cleanup, refresh, and auditable local state.             |
| `scripts/toolkit-local-override.test.sh`             | Create | Hermetic shell contract test for target module validation, one-replace rejection, refresh, cleanup, and release pin behavior.  |
| `docs/how-to/local-toolkit-override.md:1-128`        | Modify | Document both exact targets, D&D 5e command examples, refresh, and release cleanup without changing production build guidance. |
| `internal/sandboxseed/sandboxseed.go`                | Create | Narrow production-RPC sequence and fixed presets; no repository or data construction dependency.                               |
| `cmd/sandboxseed/main.go`                            | Create | Flag parsing, Envoy dial, health command, and delegation to `internal/sandboxseed`.                                            |
| `cmd/sandboxseed/main_test.go`                       | Create | CLI address/default/health-path tests with a local gRPC test listener.                                                         |
| `internal/integration/character/sandboxseed_test.go` | Create | Real Redis + bufconn integration proof for two resets, fixed postconditions, and lobby ownership negatives.                    |

### Unit B — `game-dev`

| File                                    | Change | Responsibility                                                                                                                                   |
| --------------------------------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| `scripts/toolkit-contributor.sh`        | Create | The six-command WSL2 facade; delegates override and seed implementation to Unit A.                                                               |
| `tests/toolkit-contributor-contract.sh` | Create | Fixture-based shell tests for prerequisites, cloning boundaries, command delegation, compose order, dirty preservation, and owned down behavior. |
| `docs/toolkit-contributor-sandbox.md`   | Create | Contributor runbook with exact WSL2 prerequisite checks, normal loop, marker proof, and shutdown.                                                |
| `README.md:1-22`                        | Modify | Link the narrow contributor loop without rewriting the existing seven-repository bootstrap guidance.                                             |

### Unit C — `rpg-dnd5e-web`

| File                                                                 | Change | Responsibility                                                                                                                                            |
| -------------------------------------------------------------------- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `src/toolkit-contributor-sandbox/constants.ts`                       | Create | Fixed key, identities, one literal template, and the four permitted seat arrangements.                                                                    |
| `src/toolkit-contributor-sandbox/clients.ts`                         | Create | Feature-private fixed-identity Connect unary clients and immutable Dev interceptors.                                                                      |
| `src/toolkit-contributor-sandbox/clients.test.ts`                    | Create | Interleaved unary-header isolation tests.                                                                                                                 |
| `src/toolkit-contributor-sandbox/route.ts`                           | Create | Development-plus-query route predicate.                                                                                                                   |
| `src/toolkit-contributor-sandbox/route.test.ts`                      | Create | Development positive and production negative route tests.                                                                                                 |
| `src/toolkit-contributor-sandbox/ToolkitContributorSandbox.tsx`      | Create | Template save, fixed character display, fixed lobby sequence, error stop, and normal GameView links.                                                      |
| `src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx` | Create | PutDungeon, fixed seat-order, error-stop, and selected-link tests.                                                                                        |
| `src/App.tsx:1-347`                                                  | Modify | Route to the sandbox before normal app state only when the route predicate is true.                                                                       |
| `src/author/usePutDungeonPreview.ts:75-430`                          | Modify | Thread an optional authoring unary client through liveness, live compile, and creation preview while preserving the global default.                       |
| `src/author/usePutDungeonPreview.test.ts`                            | Modify | Prove the supplied client receives probe/preview calls and a sandbox path makes zero global-client calls.                                                 |
| `src/author/capabilityProbe.ts:1-580`                                | Modify | Accept the supplied authoring client for every capability probe, retaining the global default only for normal mounts.                                     |
| `src/author/capabilityProbe.test.ts`                                 | Modify | Prove supplied-client capability probes never call the global client.                                                                                     |
| `src/author/useSaveDungeon.ts:18-82`                                 | Modify | Accept a supplied authoring unary client and one success callback while retaining the default global client.                                              |
| `src/author/useSaveDungeon.test.ts`                                  | Modify | Prove the supplied client receives the non-validate save request and the callback fires only on success.                                                  |
| `src/author/DungeonBuilderConcept.tsx:101-623`                       | Modify | Accept sandbox-only initial YAML, no-local-draft mode, injected authoring client, save-success callback, and disabled controls while preserving defaults. |
| `src/author/DungeonBuilderConcept.test.tsx`                          | Modify | Prove template isolation, callback behavior, and no global authoring call for an injected sandbox mount.                                                  |
| `src/author/creation/CreationConcept.tsx:47-503`                     | Modify | Respect supplied canvas/file-control and generic-save-link visibility while preserving normal builder controls.                                           |
| `src/author/creation/ProposedYamlPane.tsx`                           | Modify | Hide file import/export controls and pass generic-save-link visibility to the shared result panel.                                                        |
| `src/author/YamlPane.tsx`                                            | Modify | Allow `SaveResultPanel` to suppress only its ordinary lobby link for the sandbox.                                                                         |
| `src/author/YamlPane.test.tsx`                                       | Modify | Prove suppressing the generic result link leaves normal saved/error rendering intact.                                                                     |

### Unit D — `rpg-project`

No repository file is changed for Unit D. The verification issue records its command transcript, screenshots, four normal-GameView links, negative evidence, and exact merged commits. The still-open idea PR remains the cross-repository tracking surface and is not changed until verification evidence is ready.

## Tasks

### Task 1: Open the post-approval delivery records and freeze the release sequence

**Files:**

- Modify: no repository file
- Create: the four GitHub issues in the delivery table, only after plan approval
- Test: Project 19 field read-back and issue/branch/PR relationship checks

**Interfaces:**

- Consumes: approved `ideas/toolkit-contributor-sandbox/design.md` and this approved plan.
- Produces: one exact owning issue for Units A–C, one independent verification issue for Unit D, and the recorded issue numbers used in branch/PR names.

- [ ] **Step 1: Verify the plan gate before any issue or branch command**

Run:

```bash
gh pr view 209 --repo KirkDiggler/rpg-project --json state,comments \
  --jq '.state'
grep -Fqx '**Status:** approved — 2026-08-10' \
  ideas/toolkit-contributor-sandbox/design.md
grep -Fqx '## Delivery Control, Repository Ownership, and Issue Map' \
  ideas/toolkit-contributor-sandbox/plan.md
```

Expected: all commands exit `0`; PR #209 is `OPEN`, the design status is approved, and the plan has received the separately required approval before proceeding.

- [ ] **Step 2: Create exactly the four named issues and assign Project 19 fields**

Create the three implementation issues with the exact titles and acceptance boundaries from the delivery table, then create the evidence-only verification issue. Add each issue URL to Project 19 and set its literal `Team`, `Feature`, `Kind`, and `Status` values from that table. State in every implementation issue that its PR has one closing `Closes #` reference to that issue only.

Run:

```bash
gh project item-list 19 --owner KirkDiggler --limit 500 --format json \
  | jq -r '.items[] | select(.content.title == "Add D&D 5e local override and API sandbox seeder" or .content.title == "Add WSL2 toolkit contributor sandbox facade" or .content.title == "Add development-only toolkit contributor sandbox" or .content.title == "Verify the toolkit contributor sandbox on clean WSL2") | [.content.title, .fieldValues[]?.name] | @json'
```

Expected: exit `0`; each exact title appears once and each row contains its required four field values.

- [ ] **Step 3: Create only the three issue-owned implementation worktrees**

Use the repository-specific commands in the delivery table after `git fetch origin`; do not use a local base branch and do not create any Unit D branch.

Run:

```bash
git -C ~/game-dev/rpg-api worktree list
git -C ~/game-dev worktree list
git -C ~/game-dev/rpg-dnd5e-web worktree list
```

Expected: exit `0`; every new implementation worktree branch names its actual owning issue and no verification branch exists.

- [ ] **Step 4: Record provider seams before implementation**

```bash
git -C ~/game-dev/rpg-api rev-parse origin/dev
git -C ~/game-dev rev-parse origin/main
git -C ~/game-dev/rpg-dnd5e-web rev-parse origin/dev
git -C ~/game-dev/rpg-deployment rev-parse origin/main
```

Expected: exit `0`; the issue notes contain these four baseline commit IDs. Unit C may code against the contract but cannot merge until A and B have landed.

- [ ] **Step 5: Do not make a repository commit for workflow setup**

The issues, Project fields, and worktree provenance are GitHub/process records. Unit A’s first commit is the first implementation commit.

### Task 2: Add the safe D&D 5e one-module override to `rpg-api`

**Files:**

- Modify: `scripts/toolkit-local-override.sh:1-229`
- Create: `scripts/toolkit-local-override.test.sh`
- Modify: `docs/how-to/local-toolkit-override.md:1-128`
- Test: `scripts/toolkit-local-override.test.sh`

**Interfaces:**

- Consumes: a local toolkit checkout containing `encounter/go.mod` or `rulebooks/dnd5e/go.mod` and a release-pinned API checkout.
- Produces: `on --target rulebooks/dnd5e --src "$TOOLKIT_SOURCE"`, `refresh --src "$TOOLKIT_SOURCE"`, `status`, and `off` with a single active replacement invariant; Unit B consumes these exact commands.
- Preserves: `on` with no `--target` selects `encounter`; `scripts/verify-release-pin.sh` remains the pre-commit refusal for any local override residue.

- [ ] **Step 1: Write the failing hermetic override contract test**

Create `scripts/toolkit-local-override.test.sh` with a temporary Git API module and two temporary toolkit module roots. The test must assert all of these observable commands:

```bash
./scripts/toolkit-local-override.sh on --target rulebooks/dnd5e --src "$toolkit"
grep -Fq 'github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e => ./local-toolkit/rulebooks/dnd5e' go.mod
./scripts/toolkit-local-override.sh refresh --src "$toolkit"
./scripts/toolkit-local-override.sh status | grep -Fq 'rulebooks/dnd5e'
./scripts/toolkit-local-override.sh off
test ! -e local-toolkit/rulebooks/dnd5e
```

Also assert that a source declaring any other module fails before `go.mod` or `local-toolkit/` changes; a second or unknown replace fails; and `off` preserves an unrelated non-empty `local-toolkit/` sibling rather than recursively deleting it. For each of `refresh`, `status`, and `off`, seed a valid allowlisted module with either an RHS other than `./local-toolkit/<target>` or a missing/mismatched ownership record, snapshot `go.mod` and `local-toolkit/`, and assert failure leaves both snapshots byte-for-byte unchanged. Finally, turn on `encounter`, attempt `on --target rulebooks/dnd5e`, and assert it fails before any snapshot changes; repeat the mirror case from D&D 5e to encounter.

- [ ] **Step 2: Run the contract test and verify the current script fails for the requested target**

Run:

```bash
api_issue="$(gh issue list -R KirkDiggler/rpg-api --state open --search 'in:title "Add D&D 5e local override and API sandbox seeder"' --json number,title | jq -r '.[] | select(.title == "Add D&D 5e local override and API sandbox seeder") | .number')"
cd "$HOME/game-dev/rpg-api/.worktrees/${api_issue}-dnd5e-sandbox"
bash scripts/toolkit-local-override.test.sh
```

Expected: non-zero before implementation because the current script hard-codes only `github.com/KirkDiggler/rpg-toolkit/encounter` and has no `--target rulebooks/dnd5e` behavior.

- [ ] **Step 3: Implement the allowlisted target table and exact lifecycle**

Replace the one hard-coded `MODULE`/`LOCAL_DIR` pair with literal target metadata, retaining the current `encounter` default:

```bash
TARGET_ENCOUNTER='encounter'
TARGET_DND5E='rulebooks/dnd5e'

module_for_target() {
  case "$1" in
    "$TARGET_ENCOUNTER") printf '%s\n' 'github.com/KirkDiggler/rpg-toolkit/encounter' ;;
    "$TARGET_DND5E") printf '%s\n' 'github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e' ;;
    *) printf 'error: unsupported override target: %s\n' "$1" >&2; return 1 ;;
  esac
}
```

Parse `on [--target encounter|rulebooks/dnd5e] [--src "$TOOLKIT_SOURCE"]`, `refresh [--src "$TOOLKIT_SOURCE"]`, `status`, and `off`. Keep an ignored state file at `local-toolkit/.toolkit-local-override-state` with literal, newline-delimited `target`, `module`, `replace`, `source`, `revision`, and `synced_at` keys; reject duplicate, missing, or mismatched ownership keys without `source`-ing the file. Derive the selected target from the one active allowlisted replacement for refresh/status/off and require its JSON `New.Path` to equal exactly `./local-toolkit/$target` before reading state or mutating. `on` accepts only no active replace or this exact owned target; it rejects a different active target before `rsync`, so it cannot transiently add a second replacement. Validate module declarations before `rsync`; write state only after a successful sync and exact replacement check; and make `off` prevalidate ownership before dropping only the selected replacement, synced directory, and state file. Keep host `go build ./...` checks after sync and after off. Update the how-to with literal D&D 5e commands and retain the encounter examples.

- [ ] **Step 4: Run focused override and release-pin checks**

Run:

```bash
bash scripts/toolkit-local-override.test.sh
./scripts/verify-release-pin.sh
git diff --check
```

Expected: all exit `0`. The override test covers active local behavior in its fixture; `verify-release-pin.sh` passes in the release-pinned worktree with no active local tree.

- [ ] **Step 5: Commit the override delivery slice**

```bash
git add scripts/toolkit-local-override.sh scripts/toolkit-local-override.test.sh \
  docs/how-to/local-toolkit-override.md
git commit -m "feat: support local dnd5e toolkit override"
```

Expected: one focused API commit; do not stage `go.mod`, `go.sum`, or `local-toolkit/`.

### Task 3: Add the production-RPC sandbox seeder and API integration proof

**Files:**

- Create: `internal/sandboxseed/sandboxseed.go`
- Create: `cmd/sandboxseed/main.go`
- Create: `cmd/sandboxseed/main_test.go`
- Create: `internal/integration/character/sandboxseed_test.go`
- Test: `cmd/sandboxseed/main_test.go`, `internal/integration/character/sandboxseed_test.go`, existing auth/lobby tests

**Interfaces:**

- Consumes: the existing generated v1alpha1 `CharacterServiceClient`, existing `CreateLobby`/`JoinLobby` ownership enforcement, and Envoy at `localhost:8080`.
- Produces: `sandboxseed.Seed(ctx, client)` for the two fixed presets and the `cmd/sandboxseed` `--address`/`--health` CLI used by Unit B.
- Does not produce: a CharacterService implementation, a repository API, a storage path, new protocol schema, a generic character builder, or a generic option resolver.

- [ ] **Step 1: Write failing real-service integration coverage first**

Create a second suite in package `character_integration`, reusing its package `TestMain` shared Redis fixture. Register it with a top-level test that Go can select, then call the missing package twice and make exact assertions:

```go
func TestSandboxSeedSuite(t *testing.T) {
    if testing.Short() {
        t.Skip("skipping integration test in short mode")
    }
    suite.Run(t, new(SandboxSeedSuite))
}

func (s *SandboxSeedSuite) TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs() {
    require.NoError(s.T(), sandboxseed.Seed(s.ctx, s.server.CharacterClient))
    require.NoError(s.T(), sandboxseed.Seed(s.ctx, s.server.CharacterClient))

    fighter := s.listExactlyOne("toolkit-sandbox-fighter")
    require.Equal(s.T(), "Toolkit Sandbox Fighter", fighter.GetName())
    require.Contains(s.T(), fighter.GetFightingStyles(), dnd5ev1alpha1.FightingStyle_FIGHTING_STYLE_PROTECTION)
    require.Equal(s.T(), "shield", fighter.GetEquipmentSlots().GetOffHand().GetItemId())
    require.EqualValues(s.T(), 16, fighter.GetAbilityScores().GetStrength())

    barbarian := s.listExactlyOne("toolkit-sandbox-barbarian")
    require.Equal(s.T(), "Toolkit Sandbox Barbarian", barbarian.GetName())
}
```

In the same suite, create a lobby using the fighter’s character under the barbarian Dev metadata and assert `codes.PermissionDenied`; create a fighter-host lobby and attempt `JoinLobby` as barbarian using the fighter character ID, again asserting `codes.PermissionDenied`. This proves each lobby verb enforces ownership without claiming unrelated character-ID RPCs do.

- [ ] **Step 2: Run the focused integration test and confirm it fails before the package exists**

Run:

```bash
go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1
```

Expected: non-zero compile failure because `github.com/KirkDiggler/rpg-api/internal/sandboxseed` does not yet exist.

- [ ] **Step 3: Implement the narrow fixed RPC sequence**

Define only the RPC methods the seeder uses and execute each identity with fresh outgoing Dev metadata:

```go
type CharacterRPC interface {
    ListCharacters(context.Context, *dnd5ev1alpha1.ListCharactersRequest, ...grpc.CallOption) (*dnd5ev1alpha1.ListCharactersResponse, error)
    DeleteCharacter(context.Context, *dnd5ev1alpha1.DeleteCharacterRequest, ...grpc.CallOption) (*dnd5ev1alpha1.DeleteCharacterResponse, error)
    CreateDraft(context.Context, *dnd5ev1alpha1.CreateDraftRequest, ...grpc.CallOption) (*dnd5ev1alpha1.CreateDraftResponse, error)
    UpdateName(context.Context, *dnd5ev1alpha1.UpdateNameRequest, ...grpc.CallOption) (*dnd5ev1alpha1.UpdateNameResponse, error)
    UpdateRace(context.Context, *dnd5ev1alpha1.UpdateRaceRequest, ...grpc.CallOption) (*dnd5ev1alpha1.UpdateRaceResponse, error)
    UpdateClass(context.Context, *dnd5ev1alpha1.UpdateClassRequest, ...grpc.CallOption) (*dnd5ev1alpha1.UpdateClassResponse, error)
    UpdateBackground(context.Context, *dnd5ev1alpha1.UpdateBackgroundRequest, ...grpc.CallOption) (*dnd5ev1alpha1.UpdateBackgroundResponse, error)
    UpdateAbilityScores(context.Context, *dnd5ev1alpha1.UpdateAbilityScoresRequest, ...grpc.CallOption) (*dnd5ev1alpha1.UpdateAbilityScoresResponse, error)
    GetDraft(context.Context, *dnd5ev1alpha1.GetDraftRequest, ...grpc.CallOption) (*dnd5ev1alpha1.GetDraftResponse, error)
    FinalizeDraft(context.Context, *dnd5ev1alpha1.FinalizeDraftRequest, ...grpc.CallOption) (*dnd5ev1alpha1.FinalizeDraftResponse, error)
    GetCharacter(context.Context, *dnd5ev1alpha1.GetCharacterRequest, ...grpc.CallOption) (*dnd5ev1alpha1.GetCharacterResponse, error)
    EquipItem(context.Context, *dnd5ev1alpha1.EquipItemRequest, ...grpc.CallOption) (*dnd5ev1alpha1.EquipItemResponse, error)
}

func Seed(ctx context.Context, client CharacterRPC) error
```

Encode the two complete literal preset rows from the contract inventory. Set `ListCharactersRequest.PageSize` to `100`; fail on a non-empty `NextPageToken`; delete only `response.Characters[i].Id`; never use `player_id` request fields as authority. Use `metadata.AppendToOutgoingContext(ctx, "authorization", "Dev "+identity)`. For the fighter, scan `GetCharacterResponse.Character.Inventory` for the returned `ItemId == "shield"`, send `EquipItemRequest{CharacterId: characterID, ItemId: shieldID, Slot: EQUIPMENT_SLOT_OFF_HAND}`, and assert the returned off-hand ID. Print identity, resulting character ID, Strength, and fighter off-hand value so Unit D can use Strength as a real API projection.

In `cmd/sandboxseed/main.go`, use `grpc.NewClient(address, grpc.WithTransportCredentials(insecure.NewCredentials()))`; normal mode constructs the generated v1 client and calls `sandboxseed.Seed`; `--health` uses `grpc_health_v1.NewHealthClient(conn).Check` and requires `HealthCheckResponse_SERVING`.

- [ ] **Step 4: Run focused tests, cross-owner regression checks, and the complete API gate**

Run:

```bash
go test ./cmd/sandboxseed -count=1
go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1
go test ./internal/auth -run TestUnaryAuthInterceptor_DevScheme_NotAllowed -count=1
go test ./internal/orchestrators/lobby -run '^TestLobbySuite$' -count=1
make pre-commit
```

Expected: every command exits `0`. The integration run is serialized with other Docker-backed API integration runs; it proves the real section sequence, idempotent double seed, post-finalize equip, and both ownership-negative lobby verbs.

- [ ] **Step 5: Commit and prepare the one Unit A PR**

```bash
git add internal/sandboxseed/sandboxseed.go cmd/sandboxseed/main.go \
  cmd/sandboxseed/main_test.go internal/integration/character/sandboxseed_test.go
git commit -m "feat: add API-backed toolkit sandbox seed"
git status --short
git diff origin/dev...HEAD --check
```

Expected: all validation commands exit `0`; the branch contains the two focused Unit A commits only. Open one PR to `dev` with exactly one closing `Closes #` line naming the Unit A issue.

### Task 4: Build the WSL2 contributor facade after Unit A lands

**Files:**

- Create: `scripts/toolkit-contributor.sh`
- Create: `tests/toolkit-contributor-contract.sh`
- Create: `docs/toolkit-contributor-sandbox.md`
- Modify: `README.md:1-22`
- Test: `tests/toolkit-contributor-contract.sh`, `tests/bootstrap-contract.sh`

**Interfaces:**

- Consumes: Unit A’s exact override commands and `cmd/sandboxseed --address`/`--health` contracts, plus existing deployment compose files.
- Produces: six explicit contributor commands with no branch mutation or copied API logic.
- Preserves: `bootstrap.sh` and `scripts/dev-env.sh` behavior; neither is changed by this unit.

- [ ] **Step 1: Write the failing shell contract test with fixture executables**

Create a temporary workspace with fake `git`, `docker`, `go`, `node`, `npm`, `rsync`, `jq`, and `ssh` executables that append their arguments to a log. Make the test assert the contributor script:

```bash
./scripts/toolkit-contributor.sh bootstrap
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed
./scripts/toolkit-contributor.sh status
./scripts/toolkit-contributor.sh down
```

The assertions must require exactly these observable command fragments, in order where relevant:

```text
rpg-api/scripts/toolkit-local-override.sh on --target rulebooks/dnd5e --src "$ROOT/rpg-toolkit"
docker build -f "$ROOT/rpg-api/Dockerfile.local-toolkit" -t rpg-api:local "$ROOT/rpg-api"
docker compose -f docker-compose.local-dev.yml -f docker-compose.api.yml -f docker-compose.local-api-src.yml up -d
go run ./cmd/sandboxseed --address localhost:8080 --health
go run ./cmd/sandboxseed --address localhost:8080
docker compose -f docker-compose.local-dev.yml -f docker-compose.api.yml -f docker-compose.local-api-src.yml down
rpg-api/scripts/toolkit-local-override.sh off
```

It must also prove bootstrap clones only the four named repositories when absent, leaves valid dirty checkouts untouched, rejects a wrong origin without cleaning/resetting/stashing/switching, and down never removes a checkout. Configure the fake health command to fail twice and serve on its third call; assert both `start` and `refresh` issue exactly three `go run ./cmd/sandboxseed --address localhost:8080 --health` calls and then succeed. Configure it to fail all 30 calls and assert the command exits non-zero without printing the successful-start message.

- [ ] **Step 2: Run the contract test and verify the facade is absent**

Run:

```bash
game_dev_issue="$(gh issue list -R KirkDiggler/game-dev --state open --search 'in:title "Add WSL2 toolkit contributor sandbox facade"' --json number,title | jq -r '.[] | select(.title == "Add WSL2 toolkit contributor sandbox facade") | .number')"
cd "$HOME/game-dev/.pi-worktrees/game-dev-${game_dev_issue}"
bash tests/toolkit-contributor-contract.sh
```

Expected: non-zero because `scripts/toolkit-contributor.sh` does not yet exist.

- [ ] **Step 3: Implement the six explicit commands without changing global environment state**

Use a single root resolver and explicit repository constants:

```bash
REQUIRED_REPOS=(rpg-toolkit rpg-api rpg-dnd5e-web rpg-deployment)
COMPOSE_FILES=(
  docker-compose.local-dev.yml
  docker-compose.api.yml
  docker-compose.local-api-src.yml
)
```

`bootstrap` must check WSL2 through `/proc/sys/kernel/osrelease`, Docker daemon reachability through `docker info`, tool executables through `command -v`, and GitHub access through `git ls-remote git@github.com:KirkDiggler/rpg-api.git HEAD`. For every existing required root, use `git -C "$root" remote get-url origin` and accept only its exact SSH origin; never call checkout, reset, clean, stash, pull, fetch, or installation commands. For every absent root, clone its exact SSH URL.

Build the compose argument array only as the three existing filenames above and invoke it from `rpg-deployment`. Define one `wait_for_envoy` helper used by both `start` and `refresh`: it runs `(cd "$ROOT/rpg-api" && go run ./cmd/sandboxseed --address localhost:8080 --health)` at most 30 times, sleeps 2 seconds between failed attempts, returns immediately on `SERVING`, and returns non-zero after the 30th failure without printing a success URL. `start` uses full `up -d`; `refresh` uses `up -d --no-deps rpg-api` only after override refresh and image build succeed, then both call that helper. `status` calls API override status and one health check without listing arbitrary containers. `down` calls compose down, then API override off, and exits on a refusal rather than deleting another replacement.

Write the runbook with the literal WSL2 prerequisites, first checkout, daily loop, refresh rule, normal web command/URL, and the marker proof below. Link it from the README in one concise paragraph.

- [ ] **Step 4: Run shell syntax, focused contracts, existing bootstrap contract, and diff hygiene**

Run:

```bash
bash -n scripts/toolkit-contributor.sh
bash tests/toolkit-contributor-contract.sh
bash tests/bootstrap-contract.sh
git diff --check
```

Expected: every command exits `0`. The test log contains the required three-file compose order and no branch-switching or global-install command.

- [ ] **Step 5: Commit and open the one Unit B PR after A is merged**

```bash
git add scripts/toolkit-contributor.sh tests/toolkit-contributor-contract.sh \
  docs/toolkit-contributor-sandbox.md README.md
git commit -m "feat: add toolkit contributor sandbox facade"
git diff origin/main...HEAD --check
```

Expected: exit `0`. Open one PR to `main` with exactly one closing `Closes #` line naming the Unit B issue.

### Task 5: Add immutable development-only sandbox clients and route gate in the web repository

**Files:**

- Create: `src/toolkit-contributor-sandbox/constants.ts`
- Create: `src/toolkit-contributor-sandbox/clients.ts`
- Create: `src/toolkit-contributor-sandbox/clients.test.ts`
- Create: `src/toolkit-contributor-sandbox/route.ts`
- Create: `src/toolkit-contributor-sandbox/route.test.ts`
- Modify: `src/App.tsx:1-347`
- Test: feature client and route tests

**Interfaces:**

- Consumes: existing Connect service descriptors, `createGrpcWebTransport`, and normal App route behavior.
- Produces: a fixed two-identity `ToolkitSandboxClients` object and `isToolkitContributorSandboxRoute(mode, search)` predicate.
- Preserves: `src/api/client.ts` global auth clients, normal `?playerId` routing, Discord auth, and production behavior.

- [ ] **Step 1: Write failing header-isolation and production-gate tests**

Add `clients.test.ts` with two fake unary requests passed through the fighter and barbarian feature interceptors in interleaved promise order:

```ts
await Promise.all([
  fighterInterceptor(fighterNext)(fighterRequest),
  barbarianInterceptor(barbarianNext)(barbarianRequest),
  fighterInterceptor(fighterNext)(secondFighterRequest),
]);

expect(capturedAuthorization).toEqual([
  "Dev toolkit-sandbox-fighter",
  "Dev toolkit-sandbox-barbarian",
  "Dev toolkit-sandbox-fighter",
]);
```

Add `route.test.ts` assertions:

```ts
expect(
  isToolkitContributorSandboxRoute("development", "?toolkitSandbox=1"),
).toBe(true);
expect(
  isToolkitContributorSandboxRoute("production", "?toolkitSandbox=1"),
).toBe(false);
expect(
  isToolkitContributorSandboxRoute("development", "?toolkitSandbox=0"),
).toBe(false);
```

- [ ] **Step 2: Run the feature tests and verify they fail before the feature module exists**

Run:

```bash
web_issue="$(gh issue list -R KirkDiggler/rpg-dnd5e-web --state open --search 'in:title "Add development-only toolkit contributor sandbox"' --json number,title | jq -r '.[] | select(.title == "Add development-only toolkit contributor sandbox") | .number')"
cd "$HOME/game-dev/rpg-dnd5e-web/.worktrees/${web_issue}-toolkit-contributor-sandbox"
npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts
```

Expected: non-zero module-resolution failure because the sandbox feature files do not yet exist.

- [ ] **Step 3: Implement the literal constants, fixed clients, and route predicate**

Put the exact stable key, two labels, four fixed arrangements, and YAML template in `constants.ts`:

```ts
export const TOOLKIT_SANDBOX_YAML = `version: 1
key: toolkit-contributor-sandbox
name: "Toolkit Contributor Sandbox"
theme: crypt
height: 1
canvas: { width: 12, height: 6 }
rooms: []
connectors: []
walls: []
wallLines: []
holes: []
start: [1, 3]
end: null
place:
  - { ref: "dnd5e:monsters:skeleton", at: [7, 2] }
  - { ref: "dnd5e:monsters:skeleton", at: [9, 4] }
`;
```

In `clients.ts`, create the two fixed client sets once. Each set uses a closure interceptor with the literal `ToolkitSandboxPlayer` argument and a transport pointed at the existing API host convention; the exported feature value contains only authoring, character, and lobby unary clients. It must not import `./api/auth` or `./api/client`. In `route.ts`, require the literal development mode and `toolkitSandbox=1` query value. In `App.tsx`, short-circuit to the sandbox component only when this predicate is true; the false branch remains the existing App tree.

- [ ] **Step 4: Run focused tests and the web static gate**

Run:

```bash
npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts
npm run typecheck
npm run build
```

Expected: every command exits `0`; the production Vite build succeeds and the route predicate’s production case is false.

- [ ] **Step 5: Commit the isolated web client/route slice**

```bash
git add src/toolkit-contributor-sandbox/constants.ts \
  src/toolkit-contributor-sandbox/clients.ts \
  src/toolkit-contributor-sandbox/clients.test.ts \
  src/toolkit-contributor-sandbox/route.ts \
  src/toolkit-contributor-sandbox/route.test.ts src/App.tsx
git commit -m "feat: add isolated toolkit sandbox clients"
```

Expected: one focused commit on the Unit C branch; do not open or merge the PR before Units A and B are landed.

### Task 6: Make Dungeon Builder accept the sandbox’s injected one-template contract

**Files:**

- Modify: `src/author/usePutDungeonPreview.ts:75-430`
- Modify: `src/author/usePutDungeonPreview.test.ts`
- Modify: `src/author/capabilityProbe.ts:1-580`
- Modify: `src/author/capabilityProbe.test.ts`
- Modify: `src/author/useSaveDungeon.ts:18-82`
- Modify: `src/author/useSaveDungeon.test.ts`
- Modify: `src/author/DungeonBuilderConcept.tsx:101-623`
- Modify: `src/author/DungeonBuilderConcept.test.tsx`
- Modify: `src/author/creation/CreationConcept.tsx:47-503`
- Modify: `src/author/creation/ProposedYamlPane.tsx`
- Modify: `src/author/YamlPane.tsx`
- Modify: `src/author/YamlPane.test.tsx`
- Test: the listed existing authoring tests plus injected-client, callback, and zero-global-call assertions

**Interfaces:**

- Consumes: `SandboxUnaryClients.authoring` and `TOOLKIT_SANDBOX_YAML` from Unit C.
- Produces: optional injected authoring client, optional initial YAML, optional `onSaveSucceeded(key)` callback, and sandbox control flags; all optional props preserve the existing AuthorView behavior.
- Produces for the sandbox: the supplied client handles liveness, capability probes, live/creation preview calls with `validateOnly: true`, and save calls with `validateOnly: false`; no local-storage draft restore/autosave; no new-canvas or YAML import/export control; and no generic builder lobby link.

- [ ] **Step 1: Write failing injected-client and template-isolation tests**

Extend `useSaveDungeon.test.ts` with a supplied client whose `putDungeon` spy must receive:

```ts
expect(putDungeon).toHaveBeenCalledWith(
  expect.objectContaining({
    key: "toolkit-contributor-sandbox",
    validateOnly: false,
  }),
);
```

Extend `usePutDungeonPreview.test.ts` with a supplied client whose mount liveness call, capability suite, and preview call all use that client (the preview carries `validateOnly: true`), while the mocked global `authoringClient.putDungeon` has zero calls. Extend `capabilityProbe.test.ts` and `useCreationFloorPlanPreview` coverage with supplied-client calls and the same zero-global assertion. Extend `useSaveDungeon.test.ts` to prove `onSaveSucceeded` receives the request key exactly once after `success: true` and never for `success: false` or a throw. Extend `DungeonBuilderConcept.test.tsx` to render with literal injected template and persistence disabled, edit the text, unmount, rerender, and assert the rerendered YAML is the original template rather than the edited local-storage value; also assert its injected mount makes zero global authoring calls, fires its callback only after a successful non-validate save, and suppresses the ordinary `http://localhost:3001/` save-result link. Extend `YamlPane.test.tsx` to prove hiding that link does not hide saved, validation-error, or transport-error feedback.

- [ ] **Step 2: Run the focused tests and observe the missing injection interfaces**

Run:

```bash
npm run test:run -- src/author/useSaveDungeon.test.ts src/author/usePutDungeonPreview.test.ts src/author/capabilityProbe.test.ts src/author/DungeonBuilderConcept.test.tsx src/author/YamlPane.test.tsx
```

Expected: non-zero TypeScript or assertion failure because the hooks and builder currently import/use only global authoring client and always use creation-mode draft storage.

- [ ] **Step 3: Implement narrowly optional injection points**

Use a minimal structural client interface in the authoring hooks:

```ts
export interface AuthoringUnaryClient {
  putDungeon: typeof authoringClient.putDungeon;
}

export function useSaveDungeon(
  client: AuthoringUnaryClient = authoringClient,
  onSaveSucceeded?: (key: string) => void,
): UseSaveDungeonResult;
export function usePutDungeonPreview(
  doc: DungeonDoc | null,
  yamlText: string,
  forceFixtures = false,
  client: AuthoringUnaryClient = authoringClient,
): UsePutDungeonPreviewResult;
export function useCreationFloorPlanPreview(
  doc: DungeonDoc | null,
  yamlText: string,
  serverState: ServerState,
  capabilities: ServerCapabilities | null,
  client: AuthoringUnaryClient = authoringClient,
): { floorPlan: FloorPlan | null };
export function probeAllCapabilities(
  client: AuthoringUnaryClient = authoringClient,
): Promise<ServerCapabilities>;
```

Add this builder contract with defaults that exactly preserve current AuthorView behavior:

```ts
export interface DungeonBuilderConceptProps {
  forceFixtures?: boolean;
  initialYaml?: string;
  authoringClient?: AuthoringUnaryClient;
  persistDraft?: boolean;
  allowNewCanvas?: boolean;
  allowYamlFileIO?: boolean;
  onSaveSucceeded?: (key: string) => void;
  showSaveResultLink?: boolean;
}
```

When `initialYaml` is supplied and `persistDraft` is false, parse only that literal at mount and never call `loadDraft`, `saveDraft`, or `discardDraft`; a reload therefore returns to the template. Thread `allowNewCanvas` and `allowYamlFileIO` into `CreationConcept` and `ProposedYamlPane` so the sandbox cannot create an alternate canvas or import/export a scenario. Thread the injected `authoringClient` through `usePutDungeonPreview`, `probeAllCapabilities`, `compileLive`, `useCreationFloorPlanPreview`, and `useSaveDungeon`; those helpers use their global default only when the prop is omitted. Have `useSaveDungeon` invoke `onSaveSucceeded(key)` directly in its `response.success` branch, exactly once per successful non-validate request. Thread `showSaveResultLink` to `SaveResultPanel` as `showLobbyLink`; its default is `true`, while `false` retains saved/error feedback but omits only the normal generic lobby anchor. Do not alter defaults, AuthorView, ConceptsView, or the existing global API client.

- [ ] **Step 4: Run authoring regressions and full web quality checks**

Run:

```bash
npm run test:run -- src/author/useSaveDungeon.test.ts src/author/usePutDungeonPreview.test.ts src/author/capabilityProbe.test.ts src/author/DungeonBuilderConcept.test.tsx src/author/YamlPane.test.tsx
npm run format:check
npm run lint
npm run typecheck
npm run build
```

Expected: every command exits `0`; normal authoring tests still use global behavior, while the new tests prove injected save/preview and template isolation.

- [ ] **Step 5: Commit the reusable-but-narrow builder injection slice**

```bash
git add src/author/usePutDungeonPreview.ts src/author/usePutDungeonPreview.test.ts \
  src/author/capabilityProbe.ts src/author/capabilityProbe.test.ts \
  src/author/useSaveDungeon.ts src/author/useSaveDungeon.test.ts \
  src/author/DungeonBuilderConcept.tsx src/author/DungeonBuilderConcept.test.tsx \
  src/author/creation/CreationConcept.tsx src/author/creation/ProposedYamlPane.tsx \
  src/author/YamlPane.tsx src/author/YamlPane.test.tsx
git commit -m "feat: support injected sandbox dungeon builder"
```

Expected: one focused Unit C commit that changes no ordinary authoring route behavior.

### Task 7: Implement the fixed sandbox authoring-to-lobby flow and normal links

**Files:**

- Create: `src/toolkit-contributor-sandbox/ToolkitContributorSandbox.tsx`
- Create: `src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx`
- Modify: `src/App.tsx:1-347` only if the Task 5 route needs the completed component import
- Test: sandbox component tests and all Unit C feature/authoring tests

**Interfaces:**

- Consumes: Unit C fixed constants and clients, `DungeonBuilderConcept` injected props, existing `PutDungeon`, `ListCharacters`, `CreateLobby`, `JoinLobby`, `SetReady`, and `StartEncounter` methods.
- Produces: a development-only screen with one template save and four exact party choices, then selected normal `GameView` links.
- Does not consume: global auth state, generic lobby hooks, `ListDungeons`, a scenario catalog, direct storage, or harness as a success surface.

- [ ] **Step 1: Write the failing fixed-sequence component tests**

Mock only the feature-local fixed clients. Assert save sends the exact stable key and that the fighter-then-barbarian arrangement makes this sequence:

```ts
expect(fighter.authoring.putDungeon).toHaveBeenLastCalledWith(
  expect.objectContaining({
    key: "toolkit-contributor-sandbox",
    validateOnly: false,
  }),
);
expect(fighter.character.listCharacters).toHaveBeenCalledOnce();
expect(barbarian.character.listCharacters).toHaveBeenCalledOnce();
expect(fighter.lobby.createLobby).toHaveBeenCalledWith(
  expect.objectContaining({
    campaignId: "toolkit-contributor-sandbox",
    characterId: "fighter-char",
  }),
);
expect(barbarian.lobby.joinLobby).toHaveBeenCalledWith(
  expect.objectContaining({
    joinRef: "join-fighter",
    characterId: "barbarian-char",
  }),
);
expect(fighter.lobby.setReady).toHaveBeenCalledWith(
  expect.objectContaining({ lobbyId: "lobby-1", ready: true }),
);
expect(barbarian.lobby.setReady).toHaveBeenCalledWith(
  expect.objectContaining({ lobbyId: "lobby-1", ready: true }),
);
expect(fighter.lobby.startEncounter).toHaveBeenCalledWith(
  expect.objectContaining({
    lobbyId: "lobby-1",
    dungeonKey: "toolkit-contributor-sandbox",
  }),
);
```

Add separate cases for fighter-only, barbarian-only, and barbarian-then-fighter. Assert each success renders exactly its selected `/?playerId=` normal links. Add a wrong-owner `CreateLobby` rejection case and assert it renders the RPC error, never calls ready/start, and renders no result link.

- [ ] **Step 2: Run the sandbox component tests and verify they fail before implementation**

Run:

```bash
npm run test:run -- src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx
```

Expected: non-zero module-resolution failure because `ToolkitContributorSandbox.tsx` does not yet exist.

- [ ] **Step 3: Implement one save gate and four literal party actions**

Render the injected `<DungeonBuilderConcept>` with these exact props:

```tsx
<DungeonBuilderConcept
  initialYaml={TOOLKIT_SANDBOX_YAML}
  authoringClient={clients.fighter.authoring}
  persistDraft={false}
  allowNewCanvas={false}
  allowYamlFileIO={false}
  onSaveSucceeded={handleSaveSucceeded}
  showSaveResultLink={false}
/>
```

Define `handleSaveSucceeded(key: string)` in the sandbox, reject any key other than `TOOLKIT_SANDBOX_KEY` visibly, clear stale result/error state, and load the two fixed lists. Only after both lists each contain exactly one character may it enable party choices; a missing/non-single list, validation response, or failed RPC leaves them disabled and renders the error. This callback is the only path that enables party choices. Implement the four literal arrangements from `constants.ts`; do not accept arbitrary order, player labels, IDs, count, join reference, dungeon key, or delete request. Await each RPC in order. Catch each error at the action boundary, show its message, and return without a later RPC. Render only normal player URL anchors for selected successful identities.

- [ ] **Step 4: Run focused, production-gate, and complete web validation**

Run:

```bash
npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts \
  src/toolkit-contributor-sandbox/route.test.ts \
  src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx \
  src/author/useSaveDungeon.test.ts src/author/usePutDungeonPreview.test.ts \
  src/author/capabilityProbe.test.ts src/author/DungeonBuilderConcept.test.tsx \
  src/author/YamlPane.test.tsx
npm run ci-check
```

Expected: every command exits `0`; the feature tests prove all four choices and stop-on-error behavior, while the route test proves production cannot activate the component.

- [ ] **Step 5: Commit and open the one Unit C PR only after providers land**

```bash
git add src/toolkit-contributor-sandbox/ToolkitContributorSandbox.tsx \
  src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx src/App.tsx
git commit -m "feat: add toolkit contributor sandbox flow"
git diff origin/dev...HEAD --check
```

Expected: exit `0`. After A and B are merged and the API dependency is resolvable from `dev`, open one PR to `dev` with exactly one closing `Closes #` line naming the Unit C issue.

### Task 8: Verify the merged cross-repository loop on a clean WSL2 machine

**Files:**

- Modify: no repository file
- Create: evidence attachments and one evidence comment on the Unit D verification issue
- Test: focused repository suites, Envoy live loop, browser/visual evidence, and clean rerun

**Interfaces:**

- Consumes: merged Units A–C and clean Ubuntu WSL2 with Docker Desktop WSL integration.
- Produces: reproducible acceptance evidence for #208; no code branch, no code PR, and no implementation issue beyond the already-created verification issue.

- [ ] **Step 1: Run focused repository gates at merged commits**

Run:

```bash
cd ~/game-dev/rpg-api && bash scripts/toolkit-local-override.test.sh && go test ./cmd/sandboxseed -count=1 && go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1 && make pre-commit
cd ~/game-dev && bash tests/toolkit-contributor-contract.sh && bash tests/bootstrap-contract.sh
cd ~/game-dev/rpg-dnd5e-web && npm run test:run -- src/toolkit-contributor-sandbox/clients.test.ts src/toolkit-contributor-sandbox/route.test.ts src/toolkit-contributor-sandbox/ToolkitContributorSandbox.test.tsx && npm run ci-check
```

Expected: every command exits `0`. Record the merged commit IDs and compact exit-code transcript in Unit D.

- [ ] **Step 2: Prove first bootstrap and idempotent bootstrap without modifying existing checkouts**

Run:

```bash
git clone git@github.com:KirkDiggler/game-dev.git ~/toolkit-sandbox-clean/game-dev
cd ~/toolkit-sandbox-clean/game-dev
./scripts/toolkit-contributor.sh bootstrap
./scripts/toolkit-contributor.sh bootstrap
```

Expected: both commands exit `0`; exactly `rpg-toolkit`, `rpg-api`, `rpg-dnd5e-web`, and `rpg-deployment` are present from this facade and the second transcript reports unchanged valid roots.

- [ ] **Step 3: Start through the exact composition and prove Envoy health**

Run:

```bash
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh status
cd tools/browser && npm ci && npx playwright install chromium
cd ../../rpg-dnd5e-web && npm ci && npm run dev
```

Expected: start/status exit `0`; status reports the D&D 5e local override source/revision and Envoy-serving API; Vite serves `http://localhost:3001/?toolkitSandbox=1`.

- [ ] **Step 4: Prove a reversible local rulebook marker through an existing API projection**

The baseline fighter uses base Strength 15 and Human’s `AbilityIncreases[abilities.STR]` value 1 in `rpg-toolkit/rulebooks/dnd5e/races/data.go`, so `cmd/sandboxseed` must report Strength 16 after `seed`.

Run:

```bash
cd ~/toolkit-sandbox-clean/game-dev
./scripts/toolkit-contributor.sh seed
perl -0pi -e 's/(Human: \{.*?abilities\.STR: )1,/${1}2,/s' rpg-toolkit/rulebooks/dnd5e/races/data.go
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed
git -C rpg-toolkit checkout -- rulebooks/dnd5e/races/data.go
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed
```

Expected: each command exits `0`; seed output shows fighter Strength 16 before the edit, 17 only after refresh plus reseed, and 16 after restoring the source plus refresh plus reseed. This uses the existing `GetCharacter.ability_scores.strength` projection; it adds no debug protocol/API value.

- [ ] **Step 5: Capture authoring, all party orders, and normal GameView evidence**

Open `http://localhost:3001/?toolkitSandbox=1`, save the populated template, then execute exactly Fighter, Barbarian, Fighter then Barbarian, and Barbarian then Fighter. For every result, open the displayed normal `?playerId=` link and capture browser evidence that normal `GameView`/`EncounterView` renders. Optional `?encounterId=` harness pages may be captured only as diagnosis alongside, never instead of normal links.

Before capture, the clean checkout must have completed `cd tools/browser && npm ci && npx playwright install chromium` from Step 3; `screenshot.mjs` imports Playwright and is not usable before that setup. Capture a distinct normal-route screenshot for every selected identity/order so a later order cannot overwrite earlier evidence:

```bash
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
  /tmp/toolkit-sandbox-fighter-only-fighter-gameview.png
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
  /tmp/toolkit-sandbox-barbarian-only-barbarian-gameview.png
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
  /tmp/toolkit-sandbox-fighter-then-barbarian-fighter-gameview.png
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
  /tmp/toolkit-sandbox-fighter-then-barbarian-barbarian-gameview.png
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-barbarian' \
  /tmp/toolkit-sandbox-barbarian-then-fighter-barbarian-gameview.png
node ~/toolkit-sandbox-clean/game-dev/tools/browser/screenshot.mjs \
  'http://localhost:3001/?playerId=toolkit-sandbox-fighter' \
  /tmp/toolkit-sandbox-barbarian-then-fighter-fighter-gameview.png
```

Expected: each selected link reaches normal GameView; retain all six order-qualified files and the successful `PutDungeon` key evidence.

- [ ] **Step 6: Run the required negative checks and owned shutdown**

Run:

```bash
# API unit evidence already proves this exact production-mode auth negative.
cd ~/toolkit-sandbox-clean/game-dev/rpg-api
go test ./internal/auth -run TestUnaryAuthInterceptor_DevScheme_NotAllowed -count=1

# Web production route negative.
cd ~/toolkit-sandbox-clean/game-dev/rpg-dnd5e-web
npm run test:run -- src/toolkit-contributor-sandbox/route.test.ts

cd ~/toolkit-sandbox-clean/game-dev
./scripts/toolkit-contributor.sh down
test ! -e rpg-api/local-toolkit/rulebooks/dnd5e
```

Expected: every command exits `0`; Unit A integration evidence documents wrong-owner Create/Join rejections, the production Dev-header test rejects the scheme, the production route test rejects the query, and down removes only the owned D&D 5e tree after stopping the stack.

- [ ] **Step 7: Publish only the independent verification evidence**

Post one concise Unit D evidence comment containing the command transcript with exit codes, API/Envoy marker results, two consecutive seed results, ownership/production negatives, all four party-order normal GameView artifacts, and the clean WSL2 rerun result. Keep #209 open and do not add a closing keyword for Unit D.

## Validation Matrix

| Requirement                         | Automated evidence                                                                                                                                                                                                                                                | Live evidence                                                                                                                  |
| ----------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| One D&D 5e override only            | `bash scripts/toolkit-local-override.test.sh` proves target/module validation, exact owned RHS/state validation, target-switch refusal without mutation, unknown/second rejection, refresh, and exact cleanup; `./scripts/verify-release-pin.sh` rejects residue. | `status` reports one owned D&D 5e source/revision/sync time; start/refresh build `Dockerfile.local-toolkit`.                   |
| API seeding only                    | `go test ./internal/integration/character -run '^TestSandboxSeedSuite$/^TestSandboxSeed_ResetsTwoIdentitiesThroughRPCs$' -count=1` proves two consecutive RPC seed runs, fixed names/classes/style, off-hand shield, and ownership negatives.                     | `./scripts/toolkit-contributor.sh seed` twice prints two identities, exactly one each, and fighter shield/Strength projection. |
| Envoy path                          | `go test ./cmd/sandboxseed -count=1` covers address/health behavior; facade contract tests prove bounded fail-then-serve and timeout polling.                                                                                                                     | facade polls `go run ./cmd/sandboxseed --address localhost:8080 --health` through Envoy; no Redis endpoint is used.            |
| WSL facade boundaries               | `bash tests/toolkit-contributor-contract.sh` proves four-repo bootstrap only, dirty preservation, delegation, compose order, and down scope.                                                                                                                      | clean WSL2 bootstrap twice, start/status/refresh/seed/down transcript.                                                         |
| Local rulebook proof                | API integration baseline asserts human fighter Strength 16.                                                                                                                                                                                                       | temporary Human STR bonus 1→2 produces 16→17 only after refresh/reseed, then 17→16 after restoration/refresh/reseed.           |
| Sandbox template and auth isolation | Vitest tests assert the literal key/template, injected liveness/capability/preview/save clients, zero global-client calls for a sandbox mount, callback-gated party choices, no local draft persistence, and interleaved immutable headers.                       | save one populated builder canvas under `toolkit-contributor-sandbox`.                                                         |
| Four lobby combinations             | Component tests assert Fighter, Barbarian, Fighter→Barbarian, Barbarian→Fighter and error stops. API integration asserts wrong-owner Create/Join failures.                                                                                                        | all four arrangements reach ordinary `?playerId=` GameView links.                                                              |
| Production gates                    | `TestUnaryAuthInterceptor_DevScheme_NotAllowed` and `route.test.ts` production query negative.                                                                                                                                                                    | no production server/build sandbox route or accepted Dev header is claimed.                                                    |

## Source Seams Verified Before Planning

These were inspected from freshly fetched upstream refs without switching primary worktrees:

| Repository/ref                 | Commit inspected                           | Exact seam                                                                                                                                                                                                                                                  | Planning consequence                                                                                                                              |
| ------------------------------ | ------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| `rpg-api` `origin/dev`         | `9556baf9cc7eaefe3929c8d38f342d2ec0d61a42` | `scripts/toolkit-local-override.sh:1-229` currently syncs `encounter` into ignored `local-toolkit/encounter`; `Dockerfile.local-toolkit:1-53` copies full context before module download; `scripts/verify-release-pin.sh:1-43` rejects replaces/local tree. | Extend the existing one-module helper and retain normal release path; do not invent a second override mechanism.                                  |
| `rpg-api` `origin/dev`         | `9556baf9cc7eaefe3929c8d38f342d2ec0d61a42` | `internal/integration/character/creation_test.go:118-270` has real fixed fighter section calls; `:477-529` has real barbarian calls; `internal/handlers/dnd5e/v1alpha1/character/handler.go:98-977` implements the listed creation/equipment RPCs.          | Seeder uses these literal production request shapes and no requirements/choice-discovery path.                                                    |
| `rpg-api` `origin/dev`         | `9556baf9cc7eaefe3929c8d38f342d2ec0d61a42` | `internal/orchestrators/lobby/create_lobby.go:12-72` and `join_lobby.go:12-117` call `resolveCharacter` with authenticated player; `internal/auth/interceptor.go:19-116` rejects Dev scheme when DevMode is false.                                          | Test Create and Join wrong-owner negatives; do not make broader character-service ownership claims.                                               |
| `rpg-api` `origin/dev`         | `9556baf9cc7eaefe3929c8d38f342d2ec0d61a42` | `cmd/server/server.go:96-108,294-319` gates Dev auth and AuthoringService; `cmd/client/character.go:1-138` uses insecure gRPC dial; `internal/integration/harness/harness.go:37-88,193-198` provides real Redis/bufconn CharacterService fixture.           | CLI dials Envoy as a gRPC client; test the production service with metadata rather than storage.                                                  |
| `game-dev` `origin/main`       | `3108e32cef96cc3da7b832a036222a8bde7c094b` | `bootstrap.sh:1-196` and `scripts/workspace-repos.sh:1-11` own a broader seven-repo bootstrap; `scripts/dev-env.sh:145-305` owns a two-file developer environment; `tests/bootstrap-contract.sh:1-102` is the shell contract style.                         | Add an independent narrow facade; do not mutate broad bootstrap or dev-env.                                                                       |
| `rpg-deployment` `origin/main` | `453c3894e9563067cbf60fdb2e2292e7b71bb377` | `docker-compose.local-dev.yml:1-91` provides Dev auth/Envoy; `docker-compose.api.yml:1-46` provides authoring/content; `docker-compose.local-api-src.yml:1-24` replaces only API image.                                                                     | Consume these unchanged in literal local-dev → api → local-api-src order.                                                                         |
| `rpg-dnd5e-web` `origin/dev`   | `b835282391cce897bc90fa94bea2fb88d352c72c` | `src/api/client.ts:1-124` owns mutable global auth clients; `src/App.tsx:27-347` owns dev query gates and normal `GameView`; `src/api/useDevPlayerIdAuth.ts:1-19` mutates global auth.                                                                      | Sandbox uses isolated feature clients and does not touch global auth or normal `?playerId` behavior.                                              |
| `rpg-dnd5e-web` `origin/dev`   | `b835282391cce897bc90fa94bea2fb88d352c72c` | `src/author/DungeonBuilderConcept.tsx:101-623`, `usePutDungeonPreview.ts:75-430`, `capabilityProbe.ts:1-580`, `useSaveDungeon.ts:18-82`, and `creation/CreationConcept.tsx:47-503` are existing builder/save seams.                                         | Inject one client through every authoring call, template/persistence/control option, and reuse existing editor controls.                          |
| `rpg-dnd5e-web` `origin/dev`   | `b835282391cce897bc90fa94bea2fb88d352c72c` | `src/components/game/LobbyFlow.tsx:67-226` uses Create/Join/Ready/Start; `src/components/game/GameView.tsx:18-66` is normal resume/play surface.                                                                                                            | Sandbox drives the same RPC order with bound clients, then shows normal GameView links rather than replacing it.                                  |
| `rpg-toolkit` `origin/main`    | `0fbcbcae9ae72a0c9ce22d6583023bc80cc098e9` | `rulebooks/dnd5e/races/data.go:42-61` projects Human’s `abilities.STR: 1` through normal character creation.                                                                                                                                                | The WSL proof temporarily changes only this local source value to 2 and observes returned Strength; no toolkit commit/change is part of delivery. |

## Plan Self-Review

### Spec coverage

- Unit A covers safe D&D 5e override target/module validation, exact owned RHS/state enforcement, no-mutation target-switch/refusal cases, refresh, exact off cleanup, release-pin refusal, real Envoy RPC seeding, fixed choices, authenticated list/delete reset, post-finalize shield equip, double seed, and cross-owner lobby negatives.
- Unit B covers WSL2 prerequisites, restricted bootstrap, exact compose order, bounded Envoy startup polling with fail-then-serve/timeout tests, explicit start/refresh/seed/status/down commands, no checkout mutation, and owned shutdown.
- Unit C covers the development-only query gate, immutable per-identity authoring/character/lobby clients, injected authoring client flow through probe/capability/preview/save with zero global calls, callback-gated template save, fixed arrangements, lobby ordering, stop-on-error, and normal GameView links.
- Unit D covers clean WSL2 rerun, installed Playwright/Chromium screenshot runtime, all four arrangements with order-qualified normal-GameView evidence, reversible rulebook marker, production Dev/sandbox negatives, and supplemental-only harness posture.
- The global constraints explicitly exclude toolkit/proto/deployment work, direct storage/data construction, unimplemented generic draft RPCs, discovery/catalog/watchers, generic client factory, arbitrary cleanup, and production-authoring scope expansion.

### Deferred-work marker scan

Run before committing any implementation task:

```bash
grep -nE 'T''BD|TO''DO|impl''ement later|fill in det''ails|similar to ta''sk' \
  ideas/toolkit-contributor-sandbox/plan.md
```

Expected: exit `1` with no matches. The plan uses literal names, fields, values, commands, and state transitions instead of deferred markers.

### Interface consistency

- Unit B calls only Unit A’s documented `on --target rulebooks/dnd5e`, `refresh --src`, `status`, `off`, `cmd/sandboxseed --address`, and `--health` interfaces.
- Unit C’s `SandboxUnaryClients` names match the existing `putDungeon`, `listCharacters`, `createLobby`, `joinLobby`, `setReady`, and `startEncounter` client methods.
- The builder receives the same `AuthoringUnaryClient` in mount probe, `probeAllCapabilities`, `compileLive`, creation preview, and save hooks, so its sandbox template cannot accidentally use mutable global auth while normal routes retain their default client.
- `onSaveSucceeded(key)` is declared by the builder, called by the non-validate save success branch, and consumed only by the sandbox’s party-enable handler; `showSaveResultLink` defaults to normal behavior and is false only for sandbox composition.
- All fixed identities and the stable dungeon key are declared once in `constants.ts` and used in seed, lobby, and normal-link assertions.

## Final Delivery Order

1. Merge Unit A to `rpg-api/dev`; verify its one issue/one PR closing reference and release-pin-clean diff.
2. Merge Unit B to `game-dev/main`; verify the facade consumes the landed Unit A commands and exact compose overlays.
3. Rebase/refresh Unit C only as necessary against landed provider contracts, then merge it to `rpg-dnd5e-web/dev`.
4. Execute Unit D from clean WSL2, post its evidence comment, then update the still-open tracking surface according to the approved review workflow. Do not begin a new implementation wave from this plan.
