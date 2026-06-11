# Brief 001: healing_basics example (round 1 smoke test)

First qwen dispatch: recipe-sized, chain-only (no topics needed), pattern
to copy exists. Run from a checkout of rpgkit containing
`examples/chain_basics/` (PR #6 branch `feat/example-chain-basics`, or main
once merged). Paste everything below the line into the session.

---

You are working in the rpgkit repository — a C++20 toolkit for building
game combat systems. Your task is to create ONE new example program,
following an existing example exactly.

## Read first

1. `docs/how-to/build-a-damage-chain.md` — the recipe you are applying
2. `examples/chain_basics/main.cpp` — the pattern to copy
3. `examples/chain_basics/CMakeLists.txt` and `examples/CMakeLists.txt` —
   how examples are wired into the build

## Task

Create `examples/healing_basics/` — a runnable program that resolves a
HEALING amount through a `rpg::core::Chain<int>`, modeled after
chain_basics:

- Stage list: `{"base", "boosts", "cap"}`
- Modifier 1: id `"wisdom"`, stage `"boosts"`, flat +2
- Modifier 2: id `"healing-word"`, stage `"boosts"`, adds 1d4 (use the same
  `std::mt19937` + `std::uniform_int_distribution` pattern as chain_basics —
  the die must roll inside the lambda so it re-rolls per execute)
- Modifier 3: id `"max-hp-cap"`, stage `"cap"`, clamps the running value to
  at most 10 (the target is missing only 10 HP; healing past full is wasted)
- Execute with a base heal of 6, print each breakdown step and the final
  value, in the same output style as chain_basics

## Files

- Create: `examples/healing_basics/main.cpp`
- Create: `examples/healing_basics/CMakeLists.txt` (copy chain_basics's,
  change the target name to `healing_basics`)
- Edit: `examples/CMakeLists.txt` — add `add_subdirectory(healing_basics)`

## Rules

- Do NOT modify anything under `core/` or any other existing file except the
  one-line edit to `examples/CMakeLists.txt`.
- Every `chain.add(...)` returns a Status that must be checked (chain_basics
  shows the `mustBeOk` helper pattern — copy it).
- Match the comment style of chain_basics: explain WHY at each step.

## Verify before you are done

```sh
make build
./build/debug/examples/healing_basics/healing_basics   # run it twice; the
                                                       # healing-word line
                                                       # must vary
make pre-commit                                        # must end green
```

If clang-format or clang-tidy complain, fix what they say (run `make fmt`
for formatting). Do not disable any check.

---

## Round-1 observation log (fill in after the run)

- Model/quant:
- Interventions (each becomes a finding):
- Gates hit (fmt/tidy/build failures and whether the model self-recovered):
- Verdict vs success criteria:
