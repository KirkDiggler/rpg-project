# Selected-action target hover — checked UI plan

Authority: operator request on 2026-10-09, project#543. See [delivery.md](delivery.md)
and design R9. This task remains in the UI lane; toolkit#1987/API#1084 keep their
metadata-delivery work and Patient Defense remains separate toolkit#1986.

## Scope and inspected baseline

Web dev `8ab7a11b` contains merged UI consumer#1238; new worktree
`rpg-dnd5e-web/.worktrees/target-hover-information`, branch
`feat/target-hover-information`. No new proto/API field or command is needed.

Measured path: `SessionCanvas.onHoverEntity` already reaches
`SessionEncounterView` → `CombatExperience.hoveredTarget` → `TargetSurface` →
`MapFirstTargeting.hoveredTarget`. The desktop component updates only its preview
banner; it renders effects only after explicit `details` state is set by an Info
button. Candidate overrides and target-held rows already have shared renderers.
The compact target-effect path already responds to inspection and is retained.

**Done:** a unique offered target's mouse/pen hover or list keyboard focus shows
provider effect answers without selecting/confirming. Its visible scrollbar is
reachable; reading input belongs to the panel rather than passing to the map.
Explicit full inspection stays available. No general condition browser, hidden
stats, outcome prediction, gameplay repair or backend implementation.

## Task H1 — Target-effect peek and explicit full reader

**Owner:** web presentation. **Prerequisites:** existing generated candidate/effect
fields and merged `ActionInformationContent`/`EffectRows`.
**Files:** `src/components/session/combat-experience/MapFirstTargeting.tsx`,
`MapFirstTargeting.module.css`, `MapFirstTargeting.test.tsx`.
**Interfaces:** existing `hoveredTarget?: string|null`, current
`MemberTargetingInput.declaration`, `selectedMembers`, freshness/turn facts and
unchanged callbacks. No new public data interface. `LocalView.preview` owns the
last unique inspected candidate; `LocalView.details` owns explicit full inspection.

**Behavior:**
- A new nonempty map hover resolves by exact, unique candidate member ID. A valid
  candidate updates `preview`; a foreign ID supplies no replacement. Empty map
  space/non-candidate transit keeps the last named preview readable. Withdrawal
  or ambiguity of the inspected candidate clears invalid information.
- With no explicit full reader, a valid preview shows a pointer-reachable, keyboard-focusable
  `role=tooltip` containing the action/target context and the selected action's
  supplied base facts, followed by target-held rows and the actor-effect answers
  for that target. Both action and target cards share the same fact renderer. Missing rows say no
  information was supplied, not that the target has no conditions.
- Mouse/pen entry and keyboard focus on list rows/selected chips take the same
  inspection path. Touch still has explicit Info/selected-target controls.
- Info pins the existing full card (description/base facts, then effects) and
  may focus that explicitly requested reader. Merely hovering never steals
  focus. An explicit reader stays on its named target rather than changing
  underneath the pointer. Close/Escape clears information, not picks or actions.
- Candidate withdrawal/ambiguity and action change clear invalid information;
  no rows are retained as separate cached effect data. Unavailable candidates
  remain inspectable; stale information is labelled. An empty selector or a
  non-member action supplies no candidate inspection.

**Tests:** generated two-candidate attack with one actor row and distinct
candidate overrides plus full held rows. Hover A/B shows each answer in the
proper list; no choose/confirm/cancel call and selected IDs unchanged. Exercise
unavailable/stale, zero rows, foreign/duplicate IDs, withdrawal/re-offer, action
change, selected chips, focus, touch-not-auto-hover, focusable/read-only preview,
explicit full-reader priority and Close/Escape. Keep prior selection/confirmation
regressions intact.

- [x] Add regressions and observe the intended pre-change failure.
- [x] Implement the UI-only state/render path and bounded preview style.
- [x] Run focused tests and typecheck; retain compact-target parity.

**Verification:** web cwd `npm run test:run -- src/components/session/combat-experience/MapFirstTargeting.test.tsx src/components/session/combat-experience/memberTargeting.test.ts`; `npm run typecheck`.
**Evidence:** implemented and checked in web#1241; source and test receipts below.

## Task H2 — Joined route and browser proof

**Owner:** UI integration. **Prerequisite:** H1.
**Files:** `src/components/session/SessionEncounterView.test.tsx`, existing
`src/concepts/desktop-hotbar/`/`organized-hud/` fixture consumers or a bounded
fixture addition beside `src/concepts/action-information/`; owning
`docs/architecture/components/combat-v2.md`; ignored browser evidence.
**Interfaces:** existing canvas `onHoverEntity(member|null)` callback through
production composition, unchanged action/target IDs and RPC signatures.
**Behavior:** arm an offered action through the real controller, hover a canvas
candidate, render its supplied effects, and retain the exact same map/selection.
No input inferred from effect text. Data refresh changes rows, not authority.
**Tests:** joined SessionEncounterView regression invokes the actual canvas prop
callback; effect content appears before a target click; RPC spies remain zero.
Browser exercises mouse transfer from target to panel, real wheel/keyboard
scrolling with no click-through or camera motion, target switches, stale/withdrawn
fixtures, keyboard/touch and narrow/short frame bounds. Fixture proof is labelled
as such; a live provider-backed run remains an integration acceptance item in
[delivery.md](delivery.md), not a fabricated completed check.

- [x] Add joined-route assertions with generated provider data.
- [x] Read browser screenshots; record commands, callbacks and geometry.
- [x] Run full `npm run ci-check` before publishing the web PR.
- [x] Independent review with published findings/dispositions.

**Verification:** focused route tests, actual browser renderer, then complete
local gate at PR boundary. No automatic merge/deployment of this follow-up.
**Evidence:** joined route and production-renderer fixture proof recorded below; independent closure published. Live provider delivery remains a separate owning-team acceptance item.

## Coverage and seam check

| Requirement | Task | Proof |
|---|---|---|
| Hover/focus reveals target effect answers | H1,H2 | Two-candidate overrides/held rows + joined canvas callback |
| Inspection is not selection or execution | H1,H2 | Callback/RPC counts, unchanged picks, panel click/wheel ownership |
| Missing/stale/ambiguous data stays honest | H1 | Generated absence, freshness and uniqueness tests |
| Full readable information remains available | H1,H2 | Info controls, focus, Close/Escape and bounded scroll |
| No new provider or proto work | H1,H2 | Existing props/messages only; named owning-team handoffs |

| Producer | Consumer | Contract | Availability / check |
|---|---|---|---|
| SessionCanvas | Existing route → MapFirstTargeting | `member|null` hover identity | Already wired; route test proves forward path |
| Current declaration/candidate | `effectLinesFor` / `heldEffectLinesFor` | ID-matched actor answer vs full target-held row | Existing wire; unique candidate required |
| Shared effect helpers | Peek/full reader | Verbatim descriptions, states, reasons, benefits | No folding or extra knowledge |
| UI gesture | Existing selection controller | Original choose/confirm callbacks only on deliberate controls | Hover/focus must not call them |

Plan check: preserve initial closed information with no hover, keep explicit
inspection distinct from automatic opening. Any window with scrollable content
must accept reading input; it must not click through to the map. No second target identity,
inspection RPC or condition catalogue is introduced.

## Interaction correction — reachable scrolling

The operator found that the automatic window displayed a scrollbar but could
not be entered with the mouse. The old pointer-transparent policy below is
historical evidence, not current acceptance. Replace it with a stable,
pointer-reachable and keyboard-focusable reader. Preserve the last named valid
candidate during pointer travel, add a Close preview control, and prove that
scroll/click/close change neither selection nor the camera. A real browser wheel
input and hit-test assertion are required; programmatically setting scrollTop
would not prove this repair. No toolkit/API scope changes.

## Initial checkpoint (before the interaction correction)

Web [PR#1241](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1241), head
`713056cc670eba5702ddafb9f5e7ab34d595a339`, implements H1 and the consumer checks
in H2. Six new component regressions plus a joined route extension cover the
new behavior; the initial four hover tests failed against the old implementation.
Focused component/helper/route tests pass161/161, and complete local CI passes:
569 files/1 skipped,7,798 tests/5 skipped. Logs: `/tmp/target-hover-ci.log`,
`/tmp/rpg-dnd5e-web-ci-check.elFc8L/tests.log`.

Browser proof uses production SessionCanvas and combat components with generated
fixture offers, at1440 desktop and1000×900/501. Guard hover shows its held Faerie
Fire separately from the actor's target-specific Sneak Attack answer; the
unavailable archer shows a different answer and no inherited held rows. Peeks are
pointer-transparent, take no focus/intent, and hide over other dock inspections.
Explicit full-reader keyboard scrolling/closing, keyboard list preview and
native touch Info/Close remain read-only; a deliberate scalar map click still
emits the original fixture intent and the canvas node remains identical. A stale
fixture change withdraws the old targeting scope. Screenshots were inspected;
no page errors/gameplay HTTP requests or tested viewport overflow.

Evidence: ignored web `evidence/target-hover/` browser/touch scripts, JSON and PNGs.
Preview: `http://localhost:3041/?concept=desktop-hotbar&preview=1`, Martial profile,
Longsword, hover a target. Private runtime roots were synced with pinned catalog
subtrees; no licensed binaries enter the public diff. Independent review is
running. Real provider-backed description delivery remains the separate gate
listed in delivery.md; no follow-up merge/deployment is claimed.

## Independent review follow-up

The [initial review](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1241#pullrequestreview-5464733774)
found no Critical/Important issues and four minor coverage suggestions. All four
have Addressed replies at `2e8cdb7db7de316da31deda81a5552e21b418780`: unfiltered
focusin suppression, same-ID provider refresh, a new action under the same
hovered member, and described-by linkage while peeking/pinned/suppressed. Only
the test file changed; production and browser-proof code are identical to the
reviewed implementation. The added assertions pass, and full CI is green:
569 files/1 skipped,7,800 tests/5 skipped. Logs: `/tmp/target-hover-review-ci.log`
and `/tmp/rpg-dnd5e-web-ci-check.MPypVD/tests.log`.

[Retained independent closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1241#pullrequestreview-5464797461)
is published and read back at `2e8cdb7d`. All four coverage dispositions were
re-verified; no new findings, Critical or Important issues. All four threads are
resolved. The reviewer reran26 focused tests, typecheck and formatting/lint on
the changed test file. Parent inspected pixels; reviewer checked code and DOM
observation evidence rather than claiming image-reading capability. GitHub's
checks now all pass on the same head, alongside the full local gate. This UI
follow-up is ready for the operator's merge decision, not merged/deployed.

## Base-facts walkthrough refinement

The operator's target-panel screenshot clarified that the same action base
facts belong above the target peek's effect lists. `d358cc82` extracts the
shared `ActionInformationFacts` renderer and uses the declaration's supplied
details in both paths; it does not derive target-adjusted damage. The preceding
concept-only `6c2ac388` supplies an illustrative Longsword payload, not a runtime
lookup or character-stat inference. The ordering regression failed before the
peek change; browser screenshot was inspected and inspection remains inert.

Current head `d358cc825ed6d7759212a5c5bf775e32eaee90d2` passes the full local gate:
569 files/1 skipped,7,801 tests/5 skipped, plus format/lint/typecheck/build.
All GitHub checks pass. [Independent delta review](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1241#pullrequestreview-5465596712)
is published/read back at this head:69 focused tests, typecheck/lint/format and
mutation checks; no Critical/Important findings or requested corrections.
The same-role review replaces the unavailable retained run after restart.
UI ready for the operator's merge decision; provider delivery and deployment
remain distinct.
