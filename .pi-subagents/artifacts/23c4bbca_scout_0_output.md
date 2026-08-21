# Dungeon Builder v0.4 — evidence brief

## Executive finding

The strongest local v0.4 authority is **not on `main`**. It is the pushed ref `spec/v0.4-proposal` / `origin/spec/v0.4-proposal` at `40a3938fff9093ae2913da9ea100d8ab63193cb3`, whose files say **RATIFIED v0.4 (2026-08-08)**. The branch name is stale: the final commit ratifies the contract. However, the same authority expressly says v0.4 **claims no downstream implementation**, PR #203 remains open as the tracking surface, and issue #206 owns acceptance.

The wall contract is two-layered:

- canonical/server input remains edge-native YAML **`walls:`** plus server-generated canonical envelope edge pairs;
- **`wallLines` is client-local**, never a server/YAML wire construct. It may be losslessly compiled to canonical region cells/`walls:` before validation, otherwise the client must stop.

Therefore, **a server-side Tranche-A mask/envelope/edge implementation is required**, but **no server `wallLines` or straight-wall-run slice is specified**. Whether Tranche A is managed as a separately named “walls slice” is not settled locally.

## Files Retrieved

1. `ideas/dungeon-builder/design.md` (main, lines 3-33) — v0.3 authority, Wave 0/1 scope, delivered authored-edge history, and stale Wave 1 status.
2. `ideas/dungeon-builder/plan.md` (main, lines 3-33) — v0.3 implementation order and evidence gates; also says Wave 1 had not started.
3. `ideas/dungeon-builder/spec/v0.3/spec.md` (main, lines 1-18, 24-61, 221-259, 326-453) — ratified baseline; `walls:` is edge-native while `wallLines:` was above v0.3.
4. `ideas/dungeon-builder/spec/v0.3/README.md` (main, lines 18-42, 218-226) — spec-level cadence and historical straight-wall footprint context.
5. `spec/v0.4-proposal:ideas/dungeon-builder/spec/v0.4/spec.md` (lines 1-45, 64-230, 407-417, 419-536) — final ratified v0.4 delta, canonical envelope/wall contract, all cross-layer acceptance specimens, scope, and tracking status.
6. `spec/v0.4-proposal:ideas/dungeon-builder/spec/v0.4/README.md` (lines 1-29, 45-95, 172-230) — v0.4 rationale, delivered-v0.3 claim, explicit no-implementation claim, client-local boundary, reconciliation, and issue/PR pointers.
7. `ideas/dungeon-walls/design.md` (main, lines 16-35, 147-167) — older visual-wall-run design: server rules unchanged, wall meshes/runs client-side, server-emitted wall runs deferred.
8. `ideas/dungeon-walls/plan.md` (main, lines 1-17, 21-64) — older web-only run/render slices, explicitly no toolkit/proto/API dependency.
9. `sessions/active.md` (main, lines 1-19) — stale local handoff saying Wave 1 had not started.

## Key Code / Contract

Final v0.4 topology discriminator (`spec/v0.4-proposal:.../spec.md:68-85`):

```yaml
canvas:
  width: 12
  height: 8
  floor_source: regions
rooms: []
```

- omission/`bounds` keeps the v0.3 full rectangle;
- `regions` makes the deduplicated union of `regions[].cells` the structural floor;
- in-bounds cells outside that union are void.

Existing authored wall shape (`ideas/dungeon-builder/spec/v0.3/spec.md:221-251`):

```yaml
walls:
  - { from: [7, 0], to: [7, 1], kind: solid }
  - { from: [7, 4], to: [7, 5], kind: door }
```

Authored `walls:` requires two distinct adjacent floor endpoints; identity is an undirected normalized pair, reversed duplicates reject, doors derive identity from the normalized endpoints, and runtime uses `HexRecord.edges` rather than a flat `Space.walls` field (`v0.3/spec.md:229-251`).

Final v0.4 generated envelope rules (`spec/v0.4-proposal:.../spec.md:117-143`):

- every floor-to-void/off-canvas side becomes an implicit generated solid edge;
- `FloorPlan.edges` reuses the existing flat `{from,to,kind}` pair;
- exactly one endpoint is in `floor_cells` and owns the edge; the other is the actual adjacent void/off-canvas coordinate;
- runtime attaches it only to the owning floor `HexRecord`; there is no void record or duplicate attachment;
- authored interior `walls:` retains the two-floor-endpoint v0.3 rule, and no authored envelope door exists.

Final client boundary (`spec/v0.4-proposal:.../spec.md:407-417`):

> `wallLines`, continuous wall footprints, and fractional coverage are client-local. They may compile losslessly into canonical region cells or `walls:` before `validate_only`; the platform accepts only canonical YAML and must not implement a particular TypeScript algorithm. Failure to compile losslessly hard-stops rather than approximating or stripping.

## Canonical status and git facts

- Working branch: `main` = `3b11d66a7b48561b15c12bc42001974a90691d4b`; its remote-tracking ref advanced during investigation to `origin/main` = `22aee544a42906c2f8c01a0e1eb4935c252dcda2`, so the checkout is one commit behind.
- Ratified v0.4 ref: local and origin `spec/v0.4-proposal` = `40a3938fff9093ae2913da9ea100d8ab63193cb3`.
- The ratification commit's parent is reviewed proposal head `0e97bc838f8d3bcc84852f58ee6b1383cb333ed2`; v0.4 records the bounded review as **GO with zero field-shape/transport blockers** (`v0.4/spec.md:525-530`; `README.md:205-215`).
- Merge base is `3ee1efbc214d5e77ddcbe84dea6acc1cdae86f1a`; local `main...spec/v0.4-proposal` is **10 main-only / 6 branch-only**, while current `origin/main...spec/v0.4-proposal` is **11 / 6**. Neither main ref contains v0.4 (`git branch --contains 40a3938` names only the v0.4 branch).
- The six branch commits are `2b188a4` → `9b2fd75` → `7814808` → `c37a1e1` → `0e97bc8` → `40a3938` (proposal through ratification).
- Final local text says PR **#203 remains open** through downstream delivery and issue **#206** is the owning tracker (`v0.4/spec.md:525-536`; `v0.4/README.md:217-230`). This is a recorded local claim, not a live GitHub query.

## Delivered versus unfinished

### Delivered / attested locally

1. **v0.3 authored edge-native `walls:`** is listed as already compiling/live-verified (`v0.3/spec.md:8-13`). Main's delivery record points to rpg-api-protos#206, rpg-toolkit#881 (`tools/spatial/v0.6.0`, `rulebooks/dnd5e/v0.71.0`, `encounter/v0.48.0`), and rpg-api#769; project issues #176-#179 remain open history but are called delivered (`design.md:31-33`; `plan.md:29-33`).
2. **v0.3 Wave 0 and Wave 1** are both called LIVE VERIFIED by the later v0.4 authority, with Wave 0 evidence on #192 and Wave 1 evidence on #180 (`v0.4/README.md:24-28`).
3. **v0.4 contract review/ratification only** is delivered: ratified status and no remaining spec-shape questions (`v0.4/spec.md:3-10,525-530`).

### Not delivered / acceptance still unfinished

The v0.4 README explicitly says v0.4 “still claims no implementation” (`README.md:24-29`) and “ratification ... does not claim downstream implementation” (`README.md:205-215`). Consequently **all five normative §9 cross-layer specimens remain without local implementation evidence**:

1. **Tranche A irregular floor/envelope** (`spec.md:419-443`): exact sorted region-union mask, outer/interior-void generated pairs with one floor owner, deterministic entrance, consistent write/start/snapshot reload/pathing/targeting/placement/LoS/fog, and atomic rejection when content loses floor.
2. **Tranche A draft/run lifecycle** (`spec.md:445-456`): tiny masks preview with absent entrance but fail strict write; two islands distinguish PartyCap seating from whole-floor connectedness.
3. **Tranche B placement offset** (`spec.md:458-488`): exact optional triple through authoring/runtime and ref edit, with no mechanical leakage.
4. **Tranche C monster config** (`spec.md:490-503`): three-layer merge, scalar presence, room/canvas/boss parity, exact source-path errors, prop rejection, provider handoff, and no runtime state in YAML.
5. **Tranche D lighting** (`spec.md:505-514`): raw/effective presence, inherited/absent values, fog authorization, and proof lighting cannot affect LoS/visibility/reveal/fog/targeting.

No local v0.4 downstream proto/toolkit/API/web implementation commits, releases, or test evidence are cited. The documentation revision expressly created no implementation issues (`README.md:228-230`).

## Is a server walls slice expected?

**Functional answer: yes for generated canonical envelope edges; no for `wallLines`/straight render runs.**

- Tranche A requires the compiler/server to derive one mask used by mechanics (`spec.md:105-115`), generate solid floor/void envelope pairs (`spec.md:117-143`), project full authoring edges and fog-safe runtime ownership (`spec.md:218-230`), and snapshot those edges with encounters (`spec.md:199-216`). This cannot be satisfied by client rendering alone.
- It reuses the existing edge pair wire; the spec says there is “no open wire decision” (`spec.md:141-143`). A new wall message is not expected.
- `wallLines` is explicitly client-local (`spec.md:407-417`, scope table `518-523`). The older dungeon-walls design likewise leaves wall meshes/runs client-derived and defers toolkit-emitted wall runs (`ideas/dungeon-walls/design.md:147-155`; `plan.md:3-8`).
- No locally available text mandates a separately named server “walls slice.” Issue #206/PR #203 track the whole v0.4 contract; implementation issue decomposition was intentionally not created. Treat “server walls slice” as a planning choice for Tranche A, not as a missing `wallLines` API.

## Findings / residual risks

- **[high] Default-branch authority gap — `main` lacks the ratified v0.4 files.** Implementers reading only main see v0.3 and can miss the ratified delta. Evidence: branch containment/merge-base facts above.
- **[high] Do not implement `wallLines` server-side.** The normative endpoint is canonical region cells/`walls:`; server work is mask/envelope edge derivation and transport, not continuous-line or fractional-coverage acceptance (`v0.4/spec.md:407-417`).
- **[medium] Status drift inside local documentation.** Main `design.md:31-33`, `plan.md:33`, and `sessions/active.md:5-15` say v0.3 Wave 1 had not started, while later v0.4 `README.md:24-28,193-203` says both waves are LIVE VERIFIED and #180 complete. The later v0.4 text is the relevant v0.4 reconciliation, but the stale main text should not be used as current delivery status.
- **[medium] Generated versus authored pair validation can be confused.** Authored `walls:` still requires two floor endpoints; generated envelope `FloorPlan.edges` deliberately has exactly one floor endpoint. Applying authored-source validation to projected envelope pairs would reject the v0.4 contract.
- **[medium] All v0.4 cross-layer acceptance remains unattested locally.** Ratification is not implementation; PR #203/#206 live state and any downstream repo work require external verification.

## Unresolved questions

1. Has PR #203 or downstream work changed after the local refs were last updated? Local evidence says open/no implementation, but no live GitHub state was queried.
2. Will #206 split Tranche A into a separately named server walls/edges slice, or keep it within one topology vertical? The spec mandates behavior, not issue decomposition.
3. When will `main` receive the ratified v0.4 files and reconcile the stale Wave 1 delivery statements?
4. Which downstream proto/toolkit/API/web versions and evidence comments will attest each §9 specimen? None are named in the final local v0.4 documents.

## Architecture

The builder may use rich local editing conveniences, but submits strict canonical YAML. The toolkit/compiler owns validation and derivation (region-union floor, envelope edges, entrance, placement/config/lighting resolution). The API transports rather than reinterprets. Authoring receives the full canonical projection; encounter runtime receives only fog-authorized `HexRecord`/zone/placement/lighting data. Authored YAML and running encounter snapshots have separate persistence lifecycles.

## Start Here

Start with `spec/v0.4-proposal:ideas/dungeon-builder/spec/v0.4/spec.md`, especially lines 64-230 (Tranche A), 407-417 (`wallLines` boundary), and 419-536 (acceptance/status). Then read v0.3 `spec.md:221-259` for the baseline authored `walls:` rules.