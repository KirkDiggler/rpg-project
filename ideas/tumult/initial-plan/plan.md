# tumult Slice 1 — Core seam + simplification proof

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stand up the `tumult` sibling repo — a plain-C++ combat library on rpgkit v0.3.0 — with `Character`, `Encounter`, the three signature effects (Vulnerable, Tough Skin, Bleed), and a GoogleTested `formatStep` that reads modifier `source` from the chain receipt. This is the proof that v0.3.0 receipts simplify host explanations.

**Architecture:** Header-leaning library (`include/tumult/`) compiled as a CMake `INTERFACE` target on rpgkit core; GoogleTest via FetchContent. `Encounter` owns a `rpg::core::Bus` and the request-topic plumbing that previously lived inline in `rpgkit-ue`'s `URPGKitEncounterRuntime` (PR #4). All resolution returns `(Status, …Receipt)` per v0.3.0. UE consumes this in Slice 2; terminal in Slice 7. No host types in here.

**Tech Stack:** C++20, CMake ≥ 3.25, Ninja (debug/release presets), GoogleTest v1.17.0 via FetchContent, clang-format (Google base, col 100), clang-tidy (bugprone/performance/modernize/readability/cppcoreguidelines, WarningsAsErrors). rpgkit pinned via FetchContent at tag **v0.3.0**.

## Global Constraints

- **rpgkit dependency:** FetchContent at tag `v0.3.0`; never `replace`, never `core/` edits. A gap in rpgkit is a filed finding against `rpgkit`, not a change in `tumult`.
- **No host types:** no `UCLASS`/`USTRUCT`/`FString`/`TObjectPtr`/`std::istream`-driven UX. Plain `std::string`, plain structs.
- **No exceptions across the API:** fallible operations return `rpg::core::Status`; `[[nodiscard]]` enforced by `-Werror`. Never `throw` past a library boundary. Never return success on failure.
- **C++20** (`CMAKE_CXX_STANDARD 20`, `EXTENSIONS OFF`). Header-only core (`INTERFACE` target).
- **Naming:** types `PascalCase`, methods `lowerCamelCase`, headers `snake_case.hpp` under `include/tumult/` (mirrors rpgkit).
- **TDD:** failing test first, watch it fail, minimal impl, watch it pass, commit.
- **Pre-commit:** `make pre-commit` = `lint` + `test` = `fmt-check` + `clang-tidy` + `ctest`. Green before every push. Never `--no-verify`.
- **Git:** issue-first (open a `tumult` issue per task citing `rpgkit-ue#9/#14/#15/#16` as the source use case on board #15; one issue per PR); branch fresh from `main`; **merge, never rebase**; **never merge without Kirk's explicit approval**.
- **Receipt discipline:** `Action::activate`, `Effect::apply`/`remove` return `(Status, …Receipt)` — receipts are populated *even on failure*. Slice 1 surfaces the `EffectReceipt` plumbing; `ActionReceipt` is Slice 3 (cards-as-Actions) and is NOT used here.
- **Slice 1 scope (YAGNI):** no cards-as-Actions, no ActionReceipt routing, no enemy intents, no multi-target, no combat-log reform, no win/loss flag, no second edge. `Strike` is a direct `Encounter::strike` verb.

---

## File Structure

```text
tumult/
  .clang-format                    Google base, col 100, pointer left
  .clang-tidy                      rpgkit's check set; HeaderFilterRegex: tumult/include/.*
  .clangd
  .editorconfig
  .gitignore                       build/, compile_commands.json, .cache/
  .github/workflows/ci.yml         clang++ + g++ + MSVC + lint
  AGENTS.md                        issue-first, merge-never-rebase, test-first, no --no-verify rules
  CLAUDE.md                        repo-local binding decisions cross-linked from rpg-project/ideas/tumult
  CMakeLists.txt                   FetchContent rpgkit v0.3.0 + googletest; add_subdirectory(include)
  CMakePresets.json                debug/release Ninja presets mirroring rpgkit
  LICENSE                          MIT (text-identical to rpgkit/LICENSE)
  Makefile                         configure build test fmt fmt-check tidy lint pre-commit clean
  README.md                        one-liner + status-doc pointer
  docs/status.md                   living doc — current health, active, paused, edges
  include/tumult/
    character.hpp                  Character struct (id, name, curHP/maxHP, block, isAlive)
    breakdown.hpp                  BreakdownStep + formatStep (the simplification proof)
    encounter.hpp                  Encounter class; StrikeResult; damage event/topic defs
    effects/vulnerable.hpp         VulnerableEffect : rpg::core::Effect (source "vulnerable")
    effects/tough_skin.hpp         ToughSkinEffect : rpg::core::Effect (source "tough-skin")
    effects/bleed.hpp               BleedEffect : rpg::core::Effect (source "rend"), turn.ended tick
  include/CMakeLists.txt           INTERFACE target tumult + alias tumult::core
  tests/
    CMakeLists.txt                 tumult_tests executable; gtest_discover_tests PRE_TEST
    character_test.cpp
    breakdown_test.cpp              THE simplification assertion (formatStep reads source)
    encounter_test.cpp             setup, strike (no-mod, vulnerable, tough_skin), block,
                                   raw damage via topic, effect lifecycle receipts
    vulnerable_test.cpp            apply/remove receipt shape
    tough_skin_test.cpp            apply/remove receipt shape
    bleed_test.cpp                 apply/remove receipt; turn.ended tick; auto-remove at 0 stacks
    integration_test.cpp            hero-strikes-goblin: chain source reads "vulnerable"
                                   (end-to-end simplification assertion)
```

Responsibility per file:
- `character.hpp` — value struct; no behavior beyond `isAlive` and a future HP-bound helper.
- `breakdown.hpp` — the contract a host renderer consumes. `formatStep` is the plain-C++ formatter a UE HUD and a terminal renderer both call.
- `encounter.hpp` — owns the bus, the characters, the active effects, request-topic subscriptions; one method per resolution verb.
- `effects/*.hpp` — one file per effect; each declares its own `rpg::core::Effect` subclass with an `onApply` that subscribes via `track()`.
- `tests/*` — one test file per public surface; `integration_test.cpp` asserts the full chain-source-to-formatter path.

---

## Task 1: Repo scaffold (CMake + Makefile + presets + CI + agent rules)

**Goal:** `git init` the new sibling repo, build the empty-header-only library, see `make test` pass with one trivial test, see `make pre-commit` green.

**Files:**
- Create: `LICENSE`, `README.md`, `AGENTS.md`, `CLAUDE.md`, `docs/status.md`
- Create: `CMakeLists.txt`, `CMakePresets.json`, `Makefile`
- Create: `.clang-format`, `.clang-tidy`, `.clangd`, `.editorconfig`, `.gitignore`
- Create: `.github/workflows/ci.yml`
- Create: `include/CMakeLists.txt`, `include/tumult/dummy.hpp`
- Create: `tests/CMakeLists.txt`, `tests/dummy_test.cpp`

**Interfaces:**
- Consumes: rpgkit v0.3.0 (FetchContent) — confirm tag exists via `git ls-remote --tags https://github.com/KirkDiggler/rpgkit v0.3.0`.
- Produces: `tumult::core` CMake INTERFACE target (alias of `tumult`); `tumult::core` includes `include/`; presets `debug`/`release`; `make pre-commit` green.

- [ ] **Step 1: Verify rpgkit v0.3.0 tag exists**

Run: `git ls-remote --tags https://github.com/KirkDiggler/rpgkit v0.3.0`
Expected: one line ending in `refs/tags/v0.3.0`. If empty — STOP and ask Kirk; do not fabricate a tag.

- [ ] **Step 2: Create the local working tree**

```bash
mkdir -p /home/kirk/personal/tumult
cd /home/kirk/personal/tumult
git init
git checkout -b main
```

- [ ] **Step 3: Write `LICENSE` (MIT, identical text to rpgkit's)**

Copy text from `https://raw.githubusercontent.com/KirkDiggler/rpgkit/main/LICENSE` (KirkDiggler copyright, MIT, year 2026). Pagination is fine — one line at a time is fine — but the text must match rpgkit's so the two repos are co-licensable.

- [ ] **Step 4: Write `README.md`**

```markdown
# tumult

A combat-game library on [rpgkit](https://github.com/KirkDiggler/rpgkit) —
characters, statuses, an encounter loop, and the resolution rules that turn
authored intent into observable fact. Built to be hosted by Unreal (via
rpgkit-ue) or a terminal (via rpgkit-demo-game); tumult itself never knows a
host.

## Status

Early — Slice 1 in progress. See `docs/status.md`.

## Design

[rpg-project/ideas/tumult/initial-plan/design.md](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/tumult/initial-plan/design.md)

## License

[MIT](LICENSE)
```

- [ ] **Step 5: Write `.gitignore`**

```gitignore
build/
build*/
compile_commands.json
.cache/
```

- [ ] **Step 6: Write `.clang-format` (matches rpgkit)**

```yaml
---
BasedOnStyle: Google
ColumnLimit: 100
IndentWidth: 2
DerivePointerAlignment: false
PointerAlignment: Left
```

- [ ] **Step 7: Write `.clang-tidy`**

```yaml
---
Checks: >
  bugprone-*,
  performance-*,
  modernize-*,
  readability-*,
  cppcoreguidelines-*,
  -modernize-use-trailing-return-type,
  -readability-identifier-length
WarningsAsErrors: '*'
HeaderFilterRegex: 'tumult/include/.*'
```

- [ ] **Step 8: Write `.clangd`**

```yaml
CompileFlags:
  CompilationDatabase: build/debug
```

- [ ] **Step 9: Write `.editorconfig`**

```ini
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true

[*.{hpp,cpp}]
indent_style = space
indent_size = 2
```

- [ ] **Step 10: Write `CMakePresets.json`**

```json
{
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
  "configurePresets": [
    {
      "name": "debug",
      "displayName": "Debug (Ninja)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "TUMULT_BUILD_TESTS": "ON" }
    },
    {
      "name": "release",
      "displayName": "Release (Ninja)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "TUMULT_BUILD_TESTS": "ON" }
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug" },
    { "name": "release", "configurePreset": "release" }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "debug", "output": { "outputOnFailure": true } },
    { "name": "release", "configurePreset": "release", "output": { "outputOnFailure": true } }
  ]
}
```

- [ ] **Step 11: Write `CMakeLists.txt`**

```cmake
cmake_minimum_required(VERSION 3.25)
project(tumult VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TUMULT_BUILD_TESTS "Build tumult tests" ${PROJECT_IS_TOP_LEVEL})

add_library(tumult_warnings INTERFACE)
if(MSVC)
  target_compile_options(tumult_warnings INTERFACE /W4 /WX)
else()
  target_compile_options(tumult_warnings INTERFACE -Wall -Wextra -Werror)
endif()

include(FetchContent)
FetchContent_Declare(
  rpgkit
  GIT_REPOSITORY https://github.com/KirkDiggler/rpgkit.git
  GIT_TAG        v0.3.0
)
FetchContent_MakeAvailable(rpgkit)

if(TUMULT_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  include(GoogleTest)
endif()

add_subdirectory(include)
if(TUMULT_BUILD_TESTS)
  add_subdirectory(tests)
endif()
```

- [ ] **Step 12: Write `Makefile`**

```makefile
.PHONY: configure build test lint fmt fmt-check tidy pre-commit clean

configure:
	cmake --preset debug

build: configure
	cmake --build --preset debug

test: build
	ctest --preset debug --output-on-failure

FMT_FILES = $(shell find include tests -name '*.hpp' -o -name '*.cpp' 2>/dev/null)

fmt:
	clang-format -i $(FMT_FILES)

fmt-check:
	clang-format --dry-run --Werror $(FMT_FILES)

tidy: configure
	run-clang-tidy -p build/debug -quiet $(FMT_FILES)

lint: fmt-check tidy

pre-commit: lint test

clean:
	rm -rf build
```

- [ ] **Step 13: Write `include/CMakeLists.txt`**

```cmake
add_library(tumult INTERFACE)
add_library(tumult::core ALIAS tumult)
target_include_directories(tumult INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(tumult INTERFACE cxx_std_20)
target_link_libraries(tumult INTERFACE rpg::core)
```

- [ ] **Step 14: Write `include/tumult/dummy.hpp` (placeholder so includes compile)**

```cpp
#ifndef TUMULT_DUMMY_HPP_
#define TUMULT_DUMMY_HPP_

// Removed in Task 2; exists only to prove the include path wires up.
namespace tumult {}
#endif  // TUMULT_DUMMY_HPP_
```

- [ ] **Step 15: Write `tests/CMakeLists.txt`**

```cmake
add_executable(tumult_tests
  dummy_test.cpp
)
target_link_libraries(tumult_tests PRIVATE tumult::core tumult_warnings GTest::gtest_main)
target_compile_definitions(tumult_tests PRIVATE TUMULT_PROJECT_VERSION="${PROJECT_VERSION}")
gtest_discover_tests(tumult_tests DISCOVERY_MODE PRE_TEST)
```

- [ ] **Step 16: Write `tests/dummy_test.cpp` (smoke test)**

```cpp
#include <gtest/gtest.h>

#include "tumult/dummy.hpp"

TEST(Smoke, Compiles) { EXPECT_EQ(1, 1); }
```

- [ ] **Step 17: Write `AGENTS.md`**

```markdown
# tumult

A combat-game library on rpgkit — characters, statuses, an encounter loop.
Hosts (UE via rpgkit-ue, terminal via rpgkit-demo-game) wrap a
`tumult::Encounter`. tumult never knows a host.

**Source design (don't re-derive):**
[rpg-project/ideas/tumult/initial-plan/design.md](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/tumult/initial-plan/design.md)
· implementation plan: `plan.md` beside it.

## The boundary

**tumult knows the game.** Characters, statuses, intent, the combat loop.
Never host types (`UClass`, `FString`, `TObjectPtr`, `std::istream`-driven
UX). Hosts are edges; rpgkit is a dependency, not an edge.

## Workflow

- **Issue-first.** No branch without an issue; one PR per logical unit. Cite
  the `rpgkit-ue#N` use case the task proofs (board #15: `#9`, `#14`, `#15`,
  `#16`).
- Branch from fresh main; never reuse old branches; merge, never rebase.
- TDD: test first, implement, `make test` green, commit.
- **`make pre-commit` green before every push** (fmt-check + clang-tidy +
  tests). CI enforces the same. Never `--no-verify`.
- Open the PR, wait for Copilot review, fix or reply on every thread.
- **Never merge without Kirk's explicit approval.**

## Testing

GoogleTest via FetchContent; tests live in `tests/`. Hand-written fakes
preferred; add `GTest::gmock` only when a test first needs it.

## Docs

Living docs: `docs/status.md` (current health, active, paused, edges).
Update them **in** the PR that changes reality, not after.
```

- [ ] **Step 18: Write `CLAUDE.md`** (cross-link to the design; this is the binding-decisions hook)

```markdown
# tumult

Combat-game library on [rpgkit](https://github.com/KirkDiggler/rpgkit). Sibling
to `rpgkit`, `rpgkit-ue`, `rpgkit-demo-game`; tucked into the workspace at
`/home/kirk/personal/tumult`.

**Source design (don't re-derive):**
[rpg-project/ideas/tumult/initial-plan/design.md](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/tumult/initial-plan/design.md)
· implementation plan: `plan.md` beside it.
· per-repo rules: `AGENTS.md` in this repo.

## The boundary

**tumult knows the game.** Hosts are edges; rpgkit is a dependency, not an
edge. If a host type (`UClass`, `FString`, `std::istream`-driven UX) shows
up in `include/tumult/`, say something.

## Binding decisions

Inherited from rpgkit (binding decisions 1–9 there): no exceptions in the
API, header-only core, `std::any` erased Bus + typed `Topic<T>` veneer,
stages (no priorities), opaque `SubscriptionId`, breakdown in scope, naming,
synchronous ordered fail-fast delivery, params-struct signatures for
multi-field operations.

Tumult-local decisions (locked in `ideas/tumult/initial-plan/design.md`):

| # | Decision |
|---|---|
| T1 | `Encounter` owns its `rpg::core::Bus`. Host-shared bus is a later pressure. |
| T2 | `Character` is a value struct; effects hold an EntityRef-like id, not a pointer. |
| T3 | `BreakdownStep` mirrors `Chain::Step` cheaply (plain struct); formatter lives in tumult. |
| T4 | `formatStep` is the simplification contract — two edges, one formatter, no host re-derivation. |
```

- [ ] **Step 19: Write `docs/status.md`**

```markdown
# tumult Status

_Last updated: 2026-06-23 (Slice 1 — scaffold in progress)_

## Current health

Slice 1 starting — repo scaffold builds green with a smoke test. Next:
`Character`, then `BreakdownStep` + `formatStep`, then `Encounter` and the
three effects.

## Active work

- Slice 1: core seam + simplification proof (this is the first PR)
  - Task 1: repo scaffold
  - Task 2: Character + tests
  - Task 3: BreakdownStep + formatStep + tests (the simplification proof)
  - Task 4: Encounter skeleton (setup, findCharacter, bus)
  - Task 6: Encounter::strike (no-mod)
  - Task 7: VulnerableEffect + strike-with-vulnerable
  - Task 8: ToughSkinEffect + strike-with-tough-skin
  - Task 9: BleedEffect (turn.ended tick, auto-remove)
  - Task 10: block via block.requested topic
  - Task 11: dealRawDamage + EffectReceipt lifecycle
  - Task 12: integration test — chain source reads "vulnerable" end-to-end
  - Task 13: CI workflow + docs freeze

## Edges (later slices)

- `rpgkit-ue` Slice 2: migrate `URPGKitEncounterRuntime` to hold a
  `tumult::Encounter` and reflect `StrikeResult`/`BreakdownStep` to HUD.
- `rpgkit-demo-game` Slice 7: terminal edge migrates to host tumult.

## Known rough edges

- (none yet)
```

- [ ] **Step 20: Configure & build & test the smoke**

Run: `make test`
Expected: `ctest --preset debug --output-on-failure` produces `1/1 Smoke.Compiles ... Passed`. If FetchContent fails (network), STOP and ask Kirk — do not switch to vendored headers.

- [ ] **Step 21: Run pre-commit**

Run: `make pre-commit`
Expected: `fmt-check` clean; `clang-tidy` clean (HeaderFilterRegex matches `tumult/include/.*`); `test` green.

- [ ] **Step 22: Initial commit**

```bash
git add -A
git commit -m "chore: repo scaffold - rpgkit v0.3.0 FetchContent, GoogleTest, smoke test"
```

Repo creation note (separate from this plan's commit): Kirk creates the
remote `KirkDiggler/tumult` on GitHub (`gh repo create KirkDiggler/tumult --private --source=. --remote=origin --push`). The first push to `origin main`
happens AFTER Kirk reviewers the design; first PR is this scaffold commit on
a feature branch per Slice 1.

---

## Task 2: `Character` struct

**Files:**
- Create: `include/tumult/character.hpp`
- Modify: `tests/CMakeLists.txt` (replace `dummy_test.cpp` with `character_test.cpp` in the sources list), delete `tests/dummy_test.cpp`
- Modify: `include/tumult/dummy.hpp` → delete; remove `#include "tumult/dummy.hpp"` from any remaining test it lingers in (only `dummy_test.cpp` referenced it; that file is being removed)
- Test: `tests/character_test.cpp`

**Interfaces:**
- Consumes: (nothing — first public type)
- Produces: `tumult::Character` (see signature below), used by Task 4's `Encounter`.

- [ ] **Step 1: Replace `dummy.hpp` with `character.hpp`**

```cpp
#ifndef TUMULT_CHARACTER_HPP_
#define TUMULT_CHARACTER_HPP_

#include <string>

namespace tumult {

// A combat participant. Value type — copy freely; identified by `id`.
// The host owns the field-of-view/order; tumult owns the HP/block numbers.
struct Character {
  std::string id;
  std::string name;
  int curHp = 0;
  int maxHp = 0;
  int block = 0;

  [[nodiscard]] bool isAlive() const { return curHp > 0; }
};

}  // namespace tumult

#endif  // TUMULT_CHARACTER_HPP_
```

(Note: `clang-tidy`'s `readability-identifier-naming` may prefer `curHp`/`maxHp`; rpgkit uses `lowerCamelCase` for fields. If clang-tidy complains, use `curHp`/`maxHp` — already in this form. Field underscore tolerance: `HeaderFilterRegex` covers `tumult/include/.*`, so the header IS linted; fix any clang-tidy hit by renaming, never by silencing.)

- [ ] **Step 2: Remove `include/tumult/dummy.hpp`**

```bash
git rm include/tumult/dummy.hpp
```

- [ ] **Step 3: Write the failing test `tests/character_test.cpp`**

```cpp
#include <gtest/gtest.h>

#include "tumult/character.hpp"

TEST(Character, IsAliveWhenHpPositive) {
  tumult::Character c{.id = "hero", .name = "Hero", .curHp = 10, .maxHp = 20};
  EXPECT_TRUE(c.isAlive());
}

TEST(Character, IsDeadAtZeroHp) {
  tumult::Character c{.id = "hero", .name = "Hero", .curHp = 0, .maxHp = 20};
  EXPECT_FALSE(c.isAlive());
}

TEST(Character, DefaultConstructedIsDead) {
  tumult::Character c;
  EXPECT_FALSE(c.isAlive());
  EXPECT_EQ(c.block, 0);
}

TEST(Character, CopyableValue) {
  tumult::Character a{.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6};
  tumult::Character b = a;
  b.curHp = 3;
  EXPECT_EQ(a.curHp, 6);
  EXPECT_EQ(b.curHp, 3);
}
```

- [ ] **Step 4: Update `tests/CMakeLists.txt`** to swap sources

```cmake
add_executable(tumult_tests
  character_test.cpp
)
target_link_libraries(tumult_tests PRIVATE tumult::core tumult_warnings GTest::gtest_main)
target_compile_definitions(tumult_tests PRIVATE TUMULT_PROJECT_VERSION="${PROJECT_VERSION}")
gtest_discover_tests(tumult_tests DISCOVERY_MODE PRE_TEST)
```

```bash
git rm tests/dummy_test.cpp
```

- [ ] **Step 5: Run the test, verify it fails then passes**

Run: `make test`
Expected: build green; 4 `Character.*` tests pass.

- [ ] **Step 6: Run pre-commit**

Run: `make pre-commit`
Expected: green (clang-tidy may warn on `Character c` — `modernize-use-default-member-init` etc.; fix by following clang-tidy hit-by-hit, never by silencing; the struct as written is clang-tidy-clean against the same check set rpgkit uses).

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat: Character struct (id/name/HP/block, isAlive)"
```

---

## Task 3: `BreakdownStep` + `formatStep` (THE simplification proof)

**Files:**
- Create: `include/tumult/breakdown.hpp`
- Modify: `tests/CMakeLists.txt` (add `breakdown_test.cpp`)
- Test: `tests/breakdown_test.cpp`

**Interfaces:**
- Consumes: (nothing rpgkit-side)
- Produces: `tumult::BreakdownStep`, `tumult::formatStep(const BreakdownStep&) -> std::string`. Used directly by Task 4 (`Encounter` fills `BreakdownStep` from `Chain::Result::breakdown`) and Task 12 (integration test).

- [ ] **Step 1: Write the failing test**

```cpp
#include <gtest/gtest.h>

#include "tumult/breakdown.hpp"

TEST(FormatStep, PlainBaseStepOmitsEmptySource) {
  tumult::BreakdownStep s{.id = "", .stage = "base", .source = "", .before = 7, .after = 7};
  EXPECT_EQ(tumult::formatStep(s), "base: 7 -> 7");
}

TEST(FormatStep, EffectsStepWithSourceIncludesSourceInParens) {
  tumult::BreakdownStep s{
      .id = "vulnerable-goblin", .stage = "effects", .source = "vulnerable", .before = 5, .after = 8};
  EXPECT_EQ(tumult::formatStep(s), "effects / vulnerable-goblin (vulnerable): 5 -> 8");
}

TEST(FormatStep, EffectsStepWithoutSourceOmitsParens) {
  tumult::BreakdownStep s{
      .id = "unknown-mod", .stage = "effects", .source = "", .before = 5, .after = 9};
  EXPECT_EQ(tumult::formatStep(s), "effects / unknown-mod: 5 -> 9");
}

TEST(FormatStep, FinalStageRenderedLikeBase) {
  tumult::BreakdownStep s{.id = "", .stage = "final", .source = "", .before = 8, .after = 8};
  EXPECT_EQ(tumult::formatStep(s), "final: 8 -> 8");
}
```

- [ ] **Step 2: Add `breakdown_test.cpp` to `tests/CMakeLists.txt`**

```cmake
add_executable(tumult_tests
  character_test.cpp
  breakdown_test.cpp
)
```

- [ ] **Step 3: Run the test, verify it fails**

Run: `make test`
Expected: FAIL with `fatal error: 'tumult/breakdown.hpp' file not found`.

- [ ] **Step 4: Write `include/tumult/breakdown.hpp`**

```cpp
#ifndef TUMULT_BREAKDOWN_HPP_
#define TUMULT_BREAKDOWN_HPP_

#include <string>

namespace tumult {

// One step of a folded chain, mirrored (cheap struct copy) from
// rpg::core::Chain<DamageEvent>::Result::breakdown for host consumption.
// Mirrored — not wrapped — so the host renderer never takes a chain header.
struct BreakdownStep {
  std::string id;       // modifier id ("vulnerable-goblin"); "" on base/final
  std::string stage;    // "base" | "effects" | "final"
  std::string source;   // effect source ("vulnerable"); "" if uninitialized
  int before = 0;
  int after = 0;
};

// The simplification contract. Reads `source` from the receipt — never a
// host-side hardcoded string. Format is the unit a UE HUD and a terminal
// renderer both consume without re-deriving.
//
// Shape:
//   base/final with no modifier: "<stage>: <before> -> <after>"
//   effects with no source:      "<stage> / <id>: <before> -> <after>"
//   effects with source:        "<stage> / <id> (<source>): <before> -> <after>"
inline std::string formatStep(const BreakdownStep& s) {
  if (s.id.empty()) {
    return s.stage + ": " + std::to_string(s.before) + " -> " + std::to_string(s.after);
  }
  if (s.source.empty()) {
    return s.stage + " / " + s.id + ": " + std::to_string(s.before) + " -> " +
           std::to_string(s.after);
  }
  return s.stage + " / " + s.id + " (" + s.source + "): " + std::to_string(s.before) + " -> " +
         std::to_string(s.after);
}

}  // namespace tumult

#endif  // TUMULT_BREAKDOWN_HPP_
```

- [ ] **Step 5: Run the test, verify it passes**

Run: `make test`
Expected: 4 `FormatStep.*` tests pass.

- [ ] **Step 6: Run pre-commit**

Run: `make pre-commit`
Expected: green.

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat: BreakdownStep + formatStep (receipt-driven explanation formatter)"
```

---

## Task 4: Encounter skeleton (setup, findCharacter, bus access)

**Files:**
- Create: `include/tumult/encounter.hpp`, `include/tumult/encounter.cpp` (Encounter has stateful subscription tracking; keep the class in the header but the request-subscription wiring TUs in a .cpp to keep include-time build light and clang-tidy HeaderFilterRegex honest)
- Modify: `tests/CMakeLists.txt` (add `encounter_test.cpp` and `encounter.cpp` to the test target's link set — or, if the test target already links `tumult::core` INTERFACE, add `encounter.cpp` to a `tumult_core_objects` STATIC lib)

  Decision: make `encounter.cpp` part of the INTERFACE library's `OBJECT` form. Concretely, change `include/CMakeLists.txt` to:

```cmake
add_library(tumult INTERFACE)
add_library(tumult::core ALIAS tumult)
target_include_directories(tumult INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(tumult INTERFACE cxx_std_20)
target_link_libraries(tumult INTERFACE rpg::core)

# Stateful TUs (Encounter's topic subscriptions) live as an OBJECT library so
# consumers that link tumult::core still get them, without INTERFACE carrying
# source files (which CMake technically supports but muddles multi-config).
add_library(tumult_objects OBJECT
  encounter.cpp
)
target_link_libraries(tumult_objects PRIVATE tumult::core tumult_warnings)
target_include_directories(tumult_objects PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Append the object lib's $<TARGET_OBJECTS:tumult_objects> to tumult's link.
target_link_libraries(tumult INTERFACE $<TARGET_OBJECTS:tumult_objects>)
```

(That writes the build file in this task; later tasks only add `.cpp` files to the `tumult_objects` source list.)

- Test: `tests/encounter_test.cpp`

**Interfaces:**
- Consumes: `tumult::Character` (Task 2), `rpg::core::Bus`, `rpg::core::TopicDef`
- Produces: `tumult::Encounter` with `setup`, `shutdown`, `findCharacter`, `bus`. Use site signature (called by later tasks):

```cpp
class Encounter {
 public:
  void setup(const Character& hero, const Character& enemy);
  void shutdown();
  [[nodiscard]] bool isReady() const;
  [[nodiscard]] const Character* findCharacter(const std::string& id) const;
  rpg::core::Bus& bus();
};
```

- [ ] **Step 1: Write the failing test `tests/encounter_test.cpp`**

```cpp
#include <gtest/gtest.h>

#include "tumult/character.hpp"
#include "tumult/encounter.hpp"

TEST(Encounter, SetupPopulatesHeroAndGoblin) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  EXPECT_TRUE(e.isReady());
  const auto* hero = e.findCharacter("hero");
  ASSERT_NE(hero, nullptr);
  EXPECT_EQ(hero->maxHp, 20);
  const auto* goblin = e.findCharacter("goblin");
  ASSERT_NE(goblin, nullptr);
  EXPECT_EQ(goblin->curHp, 6);
}

TEST(Encounter, FindCharacterReturnsNullForUnknown) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  EXPECT_EQ(e.findCharacter("nope"), nullptr);
}

TEST(Encounter, ShutdownClearsReadyAndCharacters) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  e.shutdown();
  EXPECT_FALSE(e.isReady());
  EXPECT_EQ(e.findCharacter("hero"), nullptr);
}

TEST(Encounter, BusIsAccessibleAfterSetup) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  EXPECT_NO_FATAL_FAILURE({ (void)e.bus(); });
}
```

- [ ] **Step 2: Update `tests/CMakeLists.txt`**

```cmake
add_executable(tumult_tests
  character_test.cpp
  breakdown_test.cpp
  encounter_test.cpp
)
target_link_libraries(tumult_tests PRIVATE tumult::core tumult_warnings GTest::gtest_main)
target_compile_definitions(tumult_tests PRIVATE TUMULT_PROJECT_VERSION="${PROJECT_VERSION}")
gtest_discover_tests(tumult_tests DISCOVERY_MODE PRE_TEST)
```

(Note: encounter.cpp is linked transitively via `tumult::core` now that include/CMakeLists.txt adds `tumult_objects`.)

- [ ] **Step 3: Update `include/CMakeLists.txt`** to add the `tumult_objects` OBJECT library (full content shown in the Files block above). Write that file in this step.

- [ ] **Step 4: Run, verify it fails**

Run: `make test`
Expected: FAIL, `fatal error: 'tumult/encounter.hpp' file not found`.

- [ ] **Step 5: Write `include/tumult/encounter.hpp`**

```cpp
#ifndef TUMULT_ENCOUNTER_HPP_
#define TUMULT_ENCOUNTER_HPP_

#include <array>
#include <memory>
#include <string>

#include "rpg/core/bus.hpp"
#include "tumult/character.hpp"

namespace tumult {

// Authoritative damage event on the chained combat.damage topic.
struct DamageEvent {
  std::string attackerId;
  std::string targetId;
  int baseAmount = 0;
};

// Topic definitions — declared once, bound to the Encounter's Bus.
inline const rpg::core::TopicDef<DamageEvent>& combatDamageTopic() {
  static const rpg::core::TopicDef<DamageEvent> kDef{"combat.damage"};
  return kDef;
}
inline const rpg::core::TopicDef<int>& turnEndedTopic() {
  static const rpg::core::TopicDef<int> kDef{"turn.ended"};
  return kDef;
}
inline const rpg::core::TopicDef<DamageEvent>& rawDamageRequestedTopic() {
  static const rpg::core::TopicDef<DamageEvent> kDef{"combat.raw_damage.requested"};
  return kDef;
}
inline const rpg::core::TopicDef<DamageEvent>& blockRequestedTopic() {
  static const rpg::core::TopicDef<DamageEvent> kDef{"combat.block.requested"};
  return kDef;
}
inline const std::vector<std::string>& damageStages() {
  static const std::vector<std::string> kStages = {"base", "effects", "final"};
  return kStages;
}

// Stateful runtime owner of one encounter — a bus, two characters, and
// (later) active effects + request-topic subscriptions. Mirrors rpgkit-ue's
// URPGKitEncounterRuntime: same shape, no UObject.
class Encounter {
 public:
  Encounter();
  ~Encounter();

  Encounter(const Encounter&) = delete;
  Encounter& operator=(const Encounter&) = delete;
  Encounter(Encounter&&) = delete;
  Encounter& operator=(Encounter&&) = delete;

  void setup(const Character& hero, const Character& enemy);
  void shutdown();
  [[nodiscard]] bool isReady() const { return ready_; }

  [[nodiscard]] const Character* findCharacter(const std::string& id) const;

  rpg::core::Bus& bus();

 private:
  void subscribeRequestTopics();
  void unsubscribeRequestTopics();

  std::unique_ptr<rpg::core::Bus> bus_;
  Character hero_{};
  Character enemy_{};
  bool hasHero_ = false;
  bool hasEnemy_ = false;
  bool ready_ = false;

  rpg::core::SubscriptionId rawDamageSubId_{};
  rpg::core::SubscriptionId blockSubId_{};
};

}  // namespace tumult

#endif  // TUMULT_ENCOUNTER_HPP_
```

(Note: requests here reuse `DamageEvent` for both raw-damage and block requests, matching the prototype. A later slice can split into `RawDamageRequest` and `BlockRequest` if effects pressure it; YAGNI now.)

- [ ] **Step 6: Write `include/tumult/encounter.cpp`**

```cpp
#include "tumult/encounter.hpp"

#include <utility>

namespace tumult {

Encounter::Encounter() = default;

Encounter::~Encounter() { shutdown(); }

void Encounter::setup(const Character& hero, const Character& enemy) {
  shutdown();
  bus_ = std::make_unique<rpg::core::Bus>();
  hero_ = hero;
  enemy_ = enemy;
  hasHero_ = true;
  hasEnemy_ = true;
  ready_ = false;
  subscribeRequestTopics();
  ready_ = true;
}

void Encounter::shutdown() {
  if (bus_) {
    unsubscribeRequestTopics();
    bus_.reset();
  }
  hasHero_ = false;
  hasEnemy_ = false;
  ready_ = false;
}

const Character* Encounter::findCharacter(const std::string& id) const {
  if (hasHero_ && hero_.id == id) return &hero_;
  if (hasEnemy_ && enemy_.id == id) return &enemy_;
  return nullptr;
}

rpg::core::Bus& Encounter::bus() { return *bus_; }

void Encounter::subscribeRequestTopics() {
  // No-op in Slice 1 skeleton; raw-damage/block handlers are added in
  // Tasks 10 and 11. Subscribed here so the Encounter has a single
  // subscribe/unsubscribe seam.
}

void Encounter::unsubscribeRequestTopics() {
  // Mirrors subscribeRequestTopics; populated in Tasks 10 & 11.
  if (rawDamageSubId_.value != 0) {
    (void)bus_->unsubscribe(rawDamageSubId_);
    rawDamageSubId_ = rpg::core::SubscriptionId{};
  }
  if (blockSubId_.value != 0) {
    (void)bus_->unsubscribe(blockSubId_);
    blockSubId_ = rpg::core::SubscriptionId{};
  }
}

}  // namespace tumult
```

- [ ] **Step 7: Run, verify it passes**

Run: `make test`
Expected: 4 `Encounter.*` tests pass.

- [ ] **Step 8: Run pre-commit**

Run: `make pre-commit`
Expected: green. Watch for clang-tidy flags on `hero_{}`/`enemy_{}` — `cppcoreguidelines-pro-type-member-init` may want explicit 0; the designated `= {}` should satisfy it. If a hit surfaces, add a member-init-list entry per clang-tidy's suggestion (e.g., `hero_{"", "", 0, 0, 0}`); never silence.

- [ ] **Step 9: Commit**

```bash
git add -A
git commit -m "feat: Encounter skeleton (setup/shutdown/findCharacter/bus)"
```

---

## Task 5: Encounter::strike — base path (no modifiers)

**Files:**
- Modify: `include/tumult/encounter.hpp`, `include/tumult/encounter.cpp`
- Modify: `tests/CMakeLists.txt` (already linked)
- Test: `tests/encounter_test.cpp` (add cases)

**Interfaces:**
- Consumes: Task 4's Encounter.
- Produces: `tumult::StrikeResult`, `Encounter::strike(att, target, base) -> StrikeResult`.

```cpp
struct StrikeResult {
  int finalDamage = 0;
  int blocked = 0;
  int hpAfter = 0;
  std::vector<BreakdownStep> breakdown;
};

StrikeResult Encounter::strike(const std::string& attacker, const std::string& target, int base);
```

- [ ] **Step 1: Add failing tests to `tests/encounter_test.cpp`**

```cpp
TEST(EncounterStrike, NoModifiersCarriesOneBaseStep) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  auto r = e.strike("hero", "goblin", 7);
  EXPECT_EQ(r.finalDamage, 7);
  EXPECT_EQ(r.blocked, 0);
  EXPECT_EQ(r.hpAfter, 0);  // 6 - 7 floored at 0
  ASSERT_EQ(r.breakdown.size(), 1u);
  EXPECT_EQ(r.breakdown[0].stage, "base");
  EXPECT_EQ(r.breakdown[0].source, "");
  EXPECT_EQ(r.breakdown[0].before, 7);
  EXPECT_EQ(r.breakdown[0].after, 7);
}

TEST(EncounterStrike, UnknownTargetDoesNotCrash) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  auto r = e.strike("hero", "nope", 7);
  EXPECT_EQ(r.finalDamage, 0);
  EXPECT_TRUE(r.breakdown.empty());
}

TEST(EncounterStrike, BlockAbsorbsDamageAndReportsBlocked) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  e.addBlock("goblin", 4);   // added in a later task; here we stub it via direct API
  auto r = e.strike("hero", "goblin", 7);
  EXPECT_EQ(r.finalDamage, 7);
  EXPECT_EQ(r.blocked, 4);
  EXPECT_EQ(r.hpAfter, 3);  // 6 - (7 - 4) = 3
}
```

(For the third test: this introduces `Encounter::addBlock(fighter, amount)` ahead of Task 10. Put the trivial mutating impl in `encounter.cpp` now (it doesn't subscribe yet); the full topic-based version is Task 10. If this creates a circular task ordering concern, defer the third test to Task 10 — see the "adjust" step at end of Task 5.)

- [ ] **Step 2: Run the test, verify it fails**

Run: `make test`
Expected: FAIL — `no member named 'strike'`.

- [ ] **Step 3: Update `include/tumult/encounter.hpp`** to add `StrikeResult` and `strike`

Add to the public section, after `bus()`:

```cpp
  tumult::StrikeResult strike(const std::string& attacker, const std::string& target, int base);
  void addBlock(const std::string& fighter, int amount);
```

(Place `StrikeResult` definition above `Encounter`, after `DamageEvent` topic defs.)

```cpp
struct StrikeResult {
  int finalDamage = 0;
  int blocked = 0;
  int hpAfter = 0;
  std::vector<BreakdownStep> breakdown;
};
```

(Add `#include <vector>` and `#include "tumult/breakdown.hpp"` near the top of encounter.hpp.)

- [ ] **Step 4: Implement `Encounter::strike` + `Encounter::addBlock` in `include/tumult/encounter.cpp`**

```cpp
#include "tumult/encounter.hpp"

#include <algorithm>
#include <utility>

#include "rpg/core/chain.hpp"
#include "rpg/core/topic.hpp"
#include "tumult/breakdown.hpp"

namespace tumult {

StrikeResult Encounter::strike(const std::string& attacker, const std::string& target, int base) {
  StrikeResult result;
  if (!ready_ || !bus_) return result;

  Character* targetPtr = nullptr;
  if (hasHero_ && hero_.id == target) targetPtr = &hero_;
  else if (hasEnemy_ && enemy_.id == target) targetPtr = &enemy_;
  if (targetPtr == nullptr) return result;

  DamageEvent event{attackerId = attacker, targetId = target, baseAmount = base};
  rpg::core::Chain<DamageEvent> chain(damageStages());
  rpg::core::ChainedTopic<DamageEvent> topic = combatDamageTopic().onChained(*bus_);
  rpg::core::Status status = topic.publish(event, chain);
  if (!status.isOk()) return result;

  auto chainResult = chain.execute(event);
  result.finalDamage = chainResult.value.baseAmount;

  for (const auto& step : chainResult.breakdown) {
    result.breakdown.push_back({
      .id = step.id, .stage = step.stage, .source = step.source,
      .before = step.before.baseAmount, .after = step.after.baseAmount,
    });
  }

  int blocked = std::min(targetPtr->block, result.finalDamage);
  targetPtr->block -= blocked;
  result.blocked = blocked;
  int hpDamage = result.finalDamage - blocked;
  targetPtr->curHp = std::max(0, targetPtr->curHp - hpDamage);
  result.hpAfter = targetPtr->curHp;
  return result;
}

void Encounter::addBlock(const std::string& fighter, int amount) {
  Character* who = nullptr;
  if (hasHero_ && hero_.id == fighter) who = &hero_;
  else if (hasEnemy_ && enemy_.id == fighter) who = &enemy_;
  if (who != nullptr) who->block += amount;
}

}  // namespace tumult
```

(Note: needs `Character* findCharacter` non-const form for mutation; add `Character* mutCharacter(const std::string&)` private helper in encounter.hpp/cpp — share the lookup logic with the const `findCharacter`. Adjust in Step 5.)

- [ ] **Step 5: Run the test, verify it passes**

Run: `make test`
Expected: 7 tests pass (4 from Task 4 + 3 new). Adjust as the `mutCharacter`/`findCharacter` non-const helper rolls in.

- [ ] **Step 6: Run pre-commit**

Run: `make pre-commit`
Expected: green.

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat: Encounter::strike (base path, block absorption, BreakdownStep from chain receipt)"
```

---

## Task 6: VulnerableEffect + strike-with-vulnerable

**Files:**
- Create: `include/tumult/effects/vulnerable.hpp`, `include/tumult/effects/vulnerable.cpp`
- Modify: `include/CMakeLists.txt` (add `effects/vulnerable.cpp` to `tumult_objects`)
- Modify: `include/tumult/encounter.hpp`/`.cpp` — add `applyEffect/removeEffect` returning `(Status, EffectReceipt)`
- Test: `tests/vulnerable_test.cpp`, additions to `tests/encounter_test.cpp`

**Interfaces:**
- Consumes: `Encounter::bus()` (Task 4), `Encounter::strike` (Task 5).
- Produces: `tumult::VulnerableEffect(target, percentBonus, remainingTurns)`. Source = `"vulnerable"`. Subscribes `combat.damage` chained (modifies incoming damage to target) and `turn.ended` (decrements turns). Self-removes at 0 turns.

```cpp
class VulnerableEffect : public rpg::core::Effect {
 public:
  VulnerableEffect(std::string targetId, int percentBonus, int remainingTurns);
  rpg::core::Status onApply(rpg::core::Bus& bus) override;
  // (state on the instance: targetId_, percentBonus_, remainingTurns_)
};
```

`Encounter`'s new methods:

```cpp
std::pair<rpg::core::Status, rpg::core::EffectReceipt> applyEffect(rpg::core::Effect& effect);
std::pair<rpg::core::Status, rpg::core::EffectReceipt> removeEffect(rpg::core::Effect& effect);
```

`Encounter` keeps a `std::vector<rpg::core::Effect*> activeEffects_` for the round-trip; ownership STAYS with the caller (mirrors rpgkit-ue today). Tasks 11 refines to `std::unique_ptr` if a test forces it.

- [ ] **Step 1: Write the failing test `tests/vulnerable_test.cpp`**

```cpp
#include <gtest/gtest.h>

#include "tumult/effects/vulnerable.hpp"
#include "tumult/encounter.hpp"

TEST(VulnerableEffect, ApplyReturnsReceiptWithSource) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::VulnerableEffect vuln("goblin", 50, 2);
  auto [status, receipt] = e.applyEffect(vuln);
  ASSERT_TRUE(status.isOk());
  EXPECT_EQ(receipt.source, "vulnerable");
  EXPECT_FALSE(receipt.subscriptions.empty());
}

TEST(VulnerableEffect, StrikeAgainstVulnerableDoublesHalfStep) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::VulnerableEffect vuln("goblin", 50, 2);
  e.applyEffect(vuln);
  auto r = e.strike("hero", "goblin", 5);
  // chain: base 5 -> effects vulnerable-goblin (vulnerable): 5 -> 8
  ASSERT_EQ(r.breakdown.size(), 1u);  // base runs condensed; effects step IS the modifier step
  EXPECT_EQ(r.breakdown.back().source, "vulnerable");
  EXPECT_EQ(r.breakdown.back().before, 5);
  EXPECT_EQ(r.breakdown.back().after, 8);  // ceil(5 * 1.5) = 8
  EXPECT_EQ(r.finalDamage, 8);
}

TEST(VulnerableEffect, RemoveReturnsReceiptAndStopsContributing) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::VulnerableEffect vuln("goblin", 50, 2);
  e.applyEffect(vuln);
  auto [status, receipt] = e.removeEffect(vuln);
  ASSERT_TRUE(status.isOk());
  EXPECT_EQ(receipt.source, "vulnerable");
  auto r = e.strike("hero", "goblin", 5);
  EXPECT_EQ(r.finalDamage, 5);  // no modifier now
}
```

- [ ] **Step 2: Run, verify it fails**

Run: `make test`
Expected: FAIL, `'tumult/effects/vulnerable.hpp' not found`.

- [ ] **Step 3: Write `include/tumult/effects/vulnerable.hpp`**

```cpp
#ifndef TUMULT_EFFECTS_VULNERABLE_HPP_
#define TUMULT_EFFECTS_VULNERABLE_HPP_

#include <string>

#include "rpg/core/effect.hpp"

namespace tumult {

class VulnerableEffect : public rpg::core::Effect {
 public:
  VulnerableEffect(std::string targetId, int percentBonus, int remainingTurns);
  rpg::core::Status onApply(rpg::core::Bus& bus) override;

  // Host-facing getters (Blueprint/terminal reflection in Slice 2/7).
  [[nodiscard]] const std::string& targetId() const { return targetId_; }
  [[nodiscard]] int percentBonus() const { return percentBonus_; }
  [[nodiscard]] int remainingTurns() const { return remainingTurns_; }

 private:
  std::string targetId_;
  int percentBonus_;
  int remainingTurns_;
};

}  // namespace tumult

#endif  // TUMULT_EFFECTS_VULNERABLE_HPP_
```

- [ ] **Step 4: Write `include/tumult/effects/vulnerable.cpp`**

```cpp
#include "tumult/effects/vulnerable.hpp"

#include <cmath>
#include <utility>

#include "rpg/core/bus.hpp"
#include "rpg/core/chain.hpp"
#include "rpg/core/status.hpp"
#include "rpg/core/topic.hpp"
#include "tumult/encounter.hpp"

namespace tumult {

VulnerableEffect::VulnerableEffect(std::string targetId, int percentBonus, int remainingTurns)
    : rpg::core::Effect("vulnerable-" + targetId, "vulnerable"),
      targetId_(std::move(targetId)),
      percentBonus_(percentBonus),
      remainingTurns_(remainingTurns) {}

rpg::core::Status VulnerableEffect::onApply(rpg::core::Bus& bus) {
  rpg::core::ChainedTopic<DamageEvent> damageTopic = combatDamageTopic().onChained(bus);
  auto id = damageTopic.subscribe(
      [this](const DamageEvent& event, rpg::core::Chain<DamageEvent>& chain) -> rpg::core::Status {
        if (remainingTurns_ <= 0 || event.targetId != targetId_) return rpg::core::Status::ok();
        const int bonus = percentBonus_;
        return chain.add({.stage = "effects",
                          .id = "vulnerable-" + targetId_,
                          .source = "vulnerable",
                          .modifier = [bonus](DamageEvent e) -> DamageEvent {
                            e.baseAmount = static_cast<int>(
                                std::ceil(static_cast<float>(e.baseAmount) *
                                          (1.0F + static_cast<float>(bonus) / 100.0F)));
                            return e;
                          }});
      });
  track(id);

  rpg::core::Topic<int> turnTopic = turnEndedTopic().on(bus);
  auto turnId = turnTopic.subscribe([this](const int& /*turnNumber*/) -> rpg::core::Status {
    if (remainingTurns_ > 0) --remainingTurns_;
    return rpg::core::Status::ok();
  });
  track(turnId);
  return rpg::core::Status::ok();
}

}  // namespace tumult
```

- [ ] **Step 5: Update `include/CMakeLists.txt`** to add `effects/vulnerable.cpp` to `tumult_objects`:

```cmake
add_library(tumult_objects OBJECT
  encounter.cpp
  effects/vulnerable.cpp
)
```

And add `include/tumult/effects/` to FMT_FILES in Makefile (the existing find `-name '*.cpp'` already covers this; no Makefile edit needed unless the find flags an issue — verify).

- [ ] **Step 6: Add `applyEffect`/`removeEffect` to `Encounter`**

In `include/tumult/encounter.hpp`'s public section, add:

```cpp
  std::pair<rpg::core::Status, rpg::core::EffectReceipt> applyEffect(rpg::core::Effect& effect);
  std::pair<rpg::core::Status, rpg::core::EffectReceipt> removeEffect(rpg::core::Effect& effect);
```

(Add `#include "rpg/core/effect.hpp"` and `#include <utility>` near the top.)

In the private section, add:

```cpp
  std::vector<rpg::core::Effect*> activeEffects_;
```

In `include/tumult/encounter.cpp`:

```cpp
std::pair<rpg::core::Status, rpg::core::EffectReceipt> Encounter::applyEffect(rpg::core::Effect& effect) {
  if (!ready_ || !bus_) {
    return {rpg::core::Status::error("encounter not ready"), rpg::core::EffectReceipt{}};
  }
  if (std::ranges::find(activeEffects_, &effect) != activeEffects_.end()) {
    return {rpg::core::Status::error("effect already applied: " + effect.id()),
            rpg::core::EffectReceipt{.id = effect.id(), .source = effect.source()}};
  }
  auto result = effect.apply({.bus = *bus_});
  if (result.first.isOk()) activeEffects_.push_back(&effect);
  return result;
}

std::pair<rpg::core::Status, rpg::core::EffectReceipt> Encounter::removeEffect(rpg::core::Effect& effect) {
  auto it = std::ranges::find(activeEffects_, &effect);
  if (it == activeEffects_.end()) {
    return {rpg::core::Status::error("effect not active: " + effect.id()),
            rpg::core::EffectReceipt{.id = effect.id(), .source = effect.source()}};
  }
  auto result = effect.remove();
  if (result.first.isOk()) activeEffects_.erase(it);
  return result;
}
```

(`#include <algorithm>` and `#include <ranges>` if not already.)

- [ ] **Step 7: Run the test, verify it passes**

Run: `make test`
Expected: 3 `VulnerableEffect.*` tests pass.

- [ ] **Step 8: Adjust breakdown assertion if needed**

The naive chain `["base", "effects", "final"]` with no modifiers in `base` produces only the `base` step (before=after=5). When Vulnerable contributes an `effects` step, that's a 2nd entry; rpgkit's `Chain::execute` iterates `entries_` only — it does NOT emit rows for empty stages. Verify against `chain.hpp` lines 80-100: only contributors produce `Step`s. So the test's `ASSERT_EQ(r.breakdown.size(), 1u)` is FALSE — there are TWO steps (base passthrough + effects). Fix the assertion:

```cpp
ASSERT_EQ(r.breakdown.size(), 2u);
EXPECT_EQ(r.breakdown[0].stage, "base");
EXPECT_EQ(r.breakdown[1].stage, "effects");
EXPECT_EQ(r.breakdown[1].source, "vulnerable");
```

Wait — actually, the prototype in `rpgkit-ue/RPGKitBus.cpp:39` constructs the chain WITHOUT a base contributor; only effects add modifiers. So `base` stage emits nothing. Confirm against rpgkit tutorials:

`Base` stage produces a step ONLY if someone subscribes a base modifier. In tutorials/08 the chain has `base` stage but no subscriber contributes to it; the breakdown shows only effect steps. So the test's `ASSERT_EQ(size, 1u)` after Vulnerable measures the effects step alone (the only contributor). Adjust the failing-test expectation accordingly:

```cpp
ASSERT_EQ(r.breakdown.size(), 1u);
EXPECT_EQ(r.breakdown[0].stage, "effects");
EXPECT_EQ(r.breakdown[0].source, "vulnerable");
EXPECT_EQ(r.breakdown[0].before, 5);
EXPECT_EQ(r.breakdown[0].after, 8);
```

But then `EncounterStrike.NoModifiersCarriesOneBaseStep` from Task 5 also breaks — if no one subscribes the `base` stage, no step emits. So either:

a) `Encounter::strike` itself subscribes a `base` identity-modifier (id `"base"`, source `""`) so the breakdown always shows the base passthrough, OR
b) tests assert zero breakdown on no-mod strike.

Decision: (a) — Encounter registers a one-time `combat.damage` chained subscriber from `setup()` that adds `{stage="base", id="base", source="", modifier=identity}`. This guarantees the HUD always has a base row to read "base damage was X" from; mirrors what a rulebook tutorial does. Add the subscription in `subscribeRequestTopics()` (renamed to `subscribeEncounterTopics()` to cover both requests AND the base identity modifier). Track the sub id; unsubscribe on shutdown.

Update Task 4's `subscribeRequestTopics` accordingly: the existing body changes from "no-op" to "subscribe the base identity modifier" — this is the *adjustment* between Task 4 and Task 6. Fix `Task 4 / Step 6` code if Tasks 4 and 6 are being implemented in separate sessions, or fold it in now.

Apply the change to `encounter.cpp` `subscribeRequestTopics`:

```cpp
void Encounter::subscribeRequestTopics() {
  rpg::core::ChainedTopic<DamageEvent> topic = combatDamageTopic().onChained(*bus_);
  baseDamageSubId_ = topic.subscribe(
      [](const DamageEvent& /*event*/, rpg::core::Chain<DamageEvent>& chain) -> rpg::core::Status {
        return chain.add({.stage = "base", .id = "base", .source = "",
                          .modifier = [](DamageEvent e) -> DamageEvent { return e; }});
      });
}
```

(Add `baseDamageSubId_` member to encounter.hpp; unsubscribe in `unsubscribeRequestTopics`.)

Now `NoModifiersCarriesOneBaseStep` passes (size=1: base with before=after=7, source=""), and `StrikeAgainstVulnerableDoublesHalfStep` passes (size=2: base then effects-vulnerable).

Pass the test. Adjust test assertions in Task 5 + Task 6 to `size == 2` for the vulnerable case. Done — this is why TDD + retracting earlier tasks matters; expect to walk back Task 5's `size == 1` claim to `size > 0`.

- [ ] **Step 9: Run pre-commit**

Run: `make pre-commit`
Expected: green.

- [ ] **Step 10: Commit**

```bash
git add -A
git commit -m "feat: VulnerableEffect (source 'vulnerable'); Encounter::applyEffect/removeEffect returning EffectReceipt"
```

---

## Task 7: ToughSkinEffect

**Files:**
- Create: `include/tumult/effects/tough_skin.hpp`, `include/tumult/effects/tough_skin.cpp`
- Modify: `include/CMakeLists.txt` (add `effects/tough_skin.cpp`)
- Test: `tests/tough_skin_test.cpp`

**Interfaces:**
- Consumes: `Encounter::applyEffect`, `Encounter::strike`.
- Produces: `tumult::ToughSkinEffect(targetId, reduction)`. Source = `"tough-skin"`. Subscribes `combat.damage` chained (flat reduction to target). No duration.

- [ ] **Step 1: Write failing test `tests/tough_skin_test.cpp`**

```cpp
#include <gtest/gtest.h>

#include "tumult/effects/tough_skin.hpp"
#include "tumult/encounter.hpp"

TEST(ToughSkinEffect, ApplyReturnsReceiptWithSource) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::ToughSkinEffect ts("goblin", 1);
  auto [status, receipt] = e.applyEffect(ts);
  ASSERT_TRUE(status.isOk());
  EXPECT_EQ(receipt.source, "tough-skin");
}

TEST(ToughSkinEffect, StrikeAgainstToughSkinReducesByOne) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::ToughSkinEffect ts("goblin", 1);
  e.applyEffect(ts);
  auto r = e.strike("hero", "goblin", 7);
  ASSERT_GE(r.breakdown.size(), 2u);
  const auto& effectsStep = r.breakdown[1];
  EXPECT_EQ(effectsStep.source, "tough-skin");
  EXPECT_EQ(effectsStep.before, 7);
  EXPECT_EQ(effectsStep.after, 6);
  EXPECT_EQ(r.finalDamage, 6);
}

TEST(ToughSkinEffect, DamageNotBelowZero) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::ToughSkinEffect ts("goblin", 100);
  e.applyEffect(ts);
  auto r = e.strike("hero", "goblin", 5);
  EXPECT_EQ(r.finalDamage, 0);  // floor at 0
}
```

- [ ] **Step 2: Run, verify it fails**

Run: `make test`
Expected: FAIL.

- [ ] **Step 3: Write `include/tumult/effects/tough_skin.hpp`**

```cpp
#ifndef TUMULT_EFFECTS_TOUGH_SKIN_HPP_
#define TUMULT_EFFECTS_TOUGH_SKIN_HPP_

#include <string>

#include "rpg/core/effect.hpp"

namespace tumult {

class ToughSkinEffect : public rpg::core::Effect {
 public:
  ToughSkinEffect(std::string protectedId, int reduction);
  rpg::core::Status onApply(rpg::core::Bus& bus) override;

  [[nodiscard]] const std::string& protectedId() const { return protectedId_; }
  [[nodiscard]] int reduction() const { return reduction_; }

 private:
  std::string protectedId_;
  int reduction_;
};

}  // namespace tumult

#endif  // TUMULT_EFFECTS_TOUGH_SKIN_HPP_
```

- [ ] **Step 4: Write `include/tumult/effects/tough_skin.cpp`**

```cpp
#include "tumult/effects/tough_skin.hpp"

#include <algorithm>
#include <utility>

#include "rpg/core/bus.hpp"
#include "rpg/core/chain.hpp"
#include "rpg/core/status.hpp"
#include "rpg/core/topic.hpp"
#include "tumult/encounter.hpp"

namespace tumult {

ToughSkinEffect::ToughSkinEffect(std::string protectedId, int reduction)
    : rpg::core::Effect("tough-skin-" + protectedId, "tough-skin"),
      protectedId_(std::move(protectedId)),
      reduction_(reduction) {}

rpg::core::Status ToughSkinEffect::onApply(rpg::core::Bus& bus) {
  rpg::core::ChainedTopic<DamageEvent> topic = combatDamageTopic().onChained(bus);
  auto id = topic.subscribe(
      [this](const DamageEvent& event, rpg::core::Chain<DamageEvent>& chain) -> rpg::core::Status {
        if (event.targetId != protectedId_) return rpg::core::Status::ok();
        const int reduction = reduction_;
        return chain.add({.stage = "effects",
                          .id = "tough-skin-" + protectedId_,
                          .source = "tough-skin",
                          .modifier = [reduction](DamageEvent e) -> DamageEvent {
                            e.baseAmount = std::max(0, e.baseAmount - reduction);
                            return e;
                          }});
      });
  track(id);
  return rpg::core::Status::ok();
}

}  // namespace tumult
```

- [ ] **Step 5: Update `include/CMakeLists.txt`**

```cmake
add_library(tumult_objects OBJECT
  encounter.cpp
  effects/vulnerable.cpp
  effects/tough_skin.cpp
)
```

- [ ] **Step 6: Run, verify it passes**

Run: `make test`
Expected: 3 `ToughSkinEffect.*` tests pass.

- [ ] **Step 7: Run pre-commit, commit**

```bash
make pre-commit
git add -A
git commit -m "feat: ToughSkinEffect (source 'tough-skin', flat reduction)"
```

---

## Task 8: BleedEffect (turn.ended tick, auto-remove at 0 stacks)

**Files:**
- Create: `include/tumult/effects/bleed.hpp`, `include/tumult/effects/bleed.cpp`
- Modify: `include/CMakeLists.txt` (add `effects/bleed.cpp`)
- Modify: `include/tumult/encounter.hpp`/`.cpp` — add `Encounter::endTurn()` (publishes `turn.ended`) and `Encounter::dealRawDamage(target, amount)` (the Bleed tick handler)
- Test: `tests/bleed_test.cpp`

**Interfaces:**
- Consumes: `Encounter::applyEffect`, `Encounter::bus`, `Encounter::endTurn`, `Encounter::dealRawDamage`.
- Produces: `tumult::BleedEffect(targetId, damagePerStack, stacks)`. Source = `"rend"`. Subscribes `turn.ended` only; each tick deals `damagePerStack` raw damage via `combat.raw_damage.requested`; at 0 stacks, self-removes.

`Encounter::endTurn()`:

```cpp
rpg::core::Status Encounter::endTurn();  // publishes turn.ended with ++turnNumber_
```

`Encounter::dealRawDamage(target, amount)`:

```cpp
rpg::core::Status Encounter::dealRawDamage(const std::string& target, int amount);  // direct mutation; also the handler body for raw_damage.requested
```

- [ ] **Step 1: Wire the raw_damage.requested topic in Encounter first (so Bleed can publish it)**

Modify `Encounter::subscribeRequestTopics` in `include/tumult/encounter.cpp`:

```cpp
void Encounter::subscribeRequestTopics() {
  rpg::core::ChainedTopic<DamageEvent> combatTopic = combatDamageTopic().onChained(*bus_);
  baseDamageSubId_ = combatTopic.subscribe(
      [](const DamageEvent& /*e*/, rpg::core::Chain<DamageEvent>& chain) -> rpg::core::Status {
        return chain.add({.stage = "base", .id = "base", .source = "",
                          .modifier = [](DamageEvent ev) -> DamageEvent { return ev; }});
      });

  rpg::core::Topic<DamageEvent> rawTopic = rawDamageRequestedTopic().on(*bus_);
  rawDamageSubId_ = rawTopic.subscribe(
      [this](const DamageEvent& req) -> rpg::core::Status { return dealRawDamage(req.targetId, req.baseAmount); });
}
```

Add `dealRawDamage` impl in `encounter.cpp`:

```cpp
rpg::core::Status Encounter::dealRawDamage(const std::string& target, int amount) {
  Character* who = mutCharacter(target);
  if (who == nullptr) return rpg::core::Status::error("unknown target: " + target);
  who->curHp = std::max(0, who->curHp - amount);
  return rpg::core::Status::ok();
}

rpg::core::Status Encounter::endTurn() {
  ++turnNumber_;
  rpg::core::Topic<int> topic = turnEndedTopic().on(*bus_);
  return topic.publish(turnNumber_);
}
```

(Add `int turnNumber_ = 0;` member to encounter.hpp.)

- [ ] **Step 2: Write the failing test `tests/bleed_test.cpp`**

```cpp
#include <gtest/gtest.h>

#include "tumult/effects/bleed.hpp"
#include "tumult/encounter.hpp"

TEST(BleedEffect, ApplyReturnsReceiptWithSourceRend) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::BleedEffect bleed("hero", 2, 3);
  auto [status, receipt] = e.applyEffect(bleed);
  ASSERT_TRUE(status.isOk());
  EXPECT_EQ(receipt.source, "rend");
}

TEST(BleedEffect, EndTurnTicksDamageAndDecrementsStacks) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::BleedEffect bleed("hero", 2, 3);
  e.applyEffect(bleed);
  ASSERT_TRUE(e.endTurn().isOk());
  const auto* hero = e.findCharacter("hero");
  ASSERT_NE(hero, nullptr);
  EXPECT_EQ(hero->curHp, 18);  // 20 - 2
  EXPECT_EQ(bleed.stacks(), 2);
}

TEST(BleedEffect, AutoRemovesAtZeroStacks) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 10, .maxHp = 10});
  tumult::BleedEffect bleed("hero", 2, 1);
  e.applyEffect(bleed);
  ASSERT_TRUE(e.endTurn().isOk());
  EXPECT_FALSE(e.isEffectActive(bleed));  // adds Encounter::isEffectActive for tests
  const auto* hero = e.findCharacter("hero");
  EXPECT_EQ(hero->curHp, 18);  // ticked once then self-removed
  // Second endTurn: bleed is gone, no further damage.
  ASSERT_TRUE(e.endTurn().isOk());
  EXPECT_EQ(hero->curHp, 18);
}
```

- [ ] **Step 3: Add `Encounter::isEffectActive` (test helper)**

In `encounter.hpp` public:

```cpp
  [[nodiscard]] bool isEffectActive(const rpg::core::Effect& effect) const;
```

In `encounter.cpp`:

```cpp
bool Encounter::isEffectActive(const rpg::core::Effect& effect) const {
  return std::ranges::find(activeEffects_, &effect) != activeEffects_.end();
}
```

- [ ] **Step 4: Write `include/tumult/effects/bleed.hpp`**

```cpp
#ifndef TUMULT_EFFECTS_BLEED_HPP_
#define TUMULT_EFFECTS_BLEED_HPP_

#include <string>

#include "rpg/core/effect.hpp"

namespace tumult {

class BleedEffect : public rpg::core::Effect {
 public:
  BleedEffect(std::string targetId, int damagePerStack, int stacks);
  rpg::core::Status onApply(rpg::core::Bus& bus) override;

  [[nodiscard]] const std::string& targetId() const { return targetId_; }
  [[nodiscard]] int damagePerStack() const { return damagePerStack_; }
  [[nodiscard]] int stacks() const { return stacks_; }

 private:
  std::string targetId_;
  int damagePerStack_;
  int stacks_;

  rpg::core::Bus* bus_ = nullptr;  // non-owning; needed to publish raw_damage.requested on tick
};

}  // namespace tumult

#endif  // TUMULT_EFFECTS_BLEED_HPP_
```

- [ ] **Step 5: Write `include/tumult/effects/bleed.cpp`**

```cpp
#include "tumult/effects/bleed.hpp"

#include <utility>

#include "rpg/core/bus.hpp"
#include "rpg/core/status.hpp"
#include "rpg/core/topic.hpp"
#include "tumult/encounter.hpp"

namespace tumult {

BleedEffect::BleedEffect(std::string targetId, int damagePerStack, int stacks)
    : rpg::core::Effect("bleed-" + targetId, "rend"),
      targetId_(std::move(targetId)),
      damagePerStack_(damagePerStack),
      stacks_(stacks) {}

rpg::core::Status BleedEffect::onApply(rpg::core::Bus& bus) {
  bus_ = &bus;
  rpg::core::Topic<int> turnTopic = turnEndedTopic().on(bus);
  auto id = turnTopic.subscribe([this](const int& /*turn*/ ) -> rpg::core::Status {
    if (stacks_ <= 0) return rpg::core::Status::ok();
    if (bus_ == nullptr) return rpg::core::Status::error("bleed lost bus mid-tick");

    DamageEvent req{.attackerId = "bleed-" + targetId_, .targetId = targetId_, .baseAmount = damagePerStack_};
    rpg::core::Topic<DamageEvent> raw = rawDamageRequestedTopic().on(*bus_);
    rpg::core::Status status = raw.publish(req);
    if (!status.isOk()) return status;

    --stacks_;
    if (stacks_ <= 0) {
      // Self-remove: core's Effect::remove() handles unsubscription;
      // we do NOT call bus_->unsubscribe on the bleed-turn sub here —
      // remove() will sweep it. We just return ok; the caller (turn.ended
      // delivery) doesn't know we self-removed.
      (void)remove();  // returns (Status, EffectReceipt); we discard both here.
    }
    return rpg::core::Status::ok();
  });
  track(id);
  return rpg::core::Status::ok();
}

}  // namespace tumult
```

(Note: this calls `remove()` from inside `turn.ended` delivery. The base `Effect::remove()` only unsubscribes `tracked_`; it's safe mid-publish because `Bus::publish` iterates a snapshot. The bleeding turn-sub might iterate twice after self-removal — confirm with the Bus::publish snapshot semantics: yes, it copies subscribers, so the bleed sub removed itself AFTER it returned; the next `turn.ended` publish (the next turn) does NOT see it. Confirm in test `AutoRemovesAtZeroStacks` — second endTurn does not tick further bleed.)

But `remove()` from inside the bleed subscriber: does `Bus::publish` re-iterate the live `topics_["turn.ended"]` to find the bleed sub erased? It copies BEFORE delivering, so mid-delivery unsubscribe takes effect on the NEXT publish. Fine.

- [ ] **Step 6: Update `include/CMakeLists.txt`**

```cmake
add_library(tumult_objects OBJECT
  encounter.cpp
  effects/vulnerable.cpp
  effects/tough_skin.cpp
  effects/bleed.cpp
)
```

- [ ] **Step 7: Run, verify tests pass**

Run: `make test`
Expected: 3 `BleedEffect.*` tests pass. If `AutoRemovesAtZeroStacks` fails (hero takes 18-2 = 16 instead of 18-meaning-removed), check the `(void)remove()` order — remove BEFORE the Topic publish completes returns Status ok then decrements above expected behavior. Investigate; do NOT silence — fix.

- [ ] **Step 8: Run pre-commit, commit**

```bash
make pre-commit
git add -A
git commit -m "feat: BleedEffect (source 'rend', turn.ended tick, self-remove at 0 stacks); Encounter::endTurn/dealRawDamage"
```

---

## Task 9: Block via request topic (`combat.block.requested`)

**Files:**
- Modify: `include/tumult/encounter.cpp` (wire the `block.requested` topic to `addBlock`).
- Test: additions to `tests/encounter_test.cpp`.

**Interfaces:**
- Consumes: `Encounter::addBlock` (Task 5).
- Produces: middle layer that effects or other request publishers can hit `combat.block.requested`; subscription inside Encounter handles the mutation.

- [ ] **Step 1: Write the failing test addition**

```cpp
TEST(EncounterBlockRequested, BlockRequestAddsBlockToFighter) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 6, .maxHp = 6});
  // Publish a block request via the bus.
  tumult::DamageEvent req{.attackerId = "hero", .targetId = "hero", .baseAmount = 4};
  rpg::core::Topic<tumult::DamageEvent> topic = tumult::blockRequestedTopic().on(e.bus());
  ASSERT_TRUE(topic.publish(req).isOk());
  const auto* hero = e.findCharacter("hero");
  ASSERT_NE(hero, nullptr);
  EXPECT_EQ(hero->block, 4);
}
```

- [ ] **Step 2: Run, verify it fails**

Run: `make test`
Expected: FAIL — `hero->block == 0` (the request topic isn't subscribed yet).

- [ ] **Step 3: Wire `block.requested` in `Encounter::subscribeRequestTopics`**

Add to `encounter.cpp::subscribeRequestTopics` (after the raw damage wiring):

```cpp
  rpg::core::Topic<DamageEvent> blockTopic = blockRequestedTopic().on(*bus_);
  blockSubId_ = blockTopic.subscribe(
      [this](const DamageEvent& req) -> rpg::core::Status {
        addBlock(req.targetId, req.baseAmount);
        return rpg::core::Status::ok();
      });
```

- [ ] **Step 4: Run, verify it passes**

Run: `make test`
Expected: new test passes.

- [ ] **Step 5: Run pre-commit, commit**

```bash
make pre-commit
git add -A
git commit -m "feat: block.requested topic mutates Character.block via Encounter subscription"
```

---

## Task 10: Integration test — chain `source` reads end-to-end

**Files:**
- Create: `tests/integration_test.cpp`
- Modify: `tests/CMakeLists.txt` (add `integration_test.cpp`)

**Interfaces:**
- Consumes: `Encounter`, `VulnerableEffect`, `ToughSkinEffect`, `BleedEffect`, `formatStep`.
- Produces: the Slice-1 acceptance gate — the simplification proof in one test that exercises the whole pipeline.

- [ ] **Step 1: Write the test**

```cpp
#include <gtest/gtest.h>

#include <string>

#include "tumult/breakdown.hpp"
#include "tumult/effects/bleed.hpp"
#include "tumult/effects/tough_skin.hpp"
#include "tumult/effects/vulnerable.hpp"
#include "tumult/encounter.hpp"

TEST(Integration, HeroStrikeBreakdownNamesEveryModifierFromReceipts) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 30, .maxHp = 30});

  tumult::VulnerableEffect vuln("goblin", 50, 2);
  ASSERT_TRUE(e.applyEffect(vuln).first.isOk());
  tumult::ToughSkinEffect toughSkin("goblin", 1);
  ASSERT_TRUE(e.applyEffect(toughSkin).first.isOk());

  auto r = e.strike("hero", "goblin", 8);
  // base: 8 -> 8 (identity base modifier)
  // effects order: subscribers fire in subscription order: vulnerable first
  //   (applied before tough_skin), then tough_skin.
  // vulnerable: ceil(8 * 1.5) = 12
  // tough_skin: max(0, 12 - 1) = 11
  // breakpoint: the chain executes stage "effects" with contributors in
  // insertion order; vulnerable was applied first.
  ASSERT_GE(r.breakdown.size(), 3u);
  EXPECT_EQ(r.breakdown[0].stage, "base");
  EXPECT_EQ(r.breakdown[0].source, "");
  EXPECT_EQ(r.breakdown[1].stage, "effects");
  EXPECT_EQ(r.breakdown[1].source, "vulnerable");
  EXPECT_EQ(r.breakdown[1].before, 8);
  EXPECT_EQ(r.breakdown[1].after, 12);
  EXPECT_EQ(r.breakdown[2].stage, "effects");
  EXPECT_EQ(r.breakdown[2].source, "tough-skin");
  EXPECT_EQ(r.breakdown[2].before, 12);
  EXPECT_EQ(r.breakdown[2].after, 11);
  EXPECT_EQ(r.finalDamage, 11);

  // The simplification proof — the formatter reads `source` from each step:
  std::string rendered;
  for (const auto& step : r.breakdown) {
    rendered += tumult::formatStep(step) + "\n";
  }
  EXPECT_NE(rendered.find("(vulnerable)"), std::string::npos);
  EXPECT_NE(rendered.find("(tough-skin)"), std::string::npos);
}

TEST(Integration, BleedAndStrikeShareTheBus) {
  tumult::Encounter e;
  e.setup({.id = "hero", .name = "Hero", .curHp = 20, .maxHp = 20},
          {.id = "goblin", .name = "Goblin", .curHp = 30, .maxHp = 30});

  tumult::BleedEffect bleed("goblin", 3, 2);
  e.applyEffect(bleed);
  e.endTurn();
  // bleed ticks: goblin takes 3 raw damage
  EXPECT_EQ(e.findCharacter("goblin")->curHp, 27);

  auto r = e.strike("hero", "goblin", 10);
  EXPECT_EQ(r.finalDamage, 10);
  // Second endTurn ticks one more then bleed self-removes.
  e.endTurn();
  EXPECT_EQ(e.findCharacter("goblin")->curHp, 27 - 3 - 10);  // 14
  // Third turn: bleed is gone.
  e.endTurn();
  EXPECT_EQ(e.findCharacter("goblin")->curHp, 14);  // no further tick
}
```

- [ ] **Step 2: Update `tests/CMakeLists.txt`**

```cmake
add_executable(tumult_tests
  character_test.cpp
  breakdown_test.cpp
  encounter_test.cpp
  vulnerable_test.cpp
  tough_skin_test.cpp
  bleed_test.cpp
  integration_test.cpp
)
```

- [ ] **Step 3: Run, verify all green**

Run: `make test`
Expected: all tests pass, including the two `Integration.*` cases — Slice 1 acceptance gate is GREEN.

If `BleedAndStrikeShareTheBus` fails on the third-turn curHp math, walk the events: bleed tick on turn N publishes raw damage, deals 3 to goblin — but ALSO striking reduces goblin HP. The test's `27 - 3 - 10 = 14` sequencing assumes tick-before-strike-before-tick. The actual order:

1. After apply bleed: goblin.curHp = 30, stacks = 2.
2. `e.endTurn()` (turn 1): bleed ticks 3 damage → goblin.curHp = 27, stacks = 1.
3. `e.strike("hero", "goblin", 10)`: combat.damage chain runs; bleed doesn't subscribe combat.damage (only turn.ended). So strike = 10 → goblin.curHp = 17.
4. `e.endTurn()` (turn 2): bleed ticks 3 → goblin.curHp = 14, stacks = 0; bleed self-removes.
5. `e.endTurn()` (turn 3): no bleed → no tick; curHp = 14.

Test reads `27 - 3 - 10 = 14` after step 4 — wrong (it should read `17 - 3 = 14` after step 4); the test assertion should check `curHp == 14` after step 4, then `curHp == 14` after step 5 (unchanged). Fix the assertion:

```cpp
EXPECT_EQ(e.findCharacter("goblin")->curHp, 14);  // 17 - 3 (turn 2 tick) = 14; bleed self-removes
e.endTurn();
EXPECT_EQ(e.findCharacter("goblin")->curHp, 14);  // bleed gone; no tick
```

Add the intermediate check `EXPECT_EQ(e.findCharacter("goblin")->curHp, 17)` after the strike too, to be honest about the order.

- [ ] **Step 4: Run pre-commit, commit**

```bash
make pre-commit
git add -A
git commit -m "test: integration - chain source reads end-to-end through formatStep (Slice 1 acceptance)"
```

---

## Task 11: CI workflow + docs freeze

**Files:**
- Create: `.github/workflows/ci.yml`
- Modify: `docs/status.md` (mark Slice 1 ready for review)
- Modify: `README.md` (status bump: Slice 1 in review)

- [ ] **Step 1: Write `.github/workflows/ci.yml`**

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:

jobs:
  build-linux:
    name: linux (${{ matrix.cxx }})
    runs-on: ubuntu-latest
    strategy:
      fail-fast: false
      matrix:
        cxx: [clang++, g++]
    env:
      CXX: ${{ matrix.cxx }}
    steps:
      - uses: actions/checkout@v4
      - name: Install toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build clang
      - name: Configure
        run: cmake --preset debug
      - name: Build
        run: cmake --build --preset debug
      - name: Test
        run: ctest --preset debug --output-on-failure

  build-windows:
    name: windows (MSVC)
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Configure
        run: cmake -B build -DTUMULT_BUILD_TESTS=ON
      - name: Build
        run: cmake build build --config Debug
      - name: Test
        run: ctest --test-dir build -C Debug --output-on-failure

  lint:
    name: lint
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build clang clang-format clang-tidy
      - name: Lint
        run: make lint
```

- [ ] **Step 2: Update `docs/status.md`**

```markdown
# tumult Status

_Last updated: <auto> (Slice 1 — ready for review)_

## Current health

Slice 1 complete and tested green on Linux (clang + gcc). The simplification
proof — `Integration.HeroStrikeBreakdownNamesEveryModifierFromReceipts` —
asserts that `formatStep` reads `source` from each chain step receipt and
names every modifier. UE activation is Slice 2 (Kirk's Windows step).

## Active work

- Slice 1 in review
  - Character, BreakdownStep + formatStep, Encounter skeleton, strike,
    Vulnerable/Tough Skin/Bleed, request topics, integration test.
- Open: PR review, then Slice 2 (rpgkit-ue migrates to host tumult).

## Edges (later slices)

- rpgkit-ue Slice 2: migrate URPGKitEncounterRuntime to hold a
  tumult::Encounter and reflect StrikeResult/BreakdownStep to HUD.
- rpgkit-demo-game Slice 7: terminal edge migrates to host tumult.

## Known rough edges

- Encounter holds a non-owning Effect* for activeEffects; ownership stays with
  the caller (mirrors rpgkit-ue today). Slice 3 (cards-as-Actions) may force
  unique_ptr; deferred.
- BreakdownStep is plain; if a later receipt shape needs host-supplied opaque
  metadata, revisit.
```

- [ ] **Step 3: Run pre-commit, commit**

```bash
make pre-commit
git add -A
git commit -m "ci: linux + windows + lint workflows; docs: Slice 1 ready for review"
```

- [ ] **Step 4: Open the PR for Slice 1**

Kirk creates the remote `KirkDiggler/tumult` (or assigns the agent), runs:

```bash
gh repo create KirkDiggler/tumult --private --source=. --remote=origin --push
git checkout -b feat/slice-1-core-seam
git push -u origin feat/slice-1-core-seam
gh pr create --title "feat: Slice 1 — core seam + simplification proof" --body "First PR for tumult. Implements the design at rpg-project/ideas/tumult/initial-plan/design.md. Closes the first-slice work; UE hookup is Slice 2 (Kirk's Windows step). Acceptance: \`Integration.HeroStrikeBreakdownNamesEveryModifierFromReceipts\` green — \`formatStep\` reads \`source\` from \`Chain::Step::source\`, not a host string."
```

Wait for Copilot review; address every comment; **NEVER merge without Kirk's explicit approval.**

---

## Self-Review

Spec coverage (design.md → tasks):
- "Repo skeleton + Character + Encounter + strike/chain/source wiring + Vulnerable/Tough Skin/Bleed + formatStep" — Tasks 1–11. ✅
- "Tests green on Linux" — Tasks 2–10 test cycles; Task 11 CI. ✅
- "`strike` against a Vulnerable target produces a breakdown whose step carries `source == \"vulnerable\"`... proven by a GoogleTest" — `vulnerable_test.cpp::StrikeAgainstVulnerable*` and `integration_test.cpp::HeroStrikeBreakdownNamesEveryModifierFromReceipts`. ✅
- "The path to a second edge (terminal) is mechanical" — `formatStep` plain C++, no host deps; `BreakdownStep` plain struct. Mechanical. ✅
- "PR #4 lands independently" — design doc + plan are silent on rpgkit-ue code changes in this slice; Task 11's PR opens on tumult, not rpgkit-ue. ✅
- Phase boundaries (non-goals honored): no cards-as-Actions, no ActionReceipt routing, no enemy intents, no second edge. ✅

Placeholder scan: axial TODOs in code (e.g., the Task 4 "no-op" `subscribeRequestTopics`) are explicitly resolved in Tasks 6–9 with full implementations. No "TBD"/"fill in".

Type consistency audit:
- `Character` fields: id/name/curHp/maxHp/block — used consistently across Tasks 2–10 (curHp IS what Task 5 mutates).
- `BreakdownStep` fields: id/stage/source/before/after — Tasks 3, 5, 6, 7, 10 all read the same shape.
- `StrikeResult` fields: finalDamage/blocked/hpAfter/breakdown — Tasks 5, 6, 7, 10 consistent.
- `Encounter::strike` signature: `(const std::string&, const std::string&, int) -> StrikeResult` — Tasks 5, 6, 7, 10 consistent.
- `Encounter::applyEffect`/`removeEffect` return `std::pair<rpg::core::Status, rpg::core::EffectReceipt>` — Tasks 6, 7, 8 consistent.
- `Combat.damage`/`turn.ended`/`raw_damage.requested`/`block.requested` topic ids are string literals in `encounter.hpp` topic defs — Tasks 4, 5, 6, 8, 9 reference the same defs.

Adjustments baked into the plan above (do not skip when executing):
- Task 6 Step 8: the `subscribeRequestTopics` "no-op" in Task 4 becomes a real base-modifier subscriber in Task 6 (or now in Task 4 — your call at execution time; the Task 6 step calls out the retraction explicitly).
- Task 5 Step 1: `NoModifiersCarriesOneBaseStep` breaks unless the base identity subscriber is wired; the Task 6 adjustment resolves it. Either sequence the adjustments in this order: Task 4 fully (with no-op) → Task 5 (test currently expects size=1 — will FAIL until Task 6 fixes `subscribeRequestTopics`) → Task 6 fixes connector. BETTER: fold the base identity subscriber into Task 4 immediately (renaming its Step 7 contents accordingly) so Task 5 passes cleanly. Recommended execution adjustment: lift the "base identity subscriber" up into Task 4 Step 6 before implementing Task 5.
- Task 10 Step 1 / Step 3: the Bleed-and-strike math assertion was incorrect; the fix is baked in.

All specs/decisions explicit; the only residual decision (deferred to plan execution) is whether `encounter.cpp` lives as `OBJECT` library (preferred here, simpler for header-only INTERFACE alignment) or as a separate `STATIC` lib. The plan chooses OBJECT.

---

## Execution Handoff

Plan complete and saved to `rpg-project/ideas/tumult/initial-plan/plan.md`. Two execution options:

**1. Subagent-Driven (recommended)** — I dispatch a fresh subagent per task, review between tasks with two-stage review. Best for keeping my context lean between tasks and giving each task a focused reviewer.

**2. Inline Execution** — Execute tasks in this session with checkpoints for review. Faster turnaround; my context fills up with implementation details.

Which approach?