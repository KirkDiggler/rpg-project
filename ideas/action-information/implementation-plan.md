# Action information — implementation plan

Tracking: [project#543](https://github.com/KirkDiggler/rpg-project/issues/543). Authority: operator agreement recorded there, [law](design.md), [walkthrough](README.md).

## Goal, constraints and baseline

Explain the current gameplay offer before commitment: description and base facts above its existing contextual effects. Include nested option explanations. Preserve action authority, selectors, targets, effects semantics, accessible reading and mobile. Do not add gameplay, item-use offers, totals, predicted rolls or a browser rulebook. No merge/deployment is authorized.

Inspected baselines: web `e22ec3c0`; protos `ee88311`; API `ed4db4f6`; toolkit fetched main `7472c728`; project `2f799c7`. Worktrees: `.worktrees/action-information` in each repository. Toolkit dependent modules require separate release-unit branches/worktrees when their tasks begin; no PR may mix nearest-go.mod modules.

Measured facts:

- `DesktopActionSurface.tsx` renders `EffectRows` after `buildActionTooltip(...).lines`; `DesktopEffects.tsx` uses the armed declaration. The screenshot alone does not demonstrate lost effect rows.
- `session/effects.go:attachEffects` asks only compiled Attack and spell-attack profiles. A saving-throw Bane offer has no such rows. Preserve that scope.
- `spells/data.go` already explains Bane. Ability/feature objects carry descriptions, but `character.AvailableAbility` drops them. `Definition`, `SpellRef` and `AbilityRef` currently do not deliver base descriptions to the live UI. `CastOption` is ID/label only.
- `weaponattack.Assemble` already produces grip-specific damage pools and ability evidence. Positive off-hand base modifiers are omitted in `resolution/strike.go:rollDamage`; do not implement a second version in a renderer.
- `session/declaration_id.go` marshals action definitions into selector material. New prose must be removed from the selector projection, including nested option descriptions.
- Protos publishes generated bindings only through merge CI. Local generated SDK overrides are not the development path. Consumer integration must wait for the operator-authorized contract merge/publication; toolkit modules can develop on pushed pseudo-versions.

## Sequence and seams

Development: T1 fixture-driven reusable renderer → T2 contract → T3 root provider → T4 resolution → T5 session → T6 API → T7 joined web and live walk. T3 can proceed while SDK publication is pending. T4/T5 consume pushed provider commits, not sibling source directories. T2 publishes early under the proto exception, with explicit operator merge authorization. After the integrated walk, release root → resolution → session → API; adopt real tags before consumer merges. Web adopts the actual published proto version and merges only after the API can supply its facts.

No new persistence domain or stream event is needed. Descriptions copied into frozen option/offer payloads must survive existing serialization; this does not rerun an already frozen action. Existing stream-triggered afford refresh carries current information.

## Task 1 — Reusable read-only information body and consumer proof

**Delivers:** R1, R4–R6; concrete outside-in data needs before publishing wire fields.
**Owner:** web presentation, not rules.
**Prerequisites:** existing `ActionTooltipLine`, `ActionEffectLine`, `EffectRows`; no unpublished SDK.
**Files:** new `src/components/session/combat-experience/ActionInformationContent.tsx`, `.module.css`, `.test.tsx`; new `src/concepts/action-information/ActionInformationConcept.tsx`; existing `src/concepts/ConceptsView.tsx`.
**Interfaces:** component takes `description: string`, `lines: readonly ActionTooltipLine[]`, `effects: readonly ActionEffectLine[]`; optional separate target-held rows and target label. It has no command callback. Wire-facing inputs are plain description plus ordered `{label,value}` facts, independent of existing effect rows.
**Behavior:** description → facts → effects; missing description explicitly readable; empty effects render no false “no effects” claim; no effect total. Body is usable inside hover/pinned containers and by keyboard/touch. Fixture copy is explicitly a concept, never a live content table.
**Tests:** `ActionInformationContent` assertions for Bane with zero effects; Warhammer base `1d8 + STR modifier (+3) · Bludgeoning` preceding an independent Rage row; non-applying/dependent/later rows retained; missing description; literal text rather than HTML; duplicate fact labels remain separate ordered rows; no action controls or execution callbacks.
- [x] Add discriminating tests; observe pre-change failure.
- [x] Implement component and bounded concept consumer.
- [x] Run focused tests, typecheck and desktop/mobile browser proof.
**Verification:** in web worktree, `npm run test:run -- src/components/session/combat-experience/ActionInformationContent.test.tsx`; `npm run typecheck`; launch Vite on an unused port and inspect `?concept=action-information` at 1440 and 393 widths. No live-provider claim from these fixtures.
**Completion evidence:** six focused tests pass and `tsc -b --noEmit` passes. Browser proof renders Warhammer, Bane, Dodge and Command; 393px viewport has no horizontal overflow or page errors. Parent inspected Warhammer/Bane/mobile PNGs. Ignored evidence: web `evidence/action-information/`; fixture preview at `http://localhost:3040/?concept=action-information&preview=1`. This is not a live provider integration.

## Task 2 — Publish the information contract

**Delivers:** R1–R3, R5, R7 transport, preserving effects as separate fields.
**Owner:** protos.
**Prerequisites:** T1 demonstrates description + ordered facts separate from effects.
**Files:** `dnd5e/api/session/v1alpha1/types.proto`; `docs/architecture/components/session-service.md`.
**Interfaces:** `Declaration.information = 22` is optional message `ActionInformation { string description = 1; repeated ActionInformationDetail details = 2; }`; `ActionInformationDetail { string label = 1; string value = 2; }`; `CastOption.description = 3`. Text is provider-authored, display-only, not parsed, folded or used for eligibility. Absent information is unavailable metadata, not an empty-effect claim. IDs/labels and all execution request types remain unchanged.
**Behavior:** additive contract; unavailable compiled offers retain metadata; description is independent of effect count. Existing candidate overrides retain their contracts.
**Tests:** Buf lint/format/breaking and CI-owned generated SDK validation, not bespoke generated-code tests.
- [x] Author fields and field-presence/authority comments.
- [x] `make format`; `make test`.
- [ ] Publish PR and record SDK publication gate; no automatic merge.
**Verification:** run from proto worktree; checks must pass against fetched `origin/main`.
**Completion evidence:** pending.

## Task 3 — Rulebook-owned descriptions and base facts

**Delivers:** R2, R3, R5. One root-module PR.
**Owner:** toolkit `rulebooks/dnd5e` root module: content and rules.
**Prerequisites:** T1/T2 establish the display contract; root work can run without generated SDKs.
**Files:** `combat/actions/definition.go`, `cast.go`; new `combat/actions/information.go`, `information_test.go`; `combat/weaponattack/weaponattack.go`; new `damage/ability_contribution.go`, `_test.go`; `character/action_economy.go`, `action_economy_types.go`, associated tests; `spells/cast.go`, `shillelagh.go`, `cast_test.go`; `events/{offer,attack_roll_offer,post_hit}.go` and corresponding reaction producers under `conditions/` and `features/` where those offers originate.
**Interfaces:** `Definition.Description` and `CastOption.Description` preserve authored content in cloned/serialized definitions. `AvailableAbility.Description` copies `ca.Description()` / `f.Description()`. New `actions.DescribeInput{Definition Definition}` → `DescribeOutput{Information Information}`; `Information{Description string, Details []InformationDetail}` and `InformationDetail{Label,Value string}`. A pure `damage.IncludesAbilityModifier(input AbilityModifierInput{Modifier int,OffHand bool}) bool` owns the existing base inclusion rule for both inspection and execution. Reaction offer/option metadata carries its own description through existing payloads. New `actions.BasicInformationInput{Kind BasicActionKind}` and `actions.BasicInformation` supply root-authored descriptions for the closed existing basic kinds Move, EndTurn, DeathSave, Intimidate and Persuade; unknown kinds return no invented description. Session only maps its verb to the corresponding typed kind.
**Behavior:** spell definitions copy their existing catalogue description; weapon assembly authors a generic attack explanation with typed delivery, not weapon-name cases. Base facts read the assembled profile's dice/type/ability and intrinsic flat bonus without folding effects, rolling, predicting or altering the input. Positive off-hand ability is omitted, negative retained; zero remains distinguishable. No new availability validation depends on prose. Add explanations at the current Command, Thorn Whip, Shillelagh and reaction-option content producers, not in session or web.
**Tests:** testify suites named `InformationSuite`, `AbilityModifierInformationSuite`; Warhammer one/two-handed 1d8/1d10; finesse DEX; alternate ability already assembled; multiple typed pools; intrinsic bonus; on-hit-only/no damage; off-hand positive/negative/zero; clone/input immutability; Bane exact catalogue text; every executable spell's declared options have descriptions; spent Dodge/features keep text. Reaction producer tests retain description in their data.
- [ ] Add tests and confirm old behavior fails.
- [ ] Implement metadata/projection and shared rule without touching dependent modules.
- [ ] Run root focused tests and module readiness gate; push a meaningful checkpoint and open its required draft immediately.
**Verification:** from toolkit `rulebooks/dnd5e`, `go test ./combat/actions ./combat/weaponattack ./damage ./spells ./character ./conditions ./features`; module `go test ./...`; repository hooks/lint as required by `docs/how-to/run-tests.md`. No local replace/go.work.
**Completion evidence:** pending.

## Task 4 — Resolution shares the base rule and preserves option prose

**Delivers:** R3, R5–R6 without changing execution outcomes. Separate resolution-module PR.
**Owner:** toolkit `rulebooks/dnd5e/resolution`.
**Prerequisites:** T3 pushed root commit pinned using `go get` in this module.
**Files:** `strike.go`, `step.go`, `attack_roll_reaction.go`, `strike_pose.go`, `post_hit_reaction.go`; `off_hand_attack_test.go`, new information-preservation tests; `go.mod`, `go.sum`.
**Interfaces:** execution replaces only `!offHand || modifier < 0` with T3's shared inclusion function. `Choice.Description` and `Ask.Offer` description copy the originating provider's text, preserving ID/label and frozen mechanics.
**Behavior:** execute exactly as before; never run the inspection formatter to execute. Frozen payload retains descriptions, and resuming still reads frozen mechanics, not new prose.
**Tests:** positive/negative/zero off-hand outcome regressions; style-restored modifier still arrives as an effect; reaction choices preserve descriptions before/after serialized freeze; no new spending/rolls.
- [ ] Pin pushed root; add tests, implement and run module suite.
- [ ] Publish bounded draft and evidence.
**Verification:** resolution cwd `go test ./...` plus hooks; review diff for one nearest-go.mod module only.
**Completion evidence:** pending.

## Task 5 — Session carries information without owning it

**Delivers:** R2–R7, one current offer and immutable selectors. Separate session-module PR.
**Owner:** toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** T3/T4 pushed commits pinned in this module.
**Files:** `types.go`, `afford.go`, `offers.go`, `casts.go`, `activations.go`, `declaration_id.go`; new `information.go`, `information_test.go`; `react_pending_attack.go`, `react_post_hit.go`, `cast.go`; `declaration_id_test.go`, `effects_test.go`; `doc.go`; `go.mod`, `go.sum`.
**Interfaces:** seam-owned pointer `Declaration.Information *ActionInformation`, with independent seam-owned detail type; `CastOption.Description`. Copy root facts, never format a game rule. Use T3 content for supported non-definition verbs (Move, End Turn, Death Save, social actions), not text authored in session. Persist current reaction explanations with existing window payloads. Strip definition/option presentation metadata from the clone used by selector serialization.
**Behavior:** retain information on unavailable compiled offers and declarations with zero effects; missing/early-blocker information remains absent rather than inventing an offer. No changes to requested verb sets, IDs, costs, gates, candidate universes, or existing `attachEffects` scope. Description changes cannot change selectors; option-ID or profile changes still do.
**Tests:** Bane text with zero effects; Dodge spent but described; weapon facts plus unchanged effect rows; world/turn refresh; option and reaction persistence; modifying only description/details leaves selector stable; action-shape change alters it; fresh slices do not alias root values. Existing boundary/no-rules tests pass.
- [ ] Pin providers; add discriminating integration tests.
- [ ] Implement projections and selector metadata exclusion.
- [ ] Run module tests and publish bounded draft.
**Verification:** session cwd `go test ./...`; include real `Afford` assertions, not converter-only fakes.
**Completion evidence:** pending.

## Task 6 — API field-for-field delivery

**Delivers:** R2, R5, R7 across Go SDK → protobuf.
**Owner:** API transport.
**Prerequisites:** published T2 bindings and pushed T5 SDK pin.
**Files:** `internal/handlers/dnd5e/session/v1alpha1/convert.go`, `convert_test.go`; `go.mod`, `go.sum`; owning session architecture documentation.
**Interfaces:** SDK information and detail rows → same protobuf fields; SDK `CastOption.Description` → wire description. Nil information stays nil; present empty/zero-valued text is not fabricated. Preserve effect/candidate rows in the same declaration.
**Behavior:** no lookup, formatting, arithmetic or new RPC. Existing authorization and refresh paths unchanged.
**Tests:** field-for-field available/unavailable offer with description, multiple ordered details, nested choice text and effect rows; nil information; empty option descriptions; refusal and selector identical.
- [ ] Pin actual providers and add converter tests.
- [ ] Implement and run focused tests, then `make ci-check` before pushing.
- [ ] Publish draft; do not call pseudo-version integration merge-ready.
**Verification:** API cwd `go test ./internal/handlers/dnd5e/session/v1alpha1/...`; `make ci-check`.
**Completion evidence:** pending; blocked on SDK publication until T2 merges.

## Task 7 — Join live inspection and prove effects beneath base facts

**Delivers:** R1–R7 integrated user journey.
**Owner:** web/live integration.
**Prerequisites:** T1 shared body, published T2 TypeScript SDK, T6 running local API.
**Files:** `actionTooltip.ts`, `DesktopActionSurface.tsx`, `DesktopEffects.tsx`, `ActionDock.tsx`, `OrganizedActionSurface.tsx`, `CombatExperience.tsx`, `MapFirstTargeting.tsx`, existing tooltip/target/option components reached by these; related `*.test.tsx`; `src/components/session/SessionEncounterView.test.tsx`; `package.json`, `package-lock.json`; `docs/architecture/components/combat-v2.md`.
**Interfaces:** `buildActionTooltip` copies `declaration.information` description/details, then existing cost/target facts. Shared body renders base facts before `EffectRows`. Any current-target argument must be tied to that exact armed declaration. Held-target effects remain separate from actor rows. Nested option text is read from the supplied option, with no option-ID lookup table.
**Behavior:** hover reads before selection; selected/pinned inspection also reads; no click behavior change. Description persists with zero effects and when unavailable. Fresh declarations replace text; stale data remains labelled. Unknown/missing data is explicit, never reconstructed. Long text scrolls within viewport. Desktop/mobile share meanings; touch inspection cannot execute.
**Tests:** route fixture pairs base Warhammer facts with real-shaped Rage rows; hover/focus before selection makes no RPC; target-specific answers replace only matching actor rows; unrelated action does not borrow target; target-held effects retain ownership; zero-effect Bane and spent Dodge described; choice text visible before confirming; armed-icon re-click preserves picks; stale/duplicate/current-option guards remain green.
- [ ] Adopt published SDK and wire shared component.
- [ ] Add/execute discriminating joined renderer/controller regressions.
- [ ] Run real local stack and browser acceptance.
- [ ] Full `npm run ci-check` at PR boundary, independent review and published dispositions.
- [ ] After operator-authorized releases, repin actual tags and rerun affected checks.
**Verification:** focused tests first; full CI only at PR boundary. Root launcher uses `envs/local/action-information.env` with isolated unused ports. Browser: hover Warhammer with active Rage before click (base first, effects below), plain attack with no contextual rows, Bane, Dodge, described Command choices, unavailable offer, target-specific and target-held rows, refresh/reload, keyboard and 393px touch. Network log confirms inspection causes no gameplay command; user-directed confirmations retain existing one-shot semantics. Screenshots must be read, not merely saved.
**Completion evidence:** pending.

## Visible plan checks

| Requirement / acceptance | Tasks | Concrete proof |
|---|---|---|
| Base information above effects; neither substitutes for the other | T1,T7 | DOM ordering and actual Warhammer-with-Rage screenshot |
| Bane/Dodge explain themselves with zero effects | T3,T5,T6,T7 | Catalogue/ability equality, Afford/wire assertions, live cards |
| Assembled damage, no folded total or new off-hand rule | T3,T4,T7 | Grip/ability/off-hand tests and unchanged execution regressions |
| All offered actions and nested options explained | T3–T7 | Provider coverage, frozen option persistence, visible Command choices |
| Unavailable/missing/stale remain honest | T1,T5–T7 | Spent Dodge and explicit absence/stale tests |
| Hover/focus/touch inert; target scope correct | T1,T7 | No command callbacks, route RPC spy, target guard assertions |
| IDs and gameplay unchanged | T4–T7 | Description-only selector stability; existing targeting and resolution suites |

| Provider | Consumer | Produced = consumed | Dependency | Proof |
|---|---|---|---|---|
| T1 consumer fixture | T2 contract | Description + ordered label/value facts; effects separate | No generated dependency | Reusable renderer tests/browser |
| T3 content/assembly | T5 session | `Information{Description,Details}`; option descriptions | Pushed root pin | Afford integration |
| T3 base inclusion policy | T4 resolution | Modifier/off-hand → inclusion | Pushed root pin | Off-hand regression suite |
| T3 offers | T4 poses → T5 windows | Choice/offer description, same IDs | Root → resolution → session pins | Serialization/resume assertions |
| T5 SDK + T2 proto | T6 API | Optional information, ordered detail rows, option description | Published proto + pushed SDK | Field-for-field converter tests |
| T6 live response + T2 TS | T7 web | Exact wire metadata + existing effect/candidate rows | Published SDK, running API | Browser HTTP/DOM/screenshots |

Plan review dispositions: keep attack/spell-attack effect applicability unchanged; do not infer a renderer bug from Bane. Do not copy creation catalogue data into a client rule table. Weapon information must use assembled profiles. Remove prose from selector material. Root, resolution and session remain separate PRs. API/web compilation against new fields is pending proto publication, not hidden behind local SDK overrides. No implementation completion or live verification is claimed yet.
