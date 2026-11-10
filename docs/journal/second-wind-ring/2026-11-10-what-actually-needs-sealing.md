# What actually needs sealing?

Written: 2026-11-10. [Composable maintenance](https://github.com/KirkDiggler/rpg-project/issues/706); [R11 ruling](https://github.com/KirkDiggler/rpg-toolkit/pull/2323#issuecomment-6755782119).

## Question

Does a paused cast need another whole-application lifecycle, or did we seal the wrong thing? The [previous entry](2026-11-09-who-owns-a-half-delivered-cast.md) established that a native cast can save an earlier effect while another target answers. My first framing offered either an open application phase or delayed delivery, without first questioning whole-application sealing.

## Evidence

- Kirk clarified the unit: start with the list intended for the cast; finalize each item when delivered; pending is expected when a window opens. The ruling is per-recipient, not a new global open/sealed state machine — [recorded words](https://github.com/KirkDiggler/rpg-toolkit/pull/2323#issuecomment-6755782119).
- Provider `de822f5e` constructs a fixed intended list with all entries pending. FinalizeMember resolves one entry with or without a live attachment. Explicit disjoint pending/live subsets round-trip; finalized or removed entries cannot reopen, and ending freezes later completions — [provider PR](https://github.com/KirkDiggler/rpg-toolkit/pull/2327).
- New tests first failed against the missing API. Pending-as-live and reopen-finalized mutants fail; exact restoration passes full provider `go test -race ./... -count=1`, vet and lint, with 0 lint issues. This is generic-tool proof, not root binding or native pause acceptance — [provider PR](https://github.com/KirkDiggler/rpg-toolkit/pull/2327).

## Interpretation

An unfinished delivery is not an invalid application. The intended list is immutable; each delivery outcome is finalized independently. Current membership belongs to the coordinator, while the frozen cast keeps the execution position needed to continue—not another membership list.

The earlier stop was justified by a conflict in the written contract, but the way I presented the decision obscured the simpler assumption to change. R11 removes the whole-application sealing requirement rather than making every application carry an additional delivery phase. A resolved no-effect outcome must not fabricate a member-loss event, and an ended application must not be revived by a late answer.

## Next

Adopt the pushed provider in the rulebook's fixed-candidate construction and per-recipient finalization, then migrate the native resolver/host persistence path. The amended checked plan names those tests. No further operator decision is needed for that work; review, integration/QA and explicit merge authorization remain separate gates. Journal PR #863 remains open.
