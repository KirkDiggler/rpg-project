# Desktop hotbar: integration investigation

Working record for [web #1225](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1225), the [walkthrough](README.md) and [law](design.md).

## Brief and authority

Bring the accepted desktop bar into the live experience: visible commands, readable inspection, map-first selection, and unobtrusive history without a mobile regression. The [initial concept acceptance](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1225#issuecomment-6038061412) was not a promotion authorization. On 2026-10-08, the operator narrowed the live slice: leave favorites out and learn from using the bar before deciding what customization is useful.

Favorites, star/edit controls and favorite identity/persistence are explicitly excluded, not unresolved blockers. Rows and paging remain in scope. The concept's favorite experiment may remain available separately; it is not silently enabled on the live route.

This document is investigation, **not a checked implementation plan**. No live behavior changes are made by these documents. R10–R13 remain architectural questions, not tasks silently delegated to an implementer; R9 is deferred and non-blocking. Resolve the affected live contracts, then derive task contracts, requirement coverage and provider/consumer seam checks using the design skill's planning guide. Merge and deployment remain separate gates.

## Inspected baseline

| Repository / surface | Revision | Relevance |
|---|---|---|
| Web concept worktree, `feat/desktop-hotbar` | `4b57ce973c369ac3a556ea0141b473501d8ae675` | Accepted renderer, fixture selection, local favorites and category mounting |
| Web base of concept | `7d4bcb1e9f9cc5be46504f7ddd62c034d55b40d6` | Live controller and encounter inspected in the concept worktree |
| Fetched web `origin/dev` | `08e77999a29e4e2e272ac40bd6043e2344032e52` | Includes discovery-sharing and character-creation spell-description changes absent from the concept base |
| Project design worktree | `f7813385e01c934c9b5d172cd7356b48dc5d55ef` | Updated design skill, walkthrough guide and planning guide |

Web changes must reconcile the current encounter/discovery/story code rather than overwrite it with the older concept base. The new spell-description work addresses character creation, not proof that `Declaration` or `CastOption` supplies combat explanation text.

## Measured seams and consequences

| Seam | What inspection establishes | Consequence / unresolved decision |
|---|---|---|
| `liveActionPresentation` → desktop grouping | Existing adapter joins known cantrip/spell refs and exact feature refs for organized HUD hints. Desktop renderer additionally consumes `desktopSectionByDeclarationId` and `desktopSpellKindByDeclarationId`. | Do not enable the new mode without a complete live mapping and honest unknown/absence behavior (R10). |
| Preferences → current offers | `DesktopHotbarLayout.favoriteIdsBySection` stores fixture declaration IDs; `currentFavorites` filters against current IDs. The live controller treats selectors as generation-bound offers. | Persisting this object directly would lose or misbind favorites across refreshed offers. Exclude favorites and their controls from this live slice; defer identity and storage until a later design (R9). |
| Map/list → controller | Concept uses `toggleMemberTarget` and committed-snapshot callback protection. Live `onTargetClick` appends CAST members, rejects already selected members, and immediately dispatches single-target CAST. | Multi-target deselection and re-click preservation require real controller work; copying JSX is insufficient (R11). |
| Confirmation → RPC | Live `onConfirmTargets` delegates to `onCastTargets`; that path requires CAST/MEMBER, current option, unique current available candidates and provider bounds. | A generic multi-member fixture is not evidence of non-CAST list dispatch. Inventory supported live contracts before extending scope (R11). |
| Refresh → pending selection | Controller compares the armed selector to current declarations and gates authority/turn. Move alone has an explicit selection-mode remapping path. | Decide pending-pick invalidation without transferring stale executable IDs or copying Move's special lifecycle blindly (R11). |
| Effects → inspection | `DesktopEffects` consumes one declaration and optional target through `effectLinesFor`. Concept may supply `desktopEffectsDeclarationId`. | No global passive catalog is established. Settle the unselected context (R12). |
| Options → tray | `CastOption` supplies an opaque ID and label. | Detailed option explanation requires an owning provider contract, not client-written spell rules (R10). |
| Stream → notices | Stream callback retains per-event `live`/`catchup` metadata. `useStoryNotices` receives story entries, scope and aggregate stream state, not per-entry provenance. | Prove background catch-up, not just initial mount/reconnect. Aggregate live state alone cannot establish that an added entry should announce. Preserve release pacing while threading the needed provenance. This is an integration gap answered by R7, not permission to weaken it. |
| Art → deployed build | Preview art uses ignored `interface-preview` files; private provider has reviewed runtime roots; Docker packaging invokes runtime sync. | Promote the selected set through the asset owner before claiming build availability. A local successful image load proves no release custody (R8). |

## Integration approaches to judge

1. **Presentation-only activation.** Reuse the live controller unchanged and enable desktop icons. Small diff, but it cannot satisfy accepted target toggling and leaves metadata unresolved. It is not a complete promotion.
2. **Shared presentation plus explicit live seams — recommended.** Keep `CombatExperience`, the live controller and session provider ownership. Fill the proven adapter/controller/provenance gaps and settle missing metadata contracts explicitly. Keep favorites and preference storage out of the live slice. The cost is provider/consumer work where the facts are absent, rather than a frontend illusion of completeness.
3. **Carry the fixture harness into production.** It reproduces the demonstration but creates a second interaction owner without real authority/refresh guarantees. Reject: the fixture is a test consumer, not the live controller.

These alternatives explain the integration choice; they do not reopen accepted layout behavior.

## Observable acceptance and future plan coverage

| Requirement | Concrete joined-path proof required | Current planning status |
|---|---|---|
| R1, R3 | Real offers gain/lose categories without class branches; unavailable offers retain their category and provider refusal. Unknown refs do not invent a type or disappear. | Live metadata contract R10 open |
| R2, R4, R9 | Live desktop at wide/narrow/short viewports; 1–4 balanced rows and overlapping final pages. No favorite stars, favorite-edit controls or preference storage are mounted/written. Mobile retains its controls and interaction. | Favorites explicitly excluded; activation R13 open |
| R5 | Current private status, stale/unavailable status, contextual effect inspection and actual option IDs displayed on the real encounter route. Missing descriptions stay honest. | R10/R12 open |
| R6 | Real multi-target CAST: map/list/chips agree; self/allies are selectable when offered; repeated command click preserves picks; separate confirm sends exact IDs once. Cancel/rearm, delayed clicks, scope changes and authority refresh cannot execute stale picks. | R11 open; fixture proof is not live proof |
| R7 | Live event announces once after presentation release; initial, reconnect and periodic/focus catch-up entries remain history-only. Expiry leaves history; scope changes do not leak cards. Discovery narration from current dev remains intact. | Provenance integration needs task contract |
| R8 | Clean production-style asset stage loads all selected icons, checks private custody, and provides readable missing-image behavior without public binary additions. | Provider selection/promotion needed |
| R13 | Real route and mobile walk, complete `npm run ci-check`, independent PR review verdict and all relevant released provider pins. | No promotion-ready claim; no PR opened |

## Sequence and review checkpoints

Explain the shape and decide only unresolved architecture with the operator, one decision at a time. Then make a checked plan naming exact files, introduced/consumed contracts, negative tests and runnable commands. Reconcile against current web dev before implementing it.

Development follows the consumer need outward to any missing provider primitive. Any required provider contract, generated bindings, asset release or dependency pin must be named in the plan; do not presume the work is web-only. Merge dependencies go inside-out, and released pins need joined verification. No new provider work is authorized by this investigation alone.

The final web PR boundary requires a complete `npm run ci-check`, then the applicable independent review round with its published verdict, plus real-route visual/interaction verification. Focused concept checks do not replace those gates. Neither a passing gate nor design agreement authorizes merge or deployment.

## Documentation checks

The walkthrough separates ownership, typed contracts, refusals and costs from these revision-bound findings. The law keeps four sections. A source-map path check and genre scan accompany publication; they are documentation checks, not a claim of application CI or an independent design review.
