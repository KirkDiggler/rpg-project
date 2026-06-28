## DR-009 · 2026-06-27 · The recipe is verified by a CI sample consumer, not just prose
Seam/primitive:  the consumption contract's truth source
Decision:        `examples/host-consumer/` compiles tumult the documented way
                 (the four stateful sources + `include/` + the header-only
                 `rpg::core` + `cxx_std_20`) and runs as a `ctest` in the
                 existing CI jobs — so the recipe's compile path is executed,
                 not merely described.
Rejected:        docs-only recipe (rots silently — prose drifts from what
                 actually compiles and no one notices until a host tries it).
Why:             makes "easy to wrap" observable + non-rotting; a broken recipe
                 fails CI. Costs no `ci.yml` change — a new `add_test` rides the
                 jobs that already run `ctest`.
