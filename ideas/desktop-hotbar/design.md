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
- **R4 — Rows and paging.** The player selects one through four balanced rows, initially one. Short final pages overlap the preceding tail. Narrow sections scroll rather than change the chosen row count.
- **R9 — Slice boundary.** This live-bar slice excludes favorites, star/edit controls and favorite identity or persistence work. Favorites remain a follow-up design question informed by play with the bar.
- **R5 — Status and inspection.** HP, AC and movement occupy fixed Status presentation. Action inspection shows provider-supplied base action information, including when the action has no effects, with contextual effects alongside it when present. Inspection is informational; executable features remain commands. Provider-authored command options appear in a tray without replacing the toolbar.
- **R12 — Inspection context.** With no action selected or inspected, the contextual window has no action context; it does not borrow an arbitrary attack. For an inspected action, an empty effect list does not suppress its base information. Missing provider facts are not fabricated.
- **R6 — Member targeting.** Map selection and the optional list share one selection. Selected members have visible map markers and removable chips. Multi-target actions require separate confirmation; re-clicking an armed multi-target command preserves its picks. Candidate membership, availability and bounds come from the provider.
- **R7 — Feedback.** The log is optional and initially closed. Temporary notices present released actor/action/result story entries, with at most three visible for six seconds, without deleting those entries from history. Initial and recovered history does not replay as new notices. Debug inspection does not reflow the toolbar or pass clicks through to the map.
- **R8 — Assets.** Licensed source and runtime artwork remain in the private canonical asset store or ignored local previews, never in public source control. Production consumes reviewed runtime assets through the owning asset contract.
- **R10 — Live metadata.** Display classification joins exact existing provider refs. Unclassified offers remain visible; unknown spell kinds remain in Other spells. Inventory does not mint item-use offers. Inspection displays available provider facts without fabricating missing descriptions or attack statistics.
- **R11 — Live interaction.** The controller retains current-selector, authority, option and candidate checks. Desktop member-list selection uses existing CAST list input; scalar verbs retain their existing protocol. Unsupported list shapes fail closed rather than dispatching a partial action. Cancelled, replaced or invalidated selections cannot act through delayed callbacks.
- **R13 — Activation.** The real encounter uses the desktop bar in desktop-sized containers and the existing surface in smaller containers. Responsive changes do not remount the map or execute an action. Merge and deployment remain separate gates.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Existing workspace authority boundary | Workspace law | — |
| R2 | settled | Accepted desktop concept and mobile preservation | KirkDiggler | 2026-10-07 |
| R3 | settled | Offer-driven sections and spell grouping | KirkDiggler | 2026-10-07 |
| R4 | settled | Rows and pagination; favorites excluded by R9 | KirkDiggler | 2026-10-08 |
| R5 | settled | Status, base action information plus contextual effects, and option tray | KirkDiggler | 2026-10-08 |
| R6 | settled | Targeting interaction; not new RPC capability | KirkDiggler | 2026-10-07 |
| R7 | settled | Optional history and transient feedback | KirkDiggler | 2026-10-07 |
| R8 | settled | Existing licensed-asset boundary | Workspace law | — |
| R9 | deferred-until-post-bar-playtest | Favorites and their identity, ownership and lifetime; excluded from this slice | KirkDiggler | 2026-10-08 |
| R10 | settled | Existing provider joins and honest absence; no new content contract in this slice | ui-ux, derived from R1/R3 and accepted slice | 2026-10-08 |
| R11 | settled | Existing live RPC shapes and fail-closed lifecycle; no new game capability | ui-ux, derived from R1/R6 and existing controller contracts | 2026-10-08 |
| R12 | settled | No arbitrary idle context; action information remains visible without effects | KirkDiggler | 2026-10-08 |
| R13 | settled | Responsive live bar; mobile preserved; no automatic merge/deploy | ui-ux, derived from KirkDiggler's live-bar goal and R2 | 2026-10-08 |

## Open

- **R9 (deferred, non-blocking):** Does play with the bar justify favorites? If so, what follows the character across encounters, refreshes and devices, and which provider identity distinguishes offers without persisting executable declaration IDs?
- **R10 (follow-up, non-blocking):** Richer attack statistics and option descriptions need a provider contract when that information is added. The live bar does not claim that existing declaration fields supply it.

No unresolved architectural decision blocks the defined slice. The [implementation plan](implementation-plan.md) names the concrete joins, lifecycle checks and verification.
