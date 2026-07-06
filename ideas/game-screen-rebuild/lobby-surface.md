# Lobby Surface: slice 1 of the game screen rebuild

## Status: Design — service split approved (Kirk, 2026-07-06), join-ref carrier taken as recommended default

Parent design: [design.md](design.md). This is the "design detail in its own plan doc"
that slice 1 called for.

## The Problem

The v1alpha2 encounter stack has no way for a party to assemble. `CreateEncounter` builds
a single-player encounter for the caller — no join, no ready-up, no character binding, no
membership events on the stream. The only multi-player v2 encounters that have ever
existed are devseed fixtures written straight to Redis, bypassing the RPC surface.
(Survey 2026-07-06: rpg-api-protos @ 371c451, rpg-api @ c682729.)

The v1alpha1 lobby is the design donor but not a port candidate: in-memory-only storage,
logic inline in the 5,844-line v1 orchestrator (the `lobby.go` its docs mention does not
exist), and only the first `character_id` in a request is ever bound.

## Decisions

1. **Separate `LobbyService`** in `dnd5e/api/v1alpha2` — not RPCs bolted onto
   `EncounterService` (Kirk, 2026-07-06). Rationale, in Kirk's frame: services version
   independently. The encounter service is where iteration churn lives and will need
   more versions; the lobby service can stay stable — or grow a feature — without
   forcing an encounter-service rewrite. It also keeps the encounter types a clean 1:1
   toolkit mirror: a lobby is pure API state, the toolkit has zero lobby concept (the
   boundary rule), and bolting a `WAITING` pseudo-mode onto `EncounterMode` would
   pollute an enum that maps straight onto toolkit state.
2. **One join mechanism, two carriers** (recommended default, taken without objection —
   overridable in review). `JoinLobby` takes an opaque `join_ref` minted at
   `CreateLobby`. In the Discord Activity, the instance supplies it automatically —
   everyone in the Activity is already in the same channel, nobody types a code. In
   dev/playtest, a URL param or displayed short code supplies the same ref — which is
   what keeps 4-browser MCP playtest joins trivial outside Discord. One system, not a
   Discord path plus a fallback path.
3. **Host-triggered start.** `StartEncounter` is called by the host after all members
   ready — a final go/no-go moment — rather than auto-start on last ready.
4. **Toolkit owns nothing here.** The lobby is orchestration: join refs, membership,
   ready flags, lifecycle. Clients send references (`character_id`, `join_ref`,
   `lobby_id`) and render pushed state — never calculations.

## Proposed Surface

`dnd5e/api/v1alpha2/lobby/service.proto` — six RPCs:

```
CreateLobby(campaign_id, character_id)      → {lobby_id, join_ref, host_player_id}
JoinLobby(join_ref, character_id)           → {lobby_id, members[]}
SetReady(lobby_id, ready)                   → {}          (broadcast on stream)
LeaveLobby(lobby_id)                        → {}          (pre-start only)
StartEncounter(lobby_id)                    → {encounter_id}   (host-only, all-ready gated)
StreamLobby(lobby_id)                       → stream LobbyEvent
```

```
LobbyMember { player_id, character_id, character_name, is_host, is_ready }
LobbyEvent  { snapshot { members[] }        // first event on subscribe, mirrors
            | member_joined { member }      //   StreamEncounter's snapshot-then-deltas
            | member_left { player_id }
            | member_ready { player_id, ready }
            | encounter_started { encounter_id } }
```

Lifecycle: `WAITING → STARTED`, terminal. On `encounter_started`, clients drop the lobby
stream and subscribe `StreamEncounter(encounter_id)` — the already-proven
snapshot-first flow. No lobby state survives the handoff.

Character binding happens at create/join: each player binds exactly one `character_id`,
and the server validates the character belongs to the authenticated player (the v1 lobby
never validated this). Party display data (`character_name`, later class/level) is
server-enriched into `LobbyMember` — the web renders it, never fetches-and-computes.

`StartEncounter` is where rpg-api constructs the toolkit encounter: loop
`enc.AddPlayer` once per ready member, seed HP per member from the character store
(generalizing today's single-caller create path), persist once to `enc:v2:<id>` in
Redis, emit `encounter_started`.

## Layering (do it right the first time)

The v2 create/hydration path currently lives in the handler package —
[rpg-api#616](https://github.com/KirkDiggler/rpg-api/issues/616)'s complaint. The lobby
implementation does NOT repeat that: handler → lobby orchestrator (Input/Output types,
gomock suites) → lobby repository (Redis, `lobby:` prefix, in-memory variant for tests),
outside-in per the rpg-api development pattern. `StartEncounter` is the natural seam to
also pull encounter construction into the orchestrator layer — whether #616's full
cleanup rides this wave or stays separate is a scheduling call at implementation, but
new lobby code starts layered.

## Discord-instance carrier (implementation note, not contract)

Players already authenticate individually (Discord OAuth → `player_id`); no
channel/Activity-instance scoping is threaded through rpg-api today, and
`CreateEncounterRequest.campaign_id` has been a validated-but-unused placeholder. The
instance→`join_ref` mapping is rpg-api internal (interceptor/context work), invisible to
the proto contract — the contract only knows `join_ref`. Dev carrier ships first (it's
what playtest needs); the Discord carrier lands with Activity integration.

## Verify (the boarded Party Assembles story)

"4 real clients in one v2 encounter": four dev-authenticated browsers, one creates, three
join via the dev carrier, all four ready up, host starts, all four land in the same
encounter and see each other on `StreamEncounter`. Driven end-to-end via MCP playtest;
needs a four-character devseed cast (parameterized fixture, per standing practice).

## Implementation order (issues filed at pickup, one per repo)

1. **rpg-api-protos**: `lobby/service.proto` + generated code.
2. **rpg-api**: outside-in — handler stubs (`codes.Unimplemented`) → orchestrator
   interfaces + mock suites → implementation → Redis repo. Dev join-ref carrier included.
3. **rpg-dnd5e-web**: `LobbyFlow` (create/join/ready/roster on the new service) — this is
   the front of slice 2's `GameView` in the parent design.

## Adjacent findings (not this slice's scope)

- rpg-api docs drift: `encounter-orchestrator.md` references a `lobby.go` that doesn't
  exist; v1 lobby docs describe a file layout that never was. Refresh rides the wave
  that touches those areas (docs close-the-loop).
- v1 lobby storage is in-memory only — one more reason the v1 path can't limp along and
  the clean cutover is right.

## Load-bearing digest

- Nothing on v1alpha2 can assemble a party today; devseed writing straight to Redis is
  the only multi-player path in existence.
- Separate `LobbyService` so the lobby versions independently of encounter churn and the
  encounter types stay a pure toolkit mirror.
- One join mechanism (`join_ref`), two carriers: Discord instance auto-supplies it,
  dev/playtest passes it explicitly — playtest verification never depends on Discord.
- `StartEncounter` is the lobby→encounter seam: build toolkit encounter per ready
  member, persist once, emit `encounter_started`, clients switch streams.
