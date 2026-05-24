# Equipment Data Enrichment — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Flow weapon/armor/gear stats from toolkit through protos and API so the UI can display meaningful equipment details during character creation.

**Architecture:** Four-repo pipeline following the boundary rule. Toolkit enriches equipment choices at the source by resolving IDs to full stat structs. Protos carry the data via an existing `Equipment` message added to `EquipmentItem`. API maps toolkit types to proto mechanically. UI renders with a reusable `EquipmentCard` component.

**Tech Stack:** Go (toolkit, API), Protocol Buffers + buf (protos), React + TypeScript + Tailwind (UI)

**Design doc:** `rpg-project/ideas/equipment-enrichment/design.md`

---

## File Structure

### rpg-api-protos (Task 1)
| Action | File | Responsibility |
|--------|------|----------------|
| Modify | `dnd5e/api/v1alpha1/choices.proto:103-115` | Add `equipment_detail` field to `EquipmentItem` |

### rpg-toolkit (Tasks 2-4)
| Action | File | Responsibility |
|--------|------|----------------|
| Create | `rulebooks/dnd5e/equipment/detail.go` | `EquipmentDetail`, `WeaponDetail`, `ArmorDetail` types |
| Create | `rulebooks/dnd5e/equipment/detail_test.go` | Tests for detail types and resolver |
| Modify | `rulebooks/dnd5e/equipment/equipment.go` | Add `ResolveEquipmentDetail` function |
| Modify | `rulebooks/dnd5e/character/choices/requirements.go:74-78` | Add `Detail` field to `EquipmentItem` |
| Modify | `rulebooks/dnd5e/character/choices/requirements.go` | Populate detail in class requirement builders |

### rpg-api (Tasks 5-6)
| Action | File | Responsibility |
|--------|------|----------------|
| Modify | `internal/handlers/dnd5e/v1alpha1/character/converters.go:2305-2460` | Map `Detail` to proto `equipment_detail` in `createEquipmentChoice` |
| Create | `internal/handlers/dnd5e/v1alpha1/character/cost_parser.go` | Parse "25 gp" → proto `Cost` message |
| Create | `internal/handlers/dnd5e/v1alpha1/character/cost_parser_test.go` | Tests for cost/weight parsing |
| Modify | `internal/handlers/dnd5e/v1alpha1/character/converters_test.go` | Test equipment detail mapping |

### rpg-dnd5e-web (Tasks 7-8)
| Action | File | Responsibility |
|--------|------|----------------|
| Create | `src/components/equipment/EquipmentCard.tsx` | Reusable card rendering weapon/armor/gear stats |
| Modify | `src/components/choices/EquipmentBundleChoice.tsx` | Wire `EquipmentCard` into bundle items and category dropdowns |

---

## Task 1: Proto — Add `equipment_detail` to `EquipmentItem`

**Repo:** `rpg-api-protos`
**Branch:** `feat/equipment-detail-field`
**Issue:** Create issue on project board before starting.

**Files:**
- Modify: `dnd5e/api/v1alpha1/choices.proto:103-115`

- [ ] **Step 1: Create branch**

```bash
cd /home/kirk/personal/rpg-api-protos
git checkout main && git pull
git checkout -b feat/equipment-detail-field
```

- [ ] **Step 2: Add `equipment_detail` field to `EquipmentItem`**

In `dnd5e/api/v1alpha1/choices.proto`, find the `EquipmentItem` message (line 103). Add `equipment_detail` field after the `type_hint` oneof:

```protobuf
message EquipmentItem {
  string selection_id = 1;
  int32 quantity = 2;

  // Optional type hint for client convenience (can help with icon display, etc.)
  oneof type_hint {
    Weapon weapon = 3;
    Armor armor = 4;
    Tool tool = 5;
    Pack pack = 6;
    Ammunition ammunition = 7;
  }

  // Resolved equipment stats for display (weapon damage, armor AC, etc.)
  // Populated by toolkit during choice building. Optional for backward compatibility.
  Equipment equipment_detail = 8;
}
```

The `Equipment` message is defined in `character.proto` in the same package — no import needed.

- [ ] **Step 3: Verify proto compiles**

```bash
buf lint
buf build
```

Expected: no errors.

- [ ] **Step 4: Commit and push**

Code generation happens in CI after merge — generated code lands on the `@generated` branch. Only commit the `.proto` source file.

```bash
git add dnd5e/api/v1alpha1/choices.proto
git commit -m "feat: add equipment_detail field to EquipmentItem message

Adds an optional Equipment field to EquipmentItem so equipment choices
can carry resolved stats (damage, AC, properties, etc.) for UI display."
```

- [ ] **Step 5: Create PR**

```bash
git push -u origin feat/equipment-detail-field
gh pr create --title "Add equipment_detail to EquipmentItem" --body "..."
```

**Wait for user to merge.** CI builds generated code on `@generated` branch after merge. Downstream repos (API, web) consume from there.

---

## Task 2: Toolkit — Define `EquipmentDetail` types

**Repo:** `rpg-toolkit`
**Branch:** `feat/equipment-detail-types`
**Issue:** Create issue on project board before starting.
**Depends on:** Nothing — toolkit types are independent of protos.

**Files:**
- Create: `rulebooks/dnd5e/equipment/detail.go`
- Create: `rulebooks/dnd5e/equipment/detail_test.go`

**Reference files** (read-only — understand the types these wrap):
- `rulebooks/dnd5e/weapons/types.go` — `Weapon`, `WeaponCategory`, `WeaponProperty`, `Range`
- `rulebooks/dnd5e/armor/armor.go` — `Armor`, `ArmorCategory`
- `rulebooks/dnd5e/damage/damage.go` — `damage.Type`
- `rulebooks/dnd5e/tools/tools.go` — `Tool`
- `rulebooks/dnd5e/packs/packs.go` — `Pack`
- `rulebooks/dnd5e/ammunition/types.go` — `Ammunition`
- `rulebooks/dnd5e/shared/equipment.go` — `EquipmentType`, `EquipmentID`

- [ ] **Step 1: Create branch**

```bash
cd /home/kirk/personal/rpg-toolkit
git checkout main && git pull
git checkout -b feat/equipment-detail-types
```

- [ ] **Step 2: Write the failing tests for detail types and resolver**

Create `rulebooks/dnd5e/equipment/detail_test.go`:

```go
package equipment

import (
	"testing"

	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/armor"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/damage"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/shared"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/weapons"
	"github.com/stretchr/testify/suite"
)

type DetailTestSuite struct {
	suite.Suite
}

func TestDetailSuite(t *testing.T) {
	suite.Run(t, new(DetailTestSuite))
}

func (s *DetailTestSuite) TestResolveWeapon() {
	detail := ResolveEquipmentDetail("longsword")

	s.Require().NotNil(detail)
	s.Assert().Equal("Longsword", detail.Name)
	s.Assert().Equal(shared.EquipmentTypeWeapon, detail.Type)
	s.Assert().Equal(float64(3), detail.Weight)
	s.Assert().Equal("15 gp", detail.Cost)

	s.Require().NotNil(detail.Weapon)
	s.Assert().Equal(weapons.CategoryMartialMelee, detail.Weapon.Category)
	s.Assert().Equal("1d8", detail.Weapon.Damage)
	s.Assert().Equal(damage.Slashing, detail.Weapon.DamageType)
	s.Assert().Contains(detail.Weapon.Properties, weapons.PropertyVersatile)
	s.Assert().Nil(detail.Weapon.Range)

	s.Assert().Nil(detail.Armor)
}

func (s *DetailTestSuite) TestResolveRangedWeapon() {
	detail := ResolveEquipmentDetail("longbow")

	s.Require().NotNil(detail)
	s.Require().NotNil(detail.Weapon)
	s.Assert().Equal(weapons.CategoryMartialRanged, detail.Weapon.Category)
	s.Require().NotNil(detail.Weapon.Range)
	s.Assert().Equal(150, detail.Weapon.Range.Normal)
	s.Assert().Equal(600, detail.Weapon.Range.Long)
}

func (s *DetailTestSuite) TestResolveArmor() {
	detail := ResolveEquipmentDetail("chain-mail")

	s.Require().NotNil(detail)
	s.Assert().Equal("Chain Mail", detail.Name)
	s.Assert().Equal(shared.EquipmentTypeArmor, detail.Type)
	s.Assert().Equal(float64(55), detail.Weight)
	s.Assert().Equal("75 gp", detail.Cost)

	s.Assert().Nil(detail.Weapon)
	s.Require().NotNil(detail.Armor)
	s.Assert().Equal(armor.CategoryHeavy, detail.Armor.Category)
	s.Assert().Equal(16, detail.Armor.BaseAC)
	s.Assert().False(detail.Armor.DexBonus)
	s.Assert().Equal(13, detail.Armor.StrengthRequirement)
	s.Assert().True(detail.Armor.StealthDisadvantage)
}

func (s *DetailTestSuite) TestResolveShield() {
	detail := ResolveEquipmentDetail("shield")

	s.Require().NotNil(detail)
	s.Require().NotNil(detail.Armor)
	s.Assert().Equal(armor.CategoryShield, detail.Armor.Category)
	s.Assert().Equal(2, detail.Armor.BaseAC)
}

func (s *DetailTestSuite) TestResolveTool() {
	detail := ResolveEquipmentDetail("thieves-tools")

	s.Require().NotNil(detail)
	s.Assert().Equal(shared.EquipmentTypeTool, detail.Type)
	s.Assert().Equal("Thieves' Tools", detail.Name)
	s.Assert().Nil(detail.Weapon)
	s.Assert().Nil(detail.Armor)
}

func (s *DetailTestSuite) TestResolvePack() {
	detail := ResolveEquipmentDetail("explorers-pack")

	s.Require().NotNil(detail)
	s.Assert().Equal(shared.EquipmentTypePack, detail.Type)
	s.Assert().Equal("Explorer's Pack", detail.Name)
	s.Assert().Nil(detail.Weapon)
	s.Assert().Nil(detail.Armor)
}

func (s *DetailTestSuite) TestResolveAmmunition() {
	detail := ResolveEquipmentDetail("arrows-20")

	s.Require().NotNil(detail)
	s.Assert().Equal(shared.EquipmentTypeAmmunition, detail.Type)
	s.Assert().Nil(detail.Weapon)
	s.Assert().Nil(detail.Armor)
}

func (s *DetailTestSuite) TestResolveUnknownReturnsNil() {
	detail := ResolveEquipmentDetail("nonexistent-item")

	s.Assert().Nil(detail)
}
```

- [ ] **Step 3: Run tests — verify they fail**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./equipment/ -v -run TestDetailSuite
```

Expected: FAIL — `ResolveEquipmentDetail` undefined, types undefined.

**Note:** Test values (IDs, names, stats) are based on current toolkit registries. Before running, verify IDs exist:
```bash
grep -n "Longsword\|longsword" rulebooks/dnd5e/weapons/common.go rulebooks/dnd5e/weapons/weapons.go
grep -n "ChainMail\|chain-mail" rulebooks/dnd5e/armor/armor.go
```
Adjust test expectations if IDs or values differ.

- [ ] **Step 4: Implement detail types and resolver**

Create `rulebooks/dnd5e/equipment/detail.go`:

```go
package equipment

import (
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/ammunition"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/armor"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/damage"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/packs"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/shared"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/tools"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/weapons"
)

// EquipmentDetail contains resolved stats for an equipment item.
// Used to enrich equipment choices so the UI can display meaningful details.
type EquipmentDetail struct {
	Name   string               `json:"name"`
	Type   shared.EquipmentType `json:"type"`
	Weight float64              `json:"weight"`
	Cost   string               `json:"cost"`

	// Only one of these is set, based on Type
	Weapon *WeaponDetail `json:"weapon,omitempty"`
	Armor  *ArmorDetail  `json:"armor,omitempty"`
}

// WeaponDetail contains weapon-specific stats.
type WeaponDetail struct {
	Category   weapons.WeaponCategory   `json:"category"`
	Damage     string                   `json:"damage"`
	DamageType damage.Type              `json:"damage_type"`
	Properties []weapons.WeaponProperty `json:"properties"`
	Range      *weapons.Range           `json:"range,omitempty"`
}

// ArmorDetail contains armor-specific stats.
type ArmorDetail struct {
	Category            armor.ArmorCategory `json:"category"`
	BaseAC              int                 `json:"base_ac"`
	DexBonus            bool                `json:"dex_bonus"`
	MaxDexBonus         *int                `json:"max_dex_bonus,omitempty"`
	StrengthRequirement int                 `json:"strength_requirement,omitempty"`
	StealthDisadvantage bool                `json:"stealth_disadvantage"`
}

// ResolveEquipmentDetail looks up an equipment ID across all registries
// and returns a populated detail struct. Returns nil if not found.
func ResolveEquipmentDetail(id shared.EquipmentID) *EquipmentDetail {
	// Check weapons
	if wep, ok := weapons.All[id]; ok {
		return resolveWeaponDetail(&wep)
	}

	// Check armor
	if arm, ok := armor.All[id]; ok {
		return resolveArmorDetail(&arm)
	}

	// Check tools
	if tool, ok := tools.All[id]; ok {
		return &EquipmentDetail{
			Name:   tool.Name,
			Type:   shared.EquipmentTypeTool,
			Weight: float64(tool.Weight),
			Cost:   tool.Cost,
		}
	}

	// Check packs
	if pack, ok := packs.All[id]; ok {
		return &EquipmentDetail{
			Name:   pack.Name,
			Type:   shared.EquipmentTypePack,
			Weight: float64(pack.Weight),
			Cost:   pack.Cost,
		}
	}

	// Check ammunition
	if ammo, ok := ammunition.StandardAmmunition[id]; ok {
		return &EquipmentDetail{
			Name:   ammo.Name,
			Type:   shared.EquipmentTypeAmmunition,
			Weight: ammo.Weight,
			Cost:   ammo.Cost,
		}
	}

	return nil
}

func resolveWeaponDetail(wep *weapons.Weapon) *EquipmentDetail {
	detail := &EquipmentDetail{
		Name:   wep.Name,
		Type:   shared.EquipmentTypeWeapon,
		Weight: wep.Weight,
		Cost:   wep.Cost,
		Weapon: &WeaponDetail{
			Category:   wep.Category,
			Damage:     wep.Damage,
			DamageType: wep.DamageType,
			Properties: wep.Properties,
			Range:      wep.Range,
		},
	}
	return detail
}

func resolveArmorDetail(arm *armor.Armor) *EquipmentDetail {
	detail := &EquipmentDetail{
		Name:   arm.Name,
		Type:   shared.EquipmentTypeArmor,
		Weight: float64(arm.Weight),
		Cost:   arm.Cost,
		Armor: &ArmorDetail{
			Category:            arm.Category,
			BaseAC:              arm.AC,
			DexBonus:            arm.MaxDexBonus == nil || *arm.MaxDexBonus > 0,
			MaxDexBonus:         arm.MaxDexBonus,
			StrengthRequirement: arm.Strength,
			StealthDisadvantage: arm.StealthDisadvantage,
		},
	}
	return detail
}
```

- [ ] **Step 5: Run tests — verify they pass**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./equipment/ -v -run TestDetailSuite
```

Expected: all PASS. If any test fails due to specific field values (e.g., exact weapon names, IDs), check the weapon/armor registries and adjust test expectations.

- [ ] **Step 6: Run full module tests and lint**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./...
golangci-lint run ./...
```

Expected: all pass with no new issues.

- [ ] **Step 7: Commit**

```bash
git add rulebooks/dnd5e/equipment/detail.go rulebooks/dnd5e/equipment/detail_test.go
git commit -m "feat: add EquipmentDetail types and ResolveEquipmentDetail

Defines EquipmentDetail, WeaponDetail, and ArmorDetail in the equipment
package. ResolveEquipmentDetail looks up any equipment ID and returns
populated stats for UI display."
```

---

## Task 3: Toolkit — Add `Detail` field to `EquipmentItem` in choices

**Repo:** `rpg-toolkit` (same branch as Task 2)
**Depends on:** Task 2 complete. Protos PR merged (Task 1) is NOT required — toolkit types don't depend on protos.
**Files:**
- Modify: `rulebooks/dnd5e/character/choices/requirements.go:74-78`

- [ ] **Step 1: Write the failing test**

Add a test to `rulebooks/dnd5e/equipment/detail_test.go` (or a new test in the choices package) that verifies equipment requirements include details. Since requirements are built by class-specific functions, test via the existing API:

Add to `detail_test.go`:

```go
func (s *DetailTestSuite) TestFighterRequirementsHaveDetails() {
	// Import choices package and call getFighterEquipmentRequirements
	// This test lives in equipment_test package so it can't call unexported functions.
	// Instead, test through the public GetClassRequirements API.
}
```

Actually — the requirement builders are unexported per-class functions. The enrichment should happen in the requirement builders themselves. A better test point: test through `choices.GetClassRequirements("fighter")` and verify the equipment items have Detail populated.

Add a new test file `rulebooks/dnd5e/character/choices/requirements_detail_test.go`:

```go
package choices

import (
	"testing"

	"github.com/stretchr/testify/suite"
)

type RequirementsDetailTestSuite struct {
	suite.Suite
}

func TestRequirementsDetailSuite(t *testing.T) {
	suite.Run(t, new(RequirementsDetailTestSuite))
}

func (s *RequirementsDetailTestSuite) TestFighterEquipmentItemsHaveDetails() {
	reqs := GetClassRequirements("fighter")

	for _, req := range reqs.Equipment {
		for _, opt := range req.Options {
			for _, item := range opt.Items {
				s.Assert().NotNilf(item.Detail,
					"equipment item %q in option %q should have detail populated",
					item.ID, opt.Label)
				s.Assert().NotEmpty(item.Detail.Name,
					"detail for %q should have a name", item.ID)
			}
		}
	}
}

func (s *RequirementsDetailTestSuite) TestBarbarianEquipmentItemsHaveDetails() {
	reqs := GetClassRequirements("barbarian")

	for _, req := range reqs.Equipment {
		for _, opt := range req.Options {
			for _, item := range opt.Items {
				s.Assert().NotNilf(item.Detail,
					"equipment item %q in option %q should have detail populated",
					item.ID, opt.Label)
			}
		}
	}
}
```

- [ ] **Step 2: Run tests — verify they fail**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./character/choices/ -v -run TestRequirementsDetailSuite
```

Expected: FAIL — `Detail` field doesn't exist on `EquipmentItem`.

- [ ] **Step 3: Add `Detail` field to `EquipmentItem`**

In `rulebooks/dnd5e/character/choices/requirements.go`, modify the `EquipmentItem` struct (line 74):

```go
import "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/equipment"

type EquipmentItem struct {
	ID       shared.EquipmentID       `json:"id"`
	Quantity int                      `json:"quantity"`
	Detail   *equipment.EquipmentDetail `json:"detail,omitempty"`
}
```

Check that the `choices` package doesn't already import `equipment` — if it creates a cycle, investigate. The dependency direction is `choices` → `equipment` → `weapons`/`armor`/etc., and `choices` already imports `weapons`/`armor` for IDs, so `choices` → `equipment` should be fine.

- [ ] **Step 4: Populate Detail in requirement builders**

Add a helper function near the top of `requirements.go` (or at the bottom):

```go
// enrichEquipmentItems resolves details for a slice of EquipmentItems.
func enrichEquipmentItems(items []EquipmentItem) []EquipmentItem {
	for i := range items {
		items[i].Detail = equipment.ResolveEquipmentDetail(items[i].ID)
	}
	return items
}
```

Then call `enrichEquipmentItems` in each class requirement builder after constructing the items. For example in `getFighterEquipmentRequirements()`, after building each `EquipmentOption.Items` slice:

Find each place where `Items: []EquipmentItem{...}` is constructed and wrap it:

```go
Items: enrichEquipmentItems([]EquipmentItem{
    {ID: armor.ChainMail, Quantity: 1},
}),
```

Do this for every class requirement builder: Fighter, Barbarian, Monk, Rogue (and any others present).

- [ ] **Step 5: Run tests — verify they pass**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./character/choices/ -v -run TestRequirementsDetailSuite
```

Expected: all PASS.

- [ ] **Step 6: Run full module tests and lint**

```bash
cd /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e
go test ./...
golangci-lint run ./...
```

Expected: all pass. Watch for import cycle errors — if `choices` → `equipment` causes a cycle, the `equipment` package may need to not import `choices` (it shouldn't — verify).

- [ ] **Step 7: Commit**

```bash
git add rulebooks/dnd5e/character/choices/requirements.go rulebooks/dnd5e/character/choices/requirements_detail_test.go
git commit -m "feat: enrich EquipmentItem with resolved equipment details

Adds Detail field to EquipmentItem and populates it via
equipment.ResolveEquipmentDetail in all class requirement builders."
```

---

## Task 4: Toolkit — Pre-commit, push, and PR

**Repo:** `rpg-toolkit` (same branch as Tasks 2-3)

- [ ] **Step 1: Run pre-commit checks**

```bash
cd /home/kirk/personal/rpg-toolkit
make pre-commit
```

Expected: all pass. Fix any formatting or lint issues.

- [ ] **Step 2: Push and create PR**

```bash
git push -u origin feat/equipment-detail-types
gh pr create --title "Add EquipmentDetail types and enrich equipment choices" --body "..."
```

After merge, the new toolkit version needs to be published so the API can consume it.

---

## Task 5: API — Map equipment detail to proto

**Repo:** `rpg-api`
**Branch:** `feat/equipment-detail-mapping`
**Issue:** Create issue on project board before starting.
**Depends on:** Task 1 (protos) merged + tagged, Task 4 (toolkit) merged + new version available.

**Files:**
- Create: `internal/handlers/dnd5e/v1alpha1/character/cost_parser.go`
- Create: `internal/handlers/dnd5e/v1alpha1/character/cost_parser_test.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters.go:2305-2460`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters_test.go`

**Reference files** (read-only):
- `internal/handlers/dnd5e/v1alpha1/character/converters.go` — existing `createEquipmentChoice` and `setEquipmentItemTypeHint` functions
- Proto generated types in the go module for `Equipment`, `WeaponData`, `ArmorData`, `Cost`, `Weight`

- [ ] **Step 1: Create branch and update dependencies**

```bash
cd /home/kirk/personal/rpg-api
git checkout main && git pull
git checkout -b feat/equipment-detail-mapping

# Check current dependency versions
go list -m github.com/KirkDiggler/rpg-api-protos/gen/go
go list -m github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e

# Update proto dependency to version with equipment_detail field
go get github.com/KirkDiggler/rpg-api-protos/gen/go@latest

# Update toolkit dependency to version with EquipmentDetail
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@latest

go mod tidy
```

- [ ] **Step 2: Write failing tests for cost/weight parsing**

Create `internal/handlers/dnd5e/v1alpha1/character/cost_parser_test.go`:

```go
package character

import (
	"testing"

	"github.com/stretchr/testify/suite"
)

type CostParserTestSuite struct {
	suite.Suite
}

func TestCostParserSuite(t *testing.T) {
	suite.Run(t, new(CostParserTestSuite))
}

func (s *CostParserTestSuite) TestParseCost() {
	s.Run("standard gold", func() {
		cost := parseCostString("25 gp")
		s.Require().NotNil(cost)
		s.Assert().Equal(int32(25), cost.Quantity)
		s.Assert().Equal("gp", cost.Unit)
	})

	s.Run("silver", func() {
		cost := parseCostString("5 sp")
		s.Require().NotNil(cost)
		s.Assert().Equal(int32(5), cost.Quantity)
		s.Assert().Equal("sp", cost.Unit)
	})

	s.Run("copper", func() {
		cost := parseCostString("1 cp")
		s.Require().NotNil(cost)
		s.Assert().Equal(int32(1), cost.Quantity)
		s.Assert().Equal("cp", cost.Unit)
	})

	s.Run("large amount", func() {
		cost := parseCostString("1500 gp")
		s.Require().NotNil(cost)
		s.Assert().Equal(int32(1500), cost.Quantity)
		s.Assert().Equal("gp", cost.Unit)
	})

	s.Run("empty string returns nil", func() {
		cost := parseCostString("")
		s.Assert().Nil(cost)
	})

	s.Run("malformed returns nil", func() {
		cost := parseCostString("free")
		s.Assert().Nil(cost)
	})
}

func (s *CostParserTestSuite) TestWeightToProto() {
	s.Run("integer weight", func() {
		w := weightToProto(55)
		s.Require().NotNil(w)
		s.Assert().Equal(int32(55), w.Quantity)
		s.Assert().Equal("lb", w.Unit)
	})

	s.Run("zero weight returns nil", func() {
		w := weightToProto(0)
		s.Assert().Nil(w)
	})
}
```

- [ ] **Step 3: Run tests — verify they fail**

```bash
cd /home/kirk/personal/rpg-api
go test ./internal/handlers/dnd5e/v1alpha1/character/ -v -run TestCostParserSuite
```

Expected: FAIL — functions not defined.

- [ ] **Step 4: Implement cost/weight parsing**

Create `internal/handlers/dnd5e/v1alpha1/character/cost_parser.go`:

```go
package character

import (
	"fmt"
	"strconv"
	"strings"

	dnd5ev1alpha1 "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha1"
)

// parseCostString converts a cost string like "25 gp" into a proto Cost message.
// Returns nil if the string cannot be parsed.
func parseCostString(s string) *dnd5ev1alpha1.Cost {
	if s == "" {
		return nil
	}

	parts := strings.SplitN(strings.TrimSpace(s), " ", 2)
	if len(parts) != 2 {
		return nil
	}

	qty, err := strconv.Atoi(parts[0])
	if err != nil {
		return nil
	}

	return &dnd5ev1alpha1.Cost{
		Quantity: int32(qty),
		Unit:     parts[1],
	}
}

// weightToProto converts a float64 weight to a proto Weight message.
// Returns nil if weight is zero.
func weightToProto(w float64) *dnd5ev1alpha1.Weight {
	if w == 0 {
		return nil
	}

	return &dnd5ev1alpha1.Weight{
		Quantity: int32(w),
		Unit:     "lb",
	}
}
```

- [ ] **Step 5: Run tests — verify they pass**

```bash
cd /home/kirk/personal/rpg-api
go test ./internal/handlers/dnd5e/v1alpha1/character/ -v -run TestCostParserSuite
```

Expected: all PASS.

- [ ] **Step 6: Commit**

```bash
git add internal/handlers/dnd5e/v1alpha1/character/cost_parser.go internal/handlers/dnd5e/v1alpha1/character/cost_parser_test.go
git commit -m "feat: add cost string and weight to proto converters

Parses toolkit cost strings like '25 gp' into proto Cost messages.
Converts float64 weight to proto Weight messages."
```

---

## Task 6: API — Wire equipment detail into choice converter

**Repo:** `rpg-api` (same branch as Task 5)

**Files:**
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters_test.go`

**Key context:** The `createEquipmentChoice` function (line 2305) builds proto `EquipmentBundle` messages from toolkit `EquipmentRequirement`. Currently it creates `EquipmentItem` with `selection_id`, `quantity`, and type hints. We need to also populate `equipment_detail` from the toolkit's new `Detail` field.

- [ ] **Step 1: Write the failing test**

Add to `converters_test.go` — a test that verifies equipment detail is mapped:

```go
func (s *ConvertersTestSuite) TestCreateEquipmentChoiceIncludesDetail() {
	// Build a toolkit EquipmentRequirement with Detail populated.
	// This requires importing the toolkit choices and equipment packages.
	// Call createEquipmentChoice and verify the proto EquipmentItem
	// has equipment_detail populated with correct WeaponData/ArmorData.

	reqs := choices.GetClassRequirements("fighter")
	s.Require().NotEmpty(reqs.Equipment)

	// Convert first equipment requirement
	choice := createEquipmentChoice(reqs.Equipment[0])

	// Find an option that has a weapon (e.g., the martial weapon option)
	var foundWeaponDetail bool
	var foundArmorDetail bool

	for _, bundle := range choice.GetEquipmentChoice().GetBundles() {
		for _, item := range bundle.GetItems() {
			if item.GetEquipmentDetail() != nil {
				detail := item.GetEquipmentDetail()
				if detail.GetWeaponData() != nil {
					foundWeaponDetail = true
					s.Assert().NotEmpty(detail.GetWeaponData().GetDamageDice())
				}
				if detail.GetArmorData() != nil {
					foundArmorDetail = true
					s.Assert().Greater(detail.GetArmorData().GetBaseAc(), int32(0))
				}
			}
		}
	}

	// Fighter equipment choices should include both weapons and armor
	s.Assert().True(foundWeaponDetail || foundArmorDetail,
		"expected at least one equipment item with detail")
}
```

Note: Adjust the test based on the actual proto accessor names and the converter function signature. The existing test file shows the patterns used.

- [ ] **Step 2: Run test — verify it fails**

```bash
go test ./internal/handlers/dnd5e/v1alpha1/character/ -v -run TestCreateEquipmentChoiceIncludesDetail
```

Expected: FAIL — `equipment_detail` field is nil on all items.

- [ ] **Step 3: Add detail mapping to `createEquipmentChoice`**

In `converters.go`, inside `createEquipmentChoice`, after setting the type hint on each `EquipmentItem`, add the detail mapping. Add a new helper function:

```go
// convertEquipmentDetailToProto maps a toolkit EquipmentDetail to a proto Equipment message.
func convertEquipmentDetailToProto(detail *equipment.EquipmentDetail) *dnd5ev1alpha1.Equipment {
	if detail == nil {
		return nil
	}

	proto := &dnd5ev1alpha1.Equipment{
		Id:     "", // Not needed for inline display
		Name:   detail.Name,
		Cost:   parseCostString(detail.Cost),
		Weight: weightToProto(detail.Weight),
	}

	switch detail.Type {
	case shared.EquipmentTypeWeapon:
		if detail.Weapon != nil {
			proto.Category = mapWeaponCategoryToEquipmentCategory(detail.Weapon.Category)
			proto.EquipmentData = &dnd5ev1alpha1.Equipment_WeaponData{
				WeaponData: convertWeaponDetailToProto(detail.Weapon),
			}
		}
	case shared.EquipmentTypeArmor:
		if detail.Armor != nil {
			proto.Category = mapArmorCategoryToEquipmentCategory(detail.Armor.Category)
			proto.EquipmentData = &dnd5ev1alpha1.Equipment_ArmorData{
				ArmorData: convertArmorDetailToProto(detail.Armor),
			}
		}
	}

	return proto
}

func convertWeaponDetailToProto(w *equipment.WeaponDetail) *dnd5ev1alpha1.WeaponData {
	wd := &dnd5ev1alpha1.WeaponData{
		WeaponCategory: mapWeaponCategoryToProto(w.Category),
		DamageDice:     w.Damage,
		DamageType:     mapDamageTypeToProto(w.DamageType),
		Properties:     mapWeaponPropertiesToProto(w.Properties),
	}

	if w.Range != nil {
		wd.Range = "ranged"
		wd.NormalRange = int32(w.Range.Normal)
		wd.LongRange = int32(w.Range.Long)
	} else {
		wd.Range = "melee"
	}

	return wd
}

func convertArmorDetailToProto(a *equipment.ArmorDetail) *dnd5ev1alpha1.ArmorData {
	ad := &dnd5ev1alpha1.ArmorData{
		ArmorCategory:       mapArmorCategoryToProto(a.Category),
		BaseAc:              int32(a.BaseAC),
		DexBonus:            a.DexBonus,
		StrMinimum:          int32(a.StrengthRequirement),
		StealthDisadvantage: a.StealthDisadvantage,
	}

	if a.MaxDexBonus != nil {
		ad.HasDexLimit = true
		ad.MaxDexBonus = int32(*a.MaxDexBonus)
	}

	return ad
}
```

Note: The `mapWeaponCategoryToProto`, `mapDamageTypeToProto`, `mapWeaponPropertiesToProto`, `mapArmorCategoryToProto`, `mapWeaponCategoryToEquipmentCategory`, and `mapArmorCategoryToEquipmentCategory` functions may already exist in `converters.go` (the `setEquipmentItemTypeHint` function does similar mapping). Reuse existing mappers where possible. If they don't exist as standalone functions, extract them.

Then in `createEquipmentChoice`, after the line that calls `setEquipmentItemTypeHint`, add:

```go
protoItem.EquipmentDetail = convertEquipmentDetailToProto(item.Detail)
```

Where `item` is the toolkit `EquipmentItem` being converted.

- [ ] **Step 4: Run test — verify it passes**

```bash
go test ./internal/handlers/dnd5e/v1alpha1/character/ -v -run TestCreateEquipmentChoiceIncludesDetail
```

Expected: PASS.

- [ ] **Step 5: Run full test suite and pre-commit**

```bash
cd /home/kirk/personal/rpg-api
go test ./...
make pre-commit
make ci-check
```

Expected: all pass.

- [ ] **Step 6: Commit, push, and PR**

```bash
git add internal/handlers/dnd5e/v1alpha1/character/converters.go internal/handlers/dnd5e/v1alpha1/character/converters_test.go go.mod go.sum
git commit -m "feat: map equipment detail to proto in choice converter

Populates equipment_detail on EquipmentItem proto messages when
toolkit provides resolved stats. Includes Cost/Weight conversion."

git push -u origin feat/equipment-detail-mapping
gh pr create --title "Map equipment detail to proto in choice converter" --body "..."
```

---

## Task 7: UI — Build `EquipmentCard` component

**Repo:** `rpg-dnd5e-web`
**Branch:** `feat/equipment-card`
**Issue:** Create issue on project board before starting.
**Depends on:** Task 1 (protos) merged + tagged (for generated TS types).

**Files:**
- Create: `src/components/equipment/EquipmentCard.tsx`

**Reference files** (read-only):
- `src/components/ui/Card.tsx` — Card component patterns, variants, framer motion
- `src/components/choices/EquipmentBundleChoice.tsx` — where this will be wired in
- Proto types: `@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/character_pb` — `Equipment`, `WeaponData`, `ArmorData`
- Proto enums: `@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/enums_pb` — `WeaponCategory`, `ArmorCategory`, `WeaponProperty`, `DamageType`

- [ ] **Step 1: Create branch and update proto dependency**

```bash
cd /home/kirk/personal/rpg-dnd5e-web
git checkout main && git pull
git checkout -b feat/equipment-card

# Update proto package to version with equipment_detail
npm install github:KirkDiggler/rpg-api-protos#<new-tag>
```

- [ ] **Step 2: Create the EquipmentCard component**

Create `src/components/equipment/EquipmentCard.tsx`:

The component accepts the proto `Equipment` type and renders based on which `equipment_data` oneof is set.

```tsx
import type { Equipment } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/character_pb';
import {
  WeaponCategory,
  ArmorCategory,
  WeaponProperty,
  DamageType,
} from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/enums_pb';

interface EquipmentCardProps {
  equipment: Equipment;
  compact?: boolean;
}

export function EquipmentCard({ equipment, compact = false }: EquipmentCardProps) {
  const { name, cost, weight } = equipment;
  const weaponData = equipment.equipmentData.case === 'weaponData'
    ? equipment.equipmentData.value
    : undefined;
  const armorData = equipment.equipmentData.case === 'armorData'
    ? equipment.equipmentData.value
    : undefined;

  return (
    <div className="equipment-card" style={{
      padding: compact ? '4px 8px' : '8px 12px',
      borderRadius: '6px',
      background: 'var(--bg-secondary)',
      border: '1px solid var(--border-primary)',
      fontSize: compact ? '0.85rem' : '0.9rem',
    }}>
      <div style={{ fontWeight: 600, color: 'var(--text-primary)', marginBottom: '4px' }}>
        {name}
      </div>

      {weaponData && <WeaponStats data={weaponData} compact={compact} />}
      {armorData && <ArmorStats data={armorData} compact={compact} />}

      {!compact && (
        <div style={{
          display: 'flex',
          gap: '12px',
          color: 'var(--text-secondary)',
          fontSize: '0.8rem',
          marginTop: '4px',
        }}>
          {cost && <span>{cost.quantity} {cost.unit}</span>}
          {weight && <span>{weight.quantity} {weight.unit}</span>}
        </div>
      )}
    </div>
  );
}
```

Note: The exact proto accessor syntax depends on the buf-generated TypeScript code. Check the generated `character_pb.ts` for the actual field access pattern (it may use `equipment.weaponData` directly or `equipment.equipmentData.case`). Adjust accordingly.

Add `WeaponStats` and `ArmorStats` helper components in the same file:

```tsx
function WeaponStats({ data, compact }: { data: WeaponData; compact: boolean }) {
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '2px' }}>
      <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
        <span style={{ color: 'var(--accent-primary)', fontWeight: 500 }}>
          {data.damageDice} {formatDamageType(data.damageType)}
        </span>
        <span style={{ color: 'var(--text-secondary)', fontSize: '0.8rem' }}>
          {formatWeaponCategory(data.weaponCategory)}
        </span>
      </div>
      {!compact && data.properties.length > 0 && (
        <div style={{ display: 'flex', gap: '4px', flexWrap: 'wrap' }}>
          {data.properties.map((prop) => (
            <span key={prop} style={{
              padding: '1px 6px',
              borderRadius: '3px',
              background: 'var(--bg-tertiary)',
              color: 'var(--text-secondary)',
              fontSize: '0.75rem',
            }}>
              {formatWeaponProperty(prop)}
            </span>
          ))}
        </div>
      )}
      {data.normalRange > 0 && (
        <span style={{ color: 'var(--text-secondary)', fontSize: '0.8rem' }}>
          Range: {data.normalRange}/{data.longRange} ft
        </span>
      )}
    </div>
  );
}

function ArmorStats({ data, compact }: { data: ArmorData; compact: boolean }) {
  let acText = `AC ${data.baseAc}`;
  if (data.dexBonus) {
    acText += data.hasDexLimit ? ` + Dex (max ${data.maxDexBonus})` : ' + Dex';
  }

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '2px' }}>
      <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
        <span style={{ color: 'var(--accent-primary)', fontWeight: 500 }}>
          {acText}
        </span>
        <span style={{ color: 'var(--text-secondary)', fontSize: '0.8rem' }}>
          {formatArmorCategory(data.armorCategory)}
        </span>
      </div>
      {!compact && (
        <div style={{ display: 'flex', gap: '8px', fontSize: '0.8rem', color: 'var(--text-secondary)' }}>
          {data.stealthDisadvantage && <span>Stealth disadvantage</span>}
          {data.strMinimum > 0 && <span>Str {data.strMinimum} required</span>}
        </div>
      )}
    </div>
  );
}
```

Add formatting helpers at the bottom of the file. Check `enums_pb.ts` for exact enum constant names and adjust accordingly:

```tsx
function formatDamageType(dt: DamageType): string {
  const names: Partial<Record<DamageType, string>> = {
    [DamageType.SLASHING]: 'slashing',
    [DamageType.PIERCING]: 'piercing',
    [DamageType.BLUDGEONING]: 'bludgeoning',
    [DamageType.FIRE]: 'fire',
    [DamageType.COLD]: 'cold',
    [DamageType.LIGHTNING]: 'lightning',
    [DamageType.THUNDER]: 'thunder',
    [DamageType.ACID]: 'acid',
    [DamageType.POISON]: 'poison',
    [DamageType.NECROTIC]: 'necrotic',
    [DamageType.RADIANT]: 'radiant',
    [DamageType.FORCE]: 'force',
    [DamageType.PSYCHIC]: 'psychic',
  };
  return names[dt] ?? 'unknown';
}

function formatWeaponCategory(wc: WeaponCategory): string {
  const names: Partial<Record<WeaponCategory, string>> = {
    [WeaponCategory.SIMPLE]: 'Simple',
    [WeaponCategory.MARTIAL]: 'Martial',
  };
  return names[wc] ?? '';
}

function formatArmorCategory(ac: ArmorCategory): string {
  const names: Partial<Record<ArmorCategory, string>> = {
    [ArmorCategory.LIGHT]: 'Light',
    [ArmorCategory.MEDIUM]: 'Medium',
    [ArmorCategory.HEAVY]: 'Heavy',
    [ArmorCategory.SHIELD]: 'Shield',
  };
  return names[ac] ?? '';
}

function formatWeaponProperty(wp: WeaponProperty): string {
  const names: Partial<Record<WeaponProperty, string>> = {
    [WeaponProperty.LIGHT]: 'Light',
    [WeaponProperty.HEAVY]: 'Heavy',
    [WeaponProperty.FINESSE]: 'Finesse',
    [WeaponProperty.THROWN]: 'Thrown',
    [WeaponProperty.TWO_HANDED]: 'Two-Handed',
    [WeaponProperty.VERSATILE]: 'Versatile',
    [WeaponProperty.AMMUNITION]: 'Ammunition',
    [WeaponProperty.LOADING]: 'Loading',
    [WeaponProperty.REACH]: 'Reach',
    [WeaponProperty.SPECIAL]: 'Special',
  };
  return names[wp] ?? '';
}
```

**Note on proto accessor pattern:** buf-generated TypeScript may access oneofs differently depending on version. Before implementing, check the generated `character_pb.ts` for the actual pattern. It may be `equipment.equipmentData.case === 'weaponData'` (newer buf) or direct field access like `equipment.weaponData` (older). Adjust the component accordingly.

- [ ] **Step 3: Verify it compiles**

```bash
npm run typecheck
```

Expected: no errors.

- [ ] **Step 4: Commit**

```bash
git add src/components/equipment/EquipmentCard.tsx
git commit -m "feat: add EquipmentCard component for displaying equipment stats

Renders weapon damage/properties, armor AC/category, or basic info
for tools/packs. Supports compact mode for use in dropdowns."
```

---

## Task 8: UI — Wire `EquipmentCard` into equipment choices

**Repo:** `rpg-dnd5e-web` (same branch as Task 7)

**Files:**
- Modify: `src/components/choices/EquipmentBundleChoice.tsx`

**Key context:** `EquipmentBundleChoice.tsx` currently renders equipment options as buttons with text labels. When `equipment_detail` is present on an `EquipmentItem`, render an `EquipmentCard` instead.

**Category dropdowns note:** Category choices (e.g., "choose a martial weapon") use the `ListEquipmentByType` API endpoint which already returns full `Equipment` proto messages — not `EquipmentItem`. Those items already have `WeaponData`/`ArmorData` populated via `ConvertEquipmentToProto()` in the API. So category dropdowns can already use `EquipmentCard` directly with the existing data — no additional enrichment needed on the category path. The `CategorySelector` sub-component (lines 26-170) just needs to pass the equipment data to `EquipmentCard`.

- [ ] **Step 1: Import `EquipmentCard` and wire into bundle items**

In `EquipmentBundleChoice.tsx`, import the new component:

```tsx
import { EquipmentCard } from '../equipment/EquipmentCard';
```

Find where bundle items are rendered (inside the bundle button or the selected bundle display). Where each item in `bundle.items` is displayed, add a conditional:

```tsx
{item.equipmentDetail ? (
  <EquipmentCard equipment={item.equipmentDetail} compact />
) : (
  <span>{item.selectionId}</span>
)}
```

- [ ] **Step 2: Wire into CategorySelector dropdown options**

In the `CategorySelector` component, the `<select>` dropdown shows equipment names. When the equipment list is fetched for category choices, check if the items have `equipmentDetail` and show `EquipmentCard` in a tooltip or expanded view below the dropdown.

Note: `<select>` elements can only show plain text in `<option>` tags. Consider switching to a custom dropdown (Radix Select is available in the project) to show `EquipmentCard` in the options. Or show the `EquipmentCard` for the currently selected item below the dropdown. The simpler approach:

```tsx
{/* Show detail card for currently selected item */}
{selectedEquipment?.equipmentDetail && (
  <EquipmentCard equipment={selectedEquipment.equipmentDetail} />
)}
```

- [ ] **Step 3: Run ci-check**

```bash
npm run ci-check
```

Expected: all pass (format, lint, typecheck, build).

- [ ] **Step 4: Commit, push, and PR**

```bash
git add src/components/choices/EquipmentBundleChoice.tsx
git commit -m "feat: show equipment stats in character creation choices

Uses EquipmentCard to display weapon/armor stats inline when
equipment_detail is available on choice items."

git push -u origin feat/equipment-card
gh pr create --title "Show equipment stats in character creation choices" --body "..."
```

---

## Dependency Graph

```
Task 1 (protos PR) ──────────────────────┬──> Task 5-6 (API) ──> API PR
  [wait for user merge + CI generation]  │
                                          │
Task 2 (toolkit types) ──> Task 3 (toolkit enrich) ──> Task 4 (toolkit PR)
  [start immediately]     [after protos merged]         │
                                                         └──> Task 5-6 (API)

Task 1 (protos merged) ──> Task 7-8 (UI) ──> UI PR
```

**Execution order:**
1. Task 1 (protos) + Tasks 2 (toolkit types) — start in parallel
2. Wait for protos PR merge (user merges, CI generates code)
3. Task 3 (toolkit enrich) — can proceed after protos merged
4. Task 4 (toolkit PR) — copilot reviews, user merges
5. Tasks 5-6 (API) — after both protos and toolkit merged
6. Tasks 7-8 (UI) — after protos merged (can parallel with API)

**Four PRs total (copilot reviews all Go/web repos):**
1. `rpg-api-protos` — Task 1
2. `rpg-toolkit` — Tasks 2-4
3. `rpg-api` — Tasks 5-6
4. `rpg-dnd5e-web` — Tasks 7-8
