# First look: author one room in World Builder

**Approved scope, Kirk 2026-09-14:** "making a room with walkable spaces is
step 1. placing props and making them blocking or block los. next ... getting
some walls in ... could operate off a single [region]."

This is an authoring-interaction checkpoint before the encounter/YAML integration,
not a claim that a new playable dungeon format is already implemented.

## What Kirk should be able to do

In a dedicated web-branch environment, using the existing World Builder's
placement/camera/selection experience:

1. Create a room draft with one implicit region; no region-management panel.
2. Paint and erase walkable hexes independently of the assets. A prop or floor
   mesh does not silently author walkable space.
3. Drag/place props, stack decorations, group, move, rotate, duplicate, and use
   existing arrangements without losing the behavior that already feels good.
4. Configure the selected prop's explicit movement/sight declarations and edit
   its rectangular footprint visually. Moving/rotating its owner carries the
   local footprint; there is no asset scale or physics simulation.
5. Undo/redo, reload, and export/import the local room draft without losing
   painted cells, prop properties/footprints, visual pieces, groups/supports, or
   lights. Do not overwrite the existing prop-composer draft or arrangement
   library when creating/opening a room draft.

## What this checkpoint does not do

- No walls/doors yet: they are the next authoring interaction after this look.
- No secret-room visibility, region-management UX, or in-game pickup/drop.
- No half-cover rules or ignored Half cover checkbox.
- No new RPC, fake server validation, client-side LOS/pathfinding/coverage rules,
  or claim that declared blockers already affect a playable session.
- No canonical v3 YAML freeze, Save & Play promise, backend pin changes, or
  copying a stripped-down visual-ref-only format into the future contract.

The UI clearly labels the result an **authoring draft**. It may display painted
floor and authored footprint outlines. It does not pretend those are an engine-
validated blocked-cell map. Movement and sight verification comes with the next
integration checkpoint.

## Implementation boundaries

Evolve/reuse the existing World Builder implementation and shared rendering,
not a copied second editor. A dedicated development/concept mount or room mode
is fine; the original composer remains usable.

Keep the complete existing WorldScene as visual data. Add a versioned room-draft
wrapper/metadata shape rather than destroying composition snapshots. This local
format must be distinguishable from playable dungeon YAML. Use separate local
storage ownership/keys for room drafts. Types and unit/frame declarations are
explicit; use existing shared hex math and coordinate bridges for authoring and
rendering, not duplicated geometry or D&D calculations.

Stable ID associations matter: deletion, duplication, grouping and arrangement
stamping must maintain/remap gameplay metadata deliberately, not match objects
by label/ref or silently reset configured properties. History includes both scene
and room metadata so one gesture is one undo transaction. Footprint editing
must not alter the visible mesh scale or absorb camera gestures.

Only rpg-dnd5e-web source is writable for this slice. API dev is an isolated
baseline in the named environment, not the shared :3001 stack. No toolkit,
protos, provider, global configuration, or existing live-stack changes.

## First human checkpoint and evidence

Bring Kirk in as soon as the new controls work in an actual browser. Before
that invitation, prove a small real scene: paint/erase cells, place a table and
its decorations, configure the rectangle/flags, move/rotate the owner, undo one
gesture, and reload the local room draft. Use the actual R3F interaction path,
not direct calls to component setters or screenshots of mocks. Keep licensed
models and captures untracked.

Run focused state/serialization/interaction tests and type checking needed for
that checkpoint. Do not spend a full CI/review cycle polishing an interaction
Kirk has not tried. Normal hooks remain enabled. The first look can remain a
local branch checkpoint; before the first push/PR run the required single
`npm run ci-check` and open a draft immediately. GLM 5.3 Flash review follows
the human interaction check, not before it. No automatic merge or cleanup.

The backend consumer #1753 is parked until this real consumer contract is
available. Geometry v0.14.0 and shared sight v0.15.0 remain useful released work;
this changes the next proof, not their implementation.
