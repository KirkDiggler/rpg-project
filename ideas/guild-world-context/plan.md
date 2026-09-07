# Trusted Guild-Derived World Context Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `CompositionService` and the World Builder available in a normal Discord Activity only when the same authenticated user token proves membership in the selected guild, with that canonical GuildID serving as WorldID.

**Architecture:** Keep global authentication unchanged in scope: it establishes only `PlayerID` and privately retains the request's parsed auth scheme/credential for the next interceptor. A unary interceptor allowlisted to the four `CompositionService` methods validates `x-rpg-guild-id`, resolves either the explicit Dev world or Discord membership, and installs trusted API-owned world context; handlers compare the request body's existing `world_id` to that context and never treat the body as authority. The web creates one composition source per Discord credential session and SDK GuildID, while deployment only permits and forwards the selector.

**Tech Stack:** Go, gRPC unary interceptors, `net/http`, bounded in-process cache, Redis composition repository; React 19, TypeScript, Connect-Web, Vitest/Testing Library, Discord Embedded App SDK 2.5.0; Envoy, nginx, Bash contract tests.

**Spec:** `ideas/guild-world-context/design.md` (approved at `2ad5f9c515fecb16496384fa0f05009e45612e38`; [receipt](https://github.com/KirkDiggler/rpg-project/pull/402#issuecomment-5571604359))

## Global Constraints

- This is the composition-first slice only. Global player/character auth stays unguilded; authored dungeon, lobby, session, encounter, and gameplay ownership remain untouched.
- `GuildID == WorldID`: use the canonical non-zero unsigned decimal Discord snowflake, with no prefix, registry, picker, or fallback mapping.
- `x-rpg-guild-id` is the only world transport selector. Discord requests require exactly one value; missing, empty, repeated, leading-zero, signed, whitespace-bearing, non-decimal, zero, or `uint64`-overflow values fail before the handler.
- The resolver must call `GET /api/users/@me/guilds/{guild_id}/member` with the exact access token authenticated on the same request, including when `/users/@me` identity was a cache hit. Credentials stay private to auth code and never enter handler/service inputs, responses, source identity strings, logs, or test artifacts.
- Membership alone permits Get/List. Create/Delete additionally retain the existing `RPG_AUTHORING_ENABLED=1` gate. Add no owner, admin, role, Discord permission, creator ACL, or per-record ACL check.
- Cache only successful membership decisions for at most 30 seconds and at most 1,024 entries, keyed by `(SHA-256 token digest, GuildID)`. Never cache denial, serve expired entries, use stale-on-error, log digests, store raw tokens in the new cache, or add a duplicate raw-token eviction index.
- A membership-provider `401` evicts that token's existing identity-cache entry and all digest-keyed membership entries even when identity authentication was a cache hit. The older identity cache otherwise remains unchanged; its key/bounds redesign is deferred to [rpg-api#937](https://github.com/KirkDiggler/rpg-api/issues/937).
- OAuth transaction/session binding remains deferred to [rpg-project#403](https://github.com/KirkDiggler/rpg-project/issues/403). This slice only requests the approved scope and uses valid SDK consent behavior.
- A Vite development build may use `VITE_DEV_WORLD_ID` (default `test-world`); only an API with `AUTH_DEV_MODE=true` may accept `Dev`. Production never accepts `Dev`, reads `RPG_DEV_WORLD_ID`, or falls back to `test-world`.
- The current composition proto and toolkit `composition.Data` already carry the required WorldID. Do not change rpg-api-protos or rpg-toolkit absent a concrete contract failure discovered by an existing consumer test.
- The existing owned stack is down by request. Do not start it during implementation. Automated tests use unit processes/static configuration only; real Discord consent and deployed proxy proof require a coordinated user test.
- Create one execution issue in each owning repo before cutting code branches. Use one branch for the whole repo wave: API/web from fresh `origin/dev`, deployment from fresh `origin/main`; do not create per-finding branches.
- Parent orchestration delegates implementation through native agents and independently reviews exact heads. Kirk performs merges. No worker pushes, opens PRs, changes boards, deploys, accesses live auth/production, or records tokens/secrets unless Kirk separately authorizes that operation.

---

## File and interface map

Baseline sources were read without checkout from `rpg-api@c206c568b2af27bdc6181f05e3be7478444b5c6b` (`origin/dev`), `rpg-dnd5e-web@758c3b16a570531c215f677ae928bd572c227841` (`origin/dev`), and `rpg-deployment@7fc5ee8ee6d99db066fabc35d5b1974bd5778719` (`origin/main`).

### rpg-api

**Create**

- `internal/worldcontext/context.go` and `context_test.go` — trusted `Value{WorldID string}` context carrier with `With(context.Context, Value)` and `Get(context.Context) (Value, bool)`; no credential field.
- `internal/auth/membership_cache.go` and `membership_cache_test.go` — 30-second/1,024-entry positive cache; key contains only `[32]byte` token digest plus GuildID; deterministic injected clock and oldest-expiry eviction.
- `internal/auth/world_resolver.go` and `world_resolver_test.go` — Discord/Dev resolution, status mapping, cache use, and dual-cache invalidation on provider `401`.
- `internal/auth/world_interceptor.go` and `world_interceptor_test.go` — exact `CompositionService` unary allowlist and strict selector validation.

**Modify**

- `internal/auth/context.go`, `context_test.go` — add an unexported request-auth value containing parsed scheme and credential; `GetPlayerID` remains the only public principal accessor.
- `internal/auth/interceptor.go`, `interceptor_test.go` — install private request-auth data on both identity-cache hit and miss and on Dev auth; do not add a guild requirement here.
- `internal/auth/cache.go`, `cache_test.go` — add only `Delete(token string)` for observed provider-401 invalidation; leave raw-key hardening/bounds to #937.
- `internal/auth/discord.go`, `discord_test.go`, `errors.go`, `mock/mock_discord.go` — add current-user guild-member call and precise `401`, `403/404`, and unavailable (`429`, timeout, `5xx`, malformed/unusable response) outcomes; generated mock is committed.
- `cmd/server/server.go` — construct the membership cache/resolver and chain `UnaryWorldContextInterceptor` after global auth and before logging; stream auth remains unchanged.
- `cmd/server/composition.go`, `composition_test.go` — always register `CompositionService`; remove the handler's configured singleton world while continuing to pass `RPG_AUTHORING_ENABLED`; `RPG_DEV_WORLD_ID` configures only the Dev resolver.
- `internal/handlers/api/composition/v1alpha1/handler.go`, `handler_test.go`, `integration_test.go` — consume trusted world context, compare every body `world_id`, pass only trusted WorldID, preserve authoring gates, and refuse wrong-world service output.
- `docs/architecture/components/auth.md`, `docs/status.md`, `docs/quality.md` — replace dev-only composition claims with the verified resolver boundary and explicitly retain #937 as deferred.

**Verified unchanged**

- `internal/services/composition/service.go` already separates `PlayerID` and `WorldID`.
- `internal/orchestrators/composition/orchestrator.go` already stamps creates from service input.
- `internal/repositories/composition/redis.go` already partitions `composition:<WorldID>` and rejects stored envelope WorldID/ID mismatches on Create/Get/List. Its focused tests remain part of the gate.

**Exact new interfaces**

```go
// internal/worldcontext/context.go
type Value struct { WorldID string }
func With(ctx context.Context, value Value) context.Context
func Get(ctx context.Context) (Value, bool)

// internal/auth/discord.go
type GetCurrentUserGuildMemberInput struct { Token, GuildID string }
type DiscordGuildMember struct { User *DiscordUser `json:"user"` }
type MembershipVerifier interface {
    GetCurrentUserGuildMember(context.Context, *GetCurrentUserGuildMemberInput) (*DiscordGuildMember, error)
}

// internal/auth/membership_cache.go
type MembershipCacheConfig struct {
    TTL time.Duration
    MaxEntries int
    Now func() time.Time
}
type MembershipDecision struct { PlayerID, WorldID string }
func NewMembershipCache(*MembershipCacheConfig) (*MembershipCache, error)
func (c *MembershipCache) Get(token, guildID string) (MembershipDecision, bool)
func (c *MembershipCache) Set(token, guildID string, decision MembershipDecision)
func (c *MembershipCache) DeleteToken(token string)

// internal/auth/world_resolver.go
type ResolveWorldInput struct { GuildID string }
type ResolveWorldOutput struct { WorldID string }
type WorldResolver interface {
    Resolve(context.Context, *ResolveWorldInput) (*ResolveWorldOutput, error)
}
type WorldResolverConfig struct {
    MembershipVerifier MembershipVerifier
    IdentityCache *TokenCache
    MembershipCache *MembershipCache
    DevWorldID string
}
func NewWorldResolver(*WorldResolverConfig) (WorldResolver, error)
func UnaryWorldContextInterceptor(WorldResolver) grpc.UnaryServerInterceptor
```

`MembershipCache.Get/Set/DeleteToken` accept the raw token only long enough to digest it and store no raw value. `WorldResolverConfig` contains the verifier, existing identity cache, new membership cache, and Dev WorldID. Resolver output never contains GuildID-specific or credential data below the auth seam; direct identity means its `WorldID` string equals the verified GuildID.

### rpg-dnd5e-web

**Create**

- `src/discord/DiscordProvider.test.tsx` — SDK GuildID, consent scope/prompt, cancel/denial teardown, and opaque auth-session epoch.
- `src/api/auth.test.ts` — token/player/guild set/clear behavior without exposing or logging the token.

**Modify**

- `src/discord/DiscordProvider.tsx`, `types.ts` — request `guilds.members.read`, omit `prompt`, use `discordSdk.guildId`, retain returned scopes, expose an opaque `authSessionId` and `clearAuthentication(message?)`, and remove logging of the SDK auth object.
- `src/api/auth.ts` — keep the in-memory Discord token private while storing/clearing its selected GuildID; add `getGuildId()` for the transport interceptor.
- `src/api/client.ts`, `client.test.ts` — set `x-rpg-guild-id` only on `CompositionService` requests using Discord auth; Dev and all global services remain header-free.
- `src/compositions/rpcCompositionSource.ts`, `rpcCompositionSource.test.ts` — construct a production source only for authenticated canonical SDK GuildID; preserve the explicit development source; invoke the supplied auth-clear callback on `Unauthenticated`; retain response WorldID/ID checks.
- `src/compositions/useCompositionResolutions.test.tsx`, `src/author/Palette.compositions.test.tsx` — extend the existing source/world late-result and list-reset proof to credential-session and guild switches.
- `src/compositions/compositionSource.ts`, `src/compositions/useCompositionResolutions.ts` — verified unchanged unless the new tests expose a concrete lifecycle gap; their current effect cleanup/source-object guards already isolate list and resolution publication.
- `src/App.tsx`, `App.test.tsx` — replace the dev-only source effect with source creation keyed by `(mode, authSessionId, SDK GuildID)`; clear the source before replacement; show a disabled no-guild World Builder state and consent/reconnect errors while leaving global home/character UI usable.
- `docs/architecture/components/discord.md`, `docs/status.md`, `docs/quality.md` — document normal OAuth world access and the limits of membership proof.

**Exact changed web contracts**

```ts
// src/discord/types.ts additions
interface DiscordContextType {
  readonly grantedScopes: readonly string[];
  readonly authSessionId: number;
  clearAuthentication(message?: string): void;
}

// src/api/auth.ts
export function setAuth(token: string | null, playerId: string | null, guildId?: string | null): void;
export function getGuildId(): string | null;
export function clearAuth(): void;

// src/compositions/rpcCompositionSource.ts
interface RpcCompositionSourceInput {
  mode: string;
  devWorldId?: string;
  guildId: string | null;
  authenticated: boolean;
  onUnauthenticated: () => void;
  client?: CompositionRpcClient;
}
export function createRpcCompositionSource(input: RpcCompositionSourceInput): CompositionSource | undefined;
```

SDK evidence: the installed lock resolves `@discord/embedded-app-sdk` 2.5.0. Its `output/commands/authorize.d.ts` permits only `prompt?: 'none'` and states that omitting it opens an OAuth modal when no token covers every requested scope; `schema/types.d.ts` includes `guilds.members.read`; `Discord.d.ts` exposes `readonly guildId`; and `commands/authenticate.d.ts` returns granted `scopes`. Therefore the implementation must omit `prompt`—not invent `prompt: 'consent'`—and verify returned scopes for UI messaging only.

### rpg-deployment

**Create**

- `tests/guild-world-header-contract.sh` — static assertions for every Envoy allow-list and the live/twin nginx forwarding blocks; it starts no container.

**Modify**

- `envoy/envoy.yaml`, `envoy/envoy-lab.yaml`, `envoy/envoy-lab2.yaml` — add lowercase `x-rpg-guild-id` to `allow_headers` and keep the synchronized configs aligned.
- `nginx/nginx-http.conf`, `nginx/nginx-ssl.conf` — explicitly preserve `X-Rpg-Guild-Id` in the production gRPC-Web regex location; do not validate, overwrite with a server value, or derive it.
- `Makefile` — include the static contract in `make test`.

`docker-compose.prod.yml` is unchanged: production already enables `RPG_AUTHORING_ENABLED=1` and durable `RPG_CONTENT_DIR`; no role or Discord permission gate is added.

---

### Task 1: Create execution issues and one branch per owning repo

**Files:** none yet.

**Interfaces:**
- Consumes: approved design/plan URLs and owning-repo base rules.
- Produces: three board-linked issue numbers and exactly three fresh branches, recorded by the parent before delegation.

- [ ] **Step 1: Create the three execution issues before any code branch**

Create these issue titles/scopes and add each to Project 19 with the appropriate owning team/area:

1. rpg-api — **Authorize CompositionService with trusted Discord guild world context**: API resolver/cache/context/handler/registration only; explicitly exclude #937.
2. rpg-dnd5e-web — **Bind production composition source to a Discord guild credential session**: scope consent, header, source lifecycle, and UI only; explicitly exclude #403.
3. rpg-deployment — **Pass the guild world selector through the Activity proxy**: Envoy CORS plus nginx forwarding/static proof only.

Each issue links `rpg-project#399`, PR #402, the design, and this plan. Record each resulting integer as `API_ISSUE`, `WEB_ISSUE`, or `DEPLOY_ISSUE`; these are execution outputs, not guessed numbers.

- [ ] **Step 2: Fetch and cut one fresh branch in each repo**

```bash
git -C /home/kirk/game-dev/rpg-api fetch origin
git -C /home/kirk/game-dev/rpg-api switch -c "feat/${API_ISSUE}-guild-world-context" origin/dev

git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web switch -c "feat/${WEB_ISSUE}-guild-world-context" origin/dev

git -C /home/kirk/game-dev/rpg-deployment fetch origin
git -C /home/kirk/game-dev/rpg-deployment switch -c "feat/${DEPLOY_ISSUE}-guild-world-header" origin/main
```

Expected: each `git status --short --branch` names only its new branch and has no tracked/untracked implementation files. Do not reuse the long-lived checkouts if occupied; create issue-named worktrees from the same fresh remote refs instead.

- [ ] **Step 3: Parent records delegation boundary**

Record the three starting SHAs and assignments. Workers may commit on their assigned branch but do not push/open PRs/deploy; the parent independently reviews each exact resulting head and Kirk merges.

### Task 2: Resolve trusted world context in rpg-api

**Files:**
- Create: `internal/worldcontext/{context.go,context_test.go}`
- Create: `internal/auth/{membership_cache.go,membership_cache_test.go,world_resolver.go,world_resolver_test.go,world_interceptor.go,world_interceptor_test.go}`
- Modify: `internal/auth/{context.go,context_test.go,interceptor.go,interceptor_test.go,cache.go,cache_test.go,discord.go,discord_test.go,errors.go,mock/mock_discord.go}`
- Modify: `cmd/server/server.go`

**Interfaces:**
- Consumes: the global interceptor's authenticated `PlayerID` and same-request private scheme/credential.
- Produces: `worldcontext.Value{WorldID: verifiedGuildID}` only on Composition unary calls, plus the interfaces listed in the file map.

- [ ] **Step 1: Write focused failing resolver/interceptor/client/cache tests**

Include table cases for missing header → `FailedPrecondition`; repeated/empty/noncanonical/overflow → `InvalidArgument`; Discord 200 with matching member user → context; `403/404` → `PermissionDenied`; timeout/`429`/`5xx`/bad JSON or missing/mismatched member user → `Unavailable`; and non-Composition method → no guild requirement. Pin same-token behavior with an identity-cache hit:

```go
identityCache.Set("same-token", "player-1")
verifier.EXPECT().GetCurrentUserGuildMember(gomock.Any(), &auth.GetCurrentUserGuildMemberInput{
    Token: "same-token", GuildID: "123456789012345678",
}).Return(&auth.DiscordGuildMember{User: &auth.DiscordUser{ID: "player-1"}}, nil)
```

Also prime both caches, return provider `401`, and assert `identityCache.Get("same-token")` and every digest-key membership decision for that token miss afterward. Use an injected clock to prove exact 30-second expiry, no denial caching, 1,024-entry bounding, and deterministic eviction; inspect test failure output to confirm no token/digest text is emitted.

- [ ] **Step 2: Run the RED tests**

```bash
cd /home/kirk/game-dev/rpg-api
go test ./internal/auth ./internal/worldcontext -run 'World|Guild|Membership|TokenCache_Delete' -count=1 -v
```

Expected: FAIL because the new context, resolver, verifier method, and cache do not exist.

- [ ] **Step 3: Implement the narrow auth seam**

Add the exact interfaces from the file map. Parse GuildID as ASCII `[1-9][0-9]*`, then require `strconv.ParseUint(value, 10, 64)` success. Keep request auth private in `internal/auth`; install it on cache-hit, cache-miss, and Dev paths. The world interceptor allowlist is exactly:

```text
/api.composition.v1alpha1.CompositionService/CreateComposition
/api.composition.v1alpha1.CompositionService/GetComposition
/api.composition.v1alpha1.CompositionService/ListCompositions
/api.composition.v1alpha1.CompositionService/DeleteComposition
```

Discord resolution verifies `member.User.ID == GetPlayerID(ctx)` inside `internal/auth`, maps GuildID directly to WorldID, and caches only success. Dev resolution ignores the Discord guild selector and installs the configured Dev WorldID only after the global interceptor has accepted the Dev scheme. A provider `401` calls the existing cache's new `Delete(token)` and scans/removes matching digest entries from the membership cache; no raw-token index is added.

- [ ] **Step 4: Run GREEN and race tests**

```bash
go test ./internal/auth ./internal/worldcontext -run 'World|Guild|Membership|TokenCache_Delete' -count=1 -v
go test -race ./internal/auth ./internal/worldcontext -count=1
```

Expected: PASS; race detector reports no races.

- [ ] **Step 5: Generate, inspect, and commit before any API `make ci-check`**

```bash
make generate
git diff --check
git diff --stat
git diff -- internal/auth/mock/mock_discord.go
git add internal/auth internal/worldcontext cmd/server/server.go
git commit -m "feat(auth): resolve trusted guild world context"
git status --short
```

Expected: generated mock changes match only the new verifier interface and status is clean after the normal hook. This ordering is mandatory because `scripts/ci-checks.sh` runs generation and executes `git checkout -- .` when it sees a diff.

### Task 3: Bind CompositionService to trusted context and register it outside dev

**Files:**
- Modify: `cmd/server/{composition.go,composition_test.go}`
- Modify: `internal/handlers/api/composition/v1alpha1/{handler.go,handler_test.go,integration_test.go}`
- Modify: `docs/architecture/components/auth.md`, `docs/status.md`, `docs/quality.md`
- Verify unchanged: `internal/orchestrators/composition/{orchestrator.go,orchestrator_test.go}`, `internal/repositories/composition/{redis.go,redis_test.go}`

**Interfaces:**
- Consumes: `worldcontext.Get(ctx) (Value, bool)` from Task 2.
- Produces: service inputs whose WorldID comes only from trusted context; production registration with the existing authoring flag.

- [ ] **Step 1: Write failing handler and registration tests**

For each of Create/Get/List/Delete, install world context `123456789012345678`, send missing/matching/mismatching body WorldID, and use strict gomock expectations so missing/mismatch proves zero service access. Add service-output cases where Create/Get/List returns a `composition.Data` for another world and require `Internal` with no data response. Change registration proof to require presence in both DevMode false and true; retain Create/Delete authoring-disabled tests and Get/List allowed tests.

```go
ctx := auth.WithPlayerID(context.Background(), "player-1")
ctx = worldcontext.With(ctx, worldcontext.Value{WorldID: "123456789012345678"})
_, err := handler.GetComposition(ctx, &compositionpb.GetCompositionRequest{
    WorldId: "999999999999999999", Id: "composition-1",
})
require.Equal(t, codes.PermissionDenied, status.Code(err)) // no service EXPECT
```

- [ ] **Step 2: Run the RED tests**

```bash
go test ./internal/handlers/api/composition/v1alpha1 ./cmd/server \
  -run 'Composition|World|Registration|Authoring' -count=1 -v
```

Expected: FAIL because the handler still uses configured singleton world and production registration is absent.

- [ ] **Step 3: Implement trusted handler access and unconditional registration**

Remove `WorldID` from `HandlerConfig`/`Handler`. `authorizeWorld` requires both `PlayerID` and `worldcontext.Value`, treats a missing trusted value as `FailedPrecondition`, missing body WorldID as `InvalidArgument`, and mismatch as `PermissionDenied`; it returns `PlayerID` plus trusted WorldID for service input. Validate every returned composition is non-nil, has the requested ID where applicable, and matches trusted WorldID before proto conversion. Keep repository key/envelope code and the authoring gate unchanged.

Always construct/register the service in `registerCompositionService`; the resolver, not registration, owns Dev-vs-Discord world selection. Mark health serving whenever construction succeeds.

- [ ] **Step 4: Run focused GREEN plus stored-world checks**

```bash
go test ./internal/handlers/api/composition/v1alpha1 ./internal/orchestrators/composition \
  ./internal/repositories/composition ./cmd/server -count=1 -v
```

Expected: PASS, including existing miniredis wrong-world stored-envelope failures and authoring behavior.

- [ ] **Step 5: Update living docs, regenerate, commit clean, then run the destructive full gate**

```bash
make generate
git diff --check
git diff --stat
git diff -- internal/auth/mock internal/services/composition/mock internal/repositories/composition/mock
git add cmd/server internal/handlers/api/composition internal/auth/mock docs/architecture/components/auth.md docs/status.md docs/quality.md
git commit -m "feat(composition): enforce trusted request world"
git status --short
make ci-check
git status --short
```

Expected: both status checks are clean and `make ci-check` passes. If generation changes anything before the commit, inspect and include the legitimate generated file first; never let `make ci-check` erase an uncommitted change.

### Task 4: Bind the web source and UI to the Discord credential session

**Files:**
- Create: `src/discord/DiscordProvider.test.tsx`, `src/api/auth.test.ts`
- Modify: `src/discord/{DiscordProvider.tsx,types.ts}`
- Modify: `src/api/{auth.ts,client.ts,client.test.ts}`
- Modify: `src/compositions/{rpcCompositionSource.ts,rpcCompositionSource.test.ts,useCompositionResolutions.test.tsx}` and `src/author/Palette.compositions.test.tsx`
- Verify unchanged unless a failing lifecycle test proves otherwise: `src/compositions/{compositionSource.ts,useCompositionResolutions.ts}`
- Modify: `src/{App.tsx,App.test.tsx}`
- Modify: `docs/architecture/components/discord.md`, `docs/status.md`, `docs/quality.md`

**Interfaces:**
- Consumes: SDK 2.5.0 `DiscordSDK.guildId`, authenticate `scopes`, and current Connect client.
- Produces: one production `CompositionSource` per `(authSessionId, GuildID)`, a per-Composition-RPC guild header, and local auth teardown callback on server `Unauthenticated`.

- [ ] **Step 1: Write failing transport, provider, factory, and lifecycle tests**

Assert authorize receives exactly the three scopes and no `prompt` property:

```ts
expect(authorize).toHaveBeenCalledWith({
  client_id: 'client-id',
  response_type: 'code',
  state: '',
  scope: ['identify', 'applications.commands', 'guilds.members.read'],
});
expect(authorize.mock.calls[0]![0]).not.toHaveProperty('prompt');
```

Assert `sdk.guildId`, not URL debug parsing, reaches `setAuth`; cancelled/denied auth and returned scopes missing `guilds.members.read` clear token/user/scopes/source and expose reconnect copy. In `client.test.ts`, fake a Composition request and a Character request: only the former receives one `x-rpg-guild-id`. In source/hook/App tests prove no production source without authenticated guild, direct GuildID→WorldID, explicit Dev fallback only in development, `Unauthenticated` clears auth, no-guild disabled UI copy, and late results from old token-session/guild source cannot publish after rerender.

- [ ] **Step 2: Run focused RED tests while retaining actual stdout and exit status**

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
set -o pipefail
npm test -- --run src/discord/DiscordProvider.test.tsx src/api/auth.test.ts \
  src/api/client.test.ts src/compositions/rpcCompositionSource.test.ts \
  src/compositions/useCompositionResolutions.test.tsx src/App.test.tsx \
  2>&1 | tee /tmp/guild-world-web-red.log
red_status=${PIPESTATUS[0]}
echo "vitest exit: ${red_status}"
test "${red_status}" -ne 0
```

Expected: command confirms a non-zero Vitest exit for missing new behavior; `/tmp` output contains no token value.

- [ ] **Step 3: Implement consent, transport, source, and UI lifecycle**

Use `sdk.guildId`; request `guilds.members.read`; omit `prompt`. Treat returned scopes only as UI evidence. Replace `console.log('Authentication successful!', auth)` with a credential-free summary. Every auth success or clear increments `authSessionId`; `clearAuthentication` clears React user/scopes/auth flag and module auth together.

The Connect interceptor adds `x-rpg-guild-id` only when `req.service.typeName === CompositionService.typeName`, a Discord token exists, and a guild exists. Dev requests send no guild header. `App` clears its source immediately before asynchronously creating the next source, keys creation on the opaque auth epoch and GuildID, and never embeds the token in the source. Existing source-object/world guards cancel list and resolution publication from old sources; key the World Builder surface by the same non-secret epoch/GuildID so editor-local notice, selection, library, and in-flight state cannot survive a source replacement. Keep global lobby/session/character requests and their state contracts unchanged.

Show “Open this Activity in a server to access its world” when authenticated production has no SDK guild. A cancelled/denied/missing-scope grant shows retry/reconnect and retains no old source. `PermissionDenied` shows generic access denial unless returned scopes prove the membership scope is missing; `Unavailable` keeps an explicit retry action and never substitutes a Dev source.

- [ ] **Step 4: Run focused GREEN, then the complete web gate with unsuppressed evidence**

```bash
set -o pipefail
npm test -- --run src/discord/DiscordProvider.test.tsx src/api/auth.test.ts \
  src/api/client.test.ts src/compositions/rpcCompositionSource.test.ts \
  src/compositions/useCompositionResolutions.test.tsx src/App.test.tsx \
  2>&1 | tee /tmp/guild-world-web-green.log
test_status=${PIPESTATUS[0]}
echo "focused vitest exit: ${test_status}"
test "${test_status}" -eq 0

npm run ci-check 2>&1 | tee /tmp/guild-world-web-ci.log
ci_status=${PIPESTATUS[0]}
echo "ci-check exit: ${ci_status}"
test "${ci_status}" -eq 0
```

Expected: both exit codes are zero. If `ci-check` reports only a summary failure, run its named underlying command (`npm run format:check`, `npm run lint`, `npm run typecheck`, `npm run build`, or `npm test -- --run`) without redirection and retain that stdout; do not diagnose from the suppressing wrapper alone.

- [ ] **Step 5: Commit the coherent web wave**

```bash
git diff --check
git diff --stat
git add src docs/architecture/components/discord.md docs/status.md docs/quality.md
git commit -m "feat(compositions): bind world source to Discord guild session"
git status --short
```

Expected: normal lint-staged hook passes and status is clean.

### Task 5: Permit and preserve the selector through deployment config

**Files:**
- Create: `tests/guild-world-header-contract.sh`
- Modify: `envoy/{envoy.yaml,envoy-lab.yaml,envoy-lab2.yaml}`
- Modify: `nginx/{nginx-http.conf,nginx-ssl.conf}`
- Modify: `Makefile`

**Interfaces:**
- Consumes: browser `x-rpg-guild-id` request header.
- Produces: successful CORS preflight allowance and byte-preserving forwarding to the API; no proxy-side trust decision.

- [ ] **Step 1: Write the failing static contract**

The Bash test must assert exactly one lowercase `x-rpg-guild-id` occurrence in each Envoy `allow_headers` line and an explicit line in both production gRPC-Web regex locations:

```nginx
proxy_set_header X-Rpg-Guild-Id $http_x_rpg_guild_id;
```

For nginx, it must also reject any config that assigns a literal selector, validates snowflakes, or adds the header to a non-gRPC location. Add the script to `make test`.

- [ ] **Step 2: Run RED without starting the stack**

```bash
cd /home/kirk/game-dev/rpg-deployment
bash tests/guild-world-header-contract.sh
```

Expected: FAIL because Envoy does not allow and nginx does not explicitly preserve the header.

- [ ] **Step 3: Make the minimal configuration changes**

Append `x-rpg-guild-id` to each synchronized Envoy CORS list. Add the exact `proxy_set_header` line only to the gRPC-Web service regex blocks in `nginx-http.conf` (the actual Cloudflare-terminated production path) and `nginx-ssl.conf` (its maintained SSL twin). Do not alter origin rules, manufacture a selector, enable Dev auth, or touch production secrets.

- [ ] **Step 4: Run GREEN and repository tests without bringing services up**

```bash
bash tests/guild-world-header-contract.sh
make test
git diff --check
```

Expected: PASS. `make test` may render Compose configuration but starts no containers.

- [ ] **Step 5: Commit**

```bash
git add Makefile tests/guild-world-header-contract.sh envoy nginx/nginx-http.conf nginx/nginx-ssl.conf
git commit -m "feat(proxy): pass guild world selector"
git status --short
```

Expected: clean status.

### Task 6: Independent review, merge/deploy order, and coordinated Discord proof

**Files:** no additional implementation files; record evidence on the owning issues/PRs without credentials.

**Interfaces:**
- Consumes: exact reviewed API, deployment, and web heads from Tasks 2–5.
- Produces: three review verdicts plus a credential-free normal-Discord acceptance record.

- [ ] **Step 1: Self-review each exact head before publication**

For each repo, compare its branch against the correct remote base, run `git diff --check`, confirm only mapped files changed, and scan for raw credentials/digests and scope leaks:

```bash
git diff --name-status origin/dev...HEAD        # API and web
git diff --name-status origin/main...HEAD       # deployment
git grep -n -E 'access_token.*(log|console)|same-token|Bearer [A-Za-z0-9]' -- ':!**/*_test.go' ':!**/*.test.ts*'
git status --short
git diff --cached --name-only
```

Expected: mapped files only, no credential logging, clean worktree, and no staged files. Parent dispatches fresh read-only reviewers against the exact SHAs and resolves all findings on the same repo branch; no arbitrary fix branches.

- [ ] **Step 2: Merge and deploy providers before the production caller**

Kirk merges deployment to `main` first so the permissive proxy path can deploy harmlessly before any browser sends the header. Kirk then squash-merges API to `dev`, promotes `dev` to `main` with a real merge commit under the documented release rule, and waits for the API image/deployment health to prove `CompositionService` registration outside Dev. Only then does Kirk squash-merge web to `dev`, promote web `dev` to `main` with a real merge commit, and deploy the production caller. Never deploy web while either Envoy blocks the header or API production lacks the service.

- [ ] **Step 3: Run non-live automated release checks**

Re-run the final API focused/full commands, web focused/full commands with retained exit codes/stdout, and deployment static/`make test` commands against the exact merge candidates. Do not start the currently down owned stack; local Dev-world tests prove only the explicit Dev scheme, not Discord OAuth.

- [ ] **Step 4: Coordinate the normal Discord user test**

With Kirk/user present and no token capture, use the deployed production Activity to verify:

1. an account holding the old two-scope grant sees the real Discord modal for `guilds.members.read`, can cancel into a cleared reconnect state, then can grant it;
2. a guild member opens an empty library, creates with existing authoring enabled, lists/gets, resolves a ref in preview/play, and deletes; no role is required;
3. no guild, known missing scope, non-member guild, expired token/provider `401`, provider unavailability, malformed/repeated/altered header, and body/header mismatch fail with the approved status/UI behavior;
4. wrong-world IDs cannot read/write/delete/resolve or warm a cache, and switching guild/auth session or signing out while reads are in flight leaves no prior source/list/palette/resolution/error result visible; and
5. browser Network evidence shows OPTIONS permits `x-rpg-guild-id`, the subsequent request carries it through nginx/Envoy, production rejects `Dev`, and no request reaches `test-world`.

Record only build SHAs, guild labels or redacted IDs, RPC/status outcomes, UI screenshots without tokens, and pass/fail observations. Never copy authorization headers, codes, access tokens, client secrets, token digests, or browser storage into artifacts.

- [ ] **Step 5: Close the execution gate**

Parent reports the exact reviewed heads, commands/exit codes, normal-Discord proof receipt, and any concrete blocker to Kirk. Keep rpg-project PR #402 open as the cross-repo tracking surface until implementation is complete; Kirk merges it afterward. #403 and #937 stay deferred Platform/Todo unless their own issues are separately scheduled.
