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
