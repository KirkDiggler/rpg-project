# Production Four-Class Dwarf Hair Customization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let players customize scalp hair, facial hair, shared arbitrary color, and roughness while creating any supported Dwarf class, then render the exact persisted public look for owner and peers in the normal session.

**Architecture:** A neutral proto message carries presence-safe hair semantics through the API-owned draft/final character envelope and public session shelf. The private provider publishes four versioned hairless class bodies, one shared 56-style Dwarf accessory catalog, and deterministic thumbnails; the web consumes an exact generated projection of that manifest, reuses the proven skeleton binder, and provides one creation/session renderer path.

**Tech Stack:** Buf/proto3, generated Go and TypeScript protobuf SDKs, Go 1.25, Redis/miniredis, Python 3.14 stdlib tests, Blender 5.0.1, glTF/GLB, React 19, TypeScript, React Three Fiber/Three.js, Vitest/Testing Library, Playwright browser evidence.

**Spec:** `ideas/characters/customization/production-dwarf-design.md`

## Global Constraints

- Parent Journey: `rpg-project#346`; approved design: `rpg-project#347`; tracking PR: `rpg-project#348`.
- Implementation issues are `rpg-api-protos#262`, `rpg-api#869`, `rpg-game-assets#111`, and `rpg-dnd5e-web#883`.
- Use one branch per repository wave: protos from `origin/main`, provider from `origin/main`, API from `origin/dev`, and web from `origin/dev`.
- Create every implementation checkout as an isolated worktree at execution time; do not modify long-lived workspace checkouts.
- Four Dwarf classes ship together and retain exact outfit meshes: Barbarian 01, Fighter 16, Monk 08, Rogue 10.
- Active Dwarf resolution uses four new versioned hairless bodies; immutable complete Dwarf GLBs remain exact per-class fallbacks and are never modified.
- Publish exactly 38 scalp and 18 facial-hair options under one `modular-fantasy-hero-v1:dwarf` bind profile.
- Slot absence means provider default; explicit none and exact open provider ref remain distinct.
- Shared color is optional RGB24 `0xRRGGBB`; roughness is optional finite `[0,1]`; metalness remains provider-owned at `0.0`.
- Provider defaults are Hair 04, Facial Hair 02, color `0x5A3825`, roughness `0.72`, and metalness `0.0`.
- The API validates shape/ranges but never owns style membership, asset paths, or Dwarf rendering compatibility. Toolkit receives no cosmetic field or logic.
- A persisted style ref is never interpolated into a URL. The web loads only exact manifest-projected paths.
- Licensed source, converted GLBs, and thumbnails remain in private `rpg-game-assets`; synced web binaries stay ignored and untracked.
- Provider merges before web publication. Web evidence must pin the exact provider merge and exact proto dependency.
- Existing non-Dwarf models, animations, proportions, sockets, weapons, downed behavior, and fallbacks remain unchanged.
- Accessory failure leaves the body and valid sibling slot rendered with explicit diagnostics; no source armature or second animation mixer remains mounted.
- Color/roughness changes update instance-owned materials in place. Style replacement keeps the previous valid style until the next style is prepared, then swaps synchronously.
- Creation-only: no finalized-character editor, customization event, roster invalidation, other-race picker, independent slot colors, skin/eye/outfit color, metalness control, surface taxonomy, or runtime outfit assembly.
- Feature PRs receive one fresh independent read-only final review with the current-head verdict published on the PR. Copilot remains disabled.
- Kirk performs provider clipping-sheet review, production picker review, and the final normal four-player session walk before merges that require visual judgment.

## Repository and file map

### `rpg-api-protos#262` — branch `feat/262-hair-customization`

- Create `dnd5e/api/customization/v1alpha1/types.proto`: neutral `StyleSelection` and `HairCustomization` types.
- Modify `dnd5e/api/v1alpha1/character.proto`: reserve retired Appearance fields/names and add `hair = 5`.
- Modify `dnd5e/api/session/v1alpha1/types.proto`: populate the existing public `Customization` shelf.
- Modify `docs/architecture/components/character-service.md`, `session-service.md`, and `shared-types.md`: document ownership and presence semantics.

### `rpg-api#869` — branch `feat/869-hair-customization`

- Modify `go.mod`, `go.sum`: pin the generated proto release.
- Modify `internal/entities/appearance.go`: API-owned typed hair envelope.
- Create `internal/converters/customization/converters.go` and `converters_test.go`: one shared pure character/session translation seam that preserves presence exactly.
- Create `internal/handlers/dnd5e/v1alpha1/character/appearance_validation.go`: wire-shape/range validation.
- Create `internal/handlers/dnd5e/v1alpha1/character/appearance_validation_test.go`: discriminating refusal tests.
- Modify `internal/handlers/dnd5e/v1alpha1/character/converters.go`: delegate Appearance hair conversion to the shared seam.
- Modify `internal/handlers/dnd5e/v1alpha1/character/handler.go` and tests: validate before `SetAppearance`.
- Modify draft/finalization integration tests and Redis repository tests: reload/finalize exactness.
- Modify `internal/handlers/dnd5e/session/v1alpha1/get_roster.go` and tests: public player projection.
- Modify `internal/integration/session/acceptance_test.go`: real repository roster proof.
- Update stale Appearance fixtures/cloners under `internal/sandboxseed/`, equipment tests, `docs/status.md`, `docs/quality.md`, and affected architecture component docs.

### `rpg-game-assets#111` — branch `asset/111-dwarf-customization`

- Create `scripts/configs/character-customization-v1.json`: exact production declarations.
- Create `scripts/character_customization_contract.py`: manifest/config/runtime contract.
- Extract shared Concept GLB checks into `scripts/skinned_accessory_contract.py`; keep the Concept module as a compatibility facade.
- Create `scripts/build_character_customization.py`: four bodies and 56 accessories.
- Create `scripts/render_character_customization.py`: 56 thumbnails and four-outfit evidence.
- Create `scripts/promote_character_customization.py`: deterministic stage/apply/check, manifest, inventory, and rollback.
- Create focused `scripts/test_character_customization_contract.py`, `test_build_character_customization.py`, `test_render_character_customization.py`, and `test_promote_character_customization.py`.
- Generate private outputs under `harness/models/synty/characters/customization/dwarf-v1/`, evidence under `evidence/111-dwarf-customization/`, and refresh complete inventory/mesh stats.
- Modify `README.md` with the production contract and commands.

### `rpg-dnd5e-web#883` — branch `feat/883-dwarf-customization`

- Modify `package.json`, `package-lock.json`: exact generated proto pin.
- Modify asset sync scripts/tests and create `scripts/generateDwarfCustomizationCatalog.ts`: exact license-safe manifest projection.
- Generate `src/generated/dwarfCustomizationCatalog.ts`: synchronous provider-derived body/style/thumbnail/default metadata.
- Create `src/character/customization/hairCustomization.ts` and tests: proto/domain normalization and exact resolution.
- Promote/refine `runtimeSurfaceTreatment`, `skinnedAccessory`, and `SkinnedAccessoryAttachment`; add staged replacement and entity-state material tests.
- Modify `classCharacterModels.ts`, `ClassCharacterModel.tsx`, and tests: modular Dwarf bodies, immutable exact fallbacks, and accessories.
- Replace `AppearanceSelectionModal.tsx`; create focused thumbnail-grid/control/preview components and tests.
- Modify `CharacterDraftContext.tsx`, its definition/tests, and `InteractiveCharacterSheet.tsx/tests`: response-authoritative Apply/Cancel.
- Remove dead legacy appearance editor/preset/hook/section code and adapt readonly character-sheet appearance display.
- Modify `HexEntity`, `SessionCanvas`, `SessionEncounterView`, and tests: one owner/peer customization render path.
- Create `scripts/dwarfCustomizationPublication.test.ts` and `docs/evidence/883-dwarf-customization/`: exact publication receipt and screenshots.
- Update `docs/architecture/components/hex-grid.md`, character-creation documentation, and `docs/status.md`/`docs/quality.md` when their claims change.

---

### Task 1: Publish the neutral shared proto contract (`rpg-api-protos#262`)

**Files:**
- Create: `dnd5e/api/customization/v1alpha1/types.proto`
- Modify: `dnd5e/api/v1alpha1/character.proto:imports, Appearance`
- Modify: `dnd5e/api/session/v1alpha1/types.proto:imports, Customization`
- Modify: `docs/architecture/components/character-service.md`
- Modify: `docs/architecture/components/session-service.md`
- Modify: `docs/architecture/components/shared-types.md`

**Interfaces:**
- Produces: `dnd5e.api.customization.v1alpha1.StyleSelection`
- Produces: `dnd5e.api.customization.v1alpha1.HairCustomization`
- Produces: `dnd5e.api.v1alpha1.Appearance.hair` at field 5
- Produces: `dnd5e.api.session.v1alpha1.Customization.hair` at field 1
- Consumed by: API Task 2 and web Task 7

- [ ] **Step 1: Create the issue worktree from the exact base**

```bash
git -C /home/kirk/game-dev/rpg-api-protos fetch origin
git -C /home/kirk/game-dev/rpg-api-protos worktree add \
  -b feat/262-hair-customization \
  /home/kirk/.pi/worktrees/rpg-api-protos/262-hair-customization origin/main
```

Expected: clean branch based on the current `origin/main`, never `generated`.

- [ ] **Step 2: Add the neutral customization schema**

Create `dnd5e/api/customization/v1alpha1/types.proto` with this semantic shape and explanatory comments:

```proto
syntax = "proto3";

package dnd5e.api.customization.v1alpha1;

import "google/protobuf/empty.proto";

option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/customization/v1alpha1;customizationv1alpha1";
option java_multiple_files = true;
option java_package = "com.kirkdiggler.rpg.api.dnd5e.customization.v1alpha1";

message StyleSelection {
  oneof selection {
    string style_ref = 1;
    google.protobuf.Empty none = 2;
  }
}

message HairCustomization {
  StyleSelection scalp = 1;
  StyleSelection facial_hair = 2;
  optional uint32 color_srgb = 3;
  optional float roughness = 4;
}
```

Comments must state: absent selection message means provider default; `style_ref` is opaque and never a path; `none` is explicit; RGB is `0xRRGGBB`; roughness is finite `[0,1]`; field absence uses provider defaults.

- [ ] **Step 3: Replace the dead character Appearance fields without reusing wire identities**

Import the neutral type and change `Appearance` to:

```proto
message Appearance {
  reserved 1 to 4;
  reserved "skin_tone", "primary_color", "secondary_color", "eye_color";

  dnd5e.api.customization.v1alpha1.HairCustomization hair = 5;
}
```

Do not rename `Appearance`, change its enclosing character/draft field numbers, or add catalog/profile paths.

- [ ] **Step 4: Populate the public session shelf with the same semantic type**

Import the neutral file and change only the empty shelf:

```proto
message Customization {
  dnd5e.api.customization.v1alpha1.HairCustomization hair = 1;
}
```

Update comments to preserve the public/private ruling: customization is public identity, not private sheet/rules data.

- [ ] **Step 5: Format and inspect the intentional break**

```bash
cd /home/kirk/.pi/worktrees/rpg-api-protos/262-hair-customization
buf format -w --disable-symlinks
buf lint --disable-symlinks
buf format --diff --exit-code --disable-symlinks
buf breaking --disable-symlinks \
  --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main' || true
```

Expected: lint/format pass. Breaking reports only the explicitly approved removal of Appearance fields 1–4; no unrelated break is accepted.

- [ ] **Step 6: Generate and compile both SDKs without adding generated files**

```bash
make generate
make compile-go
make compile-ts
git status --short
```

Expected: generated Go and TypeScript compile; only source/docs files are staged for commit because `gen/` is generated/ignored.

- [ ] **Step 7: Update contract documentation**

Document the neutral package, presence table, public roster projection, API-vs-provider ownership, and why the old field numbers/names are reserved. Do not claim runtime/API implementation has landed.

- [ ] **Step 8: Commit, push, and open the intentional-breaking PR**

```bash
git add dnd5e/api/customization/v1alpha1/types.proto \
  dnd5e/api/v1alpha1/character.proto \
  dnd5e/api/session/v1alpha1/types.proto \
  docs/architecture/components/character-service.md \
  docs/architecture/components/session-service.md \
  docs/architecture/components/shared-types.md
git commit -m 'feat: define shared hair customization contract (#262)'
git push -u origin feat/262-hair-customization
gh pr create --repo KirkDiggler/rpg-api-protos --base main \
  --head feat/262-hair-customization \
  --title 'feat: define shared hair customization contract (#262)' \
  --body-file /tmp/rpg-api-protos-262-pr.md
gh pr edit --repo KirkDiggler/rpg-api-protos \
  "$(gh pr view --repo KirkDiggler/rpg-api-protos feat/262-hair-customization --json number --jq .number)" \
  --add-label breaking-change-approved
```

PR body must name the approved in-place pre-alpha removal, reserved identities, generated compile evidence, parent Journey #346, and signature `— platform agent, on behalf of KirkDiggler`.

- [ ] **Step 9: Review, merge, and record the exact generated release**

Run one fresh independent read-only review, publish its current-head verdict, have Kirk merge, then wait for generated branch/tag automation. Record:

```bash
PROTO_MERGE=$(gh pr view --repo KirkDiggler/rpg-api-protos \
  feat/262-hair-customization --json mergeCommit --jq .mergeCommit.oid)
git fetch origin generated --tags
PROTO_TAG=$(git tag --merged origin/generated --sort=-v:refname | head -1)
PROTO_GENERATED_COMMIT=$(git rev-list -n 1 "$PROTO_TAG")
git merge-base --is-ancestor "$PROTO_MERGE" "$PROTO_GENERATED_COMMIT"
printf 'PROTO_MERGE=%s\nPROTO_TAG=%s\nPROTO_GENERATED_COMMIT=%s\n' \
  "$PROTO_MERGE" "$PROTO_TAG" "$PROTO_GENERATED_COMMIT"
```

Verify the release/tag contains `HairCustomization` before moving consumers. Close #262 and mark it Done only after this readback.

---

### Task 2: Add API entities, converters, and validation (`rpg-api#869`)

**Files:**
- Modify: `go.mod`, `go.sum`
- Modify: `internal/entities/appearance.go`
- Create: `internal/converters/customization/converters.go`
- Create: `internal/converters/customization/converters_test.go`
- Create: `internal/handlers/dnd5e/v1alpha1/character/appearance_validation.go`
- Create: `internal/handlers/dnd5e/v1alpha1/character/appearance_validation_test.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/converters.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/handler.go`
- Modify: `internal/handlers/dnd5e/v1alpha1/character/handler_test.go`

**Interfaces:**
- Consumes: released `customizationv1alpha1.HairCustomization`
- Produces: `entities.Appearance{Hair *HairCustomization}` with exact optional/presence data
- Produces: `customizationconverter.ProtoToEntity(*customizationpb.HairCustomization) *entities.HairCustomization`
- Produces: `customizationconverter.EntityToProto(*entities.HairCustomization) *customizationpb.HairCustomization`
- Produces: `validateAppearance(*dnd5ev1alpha1.Appearance) error`
- Consumed by: API Task 3 repository/finalization/roster work

- [ ] **Step 1: Create the API worktree and pin the generated proto**

```bash
git -C /home/kirk/game-dev/rpg-api fetch origin
git -C /home/kirk/game-dev/rpg-api worktree add \
  -b feat/869-hair-customization \
  /home/kirk/.pi/worktrees/rpg-api/869-hair-customization origin/dev
cd /home/kirk/.pi/worktrees/rpg-api/869-hair-customization
GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated
go mod tidy
go list -m github.com/KirkDiggler/rpg-api-protos/gen/go
```

Expected: the printed pseudo-version resolves to the generated commit descended from the recorded proto merge and exposes the neutral package.

- [ ] **Step 2: Write failing entity/converter tests for all presence states**

Use exact fixtures for:

```go
style := &customizationpb.StyleSelection{
    Selection: &customizationpb.StyleSelection_StyleRef{
        StyleRef: "modular-fantasy-hero:hair:38",
    },
}
none := &customizationpb.StyleSelection{
    Selection: &customizationpb.StyleSelection_None{None: &emptypb.Empty{}},
}
hair := &customizationpb.HairCustomization{
    Scalp: style,
    FacialHair: none,
    ColorSrgb: proto.Uint32(0x5A3825),
    Roughness: proto.Float32(0.72),
}
```

Assert nil selection messages remain nil/default, explicit none survives both conversions, exact refs remain byte-for-byte, and optional zero values (`color_srgb=0`, `roughness=0`) remain present rather than collapsing to absence.

- [ ] **Step 3: Run focused tests and observe the old fields fail compilation**

```bash
go test ./internal/handlers/dnd5e/v1alpha1/character \
  -run 'Appearance|UpdateAppearance' -count=1
```

Expected: FAIL because generated legacy color fields no longer exist and typed hair entities/converters are not implemented.

- [ ] **Step 4: Replace the API-owned Appearance entity**

Use JSON-safe explicit kind data:

```go
type StyleSelectionKind string

const (
    StyleSelectionKindStyle StyleSelectionKind = "style"
    StyleSelectionKindNone  StyleSelectionKind = "none"
)

type StyleSelection struct {
    Kind     StyleSelectionKind `json:"kind"`
    StyleRef string             `json:"style_ref,omitempty"`
}

type HairCustomization struct {
    Scalp       *StyleSelection `json:"scalp,omitempty"`
    FacialHair  *StyleSelection `json:"facial_hair,omitempty"`
    ColorSRGB   *uint32         `json:"color_srgb,omitempty"`
    Roughness   *float32        `json:"roughness,omitempty"`
}

type Appearance struct {
    Hair *HairCustomization `json:"hair,omitempty"`
}
```

A nil slot pointer is provider default. `KindNone` is explicit none. A style kind requires an exact non-empty `StyleRef`.

- [ ] **Step 5: Implement exact shared bidirectional conversion**

Create package `internal/converters/customization` with exported `ProtoToEntity` and `EntityToProto`; keep slot-selection helpers private to that package. Character Appearance conversion delegates to it, and Task 3's session projection imports the same package.

Use `proto.Uint32` and `proto.Float32` copies for optional scalar presence. Never map unknown/empty oneof state to default silently; validation rejects it before conversion. Tests pin nil/default, explicit none, exact ref, present zero, and non-aliasing pointer copies.

- [ ] **Step 6: Write failing request-validation tests**

Cover one valid full request and these refusals before any mock `SetAppearance` call:

- present `StyleSelection` with no oneof;
- empty style ref;
- style ref longer than 256 UTF-8 bytes;
- color `0x1000000`;
- roughness `-0.0001`, `1.0001`, NaN, positive infinity, and negative infinity;
- nil request, empty draft id, and nil Appearance.

Assert gRPC `codes.InvalidArgument`, stable field-specific messages, and zero service calls.

- [ ] **Step 7: Implement shape/range validation at the handler boundary**

Use constants and finite checks:

```go
const maxStyleRefBytes = 256

func validateAppearance(appearance *dnd5ev1alpha1.Appearance) error
func validateHairCustomization(hair *customizationpb.HairCustomization) error
func validateStyleSelection(field string, selection *customizationpb.StyleSelection) error
```

`nil` Hair and nil slot selections are valid defaults. Check `math.IsNaN(float64(value))` and `math.IsInf(float64(value), 0)` before the normalized range. Validation must not import an asset catalog or check Dwarf/style membership.

Call `validateAppearance(req.Appearance)` before conversion and before `SetAppearance`.

- [ ] **Step 8: Run focused tests and commit the API contract core**

```bash
gofmt -w internal/entities/appearance.go \
  internal/converters/customization \
  internal/handlers/dnd5e/v1alpha1/character/appearance_*.go \
  internal/handlers/dnd5e/v1alpha1/character/converters.go \
  internal/handlers/dnd5e/v1alpha1/character/handler.go
go test ./internal/converters/customization \
  ./internal/handlers/dnd5e/v1alpha1/character \
  -run 'Appearance|UpdateAppearance|Customization' -count=1
git add go.mod go.sum internal/entities/appearance.go \
  internal/converters/customization \
  internal/handlers/dnd5e/v1alpha1/character
git commit -m 'feat: validate typed hair customization (#869)'
```

Expected: focused tests pass and no legacy Appearance generated field remains referenced in the package.

---

### Task 3: Prove API persistence, finalization, and public roster projection

**Files:**
- Create: `internal/repositories/character_draft/redis_appearance_test.go`
- Modify: `internal/repositories/character/redis_equipment_test.go`
- Modify: `internal/orchestrators/session/character_repo_test.go`
- Modify: character Get/List handler tests
- Modify: `internal/integration/character/creation_test.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/get_roster.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/get_roster_test.go`
- Modify: `internal/integration/session/acceptance_test.go`
- Modify: stale Appearance fixtures/cloners under `internal/sandboxseed/` and character/equipment tests
- Modify: `docs/architecture/components/character-handler.md`
- Modify: `docs/architecture/components/character-orchestrator.md`
- Modify: `docs/architecture/components/entities.md`
- Modify: `docs/architecture/components/session-presentation.md`
- Modify: `docs/status.md`, `docs/quality.md`

**Interfaces:**
- Consumes: typed entity/converter/validator from Task 2
- Produces: exact draft → final character persistence and `PublicMemberInfo.customization.hair`
- Consumed by: web Task 9 normal session path

- [ ] **Step 1: Write a failing real Redis draft round-trip test**

Create a draft through `characterdraft.NewRedis`, update it with:

```go
color := uint32(0x123456)
roughness := float32(0.33)
appearance := &entities.Appearance{Hair: &entities.HairCustomization{
    Scalp: &entities.StyleSelection{
        Kind: entities.StyleSelectionKindStyle,
        StyleRef: "modular-fantasy-hero:hair:38",
    },
    FacialHair: &entities.StyleSelection{Kind: entities.StyleSelectionKindNone},
    ColorSRGB: &color,
    Roughness: &roughness,
}}
```

Read through the repository again and assert deep equality plus non-aliased pointer identities. Include a second case proving present zero color/roughness survive JSON.

- [ ] **Step 2: Write a failing end-to-end character creation test**

In `internal/integration/character/creation_test.go`, complete one Dwarf Fighter draft, call `UpdateAppearance`, `GetDraft`, and `FinalizeDraft`, then assert the exact hair payload on all three responses and on `GetCharacter`. Use explicit none in one slot and style ref in the other.

Add an invalid roughness case that calls `GetDraft` afterward and proves repository state did not change.

- [ ] **Step 3: Run repository/integration tests and observe missing behavior**

```bash
go test ./internal/repositories/character_draft \
  ./internal/integration/character -run 'Appearance|HairCustomization' -count=1
```

Expected: FAIL until stale fixtures are updated and presence conversion/finalization is complete.

- [ ] **Step 4: Repair every API-owned envelope copy without adding toolkit fields**

Keep finalization's existing copy:

```go
Appearance: getOutput.Draft.Appearance,
```

Update test fixtures, sandbox clone helpers, character Get/List tests, equipment tests, and `internal/orchestrators/session/character_repo_test.go` to use typed hair data. Assert Get/List responses preserve exact values and session SDK saves preserve the API-owned envelope. Ensure cloning code deep-copies optional scalar pointers and style selections so a clone mutation cannot alter its source. Do not add Appearance to toolkit `character.Data`.

- [ ] **Step 5: Write the failing public roster projection tests**

Extend player character fixtures to carry `*entities.Appearance`. Assert:

```go
alice.GetCustomization().GetHair().GetScalp().GetStyleRef()
// == "modular-fantasy-hero:hair:38"
alice.GetCustomization().GetHair().GetFacialHair().GetNone()
// non-nil google.protobuf.Empty
alice.GetCustomization().GetHair().GetColorSrgb()
// == 0x123456
```

Also assert default/nil player Appearance yields an always-present empty `Customization`, and monster shelves remain always-present and empty.

- [ ] **Step 6: Implement roster projection with the shared converter**

Import `internal/converters/customization` from both handlers. For player rows only:

```go
customization := &sessionpb.Customization{}
if got.Character.Appearance != nil && got.Character.Appearance.Hair != nil {
    customization.Hair = customizationconverter.EntityToProto(
        got.Character.Appearance.Hair,
    )
}
```

Do not duplicate conversion logic or import one handler package from another.

- [ ] **Step 7: Prove the real roster repository path**

Update `TestGetRoster_ServesTheLaunchWrittenRow` to store a character with hair Appearance and assert the exact public projection. Preserve seated authorization, class/race freshness, monster ref/name, and absence of private fields.

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1 \
  ./internal/integration/session -run 'GetRoster' -count=1
```

Expected: PASS.

- [ ] **Step 8: Run complete API gates and update living docs**

```bash
make ci-check
make pre-commit
git diff --check
```

Update docs to say Appearance is API-owned hair data, `UpdateAppearance` is creation-only, and roster customization is populated for players. Remove claims that the shelf is empty.

- [ ] **Step 9: Commit, push, review, and merge the API slice**

```bash
git add .
git commit -m 'feat: persist and project hair customization (#869)'
git push -u origin feat/869-hair-customization
gh pr create --repo KirkDiggler/rpg-api --base dev \
  --head feat/869-hair-customization \
  --title 'feat: persist and project hair customization (#869)' \
  --body-file /tmp/rpg-api-869-pr.md
```

Run one independent current-head review, publish the verdict, address every finding on the same branch, rerun `make ci-check`, and have Kirk squash-merge into `dev`. Record the exact merge, close #869, and mark it Done.

---

### Task 4: Define the complete production provider contract (`rpg-game-assets#111`)

**Files:**
- Create: `scripts/configs/character-customization-v1.json`
- Create: `scripts/skinned_accessory_contract.py`
- Modify: `scripts/character_customization_concept_contract.py`
- Create: `scripts/character_customization_contract.py`
- Create: `scripts/test_character_customization_contract.py`
- Modify: existing Concept tests only where the shared extraction changes imports

**Interfaces:**
- Consumes: exact Concept provider contract and current modular Dwarf recipes
- Produces: validated production config/manifest model for Tasks 5–6
- Consumed by: provider builder/promoter and web generated catalog

- [ ] **Step 1: Create the provider worktree from the exact merged Concept authority**

```bash
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add \
  -b asset/111-dwarf-customization \
  /home/kirk/.pi/worktrees/rpg-game-assets/111-dwarf-customization origin/main
```

Verify `origin/main` contains provider merge `4c208fad5a950d2103d763a9c8aac96d3bb342b1`.

- [ ] **Step 2: Write failing config-contract tests**

The tests must require:

```python
expected_classes = {
    "dwarf:barbarian": "01",
    "dwarf:fighter": "16",
    "dwarf:monk": "08",
    "dwarf:rogue": "10",
}
expected_scalp_refs = [f"modular-fantasy-hero:hair:{i:02d}" for i in range(1, 39)]
expected_facial_refs = [f"modular-fantasy-hero:facial-hair:{i:02d}" for i in range(1, 19)]
```

Assert unique refs/labels/outputs/thumbnails, safe relative output paths, exact source meshes, immutable fallback paths/hashes, four exact class recipes, defaults, finite PBR values, and no output collision with existing files.

- [ ] **Step 3: Run the focused test and observe missing production modules**

```bash
cd /home/kirk/.pi/worktrees/rpg-game-assets/111-dwarf-customization
python3 -m unittest scripts.test_character_customization_contract -v
```

Expected: FAIL because the production config/contract does not exist.

- [ ] **Step 4: Extract reusable GLB validation without weakening the Concept**

Move finite accessor, JOINTS/WEIGHTS, one-mesh, skin, inverse-bind, material, texture, and animation checks into `scripts/skinned_accessory_contract.py`. Keep `character_customization_concept_contract.py` importing/re-exporting the same public names so existing Concept scripts/tests and exact fixture behavior remain green.

```bash
python3 -m unittest \
  scripts.test_character_customization_concept_contract \
  scripts.test_build_character_customization_concept \
  scripts.test_promote_character_customization_concept -v
```

Expected: all existing Concept tests still pass unchanged semantically.

- [ ] **Step 5: Create the exact production declaration**

Use output root:

```text
harness/models/synty/characters/customization/dwarf-v1/
```

Declare four versioned body outputs, old complete Dwarf fallbacks with current hashes, 38 scalp options, 18 facial options, one thumbnail per option, style refs, labels, source mesh names, defaults, `uniform-pbr-v1`, and `modular-fantasy-hero-v1:dwarf` profile identity. Generate option declarations programmatically only in code; the checked-in config remains explicit and reviewable.

- [ ] **Step 6: Implement strict config/manifest validation**

Expose focused interfaces:

Implement these exact callable interfaces:

- `load_config(path: Path) -> dict[str, object]`
- `validate_config(config: Mapping[str, object], repo_root: Path) -> None`
- `validate_manifest(manifest: Mapping[str, object], runtime_root: Path) -> None`
- `expected_runtime_paths(config: Mapping[str, object]) -> tuple[Path, ...]`

Reject absolute paths, `..`, symlinks, duplicate refs/paths, non-finite values, missing exact source/archive/atlas identities, wrong defaults, unknown classes, body/accessory shape drift, and undeclared runtime files.

- [ ] **Step 7: Run focused and full tests, then commit the provider contract**

```bash
python3 -m unittest scripts.test_character_customization_contract -v
python3 -m unittest discover -s scripts -p 'test_*.py'
git add scripts/configs/character-customization-v1.json \
  scripts/skinned_accessory_contract.py \
  scripts/character_customization_concept_contract.py \
  scripts/character_customization_contract.py \
  scripts/test_character_customization_contract.py \
  scripts/test_character_customization_concept_contract.py \
  scripts/test_build_character_customization_concept.py \
  scripts/test_promote_character_customization_concept.py
git commit -m 'test: define production Dwarf customization contract (#111)'
```

---

### Task 5: Build and atomically promote four bodies plus 56 accessories

**Files:**
- Create: `scripts/build_character_customization.py`
- Create: `scripts/test_build_character_customization.py`
- Create: `scripts/promote_character_customization.py`
- Create: `scripts/test_promote_character_customization.py`
- Modify: `scripts/build_synty_complete_inventory.py` only if the new declared tree requires an allowlist/category update
- Modify: `scripts/build_mesh_stats.py` only if the new asset class requires explicit budgets
- Generate: `harness/models/synty/characters/customization/dwarf-v1/**/*.glb`
- Generate: `harness/models/synty/characters/customization/dwarf-v1/manifest.json`
- Generate: complete inventory and mesh stats

**Interfaces:**
- Consumes: validated production config from Task 4
- Produces: four hairless body GLBs, 56 accessory GLBs, and exact manifest/hash tree
- Consumed by: thumbnail/evidence Task 6 and web Task 7

- [ ] **Step 1: Write failing builder tests around observable commands and exact recipes**

Use fake Blender subprocesses to assert:

- each body recipe is its current modular race/class recipe minus exactly `Chr_Hair_04` and `Chr_FacialHair_Male_02`;
- no class outfit mesh crosses into another body;
- scalp source names are exactly `Chr_Hair_01..38`;
- facial source names are exactly `Chr_FacialHair_Male_01..18`;
- each invocation receives hash-pinned source archive/atlas inputs and a lexical `.stage/<name>` output;
- partial output, unexpected output, and nonzero Blender exits fail without touching canonical runtime files.

- [ ] **Step 2: Implement the deterministic builder by generalizing the accepted Concept path**

Expose:

Implement these exact callable interfaces:

- `build_body(config: Mapping[str, object], combination: str, output: Path) -> None`
- `build_accessory(config: Mapping[str, object], slot: str, style_ref: str, output: Path) -> None`
- `build_all(config_path: Path, stage_root: Path) -> dict[str, object]`

Reuse existing modular builder functions for head/ear/outfit/proportion/animation/arm correction/socket semantics. Accessories retain one mesh, zero clips, one opaque untextured uniform-PBR material, and the source rig only as export skin data.

- [ ] **Step 3: Run builder tests before real Blender work**

```bash
python3 -m unittest scripts.test_build_character_customization -v
```

Expected: PASS with fake subprocesses and exact 60-output declaration.

- [ ] **Step 4: Write failing promotion tests for atomicity and byte equality**

Cover:

- stage validation before canonical mutation;
- every GLB structurally decoded;
- 224 body/accessory inverse-bind compatibility checks;
- manifest bytes equal canonical report bytes;
- exact runtime tree with no extras/missing files;
- apply rollback after injected copy/validation failure;
- clean `--check` rebuild byte-equals canonical runtime;
- all pre-existing file hashes remain unchanged;
- complete inventory and mesh stats include every new GLB.

- [ ] **Step 5: Implement stage/apply/check promotion**

The command surface is:

```bash
python3 scripts/promote_character_customization.py --dry-run
python3 scripts/promote_character_customization.py
python3 scripts/promote_character_customization.py --check
```

Use fresh sibling staging, lexical containment checks, fsync/atomic rename where the existing promoter does, deterministic JSON (`sort_keys=True`, fixed separators, terminal newline), and exact inventory regeneration. A failed stage/apply/check restores the prior canonical tree byte-for-byte.

- [ ] **Step 6: Run a Dwarf dry run, then the real build**

```bash
python3 scripts/promote_character_customization.py --dry-run
python3 scripts/promote_character_customization.py
python3 scripts/promote_character_customization.py --check
```

Expected: exactly four bodies and 56 accessories are promoted; manifest/default/fallback hashes match actual bytes; no previous provider file changes.

- [ ] **Step 7: Run structural/full suites and commit runtime provider bytes**

```bash
python3 -m unittest \
  scripts.test_character_customization_contract \
  scripts.test_build_character_customization \
  scripts.test_promote_character_customization -v
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
git add scripts harness/models/synty/characters/customization/dwarf-v1 \
  harness/catalogs/synty-complete-inventory.json \
  harness/models/synty/mesh-stats.json
git commit -m 'feat: build production Dwarf customization assets (#111)'
```

Before commit, verify `git status --short` contains no raw FBX, source atlas, source zip, `.blend`, `.stage`, cache, or unrelated generated output.

---

### Task 6: Publish thumbnails, clipping evidence, and exact provider authority

**Files:**
- Create: `scripts/render_character_customization.py`
- Create: `scripts/test_render_character_customization.py`
- Generate: 56 deterministic thumbnails under the production runtime tree
- Generate: `evidence/111-dwarf-customization/README.md`
- Generate: `evidence/111-dwarf-customization/verification.json`
- Generate: four-class scalp/facial front/profile sheets under evidence
- Modify: production manifest/inventory with exact thumbnail bytes/hashes
- Modify: `README.md`

**Interfaces:**
- Consumes: exact promoted GLBs from Task 5
- Produces: complete provider merge consumed exactly by web Task 7

- [ ] **Step 1: Write failing renderer tests before rendering**

Pin deterministic scene settings, camera, Dwarf head/material witness, image dimensions, alpha/background policy, class/outfit labels, option ordering, and portable receipt schema. Require 56 unique nonblank thumbnails and 16 contact sheets:

```text
4 classes × (scalp-front, scalp-profile, facial-front, facial-profile)
```

Tests reject timestamps, absolute paths, branch names, missing labels, blank images, nondeterministic ordering, or a receipt that does not bind exact manifest/GLB/image hashes.

- [ ] **Step 2: Implement one deterministic Blender render path**

Use one scripted scene recipe for both thumbnail tiles and evidence sheets. Thumbnails show the approved neutral Dwarf head with exact style and neutral default material; contact sheets use each exact class outfit. Set the other slot to None so each category is independently readable.

- [ ] **Step 3: Render twice and prove byte identity**

```bash
python3 scripts/render_character_customization.py \
  --output-root .stage/dwarf-customization-render-a
python3 scripts/render_character_customization.py \
  --output-root .stage/dwarf-customization-render-b
(cd .stage/dwarf-customization-render-a && \
  find . -type f -print0 | sort -z | xargs -0 sha256sum) \
  > /tmp/dwarf-customization-render-a.sha
(cd .stage/dwarf-customization-render-b && \
  find . -type f -print0 | sort -z | xargs -0 sha256sum) \
  > /tmp/dwarf-customization-render-b.sha
```

Compare relative-path/hash maps, not absolute filenames. Expected: byte-identical output sets.

- [ ] **Step 4: Promote thumbnails and regenerate the exact manifest atomically**

Integrate the renderer into `promote_character_customization.py` so GLBs, thumbnails, manifest, complete inventory, and receipt are one atomic contract. `--check` must rebuild and byte-compare all of them.

- [ ] **Step 5: Generate four-class visual evidence and obtain Kirk's verdict**

Open the 16 sheets. Kirk checks collars, shoulders, ears, scalp profile, facial/chest overlap, and class identity. Record accepted findings or exact rejected refs in the config/manifest; do not silently transform individual styles in the web.

If any source style is rejected for a specific class, stop and return to design because the approved contract says one shared 56-style Dwarf catalog across all four bodies.

- [ ] **Step 6: Run final provider verification**

```bash
python3 scripts/promote_character_customization.py --check
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
```

Record test totals, manifest hash, inventory hash, inventory-tree hash, four body hashes, style/thumb counts, 224 compatibility count, and all pre-existing hash preservation in `verification.json` and README.

- [ ] **Step 7: Commit evidence/docs, push, review, and merge**

```bash
git add README.md scripts \
  harness/models/synty/characters/customization/dwarf-v1 \
  harness/catalogs/synty-complete-inventory.json \
  harness/models/synty/mesh-stats.json \
  evidence/111-dwarf-customization
git commit -m 'docs: publish Dwarf customization provider evidence (#111)'
git push -u origin asset/111-dwarf-customization
gh pr create --repo KirkDiggler/rpg-game-assets --base main \
  --head asset/111-dwarf-customization \
  --title 'asset: promote production four-class Dwarf customization provider (#111)' \
  --body-file /tmp/rpg-game-assets-111-pr.md
```

Run one independent read-only current-head review, publish its verdict, resolve findings on the same branch, rerun all gates, and have Kirk merge. Record `PROVIDER_MERGE`, close #111, mark Done, and create an exact detached/worktree view at that merge for web sync.

---

### Task 7: Generate the web catalog and productionize staged accessory attachment (`rpg-dnd5e-web#883`)

**Files:**
- Modify: `package.json`, `package-lock.json`
- Modify: `scripts/sync-game-assets.ts`, `scripts/sync-game-assets.test.ts`
- Create: `scripts/generateDwarfCustomizationCatalog.ts`
- Create: `scripts/generateDwarfCustomizationCatalog.test.ts`
- Generate: `src/generated/dwarfCustomizationCatalog.ts`
- Create: `src/character/customization/hairCustomization.ts`
- Create: `src/character/customization/hairCustomization.test.ts`
- Modify: `src/components/hex-grid/runtimeSurfaceTreatment.ts` and tests
- Modify: `src/components/hex-grid/SkinnedAccessoryAttachment.tsx` and tests
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx` and tests
- Modify: `src/components/hex-grid/classCharacterModels.ts` and tests

**Interfaces:**
- Consumes: exact proto release and `PROVIDER_MERGE`
- Produces: `DwarfCustomizationCatalog`, `resolveHairPresentation`, staged slot replacement, and active/fallback Dwarf body resolution
- Consumed by: creation Task 8 and session Task 9

- [ ] **Step 1: Create the web worktree and pin exact providers**

```bash
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  -b feat/883-dwarf-customization \
  /home/kirk/.pi/worktrees/rpg-dnd5e-web/883-dwarf-customization origin/dev
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/883-dwarf-customization
git -C /home/kirk/game-dev/rpg-api-protos fetch origin generated --tags
PROTO_TAG=$(git -C /home/kirk/game-dev/rpg-api-protos \
  tag --merged origin/generated --sort=-v:refname | head -1)
PROVIDER_PR=$(gh api repos/KirkDiggler/rpg-game-assets/issues/111/timeline \
  -H 'Accept: application/vnd.github+json' \
  --jq '[.[] | select(.event == "cross-referenced" and .source.issue.pull_request != null) | .source.issue.number] | last')
PROVIDER_MERGE=$(gh pr view "$PROVIDER_PR" \
  --repo KirkDiggler/rpg-game-assets --json mergeCommit --jq .mergeCommit.oid)
test -n "$PROTO_TAG" && test -n "$PROVIDER_MERGE"
PROVIDER_SHORT=$(printf '%s' "$PROVIDER_MERGE" | cut -c1-8)
PROVIDER_WORKTREE="/home/kirk/.pi/worktrees/rpg-game-assets/provider-$PROVIDER_SHORT"
if [ ! -d "$PROVIDER_WORKTREE" ]; then
  git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach \
    "$PROVIDER_WORKTREE" "$PROVIDER_MERGE"
fi
test "$(git -C "$PROVIDER_WORKTREE" rev-parse HEAD)" = "$PROVIDER_MERGE"
npm i --save "github:KirkDiggler/rpg-api-protos#$PROTO_TAG"
RPG_GAME_ASSETS_PATH="$PROVIDER_WORKTREE" \
  ASSETS_SYNC_SKIP_UPDATE=1 npm run assets:sync
```

Verify `package-lock.json` resolves the generated commit containing HairCustomization and `git status` shows zero tracked files under `public/models/synty/`.

- [ ] **Step 2: Write failing catalog generator tests with hostile fixtures**

Test exact projection of four body/fallback entries, 38 scalp, 18 facial, thumbnails, hashes, defaults, surface facts, and profile identity. Reject duplicate refs, duplicate paths, missing defaults, unsafe paths, absolute/source paths, unknown keys in required objects, malformed hashes, wrong counts, non-finite numbers, and any style ref used as a URL.

The checked-in projection type is:

```ts
export interface DwarfCustomizationCatalog {
  readonly schemaVersion: 1;
  readonly profileRef: 'modular-fantasy-hero-v1:dwarf';
  readonly bodies: Readonly<Record<'barbarian' | 'fighter' | 'monk' | 'rogue', {
    readonly url: string;
    readonly sha256: string;
    readonly fallbackUrl: string;
    readonly fallbackSha256: string;
  }>>;
  readonly slots: {
    readonly scalp: readonly DwarfStyleOption[];
    readonly facialHair: readonly DwarfStyleOption[];
  };
  readonly defaults: {
    readonly scalpStyleRef: string;
    readonly facialHairStyleRef: string;
    readonly colorSrgb: number;
    readonly roughness: number;
    readonly metalness: number;
  };
}
```

- [ ] **Step 3: Implement deterministic provider-to-TypeScript projection**

The generator reads only the exact ignored provider manifest, validates it, emits sorted immutable TypeScript, and includes source manifest SHA-256 plus the source checkout's exact `git rev-parse HEAD` in a header. It omits source archive paths/mesh provenance not needed by the consumer. Wire `sync-game-assets.sh` to invoke it after the private root is copied, passing the explicit provider root and refusing a dirty or non-commit-resolvable source checkout.

```bash
npx vitest run scripts/generateDwarfCustomizationCatalog.test.ts \
  scripts/sync-game-assets.test.ts
```

Expected: PASS and a second generation produces no diff.

- [ ] **Step 4: Write failing pure hair-resolution tests**

Cover proto/domain normalization for absent/default, explicit none, exact ref, present zero color/roughness, unknown ref, unsupported race/class, safe manifest URL resolution, and provider defaults. Require one normalized renderer input for character Appearance and session Customization.

```ts
export type HairSlotSelection =
  | { readonly kind: 'default' }
  | { readonly kind: 'none' }
  | { readonly kind: 'style'; readonly styleRef: string };

export interface ResolvedHairPresentation {
  readonly profileRef: string;
  readonly accessories: readonly SkinnedAccessoryPresentation[];
  readonly diagnostics: readonly HairResolutionDiagnostic[];
}
```

- [ ] **Step 5: Implement pure normalization/resolution without URL interpolation**

Lookup `styleRef` in a prebuilt exact map from the generated catalog. Unknown refs produce `unknown-style-ref` diagnostics and no accessory for that slot. Convert RGB24 with:

```ts
export function rgb24ToHex(value: number): `#${string}` {
  return `#${value.toString(16).padStart(6, '0').toUpperCase()}`;
}
```

Use catalog defaults when optional fields are absent. Do not silently clamp invalid server values; diagnose and use provider default.

- [ ] **Step 6: Write failing staged-replacement and entity-treatment tests**

Extend attachment tests to prove:

- old mesh remains mounted while the next GLB suspends/loads;
- next mesh is prepared and exact-bound before mutation;
- one synchronous commit adds next/removes old with no rendered hairless state;
- rejected next style leaves old valid style mounted and reports requested identity;
- initial rejection leaves body only;
- color/roughness preserve mesh/material UUIDs and emit no loading event;
- StrictMode does not retain probe UUIDs or dispose cached source resources;
- selected, ghost, and remembered material treatments match body semantics; and
- source bones/armature are absent after every successful replacement.

- [ ] **Step 7: Refactor attachment ownership for atomic style swaps**

Keep one slot component owning `activeRef`. Separate preparation from mount:

```ts
interface PreparedSkinnedAccessory {
  readonly mesh: THREE.SkinnedMesh;
  readonly materials: readonly THREE.MeshStandardMaterial[];
  readonly dispose: () => void;
  readonly evidence: SkinnedAccessoryBindingEvidence;
}

function prepareSkinnedAccessory(
  characterRoot: THREE.Object3D,
  sourceScene: THREE.Object3D,
  presentation: SkinnedAccessoryPresentation
): PreparedSkinnedAccessory
```

In a layout effect, prepare first, then `characterRoot.add(next.mesh)`, remove/dispose the previous active mesh, and assign `activeRef` in one synchronous turn. A separate unmount cleanup owns the final active attachment. Treatment-only effects mutate the current materials in place. `ClassCharacterModel` keys the attachment component by slot only—not style ref or URL—so a style prop change cannot unmount the slot owner before preparation.

Factor entity selected/ghost/remembered overlays so body and accessory materials receive equivalent emissive/opacity/crypt treatment without overwriting the persisted hair base color/roughness when returning to live state.

- [ ] **Step 8: Move Dwarf model resolution to generated provider truth**

Update only Dwarf standing entries to generated versioned body paths and exact fallback paths. Keep all non-Dwarf entries byte-for-byte semantically unchanged. Downed resolution continues existing behavior. Extend `PlayerCharacterModelResolution` with optional customization profile/fallback fields rather than teaching generic models about Dwarf constants.

- [ ] **Step 9: Run focused tests and commit the runtime foundation**

```bash
npx vitest run \
  scripts/generateDwarfCustomizationCatalog.test.ts \
  scripts/sync-game-assets.test.ts \
  src/character/customization/hairCustomization.test.ts \
  src/components/hex-grid/skinnedAccessory.test.ts \
  src/components/hex-grid/runtimeSurfaceTreatment.test.ts \
  src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/hex-grid/classCharacterModels.test.ts
git add package.json package-lock.json scripts src/generated \
  src/character/customization src/components/hex-grid
git commit -m 'feat: resolve production Dwarf customization assets (#883)'
```

---

### Task 8: Replace the dead appearance editor with the production Dwarf picker

**Files:**
- Replace: `src/character/creation/AppearanceSelectionModal.tsx`
- Create: `src/character/creation/AppearanceSelectionModal.test.tsx`
- Create: `src/character/creation/components/HairStyleGrid.tsx`
- Create: `src/character/creation/components/HairStyleGrid.test.tsx`
- Create: `src/character/creation/components/DwarfCustomizationPreview.tsx`
- Create: `src/character/creation/components/DwarfCustomizationControls.tsx`
- Modify: `src/character/creation/CharacterDraftContext.tsx`
- Modify: `src/character/creation/CharacterDraftContextDef.tsx`
- Create: `src/character/creation/CharacterDraftContext.test.tsx`
- Modify: `src/character/creation/InteractiveCharacterSheet.tsx` and tests
- Modify: `src/character/sheet/components/DnDAppearance.tsx` and tests
- Delete: `src/components/AppearanceEditor.tsx`, `src/config/appearancePresets.ts`, `src/hooks/useAppearance.ts`, `src/character/creation/sections/AppearanceSection.tsx`

**Interfaces:**
- Consumes: generated catalog/resolver and released `Appearance` proto
- Produces: class-accurate creation modal and one response-authoritative `updateAppearance(appearance: Appearance): Promise<CharacterDraft>`
- Consumed by: finalized character/session verification in Task 9

- [ ] **Step 1: Write failing grid and modal interaction tests**

Assert:

- exactly Default + None + 38 scalp tiles;
- exactly Default + None + 18 facial tiles;
- provider label/thumbnail/selected state and keyboard button semantics;
- failed thumbnail uses a labeled tile;
- Dwarf Barbarian/Fighter/Monk/Rogue each preview their exact body URL;
- class/race unsupported state does not offer the picker;
- arbitrary color and roughness update the live preview;
- Apply sends one Appearance with exact oneof/presence values;
- Cancel sends nothing and reopening restores persisted values; and
- changing only color/roughness causes no accessory loading status.

- [ ] **Step 2: Implement accessible thumbnail grids and controls**

Use semantic buttons with `aria-pressed`, visible focus, labels, lazy thumbnail images, and client-generated Default/None tiles. Do not render 56 WebGL canvases. Keep the actual single R3F preview as visual authority.

- [ ] **Step 3: Rebuild the preview from the real class renderer**

The preview resolves `dwarf + selected class` through the same generated model/catalog functions as the session. It runs `Idle_Relaxed`, supports orbit/zoom, and mounts no weapon unless the existing creation preview intentionally supplies one. Never substitute Fighter for another selected class.

- [ ] **Step 4: Write failing response-authority tests for the draft context**

Mock `updateAppearance` to return an updated draft. Assert local draft changes only to the response after success; a rejected RPC retains the prior draft; an absent draft ID cannot claim persistence; explicit zero color/roughness survives request construction.

- [ ] **Step 5: Implement one whole-appearance Apply path**

Change the context signature to the generated proto type or a focused domain input converted exactly once. Build proto oneofs with `create()` and `EmptySchema`; await the API; replace local draft with `response.draft`; propagate errors. Remove the old optimistic legacy color construction.

- [ ] **Step 6: Replace sheet integration and remove dead legacy UI**

Show the Appearance action only when Dwarf race and a supported class are selected. Replace the old four swatches in `InteractiveCharacterSheet` with a concise persisted hair summary/color swatch. Adapt finalized readonly `DnDAppearance` to show scalp/facial Default/None/style labels, color, and roughness without offering edits.

Use `rg` to prove no production import remains, then delete the old editor, presets, unused hook, and unused section in this task. Do not remove generic MediumHumanoid shader color capabilities; only remove the retired proto-driven UI/data plumbing.

- [ ] **Step 7: Run focused creation/sheet tests and commit**

```bash
npx vitest run \
  src/character/creation/AppearanceSelectionModal.test.tsx \
  src/character/creation/components/HairStyleGrid.test.tsx \
  src/character/creation/InteractiveCharacterSheet.test.tsx \
  src/character/sheet/components/DnDAppearance.test.tsx \
  src/character/customization/hairCustomization.test.ts
rg -n 'skinTone|primaryColor|secondaryColor|eyeColor' \
  src/character src/api src/components/hex-grid/HexEntity.tsx
git add src/character src/components/AppearanceEditor.tsx \
  src/config/appearancePresets.ts src/hooks/useAppearance.ts \
  src/components/hex-grid/HexEntity.tsx
git commit -m 'feat: customize Dwarf hair during creation (#883)'
```

Expected `rg`: no retired proto-field access in creation/API/session paths. Remaining shader/MediumHumanoid internal color vocabulary is allowed.

---

### Task 9: Wire owner/peer session rendering and publish normal-game evidence

**Files:**
- Modify: `src/components/hex-grid/HexEntity.tsx` and tests
- Modify: `src/components/session/SessionCanvas.tsx` and tests
- Modify: `src/components/session/SessionEncounterView.tsx` and tests
- Modify: local character-data transform/hook tests where Appearance enters SessionCanvas
- Create: `scripts/dwarfCustomizationPublication.test.ts`
- Create: `docs/evidence/883-dwarf-customization/README.md`
- Create: `docs/evidence/883-dwarf-customization/receipt.json`
- Create: browser screenshots under the same evidence directory
- Modify: `docs/architecture/components/hex-grid.md`
- Modify: character creation docs, `docs/status.md`, `docs/quality.md`

**Interfaces:**
- Consumes: local character `appearance.hair` and peer `PublicMemberInfo.customization.hair`
- Produces: one `HexEntity` customization prop normalized by the shared resolver
- Produces: exact merged-provider browser publication and normal four-player proof

- [ ] **Step 1: Write failing owner/peer propagation tests**

Pin two paths:

```text
local GetCharacter Appearance.hair
  → SessionEncounterView → SessionCanvas → local HexEntity
peer PublicMemberInfo.customization.hair
  → SessionCanvas roster lookup → peer HexEntity
```

Assert both reach the same resolver input and produce equal accessory presentations for equal payloads. Assert no peer sheet RPC is introduced, monster customization remains ignored, missing roster customization uses provider defaults, and non-Dwarf entities remain fixed-look.

- [ ] **Step 2: Add one typed customization prop through the render stack**

Use the neutral generated `HairCustomization` type. `SessionCanvas` receives local hair explicitly and reads peer hair from the roster row. `HexEntity` combines race/class/hair into `resolveHairPresentation` only for a standing supported Dwarf body and passes accessories/status diagnostics to `ClassCharacterModel`.

Downed models retain existing static fallback behavior. Remembered/ghost treatment applies to accessories consistently with the body.

- [ ] **Step 3: Add body-load fallback and accessory failure tests**

Inject:

- new modular body load failure → exact immutable complete Dwarf class fallback;
- fallback failure → existing generic class/model fallback;
- unknown scalp + valid facial → body and facial remain;
- bind-incompatible next scalp → previous valid scalp remains;
- missing default manifest entry → explicit contract diagnostic, no guessed ref;
- thumbnail failure has no runtime-model impact; and
- two characters with different color/roughness retain disjoint material UUIDs.

- [ ] **Step 4: Build the exact publication receipt test before screenshots**

The test reads `receipt.json` and pins:

- proto release/commit;
- API merge;
- provider merge, manifest SHA, inventory SHA/tree SHA;
- four exact active/fallback body paths/hashes;
- 38 + 18 option refs/paths/hashes and 56 thumbnail hashes;
- all browser-observed asset HTTP statuses;
- all four class outfit/body identities;
- owner/peer normalized customization values;
- 63 mapped bones and zero source armatures for successful slots;
- stable material UUIDs through color/roughness changes;
- style-swap event ordering with no hairless checkpoint;
- main/off-hand identity witnesses;
- screenshot hashes; and
- zero unexpected console/page/request failures.

Run once and observe failure because evidence does not exist.

- [ ] **Step 5: Run focused and full web CI before browser publication**

```bash
npx vitest run \
  src/character/customization \
  src/components/hex-grid \
  src/character/creation \
  src/components/session/SessionCanvas.test.tsx \
  src/components/session/SessionEncounterView.test.tsx \
  scripts/dwarfCustomizationPublication.test.ts
npm run ci-check
git diff --check
```

The publication test remains the only expected failure until receipt/screenshots are generated; all product tests must pass.

- [ ] **Step 6: Verify the normal managed environment**

Run the exact merged API and provider/web candidate stack through the managed development path. Create and finalize four Dwarves with visibly distinct combinations:

```text
Barbarian: Hair 01 + Facial None + 0x8A4B32 + roughness 0.20
Fighter: Provider Default + Provider Default + provider color/roughness
Monk: Scalp None + Facial 18 + 0x202020 + roughness 1.00
Rogue: Hair 38 + Facial 01 + 0xD6B26E + roughness 0.55
```

Reload each draft before finalization. Start one four-player session, verify owner and peer looks, idle, walk, movement, class outfits, representative main/off-hand weapons, and fallback behavior. Do not use direct state injection as final evidence.

- [ ] **Step 7: Capture reproducible browser evidence**

Capture creation grids/previews for all four classes, style replacement during throttled loading, arbitrary color/roughness stability, four-player close/orbit/tactical session views, movement, and weapon witnesses. Log exact runtime diagnostics/material identities and HTTP facts. Regenerate `receipt.json`, then run:

```bash
npx vitest run scripts/dwarfCustomizationPublication.test.ts
npm run ci-check
```

Expected: publication test and full CI pass with no tracked Synty binaries.

- [ ] **Step 8: Update docs and commit exact publication**

Document provider ownership, presence semantics, creation-only boundary, owner/peer data sources, fallbacks, staged swaps, and known deferrals. Then:

```bash
git add package.json package-lock.json scripts src docs
git commit -m 'docs: publish production Dwarf customization evidence (#883)'
git status --short
```

Expected: zero tracked files under `public/models/synty/`.

- [ ] **Step 9: Push, review, human-walk, and merge web**

```bash
git push -u origin feat/883-dwarf-customization
gh pr create --repo KirkDiggler/rpg-dnd5e-web --base dev \
  --head feat/883-dwarf-customization \
  --title 'feat: create and render production Dwarf customization (#883)' \
  --body-file /tmp/rpg-dnd5e-web-883-pr.md
```

Run one fresh independent current-head review and publish the full verdict. Resolve every finding on the same branch, rebind review if the head changes materially, rerun `npm run ci-check` and the browser receipt, then have Kirk perform the final creation/session walk and squash-merge. Record exact head/merge/checks, close #883, and mark Done.

---

### Task 10: Close the canonical production Journey record

**Files:**
- Modify: `ideas/characters/customization/production-dwarf-design.md`
- Modify: `ideas/characters/customization/production-dwarf-plan.md`

**Interfaces:**
- Consumes: exact proto/API/provider/web merges and Kirk's verdicts
- Produces: final canonical implementation record and closure of #347/#346

- [ ] **Step 1: Record exact implementation facts on the tracking branch**

Append exact merge commits, dependency tags, manifest/inventory hashes, body/style/thumbnail hashes or bound receipt references, test totals, independent review verdicts, browser facts, and Kirk's provider/picker/session verdict. Replace plan checkboxes only for work actually verified.

- [ ] **Step 2: Rebase the tracking branch and verify docs**

```bash
cd /home/kirk/.pi/worktrees/rpg-project/347-production-dwarf-customization
git fetch origin
git rebase origin/main
git diff --check origin/main...HEAD
python3 - <<'PY'
from pathlib import Path
root = Path('ideas/characters/customization')
for name in ('production-dwarf-design.md', 'production-dwarf-plan.md'):
    text = (root / name).read_text()
    for marker in ('T' + 'BD', 'T' + 'ODO', 'FIX' + 'ME', 'PLACE' + 'HOLDER'):
        assert marker not in text, (name, marker)
print('production Dwarf customization docs: OK')
PY
```

- [ ] **Step 3: Commit and push the final record**

```bash
git add ideas/characters/customization/production-dwarf-design.md \
  ideas/characters/customization/production-dwarf-plan.md
git commit -m 'docs: record production Dwarf customization verdict (#347)'
git push --force-with-lease origin design/347-production-dwarf-customization
```

Publish a PR #348 comment with current head, exact provider/consumer merges, checks, review verdicts, human verdict, boundaries, and Assets signature.

- [ ] **Step 4: Merge design record and close hierarchy**

Have Kirk merge PR #348 only after every implementation PR has merged. Verify exact design merge, close Decide #347 and Journey #346, mark them Done on Project 19, and confirm all four implementation children are closed/Done. Preserve the unrelated weapon active lane and remove only customization worktrees/temporary servers after branch safety is verified.
