# Discord worlds: corrective handoff for a fresh start

## Read this before resuming #518 / #522

**Operator: KirkDiggler. Work is paused at the operator's request.**
The operator asked to compact the session, retain the lessons, and do the work
properly. Do not resume the old workflow, merge its PRs, or treat its checked
plan as architecture already approved. This is a case file and recovery packet,
not a new repository-wide policy.

The operator's correction in this conversation:

> The protobuf contract is between the client and server. Client input cannot be
> trusted; the API is the boundary. Being explicit from the API into the toolkit
> is a separate concern. Avoiding those changes to keep the patch minimal is not
> doing the architecture properly.

The operator also explicitly rejected callback methods taking
`_ context.Context` and substituting `scope.ctx` as a band-aid.

## What must change in the reasoning

1. **Separate the two contracts.** Client → API is an untrusted transport
   boundary. API → session SDK is an internal, explicit business-operation
   contract. Avoiding redundant/untrusted protobuf fields is not a reason to
   hide mandatory world scope from SDK/repository signatures.
2. **The API verifies authority and derives WorldID.** It checks identity,
   membership/roles and private-resource ownership at the appropriate boundary.
   It passes verified world identity explicitly into the SDK. The toolkit need
   not know Discord, HTTP metadata or how the API authenticated a caller.
3. **Required world scope is explicit data, not an ambient context value.**
   Re-evaluate the session operation/repository contracts around an opaque
   WorldID. This does not require copying WorldID into every client request.
   `context.Context` retains execution concerns such as cancellation/deadlines;
   do not use it to conceal a required ownership/routing input.
4. **Do not compensate for a broken handoff by ignoring an argument.**
   The current #1926 implementation makes callbacks ignore their supplied ctx
   and use a field on writeScope. It is rejected even though its tests pass.
   If an operation needs an execution context, carry that context explicitly
   through the relevant calls. Pure clock/play leaves need not acquire unused
   ctx arguments. Reassess this after settling explicit world scope; do not
   automatically make #1926 a release prerequisite again.
5. **Correct ownership before minimizing edits.** A module boundary may require
   a real provider contract change and its consumer adoption. A smaller patch
   that preserves an implicit dependency is not preferable for this reason.

The fresh pass must determine exact SDK inputs, repository keys/ownership,
absence/error semantics and relevant consumer signatures from code. This packet
states the operator's correction, not a guessed replacement file-by-file plan.

## What went wrong in this session

- I interpreted “do not add WorldID to every gameplay protobuf request” as a
  reason to avoid explicit SDK scope too. Those are different contracts.
- I made host storage depend on trusted WorldID inside Go context and assumed
  inspecting SDK repository signatures established end-to-end propagation.
- Real integration found encounter callbacks using `context.Background()`.
  Instead of revisiting the hidden world dependency, I initially kept the fix
  session-only by storing ctx on writeScope and ignoring callback ctx parameters.
- The provider tests accurately proved that workaround retained the original
  context on the paths tested. Review confirmed the implementation against that
  chosen shape; neither proved the ownership/interface choice was right.
- I let the first slice expand into a large serial integration handoff (115 files
  in its principal API commit), then spent too long on review/status churn.
  The operator reported more than four hours without a demonstrated running
  slice. At this checkpoint no new outcome slice has shipped and the joined
  local two-world browser walk has not happened.

The useful lesson is concrete: before delegating implementation, trace verified
world identity across **both** boundaries, write its explicit provider/consumer
contract, and challenge any mandatory input recoverable only from ambient state.
Do not turn this incident into another generic approval/review ceremony.

## Product decisions still settled

- Goal: one independently owned world per Discord guild; WorldID maps to GuildID
  at the API boundary. One account may participate in more than one world.
- Owner controls admin-role authority; configured admins can manage builder/
  player access; cumulative admin → builder → player permissions.
- World/player-owned characters, drafts, possessions and progression; world-local
  parties/runs, presentation/events and authored content. No implicit transfers.
- **No existing data must be preserved.** A clean reset is acceptable at a
  coordinated, explicitly targeted cutover. This is not an instruction to reset
  the current runtime now. No migration/backfill project is needed.
- Server-created/edited dungeons belong to that server. Editing shipped content
  produces a server-owned version, not a change to everyone else's source.
- **Authored dungeons belong in the database**, with content type, authored
  content and optional comments. YAML versus JSON is not the main goal; forcing
  conversion or compression is unnecessary. Compiled/live run state remains
  distinct from the editable authored definition.
- Sharing dungeons and schema-generated editor forms are future possibilities,
  not current implementation requirements. Billing/quotas remain deferred.
- Only one actual Discord server is available now. Local proof must simulate A/B
  worlds with the same player on **one backend and one database**. Label that
  proof honestly; it is not real two-guild membership/owner verification.

The prior four outcome slices remain a useful *candidate outcome roadmap*:
characters → parties/runs → database-authored content → integration/cutover.
Their technical task contracts are not grandfathered in by this roadmap.

## Preserved Git / PR state

Verified when pausing: all four writer worktrees are clean; all PRs are OPEN;
no active native subagent runs; no code PR merged, no release minted from them,
no database reset or runtime deployment performed. The existing server is
untouched. Keep these trees/commits as evidence; do not delete/reset them merely
to make the new design look as though it was always the original one.

| Repository | Worktree / branch | Preserved candidate head | PR / status |
|---|---|---|---|
| rpg-project | `.worktrees/518-world-plan`, `docs/518-world-plan` | `7f9accd` before this corrective handoff | #519 old plan; paused/superseded pending redesign |
| rpg-api | `.worktrees/discord-world-characters`, `feat/world-owned-characters` | `5929a3f75a842fe5d5c60db308039b7a889c65ea` | #1067 draft; unaccepted candidate |
| rpg-dnd5e-web | `.worktrees/discord-world-characters`, `feat/world-owned-characters` | `8e6227e799843a826757324580f9577355f5f1c1` | #1216 draft; unaccepted candidate |
| rpg-toolkit | `.worktrees/world-character-context`, `fix/session-call-context` | `0c77994d60ba41698923de8d3eb6e72980f6bb1a` | #1926 draft; scope-context workaround explicitly rejected |

Shared record:
- Parent: https://github.com/KirkDiggler/rpg-project/issues/518
- Started character slice: https://github.com/KirkDiggler/rpg-project/issues/522
- Existing admission/configuration slice: https://github.com/KirkDiggler/rpg-project/issues/514
- Old plan: https://github.com/KirkDiggler/rpg-project/pull/519
- API candidate: https://github.com/KirkDiggler/rpg-api/pull/1067
- Web candidate: https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1216
- Provider defect record: https://github.com/KirkDiggler/rpg-toolkit/issues/1925
- Rejected provider approach: https://github.com/KirkDiggler/rpg-toolkit/pull/1926

Already-merged role admission (API #1065 / web #1214 / protos #366) is the
pre-existing baseline, not a new delivery from this slice. Re-fetch actual
branches/tags before measuring again; shared checkout contents were older than
origin during the initial inspection. API/web new work bases on origin/dev;
toolkit/project use origin/main, subject to their current instructions.

## Evidence that may be reused only after checking applicability

- API candidate includes explicit WorldID in API entities/repository inputs and
  private WorldID+PlayerID gates, plus Dev-only allowlisted A/B selection. Its
  **SDK adapter still obtains world scope from ctx**: this is the key rejected
  dependency to reassess, not an accepted contract to preserve.
- At API `5929a3f7`, the new real-gRPC/Redis integration matrix has 19 methods ×
  owner/foreign-world/wrong-player contexts. Focused character tests pass; full
  gate remains red in lobby/session tests against session v0.112.0 because the
  callbacks lose ctx. These failures do NOT authorize merging #1926's rejected
  workaround or skipping those tests.
- Web candidate's full gate passed; its stale-toast/late-callback and same-key
  request-race fixes were independently tested. Code/review findings are closed
  for that candidate, but no joined API/browser proof exists and the operator
  has not accepted the overall work. Reuse only what fits the corrected design.
- Toolkit candidate's callback regressions prove actual context loss in driven
  Announcer/Striker/Mover paths, including cancellation and resumed movement.
  The tests can help a proper repair; green tests do not rescue the rejected fix.
- Published review records remain on the PRs. Some reviewer suggestions were
  rebutted with evidence; do not turn every old comment into a new requirement.

The three callbacks currently arise from encounter/clocks.go Background calls
at the inspected baseline around lines 818, 918, 942 and 1066. Some encounter
verbs already accept ctx (Direct/ResumeTurn/ResumeDirective); others such as
EndTurn/Join/Step do not. Map actual call chains and affected modules before
choosing how execution context should propagate. Do not push ctx into pure
`play/*` leaves or put Discord role checks into the toolkit.

## Local runtime preparation — not a running environment

No `local/world-characters` stack was started. Proposed ports were API 8106 and
web 3021; recheck before using. game-dev#111 remains an unmerged launcher-support
PR at this checkpoint; do not assume the root launcher has its override support.
Existing local fixtures elsewhere are not disposable targets for this restart.

The old Dev simulation contract lives in #522: opt-in allowlisted world IDs,
Dev credentials only, real Discord credentials still membership-verified. Treat
it as candidate fixture support to reconcile with the new explicit SDK scope,
not permission for a production bypass. A=`123456789012345678`,
B=`223456789012345678` were simulation IDs, not claims about real servers.

## Fresh-session starting move

1. Read this packet, the operator's new direction, shared lens and owning package
   instructions. Treat the old detailed plan as evidence of the failure, not law.
2. Explain the corrected ownership/data flow briefly: untrusted client → API
   verifies authority → explicit verified WorldID into SDK → explicit scoped
   repository operations. Keep execution ctx and domain identity distinct.
3. Measure the SDK/session/repository consumers and draft the smallest faithful
   replacement plan at those seams. Surface actual unresolved architectural
   choices, not another blanket implementation-approval gate.
4. Explicitly decide which candidate changes remain useful and which must be
   replaced. Keep corrections visible in issues/PRs. Do not revive old workers
   with their superseded task contracts or repeat broad review rounds by default.
5. Implement one coherent outcome with local shared-backend A/B proof, then the
   applicable review/release/adoption gates. No autonomous merge/reset/deploy.

Stop there for compaction now: no more implementation or agents are queued.

— platform agent, on behalf of KirkDiggler
