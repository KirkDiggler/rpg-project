# Does pending delivery survive all the way to the private sheet?

Written: 2026-11-11. [S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706); [implementation receipts](https://github.com/KirkDiggler/rpg-toolkit/pull/2323).

## Question

The [per-recipient ruling](2026-11-10-what-actually-needs-sealing.md) made pending delivery valid. Does that remain true through the actual resolver, host persistence and player-facing readers, rather than only in the lifetime value?

## Evidence

- Native resolution casts Bane, saves an earlier delivered recipient while another target answers, reloads and resumes the same application, then removes both effects on actual subsequent damage. Self-Bless supplies its d4 before failed-concentration cleanup, with one check for two recipients and damage applied once. Full resolution gates pass at `6618456d`. [#2344](https://github.com/KirkDiggler/rpg-toolkit/pull/2344).
- Session's Manager.Cast/React regression uses JSON-round-tripped stores and a new Manager. Membership, identity, payment and exact verdict counts survive. Removing the caller identity or skipping dirty character persistence makes it fail. Full gates pass after the catch-up preserving site/checkpoint/offer/hail work at `ccb082c9`. [#2377](https://github.com/KirkDiggler/rpg-toolkit/pull/2377).
- The native join found that monster pure load held only blobs until Apply, so the binding could not see a saved monster recipient. Root now prepares those behaviors before binding and retains rollback/retry without duplicate state. It also caught absent config becoming literal null across a frozen declaration; explicit data tags fixed that without normalization. [#2329](https://github.com/KirkDiggler/rpg-toolkit/pull/2329).
- API acceptance creates/finalizes a real Life cleric, uses its domain-granted Bless, pays and casts, reloads through a fresh host, reads both private character APIs and persists end cleanup on every holder. The older converter dropped object refs; the native v2 view separately exposed a generic registry label. Both were observed failures, and description remains toolkit-owned. Full API gates pass at `a8463915`. [#1225](https://github.com/KirkDiggler/rpg-api/pull/1225).
- Current web readers reject obsolete linked shapes and retain opaque bindings. Mounted native rail/action HUD tests render source-separated server facts. Full ci-check passes at `970625f9` (794 files, 10274 tests; existing skips unchanged). [#1410](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1410).
- The isolated stack is up at web :3042 / API :8142. Detached source heads, binary module pins and both content hashes were verified. The landing and real dungeon catalog render without page errors. This is boot/early-look readiness, not completed browser gameplay QA.

## Interpretation

The consumer loop found assumptions that source-local gates could not: loaded blobs are not yet bindable behaviors; a JSON round trip can change absence; one corrected API surface does not establish that the native private reader uses it. The remedy stayed with each fact's owner rather than adding a client calculator or a compatibility path.

Parallel workspace changes continued while this wave was built. Catch-ups preserve their current contracts rather than using a lower pseudo-version to erase them. Source/module gates and API acceptance are now evidence; the operator's early look, joined independent review and bounded native walk are still distinct outstanding claims.

## Next

The early-look fixture is **Composable Maintenance Walk**, with fresh identities `maintenance-cleric` and `maintenance-bard` on :3042. After the operator looks, complete the joined review and bounded two-client/castle QA. No merge has been authorized or performed. Journal PR #863 remains open.
