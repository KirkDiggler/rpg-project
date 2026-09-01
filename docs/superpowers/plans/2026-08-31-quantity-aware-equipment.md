# Quantity-Aware Equipment Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let character creation choose the same eligible weapon twice, persist one truthful quantity stack, project both equipped slots, and let the generic web equip every owned copy.

**Architecture:** The D&D toolkit owns duplicate-choice legality, canonical quantity stacks, occupancy, and the owner projection. The encounter proto adds quantity, rpg-api maps toolkit fields directly, and the web displays remaining carried copies without deciding legality.

**Tech Stack:** Go 1.24, testify, protobuf/buf, Connect RPC, React 19, TypeScript, Vitest/Testing Library.

**Spec:** `docs/superpowers/specs/2026-08-31-run-readiness-and-activation-story-design.md`

## Global Constraints

- Keep existing character-draft and EquipItem RPCs; add no mutation endpoint.
- Inventory quantity is a positive total owned count; no item-instance IDs.
- The same item ref may occupy multiple compatible slots only up to owned quantity.
- Toolkit owns legality; rpg-api is field-for-field; web performs presentation bookkeeping only.
- Preserve authored fixed quantities, order, two-handed occupancy, and slot compatibility.
- Thrown delivery and inventory consumption are excluded and tracked by rpg-toolkit#1355.

---

### Task 1: Accept repeated equipment picks and compile canonical stacks

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/choices/validation.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/draft.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/draft_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/category_equipment_test.go`

**Interfaces:**
- Consumes: `EquipmentCategoryChoice.Choose`, ordered `EquipmentChoiceSelection.CategorySelections`.
- Produces: finalized `[]InventoryItem` with one row per equipment ID and summed `Quantity`.

- [ ] **Step 1: Add the failing Fighter choose-two test**

Add a case using `choices.FighterWeaponTwoMartial` with two `weapons.Longsword` category selections. Assert SetClass and ToCharacter succeed, the inventory has exactly one longsword row, and its quantity is two:

```go
err := draft.SetClass(&character.SetClassInput{
    ClassID: classes.Fighter,
    Choices: character.ClassChoices{
        Skills: []skills.Skill{skills.Athletics, skills.Intimidation},
        Equipment: []character.EquipmentChoiceSelection{
            {ChoiceID: choices.FighterArmor, OptionID: choices.FighterArmorChainMail},
            {
                ChoiceID: choices.FighterWeaponsPrimary,
                OptionID: choices.FighterWeaponTwoMartial,
                CategorySelections: []shared.EquipmentID{
                    weapons.Longsword,
                    weapons.Longsword,
                },
            },
            {ChoiceID: choices.FighterWeaponsSecondary, OptionID: choices.FighterRangedCrossbow},
            {ChoiceID: choices.FighterPack, OptionID: choices.FighterPackDungeoneer},
        },
        FightingStyle: fightingstyles.Defense,
    },
})
s.Require().NoError(err)
char, err := draft.ToCharacter(s.ctx, "fighter-two-longswords", s.bus)
s.Require().NoError(err)
s.assertInventoryStack(char.ToData().Inventory, weapons.Longsword, 2,
    "two identical legal picks become one quantity-two stack")
```

Also assert an ineligible repeated ID and the wrong number of picks still fail.

- [ ] **Step 2: Run the focused tests and verify RED**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character -run 'TestDraftSuite/TestCompileInventory|TestCategoryBasedEquipment' -count=1
```

Expected: FAIL because duplicate category selections are rejected.

- [ ] **Step 3: Remove equipment-only uniqueness rejection**

Delete the per-category `seen` rejection from:

- `Draft.validatePersistedCategoryEquipmentChoice`;
- `Draft.validateCategorySelections`;
- the equipment arm in `choices.Validator`.

Keep exact count and eligibility validation. Do not alter uniqueness rules for skills, tools, languages, expertise, or fighting styles.

- [ ] **Step 4: Add deterministic stack folding**

Add a focused helper in `draft.go` and run all grant/choice materialization through it:

```go
func stackInventory(items []InventoryItem) []InventoryItem {
    out := make([]InventoryItem, 0, len(items))
    index := make(map[shared.EquipmentID]int, len(items))
    for _, item := range items {
        id := item.Equipment.EquipmentID()
        if item.Quantity <= 0 {
            panic(fmt.Sprintf("BUG: nonpositive quantity %d for %s", item.Quantity, id))
        }
        if at, ok := index[id]; ok {
            out[at].Quantity += item.Quantity
            continue
        }
        index[id] = len(out)
        out = append(out, item)
    }
    return out
}
```

Return `stackInventory(inventory)` from `compileInventory`. First occurrence fixes display order.

- [ ] **Step 5: Run the root character suite and verify GREEN**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character ./character/choices -count=1
```

Expected: PASS, including fixed two-handaxe, four-javelin, ten-dart, and ammunition cases.

- [ ] **Step 6: Commit the choice/stack provider**

```bash
git add rulebooks/dnd5e/character/choices/validation.go \
  rulebooks/dnd5e/character/draft.go \
  rulebooks/dnd5e/character/draft_test.go \
  rulebooks/dnd5e/character/category_equipment_test.go
git commit -m "feat: compile duplicate equipment picks as stacks"
```

### Task 2: Enforce copy occupancy and project the authoritative slot map

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/character.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/equipment_display.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/equip_occupancy_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/equipment_display_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/load.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/load_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/data.go`
- Modify only when an old malformed fixture requires it: `rpg-toolkit/rulebooks/dnd5e/character/attack_definition_test.go`

**Interfaces:**
- Consumes: canonical or legacy inventory rows and `EquipmentSlots`.
- Strict loading rejects persisted quantity `<= 0`; lenient loading warns and drops the malformed row rather than defaulting it to one.
- Produces:

```go
type EquippedItemView struct {
    ItemID string
    Name string
    Kind string
    SlotKeys []string
    StatLine string
    Quantity int
}

type EquipmentView struct {
    Items []EquippedItemView
    Equipped EquipmentSlots
    // existing Slots, ACTotal, ACNote, MainHandDamage
}
```

- [ ] **Step 1: Add failing occupancy and projection tests**

Add these cases:

```go
func (s *EquipOccupancyTestSuite) TestOneCopyCannotOccupyTwoSlots() {
    s.char.inventory = []InventoryItem{{Equipment: &handaxe, Quantity: 1}}
    s.Require().NoError(s.char.EquipItem(SlotMainHand, weapons.Handaxe))
    s.Require().NoError(s.char.EquipItem(SlotOffHand, weapons.Handaxe))
    s.Empty(s.char.equipmentSlots.Get(SlotMainHand))
    s.Equal(weapons.Handaxe, s.char.equipmentSlots.Get(SlotOffHand))
}

func (s *EquipOccupancyTestSuite) TestTwoCopiesOccupyBothSlots() {
    s.char.inventory = []InventoryItem{{Equipment: &handaxe, Quantity: 2}}
    s.Require().NoError(s.char.EquipItem(SlotMainHand, weapons.Handaxe))
    s.Require().NoError(s.char.EquipItem(SlotOffHand, weapons.Handaxe))
    s.Equal(weapons.Handaxe, s.char.equipmentSlots.Get(SlotMainHand))
    s.Equal(weapons.Handaxe, s.char.equipmentSlots.Get(SlotOffHand))
}
```

In `equipment_display_test.go`, assert one quantity-two item and both entries in `view.Equipped`.

- [ ] **Step 2: Run the tests and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character -run 'TestEquipOccupancy|TestEquipmentDisplay' -count=1
```

Expected: projection test fails because quantity and a multi-slot map are absent.

- [ ] **Step 3: Make occupancy count generic**

Before setting the requested slot, count other slots that already reference `itemID`. If that count is at least owned copies, preserve single-copy move behavior by clearing the old slot; for quantities greater than one, refuse any true overdraw rather than manufacturing another copy. Exclude the requested slot itself so re-equipping is idempotent.

Use a typed invalid-argument/resource error with `item_id`, `owned`, and `equipped` metadata for an overdraw unreachable through today's three-slot taxonomy but possible with future slots or corrupted state.

- [ ] **Step 4: Replace singular item Slot with quantity plus cloned slot map**

Remove `EquippedItemView.Slot`, add `Quantity`, and add `EquipmentView.Equipped`. Aggregate legacy duplicate rows in the view by item ID and clone the slot map:

```go
view.Equipped = maps.Clone(c.equipmentSlots)
```

Never expose the character's mutable map.

- [ ] **Step 5: Add strict/lenient persisted-quantity tests and handling**

Test both `0` and `-1`. Strict `Load` must return a typed invalid-argument error carrying item ID, index, and quantity. Lenient load must emit its ordinary dropped-effect warning and omit the row. Capture RED before changing `loadInventory`, then add the check before catalog resolution/append. Never interpret malformed quantity as one.

If an existing strict-load test used a zero-quantity row to model absence, rewrite that fixture to omit the inventory row while retaining the same behavioral assertion and document why.

- [ ] **Step 6: Run character tests and static checks**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character -count=1
go vet ./character/...
golangci-lint run ./character/...
```

Expected: PASS with zero lint issues.

- [ ] **Step 7: Commit the occupancy/projection provider**

```bash
git add rulebooks/dnd5e/character/character.go \
  rulebooks/dnd5e/character/equipment_display.go \
  rulebooks/dnd5e/character/equip_occupancy_test.go \
  rulebooks/dnd5e/character/equipment_display_test.go \
  rulebooks/dnd5e/character/load.go \
  rulebooks/dnd5e/character/load_test.go \
  rulebooks/dnd5e/character/data.go \
  rulebooks/dnd5e/character/attack_definition_test.go
git commit -m "feat: project quantity-aware equipment occupancy"
```

### Task 3: Verify and publish the toolkit root provider

**Files:**
- Modify if invalidated: `rpg-toolkit/docs/status.md`

**Interfaces:**
- Produces: a published `rulebooks/dnd5e` tag containing Tasks 1-2.

- [ ] **Step 1: Run full root verification**

```bash
cd rpg-toolkit/rulebooks/dnd5e
gofmt -w character/choices/validation.go character/draft.go \
  character/character.go character/equipment_display.go \
  character/*_test.go
go test ./... -count=1
go vet ./...
cd ../../..
make lint-all
```

Expected for the changed root D&D module: tests, vet, and module lint pass. Run root `make lint-all` and `make pre-commit`; if either stops in an unchanged module on an established repository defect, record the exact output, prove the branch has no diff in that area, and keep the scoped changed-module evidence. The known core coverage parser defect is one such root-wide blocker; do not repair unrelated modules in this slice.

- [ ] **Step 2: Open the toolkit PR and wait for human merge**

The PR body must list RED/GREEN evidence and `Closes` the toolkit provider issue. Do not create consumer pins from an unpublished commit.

- [ ] **Step 3: Record the CI-minted root tag**

After merge:

```bash
gh release list --repo KirkDiggler/rpg-toolkit --limit 30 | rg 'rulebooks/dnd5e/'
```

Record the exact tag for Tasks 5 and 6.

### Task 4: Add quantity to the owner Item wire contract

**Files:**
- Modify: `rpg-api-protos/dnd5e/api/v1alpha2/encounter/types.proto`
- CI-generated after merge on the managed `generated` branch: Go and TypeScript SDKs. Never commit generated output on the feature branch.

**Interfaces:**
- Produces: `Item.quantity` field 7, positive total-owned count.

- [ ] **Step 1: Add the additive field**

```protobuf
message Item {
  Ref ref = 1;
  string name = 2;
  string stat_line = 3;
  string icon_key = 4;
  string kind = 5;
  repeated string slot_keys = 6;
  int32 quantity = 7; // total copies owned; updated producers always send > 0
}
```

- [ ] **Step 2: Format, generate ephemerally, and verify**

```bash
cd rpg-api-protos
buf format -w
make test
buf breaking --disable-symlinks --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
git diff --check
git status --short
```

Expected: lint/format/generation/compile gates pass and the breaking check accepts the additive field. Any generated working files are verification artifacts only and must be cleaned before commit; the feature diff contains the `.proto` source alone.

- [ ] **Step 3: Commit and publish the proto PR**

```bash
git add dnd5e/api/v1alpha2/encounter/types.proto
git commit -m "feat: expose owned item quantity"
```

Open the PR, wait for human merge, then let CI update the managed `generated` branch and publish the root/npm tag. Record both the exact generated commit and tag. Until rpg-api-protos#261 publishes module-qualified `gen/go/v…` tags, Go consumers pin the exact generated commit (which resolves to an immutable pseudo-version); they must not use the moving `@generated` branch as their final committed requirement.

### Task 5: Pin and map quantity through rpg-api

**Files:**
- Modify: `rpg-api/go.mod`
- Modify: `rpg-api/go.sum`
- Modify: `rpg-api/internal/handlers/dnd5e/v2/character/character_data.go`
- Modify: `rpg-api/internal/handlers/dnd5e/v2/character/handler_test.go`
- Modify: `rpg-api/internal/integration/harness/harness.go` to expose the already-registered v1alpha2 CharacterService test client.
- Modify: `rpg-api/internal/integration/character/creation_test.go`
- Modify if invalidated: `rpg-api/docs/status.md`

**Interfaces:**
- Consumes: toolkit `EquipmentView.Equipped`, `EquippedItemView.Quantity`, proto `Item.quantity`.
- Produces: owner `CharacterData` with both hand slots and positive item quantity.

- [ ] **Step 1: Pin published provider/proto versions**

For this delivery, pin toolkit `rulebooks/dnd5e/v0.124.0` and generated proto commit `1e5c208d02ee4d81f167bc8d5ae272016ca0bd57` with `GOPROXY=direct go get`, then `go mod tidy`. The proto commit must resolve to a Go pseudo-version ending in `1e5c208`; verify that with `go list -m -json`. No `replace`, workspace, branch name, or moving `@generated` requirement survives the commit. The pseudo-version is the immutable published generated commit fallback recorded by rpg-api-protos#261, not an unpublished feature commit.

- [ ] **Step 2: Write failing mapper and integration assertions**

Assert a quantity-two handaxe stack maps as:

```go
s.Require().Len(cd.GetInventory(), 1)
s.Equal(int32(2), cd.GetInventory()[0].GetQuantity())
s.Equal("handaxe", cd.GetEquipped()["main_hand"].GetId())
s.Equal("handaxe", cd.GetEquipped()["off_hand"].GetId())
```

The integration test must finalize a Fighter whose choose-two category contains the same martial weapon twice, read the owner v1alpha2 `CharacterData`, then use the real v1alpha2 CharacterService EquipItem path for both hands. The shared integration harness already registers that service; expose its generated client rather than constructing a parallel server path.

- [ ] **Step 3: Run tests and verify RED**

```bash
cd rpg-api
go test ./internal/handlers/dnd5e/v2/character ./internal/integration/character -count=1
```

Expected: FAIL until mapper uses new fields/map.

- [ ] **Step 4: Map fields directly**

In `mapEquipment`, set `Quantity: int32(item.Quantity)` and build `cd.Equipped` by iterating `view.Equipment.Equipped`, not `item.Slot`:

```go
for slot, itemID := range view.Equipment.Equipped {
    cd.Equipped[string(slot)] = &encounterv2pb.Ref{
        Module: refModuleDnd5e, Type: refTypeItem, Id: itemID,
    }
}
```

- [ ] **Step 5: Verify API**

```bash
go test ./internal/handlers/dnd5e/v2/character ./internal/integration/character -count=1
go test -short ./... -count=1
./scripts/verify-release-pin.sh
make pre-commit
```

Expected: PASS with no local toolkit override.

- [ ] **Step 6: Commit and open the API PR**

```bash
git add go.mod go.sum internal/handlers/dnd5e/v2/character \
  internal/integration/character docs/status.md
git commit -m "feat: pass through quantity-aware equipment"
```

### Task 6: Render duplicate selections and remaining carried quantity in web

**Files:**
- Modify: `rpg-dnd5e-web/package.json`
- Modify: `rpg-dnd5e-web/package-lock.json`
- Modify: `rpg-dnd5e-web/src/components/choices/EquipmentBundleChoice.tsx`
- Modify: `rpg-dnd5e-web/src/components/choices/EquipmentBundleChoice.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/game/equipment/equipmentTypes.ts`
- Modify: `rpg-dnd5e-web/src/components/game/equipment/InventoryLight.tsx`
- Modify: `rpg-dnd5e-web/src/components/game/equipment/InventoryLight.test.tsx`
- Modify fixtures using `ItemLike` throughout `src/components/game/**`
- Modify if invalidated: `rpg-dnd5e-web/docs/status.md`

**Interfaces:**
- Consumes: generated `Item.quantity` and `CharacterData.equipped`.
- Produces: repeated draft IDs and a carried row while un-equipped copies remain.

- [ ] **Step 1: Pin the merged proto tag**

Use the exact merged Task 4 release recorded by the coordinator:

```bash
cd rpg-dnd5e-web
test -n "$PROTO_TAG"
npm i --save "github:KirkDiggler/rpg-api-protos#$PROTO_TAG"
```

Commit both package files later; verify the lockfile resolves the new commit. `PROTO_TAG` must be the concrete published tag, never a branch or local path.

- [ ] **Step 2: Reverse duplicate-prevention tests to failing acceptance**

Replace the sibling-disabled test with assertions that both slots can select `CLUB`, and hydration of `['club-selection', 'club-selection']` is complete rather than an alert:

```ts
expect(onSelectionChange).toHaveBeenLastCalledWith(
  'bundle-a',
  new Map([[0, [CLUB, CLUB]]])
);
expect(screen.getByText(/Equipment selection complete/)).toBeTruthy();
```

Add `quantity: 2` to the handaxe/longsword fixture and test one equipped copy leaves a `×1` carried row targeting the empty hand; two equipped copies remove the carried row.

- [ ] **Step 3: Run tests and verify RED**

```bash
npm test -- --run src/components/choices/EquipmentBundleChoice.test.tsx \
  src/components/game/equipment/InventoryLight.test.tsx
```

Expected: duplicate selection is disabled and one equipped ref hides the whole stack.

- [ ] **Step 4: Remove client duplicate blocking**

Delete `disabledOptionIds` and duplicate-alert logic from `CategorySelector`. Preserve ordered slot state and repeated authoritative IDs.

- [ ] **Step 5: Add quantity to ItemLike and compute carried count**

```ts
export interface ItemLike {
  ref: RefLike;
  name: string;
  statLine: string;
  iconKey: string;
  kind: string;
  slotKeys: string[];
  quantity: number;
}
```

In `InventoryLight`, count equipped refs and derive:

```ts
const owned = item.quantity > 0 ? item.quantity : 1; // rollout compatibility only
const carriedCount = owned - (equippedCounts.get(refKey(item.ref)) ?? 0);
```

Render rows only for `carriedCount > 0`, append `×${carriedCount}` when greater than one or when part of a multi-copy stack, and leave the RPC intent unchanged.

- [ ] **Step 6: Update fixtures and verify web**

```bash
npm test -- --run src/components/choices/EquipmentBundleChoice.test.tsx \
  src/components/game/equipment/InventoryLight.test.tsx \
  src/components/game/equipment/EquipmentSlots.test.tsx \
  src/components/game/EncounterView.test.tsx
npm run ci-check
```

Expected: PASS.

- [ ] **Step 7: Commit and open the web PR**

```bash
git add package.json package-lock.json src/components docs/status.md
git commit -m "feat: render quantity-aware equipment stacks"
```

### Task 7: End-to-end acceptance

**Files:**
- No production files unless evidence exposes a defect.

- [ ] **Step 1: Build the exact API and web heads in isolated lab1**

Follow `rpg-project/docs/howto/run-the-game-locally.md`; do not touch the shared primary API.

- [ ] **Step 2: Verify the named route**

Create a Fighter selecting the same martial weapon twice. Confirm owner inventory shows quantity two, first equip leaves one carried copy, second equip fills the other hand, and a third copy cannot be fabricated.

- [ ] **Step 3: Record evidence**

Post exact commits, local route, screenshots/transcript, CI checks, and human verdict on the parent and consumer PRs. Do not merge or close director-owned parent work.
