# rpgkit Demo Game — a local model as the documentation forcing function

## Status: Exploration / design captured 2026-06-10

Use the local qwen model (RTX 4090, Ollama + opencode) to build a small demo
game against [rpgkit](https://github.com/KirkDiggler/rpgkit), writing only
game-layer code, guided only by rpgkit's examples and docs.

## The two ideas this combines

### 1. A small model is an honest documentation test

"Can a ~30B local model build a game from our examples?" is a brutal,
repeatable usability test. Large models (and the authors) paper over doc gaps
with inference; a smaller model follows what is actually written. The
consequences:

- If qwen succeeds, an outside game dev skimming at 11pm succeeds.
- Every place qwen flails is a precise pointer at a missing example, an
  unclear contract, or an API that demands too much context.
- The test re-runs cheaply forever: docs change -> rerun the brief -> see if
  the model still gets there. `docs/quality.md` graded by an external
  examiner instead of by the authors.

The general principle (worth reusing beyond rpgkit): **briefs a 30B can
execute are briefs an outside contributor can execute.**

### 2. The demo game dogfoods rpgkit as a product

rpgkit's thesis (inherited from rpg-toolkit): the toolkit is the product;
game builders should spend their time on game stuff, not engine plumbing.
The demo game is the first real consumer. Any time the game layer is forced
to spend on plumbing is a bug in our API or docs — and qwen will surface it
mechanically where a human would quietly work around it.

## The boundary rule, ported

```
qwen writes the GAME / RULEBOOK layer   -> cards, effects, monsters, the loop
rpgkit core stays untouched             -> Bus / Chain / Topic / Action / Effect
```

qwen never modifies `core/`. Every time it "needs" to, that's a finding:
either a real API gap or a doc that failed to show the composition pattern.
Both are exactly what the exercise exists to surface. (Same shape as the
game's boundary rule: the API never knows what "rage" does.)

Findings get filed as rpgkit issues, not fixed inline by the model.

## What the demo game is

Smallest honest game, not a product: a **terminal combat demo** in the
deck-builder family (the design's target genre) — a hand of a few cards
(Actions), a couple of statuses (Effects: Bleed, Block), an enemy, a turn
loop, damage resolved through chains **with the breakdown printed every
turn** (the receipt is the demo's wow moment). No rendering engine, no
persistence, no menus.

## What a ~30B model needs from examples (this dictates the doc style)

- **Short, complete, runnable files** over clever abstractions. One pattern
  per file, the *why* in comments. (Conveniently, the heavily-narrated style
  rpgkit examples already use.)
- **A cookbook, not reference prose**: copy-adaptable recipes — "define an
  event + topic", "add a status effect", "resolve damage with a breakdown",
  "gate an action on a resource". Each recipe: goal, complete code, expected
  output.
- **Strict gates as guardrails**: rpgkit's -Werror + clang-tidy + tests do a
  lot of supervision a small model can't do for itself. Small model + strict
  compiler is a much better pair than small model + vibes.
- **Small briefs**: one recipe-sized task per dispatch, not "build the game".

## Where things live

- **Cookbook examples**: in rpgkit, `examples/` + `docs/how-to/` (they're the
  product's docs; the Strike example from the v1 plan is recipe #1).
- **The demo game**: its own small repo consuming rpgkit via CMake
  FetchContent — the *honest* packaging test, since it proves the first thing
  a real game dev hits. Also keeps qwen's blast radius entirely outside
  rpgkit.

## Sequencing (depends on rpgkit v1 slice)

1. rpgkit plan-PR 3 (typed `Topic<T>` veneer) and plan-PR 4 (Action, Effect,
   Strike example) land first — game builders are never supposed to see
   `std::any`, so qwen building against today's raw bus would test the wrong
   layer. The Strike example is cookbook recipe #1.
2. Write the cookbook recipes (each doubles as Kirk's C++ learning material).
3. Stand up the demo-game repo + qwen/opencode harness.
4. Run the experiment in small briefs; collect findings as rpgkit issues.
5. Fix docs/API from findings; rerun. The rerun is the point.

## Success criteria

- qwen produces a compiling, lint-clean, playable turn loop with ≤ N
  human-touch interventions (tune N after round 1; round 1 just establishes
  the baseline).
- Zero edits to rpgkit `core/`.
- Every intervention maps to a filed doc/API issue — interventions ARE the
  experiment's output, not embarrassments.
- A stranger (human) can follow the same cookbook and get the same game.

## Open questions (deferred)

- Which qwen size/quant runs best with opencode on the 4090 for C++ — and
  does C++ (vs Go/TS) need bigger? Round 1 will tell.
- Harness shape: raw opencode sessions vs a thin dispatch script with the
  brief + gate-check loop.
- Whether cookbook recipes should be tested in CI (likely yes: examples that
  rot are worse than no examples).
