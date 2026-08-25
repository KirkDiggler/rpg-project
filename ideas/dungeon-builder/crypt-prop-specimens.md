# Builder-ready crypt prop specimens — design (rpg-project#275)

Addendum to the dungeon-builder design (`design.md`) under journey
[rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169).
Approved in conversation with Kirk on 2026-08-25.

**One sentence:** hand-assemble three complete crypt props in the private asset
repository, then expose only those visually approved artifacts through the
builder and the playable dungeon.

## 1. Outcome and quality bar

An author can place a complete skeleton cage, a complete skeleton-at-table
scene, and a correctly proportioned rug. Each reads as an intentional prop in
both the dungeon builder's 3D preview and the playable dungeon under
representative existing lighting.

The visual target is the density and legibility of Synty's POLYGON Dungeon and
Dungeon Map scenes:

- [POLYGON Dungeon Map reference](https://syntystore.com/cdn/shop/products/Screenshot_06_5c689d50-1f60-41ed-b495-b3a22a5573b7.png?v=1765773635&width=1913)
- [POLYGON Dungeon Pack reference](https://syntystore.com/cdn/shop/products/Screenshot_03_c485f90b-649c-4f45-910c-942892e7f0c6.png?v=1763950806&width=1913)

This wave does not attempt to reproduce either whole scene. It establishes the
specimen and approval flow needed to build toward that bar one trustworthy prop
at a time.

## 2. Verified current failure

The current catalog equates "promoted GLB exists" with "finished prop is ready
to author." The three specimens disprove that assumption:

- `dnd5e:props:skeleton-cage` resolves to
  `SM_Prop_Skeleton_Cage_01.glb`. Its builder thumbnail shows only a skeleton,
  although `library/prop-role-map.json` describes a hanging iron cage with a
  skeleton inside. The same pack contains the separate
  `SM_Prop_Toture_Cage_01.glb` support mesh, but it is not promoted or paired.
- `dnd5e:props:skeleton-table` resolves to
  `SM_Prop_Skeleton_Table_01.glb`. Its thumbnail shows only a posed skeleton,
  although the role map describes remains draped over a table. The same pack
  contains separate table meshes, including `SM_Prop_Table_01.glb`.
- `dnd5e:props:rug` exposes three raw variants. `SM_Prop_Rug_01.glb` measures
  roughly 12.94 m by 2.93 m before the web's shared Synty scale, and the web
  then applies an additional `renderScale: 2` to every rug variant. The live
  result is tracked by
  [rpg-dnd5e-web#642](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/642)
  as a red crumpled mass rather than a rug.

The private generated manifest repeats the incorrect descriptions, and the
public web mirror faithfully repeats that bad source truth. The failure is
therefore at promotion and visual approval, not merely in the builder UI.

## 3. Decisions and governing laws

### 3.1 Finished specimens before composability

Version 1 publishes one finished GLB per placeable prop. A finished GLB may
contain multiple named mesh nodes, but the builder and game treat it as one
asset with one anchor, facing, footprint, and semantic ref.

Runtime companion composition, component-level authoring, and breaking an
approved specimen back into independently placeable pieces are deferred. The
private provider keeps enough component provenance to support that future work
without making it part of this wave.

### 3.2 The asset provider owns visual truth

`rpg-game-assets` owns:

- the private Blender `.blend` assembly source used for the final export;
- source-pack and component provenance;
- component transforms;
- final scale, pivot, bounds, forward direction, and footprint;
- the promoted GLB;
- neutral and in-context visual evidence; and
- generated private manifests and mesh statistics.

Raw converted source components remain in `library/`. The provider MUST NOT
overwrite a raw source GLB to disguise it as an assembly. A finished assembly
is a distinct provider artifact whose provenance names every source component.

### 3.3 The web consumes; it does not repair

`rpg-dnd5e-web` resolves the stable semantic ref to the approved provider
artifact. It MUST NOT add a specimen-specific scale, rotation, offset, or
missing support mesh to make the prop look complete. In particular, the
corrected rug removes the current `renderScale: 2` exception.

The builder preview and playable dungeon MUST use the same prop resolver,
artifact URL, `PropModel`, authored facing, and authored offset. The playable
dungeon remains the final smoke test, not a second place to tune the asset.

### 3.4 Presentation does not change mechanics

The existing semantic refs and gameplay roles remain stable:

| Ref | Role | Movement | Line of sight |
|---|---|---:|---:|
| `dnd5e:props:skeleton-cage` | obstacle | blocks | blocks |
| `dnd5e:props:skeleton-table` | cover | blocks | does not block |
| `dnd5e:props:rug` | decor | does not block | does not block |

A visual correction does not move footprint or blocking rules into the web.
If the finished geometry shows that an existing gameplay role is untenable,
implementation MUST stop and earn a separate rules-contract decision rather
than silently changing mechanics in this wave.

### 3.5 Lighting is adjacent

These props are approved under representative lighting already available to
the dungeon renderer. This wave does not add a lighting field, light type,
palette control, shader, or runtime light behavior. Authored lighting is the
next visual-fidelity slice under journey #169, using
[rpg-project#190](https://github.com/KirkDiggler/rpg-project/issues/190) as
historical input after it is reconciled with the current stack.

## 4. The three specimen contracts

### 4.1 Skeleton cage

- Assembly starts from `SM_Prop_Skeleton_Cage_01` and
  `SM_Prop_Toture_Cage_01` from POLYGON Dungeon.
- The skeleton and cage read as one source-intended scene from the tactical
  camera.
- The cage is visibly present, the skeleton is contained by it, and neither
  component floats accidentally.
- The final artifact has one grounded placement anchor and one useful default
  facing. A deliberately suspended composition is allowed only if its support
  and mount read as intentional in the approved in-context view; unexplained
  floating is not allowed.

### 4.2 Skeleton at table

- Assembly starts from `SM_Prop_Skeleton_Table_01` and the same-pack
  `SM_Prop_Table_01` support.
- Before final export, the provider may substitute another same-pack table only
  if the neutral candidate comparison shows a materially better physical fit;
  the provenance record MUST name the substitution and preserve the rejected
  comparison.
- The body is visibly supported, with no obvious gap and no destructive
  intersection at the tactical camera.
- The final artifact has one grounded placement anchor and one useful default
  facing.

### 4.3 Rug

- The provider compares `SM_Prop_Rug_01`, `_02`, and `_03` at measured scale in
  the shared showcase scene.
- Exactly one reviewed variant becomes builder-ready in this wave. The other
  raw variants remain in `library/` until separately reviewed.
- The approved rug lies on the floor, reads as textile rather than rubble or a
  crumpled mass, and fits the showcase room without a web-only scale override.
- Its final exported dimensions, not a consumer multiplier, determine its
  presentation size.

## 5. Provider artifact and promotion contract

Each specimen records:

- stable semantic ref;
- final provider artifact path and SHA-256;
- source pack;
- ordered component names and source paths;
- per-component translation, rotation, and scale in the assembly;
- final axis-aligned bounds in metres;
- pivot and floor-contact convention;
- forward direction;
- intended footprint and existing gameplay role;
- triangle, vertex, material, texture, animation, and file-size statistics;
- neutral evidence paths; and
- builder-preview and playable-dungeon evidence paths.

The final GLB keeps component node names where practical. Geometry need not be
destructively joined merely to make the result one file. This preserves a
clean route to future decomposition without exposing composition to runtime
now.

Promotion order is strict:

1. assemble a candidate in Blender;
2. render neutral front, side, three-quarter, and top views;
3. verify transforms, bounds, pivot, materials, and mesh statistics;
4. render the candidate in the shared showcase dungeon;
5. receive Kirk's visual approval;
6. publish the finished GLB into the `harness/models/synty/` consumer tree;
7. regenerate manifests, complete inventory, and mesh statistics; and
8. stage and verify the exact consumer bundle.

A loadable candidate that has not completed step 5 is not builder-ready.
Existing WARN-only performance budgets remain warnings, but a warning on one of
these specimens requires an explicit disposition in the provider PR rather
than being ignored.

## 6. Data flow and catalog boundary

The runtime data flow remains:

```text
Dungeon YAML semantic ref
  -> dungeonspec and session atlas carry the opaque ref unchanged
  -> web prop resolver selects the approved artifact
  -> shared PropModel renders authored facing and offset
  -> builder preview and playable dungeon show the same asset
```

No proto, toolkit, or API change is expected. The implementation plan MUST stop
and reclassify the wave if inspection discovers that the stable refs or current
roles cannot represent the approved props.

For these three keys, the web catalog exposes exactly one approved variant per
key. "Builder-ready" is the result of that promotion and evidence gate, not a
new YAML, wire, or runtime status field. A builder thumbnail is evidence for
that exact artifact, not a generic icon for an unreviewed family. Broad
generation of the web's hand-maintained
`propManifest.ts` mirror remains
[rpg-dnd5e-web#624](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/624),
and server-side prop-ref validation remains
[rpg-project#185](https://github.com/KirkDiggler/rpg-project/issues/185).
Neither is required to prove these specimens.

## 7. Shared showcase dungeon

The web consumer owns a small reusable authored fixture containing separated,
readable placements for all three specimens. It uses current dungeon YAML,
current authored facing and offset, and fixed values from the existing dungeon
lighting path. The checked-in fixture and lighting inputs are unchanged between
builder and game review. It introduces no new lighting behavior.

The same fixture supports:

1. builder open and save;
2. builder 3D preview;
3. Save & Play or equivalent local launch; and
4. tactical-camera review in the playable dungeon.

Save and reopen MUST preserve each prop's ref, facing, and offset. Builder and
game evidence MUST identify the exact provider revision and artifact hashes so
a stale synced asset tree cannot masquerade as approval.

The fixture remains after this wave as the visual-review surface for later prop
batches.

## 8. Failure behavior

- A missing source component, invalid transform, non-finite bound, missing
  material, failed manifest generation, or provider inventory mismatch fails
  promotion.
- An unapproved variant stays in the private library and does not appear under
  these builder keys.
- A missing promoted artifact MUST NOT silently resolve to another variant or
  unrelated fallback. During development the existing explicit load/error
  presentation may render, but the palette cannot claim the specimen is ready.
- A mismatch between builder and game artifact URL or transform fails the web
  slice even if both views look individually plausible.
- A visual rejection returns the specimen to Blender assembly. It is not fixed
  with a web transform.

## 9. Verification

### Provider automation

- validate the assembly provenance record and every component path;
- inspect the final GLB for finite transforms, bounds, expected floor contact,
  materials, and texture presence;
- regenerate and check the prop manifest, complete inventory, and mesh
  statistics while preserving the existing safe-catalog verification;
- verify exact provider-stage inventory and hashes; and
- retain neutral multi-angle evidence for each accepted specimen and rejected
  rug/table candidate.

### Web automation

- every one of the three refs resolves to its one approved artifact;
- no specimen-specific `renderScale` or repair transform remains;
- builder preview and game scene use the same resolver and `PropModel` inputs;
- the showcase fixture round-trips ref, facing, and offset; and
- thumbnails name the same approved artifacts represented by the catalog.

### Human visual gate

Kirk reviews all three specimens in:

1. the builder 3D preview; and
2. the playable dungeon at the tactical camera.

Neutral renders prove inspectability, not quality. Automated tests prove
identity and transforms, not whether a skeleton looks supported. Candidate
review through the local non-mutating asset source authorizes provider
promotion; the final repeat against the exact merged provider revision
authorizes the web merge. The wave does not merge until both real views receive
the final visual approval.

## 10. Delivery and merge order

This design is the Decide slice, rpg-project#275. After Kirk approves this
written file, its implementation plan creates only the earned repository
slices:

1. **`rpg-game-assets` Build slice and PR against `main`** — authoring sources,
   assemblies, provenance, three promoted GLBs, manifests, statistics, and
   provider evidence.
2. **`rpg-dnd5e-web` Fix slice and PR against `dev`** — pin the exact merged
   provider revision, sync the artifacts, update the three catalog
   entries and thumbnails, remove the rug workaround, add the showcase
   fixture, and capture builder/game evidence.

Development may begin outside-in with the web showcase fixture against a local
non-mutating asset source. Merge remains provider first, then consumer: the web
PR pins an actual merged asset revision.

No toolkit, API, proto, or deployment slice is planned.

## 11. Stale-issue reconciliation

After the implementation evidence lands:

- [rpg-project#132](https://github.com/KirkDiggler/rpg-project/issues/132) is
  closed as superseded by journey #169 and this specimen flow; its Synty-bar
  outcome remains preserved here.
- [rpg-game-assets#35](https://github.com/KirkDiggler/rpg-game-assets/issues/35)
  is closed as superseded. Its mixed promotion brief is not implemented as
  written; any remaining desired asset earns a fresh, visually gated slice
  only when selected.
- [rpg-dnd5e-web#642](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/642)
  closes through the corrected rug consumer evidence.
- rpg-project#185 remains a separate authoritative-catalog decision.
- rpg-project#190 becomes input to the next authored-lighting design.

Historical issues are not closed merely because this design exists; the
provider and consumer evidence earn the disposition.

## 12. Not now

- runtime-composable prop assemblies;
- independently placeable cage, skeleton pose, or table components;
- every crypt prop or every Synty pack asset;
- all three raw rug variants;
- generated web catalog distribution (#624);
- server-side prop-ref validation (#185);
- authored lighting controls or new runtime lighting (#190);
- texture or shader look development unrelated to the three specimens;
- gameplay interactions with cages, tables, or rugs; or
- changing blocking or line-of-sight semantics without a separate decision.

## 13. Done when

- The private provider holds three approved, provenance-tracked final GLBs.
- Each semantic ref resolves to exactly one approved artifact in the builder.
- The rug has no web-only scale exception.
- The showcase dungeon saves, reopens, previews, and launches with ref, facing,
  and offset intact.
- Builder preview and playable dungeon use the same artifact and transform.
- Kirk approves all three props in both real 3D views under representative
  existing lighting.
- Provider merges before the exact-pinned web consumer.

## Landed — 2026-08-25

- Provider slice: [rpg-game-assets#63](https://github.com/KirkDiggler/rpg-game-assets/issues/63)
  → [PR #64](https://github.com/KirkDiggler/rpg-game-assets/pull/64), merged to
  `main` as `6c24b19861df127faa69bd4d1ab6ec8fdfad537e` from feature head
  `951ac2a44dd40e0974e102434dbc7164665c571f` (same Git tree
  `37b17b93e82cea57fc1fa5a6e2dc3a6ed6d95bdb`). Approved artifact SHA-256:
  cage `fb16c3bed0fb284e37cfbe2914e7dfa2eeae54fb429796ab5ae1526f4126a4af`,
  table `be957e0e59b4efff6cbbcafba0a473e4e20f9dbca0a7892691121b8e477a0ff6`, rug
  `9f3861707a9b3416b89ddee307662fb7c3d106751f2a3f733ac902b7432184ed`.
  Complete inventory tree SHA-256: `eaf6e1f2128c0dfe134486c2d1c22ea9c9c4535bcb1adc9306b7522343367191`.
- Consumer slice: [rpg-dnd5e-web#814](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/814)
  → [PR #820](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/820), merged to
  `dev` as `12b9c8a69ae0335e076872f8b89cba3fe025f5aa` from reviewed head
  `3de2e1ad7b83eaee9c7a3ef10d1052c8175475fa` (same Git tree
  `4ff6ca291d8e9cc01f34dc5854d7aa26f4fc70ec`).
- Shared renderer rule: `DUNGEON_SURFACE_Y = 0.2` lifts the Synty floor and all
  props uniformly in builder and game; no specimen-specific repair remained.
- Public landed evidence lives at
  `ideas/dungeon-builder/evidence/crypt-prop-specimens/README.md`, with
  `final-builder.png`
  `29f965fa4d38440a62d0250763d8e773aaa2f3278247fd04f7e39af1c59efb59` and
  `final-game.png`
  `e8d3625906992bcbc504f85196203efe6fd01c44de79bdc9120e396bd6ed7720`.
- Real-path proof: builder route
  `?concept=dungeon-builder&authorFixture=crypt-props` and Home → Stanthony →
  Dungeon Builder → Load → Save → Open → Save & Play preserved the showcase YAML
  byte-for-byte at `a3927fdbf6b38fb886c55b72f58ad116dd8282917fb279a4f8c5f3c4a5e25542`
  and preserved each prop ref/facing/offset across reopen.
- Tests and checks: local integrated head `3de2e1a` passed `npm run ci-check` (format check, lint check, TypeScript type check, build, built theme CSS carries the combat-HUD block, production assets exclude Toolkit Contributor Sandbox code, tests) and the visible `npm run test:run` reported `Test Files 210 passed | 1 skipped (211)` and `Tests 3443 passed | 1 skipped (3444)`; the focused rerun still passed 10 files / 183 tests; PR #820 had green **Lint and Type Check**, **Deploy Preview**, **Test**, and **Security Audit** checks before merge.
- Kirk verdicts: provider exact-merge review “yeah they look great.” Current
  integrated verdict “it does.”
- Next lane: authored lighting remains open on journey
  [rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169),
  using [rpg-project#190](https://github.com/KirkDiggler/rpg-project/issues/190)
  as historical input to reconcile against the current stack.
