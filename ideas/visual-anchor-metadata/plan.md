# Visual Anchor Metadata — delivery plan on ratified world offset

**Status:** Revised planning proposal for renewed Kirk review. It implements the
course-corrected [design](./design.md) against ratified Dungeon YAML v0.4 at PR #203
HEAD `40a3938`; it does not claim plan approval or implementation readiness.

This document is executable only after Kirk renews approval. Until then:

- do not create implementation issues/branches/PRs/tags/rulesets;
- keep #204/#205 and #203/#206 open;
- do not merge either design surface; and
- do not report the superseded semantic-anchor proposal as current.

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
5. representative room/canvas props and room monster/boss prove generic transport while
   bookcase and torch prove two distinct intrinsic points without YAML wall/support
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

## Current baselines and seams audited before issue creation

Capture fresh exact heads again when issues are cut. Planning audit used:

- `rpg-toolkit origin/main@1ecd20a9`: `encounter/dungeonspec/{spec,decode,
  validate,compile}.go` and fixture/unit suites;
- `rpg-api-protos origin/main@6c4a9160`: authoring
  `dnd5e/api/authoring/v1alpha1/service.proto`, generated Go/TS, encounter v1alpha2
  types/events; exact authorized placement owner must be confirmed in the platform
  implementation issue, not guessed here;
- `rpg-api origin/dev@2646724d`: `internal/orchestrators/authoring`, dungeon registry
  and encounter creation/snapshot, authoring handler, v2 authorized hex projection;
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

Those heads are evidence, not branch bases for future work. Every issue starts from
fresh owning base and records exact HEAD.

## Work graph after renewed approval

Create exactly **five** implementation issues—T/P/A/G/W—and exactly one branch/PR in
each owning repository. There is no C/wrapper issue and no `game-dev` issue. Director
coordination/evidence remains comments on existing #206/#204 and PR #203/#205.

Issue creation requires both renewed #205 approval and the deliberate #206 decision to
open Wave B; #206 currently says the wave is not cut. Approval of this plan does not
silently bypass that sequencing gate.

| ID | Repository/base | Owning lane | Deliverable | Depends on |
| --- | --- | --- | --- | --- |
| T | `rpg-toolkit/main` | Platform | strict YAML offset decode/validate/presence/compile | #206 Wave B opened; approved #203/#205 plans |
| P | `rpg-api-protos/main` | Platform | optional authoring + authorized runtime transport, omission preserved | #206 Wave B opened; web/API consumer fixtures |
| A | `rpg-api/dev` | Platform | source persistence, authoring projection, snapshot/runtime projection, no interpretation | T + P released |
| G | `rpg-game-assets/main` | Assets | declarations, safe generator, two enrolled assets/hints, exact provider handoff | renewed #205 approval |
| W | `rpg-dnd5e-web/dev` | Assets with UI/Platform review | Builder authoring + actual Game generic offset, shared catalog selector/resolver, atomic sync | A available + G merged provenance |

At cut time add every issue to **KirkDiggler Project 19** with exact fields:

- T/P/A: `Team=Platform`, `Feature=The Dungeon`, `Kind=Build`, `Status=Todo`;
- G/W: `Team=Assets`, `Feature=The Dungeon`, `Kind=Build`, `Status=Todo`.

Use then-current owning Platform/Assets board policy and repo labels in addition to
those fields; do not invent a separate Assets or `game-dev` board. Move each item to
`In Progress` only when its work session begins and `In Review` only when its ready PR
opens. Each issue/PR stays open through its independent gate and all GitHub comments
carry the required role signature.

### Development order: outside-in

1. W owner writes contract-level TS fixtures/interfaces first on its future branch:
   projected optional world triple, semantic ref/at/facing, selector result, expected
   exact matrix. These are not reported as Game implementation.
2. P shapes the smallest optional transport that satisfies authoring and authorized
   runtime consumers, including omission versus explicit all-zero.
3. A maps P and T without calculations; T implements the source/compiler contract.
4. G implements the smallest real safe catalog satisfying consumer fixtures.
5. W replaces fixtures with actual API projection + exact provider revision and wires
   Builder and Game.

This is development sequencing, not merge sequencing.

### Provider-first merge/release order

1. T merges to `rpg-toolkit/main`; record the actual tagged module/version produced by
   normal repo release automation (do not invent one in advance).
2. P merges to `rpg-api-protos/main`; record exact generated artifact revision/tag.
   T and P may merge in either order only if independently green; A needs both.
3. A pins released T/P, merges to `rpg-api/dev`, and is deployed/available in the
   paired acceptance environment.
4. G merges to `rpg-game-assets/main`; record the exact merged commit SHA chosen below.
5. W's exact PR head pins A-compatible protos and exact G commit, passes required
   private-asset checks, then merges to `rpg-dnd5e-web/dev`.
6. Director assembles evidence on existing #206/#204 and PR #203/#205; Kirk decides
   whether either tracking PR may merge/close.

No consumer merge precedes its provider. A web-only hand-built offset fixture cannot
satisfy the Game gate.

## T — toolkit issue/PR

### Files/seams

Start at fresh `origin/main`. Expected owners:

- `encounter/dungeonspec/spec.go`: optional authored presence type on room/canvas place
  and boss without a generic renderer transform type;
- `decode.go`: strict known-field decode;
- `validate.go`: exact length-three + finite values, canonical source paths;
- `compile.go`: copy presence/value to compiled placement; room-local to absolute `at`
  changes must not alter offset; and
- existing fixture/unit files for all placement locations.

### Acceptance

- Accept omitted and exact `[0,0,0]` as distinct authored presence cases.
- Accept positive/negative finite values on room place, canvas place, boss.
- Reject 0/1/2/4 components, string/null components, NaN and infinities through all
  decode paths with exact `ValidationError.field`.
- Keep facing/mount validation independent; offset cannot make invalid combinations
  valid.
- Round-trip/copy exactly; no clamping, pivot, snapping, facing rotation, or asset ref
  lookup.
- Prove blockers/mechanics unchanged.

### Validation

Run normal Go format/lint/test for the module plus named dungeonspec unit/fixture suites.
Record exact command/version and full green output in PR evidence.

## P — protos issue/PR

### Contract

Use the existing placement message(s) actually returned by:

- Authoring `FloorPlan.placements[]`; and
- authorized runtime `HexRecord.contents[]` (or its current concrete placement payload).

Add the smallest optional exact world-vector representation that preserves omission
versus authored zero. Reuse an existing suitable world vector only if it has exact
finite/presence semantics and no gameplay meaning; otherwise define a narrowly named
placement-offset message. Do not add catalog ids, intrinsic points, matrices,
quaternions, scale, semantic wall/support, or a generic transform.

### Acceptance

- Go/TS generated APIs expose optional presence and three exact components.
- Binary/JSON compatibility suite proves old payload omission and new zero/nonzero.
- Authoring and runtime fields have the same canonical game-world-unit/axis
  documentation and cover room/canvas props, room monsters, and boss placement.
- Snapshot/event placement types used by current web ingestion preserve the same
  optional presence/value; do not define an authoring-only dead field.
- Generated artifacts and breaking-change checks pass.
- No server replacement identity/call is introduced.

The exact proto type/field number is an implementation decision reviewed on P; this
plan intentionally does not fabricate it before inspecting the then-current heads.

## A — API issue/PR

### Flow

1. Accept exact source YAML through existing `PutDungeon(validate_only=true)`.
2. Preserve authored source and toolkit optional compiled value.
3. Project optional offset in authoring FloorPlan with source path/ref/absolute at/
   applicable facing.
4. Persist it into the immutable encounter snapshot at creation.
5. Return it on the authorized runtime placement in reconnect snapshot and live
   visible/remembered projection used by current web encounter state.
6. Preserve the same world-axis value when an authored monster/boss moves or is
   re-placed; only its canonical origin changes, and vacated records cannot retain a
   stale placement offset.
7. Never recompute it after source edits for an already-created encounter.

### Acceptance matrix

- representative room prop, canvas prop, room monster, and room boss;
- omission, explicit `[0,0,0]`, positive/negative XYZ;
- validate-only then save/reload then encounter create then authorized runtime read;
- reconnect snapshot, live update, VISIBLE-over-REMEMBERED, resight/vacate, and
  movement/re-placement with exact presence/value;
- fog authorization still suppresses unauthorized content;
- ref edit retain/change/remove cases are independent complete documents;
- unchanged owning cell/blockers/LoS/pathing/range/targetability/fog;
- no catalog import, clamping, vector rotation, or asset-specific behavior; and
- old documents/encounter snapshots remain readable.

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
entry, record screenshots and numeric matrix/control table across at least:

- three distinct valid existing facings;
- positive/negative local right and forward controls;
- positive vertical where visually relevant; and
- two representative camera distances including Discord viewport.

Reviewers confirm chosen range/step provides useful adjustment without presenting it
as YAML validity. If evidence cannot justify hints, stop for explicit #205 design
amendment; do not mark them optional.

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

### Provider provenance: explicit Kirk call

**Recommendation:** after G merges, choose its exact merged commit SHA. W commits a
provider lock containing exactly that G SHA, catalog digest, generator/tool identity
and version, and required repository/schema identity. It contains neither a web SHA
nor mutable branch name. Authenticated local/Docker sync fetches the G commit directly
and verifies `HEAD == locked SHA`. Actual `github.sha`/web HEAD is recorded only by the
running W check as a non-committed artifact and deployment/image label alongside the
verified lock hash.

If Kirk instead chooses a named release, G must additionally:

- create annotated `web-assets/v1.0.0` only after merge;
- install a no-bypass repository tag ruleset protecting `web-assets/v*` from update
  and deletion before creating the tag;
- W's provider lock records tag object id and
  `git rev-parse web-assets/v1.0.0^{}` peeled commit plus catalog/tool facts; and
- W verifies all of them. A lightweight/unprotected/moveable tag is not acceptable.

No tag or ruleset is created before Kirk's renewed plan approval and explicit choice.

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

Start one future branch from fresh `origin/dev`. W includes platform offset consumption
and visual catalog integration so one actual matrix path can be proven; it does not
replace T/P/A.

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
- generic `M_generic` exactly once for room/canvas prop and monster/boss fixtures;
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
   generator/tool identity/version (or protected tag object + peeled commit if Kirk
   chose tag); the lock contains no web SHA;
2. authenticated fetch/checkout exact revision in a clean temp directory;
3. assert exact HEAD/peeled SHA before reading files;
4. stage the **complete** legacy `harness/models/synty/` tree plus safe catalog;
5. verify byte-identical checked catalog, inventory, every enrolled digest, schema, and
   generator identity; and
6. atomically replace public tree+catalog after every check succeeds.

The running check, not a tracked file, emits evidence containing actual `github.sha`/
web HEAD, provider lock hash, verified provider SHA/catalog digest, and tool version.
Deployment/image labels record the same split. No tracked G/W wrapper contains its own
future/current web build SHA.

### 3. Public and protected exact-head gates

**Automatic public PR check (no secrets):** on every W PR, run schema/id/unit checks,
pure selector/resolver fixtures, public-safe catalog fixtures, lint/typecheck/unit/build
where licensed files are not required, and scan the diff/artifact manifest. Forks and
Dependabot receive only this path.

**Licensed exact-head check:** define the workflow on trusted `dev` and invoke it only
after review through a protected GitHub Environment requiring manual Kirk approval.
The trusted preflight must:

1. accept PR number + expected head SHA, query GitHub, and prove target is `dev`, head
   repo equals `KirkDiggler/rpg-dnd5e-web` (not fork/Dependabot), actor/author is an
   approved same-repository collaborator, and expected SHA equals the current PR head;
2. run no `pull_request_target` checkout of arbitrary code with secrets;
3. obtain a short-lived, least-privileged, read-only credential scoped only to reading
   the private provider, fetch/verify the locked G revision with trusted workflow code,
   then unset/revoke the credential and delete credential-bearing config before W head
   scripts execute;
4. checkout the exact W SHA and assert `git rev-parse HEAD == expected SHA`;
5. run full atomic stage in local-script and Docker-build modes, build/test/lint/
   typecheck, and paired Playwright/actual-Game evidence; and
6. publish the required status against that exact SHA.

Secret absence/expiry, trust mismatch, stale SHA, or missing Environment approval fails
the licensed gate; it never degrades to warning/skip. Scrub authenticated URLs/tokens
from git config, process environment, command echo, logs, and evidence. Disable general
caches or allowlist only dependency/build-cache paths that cannot contain the provider
checkout or public asset stage. Artifact upload uses an explicit allowlist (safe JSON,
lock/build evidence, hashes/inventory names, matrices, performance JSON, license-safe
screenshots); a pre-upload scan rejects `.glb`, archives, tokens, authenticated URLs,
private checkout content, or any unlisted file. Never upload the private checkout or
licensed public stage.

Before merge, query GitHub and assert the licensed required check `headSha` equals the
current W PR HEAD; stale green checks after pushes do not count. After merge, repeat a
protected smoke stage/build on exact `origin/dev` merge SHA and attach only allowlisted
non-committed evidence; do not substitute a local dirty tree.

### 4. Catalog projection and scale partition

Commit only safe generated JSON and the provider lock (G SHA + catalog digest + tool
identity, never web SHA). Preserve the complete legacy asset tree in built/public
staging without committing licensed GLBs; actual web HEAD remains non-committed check/
image evidence.

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
fixtures include a room prop, canvas prop, room monster, and room boss. Only the
enrolled bookcase/torch prop cases additionally select/apply catalog calibration.

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
room prop, canvas prop, room monster, and room boss, including one movement/re-placement
case. Storybook/Concepts Lab/local fake projection is not Game evidence.

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
- Builder + actual Game integration using A service/snapshot for representative room
  prop, canvas prop, room monster, and room boss;
- generic `P` exactly once on every fixture, catalog `C` only on enrolled props;
- six facing values (not claimed as six wall edges);
- positive/negative X/Y/Z, facing nonrotation of offset;
- replacement retain/change/remove;
- scale double-application and companion identity guards;
- invalid catalog/provenance/digest/no-point/load fallback;
- complete-tree atomic stage failure injection;
- full lint/typecheck/unit/build/Playwright from project environment.

## Cross-repo acceptance evidence

Use one checked evidence index on #205 linking immutable artifacts from T/P/A/G/W.
Every non-committed evidence artifact names issue/PR, actual exact web HEAD/base HEAD,
tracked provider-lock hash, provider SHA/tag+peeled SHA, catalog/tool/GLB digests,
commands, and pass/fail. The tracked provider lock itself never contains web HEAD.

### Platform evidence

- source → validate-only projection → save/reload → encounter snapshot → authorized
  runtime projection for omission/zero/nonzero;
- authoring/runtime exact component equality;
- old document/snapshot compatibility;
- source-path errors for malformed/nonfinite;
- fog authorization and mechanical invariants;
- ref edit retain/change/remove; and
- authorized VISIBLE/REMEMBERED/resight/vacate plus movement/re-placement snapshot/event
  truth sufficient for the web ingestion cases.

### Paired visual evidence

For identical provider/projection facts, capture Builder and actual Game:

- bookcase and torch matrices numerically equal within `1e-6` per element;
- screenshots at 1280×720 representative Discord viewport and one close diagnostic;
- omission, explicit `[0,0,0]`, and ±X/±Y/±Z representatives;
- representative room prop, canvas prop, room monster, and room boss with generic
  offset exactly once in both Builder and actual Game; only enrolled props show
  additional catalog calibration;
- VISIBLE-over-REMEMBERED, reconnect, live/resight/vacate, and movement/re-placement
  frames with no stale/double offset;
- six existing facing values where baseline accepts facing;
- facing changes model orientation while offset world vector stays unchanged;
- two pivot-distinct intrinsic points;
- replacement retain/change/remove; and
- no-point and model-load fallback at `p+o`.

Interior-wall/generated-envelope scenes may be captured as authoring examples only.
Do not label them wall registration/support parity and do not require six wall edges.

### Gameplay invariants

Compare before/after authoritative data for owning cell, blockers, collision/pathing,
range, targetability, LoS, fog/visibility, and interaction id. They must be exactly
unchanged across model/offset changes.

### Licensing

Attach file-name/inventory/license scan summaries only. Never upload raw GLBs/private
source or secret URLs to public project evidence.

## Quantitative performance gates

Run named methods on W exact PR HEAD/provider lock. Record CI image, CPU/OS, repository
Node/browser versions, base/head/provider SHAs, and case-table digest.

### Pure resolver statistic

- Environment: production resolver build under the repository CI Node version on one
  pinned Linux runner.
- Case table: six facings × three fixed nonzero world offsets, all translations in
  canonical game-world units.
- Sampling unit: one timed batch of 1,000 resolves, elapsed by `performance.now()` and
  divided by 1,000; do not time/log each call.
- Per run: fresh Node process, 10 untimed warmup batches, then 100 timed batch samples.
- p95: sort the 100 per-resolve samples; nearest-rank index
  `ceil(0.95*100)-1` zero-based.
- Aggregation: three fresh processes in the same job/runner; median of the three
  per-run p95s.
- Budget: median p95 ≤ `0.05 ms/resolve`.

### Actual-route base/head comparison

Run exact `origin/dev` base then exact W head sequentially in each of three paired
cycles on the same job/runner, Chromium version, fixed route/camera, and exact provider
lock. Wait for the same bookcase/torch to load plus 120 warmup frames, then use current
`DevPerfProbe`'s 8,000 ms window. Aggregate frame time as the median of the three
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

Attach allowlisted raw JSON/traces/environment only; never GLBs/private checkout. Any
breach blocks or requires a specific Kirk-approved measured exception on #205.

## Drift and atomicity gates

The following all hard-fail G/W release checks:

- generated catalog differs from generator output;
- catalog expected digest/revision differs between public artifact/runtime import;
- provider exact HEAD/peeled commit, catalog digest, or tool identity differs from
  the tracked W provider lock;
- enrolled GLB SHA-256 differs;
- complete legacy tree inventory is missing/partially from another provider revision;
- catalog schema/id/number/hint/companion invalid;
- asset is both enrolled and legacy-scaled;
- local and Docker stages differ byte-for-byte; or
- exact-head required check is stale relative to current W PR HEAD.

No warning-only drift path is acceptable for release.

## Rollout and rollback rehearsal

### Rollout

1. Merge/release T and P.
2. Merge/deploy A with passthrough disabled from mechanics by construction.
3. Validate authored source/persistence/runtime projection in a nonproduction dungeon.
4. Merge G, capture exact provider SHA, and cut the tracked W provider lock with G
   SHA + catalog digest + generator/tool identity (no web SHA).
5. On W exact head, atomically stage full legacy tree+catalog from the lock through the
   protected trust gate; record actual web HEAD only in non-committed evidence/labels.
6. Enable generic offset in Builder/actual Game for every valid placement kind, then
   enrolled bookcase/torch catalog calibration.
7. Run complete evidence/performance matrix.
8. Merge W to dev only after independent gates and Kirk approval.
9. Repeat exact dev-merge smoke; update #203/#205 tracking, no automatic merge.

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

1. Renewed course-corrected design and plan approved.
2. Exactly five T/P/A/G/W owning issues/branches/PRs are created only afterward and
   boarded with the specified Project 19 Platform/Assets fields; no wrapper issue.
3. v0.4 exact optional offset works end-to-end from YAML through actual Game, preserving
   omission/explicit-zero/value across snapshot/live/fog/movement state and remaining
   mechanically inert.
4. Safe catalog generator/registrar ships exact two ids/defaults, digest-bound scale/yaw/
   points, and required evidence-backed UX hints, all promoted into canonical game-world
   units through a tested conversion.
5. Tracked W provider lock pins exact G commit (or protected tag object + peeled commit),
   catalog digest, and tool identity with no web SHA; actual web HEAD appears only in
   non-committed evidence/labels.
6. Complete legacy tree + catalog stages atomically and identically for local/Docker;
   automatic public and manually approved licensed exact-head gates pass without secret/
   artifact leakage.
7. Named web state/helper seams prove generic `P` exactly once for room/canvas props and
   room monster/boss; only enrolled props receive catalog `C`; no double scale.
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

## Kirk judgments required before issue creation

1. **Provider provenance:** approve a tracked W lock with exact merged G SHA + catalog
   digest + tool identity and no web SHA (recommended), or require protected annotated
   `web-assets/v1.0.0` + object/peeled commit in that lock. Actual web HEAD stays in
   non-committed evidence/labels.
2. **Immutable ids:** approve exact literals/grammar and migration rule.
3. **Required hints:** approve evidence-backed initial UX hints as provider-release
   requirements, never server validity.
4. **Wall limit:** accept offset-only v0.4 and defer semantic wall/support persistence.
5. **Performance:** approve named quantitative budgets or supply replacements.
6. **#43:** confirm separate/nonblocking for #205 completion.

No judgment is assumed by this draft.
