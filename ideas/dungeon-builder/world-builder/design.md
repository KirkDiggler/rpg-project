# World Builder dungeon authoring

**Status:** draft for Kirk's review, 2026-09-14. Direction agreed in conversation;
contract recommendations below are proposals, not new project policy or laws.
**Journey:** [Composable Dungeon Builder #169](https://github.com/KirkDiggler/rpg-project/issues/169).
**Related:** [terrain #428](https://github.com/KirkDiggler/rpg-project/issues/428)
and its [design PR #429](https://github.com/KirkDiggler/rpg-project/pull/429).

## 1. The experience

Paint where people can walk. Place and arrange assets with the World Builder's
existing freedom. Select a prop, shape its gameplay footprint visually, and
configure what it communicates to the engine. Save, reopen, and play the same
dungeon without losing authored meaning.

This evolves the World Builder and the dungeon document together. It is neither
a second editor nor an exporter that squeezes a free scene into today's
one-placement-per-cell format.

### What Kirk agreed to

- Playable space is painted independently of visual assets.
- Props carry explicit gameplay properties: movement blocking, line-of-sight
  blocking, half cover, and other supported properties as they acquire a use.
- Unchecking movement blocking allows that prop to be passable; its appearance
  does not decide the answer.
- Moving or rotating a prop moves its spatial effects with it. The author does
  not repaint obstruction cells afterward.
- Start with a visually editable rectangular footprint, independent of mesh
  bounds, rather than painting a prop's occupied hexes by hand.
- Keep the current placement, stacking, grouping, arrangements, lights, and
  undo/redo experience. Start with a small playable proof.
- Evolve the current YAML where necessary. It is not a compatibility ceiling.

The examples about illusions were illustrations, not requests for illusion
rules. Automatic height-derived cover, crouching, a take-cover action, physics,
and scale controls were assistant extrapolations, not agreed requirements.

## 2. What exists, and what does not

Source inspection used these snapshots; no new gameplay or test result is
claimed by this design:

| Repository | Revision |
|---|---|
| rpg-dnd5e-web, origin/dev | `75fb53569991c1cd02d87a5d09c077b84701e5a2` |
| rpg-toolkit, origin/main | `3968a04cae8f793a875c3748dd414d748f900e06` |
| rpg-api, origin/dev | `d8504cc4393e322548731e4d43c5e44abc98c85c` |

**World Builder:** continuous X/Z placement, upright arbitrary yaw, raised
objects, support/group relationships, arrangement stamping, lights, and
transactional gizmos already exist. The saved scene is visual Composition
JSON, not dungeon YAML. It has a finite small-workspace extent and no authored
scale. See web `src/concepts/world-building/CONTRACT.md`, `types.ts`,
`sceneState.ts`, and `serialization.ts`.

**Dungeon authoring:** `src/author/DungeonBuilder.tsx` already drives canonical
YAML, debounced server validation, an atlas preview, save/reopen, and Save &
Play. `dungeonYaml.ts` supplies document operations worth reusing. Its cell
placement replaces an occupant; free yaw, overlapping instances, and exact
scene-root transforms do not survive that contract.

**Spatial:** v0.12.0 supplies Field; v0.13.0 supplies the hex embedding and
rotated rectangular Coverage. Coverage returns cell-area fractions and is
currently anchored at a cell centre or edge. It does not yet accept arbitrary
planar translation or return thin-obstacle crossing facts. See toolkit
`tools/spatial/{field,embedding,coverage}.go`.

**Live props:** encounter compiles a prop into one anchor-cell occupancy entry.
Facing and offsets are visual-only. Its shared `CellAt` fold improves pathing,
but no prop footprint feeds that fold or the room's sight queries. The shipped
spell coverage caller is not multi-hex prop support. See toolkit
`rulebooks/dnd5e/encounter/{compilefield,cellfacts,shape,atlas}.go`.

**Half cover:** the survey did not establish an end-to-end cover provider and
consumer. A follow-up targeted search found the existing AC breakdown carrier,
not an authored-cover implementation. A checkbox alone cannot deliver cover.
Treat geometric qualification and D&D rule application as work to prove, not
as an already-shipped capability.

## 3. Authoring model

### Painted space

Painted cells say where the ground permits standing and traversal, independently
of meshes. Unpainted space is not made traversable by adding a floor asset.
Painting does not imply a solid surrounding wall; retain explicit void/sight
behavior and the existing authored boundary/door concepts.

Regions remain useful for names, lighting, and other existing area facts. The
first floor brush may paint into a selected region, with one region sufficient
for the first proof. There is no need to replace the region primitive to
separate mechanics from scenery.

Painted floor is not the final answer to movement: props, boundaries, and other
encounter occupants can obstruct it. Erasing floor does not silently delete its
props, start marker, or monsters; the draft remains editable and validation
identifies the now-invalid gameplay placements.

The dungeon workspace uses its authored extent instead of the composition
editor's small fixed circle and +/-12 coordinate limit. Retain explicit input
and complexity limits; do not remove validation to make the canvas larger.

### Props and visual assemblies

A gameplay prop has an identity, a visual reference or assembly, a local
footprint, explicit properties, and a placed transform. An assembly such as a
table with candles can have one table-sized gameplay footprint. Its internal
visual leaves retain their transforms, support links, grouping, and lights.

Ordinary editor grouping is not an implicit gameplay merge. Grouping two
independent blocking props moves both; it does not erase either contributor.
Saving a selection as a reusable gameplay prop is an explicit authoring choice.

Recommended property representation: movement and sight booleans plus a typed
cover value, initially `none` or `half`. The UI can present Half cover as a
checkbox. This is not an untyped property bag: unsupported gameplay properties
are rejected rather than silently stored and ignored.

Reusable definitions supply authored defaults. An individual placement can
explicitly override those properties; `false` means false, not "inherit".
Editing one placement never mutates every use of the definition. Effective
properties are fully determined before gameplay compilation.

### Visual footprint editing

Select **Footprint** to show a rectangle in the prop's local horizontal plane.
Resize its sides and move its centre relative to the prop origin. The footprint
shares the prop's yaw; moving or rotating the prop carries it without modifying
its local dimensions. Footprint edits are undoable transactions like transforms.

The measured mesh box may be offered as a starting suggestion, but accepting or
editing a gameplay footprint is an author action. A missing footprint on a prop
with spatial effects is an authoring problem, not an implicit one-cell fallback.
Pure visual dressing can have no gameplay footprint.

Start with boxes, not with a schema that claims all future shapes are boxes.
The current editor still has no scale or tilt tool. Visual Y is preserved but
does not imply a new elevation, eye-height, or three-dimensional physics model.

An optional gameplay overlay distinguishes painted floor, authored footprints,
and engine-derived obstruction. A drag may preview the rectangle locally; an
engine-derived preview is explicitly pending/stale until the matching revision
returns. Mesh bounds are never labelled as authoritative occupancy.

## 4. Document and export contract — proposed for review

The new dungeon format needs to retain three kinds of authored information:

1. **Space and setup:** painted region cells, void/orientation, boundaries,
   doors, start, monsters, and supported existing dungeon content.
2. **Reusable prop definitions:** visual assembly, local rectangle, and
   gameplay-property defaults.
3. **Instances:** stable ID, definition reference, continuous transform, and
   explicit local property/footprint overrides where authored.

**Recommendation:** keep dungeon-used authored definitions in the dungeon
document. The world library seeds a definition rather than remaining its only
copy. Reopening an export therefore does not depend on an ephemeral Redis
composition ID or the browser's local draft. External licensed model assets
remain references; they are not embedded in YAML or committed to public Git.
This portability choice is a recommendation in this draft, not something Kirk
already specified.

Importing an existing composition copies its authored snapshot, preserves its
provenance reference, and allocates dungeon-local identity. There is no live
linked-prefab update or global propagation in this first design. Deduplicate
shared definitions by explicit identity, not by matching human labels.

Store continuous planar position, visual height, and yaw without clamping to
within-cell offsets or eight compass names. Use the spatial plane and explicit
unit/orientation metadata for canonical geometry; convert at the renderer
boundary rather than using Three.js transforms as the engine's coordinate
system. Round-trip numeric values without a new float32 narrowing step. A
representative anchor cell may be derived for indexing, but is not a second
authored position and cannot substitute for the footprint.

The exact YAML grammar and transport structs are a subsequent contract design,
not frozen by this document's logical field names. Their acceptance is fixed:
all supported authoring state survives export/import, unsupported fields fail
visibly, false is distinct from absent, and a document is not rewritten into
an older dialect with loss. Deterministic canonical output is sufficient;
preserving arbitrary comments and formatting is not part of the first proof.

Use a new explicit format version. Keep the existing version-2 reader and old
builder usable during the new path's development; the old builder refuses the
new version rather than resaving it destructively. Legacy content migration is
an explicit conversion with comparison evidence, not a boot-time rewrite. In
particular, legacy cosmetic offsets must not silently become displaced
collision geometry. Do not modify existing saved games or delete content as a
side effect of introducing the new authoring path.

Editing and saving updates the authoring preview and future launches, not an
already-running game. Moving a prop in the editor is not a live-DM mutation
protocol. Existing runtime hold/drop/arrival behavior remains separately
responsible for updating the props it can move.

## 5. Engine ownership and data flow

```text
World Builder document + user edits
  -> YAML / authoring request
  -> strict decode and validation
  -> resolved definitions + placed footprints
  -> spatial geometry
  -> encounter cell/boundary/sight facts
  -> shared atlas preview and playable encounter
  -> rulebook resolution when a rule consumes a positional fact
```

- **Web** owns authoring gestures, visual footprint editing, document editing,
  and rendering. It displays engine answers rather than calculating combat
  cover or maintaining a competing movement-blocking map.
- **Toolkit authoring/encounter** owns the canonical field, validated
  placements, effective properties, and their contribution to the live map.
  `CellAt`, route, step, placement, and sight must agree about the same props.
- **Spatial** owns continuous placement geometry, coverage, crossings, and
  sight queries. Improve those existing tools instead of creating geometry in
  the API or a parallel builder-only collision system. Keep D&D bonuses out.
- **Resolution/rulebook** owns the mechanical meaning of a qualifying cover
  contribution. Session exposes the capability without reconstructing geometry.
- **Protos/API** carry the types, authenticate/resolve content, compile, and
  persist. They do not derive occupancy or implement cover bonuses.
- **Assets** continues to own model bytes, references, and visual calibration.
  This slice does not require re-exporting meshes to author gameplay meaning.

Store authored definitions and transforms as the source, not hand-painted
copies of every prop's blocked cells. Any derived index has a defined rebuild
path from the same inputs. Placement edits, removal, reload, and any existing
hold/drop/arrival path that accepts these props must remove stale contributions
before exposing the updated state.

A thin blocking rectangle must not become mechanically empty merely because it
covers less than half of each hex. Geometry needs the relevant crossing facts,
not just area coverage. Existing walls and doors remain contributors through
the shared boundary mechanisms, not a second wall editor reimplementation.
Cell-area thresholds and obstruction policy belong to the consuming encounter
contract; a spell's half-area threshold is not automatically a prop rule.

Footprints can visually overhang the painted area. Only painted playable cells
can become available to creatures; prop overhang never creates floor. Sight
geometry must retain relevant authored obstructions rather than silently losing
a footprint portion because a floor-only grid clipped the query.

## 6. Cover is a real second proof, not a decorative field

Half cover is an explicit authored capability, not a deduction from the mesh's
height. The engine determines whether that prop is relevant to a particular
source/target relationship. Standing beside an irrelevant cover prop must not
add a permanent bonus to the character sheet.

The recommended rulebook behavior is standard half cover: +2 AC and +2
Dexterity saves when cover applies, not disadvantage. There is no new crouch
verb, exposure simulation, or illusion rule in this proposal. Multiple half
cover props do not add repeated +2 bonuses.

**This needs a focused companion contract before implementation.** It must
specify the geometric source/target query (including the existing sightline
conventions), the cases where cover applies or does not, and how relevant
attack/save paths consume the result. A centre-ray shortcut must not quietly
be presented as complete D&D cover. An existing generic AC component is not
proof of any of these consumers.

This decomposition keeps the first geometry/authoring proof small without
losing Kirk's cover requirement. Half cover is part of the requested outcome;
if it is not implemented yet, the editor does not offer it as a working
checkbox or accept it into a supposedly playable document. The overall
outcome is not declared delivered until the cover proof also passes.

## 7. Small proofs and failure behavior

### Proof A: paint, place, move, save, play

Use a small pointy-top region, a party start, a monster, a solid statue, and a
rotated multi-hex table with candles.

1. Paint floor without placing a floor asset. Place visual dressing in
   unpainted space and show that it creates no playable cells.
2. Place and stack using the existing World Builder gestures. Shape the
   statue/table rectangles and configure movement and sight independently.
3. Move and rotate a prop across hex boundaries. The visual rectangle follows;
   the engine reports the new obstruction and clears the old one.
4. Uncheck movement blocking on one instance. Floor becomes traversable unless
   another contributor blocks it; sight remains governed by its own property.
5. Save/export, reopen, edit, and Save & Play. Definitions, transforms,
   relationships, lights, properties, and footprints survive. Player and
   monster movement agree with the engine overlay and sight responds correctly.
6. Repeat with overlapping independent props and a thin blocking rectangle.
   Removing one overlapping contributor cannot erase the other's effect.

### Proof B: authored half cover

Add an explicitly configured half-cover barricade. Test relevant and irrelevant
attack directions, unobstructed positions, multiple contributing props, moving
or removing the barricade, reload, and applicable Dexterity saves. Report the
source of the mechanical modifier through the existing breakdown/story path.
An opaque blocker still blocks sight; Half cover is not an override for it.

### Validation and errors

- Drafts can be incomplete. A failed preview/import/save leaves the last good
  document and persisted file intact; incomplete drafts are not called playable.
- Errors identify the instance, footprint, cell, or field the author can edit.
  Missing definitions/assets are visible dependencies, never substituted models
  or an arbitrary default dungeon.
- Reject non-finite transforms, nonpositive rectangles, invalid references,
  duplicate identities, and unsupported gameplay properties. Permit overlapping
  visual instances instead of the current one-placement-per-cell replacement.
- Check party start and all proposed party seats against the final standability
  and occupancy rules, not just floor membership or a seat count. A scout found
  `dungeonspec/compile.go:750-801` filters occupied seats but apparently not
  sealed cells; this remains source-inspected and unreproduced, not a fixed bug.
- Content resolution and launch validation are distinct from YAML compilation.
  Resolve required monsters/definitions before advertising play readiness; no
  success message for an encounter that could not be fully created.

### Evidence required

Use asymmetric coordinate cases, non-cardinal rotation, fractional translation,
negative coordinates, local footprint offsets, thin obstacles, and explicit
false overrides. Round-trip tests alone cannot detect a symmetric conversion
bug. Preserve existing spell-coverage, route, sight/lean-around, wall, door,
composition-rendering, and editor-cancellation behavior.

Run provider tests at each changed boundary; verify canonical transport and
storage round trips; then exercise actual browser gestures, export/reopen, and
the real game route. The same declared geometry must reach builder and game.
A green compiler or a screenshot alone is not the acceptance record.

## 8. Review boundary and next artifact

This is an architecture proposal, not an implementation plan. It deliberately
separates two sizeable questions before code is assigned:

1. The **authoring/placement contract**: exact new YAML and wire types, definition
   copying/overrides, continuous geometry and thin-obstacle policy, and legacy
   coexistence. Prove it through Proof A.
2. The **cover contract and consumers**: explicit prop capability through the
   source/target geometry and D&D rule application. Prove it through Proof B.

Kirk reviews this direction, especially the proposed document-local definitions
and the two-proof decomposition, before those contracts and their plans are
written. Follow current owning-repository release rules when implementation is
authorized; do not copy the historical PR sequence from older dungeon designs.
No code, publication, merge, deployment, shared-stack change, or content
migration is authorized by this draft.

### Corrections left visible

- "Multi-hex support already shipped" became "coverage math shipped; prop
  occupancy still needs the integration." The source distinction matters.
- Earlier dungeon design examples describe retired edge dialects and cosmetic
  placement limits. They remain history, not this format's constraint.
- Terrain PR #429 proposed definition-only booleans. This conversation explicitly
  needs a placement's movement checkbox; this draft proposes defaults plus
  instance overrides rather than silently treating the earlier proposal as law.
- The assistant initially explored height/stance and illusion behavior. Kirk
  clarified explicit configurable properties instead; those systems are not
  prerequisites.

— cross-team agent, on behalf of KirkDiggler
