# Visual Anchor Metadata — delivery plan on ratified world offset

**Status (2026-08-09):** Kirk approved the course-corrected [design](./design.md) and
plan against ratified Dungeon YAML v0.4 at PR #203 HEAD `40a3938`; #206 Wave B was
deliberately opened and delivery is active. Exactly five owning issues exist:

- T [toolkit #898](https://github.com/KirkDiggler/rpg-toolkit/issues/898) / [PR
  #903](https://github.com/KirkDiggler/rpg-toolkit/pull/903): merged, published, verified;
- P [protos #219](https://github.com/KirkDiggler/rpg-api-protos/issues/219) / [PR
  #220](https://github.com/KirkDiggler/rpg-api-protos/pull/220): merged, published, verified;
- A [API #783](https://github.com/KirkDiggler/rpg-api/issues/783) / [PR
  #788](https://github.com/KirkDiggler/rpg-api/pull/788): PR merged to `dev` and verified;
  issue stays open through cross-repo closeout;
- G [assets #44](https://github.com/KirkDiggler/rpg-game-assets/issues/44) / [PR
  #45](https://github.com/KirkDiggler/rpg-game-assets/pull/45): merged and verified; and
- W [web #737](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/737) / [PR
  #742](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/742): open at exact head
  `a33293a4d67637fff0123bc8b84aac12bb2e4466`, public CI green, awaiting local exact-head
  locked-provider numeric/hash/performance and Kirk visual approval.

Do not create duplicate T/P/A/G/W, wrapper, `game-dev`, licensed-PR workflow, or bootstrap
issues. Web #743/#744 is closed/unmerged as superseded. Keep PR #205 open through W merge,
the established trusted post-merge asset build, and signed evidence closeout; do not merge
PR #205 at this checkpoint.

## Outcome

Deliver one production path where:

1. toolkit/protos/API preserve ratified optional `offset: [x,y,z]` through authored
   source, authoring projection, encounter snapshot, and authorized runtime projection;
2. a safe digest-bound catalog externally owns intrinsic model calibration and stable
   default selection;
3. Builder may compile local controls to world axes, displays/persists the exact triple,
   and previews the shared result;
4. actual Game consumes the projected triple and applies the generic world placement
   frame exactly once to every valid placement kind; only enrolled props receive
   additional catalog calibration;
5. room prop, canvas prop, room monster, canvas monster, and room boss prove generic
   transport while bookcase and torch prove two distinct intrinsic points without YAML
   wall/support
   semantics; and
6. offset/calibration remains cosmetic and mechanically inert.

## Ratified field audit used by every issue

| Field | Exact v0.4 status |
| --- | --- |
| `ref` | existing identity |
| `at` | existing owning cell; compiler projects absolute location |
| `facing` | existing v0.3 applicability only: non-monster floor/default-mount props; six values; v0.4 does not broaden it |
| `mount` | existing v0.3 decode-known/rejected behavior unchanged; never an anchor signal |
| `offset` | optional exactly three finite game-world values relative to canonical origin; world axes; facing does not rotate/reinterpret; omission has zero effect without authored presence |
| `anchor`, `surface`, `support`, `edge`, `orientation`, `adjustment`, `height`, `rotate_degrees`, `variant_id` | not v0.4 placement fields |

No issue may introduce a parallel anchor object, server catalog lookup, raw full transform,
or gameplay-footprint coupling.

## Original planning baselines and audited seams

The five existing owning issues recorded their own fresh branch bases. The planning audit
that shaped them used:

- `rpg-toolkit origin/main@1ecd20a9`: `encounter/dungeonspec/{spec,decode,
  validate,compile}.go` plus existing `DungeonParams`/obstacle specs,
  `SpawnInstruction`, `ObstacleData`/`MonsterInput`/`MonsterData`, encounter Data JSON,
  perception Memory, and KnownHex event carriers;
- `rpg-api-protos origin/main@6c4a9160`: authoring
  `dnd5e/api/authoring/v1alpha1/service.proto` has no placement projection at that
  audited base, while existing runtime v1alpha2 `Placement` already travels in
  reconnect `Space.hexes[].contents[]` and live `HexKnowledgeChanged` records; P must
  introduce the missing authoring element/list and add one shared offset message to
  both authoring and existing runtime carriers;
- `rpg-api origin/dev@2646724d`: `internal/orchestrators/authoring`, content registry
  and StartEncounter `Params`/`Spawns` consumption, encounter repository Data save/get,
  authoring handler, reconnect `ProjectFor`, and live KnownHex event translation;
- `rpg-game-assets origin/main@d22c53f`: private prop-role source data,
  `scripts/build_prop_manifest.py`, generated global `SYNTY_SCALE=0.75`, no asset
  release tag convention or CI workflow;
- `rpg-dnd5e-web origin/dev@4b85bb7`: array-first `propManifest` selection; three rug
  `renderScale=2`; `PropModel` outer transforms; independent character 0.75;
  Builder direct model resolver; state/render ingestion through
  `src/hooks/useEncounterState.ts`, `src/components/game/EncounterView.tsx` (existing
  position/facing reverse indexes), and
  `src/components/playtest/playtestMapHelpers.ts` before actual Game
  `EncounterMap→HexGrid→HexEntity`; local pull+rsync versus Docker latest-main+copy
  divergence; Learn experiment modules; and no current v0.4 offset projection.

Those heads are historical evidence, not current release provenance.

## Living work graph and exact issue mapping

#206 Wave B is deliberately open. The five owning issues below are the complete
implementation set; director coordination/evidence remains on #206/#204 and PR #203/#205.
Do not create a sixth leg or recut any completed provider.

| ID | Owning issue / PR | Repository/base | Current exact state | Depends on |
| --- | --- | --- | --- | --- |
| T | toolkit #898 / PR #903 | `rpg-toolkit/main` | Done; merge `7549d9aa1a718ef7453f4337b6bb3818d195e1e0`; `encounter/v0.53.0` verified | complete |
| P | protos #219 / PR #220 | `rpg-api-protos/main` | Done; merge `8aa4bda5c1f09b1eaa009d52e13c3e3da6037508`; generated Go `a6648cecf193894231bf55df1fc28b3eb42cf32e` verified | complete |
| A | API #783 / PR #788 | `rpg-api/dev` | PR merged/verified as `f5c847e81c1d4e94c06ebf5e406aec17eaa52b63`; issue remains In Review for closeout | T + P released |
| G | assets #44 / PR #45 | `rpg-game-assets/main` | Done; exact provider `29e26f7e4b92bdc35277bbaa9712f7cdce8ce85a` verified | approved plan; exact lock below |
| W | web #737 / PR #742 | `rpg-dnd5e-web/dev` | In Progress/open at `a33293a4d67637fff0123bc8b84aac12bb2e4466`; public CI green; local evidence gate remains | A merged/verified + exact G lock |

Project 19 currently records T/P/G as Done, A as In Review, and W as In Progress. Web
#743/#744 is closed/unmerged as superseded and is not an implementation leg. Keep the
required signature on every GitHub comment. Only W, post-merge build verification, and
cross-repo evidence/closure remain.

### Development order: outside-in (executed sequence)

1. W owner wrote contract-level TS fixtures/interfaces first on the existing W branch:
   projected optional world triple, semantic ref/at/facing, selector result, expected
   exact matrix. These are not reported as Game implementation.
2. P shapes the smallest optional transport that satisfies authoring and authorized
   runtime consumers, including omission versus explicit all-zero.
3. A maps P and T without calculations; T implements the source/compiler contract.
4. G implements the smallest real safe catalog satisfying consumer fixtures.
5. W replaces fixtures with actual API projection + exact provider revision and wires
   Builder and Game.

This is development sequencing, not merge sequencing.

### Provider-first merge/release order and remaining gates

1. **Complete:** T merged to `rpg-toolkit/main` and published verified
   `encounter/v0.53.0`.
2. **Complete:** P merged to `rpg-api-protos/main` and published the verified generated
   Go revision named above.
3. **Complete for W dependency:** A pinned released T/P, merged to `rpg-api/dev`, and
   passed post-merge build/test plus independent verification.
4. **Complete:** G merged to `rpg-game-assets/main`; exact provider SHA and digests are
   locked below.
5. **Current:** W #742 at exact head `a33293a4d67637fff0123bc8b84aac12bb2e4466`
   has green public CI. On that unchanged head, run local exact-head locked-provider
   numeric/hash/performance verification and Kirk's local Builder + actual API-backed Game
   visual approval. Record signed scalar/hash evidence only; never upload screenshots or
   licensed bytes.
6. **Then:** merge W to `rpg-dnd5e-web/dev`; the established trusted post-merge build
   workflow handles private asset fetch/build from the exact provider lock.
7. **Closeout:** director assembles evidence on #206/#204 and PR #203/#205; PR #205
   remains open until Kirk decides whether the tracking surfaces may merge/close.

No consumer merge precedes its provider. A web-only hand-built offset fixture cannot
satisfy the Game gate.

## T — toolkit issue/PR

### Files/seams

Delivered by toolkit #898 / PR #903 from the recorded fresh base. T owns the complete
carrier chain, not only YAML:

- `encounter/core.PlacementOffset [3]float64` is the cycle-safe shared value; pointers
  preserve omission versus explicit zero wherever presence matters;
- `encounter/dungeonspec/{spec,decode,validate}.go` strictly decode exactly three finite
  values on room/canvas placement and boss with canonical source-path errors;
- `compile.go` compiles the authored value **twice for distinct consumers**:
  1. `CompiledDungeon.Placements []CompiledPlacement` is the authoring-only sidecar with
     source path, ref, absolute at, applicable facing, effective blockers, and offset;
  2. the existing runtime inputs consumed by `StartEncounter`: room props through
     `CompiledDungeon.Params`/`PlacedObstacleSpec`; canvas props through
     `Params`/`AbsolutePlacedObstacleSpec`; room monsters through
     `CompiledDungeon.Spawns`/`SpawnInstruction.At`; the distinct canvas-monster path
     through `Spawns`/`SpawnInstruction.AbsoluteAt`; and room boss through
     `Spawns`/`SpawnInstruction.At`;
- `InitDungeon`/`SeedMonsters` copy runtime truth through `ObstacleData` or
  `MonsterInput -> MonsterData`; `Encounter.Data` JSON/restore is immutable encounter
  persistence; and
- authorized knowledge copies it through `perception.Placement ->
  events.KnownHexPlacement`, with viewer Memory freezing the last authorized value.

`CompiledDungeon.Placements` is never joined to toolkit-minted runtime ids. Movement
mutates canonical `Position` only, so the same offset follows the runtime entity;
current VISIBLE truth removes it from a vacated cell while REMEMBERED truth retains the
last authorized observation. Any placement-producing observation/appearance/pass-
through helper must copy canonical runtime metadata rather than manufacture a bare
identity that drops offset.

### Acceptance and required runtime tests

- Five cases—room prop, canvas prop, room monster, canvas monster, and room boss—each
  cover omission, explicit `[0,0,0]`, and signed nonzero; malformed cardinality/type/
  null/NaN/Inf reports the exact source field.
- Compile asserts both authoring `CompiledPlacement` facts (source/ref/absolute at/
  facing/effective blockers/offset) and the corresponding Params/Spawns runtime
  carrier, explicitly distinguishing room/boss `SpawnInstruction.At` from canvas-
  monster `SpawnInstruction.AbsoluteAt`. Local-to-absolute `at` never rotates/changes
  offset.
- A real `InitDungeon -> SeedMonsters -> ToData -> JSON -> LoadFromData` path proves
  exact optional presence/value in `ObstacleData`/`MonsterData` for all five cases,
  including canvas-monster `AbsoluteAt`; legacy JSON omission remains nil.
- `KnownHexes`/viewer Memory plus event JSON prove VISIBLE and frozen REMEMBERED
  placement offsets for all five cases, including explicit zero and the canvas monster
  seeded by `AbsoluteAt`. Room- and canvas-monster movement/resight/vacate plus every
  placement overlay/helper prove offset follows the entity, a visible vacated origin
  has no stale offset, and facing never rotates it.
- Baseline comparisons prove canonical position, effective blockers, collision,
  pathing, LoS, range, and targeting unchanged. There is no clamping, pivot, snapping,
  asset lookup, identity join, or gameplay interpretation.

Run normal Go format/lint/test for the entire affected encounter module, including
named dungeonspec, InitDungeon/SeedMonsters persistence, perception/Memory, and event
JSON suites. Record exact commands/version and full green output in PR evidence.

## P — protos issue/PR

### Contract

P introduces the missing authoring placement projection; it is not present at the
audited base:

- shared `dnd5e.api.v1alpha1.PlacementOffset` is exactly three doubles `x=1`, `y=2`,
  `z=3` in canonical game-world axes;
- new authoring `FloorPlanPlacement` carries `ref=1`, absolute `at=2`, optional
  applicable `facing=3`, effective `blocks_movement=4`, effective `blocks_los=5`,
  `source_path=6`, and optional shared `offset=7`;
- new repeated `FloorPlan.placements=11` contains all compiled room/canvas props,
  room/canvas monsters, and room boss in provider-defined deterministic order; and
- the **existing distinct** runtime v1alpha2 `Placement` retains its runtime
  `entity_id=1`/optional `facing=2` and gains optional shared `offset=3`. That runtime
  placement already travels in reconnect `Space.hexes[].contents[]` and live
  `HexKnowledgeChanged.hexes[].contents[]`.

Nested message presence preserves omission versus explicit `[0,0,0]`. Authoring
`FloorPlanPlacement` is a projection value, not a runtime identity. Do not add offset
to `Entity`, introduce an identity/join seam, or add catalog ids, intrinsic points,
matrices, quaternions, scale, semantic wall/support, replacement, or generic transform.

### Acceptance and required runtime tests

- Descriptor/generated Go+TS assertions prove exact shared x/y/z doubles/tags, exact
  authoring element/list fields/tags, and optional runtime `Placement.offset=3` with
  identical game-world-unit/axis documentation.
- Binary and proto-JSON compatibility prove legacy runtime field-1-only bytes decode
  offset absent and legacy FloorPlan bytes decode placements absent; explicit zero and
  signed nonzero preserve nested presence/value.
- Nested Go+TS round trips prove authoring `FloorPlan.placements[]`, reconnect
  `Space.hexes[].contents[]`, and live `HexKnowledgeChanged.hexes[].contents[]` each
  preserve absent/zero/signed values.
- Schema/descriptor assertions prove no offset field on `Entity` and no new identity,
  replacement, or join surface.
- Generated artifacts, lint/tests, and breaking-change checks pass.

## A — API issue/PR

### Flow and ownership boundary

A consumes and projects toolkit truth; it does not own placement identity or runtime
offset association:

1. `PutDungeon(validate_only=true)` and persisted authoring reload map toolkit
   `CompiledDungeon.Placements` to authoring FloorPlan fields: source path/ref/absolute
   at/applicable facing/effective blockers/exact optional offset.
2. `StartEncounter` passes toolkit `CompiledDungeon.Params` to `InitDungeon` and
   `Spawns` to `SeedMonsters` without reinterpretation. This preserves five distinct
   inputs: room/canvas prop Params, room-monster and boss `SpawnInstruction.At`, and
   canvas-monster `SpawnInstruction.AbsoluteAt`. Toolkit mints runtime ids and returns
   `Encounter.Data`; API persists/reloads that Data as the immutable snapshot.
3. Reconnect `ProjectFor` maps toolkit perception placements to proto
   `Space.hexes[].contents[]`; live KnownHex event translation maps toolkit
   `events.KnownHexPlacement` to `HexKnowledgeChanged.hexes[].contents[]`.
4. Movement/re-placement changes toolkit canonical position only; API copies the
   authorized offset it receives. Existing encounters never recompute from later
   authored source edits.

The authoring `CompiledPlacement` sidecar has source metadata/blockers but no runtime
identity join. A must not match `(ref, at, order)`, create an offset/id side map, infer
identity from authoring projection, or rebuild a placement from current position while
dropping toolkit metadata. No API/proto identity field is added.

### Acceptance and required runtime tests

- `PutDungeon(validate_only)` plus persisted save/reload project authoring source/ref/
  absolute at/facing/effective blockers and nil-versus-explicit-zero/signed offset
  exactly for room prop, canvas prop, room monster, canvas monster, and room boss.
- Real content registry -> `StartEncounter` -> encounter repository save/get proves all
  five offsets originate from T Params/Spawns and persist in toolkit Data, explicitly
  exercising canvas-monster `AbsoluteAt`, with no sidecar join or API-owned map.
- Reconnect `ProjectFor` and live KnownHex event translation preserve exact optional
  offset for all five cases through both toolkit perception/event conversion points.
  Any fallback/current-position placement rebuild copies canonical runtime metadata
  rather than matching ref/at/order.
- Fog matrix includes moved canvas and room monsters: unauthorized contents/offset are
  absent, VISIBLE wins over REMEMBERED, and movement/re-placement plus resight/vacate
  is total and non-stale.
- Ref edits retain/change/remove only through complete-document behavior; an existing
  snapshot never recomputes, and legacy authored content/snapshots remain readable.
- Baseline comparison proves canonical position, blockers, collision/pathing, LoS,
  range/targetability/fog unchanged; no catalog lookup, clamp, rotation, asset-specific
  behavior, identity heuristic, or side map exists.

### Rollback boundary

Once source/snapshot persistence exists, rollback cannot delete/strip the field. A
server rollback must retain decode/storage/proto passthrough (even if visual consumers
are temporarily disabled), preserve existing snapshots, and keep exact authored
presence. Database/source rollback procedures must prove no lossy rewrite.

## G — asset-provider issue/PR

### Exact v1 identities

Registrar grammar:

```text
^synty:props:[a-z0-9]+(?:-[a-z0-9]+)*$
```

Exact immutable ids:

```text
synty:props:sm-prop-bookcase-small-01
synty:props:sm-prop-torch-ornate-01
```

Family declarations bind current `dnd5e:props:bookcase` and
`dnd5e:props:torch-ornate` to those exact defaults. The producer PR must record exact
current semantic ref spellings from web/API at implementation time.

A global registrar rejects collisions among all primary/companion ids, conflicting
path reuse, default outside family, invalid grammar, missing promoted output, and
silent id changes. A path move leaves id stable; an id change requires an explicit
migration record and consumer review.

### Private reviewed source

Add reviewed source declarations (recommended
`library/visual-anchor-calibrations.json`) with:

- stable id/family/default;
- promoted relative GLB path and accepted SHA-256;
- one `totalScale` converting GLB-local units to canonical game-world units;
- required `sourceForwardYawRad` plus evidence reference;
- tagged point (`floor-contact` for bookcase, `wall-attachment` for torch), expressed
  after scale/yaw in canonical game-world units; and
- **required** authoring UX hints for local right/up/forward controls: finite min/max,
  finite positive step, all in canonical game-world units, and evidence reference.

Do not copy raw source asset files or measurement dumps. Raw export defects return to
re-export; generator metadata cannot compensate malformed Root/axis/scale. Before any
Learn number is promoted, G records and tests the conversion from experiment-local
Learn scene units to canonical game-world units (including an evidenced factor of one
if identity is correct). Generator dimensional tests reject points/hints whose unit
label or conversion provenance is absent.

### Deterministic safe generator

Add a separate safe projection generator, not renderer fields inside gameplay
prop-role semantics. Recommended:

- input: reviewed declarations + existing generated prop inventory;
- output: `harness/catalogs/synty-web-assets.json` with fixed
  `lengthUnit: game-world` and schema-major enforcement for unit/axis/scale semantics;
- stable ordering/number encoding/newlines;
- no timestamp/current branch/self commit field;
- allowlisted safe fields only; and
- check mode reruns generator and byte-compares tracked output.

Hard failures:

- unsupported schema/unit, missing conversion provenance, invalid/nonfinite
  scale/yaw/point/hint, or nonzero Y on a v1 `floor-contact` point;
- missing/extraneous/duplicate ids or family defaults;
- path traversal/non-GLB/missing file;
- digest mismatch;
- global collision/default mismatch;
- forbidden private/source/geometry/material/texture keys/paths; and
- stale tracked output.

### Required initial UX hint evidence

The Learn numbers are candidates, not automatic production truth. For each enrolled
entry, Kirk inspects the local Builder and actual API-backed Game on his trusted machine
and the verifier records a numeric matrix/control table across at least:

- three distinct valid existing facings;
- positive/negative local right and forward controls;
- positive vertical where visually relevant; and
- two representative camera distances including Discord viewport.

Kirk confirms the chosen range/step provides useful adjustment without presenting it as
YAML validity. Public evidence is signed scalar/hash results only—no screenshot upload,
GLB, or private-source artifact. If evidence cannot justify hints, stop for explicit
#205 design amendment; do not mark them optional.

### Atomic provider verification

Stage into a fresh temporary directory:

1. copy the **complete** `harness/models/synty/` legacy tree from the candidate tree,
   not only enrolled GLBs;
2. generate/copy safe catalog;
3. verify every catalog digest and complete expected legacy inventory;
4. record only catalog digest + generator/tool identity available from content;
5. run catalog/schema/license checks; then
6. atomically rename/swap the whole catalog+asset stage.

G never writes its own future post-merge commit SHA into a tracked wrapper/catalog; that
SHA is unknowable until merge and belongs in W's post-merge provider lock.

Failure keeps prior entire tree/catalog; no partial rsync is visible.

### Provider provenance: decided and released

Kirk chose the exact merged G commit with no release-tag ceremony. W's provider lock
contains exact G SHA `29e26f7e4b92bdc35277bbaa9712f7cdce8ce85a`, safe catalog SHA-256
`0b816d6f08584e66c90556f9ad4d040c71086c1dbf698bf7d5030fb05c490669`, generator/tool
identity and version, inventory SHA-256
`e1e1915d330d0431248cdec57e5a591454c38820c832e7eff1a5e0d6a7c54bd1`, 2,143-file tree
SHA-256 `3bcb0584e267b4291f86370093e56091f3c62ccfb53e86e062c66ec457c4c144`, and required
repository/schema identity. It contains neither a web SHA nor mutable branch name.
Authenticated local/Docker sync fetches that exact G commit and verifies `HEAD == locked
SHA`; the established trusted post-merge build does the same. Actual web HEAD is signed
scalar/hash evidence and a deployment/image label alongside the verified lock hash.

Do not create a provider tag/ruleset alternative or another provenance issue.

### G tests/evidence

- normal Python/unit suite using repository environment;
- generator determinism/check mode twice;
- collision/adversarial/allowlist tests;
- exact GLB hashes and immutable-source proof;
- two per-entry calibration matrix fixtures;
- content-digest change tests plus renewed evidence gate for default/scale/yaw/point
  edits and schema-major test for unit/axis/order semantics;
- full-tree atomic failure injection before swap; and
- license scan confirming no raw GLB/source content enters public repo.

## W — web consumer issue/PR

The existing W branch for web #737 / PR #742 includes platform offset consumption and
visual catalog integration so one actual matrix path can be proven; it does not replace
T/P/A. Do not cut another W branch or issue.

### 1. Contract fixtures before providers

Add fixture interfaces/tests for:

```text
selectVisualVariant(catalog, semanticRef, explicitVariantId?)
resolveVisualPlacement(entryOrNone, canonicalAtOrigin, facingYaw, worldOffset)
  -> matrix + diagnostics
```

All translations are canonical game-world units. Use exact equations:

```text
P_generic = T(p) · T(o_world) · R_y(facing)
C_enrolled = T(-modelPoint) · R_y(sourceForwardYawRad) · S(totalScale)
M_generic = P_generic
M_enrolled = P_generic · C_enrolled
```

`totalScale` converts GLB-local units to canonical game-world units; post-scale point,
hints, `p`, `o`, and resolver translations share that unit.

Fixture assertions:

- stable default independent of ordering;
- exact ids above;
- omitted/explicit `[0,0,0]`/nonzero XYZ;
- generic `M_generic` exactly once for room prop, canvas prop, room monster, canvas
  monster, and room boss fixtures;
- catalog `C_enrolled` only for enrolled prop fixtures;
- all six valid existing facings;
- offset vector unchanged when facing changes;
- point/yaw/scale composition order;
- no-point identity fallback;
- replacement reloads all intrinsic fields and offset retains/changes/removes only by
  explicit client action; and
- invalid selection/catalog/provenance fails deterministically.

### 2. Exact provider ingestion and complete legacy tree

Replace both divergent paths with one script/module used by local sync and Docker:

1. read the tracked W provider lock containing exact merged G SHA, catalog digest, and
   generator/tool identity/version; the lock contains no web SHA or tag indirection;
2. authenticated fetch/checkout exact revision in a clean temp directory;
3. assert exact provider HEAD before reading files;
4. stage the **complete** legacy `harness/models/synty/` tree plus safe catalog;
5. verify byte-identical checked catalog, inventory, every enrolled digest, schema, and
   generator identity; and
6. atomically replace public tree+catalog after every check succeeds.

The running local check, not a tracked file, emits signed scalar/hash evidence containing
actual web HEAD, provider-lock hash, verified provider SHA/catalog digest, generator/tool
identity and version, numeric matrix/performance results, and pass/fail. Deployment/image
labels record the same provenance split. No tracked G/W wrapper contains its own future/
current web build SHA.

### 3. Public PR CI and trusted-machine exact-head verification

**Automatic public PR CI:** on every W PR, run schema/id/unit checks, pure selector/
resolver fixtures, public-safe catalog fixtures, lint/typecheck/unit/build where licensed
files are not required, and scan the diff/artifact manifest. Forks and Dependabot receive
the same public path. This CI receives no private-provider credential and fetches no
licensed bytes.

**Local exact-head gate:** after public CI and review, Kirk's trusted machine checks out
the current W PR head and asserts `git rev-parse HEAD` equals the reviewed SHA. Using the
tracked provider lock, the existing authenticated local path fetches the exact private G
revision, asserts provider HEAD, catalog/tool identities, inventory, and every enrolled
GLB digest, then atomically stages the complete legacy tree + catalog in both local-script
and Docker-build modes. On that exact W/provider pair it runs full build/test/lint/
typecheck, numeric matrix checks, drift/license scans, and the named performance methods.
Any stale W head, provider-lock/digest mismatch, partial stage, or numeric/performance
failure blocks merge and must be rerun on the new exact head.

Kirk then locally inspects Builder and the actual API-backed Game using that same exact
head/provider stage. His approval is recorded on #205 as signed scalar/hash evidence:
web head, provider-lock hash, provider SHA/catalog/tool/GLB digests, numeric/performance
summary digest, scenario-result scalars, and pass/fail. Do not upload screenshots,
licensed files, the private checkout, or the staged public asset tree.

This is deliberately **not** a protected licensed-assets PR workflow. It requires no
GitHub Environment secret/approval, no hostile-PR or no-egress sandbox, no workflow
bootstrap, no `pull_request_target` secret choreography, and no screenshot/artifact
upload. Once the public PR checks, local exact-head verification, and Kirk approval pass,
merge W to `dev`; the established trusted post-merge build workflow performs private
asset fetch/build from the exact provider lock. Record the resulting `dev` merge SHA and
build provenance as signed scalar/hash evidence only.

### 4. Catalog projection and scale partition

Commit only safe generated JSON and the provider lock (G SHA + catalog digest + tool
identity, never web SHA). Preserve the complete legacy asset tree in built/public
staging without committing licensed GLBs; actual web HEAD remains signed scalar/hash
verification and deployment/image-label evidence.

Generate/consume an enrolled lookup while preserving current public prop roles. Enrolled
bookcase/torch use catalog `totalScale` once. Compile-time-distinct enrolled matrix
component owns returned matrix over shared loading internals; legacy component retains
existing global/renderScale behavior. A discriminator prevents one asset entering both.

Keep the three rug `renderScale=2`, non-enrolled siblings, wall/character paths, and
#624 behavior unchanged. Add assertions:

- enrolled: no outer `SYNTY_SCALE`/`renderScale`;
- legacy: current effective scale unchanged;
- companion meshes inherit the identical sole matrix; and
- character resolver remains independent.

### 5. Shared selector/resolver

One pure selector and one pure matrix resolver are imported by Builder and Game.
No model-specific component branch, runtime GLB measurement, React state, camera,
wall geometry, gameplay state, or network access enters either pure function.

The resolver returns generic `P=T(p)·T(o)·R_y(φ)` for every valid placement. Only an
enrolled prop appends catalog `C`; its full `M=P·C` is applied once to the enrolled
group with primitive/companions under it. Non-enrolled prop/monster/boss groups apply
`P` once and retain only their partitioned model-internal calibration below it. Remove
outer position/yaw/scale duplicates for enrolled models; no path applies `o` twice.

### 6. Projection ingestion before render

Name and wire the actual pre-render seams, not only `HexEntity`:

- `src/hooks/useEncounterState.ts`: hydrate optional offset presence/value from the
  reconnect snapshot and every live encounter update that carries placements;
- `src/components/game/EncounterView.tsx`: build `offsetByEntityId` beside the existing
  position/facing reverse indexes, with current VISIBLE truth winning over stale
  REMEMBERED truth; and
- `src/components/playtest/playtestMapHelpers.ts`: extend the real `HexEntityData`
  mapping so offset survives the production helper boundary into the renderer.

Use one presence-aware offset extractor/merge policy across snapshot and events. Tests
must distinguish omission from explicit `[0,0,0]` and nonzero; prove
VISIBLE-over-REMEMBERED; reconnect snapshot hydration; live update, resight and vacate;
and movement/re-placement. On movement/re-placement the current canonical origin `p`
changes while the unchanged world-axis `o` follows the entity; the vacated cell cannot
leave a stale offset/index entry.

Generic offset applies exactly once to every valid placement kind. Required end-to-end
fixtures include a room prop, canvas prop, room monster, canvas monster, and room boss.
Only the enrolled bookcase/torch prop cases additionally select/apply catalog
calibration.

### 7. Builder

Builder:

- decodes/projected optional offset presence;
- displays exact world X/Y/Z triple and omission;
- may show local right/up/forward controls using required entry hints;
- converts local delta once with existing facing yaw to world `offset`;
- previews the exact candidate through `PutDungeon(validate_only=true)`;
- rejects unsupported server fields loudly; never strips offset to pass;
- saves complete document with exact finite triple; and
- on ref edit presents explicit retain/change/remove choice—no implicit reset or
  preservation policy.

A transient selected wall/context may aid controls but is not serialized; UI states
plainly that changing wall geometry does not reattach persisted world offset.

### 8. Actual Game

Wire the production path from the named state/helper seams through
`EncounterMap→HexGrid→HexEntity` (or then-current equivalent) to the same matrix
resolver. Use current canonical origin, applicable facing, and exact projected world
offset. Apply generic `P` exactly once to every valid prop/monster/boss; catalog
selection/calibration runs only for enrolled props. Do not read source YAML, Builder
state, wall geometry, or Learn data.

Actual joined encounters from A-backed snapshots are mandatory for representative
room prop, canvas prop, room monster, canvas monster, and room boss, including movement/
re-placement on both monster input paths. Storybook/Concepts Lab/local fake projection
is not Game evidence.

### 9. Ref replacement

Test complete-document candidates:

1. bookcase at/facing/offset;
2. torch retaining exact offset;
3. torch changing offset;
4. torch omitting offset; and
5. invalid target ref/default leaves prior document/preview intact.

New ref reloads id/path/digest/scale/yaw/point/hints/companions. No promise of persistent
wall attachment/support exists.

### 10. Fallback and rollback

Required deterministic outcomes match design table. Specifically, every fallback
applies canonical at + exact world offset; none silently drops offset or selects a
sibling variant.

Split deployment so enrollment can be disabled without disabling ratified offset:

- core offset parser/projection consumer and outer world translation remains on both
  enrolled and legacy/generic fallback paths;
- catalog enrollment feature/config may revert bookcase/torch to legacy selection;
- previous complete atomic asset+catalog stage remains available;
- persisted source and encounter snapshots remain untouched; and
- source/API/proto rollback cannot strip already-authored optional presence.

A full revert that causes Game to ignore persisted offset is prohibited after offset
is accepted in production.

### 11. Learn cleanup

- Anchor Lab may remain isolated diagnostic UI but calibrated results must call the
  production selector/resolver.
- Remove Prop Composition one-off centering/variant-selection math after replacement
  and parity proof; keep historical evidence documents/screens.
- Add dependency/build-graph test preventing Learn measurement tables from production
  Builder/Game bundles.

### 12. W tests

- pure selector/resolver unit + golden matrix tests;
- Builder local→world conversion and exact world display/persistence;
- projected omission versus explicit `[0,0,0]` versus nonzero through
  `useEncounterState.ts`, `EncounterView.tsx`, and `playtestMapHelpers.ts`;
- VISIBLE-over-REMEMBERED, reconnect snapshot hydration, live update/resight/vacate,
  and movement/re-placement without stale reverse-index state;
- Builder + actual Game integration using A service/snapshot for room prop, canvas
  prop, room monster, canvas monster, and room boss;
- generic `P` exactly once on every fixture, catalog `C` only on enrolled props;
- six facing values (not claimed as six wall edges);
- positive/negative X/Y/Z, facing nonrotation of offset;
- replacement retain/change/remove;
- scale double-application and companion identity guards;
- invalid catalog/provenance/digest/no-point/load fallback;
- complete-tree atomic stage failure injection;
- full lint/typecheck/unit/build/Playwright from project environment.

## Cross-repo acceptance evidence

Use one checked evidence index on #205 for T/P/A/G/W. Licensed-asset verification is
recorded as signed scalar/hash evidence only: issue/PR, actual exact web HEAD/base HEAD,
tracked provider-lock hash, exact provider SHA, catalog/tool/GLB digests,
command/result-summary digests, measured scalars, and pass/fail. The tracked provider
lock itself never contains web HEAD; no screenshot or licensed-byte evidence is uploaded.

### Platform evidence

- source → distinct authoring `CompiledPlacement` and runtime Params/Spawns →
  ObstacleData/MonsterData JSON snapshot → perception/event placement → authoring,
  reconnect, and live proto projection for omission/explicit-zero/signed-nonzero;
- authoring sidecar facts include effective blockers while runtime identity comes only
  from toolkit InitDungeon/SeedMonsters; static/runtime evidence proves no join/side map;
- the five-case matrix distinguishes room prop, canvas prop, room monster `At`, canvas
  monster `AbsoluteAt`, and room boss `At` from compile through Data JSON restore,
  perception/Memory/event, API repository, reconnect, live, fog, and movement;
- authoring/runtime exact component equality and old document/snapshot compatibility;
- source-path errors for malformed/nonfinite;
- fog authorization and mechanical invariants;
- ref edit retain/change/remove; and
- authorized VISIBLE/REMEMBERED/resight/vacate plus movement/re-placement snapshot/event
  truth sufficient for the web ingestion cases.

### Paired local visual approval

On Kirk's trusted machine, use identical exact-head provider/projection facts in Builder
and the actual API-backed Game. Kirk inspects the representative Discord viewport and a
close diagnostic view; record only signed scenario-result scalars and hashes proving:

- bookcase and torch matrices numerically equal within `1e-6` per element;
- omission, explicit `[0,0,0]`, and ±X/±Y/±Z representatives;
- room prop, canvas prop, room monster, canvas monster, and room boss with generic
  offset exactly once in both Builder and actual Game; only enrolled props show
  additional catalog calibration;
- VISIBLE-over-REMEMBERED, reconnect, live/resight/vacate, and movement/re-placement
  states with no stale/double offset;
- six existing facing values where baseline accepts facing;
- facing changes model orientation while offset world vector stays unchanged;
- two pivot-distinct intrinsic points;
- replacement retain/change/remove; and
- no-point and model-load fallback at `p+o`.

No screenshot is uploaded or required. Interior-wall/generated-envelope scenes may be
inspected as authoring examples only; do not label them wall registration/support parity
and do not require six wall edges.

### Gameplay invariants

Compare before/after authoritative data for owning cell, blockers, collision/pathing,
range, targetability, LoS, fog/visibility, and interaction id. They must be exactly
unchanged across model/offset changes.

### Licensing

Record signed file-name/inventory/license-scan summary hashes and scalar results only.
Never upload raw GLBs/private source, screenshots, or authenticated URLs to public
project evidence.

## Quantitative performance gates

Run named methods locally on Kirk's trusted machine against W exact PR HEAD/provider
lock. Record machine/OS, repository Node/browser versions, base/head/provider SHAs, and
case-table digest as signed scalar/hash evidence.

### Pure resolver statistic

- Environment: production resolver build under the repository Node version on Kirk's
  recorded trusted machine.
- Case table: six facings × three fixed nonzero world offsets, all translations in
  canonical game-world units.
- Sampling unit: one timed batch of 1,000 resolves, elapsed by `performance.now()` and
  divided by 1,000; do not time/log each call.
- Per run: fresh Node process, 10 untimed warmup batches, then 100 timed batch samples.
- p95: sort the 100 per-resolve samples; nearest-rank index
  `ceil(0.95*100)-1` zero-based.
- Aggregation: three fresh processes in the same local verification run; median of the
  three per-run p95s.
- Budget: median p95 ≤ `0.05 ms/resolve`.

### Actual-route base/head comparison

Run exact `origin/dev` base then exact W head sequentially in each of three paired
cycles on the same trusted machine, Chromium version, fixed route/camera, and exact
provider lock. Wait for the same bookcase/torch to load plus 120 warmup frames, then use
current `DevPerfProbe`'s 8,000 ms window. Aggregate frame time as the median of the three
reported per-run `frameTimeMs.p95` values. Head may exceed base by at most
`max(1.0 ms, 5% of base median p95)`.

For **each paired run**, current measurable `rendererInfo` counters for the same loaded
models must have exact head-minus-base delta `+0` for:

- `calls.max`;
- `triangles.max`;
- `geometries`;
- `textures`; and
- `programs`.

Do not claim GPU-byte measurement; `DevPerfProbe` exposes counts, not bytes.

Additional gates:

| Gate | Method | Budget |
| --- | --- | --- |
| Idle recompute | selector/resolver counters after stable scene over 300 rAF ticks | exactly 0 new resolves |
| Safe data | catalog + tracked provider-lock byte count | ≤8 KiB uncompressed |
| Network/cache (plan evidence only) | repeat route after warm cache | no rerender-triggered catalog/GLB request; one catalog fetch per build/page load as designed |

Record signed result-summary hashes and measured scalars only; do not upload raw traces,
screenshots, GLBs, or private-checkout content. Any breach blocks or requires a specific
Kirk-approved measured exception on #205.

## Drift and atomicity gates

The following all hard-fail G/W release checks:

- generated catalog differs from generator output;
- catalog expected digest/revision differs between public artifact/runtime import;
- provider exact HEAD, catalog digest, or tool identity differs from
  the tracked W provider lock;
- enrolled GLB SHA-256 differs;
- complete legacy tree inventory is missing/partially from another provider revision;
- catalog schema/id/number/hint/companion invalid;
- asset is both enrolled and legacy-scaled;
- local and Docker stages differ byte-for-byte; or
- local verification/approval evidence names a W head other than the current PR HEAD.

No warning-only drift path is acceptable for release.

## Rollout and rollback rehearsal

### Rollout

1. Merge/release T and P.
2. Merge/deploy A with passthrough disabled from mechanics by construction.
3. Validate authored source/persistence/runtime projection in a nonproduction dungeon.
4. Merge G, capture exact provider SHA, and cut the tracked W provider lock with G
   SHA + catalog digest + generator/tool identity (no web SHA).
5. On Kirk's trusted machine at W exact head, atomically stage the full legacy tree +
   catalog from the lock and record actual web HEAD as signed scalar/hash evidence.
6. Enable generic offset in Builder/actual API-backed Game for every valid placement
   kind, then enrolled bookcase/torch catalog calibration.
7. Run the complete local numeric/hash/performance matrix and Kirk visual inspection;
   record signed scalar/hash approval only.
8. Merge W to `dev` after public CI, independent gates, local exact-head verification,
   and Kirk approval.
9. The established trusted post-merge build workflow fetches/builds private assets from
   the provider lock on exact `dev` merge SHA; record scalar/hash provenance and update
   #203/#205 tracking, with no automatic tracking-PR merge.

### Rollback rehearsal

Inject catalog/hash failure before swap and prove prior whole stage remains. Then disable
enrollment and prove:

- legacy/generic rendering still applies exact world offset;
- persisted YAML/reloaded source/snapshot/API projection retains optional value;
- no asset sibling substitution occurs;
- current encounter mechanics/identity unchanged; and
- reenabling exact prior provider stage restores same matrices.

Document component owners and commands. Never roll back by stripping `offset` from
source/protos/API once stored.

## Definition of done

All must be true before the director can recommend delivery; only Kirk may approve or
merge:

1. Course-corrected design and plan are approved; the 2026-08-09 trust-model correction
   changes operational evidence mechanics only.
2. Exactly five owning implementation legs remain mapped to toolkit #898/#903, protos
   #219/#220, API #783/#788, assets #44/#45, and web #737/#742. Do not create duplicates,
   a wrapper, `game-dev` issue, or replacement for superseded web #743/#744.
3. v0.4 exact optional offset works end-to-end from YAML through actual Game: P has
   introduced authoring `FloorPlanPlacement`/`FloorPlan.placements[]` separately from
   existing runtime v1alpha2 `Placement`; T/A pass the five-case matrix including canvas
   monster `AbsoluteAt`; omission/explicit-zero/signed values survive snapshot/live/fog/
   movement state and remain mechanically inert.
4. Safe catalog generator/registrar ships exact two ids/defaults, digest-bound scale/yaw/
   points, and required evidence-backed UX hints, all promoted into canonical game-world
   units through a tested conversion.
5. Tracked W provider lock pins exact G commit
   `29e26f7e4b92bdc35277bbaa9712f7cdce8ce85a`, catalog/inventory/tree/GLB digests, and
   tool identity with no tag indirection or web SHA; actual web HEAD appears only in
   signed scalar/hash evidence and deployment/image labels.
6. Complete legacy tree + catalog stages atomically and identically for local/Docker;
   public PR CI passes, Kirk's trusted-machine exact-head numeric/hash/performance and
   Builder + actual API-backed Game visual gates pass, and only signed scalar/hash
   evidence is recorded. No protected PR licensed-assets workflow, Environment secret,
   no-egress sandbox, workflow bootstrap, screenshot upload, or committed licensed bytes
   is required.
7. Named web state/helper seams prove generic `P` exactly once for room prop, canvas
   prop, room monster, canvas monster, and room boss; only enrolled props receive
   catalog `C`; no double scale.
8. Achievable paired evidence and defined measurable performance statistics pass without
   semantic wall/support or GPU-byte claims.
9. Replacement retain/change/remove and all deterministic fallbacks/rollback pass.
10. Quantitative performance, drift, licensing, and experiment cleanup gates pass.
11. #203/#206 and #204/#205 evidence indexes are current and signed; status remains open
    until Kirk's merge/closure decision.

rpg-game-assets #43 is **not** a DoD dependency. Its owner must re-export/verify the
downed character without a web offset, but unrelated scheduling cannot strand this
two-prop delivery.

## Explicitly out of scope

- semantic wall edge/support/anchor persistence;
- wall-GLB measurement or automatic reattachment;
- authored variant id;
- catalog fields in YAML/protos/API;
- raw matrix/quaternion/scale/rotation persistence;
- runtime GLB measurement;
- model-specific rendering branches;
- collision/footprint/pathing/LoS change;
- broad wall/rug/character migration or #624 completion;
- raw asset edits in public repos; and
- #43 implementation/closure.

## Decision ledger

Kirk approved the revised design/plan boundary on 2026-08-09 and explicitly made these
calls:

1. **Provider provenance:** W tracks the exact merged G commit SHA + catalog digest +
   generator/tool identity with no mutable branch or web SHA; no release-tag ceremony.
   Actual web HEAD stays in signed scalar/hash evidence and deployment/image labels.
2. **Immutable ids:** use the exact v1 literals/grammar above and require an explicit
   migration record for any later id change.
3. **Required hints:** evidence-backed initial UX hints are provider-release requirements
   and Builder guidance, never YAML/server validity.
4. **Wall limit:** accept offset-only v0.4 and defer semantic wall/support persistence.
5. **Performance:** use the named quantitative methods and budgets above.
6. **#43:** keep the downed-character asset fix separate/nonblocking, with no web offset.
7. **Licensed-assets delivery course correction (2026-08-09):** do not build protected
   PR licensed-assets CI/bootstrap. Run public PR CI; on Kirk's trusted machine stage the
   exact locked provider at the exact reviewed W head and run numeric/hash/performance
   verification; Kirk locally approves Builder + actual API-backed Game visuals; record
   signed scalar/hash evidence only; merge W to `dev`; let the established trusted
   post-merge build workflow fetch/build private assets. This requires no GitHub
   Environment secret/approval, no hostile-PR exact-head secret workflow, no no-egress PR
   sandbox, no workflow bootstrap, and no screenshot upload.

The license boundary is unchanged: raw/private source remains private; promoted GLBs may
ship inside game builds; no licensed bytes are committed to the public web repository.
