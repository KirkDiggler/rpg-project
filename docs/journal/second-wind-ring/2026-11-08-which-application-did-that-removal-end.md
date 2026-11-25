# 2026-11-08 — Which application did that removal end?

Written during [Composable conditions S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706), after [the end-reporting boundary](2026-11-08-what-can-still-happen-while-an-effect-ends.md).

## Question

A replacement address says which slot a condition occupies. Is it enough to identify the application that ended? The linked design deliberately separates origin, address and application identity; the real keeper path has to preserve that separation too.

## Evidence

The [current linked codec checkpoint](https://github.com/KirkDiggler/rpg-toolkit/commit/35bba70f) stores lifetime state only on the coordinator and gives recipients exact backlinks. A mixed roll composition also exposed the old assumption that one condition means one stacking group: contributor sets now feed the existing selector, and plural information uses the same group claimant. The mixed Bless/Bane plus second-Bless test catches a shadowed contribution falsely reported as applying.

The [transport checkpoint](https://github.com/KirkDiggler/rpg-toolkit/commit/1eb2a028) prepares without applying, seals actual delivery receipts and removes effects through real character keepers. Its replacement test first accepted an old removal and deleted a new recipient at the same address. Carrying ApplicationID on linked removal facts, checking it in the binding, and using one shared keeper matcher made that regression pass. Mutants removing the checks failed the bound replacement and standalone keeper cases.

Other tests cover missing, foreign and competing owners; internal versus external end order; self-recipient loss; independent payload survival; empty delivery; and teardown without gameplay ending. Full root race/vet/lint passes. These are root/keeper proofs, not a native spell cast through session/API/browser. The old named concentration path still awaits retirement in the unfinished branch.

## Interpretation

An address is reusable. An application is not the new occupant merely because it uses that address. An old fact needs its invocation identity if it is to remain a fact about what actually ended.

The correction belongs in the fact and the shared keeper/transport seam. Parsing an opaque ID, retaining a second lifecycle, or teaching each spell how to ignore old notifications would leave the underlying identity problem intact.

## Next

Continue the [checked plan](https://github.com/KirkDiggler/rpg-toolkit/pull/2323): retire the old named implementations, update native declaration and execution callers, and prove the joined path. Fresh characters and one current path remain the finish line; no saved-state compatibility route is added. Journal #863 stays open.

— cross-team agent, on behalf of KirkDiggler
