# Composition layers matter more than provisional constructor names

Written: 2026-11-05. Work: [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [Let the package supply the constructor's noun](2026-11-04-let-the-package-name-the-constructor.md).

## Question

What did the constructor cleanup actually establish, and what should remain open
until the examples teach us enough to extract useful tools?

## Evidence

The operator clarified that the examples can be reshaped as needed. Production
use comes after extraction and after learning which shape works. The original
concern was Compose's unnamed positional parameters, not a requirement to freeze
New(Config) as the final spelling. The operator also noted that effects.Compose
reads naturally for a layer that composes a machine, while items composes effects.

The current provider and item probe still use New(Config). This entry records a
clarification for extraction; it does not claim a new implementation or request
another rename. The earlier naming discussion remains part of the record.

## Interpretation

The working layers are distinct:

- The effects machine handles state, transitions and its bag.
- The effects composer connects that machine to state-held outputs.
- The item consumer composes effects with usage orchestration and item-owned pools.

Named configuration fields make a boundary readable regardless of whether the
verb is New or Compose. `effects.Compose(effects.Config{...})` remains a plausible
extracted API; no final name is selected here.

Prototype-to-prototype imports provide evidence, not production adoption. We may
change interfaces and internals when a consumer reveals a better shape; preserving
the current example is not the objective. Avoid compatibility layers whose only
purpose would be protecting these exploratory APIs.

## Next

Continue investigating the consumer seams. At extraction, review responsibilities,
consumer evidence, names and configuration together. Retain the readability gained
from named fields without treating a provisional constructor name as a constraint.
No code change is needed for this note.
