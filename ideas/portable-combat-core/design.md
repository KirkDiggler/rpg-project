# Portable Combat Core — a language-agnostic composition engine

## Status: Exploration / design captured 2026-06-09

This is a design exploration, not committed work. It captures the **decisions**
behind rpg-toolkit's event-driven composition core so the same core can be
re-implemented in any language — specifically C++ for Unreal and (via a C ABI)
Unity — without re-deriving the architecture each time.

## What this is

rpg-toolkit's combat engine is not D&D. It's four small, system-agnostic
concepts that compose into *any* triggered-effect combat system:

- a **Bus** that routes events,
- a **Chain** that collects and applies modifiers in a declared order,
- an **Action** that fires (the verb),
- an **Effect** that subscribes and modifies (the persistent listener).

D&D 5e is *just a rulebook* sitting on top of those four. So is an
Across-the-Obelisk–style co-op combat deck-builder — which is the real target.

### What we are building

Composable combat-system **components** a game builder uses to prototype their
own game. The toolkit philosophy — *infrastructure, not implementation* — made
portable.

### What we are NOT building

- Not a game.
- Not a rulebook. We provide the components; the game builder assembles them.
  (A "spot for rulebooks" in the toolkit is a way-later thought, not v1.)
- Selectables rides along as a **separate companion tool** — a weighted
  loot-table wrapper — not part of the core nervous system.

### Target genre

Co-op combat deck-builder (Across the Obelisk / Slay the Spire family). These
games are *nothing but* triggered effects and status synergies, which is exactly
what this core makes composable. See the mapping below.

## Relationship to the current project

**This becomes its own project** (its own repo, eventually). It is incubating in
`rpg-project/ideas/` only because that's where design exploration starts.

**The existing RPG game is untouched.** rpg-toolkit (Go), rpg-api, and
rpg-dnd5e-web keep shipping on the Go toolkit; that work continues unaffected.
The portable core is a **parallel** effort — not a migration of the Go code, not
a fork that feeds changes back into it, and not a dependency the game takes on.
It *learns from* the Go toolkit's design (the decisions below were extracted from
it) but evolves independently.

Practical consequence: where this doc says the port "unifies" something the Go
code does two ways, that means **the new project starts clean** — it does **not**
mean we go refactor the live Go toolkit. The Go toolkit keeps its current shape
because the game depends on it.

## The load-bearing principle

> **The bus never crosses a language boundary.** It lives wherever the rulebooks
> live. Only *actions in* and *results/breakdowns out* cross the FFI wire.

Why: a chain's modifiers are closures that must run in the same runtime as the
contributors. You cannot hand a C# lambda to Go to be stored and re-executed
(with fresh re-rolls) later. So the bus + chains + effects are the rulebook's
**internal nervous system**, not a client/server seam. This single principle
resolves the whole port strategy (below).

## The mental model: 4 concepts, 2 verbs

- **Event** — immutable data. "An attack is happening." Just fields. Never
  behavior, never a chain inside it.
- **Topic** — a typed routing key: a string id (`"combat.attack"`) *plus* a
  payload type. The string routes; the type protects.
- **Chain** — a transient, per-resolution collector of modifiers. Born for one
  attack, filled, executed, discarded.
- **Publish** *collects* contributions. **Execute** *applies* them. Separate
  verbs, deliberately. Publishing never transforms.
- **Action** — the verb that fires.
- **Effect** — the persistent subscriber that listens and modifies.

## The core contracts (language-agnostic)

These are the contracts, stripped of Go. Generic `<T>` = the event/payload type.

### Event
Plain immutable data. No methods required.

### Topic `<T>`
- A definition: `defineTopic<T>(id: string)` — created once, statically.
- A binding: `topicDef.on(bus) -> Topic<T>` — connects the definition to a
  specific bus instance at runtime.
- Two flavors:
  - **Notification topic**: `subscribe(handler: (T) -> void/err)`, `publish(T)`.
  - **Chained topic**: `subscribeWithChain(handler: (T, Chain<T>) -> Chain<T>)`,
    `publishWithChain(T, Chain<T>) -> Chain<T>`.

### Bus
- `subscribe(topicId, handler) -> subscriptionId`
- `unsubscribe(subscriptionId)`
- `publish(topicId, payload)`
- Internally **type-erased** (routes by string id over an opaque payload). The
  typed topic flavors above are a thin veneer that casts in/out.

### Chain `<T>` (staged)
- Constructed with an **ordered stage list**: `newChain<T>(stages: string[])`.
- `add(stage, id, modifier: (T) -> T) -> err`  (duplicate id = error)
- `remove(id) -> err`
- `execute(data: T) -> T`  (folds data through stages in order; within a stage,
  insertion order)

### Action `<TInput>`
- Has identity (id, type).
- `canActivate(owner, input: TInput) -> err`  (gate: cost, target, cooldown)
- `activate(owner, input: TInput) -> err`  (spend cost, publish events, apply
  effects)

### Effect
- Has identity and a source (what granted it).
- `apply(bus) -> err`  — subscribes its handlers; marks active.
- `remove(bus) -> err` — **auto-unsubscribes all tracked handlers**; marks
  inactive.
- Internally owns a **subscription tracker** so cleanup is automatic.

## The decisions (the portable part — *why* it's shaped this way)

Each decision is stated so it can be re-implemented anywhere.

1. **A topic is `string id + payload type`; the transport is type-erased.**
   Strings route (decoupled, serializable, debuggable); types protect
   (no cast errors, IDE help). Consequence: the irreducible concurrency core is
   tiny (~120 lines in Go) and language-natural; the type safety is a *separable
   veneer* each language expresses its own way (Go `any`, C++ `std::variant` or
   `void*`+type-tag, C# `object`, TS `unknown`). **This layering is what makes
   porting tractable** — port the small erased core carefully, express the typed
   veneer idiomatically.

2. **Define topics at compile time; bind to a bus at runtime (`.On(bus)`).**
   A topic is a static fact about the game ("attacks exist, shaped like this").
   A bus is a runtime instance (per-encounter, per-test). Decoupling them: one
   topic def works across many buses; features hold no bus reference; you spin
   up an isolated bus per encounter or per test for free; and every connection
   point is *visible* in code. The anti-pattern avoided: a hidden global bus
   with stringly-typed publish.

3. **Two primitives, not one: Notification vs. Chained.** Notifications flow
   one-way (no transformation). Chained collect modifiers. Keeping them separate
   at the type level forbids the worst bug class — a notification handler quietly
   mutating the event and creating order-dependence.

4. **Handlers don't mutate — they register pure modifier functions.** Many
   features touch one event (rage +2, bless +4, vulnerable ×1.5) and none know
   about each other. If they mutated during notification, the result would
   depend on subscription order (nondeterministic). Instead each *registers* a
   `(T) -> T` into the chain, and transformation is deferred to Execute. This is
   the entire composability story.

5. **Stages own ordering — not handlers, not subscription timing.** The chain is
   built with an ordered stage list (`base -> features -> conditions ->
   equipment -> final`). A modifier joins a *named* stage. Execute folds the
   value stage-by-stage. Ordering authority lives in the **rulebook's stage
   list** (declared once, globally), while each contributor only declares *which
   stage* it belongs to. "Resistance after bonus" is guaranteed without rage and
   bless ever knowing about each other.

6. **Chains are per-execution and never embedded in events.** One resolution =
   one fresh chain. Isolates resolutions (no modifier leaks between unrelated
   events), trivially concurrent, and — because modifiers are *functions
   evaluated at Execute time* — gives fresh rolls per occurrence (Bless's `1d4`
   re-rolls every attack) for free.

7. **Modifiers carry a stable unique id** (`"rage-" + ownerId`). Powers dedup
   (don't rage twice), removal (effect ends -> pull its modifier), and the
   **breakdown** output ("rage: +2, bless: +4") that's a core selling point.

8. **The bus routes by topic only; relevance is the handler's job.** Every
   subscriber sees every event on its topic and checks "is this *mine*?" itself.
   The bus stays dumb and fast; entity filtering is domain logic, not
   infrastructure.

9. **Delivery is synchronous, ordered, fail-fast.** Publish walks subscribers in
   order, returns the first error immediately, holds no lock while calling
   handlers. Deterministic and debuggable — combat resolution is inherently
   sequential. This is a *policy* a port may revisit, but the default is
   "deterministic sequential," not async.

10. **Effects have a tracked lifecycle on the bus.** `apply` subscribes; `remove`
    auto-unsubscribes everything it registered. An effect is the unit that
    *persists* on the bus between resolutions; an action is the unit that *fires*
    once. This split keeps "what listens" and "what does" cleanly separated.

## The deck-builder mapping (validation)

| Across-the-Obelisk concept | Core concept |
|---|---|
| Playing a card | **Action** — `activate(owner, input)`; `canActivate` checks energy/target |
| Bleed, Block, Burn, Regen, Weak, Vulnerable | **Effect** — subscribes on apply, auto-cleans on expire / combat end |
| "Whenever an ally plays a card, gain 1 Block" | **Effect** subscribing to a `card.played` **notification topic** |
| Resolving damage (Str + Vulnerable ×1.5 − Block) | **Chained topic** — every contributor registers a modifier |
| Damage order: base → additive → multiplicative → block → final | **Stages** — declared once by the rulebook |
| "Across-the-Obelisk-like" vs "D&D 5e" | Two **rulebooks** over the same core |

Why it collapses complexity: each card and each status is an **independent
subscriber that knows only its own rule.** Bleed doesn't know Block exists. The
chain's stage list composes them into a deterministic result. Adding a new card
never touches existing cards — the thing that makes Slay-the-Spire-likes a
nightmare in naive `if/else` combat code.

## Decisions the new project gets to start clean on

The Go codebase carries some drift. The new project doesn't have to inherit it —
**but this is not a mandate to refactor the live Go toolkit** (the game depends on
it; see "Relationship to the current project"). These are fresh-start choices for
the portable core only.

- **Two subscription models live side by side in Go today**: the elegant
  `events/` one (typed topics + staged chains, `.On(bus)`) and an older one
  `mechanics/effects` still uses (`subscribe(bus, eventType, priority, handler)`
  with integer priorities). Integer priorities and named stages answer the same
  question. **For the new project: typed-topics + staged-chains are canonical;
  no priority-ints.** Stages are more legible (`"block-absorption"` beats
  `priority: 75`), deterministic, and rulebook-declared. The Go toolkit keeps
  both as-is; the portable core simply starts with one.
- **Effect has one shape: reactive (bus lifecycle).** "Apply a modifier to this
  chain" is not a separate top-level concept — it's just what an effect's handler
  *does* when an event fires. (The current `core/effect.Effect[T]` /
  `Apply(chain)` flavor folds into the handler body.)
- **Action vs Effect is clean:** Action is the transient verb that may *spawn*
  effects and *publish* events; Effect is the persistent thing that *listens*. A
  card (Action) applies Bleed (Effect).

## Scope: v1

The smallest honest slice that proves the whole nervous system:

> **An Action that fires, and an Effect that modifies it** — riding on the Bus +
> Chain. E.g., a "Strike" action that publishes a damage chain, and a
> "Strength"/"Bleed" effect that registers a modifier into it. Execute the chain,
> read the breakdown.

Everything else (resources/energy, targeting, turn structure, decks) is a
**rulebook/game-builder concern**, defined on top — not in core v1.

## Path forward (how the two bets actually look)

Resolved by the load-bearing principle (the bus stays with the rulebooks):

- **Bet 1 — Go shared library (the boundary spike).**
  `go build -buildmode=c-shared` -> `.dll`/`.so`/`.dylib` + C header. The bus +
  chains + any sample rulebook stay 100% inside Go. The engine sends an
  **action in** and gets a **result + breakdown out** across the C ABI. The
  engine never touches a topic or a chain. Purpose: prove the engine-embedding
  boundary (lifecycle, marshaling, Unreal frame loop, Unity P/Invoke) with zero
  rules rewrite. Get *one action resolving inside Unreal from Go* — that single
  spike teaches more than a month of C++ design.

- **Bet 2 — native C++ core (the destination).** The entire nervous system
  (bus + chains + effects + actions) is re-implemented in C++. Generics map
  cleanly to templates; modifiers to `std::function`; the type-erased core to
  `std::variant`/type-tag. Unreal subscribes to topics **natively**; Unity gets
  the same flat "action in / breakdown out" C ABI with the bus humming
  underneath on the C++ side. Native feel is the product.

Bet 1 throws away nothing: the C ABI it defines (actions in / results out) is the
same surface Bet 2's native port exposes to Unity.

## Language-mapping notes (Go → C++ / C#)

- **Type erasure**: Go `any` → C++ `std::variant<...>` or `void*` + a small
  type-id tag; C# `object`. Only the erased *core* needs this; typed topics wrap
  it.
- **Generics**: Go generics → C++ templates (natural fit for `Topic<T>`,
  `Chain<T>`); C# generics.
- **Modifiers / handlers**: closures → `std::function<T(T)>` / C# delegates.
  Modifiers are re-evaluated at Execute, so dice re-roll naturally.
- **`context.Context`** is a Go-ism for cancellation; drop it or replace with the
  host's cancellation primitive. The "context" that matters for the game is the
  event payload + chain state, which already travels in `T` and the chain.
- **Subscription ids**: keep as opaque strings/handles so the C ABI can pass them
  across the wire for `unsubscribe`.

## Open questions (deferred, not blocking v1)

- **Resources/energy** (mana, card cost, block-as-pool): core concept later, or
  always rulebook? Leaning rulebook for now; `Action.canActivate` already hooks
  it.
- **Targeting** (single/all enemies, self, allies): lives in the Action's typed
  input `TInput`, i.e. rulebook-level. Core stays unaware.
- **Async/parallel delivery**: keep synchronous default; revisit only if a
  real-time (non-turn-based) host needs it.
- **Effect expiry/stacks** (bleed -> 0): the effect's lifecycle owns it
  (decrement on its own subscribed event, `remove` itself at 0).
- **A "rulebook slot" in the toolkit**: way-later; not v1.

## Provenance

Decisions 1–10 were extracted from the Go implementation, not invented:
`events/bus.go`, `events/typed_topic.go`, `events/chained_topic.go`,
`events/topic_def.go`, `events/chain.go`, `events/CHAIN_PATTERN.md`,
`core/chain/types.go`, `core/action.go`, `core/effect/types.go`,
`mechanics/effects/core.go`. The "bus never crosses a language boundary"
principle and the Bet 1/Bet 2 framing are analysis layered on top, marked as
such above.
