# First-slice proof: a room remembered differently

Proposed walk for [individual dungeon knowledge](design.md), tracked in
[rpg-project#508](https://github.com/KirkDiggler/rpg-project/issues/508).
This is an acceptance sketch for ruling, not a working feature or implementation
plan. Design R7–R10 and the named mechanism questions remain open.

## Fixture

One room with an opaque partition and an ordinary operable door, two separately
authenticated characters, and one prop that the existing pickup verb can remove.
Choose positions against the actual sight queries, not a diagram's assumption
about what the partition blocks.

The fixed layout is explicitly pre-explored for both characters in this fixture.
Both initially observe the prop on the floor and the open door. The fixture does
not make unknown layout visible by default in normal play. A can withdraw to a
position that prevents observation of both the prop and door, while B remains
able to observe and act on them.

Full authored-document access is not a way to provide the fixture's player
visuals. The design must settle a permitted-content path before implementing the
vertical proof. A player principal used for the walk has no separate builder
privilege.

## Walk and assertions

| Step | A's permitted knowledge | B's permitted knowledge | Assert at the boundary |
|---|---|---|---|
| Both look | Prop at its observed position; door open | Same observed facts | Correct identity-bound initial reads; no world document download |
| A withdraws | Same facts, remembered | Current observations | Loss of sight does not erase or refresh A's memory |
| B picks up the prop | Prop still remembered on the floor | Observed removal; own holding | A gets no disclosure of removal or carrier through reads, events or action metadata |
| B closes the door | Door remembered open | Door observed closed | A receives no unseen door-state update |
| Save and reconnect both clients | Same stale memory | Same permitted current/latest knowledge | Fresh hydration agrees with each identity's pre-disconnect knowledge |
| A approaches and can see the closed door, but not the prop's old position | Door now closed; prop still remembered | B's own view only | Updating the door does not also refresh the room's contents |
| The door opens and A can observe the prop's old position empty | Old prop-location belief replaced; no inferred destination or carrier | B's own view only | Observation of absence is distinct from losing sight |
| Save and reconnect again | Corrected knowledge stays corrected | B's knowledge stays independent | Reload neither resurrects stale placement nor copies B's knowledge to A |

The prop remains known to exist; the walk does not require erasing its identity
or keeping a history of its former position. Whether B's carried item is itself
observable to A is controlled explicitly by the fixture so the absence test does
not accidentally gain a second source of knowledge.

## Negative controls

- Keep the old prop position out of A's sight after pickup: repeated refreshes
  and reconnects must leave A's memory alone.
- Let A observe the empty position without walking onto it: correction follows
  observation, not an arrival-only special case.
- Repeat an unchanged observation: it does not create another historical entry.
- Use B's member ID in an A-authenticated read or subscription: it must not grant
  B's view.
- Request the source document or an undisclosed placement directly as a player:
  there is no bypass to undiscovered content or unseen current state.
- Reconnect A with an empty client cache and inspect network bodies, not only the
  rendered screen. The server restores A's remembered values, not current truth
  that the browser must hide.

## Evidence required

Record toolkit tests, API identity/projection tests, event/reconnect coverage and
the real browser walk separately. An overlay primitive probe does not satisfy the
vertical proof; a screenshot does not prove non-disclosure. Publish exact tested
revisions and distinguish baseline behavior from new acceptance tests.

If a slice cannot meet an assertion, identify the specific limitation and ask for
its ruling before weakening the assertion. Keep that exception and the later
proof that removes it on the tracking issue.

## Not proved by this walk

- First exploration of unknown floor and walls.
- Finding a concealed door without discovering its room, or observing only a
  portion of a newly opened room.
- Arbitrary multi-cell props, every authored dialect, illusion or special sense.
- New-encounter/campaign persistence or changes to the authored content revision.
- Whole-product authorization closure beyond the explicitly exercised player
  paths, including catalogs, previews and shared content caches.
- A representative production performance budget.

Those are explicit next proofs, not reasons to add a second knowledge engine or
relax the final no-undiscovered-content boundary.
