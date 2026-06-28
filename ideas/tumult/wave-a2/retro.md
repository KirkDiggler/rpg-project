# tumult Wave A2 — First Loop: retro (closes the wave)

_2026-06-28. This retro is what closes A2 — not the green PR. It records decisions reviewed with Kirk; recorded faithfully, not re-derived._

## Outcome — delivered

- tumult **PR #8 merged**; tracking issue **tumult#7 closed**; board #17 item **#7 → Done** (Observability & Receipts / First Loop).
- **Signoff bar:** the CI-verified extended `host_consumer` ctest — pure engine, no playtest. A2's goal behavior is observable headless: the loop reads HP from the read-model, drives the goblin 20→0, marks it dead from the `alive` flag, observes one `StrikeResolved` per strike, and the breakdown still self-names from receipts.

## Handoff verdict — the experiment is working

The written plan (`wave-a2/plan.md` + DR-010) **carried the wave end-to-end**. A fresh session executed it with only operational guardrails and self-verified rigorously — it deleted the `strike()` publish and confirmed **both the unit tests and `host_consumer` went red**, then restored it and confirmed green. The artifacts carried enough; the handoff/autonomy goal held on this wave.

## Design decisions reviewed with Kirk (this retro)

### a. Read-model shape — ENDORSED as-is

Keep the shipped shape: a value-snapshot `std::vector<CombatantView{id, name, curHp, maxHp, alive}>` + retained `findCharacter` + the `StrikeResolved` event. The split is intentional: **pull** (`combatants()`) for current truth, **push** (`StrikeResolved`) for what-just-changed.

Alternatives weighed and rejected:
- **Live refs / `const Character*` / `span`** — leak internals (incl. `block`), lifetime-coupled to mutation, and make the host bind to the engine's own type.
- **Per-id getters** — chatty, require the host to already know the id list, no enumeration.
- **Visitor / `forEach`** — zero-copy but inverts control; awkward for UE list-binding.
- **Rich encounter-snapshot bundle** — premature; grow `CombatantView` (or add a bundle) when turn/encounter state actually needs exposing.
- **Push-only, no read-model** — fragile state reconstruction on the host side.

### b. `block` deliberately deferred from `CombatantView`

Conscious YAGNI, **not** an oversight. `block` is a core Across-the-Obelisk mechanic and `StrikeResult.blocked` already exists, so the information isn't lost. We add `block` to the view when a block-display wave needs it, rather than widen the curated edge speculatively. _(Decided by Kirk, 2026-06-28.)_

## Known seam (recorded, not fixed)

`strike()` `(void)`-ignores the `StrikeResolved` publish `Status` — a subscriber's error is invisible to the striker. Deliberate: a notification is a fire-and-forget broadcast and `strike` has no error channel. Revisit if/when a subscriber can meaningfully fail.

## Footgun (recorded)

`combatants()` returns a temporary vector; taking a pointer into it dangles. Hit during A2, surfaced by Windows MSVC + ASan, fixed by returning `std::optional<CombatantView>` by value in the example's `findView`. The modeled pattern is optional / copy-by-value; the C++ API can't prevent the pointer-into-temporary mistake.

## Convention added (pulled by pain)

**Run examples under ASan/UBSan locally before push.** The dangling-into-temporary bug was invisible on Linux release / `make` and only surfaced under MSVC-debug / ASan. Running examples under sanitizers locally catches this class pre-CI. (Folded into `capabilities.md` § "How we work".)
