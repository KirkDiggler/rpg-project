# Tumult — Decision Receipts

_Append-only. Newest on top. Each entry makes the **why** of a build decision
observable — the precondition for handing a wave-owning session more autonomy.
Format and rationale live in `capabilities.md` § "How we work"._

> Seeded 2026-06-27 with the decisions that built the capability board itself —
> so the board's own construction is auditable.

---

## DR-007 · 2026-06-27 · Spec lives in `ideas/tumult/`, board generated from it
Seam/primitive:  artifact ownership (where capability knowledge lives)
Decision:        `capabilities.md` + `decisions.md` in `rpg-project/ideas/tumult/`
                 are the source of truth; the GitHub board tracks state only.
Rejected:        board cards as the primary record (state and knowledge tangle;
                 board fields can't hold design rationale).
Why:             knowledge-in-docs / board-tracks-status — design survives board
                 churn and is diff-reviewable.

## DR-006 · 2026-06-27 · More, smaller waves to start + an explicit combine rule
Seam/primitive:  the wave granularity (workflow shape)
Decision:        split the 3 coarse waves into ~10 fine ones; at each close ask
                 primitive / seam / shape, and combine or split accordingly.
Rejected:        3 big waves (hides the seams until late; hard to hand off).
Why:             the build approach is unknown; fine granularity surfaces seams
                 early and makes single-wave handoff tractable. The list is a
                 best guess, not a contract.

## DR-005 · 2026-06-27 · Decision receipts are a first-class artifact
Seam/primitive:  the working model (process observability)
Decision:        every non-obvious call gets a light, seam-focused DR in this
                 file; no such call ships without one.
Rejected:        (a) heavyweight ADRs like rpg-toolkit's — too costly per call,
                 discourages logging; (b) no record — makes autonomy unauditable.
Why:             receipts are to our process what `(Status, Receipt)` is to
                 combat: they make the *why* observable, which is what lets a
                 wave-owning session be trusted with more rope.
Interface delta: archetype — positional `param1, param2, …` → `Input`/`Output`
                 struct (small change, large payoff, now pointable-at).

## DR-004 · 2026-06-27 · Wave order: wrap → card-play → typed-damage; statuses parallel
Seam/primitive:  sequencing across the Host / Cards / Damage / Statuses domains
Decision:        Group A (wrap) → Group B (card-play on **existing generic
                 damage**) → Group C (typed-damage depth); Group S (status base)
                 runs as a parallel thread.
Rejected:        typed damage before a playable card (front-loads a primitive
                 change with nothing at the edge to validate it).
Why:             the vertical slice can ride existing generic damage and reach
                 UE sooner; we deepen the `DamageEvent` primitive once it's
                 proven at the edge. Fluid — combine/split as seams reveal.

## DR-003 · 2026-06-27 · "Easy to wrap" is the foundation wave, not a parallel option
Seam/primitive:  the host↔library edge (consumption contract)
Decision:        wrappability (Group A) gates everything; capability depth rides
                 on a working wrap.
Rejected:        treating wrap / card / damage as three competing first-waves.
Why:             nothing reaches the edge until a host can consume tumult; a
                 capability isn't real until it's observable at the edge.

## DR-002 · 2026-06-27 · rpgkit-driven core issues are first-class on the tumult board
Seam/primitive:  cross-board tracking (tumult ↔ rpgkit, board #14)
Decision:        a tumult-driven core issue (e.g. rpgkit#52) is first-class on
                 the Tumult board AND stays on #14 — same GitHub issue on both,
                 shared open/closed state.
Rejected:        (a) links-only (core work blocking a capability isn't visible at
                 a glance); (b) a duplicate stub lane (double-tracking).
Why:             one board drives the whole engine push. Divergence risk is
                 bounded by the rule: tumult-driven rpgkit work surfaces here;
                 rpgkit-internal work stays on #14 only.

## DR-001 · 2026-06-27 · Board spine = capability domains × tumult-ue horizon
Seam/primitive:  the board's organizing axis
Decision:        rows are the 6 capability domains (Host & Wrap Surface,
                 Cards & Actions, Damage & Resolution, Statuses & Effects,
                 Encounter & Roster, Observability & Receipts); each item is
                 tagged with the tumult-ue horizon it unblocks and cites the
                 demand issue.
Rejected:        (a) consumer-horizons only (weak as a foundation catalog —
                 capabilities exist only when a host asks); (b) domain catalog
                 only (drifts from what the host needs next).
Why:             hybrid reads as a complete engine catalog *and* lines up with
                 real demand — matches "goal-shaped items that cite the use case."
