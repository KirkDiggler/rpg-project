# Toolkit contributor onboarding and live sandbox loop

**Status:** approved — 2026-08-10

**Umbrella:** [rpg-project#208](https://github.com/KirkDiggler/rpg-project/issues/208)

**Scope:** one WSL2 contributor loop, one local toolkit module, one canvas template,
and one active scenario.

## Problem

A WSL2 contributor editing `rpg-toolkit/rulebooks/dnd5e` has no canonical real-game
loop. Bootstrap is broader than this job, the API override targets another module,
the legacy seeder writes storage records directly, and Dungeon Builder is not
composed with real authoring, lobby, and normal-game paths.

The target is Ubuntu WSL2 with Docker Desktop WSL integration. Contributors work
from WSL; the workflow diagnoses missing integration but never changes Windows,
Docker Desktop, or global developer tools.

## Outcome and boundaries

`game-dev` supplies one explicit loop: bootstrap, start, refresh, seed, status,
and down. A contributor edits only the local D&D 5e rulebook, explicitly refreshes
the API, seeds two real characters, saves one preloaded Dungeon Builder canvas,
and starts a one- or two-seat encounter that resumes in normal `GameView`.

The MVP fixes one checked-in template and the
`toolkit-contributor-sandbox` key, two Dev identities, and explicit refresh—not a
watcher, scenario schema/catalog/picker/importer, alternate editor, or generic
fixture framework.

The platform boundary remains unchanged:

```text
Client sends REFERENCES (keys, IDs) -> never calculations
API orchestrates by KEY            -> never knows what "rage" does
Toolkit implements RULES           -> returns rich breakdowns for rendering
```

The web sends intent and references; `rpg-api` authenticates, orchestrates, and
persists; the rulebook owns rules; and contributor tooling coordinates only.
Neither writes Redis, constructs `Character.Data`/`MonsterData`, or mutates
equipment slots. `cmd/devseed` is not reused.

## Contributor experience

### First checkout

From WSL, after installing/enabling WSL2, Docker Desktop WSL integration, Git,
GitHub SSH access, Go, Node/npm, `rsync`, and `jq`:

```bash
git clone git@github.com:KirkDiggler/game-dev.git ~/game-dev
cd ~/game-dev
./scripts/toolkit-contributor.sh bootstrap
```

`bootstrap` verifies prerequisites and clones only `rpg-toolkit`, `rpg-api`,
`rpg-dnd5e-web`, and `rpg-deployment` when absent. It leaves valid checkouts
unchanged; it never switches branches, cleans, stashes, resets, installs
Pi/OpenCode/assets tooling, copies configuration, or changes Docker/Windows.

### Daily loop

```bash
cd ~/game-dev
./scripts/toolkit-contributor.sh start
./scripts/toolkit-contributor.sh seed
./scripts/toolkit-contributor.sh status

cd rpg-dnd5e-web
npm run dev
```

`start` enables the approved local `rulebooks/dnd5e` override, builds
`rpg-api:local`, and starts the existing tracked deployment composition in this
exact overlay order:

```bash
docker compose \
  -f docker-compose.local-dev.yml \
  -f docker-compose.api.yml \
  -f docker-compose.local-api-src.yml \
  up -d
```

The first file supplies Dev auth/Envoy, the second authoring and writable content,
and the last `rpg-api:local`. Together they provide `AUTH_DEV_MODE=true`,
`RPG_AUTHORING_ENABLED=1`, and `RPG_CONTENT_DIR=/content`; no deployment change
is planned. `start` waits for Envoy gRPC health and prints the web command and URL.

The contributor opens:

```text
http://localhost:3001/?toolkitSandbox=1
```

After a toolkit edit:

```bash
cd ~/game-dev
./scripts/toolkit-contributor.sh refresh
./scripts/toolkit-contributor.sh seed # when character output is relevant
```

`refresh` is the only source refresh: validate/resync the module, build the local
image, replace the API only after success, wait for health, and report source
revision/sync time. Browser refresh/HMR does not refresh Go source.

`status` reports only the fixed override source/revision and compose/API health.
At session end, stop Vite and run:

```bash
cd ~/game-dev
./scripts/toolkit-contributor.sh down
```

`down` stops the stack and removes only the known D&D 5e replacement/tree. It
refuses a different or second replacement and leaves checkouts and edits intact.

## Ownership and safe local override

| Repository       | Owns in this delivery                                                                  |
| ---------------- | -------------------------------------------------------------------------------------- |
| `rpg-project`    | This design, later approved plan, umbrella tracking, and final clean-WSL2 verification |
| `game-dev`       | WSL2 command facade and contributor docs/command-contract tests                        |
| `rpg-api`        | Safe one-module override and `cmd/sandboxseed`, a real API client                      |
| `rpg-dnd5e-web`  | Dev-only sandbox composition and launch links                                          |
| `rpg-toolkit`    | Locally edited D&D 5e rules; no MVP change is required                                 |
| `rpg-api-protos` | Existing contracts; no proto change                                                    |
| `rpg-deployment` | The existing three compose files above; no planned change                              |

`game-dev` delegates `seed` to the API seeder and start/refresh to the API
override helper; it does not copy either implementation. Implementation issues
are created only after design and then plan approval.

The API helper supports the exact
`github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e` module while retaining its
one-module invariant:

1. `on` checks that the source `go.mod` declares that exact module and syncs only
   `rulebooks/dnd5e/` into an ignored API Docker build context.
2. It allowlists known targets but permits exactly one active `replace`; this
   facade always selects D&D 5e. A missing source, module mismatch, unknown or
   second replacement, or target switch without `off` fails before mutation.
3. `refresh` may resync only that selected target. `off` removes only the exact
   replacement and generated tree it owns. Release-pin/CI checks reject a
   committed local replacement.
4. The local image uses the dedicated local-toolkit Dockerfile. Normal and
   release builds keep published module pins.

The rulebook replacement applies transitively where the released encounter module
imports it; a second encounter replacement is neither required nor permitted.

## API seed and identity model

| Dev player ID               | Expected character                                   |
| --------------------------- | ---------------------------------------------------- |
| `toolkit-sandbox-fighter`   | `Toolkit Sandbox Fighter`, level-1 Human Fighter     |
| `toolkit-sandbox-barbarian` | `Toolkit Sandbox Barbarian`, level-1 Human Barbarian |

These local labels are not credentials. Every seeder RPC carries
`authorization: Dev <player-id>`; the server-derived identity, not a request
`player_id`, is authoritative.

`seed` is a pre-assembly reset. For each fixed identity: authenticate, call
`ListCharacters`, delete only IDs from that authenticated list, create its fixed
character through the sequence below, then re-list and require exactly one.
Interrupted runs repeat that reset; no arbitrary character ID is accepted.

The two presets use fixed, integration-tested generated enum values and fixed
choice/option values. There is no catalog discovery or generic choice interpreter:

```text
CreateDraft
  -> UpdateName
  -> UpdateRace (fixed toolkit-validated selection)
  -> UpdateClass (fixed skills, equipment, and fighting-style values)
  -> UpdateBackground (fixed toolkit-validated selection)
  -> UpdateAbilityScores
  -> GetDraft
  -> FinalizeDraft
  -> GetCharacter/ListCharacters
```

`GetRequirements`, `SubmitChoices`, and `ValidateDraft` handlers remain
unimplemented and out of scope. `Update*` and `FinalizeDraft` are authoritative:
if a local rulebook invalidates a fixture choice, that RPC fails and `seed` stops
with the identity and RPC error.

The Fighter uses `FIGHTING_STYLE_PROTECTION` and the fixed
martial-weapon-and-shield option (for example, `fighter-weapons-primary` /
`fighter-weapon-a`). Finalization puts the shield in inventory; the seeder calls
real `EquipItem` for its returned ID and off-hand slot, then verifies it. The
Barbarian uses fixed level-1 selections (including its weapon option); Rage is a
toolkit class grant.

The default address is Envoy at `localhost:8080`; an address flag may support a
host-run API, while the `game-dev` facade uses the healthy composed stack.

The seeder's character IDs come only from each identity's authenticated
`ListCharacters`. `CreateLobby` and `JoinLobby` independently enforce character
ownership. This MVP makes no broader claim about unrelated existing
character-ID RPCs.

## Web sandbox, authoring, and play

The sandbox is present only when `import.meta.env.MODE === 'development'` and
`?toolkitSandbox=1` are both present; production builds neither route to nor
render it. It reuses `DungeonBuilderConcept` with a sandbox-only current
`version: 1` canvas (explicit dimensions and placed monster references/coordinates)
and authoring-client override, ignoring the normal `create` local-storage draft.

Its one lifecycle is:

1. Open the checked-in populated canvas as an in-memory editable draft.
2. Edit with existing Dungeon Builder controls.
3. Preview through `PutDungeon(validate_only: true)`.
4. **Save and assemble** through `PutDungeon(validate_only: false)` under the
   stable `toolkit-contributor-sandbox` key.
5. Use that successfully saved key as the only available scenario.

Reloading begins from the template; saving overwrites its development key. There
is no autosaved catalog, additional template, importer/exporter, server-side
delete, or generic metadata.

The sandbox has a private helper returning only the bound unary authoring,
character, and lobby clients it uses. Each client has an immutable Dev player ID
and interceptor that emits its own `Dev` header without reading `setAuth`.
Focused tests prove interleaved unary calls retain their headers. Normal
`?playerId` `GameView` links retain their existing per-tab auth path.

After saving, the sandbox lists the single character for each dedicated identity.
The contributor selects Fighter, Barbarian, Fighter then Barbarian, or Barbarian
then Fighter. The first selected identity hosts with `CreateLobby`; the second,
when selected, joins with its own character and returned `join_ref`; each selected
identity calls `SetReady`; only the host calls `StartEncounter` using the saved
key. Server host, readiness, party-size, and character-ownership checks remain
authoritative. A wrong-owner lobby bind is a required negative test.

After a start, show normal links only for selected identities:

```text
http://localhost:3001/?playerId=toolkit-sandbox-fighter
http://localhost:3001/?playerId=toolkit-sandbox-barbarian
```

Those links follow the existing `GetMyActiveLobby`/resume route to `GameView`.
Optional `?encounterId=...&playerId=...` harness links may aid diagnosis but never
satisfy acceptance.

On any sandbox RPC failure, show the RPC error and do not continue. This MVP adds
no retry/cleanup state machine, arbitrary record deletion, or direct Redis path.

## Security and production gates

1. Dev authentication is accepted only when the local contributor start path
   explicitly enables existing server DevMode. A production-mode server rejects
   `Dev` headers.
2. The sandbox UI and its Dev-bound clients are development-only. Production
   query strings cannot activate them.
3. The fixed labels are not secrets or production accounts, and no token is
   logged. The seeder is scoped to authenticated lists and never accepts an
   arbitrary delete target.
4. The local override remains in ignored build context, is rejected by
   release-pin checks if committed, and is absent from normal/release images.
5. This MVP does not claim deployed authoring is local-only or already
   authorization-protected. The existing production composition enables
   authoring with persistent content, and the current `PutDungeon` path has no
   author-role/ownership check. Production authoring lockdown is a prerequisite
   for any production authoring use; it is not a sandbox deployment change.

## Delivery and verification

This is the design gate. `plan.md` is added to this same still-open PR only after
design approval; implementation does not start before that gate. The later work
is one API provider wave from `origin/dev`, then the `game-dev` facade from
`origin/main`, then dev-only web composition from `origin/dev`. The idea PR stays
open through implementation and clean-WSL2 verification.

Focused automated evidence covers:

- API override target/module validation, one-replace rejection, refresh, exact
  cleanup, fixed seeder RPCs/metadata/reset, real integration behavior, and the
  Fighter's post-finalize `EquipItem`;
- `game-dev` prerequisite/bootstrap convergence, dirty-checkout preservation,
  delegated commands, and owned `down`; and
- web development/production gate, template isolation, real `PutDungeon`,
  unary per-identity headers, lobby order/ownership behavior, and normal links.

One clean supported WSL2 run proves the following, retaining concise command
output where useful:

| Case              | Required proof                                                                                                                                                                  |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Bootstrap         | Initial bootstrap succeeds and its rerun is a no-op with the four required repositories.                                                                                        |
| Start and refresh | The three-file stack is healthy through Envoy; a reversible local rulebook marker appears through the running API only after refresh and disappears after removal plus refresh. |
| Seed              | Two seed runs leave exactly the two expected characters, one per identity; the Fighter has Protection and a real equipped shield.                                               |
| Authoring         | The populated template saves through `PutDungeon` under the stable key.                                                                                                         |
| Play              | Fighter, Barbarian, Fighter → Barbarian, and Barbarian → Fighter all reach normal `GameView`.                                                                                   |
| Gates and down    | Automated negatives reject a wrong-owner bind and production `Dev`/sandbox access; `down` stops the stack and removes only the owned replacement/tree.                          |

Detailed error and cleanup assertions belong in those focused tests, not in a
separate workspace auditor or cleanup workflow. The deterministic local-marker
proof must use an existing stable API projection; it may not add a debug proto or
an API-only version value.

## Non-goals and residual detail

Out of scope are toolkit/proto changes, deployment redesign, direct storage or
character-data construction, `GetRequirements`/`SubmitChoices`/`ValidateDraft`
implementation, generic choices or scenarios, extra templates/scenarios,
watchers, mutable shared sandbox auth, replacement of normal `GameView` with the
harness, native-Windows/assets/Discord production setup, and automatic checkout
mutation.

Implementation may choose exact filenames, button styling, and the existing
stable projected field for the reversible marker. Those details must not widen
the fixed behavior or introduce a new contract.
