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
