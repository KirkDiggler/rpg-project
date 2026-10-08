# UI-only action information plan

Authority: operator correction on 2026-10-08 — stay in the UI lane; consume toolkit facts and file gaps for other teams. [Project#543](https://github.com/KirkDiggler/rpg-project/issues/543) tracks the whole result. [Design](design.md) retains description/base facts above contextual effects.

The [original plan](implementation-plan.md) T3–T6 is now a provider handoff, not this team's implementation scope. Toolkit#1987 owns metadata delivery; API#1084 owns transport; Patient Defense repair stays filed as toolkit#1986 and is excluded. Draft toolkit#1985 remains unchanged at f68a3389 for its owner to adopt. No frontend rulebook or calculations replace those gaps.

Baseline: web39c9c401; merged protos#384 (2b67e9b5), successful publication, generated v0.1.230 at53694d3. SDK installation changes only the proto pin and lock resolution. Existing T1 component proof has7 tests, plus5 existing DesktopEffects tests. The live API has not been verified supplying the new metadata.

## Task U1 — Consume published metadata and unify action bodies

**Delivers:** R1–R4, R6–R7 for UI rendering. **Owner:** web only.
**Prerequisite:** real v0.1.230 bindings, existing `ActionInformationContent`.
**Files:** `package.json`, `package-lock.json`; `src/components/session/combat-experience/actionTooltip.ts`, `actionTooltip.test.ts`, `DesktopActionSurface.tsx`, `DesktopEffects.tsx`, `OrganizedActionSurface.tsx`, `ActionDock.tsx`, `MapFirstTargeting.tsx`, `TargetSurface.tsx`; related existing tests.
**Interface:** `ActionTooltip.description: string` copies `Declaration.information.description` or empty; ordered detail rows prepend existing provider-backed cost/target/damage-type facts. Metadata strings are never parsed or used to authorize anything. `actionTooltipText` carries the description for accessible inspection. Shared body renders description, facts, then existing actor/target-held rows; legacy inline markup preserves its semantics while gaining the same text/facts.
**Behavior:** unavailable/zero-effect offers still explain themselves. Missing description is explicitly labelled, not invented. Target views preserve their existing candidate-specific effect answers and keep target-held rows separate. A hover of another action does not borrow a selected target's answers. No new callbacks, RPCs or selector logic.
**Tests:** actual generated `DeclarationSchema`/information messages with arbitrary provider prose, multiple/repeated fact labels, empty/absent metadata and zero values; unavailable Bane and Dodge; original effect rows unchanged; accessible text contains description; an existing desktop hover renders base facts above real EffectRows without invoking selection. Pinned action and target cards retain existing target context, stale labels and close behavior.
- [x] Add discriminating tests; run against old adapter before implementing.
- [x] Implement shared rendering and read-only field projection.
- [x] Focused tests and `npm run typecheck`.
**Verification:** web cwd `npm run test:run -- src/components/session/combat-experience/actionTooltip.test.ts src/components/session/combat-experience/ActionInformationContent.test.tsx src/components/session/combat-experience/DesktopActionSurface.test.tsx src/components/session/combat-experience/DesktopEffects.test.tsx src/components/session/combat-experience/MapFirstTargeting.test.tsx`; extend to affected legacy/target suites. Evidence pending.

## Task U2 — Explain supplied options before selection

**Delivers:** R5–R7. **Owner:** web only. **Prerequisite:** U1's SDK pin.
**Files:** `DesktopActionSurface.tsx`, `DesktopActionSurface.module.css`, `ActionDock.tsx`, `CombatExperience.module.css`, corresponding desktop/cast/reaction tests.
**Interface:** current `CastOption.description` displayed verbatim alongside its label; existing option ID is the only submitted value. Accessible button names remain labels; descriptions are separately associated. Missing text reads `Description not provided.` rather than deriving behavior from an ID.
**Behavior:** option descriptions are readable before clicking, including touch. Preserve duplicate/missing-ID refusal, freshness guards, cancel, icon re-click pick preservation and one-shot execution. Long text scrolls inside the available viewport and does not widen the dock.
**Tests:** description visible with callback untouched before click; click sends only the exact ID once; no description never disables an otherwise allowed option; current refreshed option copy replaces old text; duplicate ID and stale authority remain inert; cancel unchanged; legacy/mobile and reaction menus show the same meaning.
- [x] Add assertions to real component/controller suites.
- [x] Implement text and bounded layout without new gameplay intent.
- [x] Run focused suites/typecheck and browser narrow/short viewport checks.
**Verification:** `npm run test:run -- src/components/session/combat-experience/DesktopActionSurface.test.tsx src/components/session/combat-experience/castFlow.test.tsx src/components/session/combat-experience/CombatExperience.test.tsx`; browser uses generated fixtures through real components, explicitly not provider delivery proof. Evidence pending.

## Task U3 — Prove the consumer, publish UI review and handoff remaining gaps

**Delivers:** UI acceptance and honest scope. **Owner:** UI team.
**Prerequisites:** U1/U2 complete; provider issues linked.
**Files:** shared desktop concept fixtures (add generated information data, never live fallback maps); new UI integration tests as needed; `docs/architecture/components/combat-v2.md`; ignored evidence scripts/screenshots; PR/issue record.
**Behavior:** real desktop renderer accepts exact generated provider messages; Warhammer base facts precede effects, Bane/Dodge remain readable with zero rows, choices explain supplied meaning, target-specific effects remain scoped. UI absent-data behavior remains compatible with the current API. Do not claim that fixture-enriched browser output demonstrates toolkit/API adoption.
**Tests:** real composition/controller regression with generated declaration data, read-only hover/focus/inspection, existing cast/target behaviors unchanged; browser1440/1000/393 and short desktop, long description scroll, current-data replacement, no page errors or horizontal overflow. Verify source has no spell/feature description table outside explicitly labelled fixtures.
- [x] Render/read screenshots and save exact fixture/network scope.
- [x] Complete `npm run ci-check` before opening/updating web PR.
- [x] Fresh independent UI review, publish findings and dispositions.
- [x] Report UI readiness separately from provider delivery; no automatic merge/deploy.
**Verification:** full local gate once at PR boundary, normal hooks; published review at exact head. Backend-populated live encounter proof is a remaining owning-team integration dependency, not waived by UI tests. Evidence pending.

## Checked requirement coverage

| Requirement | Tasks | Proof |
|---|---|---|
| Description/base facts above effects, including zero effects | U1,U3 | Generated message tests and real-renderer screenshots |
| Current action/target effect context preserved | U1,U3 | Existing candidate answer tests plus target body assertions |
| Every supplied option explanation, including mobile/reactions | U2,U3 | Option DOM/accessibility/callback assertions |
| Missing/stale/unavailable truthful | U1,U2 | Explicit absence + existing authority guards |
| No gameplay/provider work in UI lane | All | Source diff, no rule table/number derivation, provider handoff issues |

| Provider | Consumer | Contract and availability | Proof |
|---|---|---|---|
| Published proto v0.1.230 | U1/U2 | Optional ActionInformation, ordered details, CastOption.description | Installed generated types + lock SHA |
| Toolkit#1987 / API#1084 | UI | Exact same fields; live delivery still pending | Remaining gap, not a fabricated implementation |
| Existing declaration/candidate effect rows | Shared bodies | Same actor rows/target overrides/held rows | Real EffectRows and candidate context tests |
| Existing controller | U2 | Only existing selector/option IDs on deliberate click | Callback/RPC regression assertions |

Plan corrections: no new inspection identity/RPC, no gameplay repairs, no inferred descriptions, and no requirement to implement providers within this team. The UI can be reviewed as a consumer with fixture and absent-data evidence; full player-facing data availability stays open on the linked provider issues.

## UI checkpoint and evidence

Web PR [#1238](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1238), head
`67f18fb460e93675a6d189e4b15cc64854e6cfcc`, integrates dev `3957eba7`.
Upstream #1226/#1230's structural-world and equipment/rest UI are preserved.
The temporary new-SDK compatibility handling was discarded in favour of that
owning team's implementation; narration gap web#1231 is closed against #1230.

The complete local `npm run ci-check` is green: format, lint, typecheck, build,
production/theme guards; 569 test files passed /1 skipped, 7,788 tests passed
/6 skipped. Exact logs: `/tmp/action-information-ui-ci-final.log` and
`/tmp/rpg-dnd5e-web-ci-check.n10KhL/tests.log`. An earlier gate found four
accessible effect-list name regressions; the existing names were restored and
the original assertions pass unchanged. The later complete gate is the receipt.

Actual browser proof uses generated fixtures through real ActionDock components,
not a replacement tooltip: desktop1440,1000×501/900 and393px native touch.
Warhammer base facts precede separate effect rows; Bane/Dodge retain descriptions
with zero effects. Unavailable/missing states are explicit. Long descriptions
scroll by pointer/keyboard; compact inspection clears the measured open menu,
and touch/keyboard reading leaves the intent counter unchanged. Option text
remains outside disabled buttons and all choices stay reachable under horizontal
pressure. Screenshots were read; no page errors/global overflow/gameplay HTTP
commands. Ignored evidence: web `evidence/action-information/consumer-proof.*`
and `consumer-*.png`; preview at `http://localhost:3040/?concept=action-information&preview=1`.

Probe corrections are retained in the case file: the long-running Vite server
needed forced dependency re-optimization after the SDK bump, and the base-fact
ordering assertion now requires the fact text to exist before comparing order.
The consumer harness uses the production HUD styling scope, rather than testing
an unscoped ActionDock and mistaking clipped fixture layout for live behavior.

The [initial independent review](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1238#pullrequestreview-5455868963)
found no Critical/Important issues and four Minor findings, each published inline.

## Review follow-up

Current web head: `ab7c4075f3721767b84406294e3c97a4691d71a5`.
M1/M2/M4 are addressed: one visual/native/accessibility missing-cell formatter;
per-option focusable notes instead of landmarks; common targeting-clearance
placement and height budget for the choice tray. M3 is explicitly deferred to
the existing toolkit#1987/API#1084 metadata-delivery issues: keep the agreed
absence marker, not a frontend rule description or hidden gap.

Suggested compact lifecycle tests exposed pin resurrection after withdrawal;
withdrawn readonly IDs now clear. Both the new missing-cell and withdrawal tests
failed before the fixes. Escape is pinned as closing inspection while preserving
the collection and submitting nothing. Focused tests pass62/62; the complete
local gate passes at this head:569 files/1 skipped,7,791 tests/6 skipped.
Logs: `/tmp/action-information-ui-review-ci.log`,
`/tmp/rpg-dnd5e-web-ci-check.dDHR94/tests.log`.

The browser proof reran at clean `ab7c4075`, with native touch and keyboard
inspection and an additional supplied60px targeting-clearance geometry check.
Evidence is under web `evidence/action-information/review-followup/`; updated
screenshots were read. No gameplay HTTP commands or viewport overflow.

The original review checkout was removed by the runtime and exact retained
resume rejected it. [Focused independent closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1238#pullrequestreview-5456407990)
is published and read back at `ab7c4075`, under a disclosed same-role fallback in
a parent-owned isolated checkout. The reviewer ran831 tests and typecheck;
M1/M2/M4 are verified addressed, M3's provider deferral is accepted, and no
Critical/Important findings remain. Additional optional M5 (one legacy inline
hover-card blank-marker DOM assertion) is deferred to project#543's remaining
integration checks; shared helper and other rendered-path coverage already pass.
All five finding threads carry dispositions and are resolved. All GitHub checks
pass on the same head; the worktree is clean and the PR reports CLEAN.

The UI consumer is ready for the operator's merge decision. No merge/deployment,
provider metadata delivery or Patient Defense repair is claimed by this receipt.
