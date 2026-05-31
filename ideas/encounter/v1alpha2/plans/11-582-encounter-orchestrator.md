# Design: rpg-api#582 — the clean encounter orchestrator (retire the Runner + scattered loader)

**Status:** ACCEPTED (2026-05-30, Kirk signed off). Attack verbs gated on toolkit#689; non-attack verbs + Runner removal unblocked.
**Authored by:** rpg-api team-member (design pass), under the director reframe "the Runner goes away; build the dedicated orchestrator."
**Depends on:** rpg-toolkit#689 (encounter hydration cascade — see [`10-689-encounter-hydration-cascade.md`](10-689-encounter-hydration-cascade.md)). The attack-resolving verbs land clean only once #689 removes the per-attack re-load.
**Supersedes (reconcile):** the 2026-05-06 `orchestrator-design.md` (streams/visibility-heavy vision) and the `sdk-direction-rpgapi.md:152` "no orchestrator layer in v2" note — both predate this reframe.

---

## Executive summary

1. The `Runner` (`handlers/dnd5e/v2/encounter/runner.go`) is a handler-package retrofit and **goes away**. The clean path is a **dedicated `internal/orchestrators/encounter/v2` package**, one explicit method per RPC, each `load(id) → toolkit verb → persist`, with the thin handler reduced to proto↔toolkit translation. The single-load *guarantee* survives; the Runner *form* dies.
2. The real #684 source is the combat resolver **re-loading** characters mid-attack — structurally required by today's SDK (resolver gets IDs, not entities). That is **not separable from a toolkit change**; it is rpg-toolkit#689 (the hydration cascade). `enc.load(id)` alone cannot cure it.
3. **Sequencing B** — move RPCs onto the orchestrator one at a time, each MCP-playtest-verified, deleting the old path per-verb. Non-attack verbs (Interact/SubmitCheck/ActivateFeature) carry **no** resolver dependency and move first; attack verbs (TakeAction/EndTurn/Move) resume after #689 lands so they arrive clean.
4. **Snapshot-on-subscribe** becomes a first-class orchestrator read path (current state on stream-open, then live) — catch-up belongs in the orchestrator, not the transport (the in-mem transport already has Redis pub/sub's no-replay semantics).
5. When complete: every RPC goes through one `load`; the Runner + `loadCharacterWithBus` + `applyReactionConditions` + the inline verb bodies are gone; the depguard import-guard covers the whole handler+orchestrator package.

---

## 1. Orchestrator shape & location

**Package & path: `internal/orchestrators/encounter/v2` (package `encounter`)** — aligns with `orchestrator-design.md` §2.1's v2/ segment and `rpg-api/CLAUDE.md`'s documented orchestrator layer. Today everything is in the handler package (`runner.go`, the verbs, `dnd5e_combat_resolver.go`, `translate.go` are all `package encounter` there). The move splits it into handler (translation) + orchestrator (load→verb→persist) + the resolver adapters.

### Public surface (entity types in, never `pb.`)

```go
type Orchestrator struct {
    broker   *tkenc.Broker
    encRepo  encountersv2.Repository
    charRepo characterrepo.Repository
    resolver CharacterResolver
    roller   tkdice.Roller
    now      func() time.Time
}

func (o *Orchestrator) ActivateFeature(ctx, *ActivateFeatureInput) (*ActivateFeatureOutput, error)
func (o *Orchestrator) TakeAction(ctx, *TakeActionInput) (*TakeActionOutput, error)
func (o *Orchestrator) EndTurn(ctx, *EndTurnInput) (*EndTurnOutput, error)
func (o *Orchestrator) Interact(ctx, *InteractInput) (*InteractOutput, error)
func (o *Orchestrator) SubmitCheck(ctx, *SubmitCheckInput) (*SubmitCheckOutput, error)
func (o *Orchestrator) Move(ctx, *MoveInput) (*MoveOutput, error)
func (o *Orchestrator) Get(ctx, *GetInput) (*GetOutput, error)
func (o *Orchestrator) Snapshot(ctx, *SnapshotInput) (*SnapshotOutput, error)   // §5
```

Input/Output speak toolkit/entity types (`encountercore.PlayerID`, `tkenc.ActionRef`, `core.Hex`) — never proto. Action-verb outputs are lean acks plus any caller-private payload; world changes flow as broker events, not in the response.

### The private `load(id)`

The `Runner.Run` body (`runner.go:109-155`) becomes a private method — single-load guarantee survives, closure-over-handler form dies. **It returns the encounter and nothing else** (Kirk review catch: the draft's 3-return `(*Encounter, *Data, error)` had a smell — the extra `*Data`):

```go
func (o *Orchestrator) load(ctx, in loadInput) (*tkenc.Encounter, error) {   // 2 returns
    data, err := o.encRepo.Get(ctx, in.EncounterID)        // NotFound mapping
    // membership + optional entity-ownership checks here, from data, BEFORE LoadFromData
    // (skip rehydrate on the auth-fail path)
    return tkenc.LoadFromData(ctx, data, o.broker,          // ctx per #689
        tkenc.WithCharacterResolver(o.resolver),
        tkenc.WithCombatResolver(o.buildCombatResolver(data)),
        tkenc.WithMovementResolver(o.buildMovementResolver(data)),
        tkenc.WithRoller(o.roller))
}
```

**Why no `*Data` return:** the encounter *is* the synced state — it holds every entity and, asked, returns it with state in sync. Verbs / resolver / orchestrator read entities + orchestration state **through the encounter** (a small synced-getter surface: entity-by-id, initiative, mode), never a side snapshot that can drift. Persist with `enc.ToData()` — the dirty-gated cascade (see [`10-689`](10-689-encounter-hydration-cascade.md) §1) that serializes the held entities back into the **simple `Data` snapshot**. `Data` stays simple; we are *not* making entities the sole truth / deriving `Data` on demand now (possible later tightening).

Each method: `enc := o.load(...)` → `enc.<Verb>(...)` → `o.encRepo.Save(ctx, enc.ToData())`.

### How the thin handler calls it

```go
func (h *Handler) TakeAction(ctx, req *pb.TakeActionRequest) (*pb.TakeActionResponse, error) {
    pid := auth.GetPlayerID(ctx)                         // envelope validation only
    in := protoToTakeActionInput(req, pid)               // proto→toolkit map
    out, err := h.orch.TakeAction(ctx, in)
    if err != nil { return nil, takeActionStatusError(err) }   // sentinel→gRPC code (proto-layer)
    return &pb.TakeActionResponse{}, nil                 // ack; translate.go handles the stream
}
```

`translate.go` stays in the handler package (it's proto translation). The orchestrator package gets its own depguard `files:` entry denying the 5 rulebook packages.

---

## 2. The crux — why it's #689, not inline (verified)

The toolkit `Encounter` holds **no** hydrated entities (`encounter.go:32-49`); `CombatResolver.ResolveAttack` is handed **IDs**, not entities (`combat_resolver.go:32-43` — "the encounter SDK only passes IDs"). So rpg-api's `Dnd5eCombatResolver.resolveEntity` (`dnd5e_combat_resolver.go:464`) **must** re-`character.LoadFromData` per attack → re-subscribes conditions to the same bus → #684. `enc.load(id)` can't feed hydrated entities to the resolver because the SDK never asks for them. The `defer Cleanup` is a patch, not a cure.

**Resolution:** rpg-toolkit#689 makes `Encounter.LoadFromData` own the hydration cascade and hands the resolver the **held** entity. Then this orchestrator's `load(id)` is the only hydration, and `Dnd5eCombatResolver` shrinks to translation. This is why #582's attack verbs are paused on #689.

---

## 3. Reaction conditions (OA/Shield)

`reaction_conditions.go` constructs rulebook-internal `conditions.New*` (a depguard-excluded boundary violation) on every re-load. Under #689 these ride the hydration cascade (driven by the SDK-owned `ReactionReadiness`), so the file + its depguard exclusion delete and rpg-api stops constructing conditions. rpg-api keeps only the *readiness data* ("what's ready"), which is orchestration, not rules.

---

## 4. The EndTurn seam (migrates last)

`end_turn.go` (416 lines) has three concerns; design them in, don't hack-wrap:

- **NPC-dispatch loop** (`end_turn.go:137-226`): `enc.NPCAct(newActive)` → `enc.EndTurn(newActive)` until a player is active or the roster-cap hits. This is orchestration (server acts for NPCs) — move it verbatim into `Orchestrator.EndTurn`; it speaks only `enc.NPCAct`/`enc.EndTurn`/`enc.Mode()`.
- **Turn-end condition reset** (`publishTurnEndAndPersistReset`, `:345-395`): today the worst smell — a separate `character.LoadFromData` to publish the turn-boundary, guarded by `defer Cleanup`. Under #689 the SDK's `EndTurn(ctx,…)` emits the boundary on the bus itself and held conditions reset with zero load → this helper **deletes**.
- **Pause-for-reaction serialization** (`:162-183`): on `IsNPCPausedForReaction`, `enforceSingleReactor` + serialize the opaque `*combat.AttackContext` + save + return. The marshaling belongs in the **resolver adapter** (`o.combatAdapter.SerializePending(enc)`), keeping the orchestrator method rulebook-agnostic (toolkit#657/#658 track the proper serializer-callback fix).

Net: `load → enc.EndTurn → [SDK emits turn-boundary] → npc-dispatch loop (pause-aware via adapter) → save`.

---

## 5. Snapshot-on-subscribe

Today `StreamEncounter` (`handler.go:213-296`) already does snapshot-then-live: `Subscribe` first → `ProjectFor(data, viewer, broker, now)` (`project.go:26`) → `TranslateSnapshot` + `BuildReplayEvents` → live forward loop. Lift the snapshot build into `Orchestrator.Snapshot(ctx, {EncID, Viewer})` returning the projected view (entity types); the handler translates to proto (`TranslateSnapshot`/`BuildReplayEvents` stay proto-layer). Source is the repo (`Get` → `LoadFromData` → `ProjectFor`, the one projection site). Keep `Subscribe`-before-snapshot ordering so no event is lost. No transport change; it's a read method (no verb, no persist).

---

## 6. Per-RPC sequencing (concrete, Sequencing B)

| Order | RPC | Why here | #689 dep | Deletes on land | Playtest sign-off |
|---|---|---|---|---|---|
| 1 | **Interact** | No attack resolution (door verb). Proves orchestrator skeleton + handler-thinning + depguard. | none | inline `interact.go` body | Door open / locked-door attempt, both browsers see the events |
| 2 | **SubmitCheck** | No attack resolution (skill check). `take_reaction` branch rides the adapter seam. | none | inline `submit_check.go` body | Locked-door check resolves; door opens/stays per total≥DC |
| 3 | **ActivateFeature** | Already on the Runner; pure relocation Runner→Orchestrator, no behavior change. | none | the `Runner` type + `runner.go` | Rage lights the indicator; ResourceChanged + ConditionApplied stream |
| 4 | **TakeAction** | First attack-resolution verb — **after #689** so it lands clean (held entity, no re-load). | **#689** | inline `take_action.go` body | Player attacks goblin; per-viewer AttackResolved+DamageDealt; raging resistance halves; **no "modifier ID already exists"** |
| 5 | **EndTurn** | Hardest (§4) — after #689 so `publishTurnEndAndPersistReset` dies and turn-reset needs no re-load. | **#689** | `end_turn.go` inline body, `publishTurnEndAndPersistReset`, `publishTurnEndOnBus` | Player ends turn; NPC acts server-side; SneakAttack once-per-turn resets across the boundary; full round both directions |

`Move` rides the movement resolver (same re-load issue via `dnd5e_movement_resolver.go`) → fold in with step 4 after #689.

---

## 7. Deletion ledger (end state)

| Deleted | Evidence | Unlocks |
|---|---|---|
| `Runner` + closure form | `runner.go` (whole file) | One named `load(id)` replaces the handler-package retrofit |
| `loadCharacterWithBus` | `dnd5e_combat_resolver.go:505` | The entity-level re-load on the combat path |
| `applyReactionConditions` / `applyMonsterReactionConditions` / `hasFirstLevelSpellSlot` | `reaction_conditions.go` (whole file) | Removes `conditions.New*` → depguard exclusion deleted, guard covers it |
| `publishTurnEndAndPersistReset`, `publishTurnEndOnBus`, the `defer Cleanup` | `end_turn.go:323-395` | The turn-reset re-load (#689 emits the boundary on the bus) |
| `resolveEntity`/`rehydrateMonster` re-load bodies | `dnd5e_combat_resolver.go:464-685` | Resolver shrinks to "translate AttackInput→combat.AttackInput, call ResolveAttack, translate result" |
| inline verb bodies | `take_action.go`, `end_turn.go`, `interact.go`, `submit_check.go` | Handlers become ~20-line proto maps; **depguard covers the whole package by construction** |

---

## Director notes / open items

- **Scope:** #582 = orchestrator + non-attack verbs (1-3) + Runner removal. Attack verbs (4-5) resume after #689. (Per the paused-dependency call logged on #582.)
- `ActivateFeature`'s own `defer Cleanup` re-load is the same #684 class off the critical path → its own follow-up issue (see #689 Q3 resolution).
- The 2026-05-06 `orchestrator-design.md` (streams/visibility component, `view.ResolveEvent`) is **not** the converged shape — the event spine that shipped is simpler (toolkit emits → broker → `StreamEncounter` drains → `translate.go`). Treat that doc as historical; this doc + `10-689` are current.
