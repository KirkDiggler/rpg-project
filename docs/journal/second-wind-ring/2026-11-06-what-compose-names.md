# 2026-11-06 — What does Compose name, and where does it belong?

Work: [extraction #705](https://github.com/KirkDiggler/rpg-project/issues/705), [provider #2291](https://github.com/KirkDiggler/rpg-toolkit/pull/2291).

## Question

As the effects experiment becomes an independently consumable runtime, which
names help someone understand its responsibility? The open choices were the
module home and whether its public entry should name construction or composition.
We also needed to avoid extracting a shared machine merely because two packages
currently use that noun.

## Evidence

KirkDiggler accepted `mechanics/effects/`, keeping `machine/` inside that module,
and `effects.Compose(effects.ComposeInput{...})`. The machine retains
`machine.New(machine.Config{...})`. The [ruling on #705](https://github.com/KirkDiggler/rpg-project/issues/705#issuecomment-6698616797)
records these choices before source movement.

The provider moved at [e15c6e54](https://github.com/KirkDiggler/rpg-toolkit/commit/e15c6e54ef812d5ae04326e51b6260123cd2020c).
All 25 Go files have identical parsed syntax trees after normalizing the import
prefix and agreed public-name changes, excluding comments and positions. Its
race tests, example, vet and lint pass. Both real item consumers now resolve the
pushed provider without local replacements: Second Wind S1 at `7d7ef342`, and
Rage/loot S2 at `7dadbb64`. Their full-module race tests, runnable examples, vet
and lint pass too. The [implementation record](https://github.com/KirkDiggler/rpg-toolkit/blob/feat/effects-extraction/docs/ideas/item-effect-composition/implementation.md)
links the receipts and limits.

The selected module path is not new to Go's version history: its retired
implementation has tags through v0.2.1. Those tags remain intact. No new release,
production condition migration or inventory integration has happened here.

## Interpretation

The [earlier New(Config) entry](2026-11-04-let-the-package-name-the-constructor.md)
solved an actual readability problem: positional arguments hid which vocabulary
and instance inputs were supplied. Named fields remain the answer to that problem.
The later naming discussion sharpened a different distinction: composing a
runtime from a recipe and vocabulary is an operation worth naming explicitly.
`ComposeInput` names its inputs without confusing them with the authored recipe.
Neither composition nor machine construction attaches to the bus or spends an
item charge.

The [first extraction plan](2026-11-06-from-walkthrough-to-extraction-plan.md)
recommended a top-level package. The mechanics home gives this generic runtime
a clearer place alongside resource tracking; rulebook meanings remain above it.
Reusing that path does not restore the retired implementation, but it does retain
published version history. A source directory being absent is not evidence that
its module has never existed.

The two machines still have room to develop independently. This move establishes
neither a shared abstraction nor a requirement to split effects' machine into a
separate release unit. The normalized comparison proves that this cut preserves
the existing behavior, not that all of that behavior is beyond improvement.

## Next

[#705](https://github.com/KirkDiggler/rpg-project/issues/705) owns provider review,
release and the final consumer re-pins. The next architectural work remains the
rulebook vocabulary and shared condition-creation boundary, grounded in these
consumers rather than bundled into this mechanical extraction.

— cross-team agent, on behalf of KirkDiggler
