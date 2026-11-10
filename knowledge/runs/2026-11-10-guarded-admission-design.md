# Run — 2026-11-10 — Guarded admission design

Design pass for [rpg-project#769](https://github.com/KirkDiggler/rpg-project/issues/769). Base: `rpg-project` `main` at [`b2a2dda`](https://github.com/KirkDiggler/rpg-project/commit/b2a2dda62384684afe557392f0b72b93c6f60b12). Code references are unchanged toolkit `e4ac31f8`, API `2253b33` and web `3be3c439` snapshots.

## Shipped

- No runtime implementation or deployment; the deliverable is a proposed ownership/lifecycle walkthrough, law draft and visual map, with open contracts labeled rather than treated as approved — [design scope](https://github.com/KirkDiggler/rpg-project/issues/769#issuecomment-6756047035).

## Decided

- KirkDiggler accepted one acting character's existing Speak/check path for the first playable admission delivery, retaining shared entrance/party consequences and deferring shared voting as a prerequisite, on 2026-11-10 — [participation ruling](https://github.com/KirkDiggler/rpg-project/issues/769#issuecomment-6756203706).
- Keep shared interaction progress separate from personal knowledge and each guard's private behavior in the proposal; ownership, lifecycle and new types remain open for review — [proposed contract](../../ideas/guarded-admission/design.md#law).

## Found

- Nine encounter controls and two actual session castle controls pass in fresh race runs; they prove existing stop/response/pass/persistence behavior, not a completed admission interaction — [reconciliation and exact tests](https://github.com/KirkDiggler/rpg-project/issues/769#issuecomment-6756203706).
- An ordinary open resolution window freezes every member's verbs, so reusing it for a mere invitation would change unrelated players' ability to act — [current Afford gate](https://github.com/KirkDiggler/rpg-toolkit/blob/e4ac31f8/rulebooks/dnd5e/session/afford.go#L592-L614).
- The existing immediate bump response is an explicit caller-granted opportunity; other observations use Observe and cannot silently become paid turns, which the proposed approach contract must preserve — [current reaction boundary](https://github.com/KirkDiggler/rpg-toolkit/blob/e4ac31f8/rulebooks/dnd5e/encounter/wakebeats.go#L403-L450).
- Light, dark and narrow overview renders produce no page errors or horizontal overflow; focused law scan has zero findings across one file, while the workspace scan retains44advisories across89files and exits1 — [overview and proposal](../../ideas/guarded-admission/overview.html).

## Mistaken

- The first scoped genre invocation used a child-directory symlink and scanned zero files; checking the count exposed the empty verification, and a byte-equal temporary law copy produced the real one-file check instead — [design-pass evidence home](https://github.com/KirkDiggler/rpg-project/issues/769).
