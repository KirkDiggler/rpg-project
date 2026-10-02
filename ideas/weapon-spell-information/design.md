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
- In-play information describes the character's current action, including active
  effects that change it; base character numbers alone are insufficient.
- Rage, Bless and other applicable effects are part of the information scope,
  not a later tooltip enhancement.
- Toolkit owns rules and their derived facts. API transports them; web renders
  them and sends intent without reconstructing game rules.
- Session loads and carries inputs and projects answers; it does not calculate
  modifier eligibility or combine game arithmetic.
- Resolution assembles current-action information from supplied action, character,
  effect and encounter context using rule-owned contributions.
- Each contributing rule owns its applicability and contribution. Resolution owns
  their composition; encounter owns spatial facts and observations.
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
- The effort supplies information to character creation and play. It does not
  depend on a general UI redesign.
- Component boundaries and extensibility govern the design before delivery
  slices. Future target-aware information tests the shape without automatically
  requiring its UI in the first delivery.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Catalog facts for choosing; character-specific facts for using; both surfaces in scope | KirkDiggler | 2026-10-02 |
| R2 | settled | Current-action information includes active effects such as Rage and Bless, not only base numbers | KirkDiggler | 2026-10-02 |
| R3 | open | Shared contribution mechanism, ownership and side-effect-free read contract | — | — |
| R4 | open | Target-dependent and unresolved information; observer-visible limits | — | — |
| R5 | open | Information payload, catalog coverage and end-to-end acceptance | — | — |
| R6 | settled | Prioritize component shape, composition and future fit over selecting immediate UI slices | KirkDiggler | 2026-10-02 |
| R7 | settled | Session carries inputs and answers; resolution assembles supplied context using rule-owned contributions | KirkDiggler | 2026-10-02 |
| R8 | settled | Contributions, advance opportunities and frozen offers are distinct; Inspiration is noted before acting and asked at its post-roll pause; resume preserves settled facts | KirkDiggler | 2026-10-02 |

## Open

- **R3 — One source for explanation and execution.** Define the contribution
  contract within R7's ownership, preserving modifier ordering, stacking and
  consumption. Reading information must not accidentally run an action. The
  exact extraction and interfaces need agreement before implementation. R8 fixes
  the opportunity/offer distinction and pause/resume custody. The working
  [toolkit contract proposal](toolkit-contract.md) names the proposed types and
  the normalization/ordering decision.
- **R4 — What is known before choosing a target.** Specify how current effects,
  applicable contributions and target-dependent conditions are distinguished.
  Design how selected-target context can refine an explanation and how visibility
  constrains it. Keep that architectural fit separate from deciding when the
  selected-target UI ships.
- **R5 — What crosses the wire.** Settle prose versus structured fields, sources
  and conditional explanations, and completeness for currently offered content.
  Missing descriptions and unavailable mechanics must not be conflated. Define
  the proof through normal creation, play, effect changes and reload.
