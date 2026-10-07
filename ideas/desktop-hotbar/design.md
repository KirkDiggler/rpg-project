# Desktop hotbar

## Shape

```mermaid
flowchart LR
    Facts[Session declarations and character data] --> Adapter[Live presentation adapter]
    Facts --> Controller[Combat interaction controller]
    Adapter --> HUD[Shared combat presentation]
    Controller --> HUD
    HUD -->|selection and confirmation intent| Controller
    Controller -->|current opaque selectors| API[Session RPCs]
    Assets[Private runtime artwork] --> HUD
    Stream[Session events and recovery] --> Story[Story and pacing]
    Story --> HUD
```

## Law

- **R1 — Authority.** The client renders provider facts and sends intent. Presentation does not invent offers, descriptions, effect applicability, or game legality. Existing [workspace boundaries](../../CLAUDE.md#what-the-foundation-is) govern every layer.
- **R2 — Desktop presentation.** The desktop surface is map-first, with a full-width translucent toolbar, approximately 24px side cushion, 36px icon controls, and opaque hover/focus inspection. Mobile retains its existing interaction surface.
- **R3 — Organization.** Actions, Features, Spells and Items mount only when they contain current offers. Unavailable offers still count. Cantrips form their own spell group; class, level, names and selector spelling do not determine membership.
- **R4 — Customization.** The player selects one through four balanced rows, initially one, and at most four favorites per section. Favorites remain on every page within their groups. Short final pages overlap the preceding tail. Narrow sections scroll rather than discard favorites or change the chosen row count. Edit mode changes favorites without executing commands.
- **R5 — Status and inspection.** HP, AC and movement occupy fixed Status presentation. Effects & Traits is informational and carries its action/target context; executable features remain commands. Provider-authored command options appear in a tray without replacing the toolbar.
- **R6 — Member targeting.** Map selection and the optional list share one selection. Selected members have visible map markers and removable chips. Multi-target actions require separate confirmation; re-clicking an armed multi-target command preserves its picks. Candidate membership, availability and bounds come from the provider.
- **R7 — Feedback.** The log is optional and initially closed. Temporary notices present released actor/action/result story entries, with at most three visible for six seconds, without deleting those entries from history. Initial and recovered history does not replay as new notices. Debug inspection does not reflow the toolbar or pass clicks through to the map.
- **R8 — Assets.** Licensed source and runtime artwork remain in the private canonical asset store or ignored local previews, never in public source control. Production consumes reviewed runtime assets through the owning asset contract.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Existing workspace authority boundary | Workspace law | — |
| R2 | settled | Accepted desktop concept and mobile preservation | KirkDiggler | 2026-10-07 |
| R3 | settled | Offer-driven sections and spell grouping | KirkDiggler | 2026-10-07 |
| R4 | settled | Rows, favorites, pagination and edit behavior | KirkDiggler | 2026-10-07 |
| R5 | settled | Status, contextual information and option tray | KirkDiggler | 2026-10-07 |
| R6 | settled | Targeting interaction; not new RPC capability | KirkDiggler | 2026-10-07 |
| R7 | settled | Optional history and transient feedback | KirkDiggler | 2026-10-07 |
| R8 | settled | Existing licensed-asset boundary | Workspace law | — |
| R9 | open | Durable preference identity, ownership and lifetime | — | — |
| R10 | open | Live classification and description contracts | — | — |
| R11 | open | Live target-controller coverage and invalidation lifecycle | — | — |
| R12 | open | Default effects context without a selected action | — | — |
| R13 | open | Live rollout scope and activation | — | — |

## Open

- **R9:** What follows the character across encounters, refreshes and devices? Which provider identity distinguishes two offers for the same spell, weapon or feature without persisting executable declaration IDs?
- **R10:** Which exact live fields classify every offered feature/item and distinguish spell kinds, including granted spells? Where do combat descriptions and option explanations come from when the declaration lacks them?
- **R11:** Which live verb contracts support member lists? How do pending picks behave across authoritative refresh, option change, cancellation and scope change while retaining current-selector dispatch checks?
- **R12:** With nothing selected, which provider-backed action context, if any, supplies Effects & Traits? A global passive catalog is a different contract.
- **R13:** How is the desktop surface activated on the live route, and what is the rollback boundary? Concept acceptance alone does not settle live activation, merge or deployment.
