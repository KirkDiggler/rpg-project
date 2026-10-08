# Desktop bar: live integration plan

Tracking: [web #1225](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1225). Authority: [design](design.md), [walkthrough](README.md), and the operator's direction to bring in the bar without favorites, include available base action information, and proceed through implementation details without repeated approval stops.

## Goal, constraints and baseline

Bring the accepted bar into the real desktop encounter. Keep mobile behavior, provider authority, existing command protocols, dice/reaction pacing and the single combat renderer. Retain 36px controls, 1–4 balanced rows (default one), offer-driven categories, overlapping final pages, explicit multi-target confirmation, and temporary released-story notices (three, six seconds). No favorites/edit controls or persistence in this live slice. No new game capability, description table, global passive catalog, or invented attack statistics.

Inspected web concept: `4b57ce973c369ac3a556ea0141b473501d8ae675`, based on `7d4bcb1e9f9cc5be46504f7ddd62c034d55b40d6`. Fetched web dev: `45ad8f571fb24a0c08da401f5e2d6a5aa9fb6d65`. It adds discovery-sharing/story work, character-creation descriptions, and absent-AC rendering; preserve all three. Project docs base: `f7813385e01c934c9b5d172cd7356b48dc5d55ef`. Asset checkout inspected: `36b5741aa1cb263f4232dcafa6d5bb2ad300ba2d`; reconcile current provider main before asset edits.

Owning instructions: web `CLAUDE.md`, private assets `AGENTS.md`/`README.md`, project `CLAUDE.md`, UI/UX charter and design/planning skill. Work in repository worktrees. Existing web worktree/branch remains the wave's branch: `.worktrees/desktop-hotbar`, `feat/desktop-hotbar`. Create an assets worktree only when executing Task 5. Do not modify the running preview's ignored asset custody or commit public binary files.

## Resolved questions

- **Classification:** reuse exact joins already performed by `liveActionPresentation`. CAST goes to Spells; exact `knownCantrips` / `knownSpells` membership supplies its band. Unknown/granted refs stay in Other spells. ACTIVATE matched to a `FeatureView.ref` goes to Features; other executable offers stay in Actions. Missing private metadata never removes an offer. No live item-use declaration is established by the inspected verb registry; do not manufacture Items from inventory. The section remains available to an explicitly classified future offer.
- **Inspection:** no arbitrary idle reference attack. Show base information independently of effect count. Existing `buildActionTooltip` supplies name, damage type, costs and refusal; target effect rows remain provider-authored. More attack statistics/descriptions need a later provider contract and do not block this slice.
- **Targeting:** reuse existing RPC inputs. CAST supports ordered member lists; Attack/Activate/social calls are scalar. No new non-CAST list RPC. Preserve legacy/mobile selection semantics; opt desktop into toggling. Refreshed/withdrawn selectors never inherit executable intent from a name/ref match. Invalid authority retains the existing fail-closed reset behavior and readable notice.
- **Activation:** use the measured concept's container rule (width >=1000, height >500) on the real encounter container. This is responsive presentation, not a saved setting. Smaller frames keep the existing surface. Do not remount the canvas when crossing the boundary. Rollback is reverting the live desktop opt-in, not inventing another rules path.
- **Notices:** keep per-entry delivery provenance through the existing story projection. No second event interpreter or new server contract.

These are implementations of agreed ownership/behavior using inspected contracts, not requests for further blanket approval. A newly discovered provider need or changed behavior returns only that decision to the operator.

## Sequence

Execute Tasks 1–4 in order in the web worktree because they share types/composition. Task 5 supplies private runtime files; Task 6 joins all seams and verifies the real route. Development can test web contracts with ignored local assets, but release needs the reviewed asset promotion before the web consumer. No proto/toolkit/API change is prescribed by this plan; if one becomes necessary, add its producing task and generated-binding/released-pin checks before consuming it.

### Task 1: Bar controls and action information independent of effects

**Delivers:** R2/R4/R5/R9/R12: live-capable bar without favorites, and inspected actions remain informative with zero effects.
**Owner:** Web shared presentation.
**Prerequisites:** Accepted prototype; reconcile current `origin/dev` without dropping upstream behavior; installed lockfile dependencies.
**Files:** Modify `src/components/session/combat-experience/organizedActionPresentation.ts`, `DesktopActionSurface.tsx`, `DesktopEffects.tsx`, `DesktopEffects.module.css`, `CombatExperience.tsx`; tests `DesktopActionSurface.test.tsx`, `DesktopEffects.test.tsx`, `CombatExperience.test.tsx`. Update concept composition/documentation where retaining its favorite experiment requires explicit opt-in.
**Interfaces:** Add explicit presentation-only `desktopFavorites?: boolean`, disabled unless true. Disabled means an empty favorite set regardless of supplied customization, no edit/star UI, and no storage. Keep `DesktopHotbarLayout.rows` and `onChange`. `DesktopEffects`' existing declaration/target input becomes action inspection: base `ActionTooltip.lines` plus contextual `effectLinesFor`. Select the uniquely armed declaration regardless of `effects.length`; no fallback to `desktopEffectsDeclarationId` on the live path.
**Behavior:** Rows/pages work with no favorites. Hover/focus inspection stays read-only. Base information is present for zero-effect attacks; changing action clears pinned effect inspection. Idle has an honest no-selection state, not an unrelated attack. Private status remains separate. Retain concept favorites only behind explicit opt-in; do not erase the experiment.
**Tests:** No Edit bar button/stars when the flag is absent, even with populated favorite IDs; rows 1/4 and final-page overlap still work. Explicit concept opt-in retains favorite tests. A longsword declaration with cost and slashing damage, `effects=[]`, displays name/type/cost. Switching from an effect-bearing declaration drops its effect text. Missing declaration does not show reference-attack data. Stale authority shows stale feedback, not readiness. Hovering an unavailable icon does not issue intent.

- [x] Add/update named assertions and observe relevant pre-change failures.
- [x] Implement controls and inspection; update superseded concept documentation.
- [x] Run focused checks and record outputs.

**Verification:** From web worktree: `npm run test:run -- src/components/session/combat-experience/DesktopActionSurface.test.tsx src/components/session/combat-experience/DesktopEffects.test.tsx src/components/session/combat-experience/CombatExperience.test.tsx`; `npm run typecheck`. Browser check idle/zero-effects/effects/rows, including keyboard inspection.
**Completion evidence:** Web `9564b83c`, after upstream reconciliation `2dc060b3`. Five expected pre-change failures; 110 focused tests pass across seven files, typecheck passes, lint has zero errors and five warnings in untouched files. Browser Chrome/SwiftShader at 1280×900 verifies idle/zero-effect/combined-effect inspection; 393×844 retains compact UI. Screenshots inspected, no page errors; ignored evidence `evidence/desktop-hotbar/hotbar-slice1-{plain,effects,mobile}.png`. The first browser probe needed the collapsed lab Controls opened before selecting Martial; rerun passed. These are shared-presentation/concept checks, not real-route or full-CI proof.

### Task 2: Live classification and artwork projection

**Delivers:** R1/R3: real offers populate categories without class/name rules.
**Owner:** Web adapter, never fixture data or rules owner.
**Prerequisites:** Task 1; current `Declaration`, known-spell arrays and `FeatureView` contracts inspected.
**Files:** Modify `src/components/session/combat-experience/liveActionPresentation.ts` and `.test.ts`; add `liveActionArt.ts` and `.test.ts` in that directory; exercise existing `desktopHotbarGroups.test.ts`. Final encounter wiring is Task 6.
**Interfaces:** Add opt-in `desktop?: boolean` to `LiveActionPresentationInput`. When true produce `desktopSectionByDeclarationId`, `desktopSpellKindByDeclarationId`, `desktopIcons`, with favorites disabled and no default effects declaration. When false/absent preserve existing organized-HUD output. `liveActionArt(declarations: readonly Declaration[]): Readonly<Record<string, ActionIconPresentation>>` maps exact full provider refs or typed verbs to reviewed art URLs; unknowns receive readable generic fallback, never disappear. Art lookup affects no category, label, availability or target rule.
**Behavior:** Exact known-cantrip match has precedence as in the current adapter; known leveled refs use leveled band; unknowns remain Other spells. Features use the same exact serialized `FeatureView.ref` join as the current adapter. Cost/exhaustion/level/name/opaque selector spelling never classify. No actual item-use offer means no Items category, not a disabled placeholder or inventory-derived command. Semantic art identity survives reminted selectors by producing a fresh ID-keyed map each render.
**Tests:** Adversarial names and selector strings cannot change classification. Unavailable feature/cantrip stays in its section. Missing private metadata retains every executable offer; unknown spell stays Other spells. Gaining/losing exact feature/spell metadata changes presentation only. A renamed display name retains art by exact ref, while an unknown ref gets fallback. Desktop false preserves legacy adapter assertions. Hint-only categories cannot mint offers.

- [x] Add failing adapter/art cases.
- [x] Implement exact joins and pure art mapping.
- [x] Run adapter/group checks; verify no fixture import in production modules.

**Verification:** Web worktree: `npm run test:run -- src/components/session/combat-experience/liveActionPresentation.test.ts src/components/session/combat-experience/liveActionArt.test.ts src/components/session/combat-experience/desktopHotbarGroups.test.ts`; `npm run typecheck`. `rg 'concepts/|fixture:' src/components/session/combat-experience/liveAction*.ts` must reveal no production fixture dependency.
**Completion evidence:** Web `c76591ae`: 14 adapter/art/group tests pass, typecheck and changed-file lint pass. Pre-change tests exposed absent desktop mappings and the missing art module. Exact canonical art refs were checked against existing web request fixtures and toolkit `refs/{weapons,spells,features}.go`; unknown refs use generic fallback. No production fixture import. Task 5 still must prove runtime image availability; Task 6 still must activate the adapter on the real route.

### Task 3: Desktop target toggling through the real controller

**Delivers:** R6/R11: map, list and chips feed one authoritative command owner; multi-target confirmation does not fire on selection.
**Owner:** Web live interaction/controller and map composition.
**Prerequisites:** Task 2; existing scalar RPC wrappers and `CastParams.targets` verified. Task 6 supplies responsive desktop flag.
**Files:** Modify `src/components/session/combat-experience/useSessionCombatExperience.ts`, `memberTargeting.ts`, `types.ts` as needed for the opt-in; tests `castFlow.test.tsx`, `activationTargetingFlow.test.tsx`, `intimidateFlow.test.tsx`, `persuadeFlow.test.tsx`, `memberTargeting.test.ts`. Preserve `MapFirstTargeting` and map marker contracts; encounter wiring in Task 6.
**Interfaces:** Add `memberTargetingMode?: 'legacy' | 'map-first'` to controller input, default legacy. Desktop toggles ordered `selectedCandidateMembers` through existing provider-bound helpers; list/chip/map all call the same controller callback. `onConfirmTargets` still sends CAST/MEMBER through `onCastTargets` and unchanged `CastParams { session, member, declarationId, targets, option? }`. Scalar verbs retain their current inputs and immediate single-target behavior. Explicit `onChangeCastOption(declaration)` is separate from icon re-selection: it uses the shared current-offer tray opener, clears old picks/option, and fences stale selection callbacks; ordinary armed multi-icon re-click keeps its picks.
**Behavior:** Re-click selected member removes it in desktop multi-selection; re-click armed multi-command preserves picks. Reaching max does not dispatch. Chosen option survives toggling. Confirm checks current unique offer, option, candidate availability and bounds; pending RPC suppresses repeats. Withdrawal/loss of authority resets executable interaction with existing notice; no semantic remapping of stale IDs. A late callback from cancelled/replaced selection or prior session/member must not select for the new action. CELL targeting and WORLD movement/social gates remain unchanged. A non-CAST multi shape unsupported by its scalar RPC must fail closed with readable unsupported-contract feedback, never submit the first member as if complete.
**Tests:** Real hook with mocked RPC: choose ally/self for Bless and enemies for Bane; add/remove via the same callback, preserve order/option, no RPC until confirm, exact ordered IDs once after confirm. Max-bound clicks do not send; unavailable/duplicate/withdrawn candidates and stale options cannot submit. Saved callbacks invoked after cancel/rearm/scope change do nothing. Same-command click preserves list. Scalar Attack/Help/social cases still send exactly their existing request; legacy CAST mode remains unchanged. CELL entity miss does not cast or walk.

- [x] Add failing controller tests before changing callbacks.
- [x] Implement opt-in toggling and selection/scope fencing without RPC side effects in state updaters.
- [x] Run controller/selection checks and existing scalar regression files.

**Verification:** Web worktree: `npm run test:run -- src/components/session/combat-experience/castFlow.test.tsx src/components/session/combat-experience/activationTargetingFlow.test.tsx src/components/session/combat-experience/intimidateFlow.test.tsx src/components/session/combat-experience/persuadeFlow.test.tsx src/components/session/combat-experience/memberTargeting.test.ts`; `npm run typecheck`. Joined canvas/RPC assertions belong to Task 6.
**Completion evidence:** Web `42d4dc1a`; 68 target/controller tests pass, typecheck and changed-file lint pass. Three pre-change failures exposed append-only picks, stale callbacks and unsupported scalar-list dispatch. Added batched clicks, self/ally plus opaque option, withdrawn candidate removal, authority loss, same-ID cancel/rearm and scope/mode reset coverage. Interaction updates synchronously record local intent outside React updater side effects; selection epochs fence old callbacks. The wider 812-test combat/concept run also passes. Independent review W-1 exposed a multi-member CAST plus options corner: Change choice shared the intentional re-click guard and silently did nothing. The joined route regression reproduces that defect. Pushed fix `06f766bb` introduces the explicit change-choice intent above, retains icon preservation, and passes 198 focused tests including stale/in-flight checks. Its complete boundary gate passes: 548 files passed/one skipped, 7,519 tests passed/five skipped, format/lint/types/build green, exit0. The thread records Addressed. Independent [closure review](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1228#pullrequestreview-5449874826) reproduces the old-head failure and verifies the fix without new defects; the finding thread is resolved.

### Task 4: Recovery-safe notices from the existing story

**Delivers:** R7: temporary cards for newly released live story only, with retained history.
**Owner:** Web story projection/presentation lifetime.
**Prerequisites:** Task 3; `CombatStoryFact.source` and presentation record provenance already exist.
**Files:** Modify `src/components/session/combat-experience/types.ts`, `story.ts`, `useStoryNotices.ts`; tests `story.test.ts`, `useStoryNotices.test.tsx`; add cases to `useCombatStoryPacing.test.tsx` and `CombatExperience.test.tsx` as required. Update concept story samples to explicit provenance.
**Interfaces:** Add optional `deliverySource?: 'live' | 'catchup'` to `CombatExperienceStoryExchange`. `buildCombatStory` copies `fact.source` onto each projected entry. Missing source is not announcement-eligible (history still renders); fixtures wanting notices explicitly say live. `useStoryNotices` consumes the released story and announces only newly seen live entries. No raw-event subscription in the notice hook.
**Behavior:** Preserve story ID, text and source through pacing filters and dice/reaction release. Initial/scope/resume snapshots establish baseline without announcements. Catch-up arriving while aggregate state is live never announces; duplicate delivery cannot upgrade old history into a new card. TTL starts at visible release, each card expires independently, newest three retained, history unaffected. Clearing/scope change cannot leak old notices.
**Tests:** Live/catchup facts project identical prose with distinct provenance; missing provenance remains history-only. Periodic catchup under `streamState='live'` produces no notice. Live entry withheld for dice announces once after release; mount/resume does not replay. A duplicate story ID never extends expiry. New cards do not extend old cards; six-second expiry preserves history and three-card cap. Preserve current-dev discovery story assertions.

- [x] Add provenance and background-recovery failures.
- [x] Thread metadata through the one story projection and update explicit fixture intent.
- [x] Run story/notice/pacing tests.

**Verification:** Web worktree: `npm run test:run -- src/components/session/combat-experience/story.test.ts src/components/session/combat-experience/useStoryNotices.test.tsx src/components/session/combat-experience/useCombatStoryPacing.test.tsx src/components/session/combat-experience/CombatExperience.test.tsx`; `npm run typecheck`.
**Completion evidence:** Web `154e915e`; 812 tests across shared combat and the desktop concept pass, including timer/provenance/release cases. The new background-catch-up case failed before implementation. Replay-equality tests now assert unchanged prose with explicit different source metadata; duplicate tests retain original provenance. Real-route expiry/history/reload checks are recorded under Task 6.

### Task 5: Private runtime promotion of the accepted icon set

**Delivers:** R8: accepted art loads from the production asset path, not a local preview directory.
**Owner:** Private `rpg-game-assets` runtime curation; web consumes URLs only.
**Prerequisites:** Fresh private-provider worktree/instructions; accessible licensed source archive and accepted preview files; Task 2 names the consuming semantic mappings. Verify current inventory before extending it.
**Files:** New private `library/desktop-hotbar-v1.json` (exact source member, SHA-256 and output mapping), `scripts/promote_desktop_hotbar.py`, `scripts/test_promote_desktop_hotbar.py`; new runtime PNG subset and private `SOURCES.md` under `harness/models/synty/ui/desktop-hotbar/`; regenerate `harness/catalogs/synty-complete-inventory.json` with existing generator. Update web `liveActionArt.ts` to `/models/synty/ui/desktop-hotbar/<original-filename>` URLs. Do not extend the unrelated chrome/bar contract manifest or copy the full pack.
**Interfaces:** Retain accepted `ICON_DarkFantasy_<family>_<name>_Clean.png` filenames. Initial real-command subset: Stat Speed_02, Strength_02, Mind_01; Inventory Maces_01, Swords_01, Bows_01; Status DefenseUp_03, Dead_01, Cursed_03, Fortified_01, Health_02, FortifiedHealth_01, SpeedUp_01, Stealthy_01, Health_01. Add effect-only Daggers_01 / Spirit_01 only if the joined inspection actually consumes them; no fixture-instance effect IDs in production maps. All files are exact licensed source bytes, not generated rules content. Additional commands use reviewed generic/fallback artwork, not inferred rules.
**Behavior:** Promotion checks source-member and output hashes, PNG dimensions, duplicate/output path safety and exact declared output set before publishing. `--check` compares canonical bytes without mutation; no deletion from Downloads and no replacement of unrelated runtime files. Existing runtime sync consumes the private Synty root independently from custom dice.
**Tests:** Missing source, changed hash, invalid PNG and path traversal fail without partial publication; a clean run yields the declared exact subset; `--check` detects drift. All web art URLs resolve in a freshly staged runtime tree, not just the developer's ignored preview directory.

- [x] Bind the accepted subset to source hashes in private metadata.
- [x] Add promotion validation/negative tests; promote and regenerate inventory.
- [x] Run provider gates and stage a clean consumer tree.

**Verification:** Assets worktree: `python3 -m unittest discover -s scripts -p 'test_promote_desktop_hotbar.py'`; `python3 scripts/promote_desktop_hotbar.py --check`; `python3 scripts/build_synty_complete_inventory.py --check`; `python3 scripts/verify_web_asset_stage.py --verify-only`. Exact sync contract check from web worktree: `RPG_GAME_ASSETS_PATH=<assets-worktree> RPG_WEB_ROOT=<web-worktree> ASSETS_SYNC_SKIP_UPDATE=1 sh scripts/sync-game-assets.sh --runtime-assets` after preserving ignored preview assets as needed. Commands using the new script are prescribed verification, not claims that it exists yet.
**Completion evidence:** Private assets `4ebe029e192d2a5d4643cb7c527c627cbdba1eec`, [PR #289](https://github.com/KirkDiggler/rpg-game-assets/pull/289). Fifteen 256px PNGs match both the source archive and accepted preview byte-for-byte. Eight promotion tests, check mode, complete inventory (3,521 files) and provider stage verification pass. Runtime sync from the clean commit to a fresh local consumer root passes; all 15 URLs served by the web match provider hashes. The launcher uses separate pinned model subtrees, so only the additive reviewed icon subtree is copied from that clean stage afterward. Existing worktree-local model links/preview art were preserved before launcher sync to avoid writes through shared-checkout symlinks. Provider review/merge remains before consumer release. No public binary commit.

### Task 6: Real encounter activation and integrated verification

**Delivers:** R1–R8, R11–R13 joined on the live route; R9 exclusion proven.
**Owner:** Web live composition and regression verification.
**Prerequisites:** Tasks 1–5, real local API/session fixtures, production-equivalent private asset staging. Review current upstream once more before final gate.
**Files:** Modify `src/components/session/SessionEncounterView.tsx` and `.test.tsx`, shared `CombatExperience.tsx`/tests where joining props; add `src/components/session/combat-experience/useDesktopHotbarFrame.ts` and `.test.tsx`; update `docs/architecture/components/combat-v2.md`, `src/concepts/desktop-hotbar/CONTRACT.md`. Existing `SessionCanvas.test.tsx` and `EntityTargetMarker.test.tsx` cover map identity/markers.
**Interfaces:** `useDesktopHotbarFrame(container: HTMLElement | null): boolean` observes the actual encounter content, false until measurable or without ResizeObserver; desktop iff width >=1000 and height >500. Use the same result for adapter `desktop`, controller `memberTargetingMode`, and story feedback. A stable state-setter ref provides the observer node; inspection established that the old `encounterContentRef` had no readers, so no extra composed-ref adapter is needed. Encounter-owned `desktopRows` feeds `desktopCustomization` with an empty favorite map and survives temporary dock unmounts without persistence. `renderMap` forwards optional `selectedTargets` to `SessionCanvas`; omit it on the legacy path. Story feedback scope is session/member; no storage. Keep canvas element identity stable while changing marker props.
**Behavior:** Desktop default is the new bar; smaller containers keep existing organized HUD and controller mode. Resize must not issue an RPC or silently execute pending selections. Cancel/review an active selection if switching interaction mode rather than carrying incompatible UI state. Existing navigation, discovery sharing, equipment, CELL/path clicks, reactions, death saves, world clock and run-ended state keep their owners. Preserve private missing/stale status, including absent AC (never synthesize 10). Default-closed log and notices apply only to desktop.
**Tests:** Observer 999/1000px widths and 500/501px heights choose correct surface; cleanup disconnects; missing observer uses legacy. Real encounter props wire the same desktop state through adapter/controller/map/story. No favorite controls/storage; current provider categories only. Map entity self/ally clicks reach CAST candidates; confirm reaches the mocked RPC with exact IDs; resize/cancel/stale scope cannot send. Canvas identity survives selection and mode change. Mobile, death-save/reaction, world/social, stale private status and discovery-sharing regression assertions remain passing.

- [x] Add responsive and joined-path tests; implement live wiring.
- [x] Run focused encounter/map/controller checks, then inspect real-route browser evidence.
- [x] Update owning docs; ensure production modules import no concept fixtures.
- [x] Run one complete `npm run ci-check` at the PR boundary; publish actual results.
- [x] Open linked PR(s), run the required independent review round and publish its verdict.
- [x] After operator-authorized provider merge, reverify the merged runtime dependency before consumer release.

**Verification:** Web worktree: `npm run test:run -- src/components/session/SessionEncounterView.test.tsx src/components/session/SessionCanvas.test.tsx src/components/hex-grid/EntityTargetMarker.test.tsx src/components/session/combat-experience/useDesktopHotbarFrame.test.tsx`; `npm run typecheck`; then `npm run ci-check` once before opening/updating PR. Real route: join a live fixture session, inspect an attack with no effects, cast a supported multi-target spell with map/list removal and confirm, exercise unavailable/stale offers, wait for notice expiry, recover history without replay, inspect debug overlay, resize through desktop and mobile widths. Capture requests, screenshots and browser errors. Do not substitute the concept route for this walk.
**Completion evidence:** Web `1326a39d`; 244 joined encounter/canvas/frame tests pass, then 149 focused cases cover the live-walk refinements and 135 cover the row-lifecycle check (overlapping sets, not additive totals). Isolated `local/desktop-hotbar`: web 3032/API 8112, API revision `c68267270549c0140ec2339b5639bdee8e3202cc`. Real seeded Bard entered Reference Tomb, selected/toggled Skeleton and Zombie for Bane, preserved picks on re-click, and sent exactly one decoded CAST with those ordered IDs after confirmation. Canvas identity held; three result notices expired into retained history; reload replay stayed quiet. Desktop/short/four-row/mobile screenshots inspected; debug JSON preserved bar geometry and sent no commands. Worktree-local `evidence/desktop-hotbar/live-integration.json` and screenshots retain the proof. The first probe treated valid304 cache responses as errors; read-only verification confirmed loaded image dimensions and exact served bytes. Live walking exposed delayed desktop default-log initialization and legacy world-clock rows, both corrected with regressions. Encounter-owned row state survives compact/desktop remounts without storage. The first completed full gate passed format/lint/types/build, then reported 7,516 passing tests, five skips and one unchanged `RoomPublishingPanel.test.tsx` focus assertion failure. That file has no diff against dev and passes all 29 cases in isolation. The complete gate rerun passes: 548 files passed/one skipped, 7,517 tests passed/five skipped, plus format/lint/types/build, exit0. [Web PR #1228](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1228) retains the initial failure and full successful gate output. Initial independent review is published on assets#289 and web#1228, with no Critical/Important findings and one Minor W-1. The asset verdict is clear. W-1 has an Addressed disposition with pushed `06f766bb` in its inline thread; the independent closure review verifies that fix and its thread is resolved. Its new complete boundary gate passes (7,519 tests, five skipped), with exact output added to the web PR; all GitHub checks on `06f766bb` also pass. Neither gates nor this plan authorize merge/deployment.

## Requirement coverage

| In-scope requirement / acceptance | Task(s) | Concrete proof |
|---|---|---|
| R1 provider authority and opaque identity | 2, 3, 6 | Adversarial refs/names, exact current-selector RPC assertions, stale callbacks send nothing |
| R2 desktop layout and mobile preservation | 1, 6 | Existing packing tests plus 999/1000 and 500/501 responsive boundaries; real-route mobile walk |
| R3 conditional categories and cantrip group | 2, 6 | Exact joins, unavailable retention, no manufactured Items, gained/withdrawn offers |
| R4 rows and overlapping pages | 1, 6 | Rows 1–4, existing final-page overlap assertions, narrow/short real frames |
| R5/R12 status and base information with optional effects | 1, 6 | Zero-effect attack retains base lines; switching clears old effects; idle has no arbitrary attack; missing AC remains absent |
| R6/R11 shared target picks and explicit confirmation | 3, 6 | Bane/Bless map/list/chip toggles and exact one-shot CAST request; scalar/CELL regressions |
| R7 notices/history/debug | 4, 6 | Live/catchup provenance, delayed release, independent TTL and three-card cap; debug overlay click containment |
| R8 licensed art custody | 5, 6 | Private hash-bound subset, fresh runtime-stage URL checks, no public binaries |
| R9 favorites excluded | 1, 6 | No edit/star controls despite supplied IDs; no preference storage |
| R10 facts/description absence | 1, 2 | Known metadata joins; Other spells fallback; no fabricated dice/descriptions |
| R13 activation and review boundary | 6 | Responsive live opt-in, preserved canvas, full CI and independent review |

Explicit deferrals: favorites/durable preferences; new attack statistics and option-description provider work; new item-use or non-CAST multi-target capabilities; a global passive catalog. No existing offered action is removed to make these deferrals convenient.

## Provider/consumer seam check

| Provider task/module | Consumer task/module | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|
| Existing Afford / character hooks | 2 adapter | `Declaration[]`, exact known refs, `FeatureView.ref` → ID-keyed display hints; missing data remains missing | Inspected existing fields; no bindings change | Adversarial/absent-field adapter tests |
| 2 adapter, 1 opt-in | 6 encounter / bar | `desktop` produces icon/group hints; `desktopFavorites` false/absent disables controls | 1 → 2 → 6, shared types changed sequentially | Live props plus no-favorite assertions |
| Existing CAST and scalar RPC wrappers | 3 controller / 6 map | CAST `targets[]` versus scalar `target`; exact selector and option preserved | Current wrapper signatures inspected | Hook request capture and real-route confirm |
| 3 controller | Existing map-first view, 6 canvas | Ordered member IDs; `selectedTargets` present only for desktop | 3 before 6 | Map/list/chip consistency, self selection, delayed callbacks |
| Existing `CombatStoryFact.source`, 4 projection | 4 notices, 6 feedback | Entry `deliverySource`, identity and released text; absent means history-only | Producer/consumer same task; concept fixtures explicit | Background recovery while aggregate live, release-gated notice |
| 5 private runtime subset | 2 art map, 6 deployed route | Exact original filenames under `/models/synty/ui/desktop-hotbar/` | Local dev may use ignored staged files; provider must precede release | Hash/inventory gates and clean-stage HTTP image checks |
| 6 container observer | Adapter/controller/story opt-ins | One boolean, same >=1000/>500 rule, no canvas remount | Observer owns responsive state; no storage | Boundary, resize and identity tests |

## Readiness check and remaining prerequisites

The plan assigns every slice requirement to an owner and joined proof. Existing protocol fields suffice for the current offered actions; unavailable metadata has an explicit honest fallback. No generated bindings, persistence or gameplay event schema changes are prescribed. Task 4 changes presentation metadata only. Current-dev reconciliation and private asset staging are explicit prerequisites, not claims of completion.

Task contracts are ready for execution in dependency order. The exact private source hashes are Task 5's measured inputs, not invented values in this public plan. If the accepted source files cannot be matched or the live fixtures expose an unsupported required command contract, report that concrete blocker and update the affected task instead of substituting assets or weakening dispatch checks.

Tasks 1–5 and Task 6's live wiring/walk have evidence recorded above. The real local route is opted in. Full application CI passes on the implemented source. An initial gate was stopped before completion to add the row-lifecycle fix; the first completed gate found the unrelated focus assertion described above, and one complete rerun passed. Implementation and independent review are complete: W-1 is fixed, independently verified and resolved; complete local CI and all GitHub checks pass on current consumer head `ed009335`. The operator merged private assets#289, and the merged runtime dependency is reverified below. Web merge and release remain operator-controlled actions. The published records are web#1228, assets#289 and project#540. No web merge or deployment performed.

## Operator walkthrough refinement: compact free roam

The operator's `Screenshot_20261008_091906.png` shows the desktop WORLD bar
stacking the old exploration banner, standalone utilities, action surface and
footer. Pushed web `ed009335` uses the same offers-plus-utility-footer composition
as combat, with a small movement/freshness hint. Compact/mobile composition,
provider offers, availability and command callbacks are unchanged.

A regression fails on the old composition because the hint/utilities sit outside
the desktop surface. The new layout passes 213 focused tests serially, typecheck
and changed-file lint. One parallel run hit the previously observed unchanged
scene-identity assertion; its full file passes 133 tests alone, and investigation
is tracked separately in web#1229 rather than weakening the assertion.

Real free-roam verification uses the separate seeded level-up-fighter session,
leaving the Bard session untouched. The one-row dock measures 134.5px at
1440/1280/1000px viewport widths, including a 501px-high frame; no page overflow,
page errors or gameplay commands during layout checks. Screenshots inspected;
393px mobile retains its old exploration panel. Evidence is local/ignored under
`evidence/desktop-hotbar/hotbar-free-roam*`. Full boundary CI passes on
`ed009335`: 548 files passed/one skipped, 7,522 tests passed/five skipped,
format/lint/types/build green, exit0. The retained reviewer reports no findings
on the four-file delta after 216 focused tests, typecheck and lint; its exact-head
[review is published](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1228#pullrequestreview-5450066917).
Fresh GitHub checks run on the pushed refinement. Prior gates/reviews above
describe `06f766bb`; `ed009335` is now the consumer head. Asset merge is complete;
web merge and release remain operator-controlled.

## Merged provider verification

The operator merged assets#289 at `b23b03de06a9e7b3768df8776a973c9a0d25366c`.
Its selected sprite/provenance/inventory bytes are identical to the independently
reviewed `4ebe029` provider. A clean detached provider worktree at the merged
commit passes `promote_desktop_hotbar.py --check`, complete-inventory verification
and `verify_web_asset_stage.py --verify-only`.

The web worktree is synced from this merged provider with `--pinned-runtime`,
retaining its existing character/NPC/world catalog pins and restoring the ignored
concept preview art afterward. Production-mode `--runtime-assets` also succeeds
into a fresh staging destination. All 15 sprite hashes match the merged manifest
in the local web tree, the production-style stage and the running web's HTTP
responses. The public web source tree stays clean.

No web dependency bump is required: UI sprites have no committed provider pin;
production packaging clones the asset default branch and performs runtime sync.
Do not regenerate unrelated catalogs merely to adopt this additive icon set.
The asset blocker is cleared, and current web GitHub checks are all green. This
verification does not authorize a web merge or deployment.
