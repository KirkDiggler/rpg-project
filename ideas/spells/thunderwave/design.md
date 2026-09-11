---
status: DESIGN, proposed 2026-09-11
journey: rpg-project#430 (directed movement, step 1) · rpg-project#243 (cast a spell) · builds on: ../../battlemap/directed-movement/design.md (the directive) and ../../battlemap/terrain/design.md §3.2 (coverage, pulled forward here) · mirrors: ../ward-and-area/design.md (Thunderclap, the shipped area)
law: the client sends a cell, never a shape; session announces the target kind and routes the geometry to encounter; encounter covers, resolution saves and imposes, encounter pushes
---

# Thunderwave — the first shape, and the first push

## 0. Why this spell, and what it proves

Kirk, 2026-09-11: *"we are not bound to 2014 and the PHB. see thunder clap. we
are here to prove the engine works … shapes into spatial will have a lot of
use cases and we get forced movement to boot."*

Thunderwave is the first customer of two tools and proves both in one walk:

1. **Coverage.** A shape at a transform on a grid becomes cells at a
   threshold. The cube is the first shape through it; cones, Fireball's
   spread, and every multi-cell prop follow.
2. **The directive.** An effect moves a creature. The push is the least
   permissive directive there is: a line, two cells, pays nothing, provokes
   nothing, stops in front of whatever the fold refuses.

And one seam it opens on the way, which Fireball needs next: **a cell as a
target**, sent by the client as a reference.

The walk: the bard faces two skeletons in the hall, the pillar behind one of
them. Thunderwave toward the far one. Both are covered, both save, both fail.
The open one slides two cells. The one in front of the pillar slides one and
stops, and the story names the pillar. Nobody's reaction fires.

## 1. What is true today

Verified 2026-09-11: rpg-toolkit `origin/main` `dd8620ec`; rpg-api-protos
`origin/main` `5768b06`; rpg-api and rpg-dnd5e-web on `dev` (`27c92d8`,
`4263be57`). The cast door lives on `dev` in those two repos; rpg-api `main`
has no cast handler at all.

**Thunderclap is the shipped area, and it is the template.**
`spells/cast.go:181-212`: `Target: CastTargetArea`, `Area: &CastArea{Footprint:
{Shape: AreaRadius, SizeFeet: 5, Origin: AreaOriginCaster}, Catches:
AreaCatchesOthers}`, a CON gate with `OnSuccess: Negated`, `Damage: 1d6 Thunder`.
`combat/actions/area.go` has exactly one shape (`AreaRadius`, `:17`) and one
origin (`AreaOriginCaster`, `:29`); its own doc at `:19-25` names a chosen
point as the next origin and warns it breaks the caster-centred range check.
`Footprint.Validate` (`:68`) closes both switches.

**The member set is derived in session and the geometry is one linear
scan.** `deriveAreaMembers` (`session/area.go:69-147`) switches on origin and
shape and, for a radius, calls `enc.MembersWithin` (`encounter/shape.go:70-89`),
which walks every member comparing `e.Distance`. Resolution takes the result
as `ActionInput.AreaMembers` (`resolution/action.go:37-56`). The root
`AGENTS.md` worked example is this exact seam: geometry belongs to encounter,
and the shape switch sitting in session is the thing the ownership test was
added to catch. This design moves the switch down, not up.

**A target kind arrives only with its executor.** `session/types.go:2454-2491`:
`TargetNone | TargetMember | TargetArea | TargetPath`, and the comment at
`:2459`. The path executor is the model: the offer announces `TargetPath` with
empty candidates (`offers.go:622-657`), and the cells travel on the **verb
input**, `MoveInput.Path` (`move.go:58`), tied by `DeclarationID` to the
compiled offer (`move.go:221-230`). Nothing about the geometry is in the
declaration.

**The wire copies targets verbatim.** `rpg-api …/session/v1alpha1/cast.go:40-71`
hands `req.GetTargets()` to `sdk.CastInput{Target, Targets}` with no
conversion. A cell is a new field there and on the proto.

**The client already has a ground click, and movement owns it.**
`SessionCanvas` takes `onHexClick(coord)` (`SessionCanvas.tsx:181`), entity
clicks win over floor (`:426-439`), and `SessionEncounterView.tsx:1699` wires
it straight to `walkTo`. A member-target cast arms at
`useSessionCombatExperience.ts:659-672` and fires on the member click; an area
cast fires on the arm click itself (`:1085-1092`).

**The hex grid has no plane.** `AxialHexGrid` (`tools/spatial/hex_grid.go:293`)
carries no orientation; encounter holds `OrientationPointyTop | FlatTop`
separately (`encounter/orientation.go:45-49`). Every cell-to-plane routine is
unexported on `hexGeom` in `encounter/dungeonspec/geometry.go`: `centreOf`
(`:255`), `hexOf` the six corners (`:260`), `world` (`:247`), `directionOf` the
bearing (`:284`), `standingFraction` (`:412`), and there is no plane-to-cell
inverse. Its header says it moves to spatial when a second customer needs a
corner. Coverage is that customer (terrain design §3.2).

**The bard knows one levelled spell.** `character/choices/requirements.go:430-437`:
`Options: []spells.Spell{spells.Bane}`, `Count: 1`. Bane's slot is `baneCost()`
(`spells/cast.go:144-149`, `SpellSlotLevel1: 1`), the only profile that spends
one.

**Thunderwave's row and constant are leftovers.** `spells/data.go:161`,
`types.go:74`. Kirk, 2026-09-11: not decisions. The profile below is the
spell.

## 2. Ownership

| noun | owner |
|---|---|
| the shape, its size, its anchoring rule, the threshold | content (the profile) declares; encounter applies |
| the chosen cell | the client sends it; session validates it is not the caster's cell; nobody else reads it as geometry |
| which cells the cube covers | `tools/spatial` (coverage over an embedding) |
| which members stand on covered cells | encounter (`MembersCovered`) |
| the save, the one roll, the imposed damage and the imposed move | resolution |
| the push itself, step by step, beats with a cause | encounter (`Direct`, from the directive design) |
| the cell on the wire | protos, additive; rpg-api maps it, never inspects it |
| picking a cell | the client, reusing the ground click while a cell-target cast is armed |

## 3. The shape

**Coverage needs an embedding.** Spatial gains `HexEmbedding{Orientation}`
(the moved `hexGeom`: cell centre, six corners, bearing between cells) and

```go
type CoverageInput struct {
    Footprint Footprint  // Box{W, D float64} in feet today; Polygon later
    At        Position   // the anchor cell
    Facing    float64    // degrees, from the embedding's bearing
    Anchor    AnchorRule // AtCentre | AtEdge: whether the footprint's near edge sits on the cell's boundary
}
type CoverageOutput struct {
    Cells map[Position]float64 // fraction of each cell's area under the footprint, (0,1]
    Edges []Edge               // boundaries the outline crosses (thin things; unused here)
}
func Coverage(emb HexEmbedding, g Grid, in CoverageInput) (CoverageOutput, error)
```

Fraction of a hexagon under a rotated rectangle is a convex clip; the
embedding already computes a related fraction for walls (`standingFraction`).
Thresholds stay out of spatial.

**The cube.** A 15-foot cube is a box 15 wide and 15 deep in the plane.
`AreaOriginCasterEdge` anchors its near edge on the caster's hex boundary
along the bearing from the caster's cell to the chosen cell, so the far edge
is three cells out and the caster's own cell falls below the threshold. (Not
"never under it": measured in rpg-toolkit#1656, off a grid axis the caster's
cell is covered at about one percent because its own corner pokes past the
near edge. The half rule is what keeps the caster out, and encounter asserts
that after the threshold, not before.) Encounter applies
`CoverageThreshold = 0.5`, the tabletop's half rule and ours, as a named
constant in `encounter/shape.go` beside `MembersWithin`, comparing with a
small tolerance: four cells on an axis bearing sit at exactly one half, and
"at least half" must not turn on the last bit of a float.

```go
// encounter
type MembersCoveredInput struct {
    Footprint spatial.Footprint
    Anchor    spatial.Position
    Toward    spatial.Position // the chosen cell; refused when equal to Anchor
    AtEdge    bool
}
func (e *Encounter) MembersCovered(in MembersCoveredInput) (MembersCoveredOutput, error)
```

It builds the embedding from its own orientation, asks `Coverage`, keeps
cells at or above the threshold, and returns the members standing on them in
a stable order. `MembersWithin` stays for radius.

**Not bound to the letter, and where that bites.** On a hex grid a "cube"
has no honest square; half coverage gives a blob three cells deep and two to
three wide depending on the bearing. That is the rule working, not a bug,
and the acceptance test draws it at an axis bearing and an off-axis bearing
so the shape is seen once and never argued again.

## 4. The target

`TargetCell` joins the session kinds, **with its executor in the same wave**:

- The offer for a cast whose profile says `AreaOriginCasterEdge` announces
  `TargetCell` with empty candidates, like `TargetPath`.
- `CastInput` gains `Cell *spatial.Position`. Session refuses a nil cell for a
  `TargetCell` offer, refuses the caster's own cell, and passes the cell to
  `deriveAreaMembers`, which for this origin calls `MembersCovered` and
  otherwise does what it does today. The shape switch leaves session.
- The proto `CastRequest` gains an optional `Position cell` beside `targets`;
  `TargetKind` on the wire gains `CELL`. Additive, per the protos rule.
- rpg-api copies it into the SDK input. No inspection.
- The client: when the armed declaration's kind is `CELL`, the ground click
  goes to the cast instead of `walkTo`. One routing decision at the existing
  `onHexClick` seam; entity clicks still win, so clicking a skeleton while
  armed is refused rather than misread as a cell. No preview of the covered
  cells in this slice; the server derives, and the beat shows who was caught.

The range check stays caster-centred because the cube originates from the
caster. A cell at range, Fireball's origin, is the change `area.go:19-25`
warned about and is not this slice's.

## 5. The cast

```go
Thunderwave: {
    name: "Thunderwave",
    build: func(in castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            Cost:      slotCost(resources.SpellSlotLevel1), // baneCost, generalised
            RangeFeet: 0,                                    // originates from you
            Target:    actions.CastTargetArea,
            Area: &actions.CastArea{
                Footprint: actions.Footprint{Shape: actions.AreaBox, SizeFeet: 15, Origin: actions.AreaOriginCasterEdge},
                Catches:   actions.AreaCatchesOthers,
            },
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.CON},
                DC:         saves.DCStatic(in.SpellSaveDC),
                OnSuccess:  saves.Negated, // Half when #414 lands; content flips, no code here
                Recurrence: saves.RecurrenceNone,
            },
            Damage: []damage.Damage{{Dice: "2d8", Type: damage.Thunder}},
            Move: &actions.CastMove{          // NEW, from the directive design
                Policy:   actions.MoveLine,
                Cells:    2,
                Pays:     actions.PaysNothing,
                Provokes: false,
            },
        }
    },
},
```

`CastProfile.Move` is the declaration; resolution turns it into
`ImposedMove` on a failed save, the way `Application` becomes an imposed
condition. Anchor is the caster. `Validate`: `MoveLine` requires
`Cells > 0`; `Pays`/`Provokes` zero values are the push, which is the point.

**Order on a failure, ruled here.** Damage first, then the push. A creature
the damage drops is not pushed: `Direct` already stops when the mover is
down, and a body sliding across the floor is not a story we tell. Targets
resolve in the stable order the cast already uses, so two pushes never race:
the second sees the first's result.

**One roll for all.** The 2d8 is rolled once and applied to every failure,
per ward-and-area §3's trap and its acceptance #10.

**The bard's pick** becomes `Options: {Bane, Thunderwave}`, still `Count: 1`.
The sandbox seed gives the walk's bard Thunderwave.

## 6. Acceptance contract

Through production entrypoints, mirroring ward-and-area §5.

1. **Coverage draws the cube.** At an axis bearing and at an off-axis
   bearing, the covered cells are exactly the ones a drawn 15-foot box over
   the hex plane covers by half or more; the caster's cell is never covered;
   the test states both cell sets so the shape is seen once.
2. **Derivation, not selection.** Every creature on a covered cell is caught,
   allies and unseen creatures included, the caster excluded.
3. **The cell is a reference.** A `TargetCell` cast with no cell is refused;
   with the caster's own cell is refused; the refusal names the reason.
4. **One roll for all, per-target saves.** As Thunderclap's #10 and #11.
5. **The push obeys the fold.** A failed skeleton with open floor slides two
   cells along the line from the caster; one with the pillar behind it slides
   one and the outcome names the pillar; one with a wall directly behind it
   does not move and the outcome says so.
6. **The push provokes nothing.** A skeleton pushed out of the fighter's reach
   triggers no opportunity attack, and the beat carries the cast as its cause.
7. **The dropped are not pushed.** A skeleton the damage reduces to zero stays
   where it fell.
8. **Zero targets is not an error.** A cube over empty floor pays the slot
   and the action and records honestly.
9. **The slot.** Thunderwave spends a level-1 slot; a bard with none is not
   offered it.
10. **Regression.** Thunderclap, Bane, Blade Ward, Sacred Flame, Vicious
    Mockery and True Strike resolve unchanged; the bard's levelled pick
    becomes 1-of-2.

## 7. What this slice borrows, said plainly

- **Half on a save** (#414). Walked as `Negated`. We are not bound to the
  letter; the row flips when half exists.
- **The directive** (#431): `ImposedMove`, `Route`, `Direct`, the beat's
  cause. This slice builds the `Line` policy and the `CastMove` declaration
  and consumes the rest.
- **No client preview.** The caster does not see the cube before casting.
  The honest preview is the server's coverage on the atlas, a later slice.

## 8. Rejected alternatives

- **A radius push, Thunderclap plus shove.** Proves the directive and nothing
  about shapes. The point of choosing this spell was the shape.
- **A direction on the wire instead of a cell.** A bearing is a calculation;
  a cell is a reference. And Fireball needs the cell.
- **Computing the cube in session's shape switch.** The root `AGENTS.md`
  worked example. The switch moves to encounter.
- **Snapping the cube to hex axes.** Loses the bearing the caster chose and
  makes the shape depend on which of six directions is nearest. Coverage at
  half is the rule; use it.
- **A client-side covered-cell preview computed from the shape.** A
  calculation in the client. When a preview arrives it comes from the server.

## 9. Sequence, one module per PR, merged bottom-up

1. `tools/spatial`: `HexEmbedding` (the moved `hexGeom`), `Footprint`,
   `Coverage`. Closes rpg-toolkit#1626 by construction (a cone is a polygon
   through the same call; not built here).
2. `encounter`: `dungeonspec/geometry.go` now calls spatial's embedding;
   `MembersCovered`, `CoverageThreshold`; `Route{Line}` and `Direct` from the
   directive design.
3. `combat/actions`: `AreaBox`, `AreaOriginCasterEdge`, `CastMove`.
4. `resolution`: `ImposedMove` produced from `CastMove` on a failed save;
   damage before move; dropped creatures skipped.
5. `session`: `TargetCell` with its executor; `CastInput.Cell`; the shape
   switch routed to encounter.
6. `spells`: the profile; `slotCost`; the bard's 1-of-2.
7. `rpg-api-protos`: `cell` on `CastRequest`, `CELL` on `TargetKind`.
8. `rpg-api`: the field copied through.
9. `rpg-dnd5e-web`: the armed-cell routing at `onHexClick`.

Walked on the local stack from pushed pseudo-versions before anything leaves
draft, as rung 1 was.

## 10. Where the evidence is thin

- Not run. No production code, no test claimed passing.
- The hexagon-under-rectangle clip is asserted to be a small convex clip;
  `standingFraction` is the nearest existing code and was not read line by
  line.
- Whether `AxialHexGrid`'s `GetLineOfSight` line is the "straight away" a
  table expects for the push at off-axis bearings (directed-movement §11).
- rpg-api and the web were read at `dev`; the wire's `TargetKind` enum and the
  exact `CastRequest` message were not opened in the proto file itself.
- The sandbox seed's spell list for the bard was not read.

— cross-team agent, on behalf of KirkDiggler
