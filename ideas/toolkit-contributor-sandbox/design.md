# Toolkit contributor onboarding and live sandbox loop

**Status:** design for review

**Umbrella:** [rpg-project#208](https://github.com/KirkDiggler/rpg-project/issues/208)

**Scope:** one WSL2 contributor loop, one local toolkit module, one canvas template, and one active scenario

## Problem

A contributor working on `rpg-toolkit/rulebooks/dnd5e` from Windows through
WSL2 cannot currently make a local rulebook edit and exercise it through the
real game without learning several repository-specific procedures. The current
bootstrap is broader than this job, the API override supports a different
module, the existing dev seeder writes storage records directly, and the web's
Dungeon Builder concept does not compose the real authoring, lobby, and normal
game paths into one loop.

The result is slow and unsafe onboarding: it is easy to run a published module
instead of the edited source, seed data that production APIs could not create,
or test a fixture/harness that bypasses the path a player uses.

The target environment is Ubuntu under WSL2 with Docker Desktop WSL integration.
Native Windows asset tooling is not part of this workflow. The contributor edits
files inside the WSL filesystem and runs Git, Go, Node, and Docker commands from
WSL. Docker Desktop supplies the engine; the workflow must diagnose missing WSL
integration rather than attempting to configure Windows or global developer
tools.

## Outcome and simple-first shape

A contributor gets one explicit command facade in `game-dev`, edits only the
local `rulebooks/dnd5e` checkout, explicitly refreshes the running API, seeds two
real characters, opens one preloaded Dungeon Builder canvas, chooses one or two
seats in either order, and enters the resulting encounter through normal
`GameView`.

The MVP deliberately has:

- one checked-in canvas template;
- one stable dungeon key, `toolkit-contributor-sandbox`;
- one active waiting/running scenario for the sandbox identities;
- two dedicated Dev identities and one character per identity;
- an explicit refresh command, with no watcher; and
- no scenario schema, catalog, picker, importer, or editor beyond the existing
  Dungeon Builder.

## Boundary Rule

The platform rule is preserved unchanged:

```text
Client sends REFERENCES (keys, IDs) -> never calculations
API orchestrates by KEY            -> never knows what "rage" does
Toolkit implements RULES           -> returns rich breakdowns for rendering
```

Applied here:

- the web sends dungeon, character, lobby, item, and player references plus user
  intent; it does not decide character legality or calculate game state;
- `rpg-api` authenticates, orchestrates existing character/authoring/lobby APIs,
  persists through repositories, and delegates character/equipment rules to the
  toolkit;
- `rpg-toolkit/rulebooks/dnd5e` remains the source of class choices,
  finalization, equipment rules, and combat behavior; and
- the contributor tooling coordinates processes but never constructs game data.

No path in this design writes Redis directly or constructs `Character.Data`,
`MonsterData`, encounter data, or equipment-slot data. The legacy
`rpg-api/cmd/devseed` implementation is not reused or extended.

## Exact contributor experience

### First checkout

The contributor installs/enables WSL2, Docker Desktop with WSL integration, Git,
GitHub SSH access, Go, Node/npm, `rsync`, and `jq`, then runs:

```bash
git clone git@github.com:KirkDiggler/game-dev.git ~/game-dev
cd ~/game-dev
./scripts/toolkit-contributor.sh bootstrap
```

`bootstrap` verifies the environment and clones only the repositories needed by
this loop when absent: `rpg-toolkit`, `rpg-api`, `rpg-dnd5e-web`, and
`rpg-deployment`. It is convergent: an already valid checkout is reported and
left in place. It does not switch branches, clean dirty work, install Pi or
OpenCode, copy personal configuration, install Blender/assets tooling, or change
Windows/Docker Desktop settings.

### Daily loop

From `~/game-dev`:

```bash
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh seed
./scripts/toolkit-contributor.sh status

cd rpg-dnd5e-web
npm run dev
```

`start` enables the approved local `rulebooks/dnd5e` override, builds the local
API image with authoring and Dev auth enabled, starts the existing compose stack,
waits for gRPC health through Envoy, and prints the web command and sandbox URL.
`seed` invokes the API-owned seeder against the running service. `status` reports
prerequisites, the one active override and its source, the API image/container,
health, authoring/Dev-mode gates, ports, seed roster, and repository dirtiness.

The contributor opens:

```text
http://localhost:3001/?toolkitSandbox=1
```

The sandbox opens the one template in Dungeon Builder. The contributor may edit
it, explicitly saves it, chooses Fighter, Barbarian, Fighter then Barbarian, or
Barbarian then Fighter, and starts the scenario.

After each toolkit edit:

```bash
cd ~/game-dev
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed   # rerun when character output is relevant
```

`refresh` is the only source-refresh action. It revalidates and resyncs
`rulebooks/dnd5e`, rebuilds the API image, replaces only the API container after
a successful build, waits for health, and reports the local source revision and
sync time. There is no file watcher, implicit polling, or automatic rebuild.
Browser refresh/HMR alone does not refresh Go source.

At the end of the session, stop Vite with Ctrl-C and run:

```bash
cd ~/game-dev
./scripts/toolkit-contributor.sh down
```

`down` stops the contributor stack and removes only the override state created by
this workflow, restoring the API module files to the published-pin state. It
leaves toolkit edits and every repository checkout intact. It refuses cleanup if
the active replacement or generated path does not exactly match the state the
workflow owns.

## Repository ownership

| Repository       | Owns in this delivery                                                                                                                 | Does not own                                                                            |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| `rpg-project`    | This cross-repository design, later plan, umbrella tracking, and final clean-WSL2 verification                                        | Implementation code                                                                     |
| `game-dev`       | The WSL2-facing `bootstrap`, `start`, `refresh`, `seed`, `status`, and `down` facade plus contributor docs and command-contract tests | Character rules, seed payload construction, web UI, or compose-service reimplementation |
| `rpg-api`        | The safe single-module local override and `cmd/sandboxseed`, which is a real API client                                               | Toolkit rules, web composition, or direct storage seeding                               |
| `rpg-dnd5e-web`  | The development-only sandbox composition, Dungeon Builder preload, isolated Dev clients, party ordering, and launch links             | Character creation rules or server ownership decisions                                  |
| `rpg-toolkit`    | The locally edited D&D 5e rules product and its existing Protection/equipment behavior                                                | Sandbox orchestration; no MVP toolkit change is required                                |
| `rpg-api-protos` | Existing contracts only                                                                                                               | No proto change in this MVP                                                             |
| `rpg-deployment` | Existing local compose files, unless later evidence proves a missing capability                                                       | No planned deployment change                                                            |

`game-dev` delegates: its `seed` command calls the API repository's seeder and its
start/refresh commands call the API repository's override helper. It does not
copy either implementation. Implementation and verification issues are created
only after this design and its later plan are approved.

## Override safety

The existing API override mechanism is extended to support the approved
`github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e` target while retaining its
one-module invariant.

1. `on` validates that the source `go.mod` declares the exact selected module,
   and syncs only `rulebooks/dnd5e/` into the API's gitignored Docker build
   context.
2. The helper allowlists known targets but permits exactly one active `replace`.
   The contributor facade always selects `rulebooks/dnd5e`. Sibling modules stay
   on published versions.
3. Any unknown replacement, second replacement, missing source, mismatched module
   declaration, or attempt to switch targets without `off` fails before
   mutation.
4. `refresh` can only resync the already selected target. It cannot choose a new
   target or traverse outside the validated checkout.
5. The local image uses the dedicated local-toolkit Dockerfile. Normal/release
   builds continue to use published module pins.
6. `off` removes only the exact replacement and generated local tree owned by the
   helper. Release-pin/CI checks continue to reject committed local replacements.

A local `rulebooks/dnd5e` replacement applies transitively where the released
`encounter` module imports that rulebook. A second `encounter` override is not
needed for this sandbox and is rejected while the rulebook override is active.

## API seeding and identity model

The fixed identities are:

| Dev player ID               | Expected character                                   |
| --------------------------- | ---------------------------------------------------- |
| `toolkit-sandbox-fighter`   | `Toolkit Sandbox Fighter`, level-1 Human Fighter     |
| `toolkit-sandbox-barbarian` | `Toolkit Sandbox Barbarian`, level-1 Human Barbarian |

They are local development identities, not credentials. Every seeder RPC carries
`authorization: Dev <player-id>`. Request `player_id` fields are not trusted as
authentication; the server-derived identity remains authoritative.

### Idempotent reconciliation

`rpg-api/cmd/sandboxseed` reconciles each identity independently:

1. call the implemented `ListRaces`, `ListClasses`, and `ListBackgrounds` section
   catalog RPCs and find the fixed Human/class/background choices;
2. call authenticated `ListCharacters` for the dedicated identity;
3. if exactly one character matches the expected finalized shape, reuse it;
4. if the Fighter is otherwise valid but its real shield is not equipped, call
   `EquipItem` and reuse it;
5. if the identity is empty, duplicated, or drifted, delete only IDs returned by
   that identity's authenticated `ListCharacters`, then recreate exactly one;
6. re-list/get the character and verify class, required inventory/equipment, and
   cardinality before reporting success.

The command accepts no arbitrary character ID and never deletes an ID learned
outside the dedicated identity's own list. A failure after reconciliation is
safe to rerun; the next invocation converges the same two identities back to one
valid character each. Random finalized character IDs therefore do not create
duplicates.

### Real implemented character path

For a character that must be created, the client uses only currently implemented
section RPCs:

```text
CreateDraft
  -> UpdateName
  -> UpdateRace (race choices inline)
  -> UpdateClass (skills, equipment, and fighting style choices inline)
  -> UpdateBackground (background choices inline)
  -> UpdateAbilityScores
  -> GetDraft (real validation)
  -> FinalizeDraft
  -> GetCharacter/ListCharacters (persistence verification)
```

The generated `GetRequirements` and `SubmitChoices` RPCs exist but their current
handlers are unimplemented. Implementing them, or generic choice dispatch, is
not part of this MVP and must not become a prerequisite. The seeder reads the
implemented section catalogs and submits their advertised choice/option IDs in
the corresponding `UpdateRace`, `UpdateClass`, and `UpdateBackground` requests.
If the running rulebook no longer advertises a required choice, seeding fails
with the missing section/choice named rather than hardcoding around toolkit
validation.

The Fighter submits the real
`FIGHTING_STYLE_PROTECTION` class choice and the advertised “martial weapon and
shield” equipment option. Finalization puts the shield in inventory but does not
equip it. After `FinalizeDraft`, the seeder calls the existing real `EquipItem`
RPC for the returned shield item and off-hand slot, then reads the character back
and verifies the shield is actually equipped. It never writes an equipment slot
or patches character data. The Barbarian submits ordinary level-1 skill and
equipment choices; Rage remains a toolkit class grant, not a seed-time
construction.

The default API address is Envoy at `localhost:8080`, which supports native gRPC
and is the host-visible port in the local compose stack. An explicit address
flag may support a host-run API, but the `game-dev seed` facade uses the healthy
stack address and fixed scenario—there is no generic fixture schema.

## Web sandbox and authoring lifecycle

The web exposes the sandbox only when `import.meta.env.MODE === 'development'`
and `?toolkitSandbox=1` is present. Production builds do not render or route to
it.

The sandbox reuses `DungeonBuilderConcept`; it does not fork a second editor.
The builder receives a sandbox-only initial YAML/canvas input and an authoring
client override. The template is a current `version: 1` canvas document with
explicit dimensions and top-level placed monster references/coordinates—the
shape the creation canvas renders and the current dungeon compiler accepts. It
does not use a room-chain document or unsupported count-based/rolled monsters.
Sandbox entry ignores the normal `create` local-storage draft so stale authoring
work cannot replace the template, and it does not write back to that normal
draft key.

The one authoring lifecycle is:

1. open the checked-in, populated canvas template as an in-memory editable draft;
2. edit using the existing Dungeon Builder canvas controls;
3. validate through the real `PutDungeon(validate_only: true)` preview path;
4. explicitly choose **Save and assemble**, which calls
   `PutDungeon(validate_only: false)` with the stable
   `toolkit-contributor-sandbox` key; and
5. keep the successfully saved key as the only scenario available to the seat
   composer.

A reload starts from the template again. A save overwrites the same development
key. There is no watcher, autosaved sandbox catalog, multiple-template picker,
scenario import/export format, generic scenario metadata, or server-side delete
requirement.

## Isolated clients and party composition

The current web clients read module-global auth state at request time. That is
unsafe for two identities acting concurrently in one page. The sandbox therefore
uses a development client factory that creates a transport and service clients
bound to one immutable Dev player ID. Its interceptor always emits that client's
`Dev` header and never calls or reads `setAuth` for sandbox requests.

Tests must prove that interleaved Fighter and Barbarian unary calls and streams
retain their own headers. The normal application clients and Discord auth path
remain unchanged.

After a successful dungeon save, the sandbox lists the one character owned by
each dedicated identity. The contributor selects one or both cards; click/order
controls define seat order. All four supported compositions use the same saved
template:

- Fighter;
- Barbarian;
- Fighter then Barbarian; and
- Barbarian then Fighter.

The first selected seat is host. Its isolated client calls `CreateLobby` with
its own character. For two seats, the second identity's isolated client calls
`JoinLobby` with the returned `join_ref` and its own character. Every selected
identity calls `SetReady`; only the host client calls `StartEncounter` with the
saved dungeon key. Server auth/ownership and all-ready/host checks remain
binding. The web does not impersonate one identity with another identity's
character and does not weaken these checks for ordering.

## Normal game entry and optional harness

Success is not a harness result. After `StartEncounter` returns, the sandbox
shows one normal link per selected identity:

```text
http://localhost:3001/?playerId=toolkit-sandbox-fighter
http://localhost:3001/?playerId=toolkit-sandbox-barbarian
```

Only selected identities are shown. Opening a link uses the normal app's
`GetMyActiveLobby`/resume routing and renders `GameView`; the normal link contains
no `encounterId`, because that query currently selects `PlaytestHarness`.

For diagnostics the sandbox may additionally show explicit harness links:

```text
http://localhost:3001/?encounterId=<id>&playerId=<selected-player-id>
```

Those links are supplemental and cannot satisfy the acceptance path.

## Errors, retries, and cleanup

- Every command prints the failed precondition and exits non-zero. Bootstrap and
  start refuse to switch, clean, stash, or reset an existing checkout.
- A refresh validates/syncs/builds before replacing the running API container. A
  compile failure leaves the previously running container available and reports
  that it is stale relative to local source.
- Seeder errors name the identity and RPC. A partial seed is not called success;
  rerunning performs reconciliation. It never falls back to Redis.
- Dungeon validation/save failures keep the current in-memory edits and do not
  enable party assembly. A successful save followed by lobby failure leaves only
  the same stable dungeon key, which the next save may overwrite.
- Each composition step is visible. A failed join/ready/start stops later calls,
  identifies the acting identity, and offers retry or cleanup rather than
  guessing that the server accepted the call.
- Before creating a scenario, each selected identity checks its active lobby. A
  waiting lobby can be left through the existing `LeaveLobby` API. A running
  encounter is shown with its normal link and blocks a second scenario until the
  contributor explicitly abandons it through the existing encounter cleanup API.
  No active encounter is silently destroyed.
- Cleanup asks every joined identity to leave a still-waiting lobby. Missing or
  already-terminal state is treated idempotently; authorization failures are
  surfaced. There is no direct deletion of lobby, encounter, dungeon, or Redis
  records.
- `down` stops processes and removes only owned override artifacts. It does not
  delete persisted game data or contributor source edits.

## Security and production gates

1. The API accepts `Dev` authentication only when its existing server DevMode is
   explicitly enabled by the local contributor start path. Production continues
   to reject `Dev` headers.
2. The web route and bound Dev-client factory are development-only. Production
   builds cannot activate them with a query string.
3. Fixed Dev identities are local labels, not shared secrets or production
   accounts. No token is logged.
4. Existing character ownership, lobby host, ready, party-size, and authoring
   checks are not bypassed. Negative tests bind a character to the wrong identity
   and require server rejection.
5. The seeder derives ownership from auth context, scopes reconciliation to
   `ListCharacters` results for that identity, and accepts no arbitrary delete
   target.
6. The override stays in ignored local build context, is rejected by release-pin
   checks if left active, and is never part of a normal/production image.
7. Authoring is enabled only for the local stack. This design introduces no new
   production endpoint, proto, or auth mode.

## Data and control flow

```text
WSL2 contributor edits rpg-toolkit/rulebooks/dnd5e
  -> game-dev refresh delegates to rpg-api override helper
  -> helper validates + syncs that one module into API build context
  -> local API image rebuilds and restarts behind Envoy

 game-dev seed delegates to rpg-api/cmd/sandboxseed
  -> Dev-scoped section catalogs + CharacterService draft/update/finalize RPCs
  -> toolkit validates/finalizes rules-owned character data
  -> EquipItem equips the finalized Fighter's real shield
  -> API repositories persist through normal paths

 dev-only web sandbox
  -> isolated author client -> PutDungeon(single canvas, stable key)
  -> isolated host client -> CreateLobby
  -> optional isolated second client -> JoinLobby
  -> each isolated client -> SetReady
  -> host client -> StartEncounter(saved key)
  -> API invokes normal dungeon/encounter/toolkit path
  -> per-identity normal URL -> GetMyActiveLobby -> GameView
```

The deterministic proof of local execution must be observed through this flow,
not inferred from a successful Docker build. Clean-WSL verification makes a
benign, unique local-only change to a rulebook value already projected by an
implemented API response, refreshes, and records that value through the running
API. It then removes/reverts the marker, refreshes again, and proves the
published behavior returns. The exact projected field is chosen during
implementation from a stable existing section response; adding a debug proto or
API-only version lie is not allowed.

## Tests and verification matrix

### Automated repository evidence

| Owner           | Required focused evidence                                                                                                                                                                                                                                                                                                                                                                                    |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `rpg-api`       | Shell tests for the D&D 5e target, exact module declaration, one-replace rejection, target-switch rejection, refresh resync, and exact cleanup; seeder client tests for per-RPC Dev metadata, catalog-derived section choices, Protection plus shield option, post-finalize `EquipItem`, reuse/reconcile idempotency, and no arbitrary delete; real character integration coverage; repository pre-commit/CI |
| `game-dev`      | Bootstrap/command contract tests for WSL2 and prerequisites, minimum clone set, convergent rerun, dirty-checkout refusal/preservation, delegated start/refresh/seed, health/status output, build-failure preservation, and owned down cleanup; shell syntax and repository checks                                                                                                                            |
| `rpg-dnd5e-web` | Development/production route gate; template preload and normal-draft isolation; real `PutDungeon` save; immutable per-identity headers under interleaved calls/streams; four one/two-seat ordering cases and exact lobby call order; ownership/error/cleanup cases; normal `GameView` links distinct from optional harness links; repository CI                                                              |

Tests use narrow fake clients for orchestration order and errors, plus existing
real integration paths for rule/ownership behavior. A fake transport cannot be
the only evidence for seeding, authoring, lobby start, or normal resume.

### Clean-WSL2 acceptance

Run once from a fresh supported WSL2 distribution, not the developer's existing
workspace, and retain commands, exit codes, logs, IDs, and screenshots where
noted:

| Case               | Required observation                                                                                                                                                                                                                                                         |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Bootstrap          | Initial `bootstrap` succeeds; second run is a no-op; prerequisites and four required repositories are reported; no asset/native-Windows setup occurs                                                                                                                         |
| Start/status       | Local stack becomes healthy through Envoy; status names `rulebooks/dnd5e` as the sole replacement, authoring/Dev mode, image, source path, sync time, and ports                                                                                                              |
| Explicit refresh   | A unique local-only rulebook value is absent before edit, present through the running API only after `refresh`, and absent again after marker removal plus another `refresh`                                                                                                 |
| Idempotent seed    | Two consecutive `seed` runs leave exactly one expected character under each identity; output identifies reused/reconciled IDs; API traces show section draft/finalize and equip calls; Fighter has Protection, a real shield in inventory, and that shield equipped off hand |
| Authoring          | Sandbox begins with the populated canvas template; explicit save succeeds through `PutDungeon`; recorded key is `toolkit-contributor-sandbox`                                                                                                                                |
| One-seat Fighter   | Fighter creates/readies/starts and reaches normal `GameView`                                                                                                                                                                                                                 |
| One-seat Barbarian | Barbarian creates/readies/starts and reaches normal `GameView`                                                                                                                                                                                                               |
| Two-seat F -> B    | Fighter hosts, Barbarian joins, both ready, and both normal identity links reach the same encounter                                                                                                                                                                          |
| Two-seat B -> F    | Barbarian hosts, Fighter joins, both ready, and both normal identity links reach the same encounter                                                                                                                                                                          |
| Auth negative      | A wrong-identity character bind is rejected; production-mode server rejects `Dev`; production web build does not expose the sandbox                                                                                                                                          |
| Error/cleanup      | Failed build preserves the last healthy API; failed save cannot assemble; waiting lobby cleanup is idempotent; active encounter blocks replacement until explicit abandon                                                                                                    |
| Down               | Stack stops, approved replacement/generated copy are removed, toolkit edits remain, and no workflow-created staged files remain                                                                                                                                              |

Each composition begins after explicit cleanup of the previous active scenario
and reuses the same canvas/template/key. The verification report includes both
normal links and may include harness screenshots only as supplemental debugging
evidence.

## Delivery phases

1. **Design gate:** review and approve this file on the umbrella branch. No
   implementation issue is created by this design change.
2. **Plan gate:** add `plan.md` to the same still-open `rpg-project` review PR only
   after design approval. The approved plan names implementation issues and
   acceptance commands.
3. **API provider:** deliver the safe rulebook override and real API seeder in one
   `rpg-api` wave from `origin/dev`. No toolkit/proto issue is expected.
4. **Contributor facade:** deliver `game-dev` commands/docs from `origin/main`,
   consuming the landed API contracts rather than reimplementing them.
5. **Web composition:** the web may develop its composition early, but merges its
   dev-only sandbox from `origin/dev` only against landed provider behavior and
   passes the product-behavior review gate.
6. **Independent verification:** run the issue-only clean-WSL2 matrix, route any
   defect to the owning repository as new scoped work, and close the umbrella
   only after all evidence is accepted.

Provider merge order is `rpg-api` then `game-dev` then web integration. The
`rpg-project` idea PR remains the review/tracking surface and does not merge
before implementation and clean-WSL verification complete.

## Explicit non-goals

- A generic scenario schema, catalog, registry, fixture framework, editor,
  template picker, watcher, or hot-reload daemon.
- More than one canvas template, stable dungeon key, or active sandbox scenario.
- Direct Redis writes, repository calls from the seeder, `Character.Data` or
  `MonsterData` construction, manual equipment-slot mutation, or reuse of
  `cmd/devseed`.
- Implementing `GetRequirements`, `SubmitChoices`, `ValidateDraft`, or new generic
  character-choice handlers.
- Proto changes, new services, a debug/version proto, or a second character
  creation path.
- Toolkit code changes for the MVP; Protection and the shield/equip rules already
  exist. A contributor's local toolkit edit is input to the loop, not delivery
  scope.
- Weakening auth, ownership, host, ready, or production gates; sharing one
  mutable global auth client between sandbox identities.
- Replacing normal `GameView` acceptance with `PlaytestHarness`.
- Native Windows setup, Blender/assets bootstrap, Discord production onboarding,
  deployment redesign, CI deployment, or global Pi/OpenCode configuration.
- Automatic branch switching, cleaning, stashing, resetting, or overwriting a
  contributor checkout.

## Residual questions

None block design approval. Exact implementation filenames, button styling, and
the existing toolkit-projected field used for the reversible local-only marker
remain implementation details; they may not widen the fixed behavior or add a
new contract.
