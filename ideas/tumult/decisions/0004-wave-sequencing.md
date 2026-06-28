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
