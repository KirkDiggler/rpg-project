# Effect information — consumer trace and acceptance

Working design record, not an implementation-ready plan. The player sees which
effects can apply and can inspect what they do. Numerical terms remain the shared
rule machinery's evidence, not a requirement for a new results-preview panel.
Authority: R7–R15 in [design.md](design.md) and the operator's clarification on
[#521](https://github.com/KirkDiggler/rpg-project/pull/521).

## What the player should see

| Provider answer | Presentation | Tooltip content |
|---|---|---|
| Applicable contribution | Effect shown as active for this action/context | Canonical name and description; rule-authored contextual detail where useful |
| Ineligible contribution | Same effect shown gray, not silently removed | Canonical description plus the rule's reason |
| Context not yet sufficient | Conditional indicator, not a definite rejection | Canonical description plus what is not yet established |
| Optional benefit | Available opportunity, not a committed bonus | Canonical description, timing and consequence of choosing; no early offer token |
| Unsupported assessment | Information unavailable, not “does not apply” | Preserve known canonical content; no fabricated eligibility or complete calculation |

The description answers “what is Sneak Attack?” The contextual reason answers
“why can/cannot it apply here?” They are separate provider fields. A reason such
as “already used this turn” is not a replacement for a description of the effect.
Gray styling must not disable access to the tooltip. Effect applicability does
not enable or disable the Attack button; command permission remains independent.

## Inspected sources

- Toolkit `3955eaf7094c56a713ae055f44600c125081c4ce`.
- Web `af4ee892` (dev); API `858c3e5a` (dev); protos `e80efe0` (main).
- Read-only consumer worktrees: each repository's `.worktrees/contributions-contract`.
  These are checkout/source observations, not claims about the deployed stack.

| Seam | Existing source | Finding / consequence |
|---|---|---|
| Action tooltip | Web `src/components/session/combat-experience/actionTooltip.ts:buildActionTooltip` | Projects declaration facts; no effect list. Keep the no-client-rules boundary. |
| Tooltip accessibility | Web `ActionDock.tsx:ActionTooltipCard`, `ActionDescription` in the same directory | Visible card and separate accessible description already exist; gray effects need equally inspectable content, not disabled tooltip controls. |
| Target surface | Web `TargetSurface.tsx` in that directory | Candidate buttons call `onTargetClick`; only multi-target casts have a confirmation step. |
| Target command | Web `useSessionCombatExperience.ts:onTargetClick` in that directory | Attack dispatches immediately after choosing a valid target. A one-target cast calls `runCastTargetsRef.current` immediately. There is no general pre-commit selected-target dwell state. |
| Map hover | Web `src/components/session/SessionCanvas.tsx:SessionScene` | `meshHoveredSubject` and ground hover resolve `hoveredEntityId`; the existing `onHoverEntity` prop reports it outward and has tests. `SessionEncounterView` does not currently connect that prop to effect information. Reuse it, not the unrelated old `HexGrid` route. |
| Freshness | Web `src/api/useSessionAfford.ts`; `SessionEncounterView.tsx` | Current key/generation wins; delivered events invalidate authority and schedule reads. Effect information needs its own scoped freshness, not a permanent ref/selector-only cache. |
| Wire | Protos `dnd5e/api/session/v1alpha1/types.proto:Declaration`, `AttackRef`, `SpellRef`, `TargetCandidate` | Existing fields do not carry contextual effect assessments or descriptions. Do not reconstruct them from condition names in web. |
| API | `internal/handlers/dnd5e/session/v1alpha1/afford.go:Afford` | Authenticates acting member and maps SDK input/output. New information uses the same host trust boundary; no rules belong in this handler. |
| Canonical effect content | Toolkit `conditions/display.go:DisplayFor` | Name plus optional `Detail` exists. Sneak Attack and many other entries have only names. Add truthful tooltip content under the existing content owner; no web ref-to-description table. |
| Own rule state | Toolkit `character` assembly/load and `conditions/sneak_attack.go` | Usage, equipment and governing action facts belong below session. The shared assessor supplies applicability; toolkit#1929 stays a separate owning-rule fix. |
| Observed positions/state | Toolkit `session/read.go:projectView`, `session/types.go:Seen`; encounter testimony | Position, optional observed standing and equipment have explicit observed/unknown semantics. Missing standing is not “up”; remembered position is not current placement. |
| Relationships | Toolkit `encounter/world.go:BelievedStance` | Existing method answers viewer↔subject, not the viewer's knowledge of target↔third-party hostility. Do not infer Sneak Attack eligibility from two UI ring colors or ask a hidden participant's viewpoint. |
| Target effects | Toolkit `session/types.go:Seen` | It is not a target effect snapshot or a full sheet. Any needed effect knowledge must come through a permitted provider projection; seeing a target alone does not permit loading all its rules for informational evaluation. |

All toolkit paths in the table are relative to `rulebooks/dnd5e/`. The relationship
and target-effect rows are concrete R4 provider work, not permission to substitute
live whole-world truth. Unknown facts remain unknown under R13. Geometric queries
stay in encounter; rule applicability stays with its rule; session carries values.

## Settled interaction — R15

The effect indication is **before committing**: hover/focus inspects, clicking
still performs the armed action. The operator accepted this recommendation rather
than adding a select-then-confirm step. The earlier alternatives and source finding
remain in the PR record.

- Connect the existing `SessionCanvas.onHoverEntity`; the target list supplies
  keyboard focus and an explicit read-only inspection control for touch.
- Keep inspection state separate from `selectedCandidateMember`, which currently
  participates in action dispatch. Inspecting never calls `onTargetClick`.
- Keep the inspected target associated with its effect list while the pointer or
  focus moves into that list to read a tooltip. A hover-out event must not make the
  tooltip disappear before it can be read. Clear/change inspection on explicit
  dismissal, a different inspected target, or a changed session/member/action.
- Distinguish the last inspected target from current information freshness. A
  delivered state change invalidates its answer; an older response cannot restore
  it as current. No click is delayed until an informational request completes.
- Leave attack/cast selection, command availability, refusal handling and existing
  multi-target cast confirmation unchanged. Inspection neither adds an action
  confirmation nor dismisses an execution refusal.

## Concrete acceptance to carry into the checked plan

These are expected assertions, not tests claimed to have run:

- **Applicable Sneak Attack:** supply a provider assessment of Applies for the
  inspected action/target. The effect is active and its tooltip contains the
  canonical description. Web does not inspect weapon properties or adjacency.
- **Ineligible Sneak Attack:** same identity/description, provider reason “already
  used this turn.” Show it gray with readable tooltip/reason; send no command just
  by inspecting. Pair with a toolkit test of the actual owning predicate.
- **No target/unknown facts:** a NeedsContext answer remains conditional. Do not
  show “no ally” because a list or target is absent.
- **Bless and Inspiration:** automatic contribution and optional after-roll
  benefit render distinctly; neither informational read consumes or rolls either.
- **Canonical content gap:** a name-only descriptor does not pass the tooltip
  completeness test. Do not add a UI fallback description or promise behavior the
  current owning rule does not implement.
- **Target switch:** a late answer for target A cannot overwrite target B's effect
  list, even with the same action selector. Actor/session/variant changes also
  discard obsolete responses.
- **State change:** a relevant delivered event invalidates current effect
  information. Same-selector responses replace old applicability; reads after
  reload use current authorized actor facts and observed context.
- **Hidden-state variation:** holding all permitted inputs fixed while hidden
  state changes cannot alter rows, reasons, availability or errors.
- **Unsupported rule:** retain description, mark assessment/affected calculation
  unavailable, and leave command eligibility unchanged.
- **Input modalities:** pointer hover, keyboard focus and touch inspection expose
  the same provider effect facts before submission. Moving into an effect tooltip
  preserves the inspected target. Inspection sends zero Attack/Cast/Activate calls.
  A normal target click still dispatches once without a new confirmation or a wait
  for informational completion; existing command/refusal behavior remains intact.
- **No dashboard prerequisite:** prove the effect indicator and tooltip directly;
  a formula panel, hit probability or simulated outcome cannot stand in for this
  acceptance.

With the interaction settled, derive task contracts for canonical effect content,
shared assessments, permitted context, SDK/wire/API transport, UI rendering and
joined acceptance. The existing contribution inventory supplies the rule migration
universe; this trace supplies the consumer requirement. Neither is by itself a
checked implementation plan.
