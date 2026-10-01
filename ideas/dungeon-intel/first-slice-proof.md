# First-slice proof: an unknown room, then remembered contents

Walk for the approved first-slice goal of [individual dungeon knowledge](design.md),
tracked in [rpg-project#508](https://github.com/KirkDiggler/rpg-project/issues/508).
This is an acceptance sketch, not a working feature or a settled implementation
plan. The design's named mechanism questions remain open.

## Fixture

Use ordinary doors and three bounded areas: room 1, unseen room 2 behind its
closed door, and room 3 behind a further door at the end of room 2. Prefer the
Reference Tomb if it supports the actual sight cases; a dedicated listed dungeon
is also permitted. Recheck the fixture against this unknown-layout goal rather
than treating the earlier pre-explored geometry experiment as its proof.

Two separately authenticated player characters start knowing room 1 and its
closed door. Neither has explored room 2 or room 3. Room 2 contains fixed
non-holdable scenery and one holdable prop. An opaque corner lets A withdraw
out of sight while B later gains a view and can act. Verify positions with real
sight queries and ensure pickup does not end the run.

The fixed layout and fixed scenery have one authoritative shared geometry map
on the server, with individual discovery determining what reaches each player.
There is no full-map download followed by client-side hiding, and no mutable
intel record for every fixed prop. Creatures, holdable props and door state use
current/remembered intel; discovering a door does not discover its interior.

Player principals have no separate builder privilege. Full authored-document
access is not a way to provide their visuals. Exact geometry discovery and
permitted-content delivery mechanisms require shaping before implementation.

## Walk and assertions

| Step | A's permitted knowledge | B's permitted knowledge | Assert at the boundary |
|---|---|---|---|
| Start in room 1, door closed | Room 1 and observed closed door; no room 2 or 3 interior | Own room 1 observation only | Room 2/3 layout and positioned content absent from network bodies and client scene/cache |
| A opens the door and looks into room 2; B remains obstructed | Observed room 2 geometry/fixed scenery, holdable prop and door state | No room 2 knowledge without B's own view | Opening alone does not send both players the room; delivery follows each observer |
| A sees the further door at the end of room 2 | Door at its observed position/state; no room 3 interior | Own knowledge only | A may suspect another room, but receives none of its layout or contents |
| A withdraws around the corner | Discovered fixed geometry retained; prop and observed door state become remembered | Own knowledge only | Fixed scenery is not forgotten; mutable subjects fade without revealing live truth |
| B obtains a view into room 2 | A's remembered observations unchanged | B learns its own observed geometry, prop and door state | No automatic sharing or refresh of A's memory |
| B picks up the prop and closes the first door out of A's sight | Prop remembered at its old position; door remembered open | Observed removal, own holding and closed door | No unseen state/carrier disclosure to A through reads, events or metadata |
| Save and reconnect both clients | A's discovered geometry and stale mutable observations | B's own latest knowledge | Fresh hydration preserves different views; room 3 remains absent |
| A approaches and sees the closed door, but not the prop's old position | Door updated to closed; prop still remembered | B's own view only | Door observation does not refresh contents behind it |
| The door opens and A observes the old prop position empty | Disproved location replaced, no inferred destination/carrier | B's own view only | Absence follows observation, not arrival or omission from a roster |
| Save and reconnect again | Corrected knowledge stays corrected; known geometry retained | B's knowledge stays independent | Reload neither resurrects stale placement nor supplies undiscovered room 3 |

The prop remains known to exist; no historical collection of former positions
is required. Control whether B's carried item is observable to A so the absence
check does not accidentally obtain another source of knowledge. Creature intel
is the existing behavioral model, not permission to change creature absence
semantics implicitly.

## Negative controls

- Before first observation, repeatedly refresh and reconnect with an empty client
  cache: room 2/3 layout must remain absent, not merely hidden on screen.
- Open a door remotely or keep B obstructed while A opens it: B receives no
  interior solely because the world door changed.
- Observe the far door without seeing through it: no room 3 layout is delivered.
- Keep the old prop position out of A's sight after pickup: refresh and reconnect
  leave A's remembered position alone.
- Observe the empty position without walking onto it: replacement follows sight.
- Repeat unchanged observations: no historical entries or redundant fixed-prop
  snapshots are added.
- Use B's member ID in an A-authenticated read or subscription: no inherited view.
- Request the source document, unknown geometry or an undisclosed placement
  directly as a player: no bypass to undiscovered content or unseen current state.
- Inspect initial reads, refreshes, events/replay, reconnect bodies and the
  client's loaded scene/content, not only screenshots.

## Evidence required

Record toolkit observation/discovery tests, API identity/projection tests,
event/reconnect coverage and the real browser walk separately. Primitive and
geometry overlays do not satisfy the vertical proof; a screenshot does not
prove non-disclosure. Publish tested revisions and distinguish baseline behavior
from new acceptance tests.

If a slice cannot meet an assertion, identify the specific limitation and ask for
its ruling before weakening the assertion. Keep that exception and the later
proof that removes it on the tracking issue.

## Not proved by this walk

- Secret-door discovery checks and concealment-group changes.
- Arbitrary multi-cell props, every authored dialect, illusion or special sense.
- New-encounter/campaign persistence or changes to the authored content revision.
- Whole-product authorization closure beyond the exercised player paths,
  including catalogs, previews and shared content caches.
- A representative production performance budget.

Those are explicit next proofs, not reasons to add a second knowledge engine or
relax the final no-undiscovered-content boundary.
