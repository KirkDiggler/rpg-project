# tumult Wave A1 — Consumption Contract + Reference Recipe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn `docs/host-integration.md` (created by tumult PR #4) into a copy-paste TumultUE consumption recipe, and prove that recipe with a CI-verified sample consumer (`examples/host-consumer/`) that compiles tumult "the documented way" and runs as a ctest in the existing CI jobs.

**Architecture:** A1 is a self-contained PR **in the tumult repo**. It adds no new public API. It (1) enriches the host-integration doc with the literal UE 5.8 recipe — `Tumult.Build.cs`, a pinned `sync-tumult.sh`, a `Tumult.h` umbrella header, and a `GetEncounter()`/`BeginPlay()` first call — and (2) adds a standalone executable, `examples/host-consumer/`, that vendors tumult's four stateful `.cpp` the way a host does, links rpgkit's header-only `rpg::core` (nothing else), constructs a `tumult::Encounter`, calls `strike`, and returns nonzero on any unexpected result. The executable is registered with `add_test(...)` so the **existing** ctest CI jobs run it — no `ci.yml` change. The host is **tumult-ue** (UE 5.8); we ship the recipe, their team applies it (we do **not** commit to the tumult-ue repo).

**Tech Stack:** C++20, CMake ≥ 3.25 (Ninja presets `debug`/`release`), rpgkit `rpg::core` (header-only `INTERFACE` target via FetchContent at `v0.3.0`), GoogleTest (unaffected by A1), Unreal Engine 5.8 (recipe target only — UE code lives in docs, never in the repo).

---

## Sequencing — read before starting

**A1 is sequenced AFTER tumult PR #4 (`docs/3-refresh-host-integration-docs`) merges.** A1 *edits* the `docs/host-integration.md` that PR #4 creates. The executor therefore branches from a `main` that already contains PR #4. Task 1 Step 2 asserts the file is present and **stops** if it is not (PR #4 has not merged yet — do not proceed).

A1 does **not** cut a new release: tumult `v0.1.0` is already tagged + released, and the recipe pins it. rpgkit is pinned at `v0.3.0` (the tag tumult `v0.1.0` itself builds against — see tumult's top-level `CMakeLists.txt` `FetchContent ... GIT_TAG v0.3.0`).

> Note on the brief: the source brief said "pins `v0.1.0` for both tumult and rpgkit." That is corrected here to **tumult `v0.1.0` + rpgkit `v0.3.0`**, because tumult `v0.1.0` consumes rpgkit `v0.3.0` receipts (`Chain::Step::source`, `EffectReceipt`); pinning rpgkit to `v0.1.0` would not compile. This is recorded in DR-008's interface delta.

---

## Per-wave checklist (capabilities.md § "How we work" — filled for A1)

- [x] **Goal behavior named + demand cited:** a host compiles tumult via the documented recipe and calls `Encounter` from `BeginPlay`. Proofs **tumult-ue#4**; completes **tumult#3** (host-integration docs).
- [x] **Primitive(s)/seam(s):** the **host↔library edge** — the include-vs-link consumption seam. No new primitive; A1 documents and proves the consumption contract.
- [x] **TDD / observable done-when:** the sample consumer **is** the executable test. `ctest --preset debug -R host_consumer` passes locally and in the existing CI ctest jobs; `make pre-commit` green; never `--no-verify`.
- [x] **DRs:** DR-008 (vendor source + compile as a host module) and DR-009 (recipe verified by a CI sample consumer) are appended to `decisions.md` as part of this wave.
- [x] **Boundary held — no host types in `include/tumult/`:** A1 adds nothing to `include/tumult/`. The UE snippets (`UCLASS`, `FString`, `UE_LOG`) live **only** in `docs/host-integration.md`. `examples/host-consumer/main.cpp` uses **only** `tumult::` + `rpg::core::` types — zero UE types — so it builds on any toolchain in CI.
- [x] **Stop-and-ask:** if `docs/host-integration.md` is absent at Task 1 (PR #4 not merged) — stop and report; do not recreate the file.

---

## File Structure (every file A1 creates/modifies — all in the tumult repo)

| File | Action | Responsibility |
|------|--------|----------------|
| `examples/host-consumer/main.cpp` | **Create** | The recipe proof: constructs `tumult::Encounter`, sets up two characters, calls `strike`, prints the result + breakdown via `formatStep`, returns `EXIT_FAILURE` on any unexpected value. Uses only `tumult::` + `rpg::core::` — no UE, no host types. |
| `examples/host-consumer/CMakeLists.txt` | **Create** | Builds `host_consumer` "the documented way": `main.cpp` + the four stateful `src/*.cpp`, `include/` on the include path, `rpg::core` linked (header-only — adds rpgkit include dir + `cxx_std_20`, nothing to link), `cxx_std_20`. Registers `add_test(NAME host_consumer ...)`. |
| `CMakeLists.txt` (top level) | **Modify** | Add `TUMULT_BUILD_EXAMPLES` option (default = `TUMULT_BUILD_TESTS`) + a guarded `add_subdirectory(examples/host-consumer)`. |
| `docs/host-integration.md` | **Modify** | Insert the copy-paste TumultUE recipe (`sync-tumult.sh`, `Tumult.Build.cs`, `Tumult.h`, `TumultUE.Build.cs`, `ATumultGameMode` first call) + a pointer to the CI sample consumer. |
| `docs/status.md` | **Modify** | Note A1 (recipe + CI consumer) under current health / active work. |
| `.github/workflows/ci.yml` | **Not modified** | **Stated explicitly:** the existing `build-linux` (`ctest --preset debug`) and `build-windows` (`ctest --test-dir build -C Debug`) jobs run all registered tests, including `host_consumer`, because `TUMULT_BUILD_EXAMPLES` defaults on whenever `TUMULT_BUILD_TESTS` is on (both CI jobs set tests on). No CI change is required. |
| `Makefile` | **Not modified** | **Stated explicitly:** `make lint`'s `FMT_FILES` covers `include src tests` only, so `examples/` is intentionally outside fmt-check/clang-tidy. A1's example is a build/run gate, not a style gate; keeping `examples/` out of lint avoids tidy surprises and keeps scope minimal. Extending `FMT_FILES` to `examples/` is a possible future tidy-up, out of scope for A1. |

---

## Task 1: Issue, branch from PR-#4-inclusive main, verify baseline green

**Files:** none created/modified (setup + verification only — no commit in this task).

- [ ] **Step 1: Confirm the A1 issue (already created — do NOT create another)**

The Wave A1 issue already exists: **tumult#5** ("Wave A1: consumption contract + reference recipe"), already on board #17 (Domain: Host & Wrap Surface, Horizon: Wrap Foundation). The commits and PR below reference `#5`; the PR closes it.

```bash
gh issue view 5 --repo KirkDiggler/tumult --json title,state -q '"\(.state): \(.title)"'
```

Expected: `OPEN: Wave A1: consumption contract + reference recipe`.

- [ ] **Step 2: Start from fresh main and confirm PR #4 has merged**

```bash
cd /home/kirk/personal/tumult
git checkout main && git pull
test -f docs/host-integration.md && echo "PR #4 present — OK to proceed" || echo "STOP: PR #4 not merged yet"
```

Expected: `PR #4 present — OK to proceed`.
**If you see `STOP`:** halt and report — A1 is sequenced after PR #4; do not recreate `docs/host-integration.md`.

- [ ] **Step 3: Create the A1 branch**

```bash
git checkout -b feat/a1-consumption-recipe
```

Expected: `Switched to a new branch 'feat/a1-consumption-recipe'`.

- [ ] **Step 4: Verify the baseline builds + tests green before any change**

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Expected: configure ends with `-- Generating done`; build completes with no errors; ctest ends with `100% tests passed, 0 tests failed`. This is the clean baseline the example is added on top of. (No commit — nothing changed yet.)

---

## Task 2: The CI sample consumer — `examples/host-consumer/` compiles + runs

**Files:**
- Create: `examples/host-consumer/main.cpp`
- Create: `examples/host-consumer/CMakeLists.txt`
- Modify: `CMakeLists.txt` (top level) — add `TUMULT_BUILD_EXAMPLES` + `add_subdirectory`

**API verified against the real headers** (`include/tumult/{character,encounter,breakdown}.hpp`, `src/encounter.cpp`): `tumult::Character{ std::string id; std::string name; int curHp; int maxHp; int block; }`; `tumult::Encounter()` default-constructs; `void setup(const Character&, const Character&)`; `StrikeResult strike(const std::string&, const std::string&, int base)` returns `StrikeResult{ int finalDamage; int blocked; int hpAfter; std::vector<BreakdownStep> breakdown; }`; an unmodified `strike("hero","goblin",5)` against a 20-HP, 0-block target yields `finalDamage == 5`, `blocked == 0`, `hpAfter == 15`, and exactly one breakdown step `{stage:"base", id:"", source:"", before:5, after:5}`; `const Character* findCharacter(const std::string&)`; `std::string formatStep(const BreakdownStep&)` renders the base step as `base: 5 -> 5`.

- [ ] **Step 1: Write `examples/host-consumer/main.cpp`**

```cpp
// host-consumer — a CI-verified proof of the docs/host-integration.md recipe.
//
// It compiles and runs tumult "the documented way": the four stateful sources
// plus the include/ headers plus rpgkit's header-only rpg::core, all at C++20.
// It uses ONLY tumult:: and rpg::core:: types — no Unreal, no host types — so
// it is a faithful, portable stand-in for a host's first call. If the recipe
// drifts, this stops compiling/passing and the existing ctest CI jobs fail:
// the recipe cannot rot silently.

#include <cstdlib>
#include <iostream>

#include "tumult/breakdown.hpp"
#include "tumult/character.hpp"
#include "tumult/encounter.hpp"

int main() {
  tumult::Encounter encounter;

  const tumult::Character hero{
      .id = "hero", .name = "Hero", .curHp = 30, .maxHp = 30, .block = 0};
  const tumult::Character goblin{
      .id = "goblin", .name = "Goblin", .curHp = 20, .maxHp = 20, .block = 0};
  encounter.setup(hero, goblin);

  const tumult::StrikeResult result = encounter.strike("hero", "goblin", 5);

  std::cout << "host-consumer: strike hero -> goblin (base 5)\n";
  std::cout << "  finalDamage = " << result.finalDamage << "\n";
  std::cout << "  blocked     = " << result.blocked << "\n";
  std::cout << "  hpAfter     = " << result.hpAfter << "\n";
  std::cout << "  breakdown:\n";
  for (const tumult::BreakdownStep& step : result.breakdown) {
    std::cout << "    " << tumult::formatStep(step) << "\n";
  }

  // Unmodified strike: no effects, no block. finalDamage == base; the target's
  // HP drops by exactly that much.
  if (result.finalDamage != 5) {
    std::cerr << "FAIL: expected finalDamage 5, got " << result.finalDamage << "\n";
    return EXIT_FAILURE;
  }
  if (result.blocked != 0) {
    std::cerr << "FAIL: expected blocked 0, got " << result.blocked << "\n";
    return EXIT_FAILURE;
  }
  if (result.hpAfter != 15) {
    std::cerr << "FAIL: expected hpAfter 15, got " << result.hpAfter << "\n";
    return EXIT_FAILURE;
  }

  const tumult::Character* target = encounter.findCharacter("goblin");
  if (target == nullptr || target->curHp != 15) {
    std::cerr << "FAIL: goblin curHp not mutated to 15\n";
    return EXIT_FAILURE;
  }

  std::cout << "host-consumer: OK\n";
  return EXIT_SUCCESS;
}
```

- [ ] **Step 2: Write `examples/host-consumer/CMakeLists.txt`**

```cmake
# host-consumer — the CI proof of the docs/host-integration.md recipe.
#
# This deliberately mirrors a HOST, not tumult's own build: it does NOT link the
# tumult::core target. Instead it compiles the same four stateful sources a host
# vendors, puts include/ on the include path, and links rpgkit's rpg::core —
# which is a header-only INTERFACE target, so "linking" it only adds rpgkit's
# include dir and cxx_std_20; there is no rpgkit library to link (DR-008).

add_executable(host_consumer
  main.cpp
  ${PROJECT_SOURCE_DIR}/src/encounter.cpp
  ${PROJECT_SOURCE_DIR}/src/effects/vulnerable.cpp
  ${PROJECT_SOURCE_DIR}/src/effects/tough_skin.cpp
  ${PROJECT_SOURCE_DIR}/src/effects/bleed.cpp
)

# Two include roots, exactly as the recipe documents:
#   include/   -> resolves #include "tumult/..."
#   rpg::core  -> resolves #include "rpg/core/..." (header-only, nothing linked)
target_include_directories(host_consumer PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(host_consumer PRIVATE rpg::core)
target_compile_features(host_consumer PRIVATE cxx_std_20)

add_test(NAME host_consumer COMMAND host_consumer)
```

- [ ] **Step 3: Wire the example into the top-level `CMakeLists.txt`**

In `CMakeLists.txt`, add the `TUMULT_BUILD_EXAMPLES` option immediately after the existing `TUMULT_BUILD_TESTS` option line. Change:

```cmake
option(TUMULT_BUILD_TESTS "Build tumult tests" ${PROJECT_IS_TOP_LEVEL})
```

to:

```cmake
option(TUMULT_BUILD_TESTS "Build tumult tests" ${PROJECT_IS_TOP_LEVEL})
option(TUMULT_BUILD_EXAMPLES "Build the host-consumer recipe proof (needs tests for ctest)" ${TUMULT_BUILD_TESTS})
```

Then, at the **end** of the file, change:

```cmake
if(TUMULT_BUILD_TESTS)
  add_subdirectory(tests)
endif()
```

to:

```cmake
if(TUMULT_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(TUMULT_BUILD_EXAMPLES)
  # The example registers a ctest; ensure testing is enabled even in the
  # (non-default) examples-on/tests-off configuration.
  if(NOT TUMULT_BUILD_TESTS)
    enable_testing()
  endif()
  add_subdirectory(examples/host-consumer)
endif()
```

- [ ] **Step 4: Configure**

```bash
cmake --preset debug
```

Expected: ends with `-- Configuring done` then `-- Generating done`. No errors about `host_consumer` or `rpg::core`.

- [ ] **Step 5: Build the example**

```bash
cmake --build --preset debug --target host_consumer
```

Expected: compiles `main.cpp` + the four `src/*.cpp` and links `host_consumer` with no errors or warnings.

- [ ] **Step 6: Run it as a ctest and directly, confirm expected output**

```bash
ctest --preset debug -R host_consumer --output-on-failure
```

Expected:

```
    Start 1: host_consumer
1/1 Test #1: host_consumer ....................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1
```

Then confirm the exact program output:

```bash
./build/debug/examples/host-consumer/host_consumer
```

Expected (exact):

```
host-consumer: strike hero -> goblin (base 5)
  finalDamage = 5
  blocked     = 0
  hpAfter     = 15
  breakdown:
    base: 5 -> 5
host-consumer: OK
```

- [ ] **Step 7: Confirm the full suite still passes (example didn't break tests)**

```bash
ctest --preset debug --output-on-failure
```

Expected: `100% tests passed` (the prior baseline count + 1 for `host_consumer`).

- [ ] **Step 8: Commit**

```bash
git add examples/host-consumer/main.cpp examples/host-consumer/CMakeLists.txt CMakeLists.txt
git commit -m "feat: CI-verified host-consumer example proving the consumption recipe (#5)"
```

(The issue is tumult#5.)

---

## Task 3: Enrich `docs/host-integration.md` with the copy-paste TumultUE recipe

**Files:**
- Modify: `docs/host-integration.md`

The file already exists (from PR #4) with sections: `# Host Integration`, `## Current Consumption Shape`, `## First TumultUE Step`, `## Boundary Rule`. A1 inserts a new `## Vendoring Tumult into TumultUE (copy-paste recipe)` section **between** `## Current Consumption Shape` and `## First TumultUE Step`, and adds a one-line CI-proof pointer at the end of `## Current Consumption Shape`.

- [ ] **Step 1: Add the CI-proof pointer to the end of `## Current Consumption Shape`**

Find this exact paragraph (the last paragraph of `## Current Consumption Shape`):

```
Tumult is currently CMake-first. There is no packaged Unreal plugin, installed
CMake package, or prebuilt static library yet. For the first TumultUE boundary,
it is acceptable for the Unreal module to include the headers and compile the
small Tumult source set directly. If that becomes painful, promote packaging or a
single host-facing library target as a separate Tumult issue.
```

Immediately after it, add:

```
> This exact compile path — the four stateful sources + `include/` + rpgkit's
> header-only `rpg::core`, at C++20 — is proven in CI by `examples/host-consumer/`
> (the `host_consumer` ctest). If the recipe below drifts from what actually
> compiles, that test fails. The recipe is verified, not just described.
```

- [ ] **Step 2: Insert the recipe section before `## First TumultUE Step`**

Find the heading line `## First TumultUE Step` and insert the following block immediately **before** it (keep `## First TumultUE Step` and everything after it unchanged):

````markdown
## Vendoring Tumult into TumultUE (copy-paste recipe)

TumultUE is on Unreal Engine 5.8. The recipe below vendors Tumult `v0.1.0` and
rpgkit `v0.3.0` (the rpgkit tag Tumult `v0.1.0` builds against) into the host as
a single UE module and makes the first `Encounter` call from `BeginPlay`. Tumult
uses C++20 (designated initializers, `std::ranges`), so the module compiles its
boundary as C++20.

**rpgkit is header-only.** Its `rpg::core` is an `INTERFACE` CMake target
upstream — there is no rpgkit library to build or link. The host only needs
rpgkit's headers on the include path so Tumult's sources resolve
`#include "rpg/core/..."`. The recipe puts both Tumult's `include/` and rpgkit's
`core/include` on the module's `PublicIncludePaths`.

### Vendored layout

```text
TumultUE/
  scripts/sync-tumult.sh                 # pins + fetches the vendored snapshot
  Source/
    TumultUE/
      TumultUE.Build.cs                  # host module; depends on "Tumult"
      ATumultGameMode.h                  # first call
      ATumultGameMode.cpp
    ThirdParty/
      Tumult/
        Tumult.Build.cs                  # the vendored module
        Tumult.h                         # umbrella header
        .pinned-tags                     # written by sync-tumult.sh
        include/tumult/*.hpp             # vendored from tumult v0.1.0
        src/*.cpp                        # the four stateful sources
        rpgkit/core/include/rpg/core/*.hpp   # vendored from rpgkit v0.3.0 (headers only)
```

### `scripts/sync-tumult.sh`

Pins are explicit constants so the vendored snapshot is reproducible. Re-run to
bump the pins. rpgkit is header-only, so only its headers are copied — there is
nothing to build or link.

```bash
#!/usr/bin/env bash
# Sync the vendored Tumult module to pinned release tags.
# Usage: ./scripts/sync-tumult.sh
set -euo pipefail

TUMULT_TAG="v0.1.0"
RPGKIT_TAG="v0.3.0"
DEST="Source/ThirdParty/Tumult"

echo "Syncing Tumult $TUMULT_TAG + rpgkit $RPGKIT_TAG into $DEST..."

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

git clone --depth 1 --branch "$TUMULT_TAG" https://github.com/KirkDiggler/tumult.git "$TMP/tumult"
git clone --depth 1 --branch "$RPGKIT_TAG" https://github.com/KirkDiggler/rpgkit.git "$TMP/rpgkit"

# Reset the vendored payload but keep the UE module files (Build.cs, umbrella).
rm -rf "$DEST/include" "$DEST/src" "$DEST/rpgkit"
mkdir -p "$DEST"

# Tumult: public headers + the four stateful sources.
cp -r "$TMP/tumult/include" "$DEST/include"
cp -r "$TMP/tumult/src" "$DEST/src"
cp "$TMP/tumult/LICENSE" "$DEST/LICENSE.tumult"

# rpgkit: header-only core — headers only, nothing to link.
mkdir -p "$DEST/rpgkit/core"
cp -r "$TMP/rpgkit/core/include" "$DEST/rpgkit/core/include"
cp "$TMP/rpgkit/LICENSE" "$DEST/LICENSE.rpgkit"

# Record the pins.
printf 'tumult %s\nrpgkit %s\n' "$TUMULT_TAG" "$RPGKIT_TAG" > "$DEST/.pinned-tags"

echo "Done. Vendored Tumult $TUMULT_TAG + rpgkit $RPGKIT_TAG in $DEST."
```

### `Source/ThirdParty/Tumult/Tumult.Build.cs`

Unreal Build Tool automatically compiles every `.cpp` it finds under a module's
directory, so dropping Tumult's four `src/*.cpp` under this module is enough to
build them — no source list needed. The two `PublicIncludePaths` entries expose
Tumult's and rpgkit's headers to this module and to dependents.

```csharp
// Tumult — https://github.com/KirkDiggler/tumult (vendored at v0.1.0)
// rpgkit — https://github.com/KirkDiggler/rpgkit (vendored at v0.3.0, header-only)

using System.IO;
using UnrealBuildTool;

public class Tumult : ModuleRules
{
	public Tumult(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject"
		});

		// Tumult's public headers: enables #include "tumult/..."
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

		// rpgkit core is header-only (an INTERFACE CMake target upstream — there
		// is no rpgkit library to link). Putting its headers on the path lets
		// Tumult's sources resolve #include "rpg/core/...".
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "rpgkit", "core", "include"));

		// Tumult's stateful translation units (Encounter + the three effects)
		// live under src/ and are auto-compiled as part of this module.
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "src"));

		CppStandard = CppStandardVersion.Cpp20;
	}
}
```

### `Source/ThirdParty/Tumult/Tumult.h` (umbrella header)

```cpp
// Tumult module public header — pulls in the public API in one include.
#pragma once

#include "tumult/character.hpp"
#include "tumult/breakdown.hpp"
#include "tumult/encounter.hpp"
#include "tumult/effects/vulnerable.hpp"
#include "tumult/effects/tough_skin.hpp"
#include "tumult/effects/bleed.hpp"
```

### `Source/TumultUE/TumultUE.Build.cs`

The host module depends on `Tumult`. Because `Tumult` exposes both Tumult's and
rpgkit's headers via `PublicIncludePaths`, the host module needs no rpgkit path
of its own.

```csharp
// TumultUE host module — depends on the vendored Tumult module.
using UnrealBuildTool;

public class TumultUE : ModuleRules
{
	public TumultUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Tumult"
		});

		CppStandard = CppStandardVersion.Cpp20;
	}
}
```

### First call — `ATumultGameMode` (proofs tumult-ue#4)

`tumult::Encounter` is a plain C++ type, not a `UObject`, so the host owns it as
a member and reaches it from C++. `GetEncounter()` is a C++ seam, not a
`UFUNCTION` — Blueprint-facing getters (HP, breakdown) translate at the boundary
in a later slice.

```cpp
// ATumultGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Tumult.h"  // the vendored Tumult umbrella header

#include "ATumultGameMode.generated.h"

UCLASS()
class TUMULTUE_API ATumultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// C++ seam: the authoritative tumult::Encounter for this match.
	tumult::Encounter& GetEncounter() { return Encounter_; }

private:
	tumult::Encounter Encounter_;
};
```

```cpp
// ATumultGameMode.cpp
#include "ATumultGameMode.h"

void ATumultGameMode::BeginPlay()
{
	Super::BeginPlay();

	const tumult::Character Hero{
		.id = "hero", .name = "Hero", .curHp = 30, .maxHp = 30, .block = 0};
	const tumult::Character Goblin{
		.id = "goblin", .name = "Goblin", .curHp = 20, .maxHp = 20, .block = 0};
	Encounter_.setup(Hero, Goblin);

	const tumult::StrikeResult Result = Encounter_.strike("hero", "goblin", 5);

	UE_LOG(LogTemp, Display,
		TEXT("Tumult: hero strikes goblin for %d (goblin HP now %d)"),
		Result.finalDamage, Result.hpAfter);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("Tumult strike: %d dmg, goblin HP %d"),
				Result.finalDamage, Result.hpAfter));
	}
}
```

Run the game; the log/on-screen line reads `Tumult: hero strikes goblin for 5
(goblin HP now 15)`. That is the same compile path and the same result the
`host_consumer` ctest verifies on every CI run — the recipe is proven, not just
written.
````

- [ ] **Step 3: Verify the recipe blocks landed**

```bash
grep -c "Tumult.Build.cs" docs/host-integration.md   # >= 1
grep -c "sync-tumult.sh"  docs/host-integration.md   # >= 1
grep -c "GetEncounter"    docs/host-integration.md   # >= 1
grep -c "host_consumer"   docs/host-integration.md   # >= 1
```

Expected: each prints a count `>= 1`.

- [ ] **Step 4: Re-run the consumer ctest to confirm doc + proof still agree**

```bash
ctest --preset debug -R host_consumer --output-on-failure
```

Expected: `100% tests passed, 0 tests failed out of 1`. (The doc's compile path and the proven compile path are the same recipe.)

- [ ] **Step 5: Commit**

```bash
git add docs/host-integration.md
git commit -m "docs: copy-paste TumultUE consumption recipe in host-integration (#5)"
```

(The issue is tumult#5.)

---

## Task 4: Note A1 in `docs/status.md`

**Files:**
- Modify: `docs/status.md`

- [ ] **Step 1: Update the header date + current health**

In `docs/status.md`, change the date line:

```
_Last updated: 2026-06-23 (Slice 1 — ready for review)_
```

to:

```
_Last updated: 2026-06-27 (Wave A1 — consumption recipe + CI consumer)_
```

Then, immediately under the `## Current health` paragraph that ends with `UE activation is Slice 2 (Kirk's Windows step).`, add:

```
Wave A1 (Group A) landed the consumption contract as a copy-paste recipe in
`docs/host-integration.md` (vendor Tumult `v0.1.0` + rpgkit `v0.3.0` as a UE 5.8
module; first `Encounter` call from `BeginPlay`) and proved it with a CI sample
consumer at `examples/host-consumer/` (the `host_consumer` ctest), which compiles
tumult the documented way and runs in the existing CI jobs. Proofs tumult-ue#4.
```

- [ ] **Step 2: Add A1 to active work**

In the `## Active work` list, add a bullet:

```
- Wave A1 in review
  - docs/host-integration.md copy-paste recipe; examples/host-consumer recipe
    proof wired into ctest (TUMULT_BUILD_EXAMPLES); no ci.yml change.
```

- [ ] **Step 3: Commit**

```bash
git add docs/status.md
git commit -m "docs: note Wave A1 (recipe + CI consumer) in status (#5)"
```

(The issue is tumult#5.)

---

## Task 5: Pre-commit gate, push, open the PR

**Files:** none modified — verification + PR.

- [ ] **Step 1: Full pre-commit gate (never `--no-verify`)**

```bash
make pre-commit
```

Expected: `make pre-commit` = `lint` (clang-format `--dry-run --Werror` over `include src tests`, then `run-clang-tidy`) + `test` (full ctest). All green. Note: `examples/` is intentionally outside `FMT_FILES`, so the example is gated by build+run, not by fmt/tidy.

- [ ] **Step 2: Push the branch**

```bash
git push -u origin feat/a1-consumption-recipe
```

- [ ] **Step 3: Open the PR (one PR for the wave; cite the demand; link the issue)**

```bash
gh pr create --repo KirkDiggler/tumult \
  --title "A1: consumption recipe + CI-verified sample consumer" \
  --body "$(cat <<'EOF'
Wave A1 (Group A — Easy to wrap). Completes the consumption contract started in #3 / PR #4.

What changed:
- docs/host-integration.md: copy-paste TumultUE (UE 5.8) recipe — sync-tumult.sh (pins tumult v0.1.0 + rpgkit v0.3.0), Tumult.Build.cs, Tumult.h umbrella, TumultUE.Build.cs, and the ATumultGameMode BeginPlay first call. Translates the header-only-rpgkit fact into the UE recipe (rpgkit core/include on the include path; nothing to link).
- examples/host-consumer/: a CI sample consumer that compiles tumult the documented way (four stateful sources + include/ + rpg::core header-only target + cxx_std_20), constructs a tumult::Encounter, calls strike, and returns nonzero on an unexpected result. Wired via a new TUMULT_BUILD_EXAMPLES option (default on with tests) + add_subdirectory + add_test, so the EXISTING ctest CI jobs run it — no ci.yml change.
- docs/status.md: notes A1.

Boundary: nothing added to include/tumult/; UE types live only in docs; the example uses only tumult:: + rpg::core::.

Decisions: DR-008 (vendor source + compile as a host module; rpg::core header-only so nothing linked) and DR-009 (recipe verified by a CI sample consumer, not just prose) in rpg-project/ideas/tumult/decisions.md.

Proofs tumult-ue#4.

Closes #5
EOF
)"
```

(The issue is tumult#5.)

- [ ] **Step 4: After CI + Copilot**

Wait for CI (build-linux clang/g++, build-windows MSVC, lint) to go green and for the Copilot review. Reply on each Copilot thread with validity + action (or threaded why-not). Do **not** merge — the director merges after review.

---

## Self-Review (writing-plans skill)

**1. Spec coverage**
- Enrich `docs/host-integration.md` with the literal recipe (Build.cs, sync-tumult.sh pinning both, umbrella header, `GetEncounter()`/`BeginPlay()`) → Task 3. ✓
- Translate header-only-rpgkit into the UE recipe (rpgkit `core/include` on the path; nothing to link) → Task 3 recipe prose + `Tumult.Build.cs` comment + `sync-tumult.sh` comment. ✓
- CI-verified sample consumer at `examples/host-consumer/` with `CMakeLists.txt` + `main.cpp`, compiling the four sources + `include/` + `rpg::core` + `cxx_std_20`, constructing `Encounter`, calling `strike`, printing + returning nonzero on mismatch → Task 2. ✓
- `TUMULT_BUILD_EXAMPLES` option (default on with tests) + `add_subdirectory` + `add_test`; no `ci.yml` change (stated) → Task 2 Step 3 + File Structure table. ✓
- `docs/status.md` updated in the same PR → Task 4. ✓
- Sequenced after PR #4; branch from PR-#4-inclusive main → Sequencing section + Task 1 Step 2 (stop-if-absent). ✓
- Per-wave checklist (boundary; observable done-when) → Per-wave checklist section. ✓
- Two DRs (DR-008, DR-009) → appended to `decisions.md` (this wave), referenced in PR body. ✓

**2. Placeholder scan:** No `TBD`/`TODO`/"add error handling"/"similar to". The only token left to fill is the issue number `#N`, created in Task 1 Step 1 — a normal issue-first reference, with explicit "replace `#N`" instructions at each use. No code/content placeholders.

**3. Type consistency:** `Character` fields (`id/name/curHp/maxHp/block`), `Encounter::setup`/`strike`/`findCharacter`, `StrikeResult` fields (`finalDamage/blocked/hpAfter/breakdown`), `BreakdownStep`, and `formatStep` match the real headers verbatim. CMake names consistent across tasks: option `TUMULT_BUILD_EXAMPLES`, target `host_consumer`, test name `host_consumer`, alias `rpg::core`. Recipe identifiers consistent: module `Tumult`, host module `TumultUE`, `Tumult.Build.cs`, `Tumult.h`, `ATumultGameMode`, `sync-tumult.sh`, pins `v0.1.0`/`v0.3.0`.

Result: spec fully covered; no placeholders; types consistent.
