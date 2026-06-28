# tumult Wave B1 — Cards Visible: first card as an Action with a self-naming receipt

> **For agentic workers:** REQUIRED SUB-SKILL — use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to carry this wave task-by-task through merged PR(s). Steps use checkbox (`- [ ]`) tracking.
>
> **This plan is written to the conventions in `ideas/tumult/capabilities.md` § "How we work".** Tasks state **intent + acceptance criteria**, not literal `old_string`/`new_string` diffs — the executor reads the current code and chooses the specifics (an A1 retro lesson: literal diffs rot between authoring and execution). Where a type shape is the *contract* it is given exactly (from DR-011); where it is a *call* it is named as the executor's.

**Tracking issue:** tumult#9 — "B1: Cards Visible — first card as an Action with a self-naming receipt". One issue; PR(s) cite `Closes #9`.

**Decision receipt:** [DR-011 — Card-play verb](../decisions/0011-card-play-verb.md). Bake it in; do not re-open the seam choices.

**Demand proofed:** rpgkit-ue#15 (add an action without a GameMode branch) + tumult-ue#13/#14 (the card self-names on the HUD). B1 is the headless, CI-verified proof of the card verb + its self-naming receipt — **not** a UE build. Literal UE rendering defers to tumult-ue.

**Scope note (B1 folds B2):** this wave folds the original B1 (first card as an `Action` yielding an `ActionReceipt`) and B2 ("the card self-names on the HUD") into one tumult wave. The only piece B2 named that defers is the *literal UE* render; the self-naming-at-the-edge proof lands here via the extended `host_consumer` ctest.

---

## Goal (behavior-shaped, one sentence)

A host plays one Strike **card** through a `tumult::Encounter` and gets a self-naming receipt — the card names itself and every damage modifier names its source — observable both as the returned `CardResult` (pull) and as a `CardResolved` notification (push), proven headlessly by an extended `examples/host-consumer/` ctest, with `strike()` and the A2 edge unregressed.

## Architecture (the verb layer B1 adds)

B1 is a self-contained unit **in the tumult repo**. It adds a card *verb layer* on top of A2's combat *mechanism*, all plain tumult value/identity types (no host types in `include/tumult/`), exactly as ratified in DR-011:

1. **The card owns its behavior (card-runs).** `StrikeCard : rpg::core::Action<StrikeInput>` drives the strike inside `activate()`. The card is a tumult type under `include/tumult/cards/` + `src/cards/`.
2. **A generic play verb.** `Encounter::playCard(...)` is generic over the Action's input — it never switches on card kind. A new card is a new `Action` subclass, not a new `playCard` branch.
3. **One damage mechanism (parallel, not a second path).** The card reuses the existing `strike()`; HP/block mutate identically to a direct strike of the same base. `strike()` stays public and unregressed.
4. **Encounter bundles the rich outcome via correlation (the crux).** `ActionReceipt` is identity-only and `activate()`'s return is fixed, so the `StrikeResult` cannot exit through the card's return. `playCard` recovers it by reusing A2's `StrikeResolved` event as the internal channel, correlated by `correlationId`, then bundles `CardResult{ActionReceipt, StrikeResult}` and publishes `CardResolved` (return = pull, event = push — the StrikeResult/StrikeResolved pattern, one level up).
5. **Self-naming from receipts.** Card identity is read from `receipt.id`/`receipt.type` (never a hardcoded label — the `formatStep` discipline); each modifier is named from its `source` via the existing `formatStep`; a one-line `formatCardPlay` header names the card.

The **host owns the play**; tumult adds **no** loop/hand/energy primitive (those are B3). The proof vehicle is the **extended `examples/host-consumer/` ctest** that A1 established and A2 grew.

> **Both events fire on a card play — by design.** Because the card reuses `strike()`, a card play emits the underlying `StrikeResolved` (mechanism altitude) *and* a `CardResolved` (verb altitude). A host that cares about cards subscribes to `CardResolved`; the A2 strike sub still sees the underlying strike. This is expected, not double-counting — assert it, don't suppress it. **Host-side dedup of the two events (same `StrikeResult` reaches a host subscribed to both) is unspecified here and deferred to tumult-ue** — tumult's job is to emit both faithfully at their two altitudes, not to dedup for the host.

> **Genericity asymmetry (named, not generalized — see DR-011).** `playCard` is generic on the **input** side (any `Action<TInput>` plays without a type-switch) but strike-**coupled** on the **output** side (it recovers from `strikeResolvedTopic` and returns `CardResult{StrikeResult}`). That is intended for B1's single op kind. There are **two** revisit triggers: (a) if every card becomes a thin `strike`-wrapper → converge (unify / data-driven); (b) the **first non-`strike` card** (pure-block, apply-effect) forces the recovery channel + `CardResult` to generalize. **Do not pre-build a generic `CardResult<T>` or generalized recovery now** (n=1, YAGNI) — just don't wall it off.

## Contract shapes (from DR-011 — fixed shapes; identifier/topic-id naming is the executor's)

```cpp
struct StrikeInput { /* the card's typed input: targeting + (later) cost — e.g. */ std::string targetId; int base; };

class StrikeCard : public rpg::core::Action<StrikeInput> { /* canActivate gates; activate() drives strike */ };

struct CardResult   { rpg::core::ActionReceipt receipt; StrikeResult strike; };   // card identity + the existing breakdown
struct CardResolved { /* … */ CardResult result; };                              // notification flavor, like StrikeResolved

// On Encounter (generic over the Action's input):
std::pair<rpg::core::Status, CardResult> playCard(/* Action<TInput>&, EntityRef owner, TInput input */);

// Self-naming header (per-modifier lines stay formatStep):
std::string formatCardPlay(const CardResult& result);

// Additive widening of the A2 edge for correlation:
//   strike() and StrikeResolved gain a correlationId (default ""); A2 callers/tests that omit it are unaffected.
```

## Tech / env contract

C++20, CMake ≥ 3.25 (Ninja presets `debug`/`release`), rpgkit `rpg::core` header-only at **v0.3.0 — untouched** (`Action<TInput>`, `ActionReceipt`, `EntityRef`, `ActivateParams::correlationId`, `Topic<T>` all already ship there; **no bump, no tag, no local replace**). GoogleTest for unit tests. The extended `host_consumer` uses only `tumult::` + `rpg::core::` types — no UE, no host types.

---

## Per-wave checklist (capabilities.md § "How we work" — filled for B1)

- [ ] **Goal behavior named + demand cited:** a host plays one Strike card via `playCard`, gets a self-naming `CardResult` + `CardResolved`, HP mutates as a direct strike would. Proofs **rpgkit-ue#15** + **tumult-ue#13/#14**.
- [ ] **Primitive(s)/seam(s):** the **card verb layer** — `StrikeCard : Action`, generic `playCard`, `CardResult`/`CardResolved`, correlation-bundle. Reuses the `strike` mechanism + `Topic<T>` notifications + rpgkit's `correlationId`; **no new rpgkit primitive**.
- [ ] **TDD / observable done-when:** unit tests for the card (play→mutate→names; gate-rejection) and `playCard`/`CardResolved` first; then the extended `host_consumer` ctest plays a card. `ctest` green locally and in CI; `make pre-commit` green; never `--no-verify`.
- [ ] **DR:** DR-011 (card-play verb) recorded in `decisions/` as part of this wave (lands via the rpg-project planning PR).
- [ ] **Boundary held — no host types in `include/tumult/`:** `StrikeCard`, `StrikeInput`, `CardResult`, `CardResolved` are plain tumult types; any rendering lives in the example/test, not the library.
- [ ] **Stop-and-ask** if any acceptance criterion forces a UE dependency or an rpgkit change — neither should be needed; if one appears, halt and report (it contradicts DR-011).

---

## Tasks (intent level — executor reads current code, picks specifics)

### Task 1 — Issue, fresh branch, green baseline

- [ ] Confirm tumult#9 is OPEN (do **not** create another issue).
- [ ] Start from fresh `main` (`git checkout main && git pull`); cut a branch off it (e.g. `feat/b1-card-verb`). **Never reuse an old branch.**
- [ ] Verify the baseline is green before any change: configure, build, full `ctest` pass. This is the clean baseline the wave builds on (no commit yet).

**Acceptance:** issue open; fresh branch from current main; baseline `ctest` 100% pass.

### Task 2 — `StrikeInput` + `StrikeCard : Action` (TDD)

- [ ] **Goal:** add a `StrikeInput` (the card's typed input — targeting now, cost later) and `StrikeCard : rpg::core::Action<StrikeInput>` under `include/tumult/cards/` + `src/cards/`. `canActivate(owner, input)` gates (target exists and is alive). `activate(params)` drives the strike under the hood and returns an `ActionReceipt` naming the card; cost-spend is a no-op placeholder (energy is B3). **Card→Encounter wiring:** `StrikeCard` is constructed with a reference/handle to the owning `Encounter` (e.g. `StrikeCard(Encounter&, ...)`) so its `activate()` can reach `strike()`; keep the targeting/cost data in `StrikeInput`, not the constructor. Wire the new `src/cards/*.cpp` into the build (`tumult_objects` and the `host_consumer` source list, mirroring how the effects sources are listed).
- [ ] Test first: `canActivate` returns ok for a live target and an error for a missing/dead target; `activate` against a live target produces an `ActionReceipt` whose `id`/`type` name the card; activating drives a strike whose effect on state matches a direct `strike` of the same base.

**Acceptance:** card unit tests pass; the card names itself via its receipt; the gate rejects missing/dead targets without mutating state; existing tests stay green.

### Task 3 — `Encounter::playCard` + `CardResult` + `CardResolved` (TDD)

- [ ] **Goal:** add `CardResult{ActionReceipt receipt; StrikeResult strike;}`, a `CardResolved` notification + its `TopicDef` bound on the Encounter Bus (mirror the `StrikeResolved` definition pattern in `encounter.hpp`), and `Encounter::playCard(...)` — **generic over the Action's input**, returning `(Status, CardResult)`. `playCard`: gate via `canActivate`; thread a `correlationId` into the card's `activate()`; recover the matching `StrikeResolved` (published synchronously during `activate()`) to obtain the `StrikeResult`; bundle `CardResult`; on success, publish `CardResolved` **after** the mutation. Thread a defaulted `correlationId` through `strike()` and onto `StrikeResolved` so the match is robust (additive; A2 callers that omit it are unaffected).
- [ ] Test first: playing a Strike card mutates HP/block **identically** to a direct `strike` of the same base; the returned `CardResult.receipt` names the card and `CardResult.strike.breakdown` names each modifier from its `source` (apply a `VulnerableEffect` and assert the `vulnerable` modifier is named under a card play); a subscriber to `CardResolved` registered **before** the play receives exactly one event matching the returned `CardResult`; gate-rejection (missing/dead target) returns an error `Status` with the receipt still naming the card and **no** mutation and **no** `CardResolved`; the underlying `StrikeResolved` still fires (A2 unregressed) and the `strike()` return path is unchanged.

**Acceptance:** `playCard` unit tests pass; pull (`CardResult`) and push (`CardResolved`) agree; correlation recovers the right `StrikeResult`; gate-rejection path asserted; A2/A1 events + tests unregressed.

### Task 4 — `formatCardPlay` self-naming header (TDD)

- [ ] **Goal:** add `std::string formatCardPlay(const CardResult&)` (beside `formatStep` in `breakdown.hpp` or a sibling) that renders a one-line header naming the card **from `receipt.id`/`receipt.type`** (never a hardcoded label), with the per-modifier lines produced by the existing `formatStep`.
- [ ] Test first: the header contains the card's identity read from the receipt (changing the card's id/type changes the rendered header — proving it is read, not hardcoded); the modifier lines are exactly what `formatStep` produces for the bundled breakdown.

**Acceptance:** `formatCardPlay` unit test passes; the card name in the rendered output comes from the receipt; modifier rendering is unchanged from `formatStep`.

### Task 5 — Extend `examples/host-consumer/` into the card-play proof

- [ ] **Goal:** grow the `host_consumer` (or add a clearly-named card-play phase / second example registered as its own ctest — executor's call) so it: constructs a `StrikeCard`, subscribes **once** to `CardResolved` before playing, plays the card via `playCard`, and renders the result via `formatCardPlay` + `formatStep`. Apply a `VulnerableEffect` to the target so a modifier is present to be named. Use only `tumult::` + `rpg::core::` types. Register via `add_test` so the **existing** CI ctest jobs run it (no `ci.yml` change — A1 wired `TUMULT_BUILD_EXAMPLES`).
- [ ] Assert the acceptance outcomes (below) and return nonzero on any mismatch.

**Acceptance (the extended `host_consumer` asserts, as outcomes):**
- A host plays **one** Strike card via `playCard`; the returned `CardResult` names the card **from the receipt** (not a hardcoded string) AND names each damage modifier **from its `source`** (the `vulnerable` modifier is named under the card play).
- HP/block mutate **identically** to a direct `strike` of the same base (the card rides the one mechanism — assert equality against the A2 strike numbers).
- Exactly **one `CardResolved`** per play via the single pre-loop/-play subscription; observed fields match the returned `CardResult`.
- The underlying **`StrikeResolved` still fires** for the same play (different altitude) — asserted, not suppressed.
- `canActivate` **gate-rejection** (e.g. a dead/absent target) returns an error and mutates nothing.
- `formatCardPlay` renders a header naming the card from the receipt; `formatStep` still self-names every modifier.
- No regression: `strike()`, `StrikeResolved`, `combatants()`, and all prior tests stay green.

### Task 6 — Docs in the same PR

- [ ] **Goal:** update tumult `docs/status.md` to mark A2 done and B1 (Cards Visible) as the active edge, describing the card verb (`StrikeCard`, `playCard`, `CardResult`/`CardResolved`, `formatCardPlay`) and the parallel-reuses-`strike` relationship. If the card surface becomes part of the documented consumption recipe, reflect it in `docs/host-integration.md` too. If a "known rough edge" emerges (e.g. the correlation window, the both-events-fire behavior), note it.

**Acceptance:** status doc names the card verb surface as host-facing and the card-play-with-self-naming-receipt as the proven behavior; the parallel/reuse relationship to `strike` is stated.

### Task 7 — Gate, push, PR (per repo workflow)

- [ ] Full pre-commit gate: `make pre-commit` (lint over `include src tests` + full `ctest`). **Never `--no-verify`.** Note `examples/` is outside `FMT_FILES`, so the example is gated by **build + run** in CI, not local fmt/tidy.
- [ ] Run the examples + tests under **ASan/UBSan** locally before pushing (the A2-retro convention — the card adds a correlation subscribe/unsubscribe window worth sanitizing). Treat any sanitizer finding as a stop-and-fix.
- [ ] Push the branch; open **one** PR citing the demand and `Closes #9`. Wait for CI (build-linux clang++/g++, build-windows MSVC, lint) green + Copilot review; reply on each Copilot thread with validity + action. **Do not merge** — the director merges after review.

**Acceptance:** one PR open, CI green, ASan/UBSan clean locally, Copilot threads addressed, linked to tumult#9; merge gated to the director.

---

## Env / CI authority note (A1 + A2 retro lessons)

**CI is the authoritative gate, not local pre-commit.** Locally, `clang-format` / `clang-tidy` may be absent or a different version — do not thrash on env-only lint failures; trust CI's `lint` job for the verdict. State the goal as **"the card-play proof builds and runs green as a ctest"** and let the executor pick the generator/toolchain (the presets are a convenience, not the contract). The proof **must** be an `add_test`/ctest so it runs in build-linux (clang++ & g++) and build-windows (MSVC) — exactly as A1/A2 wired it. **New since A2:** run the examples + unit tests under **ASan/UBSan** before pushing; do **not** claim done on local green alone.

## rpgkit impact

**None.** `Action<TInput>`, `ActionReceipt`, `EntityRef`, `ActivateParams::correlationId`, and `Topic<T>` all already ship in rpgkit v0.3.0; the card verb, bundle, and correlation are pure tumult. No bump, no tag, no local replace. (The only alternative that would touch rpgkit — enriching `ActionReceipt` to carry the breakdown — was rejected in DR-011 on the boundary rule.)

## Stop-and-ask triggers

- Any acceptance criterion appears to force a UE build or an rpgkit change → halt and report (contradicts DR-011; the boundary is settled).
- The correlation cannot recover the `StrikeResult` cleanly (e.g. `StrikeResolved` turns out not to publish synchronously within `activate()`) → halt and report; do not invent a stateful side-channel without a DR.
- A permission prompt blocks a needed command → stop and report; do not work around it.
