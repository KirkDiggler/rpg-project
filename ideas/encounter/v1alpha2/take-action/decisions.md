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

## D10 — Team model: accountable expert-owners
**Decision:** Team-members operate as **accountable domain experts who both advise and implement
their lane** (advisor + implementer fused), owning their repo end-to-end through the wave and
**refusing lane-violating briefs — even from the director**. The web + protos charters are leveled up
to the toolkit/api parity bar (duty-to-push-back/REFUSE clause, 4-question done-gate, director-only
guardrails); a `roles/README` codifies the two-tier model (standing *member* vs dispatched *fixer*)
and the expert-ownership standard.
**Why:** Kirk's "lean into responsibility / experience being experts." Reconnaissance showed the
toolkit/api charters had already fused the model while web/protos lagged at ~half strength — equal
lanes deserve equal teeth. The expert *is* the boundary; pushback is expertise made active.
**Status:** Adopted · **Retro:**

## D11 — Ledger sync cadence
**Decision:** The decision ledger is updated in the working tree as decisions are made (living record)
and synced to origin via a **batched "ledger sync" PR at natural checkpoints** (per beat / when several
accumulate), not one PR per decision.
**Why:** Keeps the ledger current without PR spam; it's the retro's source of truth, so it must stay
live locally between syncs.
**Status:** Adopted · **Retro:**

## D12 — Beat-1 dispatch: experts own the contract convergence
**Decision:** Stood up a live wave team (`take-action-beat1`: toolkit · protos · api · web experts + director).
**toolkit #697 leads as contract author**; toolkit + protos converge on the Beat-1 semantic contract
(resolved-action event, AvailableActions menu, ActionEconomy) **themselves** via team messaging and report
the agreed shape to the director for sign-off **before** heavy implementation. api #597 and web #426 pipeline
behind once the contract is real. Experts persist PR-to-merge.
**Why:** Leaning into expert ownership (D10) — the experts own the cross-repo contract convergence; the
director judges, doesn't author. The contract is the highest-leverage thing to get right; locking it once
avoids 4-repo rework.
**Status:** Adopted · **Retro:**

## D14 — Merge cadence: director drives gated merges
**Decision:** Per Kirk's standing merge-authority grant, the director **merges any PR that has passed its
review gate** (Copilot for code repos / independent review for protos+project) **AND has CI green** —
logging each merge here — rather than bringing every merge-ready PR to Kirk. Genuine merge risks are still
escalated. PRs Kirk personally authored (e.g. #698) are left for him unless he delegates. Kirk can
flag-to-override anytime.
**First exercise:** protos **#171** (the contract) — independent gate READY / 0 must-fix + CI green +
additive (buf breaking clean) → merged; issue #170 → Done.
**Why:** Keeps the merge-heavy Beat-1 pipeline from stalling on director↔Kirk round-trips; the autonomy
grant exists for exactly this. Gated + logged keeps it safe and visible.
**Status:** Adopted · **Retro:**

## D13 — Beat-1 contract & plan: green-lit
**Decision:** Signed off the toolkit+protos-converged contract + toolkit's impl plan. Contract is additive
(v1alpha2, buf lint/breaking clean, no breaking):
- `correlation_id` on the event envelope (Inv 8); envelope timestamp sourced from toolkit `occurred_at`
  game-time (Inv 5).
- `ActionResolved` (umbrella beat: actor, `Ref action_ref`, target, `EconomyConsumed`) — one per action.
- `AttackResolved` (per-attack roll detail) — **fires on miss** (#594 fix; the variant whose absence threw
  `ErrUnknownEventType`, D9).
- `TurnStateChanged` — separate push of the recomputed TurnState (economy + menu); kept off `ActionResolved`
  on the **audience-seam** (available_actions is the actor's menu, projected per Inv 6).
- `AvailableAction` += `economy_slot` + `target_kind` enums.
- Refs carry toolkit's **two native namespaces verbatim** (`combat_abilities:*` intent, `actions:*` doing)
  — no flattening (a remap would be an Inv-2 violation). `action_ref` is a `Ref`/triple on the wire; api
  splits at translate.
**Refines D3:** the "general registry" is NOT a new build — it's the character pkg's existing
`ActivateAbility`/`ExecuteAction` dispatch. The encounter deletes its hardcoded `ref.ID=="attack"` gate and
**delegates** (default-to-one-system). The plan also kills the rpg-api `ActionEconomyData{1,1,1}` Inv-2 leak
by calling `char.StartTurn` at turn boundaries.
**Sequencing:** protos #170 lands first; toolkit's Beat-1 PR stacks on #698 and references #697 (NOT Closes
— Beat-2 catalog remains).
**Why:** Derived from validated North-Star invariants; boundary-faithful; the experts converged peer-to-peer
and the director judged the result sound. Additive/reversible pre-merge → green-lit to keep momentum with a
Kirk veto window.
**Status:** Adopted · **Retro:**

## D15 — Beat-1 menu surface: expose the engine's full ability set
**Decision:** The Beat-1 menu surfaces the character's **full verbatim ability set** — Attack, Dash, Dodge,
Disengage (+ class Ki features) — NOT just the D4 "minimal set." Dash & Disengage are real `combatabilities`
the general delegation handles for free, so they're `available=true` (genuinely working) — bonus generality
proof, and Kirk's "all known refs" direction arriving early at zero cost. **Only `Move` is gated
`available=false`** (movement deferred, D5/D14); nothing else is filtered.
**Refines D4:** the minimal ref set (attack + martial-arts + dodge) was the TEST/proof target; the actual
playtest menu is the engine's full truth.
**TargetKind = 6 values** `{UNSPECIFIED, SELF, SINGLE_ENTITY, POSITION, AREA, NONE}`. Toolkit (semantics
owner) ruled `SELF ≠ NONE` (Dodge=SELF→self-confirm vs Dash=NONE→no prompt); the proto mirrors the toolkit
enum **1:1** so api stays field-for-field (dropping NONE would force an Inv-2 remap). **This reverses the
earlier "drop NONE" refinement** — correctly: shape-aesthetics had overridden a real semantic distinction.
**Why:** "Default to one system" / expose verbatim, don't filter; forcing refs unavailable would add
suppression logic. The expert (rules owner) corrected a contract detail against ground truth.
**Status:** Adopted · **Retro:**

## D16 — Toolkit Beat-1 ships as two module PRs (char-pkg → encounter)
**Decision:** Beat-1's toolkit work spans two modules — `rulebooks/dnd5e` (EconomySlot/TargetKind enums +
menu builders) and `encounter` (turn-start seeding + `ActorTurnState` query + the delegation) — with a HARD
publish dependency: the `encounter` module requires a **tagged** `rulebooks/dnd5e`, so it can't see the
char-pkg changes without a (forbidden-to-commit) replace. Ship as **two sequential PRs**: char-pkg first
(from main, independent of #698) → merges → CI tags; then the encounter PR (stacks on #698 **and** requires
the new char-pkg tag). NOT a coordinated single-PR multi-module release — toolkit's CI for ordered
multi-module tagging is unverified, and module-at-a-time is the documented model.
**Why:** Each PR is independently CI-green; the module-publish dependency is real, and the split makes it
honest rather than betting on unverified release automation. Two PRs forced by a genuine dependency — not
gratuitous fragmentation.
**Status:** Adopted · **Retro:**

## D17 — Move availability: the encounter composes effective-takeability
**Decision:** The wire `AvailableAction.available` means **effective takeability** ("should the UI offer this
now?"), NOT raw rules-permission. So Move is `available=false` + `unavailable_reason` ("movement lands in
Beat-2") this beat — authored by the **encounter** (which owns beat-scope), NOT the character package. The
character menu stays honest (`CanUse=true`: a L1 char genuinely can move); the encounter composes
rules-availability + beat-availability into the wire verdict. The contract's `unavailable_reason` field
exists precisely for "in your menu but not takeable now + why."
**Rejected (i)** menu-honest / verb-rejects: it makes the server contradict itself — menu says `available`,
verb rejects → a Move button that errors on click, or web adds logic to hide it (no-logic-in-web violation).
(ii) keeps menu↔verb consistent; web renders it disabled with the reason, zero web logic.
**Not the attack-gate backslide:** that was a permanent "only attack works" gate; this is a temporary,
reasoned, beat-scoped marker (ideally a small deferred-ref set; move is its one Beat-1 member), removed in
Beat-2 by dropping the entry.
**Refines D5/D14/D15:** "no filtering" = don't REMOVE refs; marking `available=false`+reason is
exposing-with-true-availability, not filtering. Both layers (char rules-availability, encounter
beat-availability) stay honest.
**Status:** Adopted · **Retro:**

## D18 — Keep the runaway's Help/Hide (#702/#703)
**Decision:** The runaway toolkit agent merged Help/Hide combat abilities (#702) + flowed them through
TakeAction (#703) to rpg-toolkit main without authorization (premature Beta-2; → `rulebooks/dnd5e v0.61.0`).
**KEEP them.** CI-green expert work that extends the action registry (the "all refs" direction), already
merged + tagged; reverting is pure churn. Logged as a **process exception** — the *product* is sound, the
*process* (unauthorized, director-unreviewed merge) was not; Beta-2 Help/Hide is effectively pulled forward.
**Why:** Don't destroy good, CI-green work for process purity. The process failure is captured in the
runaway incident below + [[feedback_no_agent_merge_authority]].
**Status:** Adopted (Kirk, 2026-06-07) · **Retro:**

## D19 — Resume to the real done-bar; merges gated through the director
**Decision:** Engine + contract are complete + healthy on main, but the wave's true done-bar — the **MCP
playtest through api #597 → web #426** — has NOT happened. Resume there. Going forward, **no agent
self-merges**: the director runs every `gh pr merge`; agents open PRs, the director gates + merges with
active PR/merge monitoring.
**Why:** Beat-1 isn't done until the goal behavior is observable end-to-end via MCP playtest (the sign-off
bar). The merge-gating change is the direct fix for the runaway.
**Status:** Adopted (Kirk, 2026-06-07) · **Retro:**

---

## Follow-ups (under #54, tracked on board #13)
Mid-wave gaps NOT required by the Beat-1 goal — filed as issues, surfaced at the retro (this list = the
wave's IsDirty, grows incrementally).
- **Dodge mechanical effect** — rpg-toolkit **#699**: Dodge dispatches + spends economy + emits
  `ActionResolved` on the real path, but `DodgeActivated → DodgingCondition.Apply` is unwired (the
  `DodgingCondition` type exists; pre-existing gap). Beat-1 proves dispatch *generality*, not Dodge's full
  effect (attacker disadvantage). [surfaced Slice 3]
- **api ActionRef parse robustness** (for the api #597 lane): when rpg-api splits the toolkit's `ActionRef`
  string (`module:type:id`) into the proto `Ref`, guard against short/empty parts rather than index-panic on a
  degenerate `::attack`. Purely defensive (no real caller produces it; web sends full triples) — wire it into
  the TakeAction translation. [protos disc-014]

---

## Process incidents (for the retro)
- **Agent silent-stall (~2.5h, 2026-06-02 ~23:25→02:00)** — the background toolkit/protos agents blocked on a
  `git worktree` permission prompt invisible to both director and Kirk, and sat ~2.5h instead of
  stop-and-reporting. **Root causes:** (1) `git worktree` was missing from the project allowlist (added
  2026-06-02); (2) the director trusted agent silence instead of polling PR state — violated both
  `feedback_invisible_permission_stalls` (pre-authorize the command set) and `feedback_proactive_state_check`
  (never infer progress from silence). Kirk caught it and granted the prompt, which resumed the agent
  mid-recovery and caused a harmless duplicate-reply collision on #700's threads. **Recovery:** director took
  over #700 directly in a worktree — fixed all 4 Copilot comments (real bonus-action enforcement bugs),
  merged → `rulebooks/dnd5e v0.60.0`; merged #698; handed the encounter PR back to the recovered agent with
  checkpoint-per-step reporting. **Lesson:** pre-authorize the FULL agent command set up front, and poll
  agent/PR state on a cadence — silence is not progress.
