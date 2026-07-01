## DR-011 · 2026-06-28 · Card-play verb: a parallel `playCard` that reuses `strike`, card-runs, Encounter bundles via correlation

Seam/primitive:  the **card verb layer** — how "play a card" reaches the combat
                 mechanism and produces a self-naming receipt at the A2 edge.
                 Touches rpgkit's `Action<TInput>` primitive (consumed, not
                 changed) and tumult's `strike()` / `StrikeResolved` edge.

Decision:        four locked choices, settled by Kirk after the B1 surface pass:

                 1. **Parallel verb (not unify, not data-driven).** Add
                 `Encounter::playCard(...)` as a *new verb layer* that **reuses
                 the existing `strike()` as the one damage mechanism** — a card
                 is not a second damage path. `strike()` stays public and
                 unregressed; the A2 observation edge is untouched.

                 2. **Card-runs (the card owns its behavior).** A concrete
                 `StrikeCard : rpg::core::Action<StrikeInput>` drives the strike
                 inside its `activate()`. `playCard` is **generic** over the
                 Action's input type — it never switches on card kind; a new card
                 is a new `Action` subclass, not a new `playCard` branch.
                 **Genericity asymmetry (named, not yet generalized):** `playCard`
                 is generic on the **input** side (any `Action<TInput>` plays
                 without a type-switch) but strike-**coupled** on the **output**
                 side — it subscribes to `strikeResolvedTopic` to recover the
                 outcome and returns `CardResult{StrikeResult}`. That coupling is
                 deliberate for B1 (one op kind: `strike`). Do **not** pre-build a
                 generic `CardResult<TOutcome>` or a generalized recovery channel
                 now — n=1, YAGNI; just naming the seam so the wave-owner doesn't
                 over-engineer and future-us sees the trigger (below).

                 3. **Card lives in tumult; rpgkit untouched.** `StrikeCard`,
                 `StrikeInput`, `CardResult`, `CardResolved`, `formatCardPlay`,
                 and `playCard` are all tumult (`include/tumult/cards/` +
                 `src/cards/`). `Action<TInput>`, `ActionReceipt`, `EntityRef`,
                 and `ActivateParams::correlationId` all already ship in rpgkit
                 v0.3.0 — **no bump, no tag, no local replace.**

                 4. **Encounter orchestrates and bundles via `correlationId`
                 (the crux).** `ActionReceipt` is identity-only
                 (`{id, type, correlationId}`) — it carries no breakdown, and
                 `activate()`'s return type `(Status, ActionReceipt)` is fixed by
                 rpgkit. So the rich `StrikeResult` produced inside `activate()`
                 **cannot leave through the card's return.** `playCard` recovers
                 it out-of-band by reusing A2's `StrikeResolved` event as the
                 internal channel: `playCard` threads a `correlationId` into
                 `ActivateParams`, the card threads it into `strike()`, `strike()`
                 stamps it onto the `StrikeResolved` it already publishes, and
                 `playCard` (subscribed around the synchronous `activate()` call)
                 picks up the matching event. It then bundles
                 `CardResult{ActionReceipt, StrikeResult}` and publishes a
                 `CardResolved` notification (mirroring StrikeResult/StrikeResolved:
                 the return is the pull, the event is the push). Card identity is
                 read from `receipt.id`/`receipt.type` (never hardcoded — the
                 `formatStep` discipline); each modifier is named from its
                 `source` via the existing `formatStep`; a one-line
                 `formatCardPlay` header names the card.

Cleanest-mechanism check (the part to read):
                 Under the locked card-runs + generic-`playCard` constraints, this
                 IS the cleanest coherent mechanism, and it uses rpgkit's
                 `correlationId` **as designed** ("the host passes the same id to
                 every core operation it wants correlated"). The constraints force
                 it: the card must run the strike inside `activate()` (card-runs),
                 and `playCard` must not know the card is a strike (generic), so
                 the `StrikeResult` cannot be obtained by `playCard` calling
                 `strike()` itself — it must come back out-of-band, and the bus
                 (`StrikeResolved`) is the out-of-band channel that already exists
                 at the A2 edge and is synchronous + fail-fast. The simpler-looking
                 alternatives each break a locked constraint or a stated rule:
                 - *Encounter-runs the strike, card supplies only identity:* would
                 obtain `StrikeResult` directly (no correlation) — but then either
                 `playCard` must switch on card type to call `strike` with the
                 right params (breaks generic) or `activate()` becomes a no-op
                 (breaks card-runs). Rejected by the locks.
                 - *Card stores its last `StrikeResult` as a member / writes to a
                 caller-supplied sink:* avoids correlation but makes the Action
                 stateful (rpgkit's Action is "fires once", transient) and adds a
                 fragile side-channel. Rejected.
                 - *Bare capture-window (subscribe around `activate()`, take
                 whatever `StrikeResolved` fires, no `correlationId`):* works for a
                 single-strike card because the bus is synchronous, but does not
                 forward-fit multi-hit / multi-action cards (several strikes per
                 play) and ignores the primitive rpgkit handed us. `correlationId`
                 is the robust, explicit, forward-fitting choice for a near-zero
                 cost (one extra field threaded through an already-additive path).
                 Conclusion: confirmed — no cleaner mechanism exists without
                 reopening a locked decision.

Rejected:        - **Unify (`strike` becomes / is subsumed by the first card,
                 one play-path):** steelman — default-to-one-system, a single
                 combat verb. Loses on goal-fit: A2 *just* shipped `strike()` +
                 `StrikeResolved` + the `host_consumer` loop as the public edge;
                 unify regresses that, and commits the whole engine to
                 "cards are the only verb" from a sample size of one. The plan
                 explicitly wants B1 to *study* the Action-verb-vs-`strike`-verb
                 seam — unify erases the seam before we can see it.
                 - **Data-driven (card = op-list + a composite resolver;
                 `strike` a reusable op):** steelman — this is where mature card
                 engines land and where multi-action cards point. Loses: it is
                 precisely "fix the card abstraction from a sample of one" — we
                 have exactly one op (`strike`). It designs a resolver primitive
                 before a second/third card reveals the op vocabulary. Earns its
                 keep at multi-action cards, not B1.
                 - **Enrich `ActionReceipt` to carry the breakdown (route the
                 `StrikeResult` out through `activate()`'s return):** steelman —
                 the most direct fix to "the receipt can't carry the breakdown."
                 Loses on the boundary: it is an rpgkit change for a tumult
                 sample-of-one, and `ActionReceipt` is deliberately a thin
                 identity/correlation shape (the host routes it to its own sinks).
                 tumult owns the rich card outcome (`CardResult`); rpgkit's
                 receipt stays thin. Rejected.

Why:             a host can add a card without a GameMode branch (rpgkit-ue#15),
                 and the card play reaches the edge as a self-naming receipt
                 (tumult-ue#13/#14) — the "receipts as primary observability"
                 thesis extended from a strike to a *verb*. Parallel keeps the A2
                 edge intact and the seam observable; card-runs keeps `playCard`
                 generic so the card layer scales by subclassing, not branching;
                 reusing `strike` keeps **one** damage mechanism (default-to-one-
                 system holds at the mechanism layer while the verb layer is
                 genuinely new); the correlation bundle adds **no new primitive
                 kind** (it reuses `Topic<T>` notifications + rpgkit's
                 `correlationId`, exactly as A2 reused `Topic<T>`).

Interface delta: tumult-only; additive; `strike()`'s existing return + A2 events
                 unregressed.
                 - New: `struct StrikeInput { ... }` (targeting/cost the card
                 needs — e.g. `targetId`, `base`; the rulebook's typed input).
                 - New: `class StrikeCard : public rpg::core::Action<StrikeInput>`
                 in `include/tumult/cards/` + `src/cards/`; `canActivate` gates
                 (target exists / alive); `activate()` drives `strike` (cost-spend
                 is a no-op placeholder until B3 energy).
                 - New: `struct CardResult { rpg::core::ActionReceipt receipt;
                 StrikeResult strike; };` (bundles card identity + the existing
                 breakdown).
                 - New: `Encounter::playCard(...)` — generic over the Action's
                 input; returns `(Status, CardResult)` (receipt populated even on
                 gate-rejection, per the rpgkit pattern + never-return-(nil,nil));
                 publishes `CardResolved` on success after mutation.
                 - New: `struct CardResolved { ... CardResult result; }` +
                 a `TopicDef<CardResolved>` bound on the Encounter Bus
                 (notification flavor, like `StrikeResolved`).
                 - New: `std::string formatCardPlay(const CardResult&)` — one-line
                 header naming the card from `receipt.id`/`type`; per-modifier
                 lines stay `formatStep`.
                 - Changed (additive): `strike()` and `StrikeResolved` gain a
                 `correlationId` (threaded from `ActivateParams`) so `playCard`
                 can match the resolved event. Default `""`; A2 callers and tests
                 that omit it are unaffected.
                 - Unchanged: rpgkit (v0.3.0, no bump); `strike()` return shape;
                 `combatants()`; `findCharacter(id)`.
                 Exact identifier / topic-id / field naming is the executor's
                 call; the shapes above are the contract.

Revisit triggers: **two**, not one — watch for either:
                 (a) **Intent-to-converge.** Parallel is a bet — "maybe a card
                 *is* a strike." If every card we add turns out to be a thin
                 `strike`-wrapper (no other op kind appears), that is the trigger
                 to revisit toward **unify** (collapse `strike` into the card path)
                 or the **data-driven** (op-list + resolver) model.
                 (b) **First non-`strike` card.** The genericity asymmetry above
                 bites the moment a card resolves to something other than a strike
                 — e.g. a pure-block card or an apply-effect card. That card's
                 outcome is not a `StrikeResult` and is not recovered from
                 `strikeResolvedTopic`, so it forces `playCard`'s recovery channel
                 and `CardResult` to generalize (or a second `play*` path). Until
                 then, leave the strike-coupled output as-is.
                 Until one of these fires, parallel + card-runs is the
                 lower-commitment shape that keeps both futures open.
