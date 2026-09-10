# Roll-window presentation ID — delivery receipt

## Outcome

Kirk accepted the scoped local playthrough and explicitly authorized promoting
the whole tested stack. The implementation reuses the existing roll-window
facts and presentation lifecycle. No new roll model/event/source or privacy
policy was introduced. The abandoned Resolution snapshot PR #1597 was closed
unmerged.

## Published and merged

| Layer | PR | Verified merge / release |
| --- | --- | --- |
| Encounter ID storage | rpg-toolkit#1599 | `0aaa1807487ce685a3503dc91f22f389747036b1`; encounter `v0.68.1` |
| Session ID forwarding | rpg-toolkit#1604 | `255046f58429c946fb0ef4091f5d7112e3d763ff`; session `v0.69.2` |
| Existing proto field | rpg-api-protos#319 | `a9b3ae99c1d57828c27ada6025003e8fae74b50d`; generated `7edb9301cd195da54c6cdffae900a7f14be76d95`, tag `v0.1.180` |
| API mapping / released pins | rpg-api#954 | `aae6ce141105104fc8298449992c962b69d36c7c` on dev |
| Collapsible log / inline JSON | rpg-dnd5e-web#1016 | `ad81d4de75042e4d3d5002cb37999f58e3d44a37` on dev |
| Target panel placement | rpg-dnd5e-web#1019 | `cf12aaffece81b915a855d91f5de21fd61b5bb91` on dev |
| Full dice/inspiration web candidate | rpg-dnd5e-web#1003 | `a01de65f33c573ba0f47f2a3c21e5e6996ba246c` on dev |

API and web now both pin the actual published contract. No local path, tarball,
workspace replacement or generated SDK edit was promoted.

## Wire-number reconciliation

The local prototype used `RollWindowOpened.presentation_id = 5` while that tag
was free. Concurrent proto #318 assigned tag 5 to calculation. Publication
preserved that field and assigned presentation ID to **6**, then regenerated
both published consumers together. The behavior is unchanged; the older local
SDK pair was not mixed with the published pair.

The publication work used isolated API/web checkouts, leaving the running
`:3006` / `:8083` local stack and its matching old SDK pair intact. Normal
`dev` stack refresh uses the published field-6 pair. Do not manually resync only
one side of the preserved old lab.

## Evidence

- Session tests, race tests and lint passed; actor and peer receive the same
  early ID/face, restart/resume preserves it, and old windows remain readable.
- Proto lint/format/breaking/generation and published-SDK CI passed.
- API published-pin guard, full tests, build and lint passed with `GOWORK=off`.
  Real-handler Bard acceptance checks both recipients and actor-only executable
  offers. Two existing single-target Cast fields gained localized compatibility
  annotations because the new SDK deprecates them; behavior is unchanged.
- Final web candidate `eeda8df44231a89636f5cbf9cd6eb7bc139122c4` passed the full
  local gate and hosted CI `34429129080`. Fresh independent Terra review found
  **0 Critical / 0 Important / 0 Minor**, published on PR #1003 before merge:
  https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1003#issuecomment-5611739483
- Earlier UI reviews covered log/collapse/wide JSON. Browser fixtures verified
  placement, width caps, focus, scrolling and safe JSON. #1019's CI exposed
  inherited asynchronous test assertions; scoped readiness waits were added,
  with no production behavior or timeout change. Its fresh hosted gate passed.
- Kirk reported shared dice visible with and without inspiration, confirmed the
  death-save roll on retry, approved log/targeting layout, and stated the scoped
  goal was met. These are the manual observations recorded, not an invented
  exhaustive test matrix.

No new live field-6 walk was performed by the final reviewer. Publication is
supported by the accepted local behavior plus provider/consumer checks against
the matching released SDKs; a normal dev-stack refresh is the next local check.
No production deployment, global configuration change or destructive cleanup
was performed.

## Remaining outside this delivery

- rpg-toolkit#1602: downed monster makes an opportunity attack.
- rpg-dnd5e-web#1013: True Strike recast/concentration wording.
- Persistent pre-attack True Strike target marker: not implemented or claimed.
- rpg-project#408: both d20 faces / kept index; no damage-dice or optimization work.

Local evidence and frozen-source inventory remain in
`game-dev/.runtime/local/410-roll-window/README.md` and
`/tmp/roll-window-promotion/`. Worktrees and the local test stack were preserved.
