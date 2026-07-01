## DR-006 · 2026-06-27 · More, smaller waves to start + an explicit combine rule
Seam/primitive:  the wave granularity (workflow shape)
Decision:        split the 3 coarse waves into ~10 fine ones; at each close ask
                 primitive / seam / shape, and combine or split accordingly.
Rejected:        3 big waves (hides the seams until late; hard to hand off).
Why:             the build approach is unknown; fine granularity surfaces seams
                 early and makes single-wave handoff tractable. The list is a
                 best guess, not a contract.
