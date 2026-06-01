# Take-Action Wave — Decision Ledger

**Wave:** TakeAction (Chapter 2: Combat Verbs, board #13, umbrella rpg-project #54)
**Purpose:** Running log of decisions made *during execution* of this wave, each with its *why*.
This is the **retro input** — at wave close we walk this list together and mark each verdict
(Keep / Improve / Drop), then carry the lessons into the next verb wave.

> Design-phase decisions (resolved with Kirk during the 2026-06-01 3-lens panel) live in
> `design.md`'s resolved-decisions table. This ledger captures director/execution decisions
> made *from kickoff forward*. It grows incrementally — do not reconstruct it at the end.

Legend — **Status:** Adopted · Pending Kirk · Deferred · Superseded.
**Retro verdict** left blank until the wave-close retro.

---

## D1 — Operating model
**Decision:** The wave runs director-led and autonomous. Director stays at director altitude
(agents/teams as sensors and implementers; no hands-on code work), surfaces real judgment calls
to Kirk early, and logs every decision here. The wave is closed by a **participatory retro after an
MCP playtest**, not by green CI.
**Why:** Kirk's kickoff framing + established director process. Keeps the narrator reliable.
**Status:** Adopted · **Retro:**

## D2 — Wave scope
**Decision:** "Take action" = make a character take any *available* action through the unified
`TakeAction` path; first verb-shaped wave of Chapter 2. Confirmed against umbrella #54 and the
validated wave design doc.
**Why:** Board + design doc already define it; code reconnaissance confirmed the design holds.
**Status:** Adopted · **Retro:**

## D3 — Canonical path: one general registry
**Decision:** The toolkit's feature factory (`rulebooks/dnd5e/features/factory.go`) is canonical.
The hardcoded attack gate (`encounter/combat_phased.go:66`, `if ref.ID != "attack" → error`)
conforms to it. #697 builds a **general, ref-keyed action registry** — no ref is special-cased.
Attacks resolve off the existing `actions.Action` interface + weapon/class data, not flat snapshot stats.
**Why:** "Default to one system." The data-driven, class-aware registry already exists for abilities;
the attack path is the lone hardcoded holdout. Make the attack verb a citizen of the registry that
already exists rather than designing a parallel system.
**Status:** Adopted · **Retro:**

## D4 — Ref phasing: spine first, catalog second
**Decision:** Full definition-of-done = `TakeAction` covers **all known L1 action refs** — but NOT
out of the gate.
- **Beat 1 (spine):** general registry + economy deduction + resolved-action event + server-pushed
  menu + web render, proven with the minimal ref set: `attack` (action) + Monk Martial Arts strike
  (bonus action) + `dodge` (one non-attack action ref, to prove generality). MCP playtest verifies.
- **Beat 2 (catalog):** remaining known refs (`dash`, `disengage`, `help`, `hide`, `move`…) added as
  factory entries on the proven spine.
**Why:** The wave's risk is the *spine*, not the *breadth* of refs. Once the registry is general,
adding refs is low-risk data entry. Build the catalog on a playtested foundation, not an unproven one.
Kirk-initiated ("all known refs"), director-phased.
**Status:** Adopted · **Retro:**

## D5 — Movement is an action ref
**Decision:** `move` is not a special system — it's an action ref like any other, seeded in Beat 2.
The hex-vs-feet representation (North Star §7 deferral) is decided *when `move` is seeded*, not at the
gate. Beat 1 emits no movement axis.
**Why:** Don't decide the unit before the ref exists; keeps Beat 1 unblocked and avoids a contract
decision under deadline pressure.
**Status:** Adopted · **Retro:**

## D6 — Idempotency deferred
**Decision:** Retried-`TakeAction` double-decrement of the economy is out of scope for this wave.
File a follow-up issue under umbrella #54; revisit post-playtest.
**Why:** Won't surface in a controlled MCP playtest; idempotency keys are their own unit of work.
**Status:** Adopted (overridable) · **Retro:**

## D7 — Web client-side gating removed
**Decision:** The client-side adjacency/range gating in `combat-v2/panels/ActionPanel.tsx` is removed
this wave, replaced by server-provided `unavailable_reason` on the action menu. The hardcoded "Attack"
button is replaced by rendering the server-pushed `AvailableAction` list.
**Why:** It's the one real boundary violation today (web computing legality). This wave exists to move
that to the server.
**Status:** Adopted · **Retro:**

## D8 — Board workflow
**Decision:** Carry over board #12's Status workflow — Todo / In Progress / In Review / Done — with
PRs awaiting Kirk's merge parked in **In Review**. PR↔issue linking via `Closes #N`.
**Why:** Established convention; no reason to diverge.
**Status:** Adopted · **Retro:**

## D9 — Dependency order (technical constraint)
**Decision:** protos #170 (resolved-action event type) + toolkit #697 (struct fields) land **before**
rpg-api #597 stops suppressing the event — otherwise `ErrUnknownEventType`. Critical path:
protos #170 + toolkit #697 → api #597 → web #426; #594 (attack-miss visibility) rides along.
**Why:** Hard runtime dependency surfaced in code reconnaissance.
**Status:** Adopted · **Retro:**
