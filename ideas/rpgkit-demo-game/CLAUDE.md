# Idea: rpgkit Demo Game (qwen as documentation examiner)

## What This Is
Use the local qwen model (4090/Ollama/opencode) to build a terminal
deck-builder demo against rpgkit, writing only game-layer code from rpgkit's
examples + docs. Doubles as (1) a repeatable documentation usability test —
if a 30B model can build from the docs, an outside dev can — and (2) the
first real consumer dogfooding rpgkit as a product.

## Key Framing
- **Boundary rule ported**: qwen writes rulebook/game code only; it never
  touches rpgkit `core/`. "qwen needs to edit core" = a filed finding, not a
  change.
- **Interventions are the output**: every human touch maps to a doc/API issue
  in rpgkit. The rerun after fixes is the point.
- **Doc style is dictated by the audience**: short complete runnable recipes
  (cookbook in rpgkit `examples/` + `docs/how-to/`), strict gates (-Werror,
  clang-tidy, tests) as the model's guardrails.
- **Demo game gets its own repo** consuming rpgkit via FetchContent — honest
  packaging test, zero blast radius on rpgkit.
- **Blocked on** rpgkit v1 slice (typed topics + Action/Effect + Strike
  example) — game code must never see `std::any`.

## Files
- `design.md` — full rationale, doc-style requirements, sequencing, success
  criteria, open questions.

## Status
Exploration / design captured 2026-06-10. Next concrete step lives in rpgkit
(plan-PRs 3–4), not here.
