# Structural layout snapshot and patch contract

Approved direction: complete permitted snapshots and entity introductions, followed
by typed component replacements. Scope is wall-opening disclosure, not a generic
patch engine, live map editing, or a new door-state channel. See [architecture](README.md)
and [the implementation plan](layout-patch-plan.md).

## Data contract

Snapshot: existing `Atlas.structural_walls` and `Atlas.structural_doors` remain
complete recipient-permitted records. `Knowledge.seq` remains the existing
recipient-local snapshot cutoff.

Both existing `RegionRevealed` and `ConcealmentRevealed` bodies gain this field:

```proto
repeated StructuralWallOpeningsReplacement
    structural_wall_openings_replacements = 12;
```

The shared type lives beside the existing structural types:

```proto
message StructuralWallOpeningsReplacement {
  string wall_id = 1;
  repeated AtlasStructuralOpening openings = 2;
}
```

`wall_id` is the same stable identity as `AtlasStructuralWall.id`. Opening
positions/widths retain canonical feet. The type contains no appearance, transform,
door state, lock, private parent association or authored-source reference.

### Presence and application

| Input | Meaning |
|---|---|
| No replacement record for W | Do not change W.openings |
| Record for W with openings | Replace W.openings completely; do not append |
| Record for W with an empty/default list | Clear W.openings |
| Newly permitted W | Carry its complete record in existing structural_walls; no replacement for the same ID |
| Newly permitted D | Carry its complete record in existing structural_doors |
| Patch references a wall absent from the client's baseline | Reject this event's layout application atomically and recover with Knowledge; do not synthesize a partial wall |

An empty/default openings list inside a present replacement is meaningful even
when protobuf/JSON omits the empty repeated field. No extra field-presence flag is
needed. Empty wall/opening IDs, duplicate wall patches, duplicate opening IDs or
same-ID full wall plus replacement within one event are malformed, not last-writer
precedence. A malformed event must not leave a newly inserted door beside a wall
whose patch failed. Existing geometry validation still applies to the assembled
layout before drawing.

## Producer

Encounter compares the same recipient's before/after projections:

1. Unknown wall → known wall: full introduction with all currently permitted cuts.
2. Known wall with changed openings: one replacement, including the complete new
   permitted opening list, not the unchanged wall dimensions/asset.
3. Unchanged wall: no layout payload for it.
4. Newly permitted independent door: full door introduction in the same event.

The current world layout is fixed during a run. Runtime edits to wall transforms,
assets, dimensions and removal/reconcealment are not introduced by this contract.
Existing full-record upserts remain decodable for historical events and compatible
producers; no existing field is removed or reinterpreted as a partial record.

Payloads are captured at event creation for the original recipient. Replay never
recomputes them against later knowledge or current world truth. The producer does
not emit a replacement naming a withheld wall, including when its door is known
independently. Ordering is deterministic by wall/door identity; opening order
matches the canonical snapshot for that wall.

## Sequencing and recovery

- One reveal event is one atomic update of structural introductions/replacements.
- Use the existing recipient stream sequence and snapshot cutoff. Do not introduce
  per-wall clocks, a second stream or an appearance RPC.
- Events included in the snapshot, or already applied afterward, are no-ops.
- Preserve events arriving during snapshot hydration; apply only those after the
  returned cutoff, in the existing delivered order.
- Existing stream gap/catch-up machinery retains ownership of transport ordering.
  Unknown patch baseline requests a coalesced Knowledge resnapshot. Failure is
  visible; do not loop endlessly or apply a partial event.
- Structural component patches do not replace existing mutable observation reads
  or unrelated legacy refresh behavior.

## Compatibility and rollout

This is additive wire evolution. Old snapshots and old full-record reveal payloads
remain valid. A client unaware of the new field cannot apply a patch-only reveal;
therefore deploy the updated structural producer/API/web as a coordinated feature
slice. Do not dual-write a full wall and replacement for the same identity to hide
that dependency. No capability-negotiation mechanism is added.

Protos generate through CI after operator-authorized merge. Go implementation can
progress independently on pushed provider checkpoints; consumers adopt actual
released toolkit tags before merging. No generated files, local replaces or
source-copy overrides are introduced.

## Acceptance cases

- Snapshot S at N + introduction event N+1 equals a fresh permitted snapshot.
- Known W with no cuts + reveal of O/D produces a replacement for W and full D,
  not another full W; applying it equals the next snapshot.
- Two sequential cut reveals replace the list without losing the earlier cut.
- An explicit empty replacement clears the list; absence leaves it unchanged.
- Duplicate/older delivery cannot duplicate entities or revert a later cut list.
- A replacement for unknown W changes neither walls nor doors before recovery.
- Hydration/reconnect with in-flight events converges to snapshot + later events.
- An unpermitted wall is not named through an independently permitted door.
- Replay of the first reveal is unchanged after a later secret is discovered.
- Concealing only a door/wall never expands the explicitly concealed floor set.

These cases are the proof boundary for this patch change. Arbitrary-placement
observation and a general continuous sight extension are not prerequisites.
