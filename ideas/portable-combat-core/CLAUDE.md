# Idea: Portable Combat Core

## What This Is
A language-agnostic design for rpg-toolkit's event-driven composition core
(Bus + Chain + Effect + Action) so it can be re-implemented in C++ for Unreal
and, via a C ABI, Unity. The goal is a portable, embeddable **combat-system
toolkit** that game builders use to prototype triggered-effect games — with a
co-op combat deck-builder (Across the Obelisk family) as the first target.

## Origin
Exploration session 2026-06-09. Kirk wanted to know what a C++ / engine version
of the toolkit would look like, and concluded that *the design is the hard part*:
"what are the decisions that went into this architecture, then you can easily
write that in any language."

## Key Framing
- **We build components, not a game and not a rulebook.** D&D 5e is "just a
  rulebook." The deck-builder is another. The core is 4 system-agnostic concepts.
- **The bus never crosses a language boundary** — it lives with the rulebooks.
  Only *actions in* and *results/breakdowns out* cross the FFI wire. This single
  principle resolves the whole port strategy.
- **v1 is the smallest slice**: an Action that fires + an Effect that modifies
  it, on the Bus + Chain. Selectables rides along as a separate loot tool.
- **Two bets**: Bet 1 = Go shared-lib spike to prove the engine boundary (zero
  rules rewrite); Bet 2 = native C++ core (the destination, native feel).

## Files
- `design.md` — the full design: 4 core contracts, the 10 decisions extracted
  from the Go code (with provenance), the deck-builder mapping, scope, and the
  Bet 1 → Bet 2 path forward.

## Status
Exploration / design captured. No implementation committed. Not yet turned into
an implementation plan — Kirk is exploring.
