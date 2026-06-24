# tumult Slice 2 — UE adopts the engine

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. **CRITICAL:** Slice 2 touches UE code (UCLASS/UFUNCTION/.generated.h) that cannot compile on Linux. Each task is marked `[LINUX]` (write + cannot-verify) or `[WINDOWS]` (verify + finalize). Do NOT claim DONE on a `[WINDOWS]` task without Kirk's UE build verification.

**Goal:** Migrate rpgkit-ue's combat runtime from inline hand-rolled logic to host a `tumult::Encounter` (vendored as a UE module), delete `URPGKitBus`, wrap the three UE effects around tumult's effects, and make the HUD breakdown read `source` from the chain receipt.

**Architecture:** tumult v0.1.0 vendored as a UE module (`ThirdParty/tumult/`) alongside the existing `ThirdParty/rpgkit/`. `ARPGKitGameMode` becomes a thin UE-to-tumult translator: `Encounter` owns bus + characters + effects; GameMode translates `StrikeResult`→`FRPGKitChainResult`, `BreakdownStep`→`FRPGKitChainStep`, `EffectReceipt`→BP hook surfacing. `URPGKitBus` deleted (session-wide subsystem → per-encounter bus scope mismatch killed).

**Tech Stack:** Unreal Engine 5.8 (upgrading from 5.7 — see Task 0), C++ modules via Build.cs, tumult v0.1.0 (plain C++20), rpgkit core (vendored header-only).

## Global Constraints

- **tumult dependency:** Vendored at `ThirdParty/tumult/` pinned to tag **v0.1.0** via `scripts/sync-tumult.sh`. Never edit tumult source in rpgkit-ue — a gap is a filed finding against tumult, not a local fix.
- **rpgkit dependency:** Still vendored at `ThirdParty/rpgkit/`; bump to **v0.3.0** as part of this slice (Task 1, Step 4). The effect receipts (`ActionReceipt`/`EffectReceipt`/`Chain::Step::source`) are the v0.3.0 API tumult v0.1.0 already consumes.
- **No host types in tumult:** tumult headers stay plain C++; UE types (`FString`, `UCLASS`, `USTRUCT`) live in rpgkit-ue only. Translation at the boundary (GameMode).
- **CE 5.8 upgrade:** `RPGKitUE.uproject` `EngineAssociation` bumps from `"5.7"` to `"5.8"` (Task 0). Validate via UE 5.8 build on Windows.
- **Blueprint compat:** Existing Blueprint assets (`BP_RPGKitGameMode`, `WBP_CombatHUD_Simple`, card DataAssets) must keep working. `BlueprintCallable` API surface on `ARPGKitGameMode` stays equivalent. NEW methods can be added; EXISTING method signatures must not change.
- **One PR per slice:** Slice 2 opens one PR. PR #4 (Extract encounter runtime) closes WITHOUT merging (D2.2).
- **Git:** Issue-first; branch `feat/slice-2-ue-adopts-tumult` from fresh main; merge, never rebase; never merge without Kirk's explicit approval.
- **Verification split:** `[LINUX]` tasks = write code, cannot compile UE. `[WINDOWS]` tasks = Kirk's UE build + playtest. Mark the split in every task header.

---

## Task 0: UE 5.8 upgrade + PR #4 closure [LINUX]

**Files:**
- Modify: `RPGKitUE.uproject` (EngineAssociation "5.7" → "5.8")
- Modify: `STATUS.md` (add PR #4 closure note + Slice 2 plan pointer)
- Close: GitHub PR #4 (Extract encounter runtime)

**Interfaces:**
- Consumes: nothing.
- Produces: uproject at 5.8; PR #4 closed; STATUS.md documents the decision.

- [ ] **Step 1: Bump EngineAssociation**

Read `RPGKitUE.uproject`. Change `"EngineAssociation": "5.7"` to `"EngineAssociation": "5.8"`. That's the only change.

- [ ] **Step 2: Close PR #4 with a salvage note**

```bash
gh pr close 4 --repo KirkDiggler/rpgkit-ue --comment "Closing without merging per D2.2 (tumult design doc). Slice 2 supersedes this — it wraps a \`tumult::Encounter\` from the start. Salvaged into Slice 2: the Blueprint compat wrappers shape, the request-topic plumbing pattern, the STATUS.md prose. See rpg-project/ideas/tumult/design.md §Slice 2 and rpg-project/ideas/tumult/plan-slice-2.md."
```

- [ ] **Step 3: Update STATUS.md**

Add a section at the top:

```markdown
## Current Direction

**Slice 2: UE adopts tumult.** Migrating the combat runtime from inline
hand-rolled logic to host a `tumult::Encounter` (v0.1.0) vendored as a UE
module. PR #4 (Extract encounter runtime) is closed — Slice 2 supersedes it
(see rpg-project/ideas/tumult/design.md §Slice 2 for D2.1–D2.4).

UE 5.8 upgrade (`EngineAssociation`: 5.7 → 5.8) is part of this slice.
```

- [ ] **Step 4: Commit + push**

```bash
git add RPGKitUE.uproject STATUS.md
git commit -m "chore: bump to UE 5.8; close PR #4 (superseded by Slice 2)"
git push origin feat/slice-2-ue-adopts-tumult
```

---

## Task 1: Vendor tumult v0.1.0 + bump rpgkit to v0.3.0 [LINUX]

**Files:**
- Create: `scripts/sync-tumult.sh`
- Create: `ThirdParty/tumult/tumult.Build.cs`
- Create: `ThirdParty/tumult/tumult.h` (module-public include header)
- Modify: `Source/RPGKitUE/RPGKitUE.Build.cs` (add `"tumult"` to PrivateDependencyModuleNames)
- Modify: `ThirdParty/rpgkit/core/include/rpg/core/version.hpp` (bump to v0.3.0) — OR re-run sync for rpgkit if a sync script exists; if not, manually update vendored headers from the v0.3.0 tag.

**Interfaces:**
- Consumes: tumult v0.1.0 (GitHub tag), rpgkit v0.3.0 (GitHub tag).
- Produces: `tumult` UE module (Build.cs + vendored source); `RPGKitUE` depends on `tumult`.

- [ ] **Step 1: Write `scripts/sync-tumult.sh`**

```bash
#!/usr/bin/env bash
# Sync vendored tumult to a pinned release tag.
# Usage: ./scripts/sync-tumult.sh v0.1.0
set -euo pipefail
TAG="${1:?usage: sync-tumult.sh <tag>}"
DEST="ThirdParty/tumult"

echo "Syncing tumult $TAG into $DEST..."
rm -rf "$DEST"
mkdir -p "$DEST"

TMP=$(mktemp -d)
git clone --depth 1 --branch "$TAG" https://github.com/KirkDiggler/tumult.git "$TMP/tumult"

cp -r "$TMP/tumult/include" "$DEST/include"
cp -r "$TMP/tumult/src" "$DEST/src"
cp "$TMP/tumult/LICENSE" "$DEST/LICENSE"

# Record the pinned tag
echo "$TAG" > "$DEST/.pinned-tag"

rm -rf "$TMP"
echo "Done. tumult $TAG vendored in $DEST."
```

- [ ] **Step 2: Run sync-tumult.sh + bump rpgkit**

```bash
chmod +x scripts/sync-tumult.sh
./scripts/sync-tumult.sh v0.1.0
```

For rpgkit: check if `ThirdParty/rpgkit/` has a sync script. If not, manually update the vendored rpgkit headers to v0.3.0:
```bash
TMP=$(mktemp -d)
git clone --depth 1 --branch v0.3.0 https://github.com/KirkDiggler/rpgkit.git "$TMP/rpgkit"
cp -r "$TMP/rpgkit/core/include" ThirdParty/rpgkit/core/include
echo "v0.3.0" > ThirdParty/rpgkit/.pinned-tag
rm -rf "$TMP"
```

Verify: `cat ThirdParty/tumult/.pinned-tag` → `v0.1.0`; `cat ThirdParty/rpgkit/.pinned-tag` → `v0.3.0`; `grep kVersion ThirdParty/rpgkit/core/include/rpg/core/version.hpp` → `0.3.0`.

- [ ] **Step 3: Write `ThirdParty/tumult/tumult.Build.cs`**

```csharp
// tumult — https://github.com/KirkDiggler/tumult (vendored at v0.1.0)

using UnrealBuildTool;

public class tumult : ModuleRules
{
	public tumult(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject"
		});

		// tumult's public headers
		string TumultInclude = Path.Combine(ModuleDirectory, "include");
		PublicIncludePaths.Add(TumultInclude);

		// tumult's compiled TUs (Encounter, effects) built as part of this module
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "src"));

		CppStandard = CppStandardVersion.Cpp20;
	}
}
```

- [ ] **Step 4: Write `ThirdParty/tumult/tumult.h` (module header)**

```cpp
// tumult module public header — pulls in the public API.
#pragma once

#include "tumult/character.hpp"
#include "tumult/breakdown.hpp"
#include "tumult/encounter.hpp"
#include "tumult/effects/vulnerable.hpp"
#include "tumult/effects/tough_skin.hpp"
#include "tumult/effects/bleed.hpp"
```

- [ ] **Step 5: Update `Source/RPGKitUE/RPGKitUE.Build.cs`**

```csharp
// rpgkit — https://github.com/KirkDiggler/rpgkit

using UnrealBuildTool;
using System.IO;

public class RPGKitUE : ModuleRules
{
	public RPGKitUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"tumult"
		});

		// rpgkit header-only library (vendored in ThirdParty/)
		// rpgkit is transitively available via the tumult module, but
		// RPGKitUE also includes rpgkit headers directly (effects).
		string RPGKitPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "rpgkit", "core", "include");
		PublicIncludePaths.Add(RPGKitPath);

		CppStandard = CppStandardVersion.Cpp20;
	}
}
```

- [ ] **Step 6: Create a `.gitignore` entry for ThirdParty (optional — keep vendored OR add to .gitignore)**

Decision: **Keep vendored in git** (matches the existing `ThirdParty/rpgkit/` pattern which is committed). Do NOT gitignore. The sync script is for updating the pin.

- [ ] **Step 7: Commit**

```bash
git add scripts/sync-tumult.sh ThirdParty/tumult/ ThirdParty/rpgkit/ Source/RPGKitUE/RPGKitUE.Build.cs
git commit -m "feat: vendor tumult v0.1.0 as UE module; bump rpgkit to v0.3.0"
```

**Verification:** `[WINDOWS]` — UE 5.8 generates project files successfully and the tumult + RPGKitUE modules are discoverable. Linux cannot verify the UE module system.

---

## Task 2: `ARPGKitGameMode::GetEncounter()` — expose the tumult Encounter [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitGameMode.h` (add `tumult::Encounter` member + `GetEncounter()` accessor)
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp` (replace `BusSubsystem` ownership with `tumult::Encounter` in `BeginPlay`/setup)

**Interfaces:**
- Consumes: `tumult::Encounter` (from `tumult/encounter.hpp`).
- Produces: `ARPGKitGameMode::GetEncounter()` → `tumult::Encounter&`. Used by Tasks 3-7.

- [ ] **Step 1: Add tumult include + Encounter member to GameMode.h**

In `RPGKitGameMode.h`, add below the existing includes:

```cpp
#include "tumult/encounter.hpp"
```

In the private section of `ARPGKitGameMode`, replace the `TObjectPtr<URPGKitBus> BusSubsystem` field with:

```cpp
	tumult::Encounter Encounter_;
```

In the public section, add:

```cpp
	UFUNCTION(BlueprintCallable, Category = "RPGKit")
	tumult::Encounter& GetEncounter() { return Encounter_; }
```

(Note: `tumult::Encounter` is not a `UCLASS`, but UE allows returning non-UObject references from `BlueprintCallable` functions via C++ call sites; Blueprint can't call this directly but C++ can. If UFUNCTION rejects a non-UObject return, make it non-UFUNCTION — it's a C++ seam, not a Blueprint seam.)

- [ ] **Step 2: Update GameMode.cpp setup to use Encounter**

In the existing `BeginPlay` or setup function (where `BusSubsystem` was fetched via `GetSubsystem<URPGKitBus>()`), replace the bus acquisition:

```cpp
	// Old:
	// if (UGameInstance* GI = GetGameInstance()) {
	//   BusSubsystem = GI->GetSubsystem<URPGKitBus>();
	// }
	// if (!BusSubsystem) { ... error ... }
	// SubscribeEncounterRequests();  // subscribes raw_damage + block on BusSubsystem

	// New:
	FRPGKitFighter HeroFighter = GetFighter("hero");
	FRPGKitFighter GoblinFighter = GetFighter("goblin");
	tumult::Character HeroChar{.id = "hero", .name = TCHAR_TO_UTF8(*HeroFighter.Name),
	                           .curHp = HeroFighter.CurrentHP, .maxHp = HeroFighter.MaxHP, .block = HeroFighter.Block};
	tumult::Character GoblinChar{.id = "goblin", .name = TCHAR_TO_UTF8(*GoblinFighter.Name),
	                             .curHp = GoblinFighter.CurrentHP, .maxHp = GoblinFighter.MaxHP, .block = GoblinFighter.Block};
	Encounter_.setup(HeroChar, GoblinChar);
```

- [ ] **Step 3: Update fighter state sync (read-back from Encounter → FRPGKitFighter)**

Add a private helper:

```cpp
void ARPGKitGameMode::SyncFighterState()
{
	if (auto* hero = Encounter_.findCharacter("hero")) {
		FRPGKitFighter& F = Fighters["hero"];
		F.CurrentHP = hero->curHp;
		F.Block = hero->block;
	}
	if (auto* goblin = Encounter_.findCharacter("goblin")) {
		FRPGKitFighter& F = Fighters["goblin"];
		F.CurrentHP = goblin->curHp;
		F.Block = goblin->block;
	}
}
```

Call `SyncFighterState()` after every `Encounter_` mutation (`strike`, `dealRawDamage`, `addBlock`, `endTurn`).

- [ ] **Step 4: Commit**

```bash
git add Source/RPGKitUE/RPGKitGameMode.h Source/RPGKitUE/RPGKitGameMode.cpp
git commit -m "feat: ARPGKitGameMode owns a tumult::Encounter; GetEncounter() accessor"
```

**Verification:** `[WINDOWS]` — UE build. Linux cannot compile UCLASS headers.

---

## Task 3: Migrate `strike` path from `URPGKitBus::ExecuteDamageChain` to `Encounter::strike` [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp` — the `ExecuteCardAction` / `Strike` path

**Interfaces:**
- Consumes: `Encounter::strike(attacker, target, base)` → `StrikeResult`.
- Produces: `FRPGKitChainResult` populated from `StrikeResult::breakdown` + `StrikeResult::finalDamage`.

- [ ] **Step 1: Translate StrikeResult → FRPGKitChainResult**

In GameMode.cpp, replace the body that called `BusSubsystem->ExecuteDamageChain(Event)` with:

```cpp
	// Old:
	// FRPGKitChainResult Result = BusSubsystem->ExecuteDamageChain(Event);
	// LatestDamageBreakdown = Result.Breakdown;
	// int32 FinalDamage = Result.Value;
	// ... block absorption, HP mutation, EmitCombatLog ...

	// New:
	tumult::StrikeResult Sr = Encounter_.strike(
		TCHAR_TO_UTF8(*Event.AttackerId),
		TCHAR_TO_UTF8(*Event.TargetId),
		Event.BaseAmount);

	// Translate BreakdownStep → FRPGKitChainStep
	LatestDamageBreakdown.Empty();
	for (const auto& step : Sr.breakdown) {
		FRPGKitChainStep S;
		S.Stage = UTF8_TO_TCHAR(step.stage.c_str());
		S.ModifierId = UTF8_TO_TCHAR(step.id.c_str());
		S.Before = step.before;
		S.After = step.after;
		LatestDamageBreakdown.Add(S);
	}

	FRPGKitChainResult Result;
	Result.Value = Sr.finalDamage;
	Result.Breakdown = LatestDamageBreakdown;

	// Block absorption is already done by tumult::Encounter::strike.
	// Sync fighter state back:
	SyncFighterState();

	// Emit combat log — now from receipt data, not paren-strings:
	if (Host)
	{
		Host->OnDamageDealt(Result);
	}
```

(Detail: the old code did block absorption inline (`Blocked = Min(Target->Block, FinalDamage)` etc). tumult::Encounter::strike already does block absorption and reports `result.blocked` + `result.hpAfter`. The sync back to `FRPGKitFighter` covers this. The `EmitCombatLog` paren-strings on the strike path retire here.)

- [ ] **Step 2: Commit**

```bash
git add Source/RPGKitUE/RPGKitGameMode.cpp
git commit -m "feat: strike path delegates to Encounter::strike; breakdown from receipt"
```

**Verification:** `[WINDOWS]` — UE build + playtest: strike a damage card, check HUD damage breakdown shows base + effects steps with source labels from the chain receipt.

---

## Task 4: Migrate `applyEffect`/`removeEffect` to `Encounter::applyEffect`/`removeEffect` [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp` — the `ApplyEffect`/`RemoveEffect` Blueprint-callable methods

- [ ] **Step 1: Replace URPGKitBus::ApplyEffect with Encounter::applyEffect**

```cpp
bool ARPGKitGameMode::ApplyEffect(URPGKitEffect* Effect)
{
	// Old: return BusSubsystem->ApplyEffect(Effect);
	// New:
	if (!Effect) return false;
	auto [status, receipt] = Encounter_.applyEffect(Effect->GetRawEffect());
	if (!status.isOk())
	{
		UE_LOG(LogTemp, Warning, TEXT("RPGKit: apply effect failed: %s"),
			UTF8_TO_TCHAR(status.message().c_str()));
		return false;
	}
	Effect->OnEffectApplied();
	if (Host) Host->OnEffectApplied(Effect->GetName());
	return true;
}
```

Similarly for `RemoveEffect`:

```cpp
bool ARPGKitGameMode::RemoveEffect(URPGKitEffect* Effect)
{
	if (!Effect) return false;
	auto [status, receipt] = Encounter_.removeEffect(Effect->GetRawEffect());
	if (!status.isOk())
	{
		UE_LOG(LogTemp, Warning, TEXT("RPGKit: remove effect failed: %s"),
			UTF8_TO_TCHAR(status.message().c_str()));
		return false;
	}
	Effect->OnEffectRemoved();
	if (Host) Host->OnEffectRemoved(Effect->GetName());
	return true;
}
```

(Note: `URPGKitEffect::GetRawEffect()` returns `rpg::core::Effect&` — but tumult's effects subclass `rpg::core::Effect` directly. Task 6 resolves this: `URPGKitEffect` will own a `tumult::VulnerableEffect` (etc) and `GetRawEffect()` returns that. For now, the `rpg::core::Effect&` cast works because tumult effects ARE rpg::core::Effect subclasses.)

- [ ] **Step 2: Commit**

```bash
git add Source/RPGKitUE/RPGKitGameMode.cpp
git commit -m "feat: applyEffect/removeEffect delegate to Encounter; EffectReceipt consumed"
```

**Verification:** `[WINDOWS]` — UE build + playtest: apply Vulnerable/Bleed from cards, verify HUD shows "applied" + the effects modify damage/tick.

---

## Task 5: Migrate `endTurn` + `addBlock` + `dealRawDamage` to Encounter verbs [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp`

- [ ] **Step 1: Replace EndTurn body**

```cpp
	// Old:
	// if (BusSubsystem) { RPGKitTopics::kTurnEnded.on(BusSubsystem->GetRawBus()).publish(...); }
	// New:
	Encounter_.endTurn();
	SyncFighterState();
```

- [ ] **Step 2: Replace AddBlock body**

```cpp
	// Old: if (FRPGKitFighter* F = FindFighter(Id)) F->Block += Amount;
	// New: Encounter_.addBlock(TCHAR_TO_UTF8(*Id), Amount);
	//      SyncFighterState();
```

- [ ] **Step 3: Replace DealRawDamage body**

```cpp
	// Old: if (FRPGKitFighter* F = FindFighter(Id)) F->CurrentHP = Max(0, F->CurrentHP - Amount);
	// New: Encounter_.dealRawDamage(TCHAR_TO_UTF8(*Id), Amount);
	//      SyncFighterState();
```

- [ ] **Step 4: Commit**

```bash
git add Source/RPGKitUE/RPGKitGameMode.cpp
git commit -m "feat: endTurn/addBlock/dealRawDamage delegate to Encounter; URPGKitBus no longer needed"
```

**Verification:** `[WINDOWS]` — UE build + playtest: end turn, verify Bleed ticks + block expiry + enemy attack work.

---

## Task 6: Migrate UE effects to wrap tumult effects [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitEffect.h` — change inner effect from hand-rolled `rpg::core::Effect*` to the tumult subclass
- Modify: `Source/RPGKitUE/RPGKitEffect.cpp` — replace inner effect implementations with tumult constructors

- [ ] **Step 1: VulnerableEffect wraps tumult::VulnerableEffect**

In `RPGKitEffect.h`, the `URPGKitVulnerableEffect` class's private `FVulnerableEffect* InnerEffectImpl` becomes:

```cpp
	tumult::VulnerableEffect* TumultVuln_ = nullptr;
```

In `RPGKitEffect.cpp`, the constructor:

```cpp
URPGKitVulnerableEffect::URPGKitVulnerableEffect()
{
	TumultVuln_ = new tumult::VulnerableEffect(
		TCHAR_TO_UTF8(*TargetEntityId),
		PercentBonus,
		RemainingTurns);
	RawEffectPtr = TumultVuln_;
}
```

(But TargetEntityId / PercentBonus / RemainingTurns are UPROPERTY fields set after construction — `PostInitProperties` or a `BeginPlay` hook re-creates the tumult effect with the authored values. For now, if the defaults match (goblin, 50, 2), the constructor works for Slice 2; Slice 5+ handles authored-data re-creation.)

- [ ] **Step 2: ToughSkinEffect wraps tumult::ToughSkinEffect**

```cpp
	tumult::ToughSkinEffect* TumultToughSkin_ = nullptr;
```

```cpp
URPGKitToughSkinEffect::URPGKitToughSkinEffect()
{
	TumultToughSkin_ = new tumult::ToughSkinEffect(
		TCHAR_TO_UTF8(*ProtectedEntityId),
		DamageReduction);
	RawEffectPtr = TumultToughSkin_;
}
```

- [ ] **Step 3: BleedEffect wraps tumult::BleedEffect**

```cpp
	tumult::BleedEffect* TumultBleed_ = nullptr;
```

```cpp
URPGKitBleedEffect::URPGKitBleedEffect()
{
	TumultBleed_ = new tumult::BleedEffect(
		TCHAR_TO_UTF8(*TargetEntityId),
		DamagePerStack,
		Stacks);
	RawEffectPtr = TumultBleed_;
}
```

- [ ] **Step 4: Delete the hand-rolled inner effect classes**

Delete `FToughSkinEffect`, `FBleedEffect`, `FVulnerableEffect` private classes and their `onApply` implementations from `RPGKitEffect.cpp`. The tumult effects own that logic.

- [ ] **Step 5: Commit**

```bash
git add Source/RPGKitUE/RPGKitEffect.h Source/RPGKitUE/RPGKitEffect.cpp
git commit -m "feat: UE effects wrap tumult:: effects; delete hand-rolled inner classes"
```

**Verification:** `[WINDOWS]` — UE build + playtest: apply Vulnerable from a card → strike goblin → damage breakdown shows `(vulnerable)` source from the tumult Receipt. Apply Bleed → end turn → ticks.

---

## Task 7: Move `PublishEvent` to GameMode as BlueprintCallable forwarder [LINUX]

**Files:**
- Modify: `Source/RPGKitUE/RPGKitGameMode.h` — add `PublishEvent` BlueprintCallable
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp` — implement the forwarder

- [ ] **Step 1: Add PublishEvent to GameMode.h**

```cpp
	UFUNCTION(BlueprintCallable, Category = "RPGKit|Bus")
	void PublishEvent(FName TopicId, const FRPGKitEventPayload& Payload);
```

- [ ] **Step 2: Implement in GameMode.cpp**

```cpp
void ARPGKitGameMode::PublishEvent(FName TopicId, const FRPGKitEventPayload& Payload)
{
	FString TopicStr = TopicId.ToString();
	if (TopicStr == "turn.ended")
	{
		Encounter_.endTurn();
		SyncFighterState();
	}
	// Future topics route through their own Encounter verbs or
	// Encounter_.bus().publish(...) if raw bus access is needed.
}
```

- [ ] **Step 3: Commit**

```bash
git add Source/RPGKitUE/RPGKitGameMode.h Source/RPGKitUE/RPGKitGameMode.cpp
git commit -m "feat: PublishEvent BlueprintCallable forwarder on GameMode"
```

**Verification:** `[WINDOWS]` — UE build + playtest. Check existing Blueprint nodes that called `URPGKitBus::PublishEvent` still work via `GameMode.PublishEvent`.

---

## Task 8: Delete `URPGKitBus` [WINDOWS-GATED]

**Files:**
- Delete: `Source/RPGKitUE/RPGKitBus.h`, `Source/RPGKitUE/RPGKitBus.cpp`
- Modify: `Source/RPGKitUE/RPGKitGameMode.h` — remove `#include "RPGKitBus.h"`, remove forward declaration `class URPGKitBus`, remove `BusSubsystem` field if it lingers
- Modify: `Source/RPGKitUE/RPGKitGameMode.cpp` — remove all `BusSubsystem->` calls (should be zero by now; verify with `grep BusSubsystem`)

- [ ] **Step 1 (WINDOWS): Scan Blueprint assets for URPGKitBus references**

```powershell
# In the rpgkit-ue project folder, search all .uasset files for URPGKitBus references.
# UE 5.8 — use the repopulated project.
find Content -name "*.uasset" -exec strings {} \; | grep -i "RPGKitBus"
```

If ANY references surface: STOP. Do NOT delete `URPGKitBus` in this PR. Instead:
- Keep `RPGKitBus.h`/`.cpp` as a deprecated shim that routes to `Encounter_` (e.g. `URPGKitBus::PublishEvent` calls `GetGameInstance()->GetWorld()->GetAuthGameMode<ARPGKitGameMode>()->PublishEvent(...)`).
- Mark the shim `UE_DEPRECATED(5.8, "Use ARPGKitGameMode::PublishEvent instead")`.
- File a follow-up issue for deletion after Blueprint assets migrate.
- Skip Steps 2-4; commit the shim.

If ZERO references: proceed to Step 2.

- [ ] **Step 2: Delete URPGKitBus files**

```bash
git rm Source/RPGKitUE/RPGKitBus.h Source/RPGKitUE/RPGKitBus.cpp
```

- [ ] **Step 3: Clean GameMode references**

In `RPGKitGameMode.h`: remove `#include "RPGKitBus.h"`, remove `class URPGKitBus;` forward decl, remove any remaining `BusSubsystem` field.

In `RPGKitGameMode.cpp`: verify `grep -n BusSubsystem` returns zero hits. If any remain, route them to `Encounter_`.

- [ ] **Step 4: Windows UE 5.8 build + playtest**

Run the full UE 5.8 build:
```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" RPGKitUEEditor Win64 Development -Project="C:\Users\kirk\source\repos\rpgkit-ue\RPGKitUE.uproject" -WaitMutex -NoHotReload
```

Expected: Succeeded.

Playtest the demo loop: deal hand → play cards → apply Vulnerable → strike → end turn → Bleed ticks → enemy attacks → block absorbs → deal new hand. Verify HUD damage breakdown shows `(vulnerable)` / `(tough-skin)` from the chain receipt, NOT from paren-strings.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: delete URPGKitBus; UE is a pure edge over tumult::Encounter"
```

---

## Task 9: Update docs + open PR [LINUX + WINDOWS]

**Files:**
- Modify: `STATUS.md` — final Slice 2 status
- Modify: `docs/unreal-workshop-roadmap.md` — reflect tumult as the engine layer
- Modify: `AGENTS.md` — workflow rules for a tumult-hosting repo

- [ ] **Step 1: Update STATUS.md**

```markdown
## Current health

Slice 2 complete: UE combat runtime hosts a `tumult::Encounter` (v0.1.0,
vendored). `ARPGKitGameMode` is a thin UE-to-tumult translator. The three
effects (`Vulnerable`/`ToughSkin`/`Bleed`) are thin UObjects wrapping tumult
effects. `URPGKitBus` deleted (per-encounter bus > session-wide subsystem).
HUD damage breakdown reads `source` from the chain receipt.

UE 5.8.
```

- [ ] **Step 2: Update AGENTS.md**

Add to "Priorities":

```markdown
6. tumult owns the game; rpgkit-ue is a UE edge. No combat logic in
   GameMode that belongs in tumult. Translation at the boundary only.
```

- [ ] **Step 3: Push + open PR**

```bash
git push origin feat/slice-2-ue-adopts-tumult
gh pr create --title "feat: Slice 2 — UE adopts tumult (encounter, effects, bus deletion)" --body "..."
```

---

## Verification summary

| Task | Linux-write | Windows-verify | Acceptance |
|------|-------------|----------------|------------|
| 0 | uproject + STATUS + PR #4 close | — | PR #4 closed |
| 1 | sync script + Build.cs + vendor | UE project gen | tumult module discoverable |
| 2 | GameMode.h/.cpp Encounter member | UE build | GameMode compiles |
| 3 | strike delegation | UE build + playtest | HUD breakdown from receipt |
| 4 | applyEffect/removeEffect delegation | UE build + playtest | Vulnerable/Bleed via receipts |
| 5 | endTurn/addBlock/dealRawDamage | UE build + playtest | Bleed ticks, block expires |
| 6 | UE effects wrap tumult effects | UE build + playtest | `(vulnerable)` on HUD |
| 7 | PublishEvent on GameMode | UE build + playtest | Blueprint publish still works |
| 8 | — | BP scan + delete + build + playtest | URPGKitBus gone, demo plays |
| 9 | docs + PR | — | PR open |

**Slice 2 acceptance:** the visual demo loop in rpgkit-ue still plays end-to-end on UE 5.8 after the migration, and the damage breakdown on the HUD reads `source` from the chain receipt (visible by matching paren-source labels from `Integration.HeroStrikeBreakdownNamesEveryModifierFromReceipts`), with no `EmitCombatLog` paren-strings remaining on the strikes path.