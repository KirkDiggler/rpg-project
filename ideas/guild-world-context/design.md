# Trusted guild-derived world context — composition first

*Design proposal for [rpg-project#399](https://github.com/KirkDiggler/rpg-project/issues/399), under journey [#169](https://github.com/KirkDiggler/rpg-project/issues/169). No implementation is authorized by this document.*

## Outcome and fixed boundary

The game has outgrown a single server-wide project. A Discord guild member should reach that guild's one world through a reusable trusted world-context boundary. The first and only deployed consumer in this slice is `CompositionService`, which makes the World Builder and composition resolution available outside local development.

The product decisions already made are:

- one guild has one world; joining the guild grants access to it;
- the guild owner pays the bills, but billing is not part of this work;
- membership is the current access policy. There is no owner, administrator, role, or Discord permission requirement now. A later policy check belongs at each handler door;
- `PlayerID` and current request `WorldID` are different identities. A future character may have a home world and visit another; this slice adds neither character ownership nor travel;
- no world picker, multiple worlds per guild, regions framework, world registry, revisions/heads, Lua, idempotency system, all-RPC migration, or production dev authentication; and
- toolkit composition `Data` remains the straightforward world-scoped value it is today. Discord concepts stop at the API boundary.

## Current seam

Today the web authorizes with `identify` and `applications.commands`, explicitly using `prompt: 'none'`; it sends only `authorization: Discord <token>` on RPCs ([web source](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/discord/DiscordProvider.tsx#L58-L66), [transport](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/api/client.ts#L23-L45)). The game server validates that token with `/users/@me` and retains only `PlayerID` in context ([Discord client](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/auth/discord.go#L66-L97), [interceptor](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/auth/interceptor.go#L101-L145)). This proves user identity, not guild membership.

The Activity's `guild_id` is currently URL-derived debug context and never reaches the API ([SDK context](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/discord/sdk.ts#L54-L67)). Production has no composition source; development alone selects `VITE_DEV_WORLD_ID` or `test-world` ([source factory](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/compositions/rpcCompositionSource.ts#L85-L98)). The game server similarly registers `CompositionService` only in auth dev mode with one configured world ([registration](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/cmd/server/composition.go#L32-L80)).

The existing proto already carries `world_id` on each composition request and result ([contract](https://github.com/KirkDiggler/rpg-api-protos/blob/d95769dca9005cdbd3693dc9f7310cbefec1b7d4/api/composition/v1alpha1/service.proto#L9-L63)). Redis is already partitioned as `composition:<WorldID>` and validates the stored envelope ([repository](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/repositories/composition/redis.go#L68-L73), [decode check](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/repositories/composition/redis.go#L155-L167)). The missing part is the trusted derivation of that ID.

## Proposed contract for approval

### 1. Deterministic one-to-one identity, not a registry

For this slice, define the toolkit-domain `WorldID` string to be the canonical decimal Discord GuildID:

```text
verified GuildID "123456789012345678" -> WorldID "123456789012345678"
```

This is an **implementation proposal requiring design approval**, not a previously approved registry decision. It is the smallest exact one-to-one mapping: no database, lifecycle service, mapping roundtrip, or empty-library bootstrap problem. The Activity already knows its guild selector, so it can set both the guild header and existing request `world_id`; the server independently verifies and derives the same value. Client agreement is never authority.

The tradeoff is explicit. Direct identity puts a Discord-shaped value into otherwise platform-neutral WorldID storage. A deterministic namespace such as `discord:guild:<id>` would avoid cross-provider collisions while retaining no-registry behavior, but would add a mapping rule that every client must share. A separate random opaque WorldID would improve provider independence, but would require durable mapping/lifecycle and a resolve response even when `ListCompositions` is empty. There is no present need for that machinery. If later evidence earns it, the migration should be designed then; toolkit code must still know only WorldID, never GuildID or Discord APIs.

### 2. One untrusted transport selector

World-scoped calls carry exactly one metadata field:

```http
authorization: Discord <user access token>
x-rpg-guild-id: 123456789012345678
```

`x-rpg-guild-id` is a selector, not a claim. Reject a missing, repeated, empty, non-canonical, or non-snowflake value before calling a handler. Do not accept guild identity from a request body, user ID, `Referer`, channel, instance, or participant list as authority.

The global authentication interceptor continues to serve all authenticated RPCs without requiring a guild. A second, method-scoped world-context interceptor/resolver opts in only `CompositionService` initially. Future RPCs can join that allowlist without changing the transport contract; global identity APIs such as character creation/listing do not acquire a guild requirement today.

The auth layer must keep the same request's Discord user token available to the internal resolver even when `/users/@me` was an identity-cache hit. It must not expose that credential to handlers, service inputs, logs, or responses. The resolver calls:

```http
GET /api/users/@me/guilds/{selected_guild_id}/member
Authorization: Bearer <the same authenticated user's token>
```

Discord documents this user-token endpoint under `guilds.members.read`; it is distinct from bot member lookup ([OAuth scopes](https://docs.discord.com/developers/topics/oauth2#shared-resources-oauth2-scopes), [Get Current User Guild Member](https://docs.discord.com/developers/resources/user#get-current-user-guild-member)). Success proves current membership for that token and selector. The API converts the verified GuildID to WorldID and installs only trusted API-owned context for the handler, conceptually:

```go
type WorldContext struct {
    WorldID string // toolkit-domain identity; no Discord type below this seam
}
```

The handler reads `PlayerID` and this trusted WorldID. It never derives authority from request `world_id`.

### 3. Handler and storage invariants

Every composition method must require `request.world_id == trusted WorldID`. Missing or mismatched values fail before service or repository access. Service inputs use the trusted value, not the request value. Creates stamp it onto `composition.Data`; reads/lists must return only records whose stored `WorldID` matches it. A stored or returned mismatch is a server integrity failure, never data to show the caller.

Membership permits `Get` and `List`. `Create` and `Delete` require both membership and the existing explicit server authoring enablement. They add no guild-owner/admin/role requirement and no per-record creator ACL. Delete remains able to delete any named composition in the current world when authoring is enabled.

This preserves the current simple `composition:<WorldID>` Redis partition and toolkit `composition.Data`. It adds no revision, head, creator, ACL, or idempotency fields.

## Web source and consent lifecycle

The Activity's SDK guild value creates an untrusted candidate source; the successful server check makes its requests usable. With the proposed direct identity, `CompositionSource.worldId` is the canonical guild snowflake. Its adapter sends that value in every composition request while the transport sends the guild selector. The existing response checks remain defense in depth.

A source is scoped to **authenticated token session + GuildID + WorldID**. When the Activity guild changes, authentication changes, re-consent occurs, or the user signs out, discard the entire source object before creating another. Browser sign-out also clears local authentication; this source, state, and cache teardown is client-local. Clear list, palette, composition-resolution, error, and in-flight state; late results from an old source must not publish into the new one. Cache keys remain at least `(source identity, WorldID, composition ID)`, as the existing resolution hook already intends ([cache reset](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/compositions/useCompositionResolutions.ts#L39-L71), [late-result guard](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/758c3b16a570531c215f677ae928bd572c227841/src/compositions/useCompositionResolutions.ts#L80-L110)). A composition reference is always resolved through the current source's WorldID; an ID learned in one world must never probe or populate another world's cache.

If there is no Activity guild, production creates no world source and makes no composition call. The World Builder entry is hidden or disabled with a clear “Open this Activity in a server to access its world” state. It must not choose the first guild, offer a picker, or fall back to `test-world`. Other global signed-in UI may continue because it has not opted into world context.

The SDK authorize request adds `guilds.members.read`. Existing users have tokens granted only `identify` and `applications.commands`; current `prompt: 'none'` cannot be assumed to upgrade consent interactively. **Proposal:** remove the forced `prompt: 'none'` for this authorization path and let Discord present required consent. A cancelled or denied grant clears auth and world-source state and shows a retry/reconnect action; it does not continue with an old token or stale world.

The SDK's returned scopes may improve the UI message, but they are not server authority. The deployed proof must exercise the real consent screen and endpoint response.

## Failure and cache policy

The proposed external behavior is deliberately fail-closed:

| Condition | Server result | UI behavior |
|---|---|---|
| no/invalid/expired token, or Discord membership call returns `401` | `Unauthenticated`; evict identity and membership entries | clear auth and world source; reconnect |
| no guild context | `FailedPrecondition` if a call is attempted | no source; explain that a server launch is required |
| malformed/repeated guild selector or missing body `world_id` | `InvalidArgument` | do not retry automatically |
| body/header-derived WorldID mismatch, verified non-member, or Discord member endpoint `403`/`404` | `PermissionDenied` | no library data; reconnect when known scopes omit `guilds.members.read`, otherwise explain no access |
| Discord timeout, `429`, `5xx`, or unusable response | `Unavailable` | retry action; no cached-expired or `test-world` fallback |
| authoring gate off on Create/Delete | existing authoring-disabled result | reads may remain available; show authoring unavailable |

Do not invent a Discord error distinction the provider did not send. In particular, a `403` can be surfaced as missing scope only when the authenticated scope list actually proves that; otherwise it remains a generic access denial.

**Cache proposal for approval:** cache successful `(token digest, GuildID) -> PlayerID/WorldID` membership decisions in process for at most 30 seconds and a fixed maximum entry count. Key by a non-reversible digest rather than a raw bearer token, never log keys, never cache denials, and never serve an expired entry. Evict relevant identity and membership entries when a provider `401` is observed. Browser sign-out sends no server signal and is not token revocation, so it does not synchronously evict server entries; positive decisions in this proposed cache instead expire within the bounded 30-second TTL. This slice adds no logout RPC or logout framework. Once the positive entry expires, provider failure is `Unavailable`; there is no stale-while-error behavior. This intentionally accepts at most the short TTL of membership/revocation lag. Starting without a membership cache is also correct but adds a Discord roundtrip to every composition RPC.

## Development-only world

Keep the existing local test path explicit and separate:

- a Vite **development** build may create the source from `VITE_DEV_WORLD_ID` (default `test-world`);
- the game server may trust that configured value only when `AUTH_DEV_MODE` permits the `Dev` scheme;
- body `world_id` must still match the configured dev world; and
- production neither accepts `Dev` auth nor derives a world from `RPG_DEV_WORLD_ID`, a missing guild, or `test-world`.

This is a test seam, not a production fallback and not evidence that Discord OAuth works.

## Deployment seam

The browser must be allowed to send `x-rpg-guild-id` through the Activity proxy. Envoy's current CORS `allow_headers` explicitly lists `authorization` but not the new field ([configuration](https://github.com/KirkDiggler/rpg-deployment/blob/7fc5ee8ee6d99db066fabc35d5b1974bd5778719/envoy/envoy.yaml#L40-L46)); implementation must add it and verify the actual preflight. Nginx must preserve the client header through the live HTTP route and its SSL twin. Neither proxy validates, overwrites, or manufactures the selector—the API remains the trust boundary.

No bot token or privileged Server Members Intent is needed for this user-token current-member endpoint. Request `guilds.members.read` in SDK `authorize`. Do not invent a Developer Portal permissions toggle: attempt real OAuth against the actual application, capture any Discord error code/message, and change portal/application restrictions only when that evidence identifies a real requirement.

## What this does not attest

Successful current-member lookup proves that the authenticated user belongs to the selected guild. It does **not** attest that the RPC originated from the selected guild's currently active Activity instance or channel. SDK/query `guild_id`, `channel_id`, `instance_id`, participants, and `Referer` are not signed origin evidence in this design. That is acceptable for the agreed “join guild to access world” policy, including a member accessing their guild world from another client context. If active-instance confinement becomes policy, it needs a separate Discord-backed attestation design; membership must not be relabeled as that proof.

## Acceptance proof for the first slice

Automated tests should cover resolver status mapping, same-token use, selector validation, body/context mismatch refusal before repository access, authoring enablement, response envelope checks, and source/cache teardown. The release proof must additionally use the deployed normal OAuth path:

1. A user with the old two scopes sees and completes the real `guilds.members.read` consent upgrade in a server Activity.
2. A member opens an empty library (proving WorldID discovery needs no record), creates a composition with authoring enabled, lists/gets it, uses its ref in preview/play rendering, and deletes it.
3. Missing guild, missing scope, cancelled consent, non-member guild, expired token/Discord `401`, provider unavailability, and a hostile altered guild header all fail closed with the behavior above.
4. Hostile header/body mismatches and wrong-world composition IDs cannot read, write, delete, resolve, or warm cache entries in either world.
5. Switching guild/source and signing out while reads are in flight leaves no prior-world palette, ref resolution, or cache result visible.
6. Proxy preflight and forwarding are observed in the deployed Activity; production rejects `Dev` auth and never reaches `test-world`.

Local mocks and dev-world tests support this proof but do not substitute for Discord consent, the real current-member endpoint, proxy forwarding, and the deployed normal-OAuth walkthrough.

## Ordered future migration, not current guarantees

The metadata/context contract is reusable, but this slice does not secure every RPC. Migrate only when each resource becomes world-owned:

1. composition reads/writes and deployed World Builder (this slice);
2. authored dungeon registry/storage, partitioning keys and files by WorldID before calling it guild-scoped;
3. lobby creation, stamping trusted WorldID; lobby join, loading the lobby then requiring its stored WorldID to equal the caller's trusted request world;
4. session/encounter launch, copying the lobby's WorldID into persisted session resources; then gameplay handlers; and
5. any other world-owned resource, always stamping creates from trusted context and requiring loaded `resource.WorldID == request WorldContext.WorldID` before read or mutation.

Global player identity and character APIs remain outside that order until their own product slice. A future character home WorldID must remain separate from the current request WorldID so visiting can be added without rewriting identity; no visit/travel behavior is implemented here.

The current authored dungeon registry is one process-wide `RPG_CONTENT_DIR` ([registry](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/dungeons/registry.go#L1-L21)). Lobby data has no WorldID ([data](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/repositories/lobby/repository.go#L48-L64)), and session persistence stores opaque toolkit blobs without an API world envelope ([repositories](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/orchestrators/session/redis_repos.go#L18-L30)). Therefore this design makes **no** current guarantee that dungeon keys, lobbies, sessions, join refs, or gameplay are world-isolated. It also adds no lobby/session WorldID, no guild partition to the registry, and no travel or cross-guild join policy.

## Adjacent security observations (not scope expansion)

The current OAuth request uses an empty `state`, while Discord recommends a session-bound state for CSRF protection ([Discord OAuth security guidance](https://docs.discord.com/developers/topics/oauth2#state-and-security)). The current game-server identity cache also uses raw access tokens as map keys for five minutes ([cache](https://github.com/KirkDiggler/rpg-api/blob/c206c568b2af27bdc6181f05e3be7478444b5c6b/internal/auth/cache.go#L14-L60)). Both deserve separate hardening. They do not justify turning this slice into a broad OAuth, session, secret, or deployment rewrite; the new membership cache must simply avoid repeating the raw-token-key pattern.

---

**Human approval gate:** Kirk must approve this written design—including the direct GuildID/WorldID identity, `x-rpg-guild-id` contract, error mapping, and cache choice—before `plan.md` or any implementation slice is created.
