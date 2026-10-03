# Weapon and spell information

## Shape

```mermaid
flowchart TD
    Session[Session: load and carry inputs] --> Resolution[Resolution: assemble the action explanation]
    Encounter[Encounter: spatial facts and observations] -->|context carried through session| Resolution
    Character[Character: assembled action and current effect state] -->|inputs carried through session| Resolution
    Contributions[Rule-owned contributions] --> Resolution
    Resolution --> Answer[Session: project the answer]
    Content[Canonical weapon and spell content] --> Catalog[Catalog information for choices]
    Answer --> Wire[API contract]
    Catalog --> Wire
    Wire --> Creation[Character creation]
    Wire --> Play[Action bar]
```

The two views answer different questions: what a weapon or spell does, and what
this character can do with it now. They share rulebook authority, not necessarily
an identical payload. Session is the carrier; resolution assembles the current
action's explanation using rule-owned contributions and supplied context. The
arrows show information flow, not Go imports. The contribution contract and exact
wire contract remain open.

## Law

- Character choices expose information about the weapon or spell being chosen.
- Permitted catalogue alternatives remain inspectable after choosing, including
  weapons or spells the player did not select. Inspection changes no selection,
  ownership, equipment, spell access or execution permission.
- Information access is independent of current action affordability. Existing
  offers and refusals remain authoritative for costs and whether an action can
  be performed; the information component does not duplicate those decisions.
- In-play information describes the character's current action, including active
  effects that change it; base character numbers alone are insufficient.
- Rage, Bless and other applicable effects are required in the current-action
  scope. The catalogue increment does not claim to deliver that live explanation.
- Toolkit owns rules and their derived facts. API transports them; web renders
  them and sends intent without reconstructing game rules.
- Session loads and carries inputs and projects answers; it does not calculate
  modifier eligibility or combine game arithmetic.
- Resolution assembles current-action information from supplied action, character,
  effect and encounter context using rule-owned contributions.
- Each contributing rule owns its applicability, contribution and explanation.
  Resolution composes the rule's answer; encounter owns spatial facts and
  observations. Information consumers do not implement effect-specific eligibility
  or compensate for defects in an owning rule.
- Action-fact normalization is explicit: settle the applicable die and ability
  before evaluating contributions that depend on them. Handler registration
  order does not implicitly define that dependency.
- A contribution already participates in a calculation; an opportunity describes
  a possible later choice; a posed offer is a concrete question on a frozen
  interaction. They are distinct data, not interchangeable modifiers.
- Bardic Inspiration is noted before acting as an after-roll opportunity, not
  included in the committed attack formula or spent by an informational read.
- The Inspiration question occurs after the attack roll and before revealing its
  outcome. The question exposes the current calculation, not the target's AC or
  unpublished success/failure result.
- Starting an action evaluates current authoritative state. Resuming a paused
  action preserves the frozen calculation and applies the answer without
  re-evaluating or rerolling completed stages.
- Accepting an Inspiration offer rolls its die and consumes the benefit;
  declining preserves it. Inspecting either view changes nothing.
- The effort supplies information to character creation and play before an action
  is committed. Existing combat-log traces are its consistency reference, not a
  new presentation feature. It does not depend on a general UI redesign.
- The player-facing proof is the applicable effect indication and its canonical
  tooltip. Ineligible effects remain inspectable in a grayed-out state with the
  rule's reason; unresolved applicability remains conditional. A contextual reason
  does not replace the description of what the effect is.
- Current-action information explains effects in play and their contributions,
  not predicted results or a separate calculation dashboard. Its mechanical scope
  is attack-roll contributions,
  normal/critical-hit damage before target defenses, spell save DC or potential
  healing as appropriate, and conditional contributions and optional benefits.
- Selected-target context refines contributions only where permitted facts can
  establish applicability. Informational reads simulate no rolls and predict no
  hit probability or final HP loss after defenses and reactions.
- Component boundaries and extensibility govern the design before delivery
  slices. Future target-aware information tests the shape without automatically
  requiring its UI in the first delivery.
- Catalogue and current-action information share canonical content ownership and
  references. Their connection is designed together; catalogue information ships
  as an independently useful first increment, followed by current-action work.
- Catalogue reads require no dummy character, invented modifiers or resolution
  interaction. Later action explanations reuse the content authority instead of
  creating a second description/mechanics catalogue.
- Player-facing assessments use the character's permitted knowledge, not hidden
  authoritative target state. Seeing a target does not grant its full sheet.
- Hidden state cannot alter informational totals, applicability, sources, pending
  rows or error shape when the permitted inputs are identical.
- Insufficient knowledge remains unresolved rather than a definite negative.
  Execution uses authoritative state; an explanation is not a guaranteed outcome.
- An unsupported assessment makes the affected calculation unavailable, never an
  apparently complete total with the unsupported contribution omitted.
- Assessment unavailability preserves the action's content description and does
  not change independently established execution permission. Missing context and
  rule-established ineligibility remain distinct from unsupported evaluation.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Catalog facts for choosing; character-specific facts for using; both surfaces in scope | KirkDiggler | 2026-10-02 |
| R2 | settled | Current-action information includes active effects such as Rage and Bless, not only base numbers | KirkDiggler | 2026-10-02 |
| R3 | open | Shared contribution mechanism, ownership and side-effect-free read contract | — | — |
| R4 | open | Concrete target/context projection and refinement contract within R13's visibility boundary | — | — |
| R5 | open | Information payload, catalog coverage and end-to-end acceptance | — | — |
| R6 | settled | Prioritize component shape, composition and future fit over selecting immediate UI slices | KirkDiggler | 2026-10-02 |
| R7 | settled | Session carries inputs and answers; resolution assembles supplied context using rule-owned contributions | KirkDiggler | 2026-10-02 |
| R8 | settled | Contributions, advance opportunities and frozen offers are distinct; Inspiration is noted before acting and asked at its post-roll pause; resume preserves settled facts | KirkDiggler | 2026-10-02 |
| R9 | settled | Explicit normalization of action facts before evaluating dependent contributions | KirkDiggler | 2026-10-02 |
| R10 | settled | Information remains inspectable independently of affordability and after selection, including unchosen alternatives; existing affordability UI/authority remains unchanged | KirkDiggler | 2026-10-02 |
| R11 | settled | Design the shared connection now; deliver catalogue independently first, then current-action/resolution information | KirkDiggler | 2026-10-02 |
| R12 | settled | Unsupported assessment makes the affected calculation unavailable; retain description and independent action legality | KirkDiggler | 2026-10-03 |
| R13 | settled | Explanation uses permitted character knowledge with no hidden-state inference; unknown stays unresolved; execution uses authoritative state | KirkDiggler | 2026-10-03 |
| R14 | settled | Explain participating effects/contributions, not predicted outcomes: attack, pre-defense normal/critical damage, spell DC/healing, conditions and opportunities; permitted target facts refine applicability | KirkDiggler | 2026-10-03 |

## Open

- **R3 — One source for explanation and execution.** Define the contribution
  contract within R7's ownership, preserving modifier ordering, stacking and
  consumption. Reading information must not accidentally run an action. The
  exact extraction and interfaces need agreement before implementation. R8 fixes
  the opportunity/offer distinction and pause/resume custody; R9 fixes explicit
  normalization before dependent contributions. The working
  [toolkit contract proposal](toolkit-contract.md) names the proposed packages;
  the [assessment contract](assessment-contract.md) refines their interfaces,
  phase boundaries and per-facet coverage. The linked
  [coverage inventory](contribution-coverage.md) records the current producers and
  migration proofs. These resolution-specific interfaces are not prerequisites
  for R11's catalogue increment.
- **R4 — Concrete permitted context.** R13 settles the visibility boundary, not
  the exact projected fields, their observation sources or the refinement API.
  Define how permitted current effects, spatial facts and target context reach
  the same rule evaluator without exposing hidden sheets. Keep that architectural
  fit separate from deciding when the selected-target UI ships. The catalogue
  increment needs only its permitted content visibility; it does not evaluate
  target facts.
- **R5 — What crosses the wire.** Settle prose versus structured fields, sources
  and conditional explanations, and completeness for currently offered content.
  Missing descriptions and unavailable mechanics must not be conflated. Separate
  information freshness from executable-offer identity. R12 settles unsupported
  assessment behavior and R14 settles the initial calculation scope, not the
  exact error/wire representation or end-to-end acceptance details. R10 settles independent inspection, including unchosen
  catalogue alternatives; it does not specify the transport or authorize
  hypothetical character-build calculations. Define the proof through normal
  creation, play, effect changes and reload. Catalogue delivery does not settle
  the current-action contribution or effect-freshness contracts; each increment
  needs its own acceptance scope under R11. The
  [effect-information consumer trace](effect-info-delivery.md) records the tooltip
  acceptance and the unresolved pre-commit target-inspection interaction.
