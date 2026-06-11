# rpgkit Bootstrap & v1 Core Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stand up `KirkDiggler/rpgkit` — a C++20 monorepo implementing the portable combat core (Bus, Chain, Action, Effect) from `design.md` — with a dev setup Kirk can use from Windsurf on Linux.

**Architecture:** Monorepo of CMake targets under the `rpg::` namespace (Abseil/EnTT-family layout). First module is `rpg::core`, a **header-only** C++20 library implementing the four contracts in `design.md`. Later modules (`mechanics/`, `rulebooks/`) slot in beside it; consumers link only the targets they want. The v1 slice that proves the nervous system: a Strike action publishes a damage chain, a Bleed effect registers a modifier, Execute returns the result **with a breakdown**.

**Tech Stack:** C++20 · CMake ≥3.25 + Ninja + CMakePresets · Clang (primary) / GCC / MSVC (CI matrix) · GoogleTest + gMock via FetchContent · clang-format + clang-tidy · clangd (Windsurf LSP) · GitHub Actions.

---

## Decisions binding on all tasks

These come from `design.md` (the 10 decisions) plus C++-specific choices made during planning. Implementers do not relitigate them.

1. **No exceptions in the core API.** Unreal compiles with exceptions disabled by default. All fallible operations return `rpg::core::Status` (a small value type: ok or error-with-message). Never throw across the API surface; never return success on failure (the Go "never return (nil, nil)" rule, translated).
2. **Header-only `rpg::core`** (`INTERFACE` CMake target). The typed veneer is templates anyway; header-only makes FetchContent and Unreal-plugin vendoring trivial. Compiled TUs can come later if build times demand.
3. **Type erasure via `std::any`** in the Bus. The bus routes `string topic id → opaque payload`; the typed `Topic<T>` veneer casts in/out (design decision 1). `std::variant` or void*+tag are later optimizations, not v1.
4. **Stages only — no integer priorities** (design "clean start" section). `Chain<T>` is constructed with an ordered stage list; modifiers join a named stage.
5. **Subscription ids are opaque handles** (`SubscriptionId`, a wrapped `uint64_t`), so a future C ABI can pass them across the wire (design language-mapping notes).
6. **The breakdown is in scope for v1.** The v1 slice is defined as "Execute the chain, read the breakdown" — `Chain<T>::execute` returns the folded value *and* a per-modifier record (id, stage). It is the core selling point (design decision 7).
7. **Naming:** types `PascalCase`, methods/functions `lowerCamelCase` (matches the design-doc contracts: `canActivate`, `isActive`), headers `snake_case.hpp` under `include/rpg/core/`. The notification-vs-chained distinction lives in the *type* (`Topic<T>` vs `ChainedTopic<T>`), so both use plain `subscribe`/`publish` rather than the design's `subscribeWithChain` method-name spelling.
8. **Synchronous, ordered, fail-fast delivery** (design decision 9). Publish walks subscribers in insertion order and returns the first error.
9. **Workflow:** issue-first, one PR per logical unit, never merge without Kirk's approval, wait for Copilot review and reply on every thread. `make pre-commit` green before every push.

## PR breakdown

| PR | Issue | Contents | Proves |
|----|-------|----------|--------|
| 1 | rpgkit#1 | Repo scaffold: CMake, presets, Makefile, lint config, CI, `core/` skeleton, docs, dev-setup guide | `make test` green locally and in CI on Linux (Clang+GCC) and Windows (MSVC) |
| 2 | rpgkit#2 | `Status`, type-erased `Bus`, staged `Chain<T>` with breakdown | Pub/sub routing; staged fold with breakdown |
| 3 | rpgkit#3 | Typed topic veneer: `TopicDef<T>`, `Topic<T>`, `ChainedTopic<T>` | The `.on(bus)` pattern; notification vs chained at type level |
| 4 | rpgkit#4 | `Action`, `Effect` (tracked lifecycle), `examples/strike` | The whole v1 slice end-to-end |

Each PR: branch from fresh main (`gcm && gl && gcb`), TDD per task, `make pre-commit` before push, open PR, Copilot review loop, Kirk merges.

---

## PR 1 — Repo scaffold

### Task 1.1: Create the repo

- [ ] **Step 1:** `gh repo create KirkDiggler/rpgkit --public --description "Composable combat-systems toolkit for game builders — C++ implementation of rpg-toolkit's design" --clone` into `/home/kirk/personal/rpgkit`
- [ ] **Step 2:** Initial commit on main: `README.md` (one paragraph: what it is, link to rpg-toolkit and to `rpg-project/ideas/portable-combat-core/design.md`, "early — not yet usable" status), `LICENSE` (MIT, copyright Kirk Diggler), `.gitignore` (`build*/`, `compile_commands.json`, `.cache/`)
- [ ] **Step 3:** File issue rpgkit#1 "Repo scaffold: build system, CI, core module skeleton" and create branch `feat/scaffold`

### Task 1.2: Build system

**Files:** `CMakeLists.txt`, `CMakePresets.json`, `core/CMakeLists.txt`, `core/include/rpg/core/version.hpp`, `core/tests/version_test.cpp`, `Makefile` (a shared `cmake/` helpers dir is deferred until a module actually needs one)

- [ ] **Step 1:** Root `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(rpgkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RPGKIT_BUILD_TESTS "Build rpgkit tests" ${PROJECT_IS_TOP_LEVEL})

if(RPGKIT_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # MSVC
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  include(GoogleTest)
endif()

add_subdirectory(core)
```

(PR 4 adds `option(RPGKIT_BUILD_EXAMPLES ...)` and `add_subdirectory(examples)` when `examples/` exists; including them now would break configure.)

- [ ] **Step 2:** `core/CMakeLists.txt`:

```cmake
add_library(rpgkit_core INTERFACE)
add_library(rpg::core ALIAS rpgkit_core)
target_include_directories(rpgkit_core INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(rpgkit_core INTERFACE cxx_std_20)

if(RPGKIT_BUILD_TESTS)
  add_executable(rpgkit_core_tests tests/version_test.cpp)
  target_link_libraries(rpgkit_core_tests PRIVATE rpg::core GTest::gtest_main)
  # add GTest::gmock here when a test first needs it — core v1 tests use hand-written fakes
  gtest_discover_tests(rpgkit_core_tests)
endif()
```

(Each later PR appends its test files to `rpgkit_core_tests`.)

- [ ] **Step 3:** `CMakePresets.json` — `debug` and `release` configure presets (Ninja generator, `build/debug` and `build/release` binary dirs, debug adds `-Wall -Wextra -Werror` via `CMAKE_CXX_FLAGS` for GCC/Clang), matching build and test presets.
- [ ] **Step 4:** `core/include/rpg/core/version.hpp` — namespace `rpg::core`, `inline constexpr std::string_view kVersion = "0.1.0";`
- [ ] **Step 5:** `core/tests/version_test.cpp` — one gtest asserting `kVersion == "0.1.0"`. This exists to prove the toolchain pipeline, not to test anything real.
- [ ] **Step 6:** `Makefile` wrapping the presets with Kirk's verbs:

```make
.PHONY: configure build test lint fmt fmt-check tidy pre-commit clean

configure:
	cmake --preset debug

build: configure
	cmake --build --preset debug

test: build
	ctest --preset debug --output-on-failure

FMT_FILES = $(shell find core examples -name '*.hpp' -o -name '*.cpp' 2>/dev/null)

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

- [ ] **Step 7:** Run `make test` — expect the version test to pass. Commit.

### Task 1.3: Lint/format config

**Files:** `.clang-format`, `.clang-tidy`, `.editorconfig`

- [ ] **Step 1:** `.clang-format`: `BasedOnStyle: Google`, `ColumnLimit: 100`, `IndentWidth: 2`, `DerivePointerAlignment: false`, `PointerAlignment: Left`
- [ ] **Step 2:** `.clang-tidy`: checks `bugprone-*, performance-*, modernize-*, readability-*, cppcoreguidelines-*` with noisy ones disabled (`-modernize-use-trailing-return-type`, `-readability-identifier-length`); `WarningsAsErrors: '*'`; **`HeaderFilterRegex: 'core/include/.*'`** — without it, clang-tidy only diagnoses test `.cpp` files and silently skips the header-only library, which is the entire product
- [ ] **Step 3:** `.editorconfig`: utf-8, lf, 2-space for C++/CMake/yaml, final newline
- [ ] **Step 4:** `make pre-commit` green. Commit.

### Task 1.4: CI

**Files:** `.github/workflows/ci.yml`

- [ ] **Step 1:** Matrix: `ubuntu-latest` × {clang++, g++} and `windows-latest` × MSVC. Linux jobs: install ninja + clang, `cmake --preset debug && cmake --build --preset debug && ctest --preset debug`. Windows job: use default generator (VS), `cmake -B build -DRPGKIT_BUILD_TESTS=ON && cmake --build build --config Debug && ctest --test-dir build -C Debug`. A separate `lint` job on Linux installs clang-format + clang-tidy + ninja and runs `make lint` (configure runs first via the `tidy` dependency, producing the compilation database) so CI enforces exactly what `make pre-commit` does.
- [ ] **Step 2:** Push, verify all jobs green on the PR. Commit fixes as needed.

### Task 1.5: Docs + CLAUDE.md

**Files:** `docs/status.md`, `docs/quality.md`, `docs/architecture/README.md`, `docs/how-to/dev-setup.md`, `CLAUDE.md`

- [ ] **Step 1:** `docs/architecture/README.md` — the monorepo layout, the four contracts (summarized from design.md with a link to `rpg-project/ideas/portable-combat-core/design.md` as the source design), the binding decisions above (no exceptions, header-only, std::any, stages-only).
- [ ] **Step 2:** `docs/how-to/dev-setup.md` — Linux setup: `sudo apt install cmake ninja-build clang clangd clang-format clang-tidy`; clone; `make test`. Windsurf: install the **clangd** extension (disable Microsoft C/C++ IntelliSense if present), `make configure` once so `build/debug/compile_commands.json` exists, plus a checked-in `.clangd` file pointing at it (`CompileFlags: { CompilationDatabase: build/debug }`). Include "coming from Go" notes: clangd ≈ gopls, CMake target ≈ go module, FetchContent ≈ go get, gtest fixture ≈ testify suite.
- [ ] **Step 3:** `docs/status.md` (current health: scaffold only; active: core v1) and `docs/quality.md` (scorecard, everything starts honest: scaffold A, core N/A)
- [ ] **Step 4:** `CLAUDE.md` — repo role (C++ implementation of the portable combat core; rpg-toolkit sibling), the binding decisions table, the boundary with rulebooks ("core is system-agnostic; D&D and deck-builders are rulebooks on top"), workflow (issue-first, `make pre-commit`, never merge without approval), pointer to design.md in rpg-project.
- [ ] **Step 5:** `make pre-commit`, commit, push, open PR 1. Copilot review loop. **Kirk merges.**

---

## PR 2 — Status, Bus, Chain

**Files:** `core/include/rpg/core/status.hpp`, `core/include/rpg/core/bus.hpp`, `core/include/rpg/core/chain.hpp`; tests `core/tests/status_test.cpp`, `core/tests/bus_test.cpp`, `core/tests/chain_test.cpp`

### Task 2.1: `Status`

Contract:

```cpp
namespace rpg::core {
class Status {
 public:
  static Status ok();
  static Status error(std::string message);
  bool isOk() const;
  const std::string& message() const;  // empty when ok
};
}
```

- [ ] TDD: test ok/error construction, `isOk`, message round-trip → implement → `make test` green → commit.

### Task 2.2: Type-erased `Bus`

Contract (design decisions 1, 8, 9):

```cpp
namespace rpg::core {
struct SubscriptionId { std::uint64_t value = 0; };  // 0 = invalid

class Bus {
 public:
  using Handler = std::function<Status(const std::any& payload)>;
  SubscriptionId subscribe(std::string_view topicId, Handler handler);
  Status unsubscribe(SubscriptionId id);
  Status publish(std::string_view topicId, const std::any& payload);
};
}
```

Behaviors to test (each its own gtest, write test first):
- [ ] publish with no subscribers is ok (no-op)
- [ ] one subscriber receives the payload
- [ ] multiple subscribers called in subscription order (record order in a vector)
- [ ] fail-fast: first handler error stops delivery, later handlers not called, error propagates
- [ ] unsubscribe removes the handler; unknown id → error
- [ ] topics are independent: publish on topic A doesn't reach topic B subscribers
- [ ] commit per green behavior or as one reviewed unit; `make pre-commit` before push

### Task 2.3: Staged `Chain<T>` with breakdown

Contract (design decisions 4–7; binding decision 6):

```cpp
namespace rpg::core {
template <typename T>
class Chain {
 public:
  explicit Chain(std::vector<std::string> stages);
  Status add(std::string_view stage, std::string_view id, std::function<T(T)> modifier);
  Status remove(std::string_view id);

  struct Step { std::string id; std::string stage; T before; T after; };
  struct Result { T value; std::vector<Step> breakdown; };
  Result execute(T data) const;
};
}
```

Behaviors to test:
- [ ] execute with no modifiers returns input unchanged, empty breakdown
- [ ] modifiers fold in stage-list order regardless of add order (add "final" stage modifier before "base" stage modifier; verify base applies first)
- [ ] within a stage, insertion order
- [ ] duplicate id → error from `add`; unknown stage → error from `add`
- [ ] `remove` pulls a modifier; removed modifier doesn't run; unknown id → error
- [ ] breakdown records each modifier's id, stage, before/after (the "rage: +2, bless: +4" story — test with int damage: base 5, +2, ×2 across stages → 14, 3 breakdown steps)
- [ ] modifiers are re-evaluated per execute (a modifier reading a counter yields different results on two executes — the "Bless re-rolls" property, design decision 6)
- [ ] PR 2: `make pre-commit`, push, Copilot loop, **Kirk merges**

---

## PR 3 — Typed topic veneer

**Files:** `core/include/rpg/core/topic.hpp`; tests `core/tests/topic_test.cpp`

Contract (design decisions 1–3):

```cpp
namespace rpg::core {
template <typename T>
class TopicDef {
 public:
  explicit TopicDef(std::string id);            // defined once, statically
  Topic<T> on(Bus& bus) const;                  // bind at runtime
  ChainedTopic<T> onChained(Bus& bus) const;
};

template <typename T>
class Topic {  // notification flavor: one-way, no transformation
 public:
  SubscriptionId subscribe(std::function<Status(const T&)> handler);
  Status publish(const T& event);
};

template <typename T>
class ChainedTopic {  // chained flavor: collects modifiers
 public:
  SubscriptionId subscribe(std::function<Status(const T&, Chain<T>&)> handler);
  Status publish(const T& event, Chain<T>& chain);  // collect only — execute is the caller's verb
};
}
```

Note the design's two-verbs rule: `ChainedTopic::publish` *collects* contributions into the chain; the caller then calls `chain.execute(...)`. Publishing never transforms.

**Deliberate adaptation from the design contracts:** the design spells these `subscribeWithChain` / `publishWithChain(T, Chain<T>) -> Chain<T>` (functional return). Here the flavor distinction already lives in the type (`ChainedTopic<T>`), so the methods keep plain names, the chain is passed by mutable reference, and the return slot carries `Status` instead — required by the no-exceptions decision, which needs every fallible call to return `Status`.

Behaviors to test:
- [ ] typed payload arrives intact through the erased bus (struct with fields, not just int)
- [ ] wrong-type payload on the same topic id surfaces as an error, not UB (the veneer's `std::any_cast` failure path returns `Status::error`)
- [ ] one `TopicDef` binds to two buses; publish on bus A doesn't reach bus B (the per-encounter-bus story, design decision 2)
- [ ] chained: handler registers a modifier into the chain during publish; chain is unchanged until `execute`
- [ ] notification handlers can't mutate the event (parameter is `const T&` — this is the compile-time guarantee; document it in the header comment)
- [ ] PR 3: `make pre-commit`, push, Copilot loop, **Kirk merges**

---

## PR 4 — Action, Effect, and the Strike example

**Files:** `core/include/rpg/core/action.hpp`, `core/include/rpg/core/effect.hpp`, `core/include/rpg/core/entity.hpp`; tests `core/tests/effect_test.cpp`, `core/tests/action_test.cpp`; example `examples/CMakeLists.txt`, `examples/strike/main.cpp` (+ root CMakeLists gets `add_subdirectory(examples)`)

### Task 4.1: `EntityRef` + `Action`

```cpp
namespace rpg::core {
struct EntityRef { std::string id; std::string type; };

template <typename TInput>
class Action {  // abstract base
 public:
  virtual ~Action() = default;
  const std::string& id() const;
  const std::string& type() const;
  virtual Status canActivate(const EntityRef& owner, const TInput& input) = 0;
  virtual Status activate(const EntityRef& owner, const TInput& input) = 0;
};
}
```

- [ ] TDD with a test double action (a fake, not a gmock — it's a base class exercise): canActivate gate refuses, activate publishes; commit

### Task 4.2: `Effect` with tracked lifecycle

```cpp
namespace rpg::core {
class Effect {  // abstract base
 public:
  virtual ~Effect() = default;
  const std::string& id() const;
  const std::string& source() const;   // what granted it
  Status apply(Bus& bus);              // calls onApply, remembers the bus, marks active
  Status remove();                     // auto-unsubscribes everything tracked, on the bus it was applied to
  bool isActive() const;
 protected:
  virtual Status onApply(Bus& bus) = 0;  // subclass subscribes via track()
  void track(SubscriptionId id);         // subscription tracker (design decision 10)
};
}
```

`apply` stores the bus (non-owning pointer); `remove` takes no bus parameter — this makes the "remove from a different bus than applied" mistake unrepresentable. Lifetime precondition, documented in the header: the bus must outlive applied effects (true by construction — an encounter's bus outlives its effects).

Behaviors to test:
- [ ] apply subscribes and marks active; the effect's handler fires on publish
- [ ] remove auto-unsubscribes **all** tracked subscriptions (subscribe to two topics in onApply; after remove, neither fires); marks inactive
- [ ] double-apply → error; remove-without-apply → error
- [ ] commit per green unit

### Task 4.3: The Strike example (the v1 acceptance test)

`examples/strike/main.cpp` — the design's v1 slice, runnable:

- [ ] Define `DamageEvent { EntityRef attacker; EntityRef target; int amount; }`, a `damageTopic` (chained), stage list `{"base", "effects", "final"}`
- [ ] `StrikeAction` (Action<StrikeInput>): activate builds a fresh `Chain<DamageEvent>`, publishes via the chained topic, executes, prints value + breakdown
- [ ] `BleedEffect` (Effect): onApply subscribes to the chained damage topic and registers a `+2` modifier with id `"bleed-" + source` into the `"effects"` stage
- [ ] `main`: create bus → apply BleedEffect → StrikeAction.activate → output shows base damage, bleed contribution in the breakdown, final value. Then `remove` the effect, strike again, bleed is gone.
- [ ] Mirror the same flow as a gtest integration test in `core/tests/` (the example is for humans; the test is for CI)
- [ ] Update `docs/status.md` + `docs/quality.md` + `docs/architecture/` for the now-real core (living docs update IN the PR, per workspace convention)
- [ ] PR 4: `make pre-commit`, push, Copilot loop, **Kirk merges** → **v1 slice done**

---

## Local dev setup (parallel to PR 1, this machine)

- [ ] `sudo apt install ninja-build clang clangd clang-format clang-tidy` (cmake 4.2.3 and gcc 15 already present)
- [ ] In the rpgkit clone: `make test` green
- [ ] Windsurf: clangd extension; open repo; verify go-to-definition works on `rpg::core::kVersion` (proves compile_commands wiring)

## Verification gate (every PR)

Before any implementer reports done (the four questions): Does it meet the **goal** of its issue? Does it follow the **patterns** (binding decisions above)? Do the **tests** prove the behaviors listed, run green via `make pre-commit`? What would a reviewer **push back** on?

## Out of scope (deferred, from design.md)

Resources/energy, targeting, turn structure, decks, async delivery, the C ABI layer for Unity, the Unreal plugin wrapper, `mechanics/` and `rulebooks/` modules. v1 ends at the Strike example proving Bus + Chain + Action + Effect compose.
