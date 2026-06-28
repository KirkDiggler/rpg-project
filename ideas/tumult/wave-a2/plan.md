# tumult Wave A2 — First Loop: host-observable encounter

> **For agentic workers:** REQUIRED SUB-SKILL — use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to carry this wave task-by-task through merged PR(s). Steps use checkbox (`- [ ]`) tracking.
>
> **This plan is written to the conventions in `ideas/tumult/capabilities.md` § "How we work".** Tasks state **intent + acceptance criteria**, not literal `old_string`/`new_string` diffs — the executor reads the current code and chooses the specifics (an A1 retro lesson: literal diffs rot between authoring and execution). Where a type shape is the *contract* it is given exactly (from DR-010); where it is a *call* it is named as the executor's.

**Tracking issue:** tumult#7 — "A2: First Loop — host-observable encounter (StrikeResolved event + combatant read-model)". One issue; PR(s) cite `Closes #7`.

**Decision receipt:** [DR-010 — Host-observation edge](../decisions/0010-host-observation-edge.md). Bake it in; do not re-open the seam choices.

**Demand proofed:** tumult-ue#5 (host displays HP read from a tumult Encounter). A2 is the headless, CI-verified proof of that edge — **not** a UE build.

---

## Goal (behavior-shaped, one sentence)

A host runs a minimal combat loop against a `tumult::Encounter`: it reads every combatant's HP from a read-model each iteration, strikes until the goblin reaches 0 HP and is marked dead, and observes each resolved strike via a single bus subscription whose breakdown still self-names from receipts.

## Architecture (the edge A2 adds)

A2 is a self-contained unit **in the tumult repo**. It adds two host-facing surfaces to `Encounter`, both plain tumult value types (no host types in `include/tumult/`), exactly as ratified in DR-010:

1. **`StrikeResolved` notification.** `strike(...)` publishes a `StrikeResolved` event on the encounter Bus **after** it mutates state, and **still returns `StrikeResult` unchanged**. Return = the caller's synchronous answer (pull); event = the broadcast an observer subscribes to once and reacts to each loop iteration (push). Event payload reuses `StrikeResult` as the single outcome shape. Same `Topic<T>` notification primitive already used by `turn.ended`.
2. **`combatants()` read-model.** A snapshot accessor returns all combatants as a value copy of a host-type-free view. The host enumerates and renders current HP each iteration without pre-knowing ids and without holding a live pointer into internal state. `findCharacter(id)` stays for point lookups.

"Dead" is the read-model's `alive` flag (`curHp == 0` → not alive) — **no** death event in A2 (deferred to Group L). The **host owns the loop**; tumult adds **no** loop primitive. The proof vehicle is the **extended `examples/host-consumer/` ctest** that A1 established.

## Contract shapes (from DR-010 — fixed; identifier/topic-id naming is the executor's)

```cpp
struct CombatantView { std::string id; std::string name; int curHp; int maxHp; bool alive; };
struct StrikeResolved { std::string attackerId; std::string targetId; StrikeResult result; };

// On Encounter:
[[nodiscard]] std::vector<CombatantView> combatants() const;   // value snapshot of all combatants
// strike() return type UNCHANGED; it additionally publishes StrikeResolved (after mutation).
```

## Tech / env contract

C++20, CMake ≥ 3.25 (Ninja presets `debug`/`release`), rpgkit `rpg::core` header-only at **v0.3.0 — untouched** (`Topic<T>`, `TopicDef<T>::on(bus)`, `publish` already ship there and are in use; **no bump, no tag**). GoogleTest for unit tests. The extended `host_consumer` uses only `tumult::` + `rpg::core::` types — no UE, no host types.

---

## Per-wave checklist (capabilities.md § "How we work" — filled for A2)

- [ ] **Goal behavior named + demand cited:** host runs a loop, reads HP from the read-model, observes each strike; goblin 20→0 and dead. Proofs **tumult-ue#5**.
- [ ] **Primitive(s)/seam(s):** the **host↔encounter observation edge** — `StrikeResolved` notification + `CombatantView` snapshot read-model. No new rpgkit primitive (reuses `Topic<T>` + the mirror-not-wrap pattern, T3).
- [ ] **TDD / observable done-when:** unit tests for `combatants()` and `StrikeResolved` first; then the extended `host_consumer` ctest runs the full loop. `ctest` green locally and in CI; `make pre-commit` green; never `--no-verify`.
- [ ] **DR:** DR-010 (host-observation edge) recorded in `decisions/` as part of this wave (lands via the rpg-project planning PR).
- [ ] **Boundary held — no host types in `include/tumult/`:** `CombatantView` / `StrikeResolved` are plain structs; the loop + any rendering live in the example/test, not the library.
- [ ] **Stop-and-ask** if any acceptance criterion forces a UE dependency or an rpgkit change — neither should be needed; if one appears, halt and report (it contradicts DR-010).

---

## Tasks (intent level — executor reads current code, picks specifics)

### Task 1 — Issue, fresh branch, green baseline

- [ ] Confirm tumult#7 is OPEN (do **not** create another issue).
- [ ] Start from fresh `main` (`git checkout main && git pull`); cut a branch off it (e.g. `feat/a2-first-loop`). **Never reuse an old branch.**
- [ ] Verify the baseline is green before any change: configure, build, full `ctest` pass. This is the clean baseline the wave builds on (no commit yet).

**Acceptance:** issue open; fresh branch from current main; baseline `ctest` 100% pass.

### Task 2 — `combatants()` read-model (TDD)

- [ ] **Goal:** add a `CombatantView` value struct and `Encounter::combatants()` returning a value snapshot of **all** combatants (`id, name, curHp, maxHp, alive`), where `alive == (curHp > 0)`. Keep `findCharacter(id)` unchanged. Do **not** expose internal-only fields (e.g. `block`).
- [ ] Test first: a caller with no prior id knowledge enumerates both combatants; the snapshot reflects current HP and the `alive` flag; after a strike that drops the goblin to 0, the next `combatants()` call reports the goblin not-alive; the returned value is independent of later internal mutation (it's a snapshot, not a live reference).

**Acceptance:** the read-model unit test passes; the snapshot curates display fields and reflects post-strike state; existing tests stay green.

### Task 3 — `StrikeResolved` notification from `strike()` (TDD)

- [ ] **Goal:** define `StrikeResolved{attackerId, targetId, StrikeResult result}` and a notification `TopicDef<StrikeResolved>` bound on the Encounter Bus (mirror the `turn.ended` / `combatDamageTopic()` definition pattern already in `encounter.hpp`). `strike(...)` publishes it **after** mutating HP/block, and **returns `StrikeResult` unchanged**.
- [ ] Test first: a subscriber registered **before** the strike receives exactly one event whose `result` matches the returned `StrikeResult` (finalDamage / blocked / hpAfter / breakdown); a subscriber that reads `combatants()` inside its handler sees **post-strike** HP (proves publish-after-mutation ordering); the direct `strike()` return value is unchanged (no regression).

**Acceptance:** the event unit test passes; ordering (after-mutation) is asserted; the return path is unregressed; all existing encounter/integration tests stay green.

### Task 4 — Extend `examples/host-consumer/` into the First-Loop proof

- [ ] **Goal:** grow the A1 `host_consumer` (or add a clearly-named second example registered as its own ctest — executor's call) so it: subscribes **once** to `StrikeResolved` before the loop; runs a loop of strikes; each iteration reads `combatants()` and renders/inspects HP; continues until the read-model reports the goblin not-alive.
- [ ] Assert the acceptance outcomes (below) and return nonzero on any mismatch. Use only `tumult::` + `rpg::core::` types. Register via `add_test` so the **existing** CI ctest jobs run it (no `ci.yml` change — A1 already wired `TUMULT_BUILD_EXAMPLES`).

**Acceptance (the extended `host_consumer` asserts, as outcomes):**
- Enumerates all combatants via the read-model with **no hardcoded id list** for rendering; reads curHp/maxHp/alive each iteration.
- The strike loop drives the goblin **20 → 0**: HP monotonically non-increasing, reaches exactly 0.
- When curHp hits 0 the read-model reports the goblin **not-alive**; the loop terminates on that signal.
- Exactly **one `StrikeResolved` per strike** via the single pre-loop subscription; observed fields match the returned `StrikeResult`.
- A handler reading `combatants()` sees **post-strike** HP.
- The observed/returned breakdown **still self-names every modifier** via `formatStep` (A1's simplification proof, preserved under the loop).
- No regression: the `StrikeResult` return path and all prior tests stay green.

### Task 5 — Docs in the same PR

- [ ] **Goal:** update tumult `docs/status.md` to mark A1 done and A2 (First Loop) as the active edge, describing the two new host-facing surfaces. If the read-model / event become part of the documented consumption recipe, reflect them in `docs/host-integration.md` too.

**Acceptance:** status doc names the `combatants()` read-model + `StrikeResolved` subscription as host-facing surface and the loop as the proven behavior.

### Task 6 — Gate, push, PR (per repo workflow)

- [ ] Full pre-commit gate: `make pre-commit` (lint over `include src tests` + full `ctest`). **Never `--no-verify`.** Note `examples/` is outside `FMT_FILES`, so the example is gated by **build + run** in CI, not local fmt/tidy.
- [ ] Push the branch; open **one** PR citing the demand and `Closes #7`. Wait for CI (build-linux clang++/g++, build-windows MSVC, lint) green + Copilot review; reply on each Copilot thread with validity + action. **Do not merge** — the director merges after review.

**Acceptance:** one PR open, CI green, Copilot threads addressed, linked to tumult#7; merge gated to the director.

---

## Env / CI authority note (A1 retro lesson)

**CI is the authoritative gate, not local pre-commit.** Locally, `clang-format` / `clang-tidy` may be absent or a different version — do not thrash on env-only lint failures; trust CI's `lint` job for the verdict. State the goal as **"`host_consumer` builds and runs green as a ctest"** and let the executor pick the generator/toolchain (the presets are a convenience, not the contract). The proof **must** be an `add_test`/ctest so it runs in build-linux (clang++ & g++) and build-windows (MSVC) — exactly as A1 wired it. Do **not** claim done on local green alone.

## rpgkit impact

**None.** `Topic<T>` (notification), `TopicDef<T>::on(bus)`, and `publish` already ship in rpgkit v0.3.0 and are already used in tumult (`turn.ended` is a `Topic<int>`). The read-model is pure tumult. No bump, no tag, no local replace.

## Stop-and-ask triggers

- Any acceptance criterion appears to force a UE build or an rpgkit change → halt and report (contradicts DR-010; the boundary is settled).
- A permission prompt blocks a needed command → stop and report; do not work around it.
