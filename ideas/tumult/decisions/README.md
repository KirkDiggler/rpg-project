# Tumult — Decision Receipts

_Each decision makes the **why** of a build decision observable — the
precondition for handing a wave-owning session more autonomy. Receipts are kept
as **one file per decision**, named `NNNN-slug.md` (e.g. `0001-board-spine.md`);
this README is the index. Format and rationale live in
`ideas/tumult/capabilities.md` § "How we work"._

> Seeded 2026-06-27 with the decisions that built the capability board itself —
> so the board's own construction is auditable.

---

## Index

- [DR-0001 — Board spine](0001-board-spine.md) — board rows are the 6 capability domains × the tumult-ue horizon tag, each citing demand.
- [DR-0002 — rpgkit issues first-class on board](0002-rpgkit-issues-first-class-on-board.md) — tumult-driven core issues are first-class on the Tumult board AND stay on #14, sharing state.
- [DR-0003 — Easy to wrap is the foundation](0003-easy-to-wrap-is-the-foundation.md) — wrappability (Group A) gates everything; capability depth rides on a working wrap.
- [DR-0004 — Wave sequencing](0004-wave-sequencing.md) — wave order wrap → card-play → typed-damage, with statuses as a parallel thread.
- [DR-0005 — Decision receipts first-class](0005-decision-receipts-first-class.md) — every non-obvious call gets a light, seam-focused DR; nothing ships without one.
- [DR-0006 — More, smaller waves](0006-more-smaller-waves.md) — split the coarse waves into ~10 fine ones plus an explicit combine/split rule.
- [DR-0007 — Spec in ideas folder](0007-spec-in-ideas-folder.md) — `capabilities.md` + decisions are the source of truth; the board tracks state only.
- [DR-0008 — Consumption model](0008-consumption-model.md) — a host vendors + compiles tumult source as its own module rather than linking a prebuilt lib.
- [DR-0009 — Recipe verified by CI](0009-recipe-verified-by-ci.md) — a CI sample consumer compiles tumult the documented way and runs as a ctest, so the recipe can't rot.
- [DR-0010 — Host-observation edge](0010-host-observation-edge.md) — `strike` publishes a `StrikeResolved` notification (after mutation) **and** keeps its `StrikeResult` return; a `std::vector<CombatantView>` snapshot is the read-model. _(The first engineering DR — the prior nine were the board's own construction.)_
