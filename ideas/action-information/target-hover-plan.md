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
provider effect answers without selecting/confirming or obscuring map clicks;
explicit full inspection stays available. No general condition browser, hidden
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
  candidate updates `preview`; a foreign/ambiguous ID clears it. Leaving the map
  (`null`) keeps the last preview readable, never selects it.
- With no explicit full reader, a valid preview shows a pointer-transparent
  `role=tooltip` containing the action/target context, provider actor-effect
  answers for that target and separate target-held rows. Missing rows say no
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
change, selected chips, focus, touch-not-auto-hover, pointer-transparent preview,
explicit full-reader priority and Close/Escape. Keep prior selection/confirmation
regressions intact.

- [ ] Add regressions and observe the intended pre-change failure.
- [ ] Implement the UI-only state/render path and bounded preview style.
- [ ] Run focused tests and typecheck; retain compact-target parity.

**Verification:** web cwd `npm run test:run -- src/components/session/combat-experience/MapFirstTargeting.test.tsx src/components/session/combat-experience/memberTargeting.test.ts`; `npm run typecheck`.
**Evidence:** pending.

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
Browser exercises mouse transfer from target to toolbar, transparent peek (a map
click is not stolen), full-reader scrolling, target switches, stale/withdrawn
fixtures, keyboard/touch and narrow/short frame bounds. Fixture proof is labelled
as such; a live provider-backed run remains an integration acceptance item in
[delivery.md](delivery.md), not a fabricated completed check.

- [ ] Add joined-route assertions with generated provider data.
- [ ] Read browser screenshots; record commands, callbacks and geometry.
- [ ] Run full `npm run ci-check` before publishing the web PR.
- [ ] Independent review with published findings/dispositions.

**Verification:** focused route tests, actual browser renderer, then complete
local gate at PR boundary. No automatic merge/deployment of this follow-up.
**Evidence:** pending.

## Coverage and seam check

| Requirement | Task | Proof |
|---|---|---|
| Hover/focus reveals target effect answers | H1,H2 | Two-candidate overrides/held rows + joined canvas callback |
| Inspection is not selection or execution | H1,H2 | Callback/RPC counts, unchanged picks, pointer-transparent peek |
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
inspection distinct from automatic pointer-transparent peeking, and do not let a
new overlay intercept the original map-target click. No second target identity,
inspection RPC or condition catalogue is introduced.
