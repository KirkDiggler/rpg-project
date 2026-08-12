<!-- markdownlint-disable MD013 -->

# Staged 3D Attack Die Concept Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Every product PR requires focused tests, its full repository gate, self-review, and an independent gate; the web PR also requires Copilot reconciliation. Kirk alone merges.

**Goal:** Prove a development-only, production-intent 3D attack die that decoratively tumbles and then shows each authoritative d20 result exactly, backed by a private asset-owned contract and immutable graduation evidence.

**Architecture:** Web owns an isolated overlay `Canvas`, strict runtime loading, token-locked 3D/SVG choice, kinematic settle, material treatment, and the four-stage concept. Assets owns the sole canonical 1–20 map, human-supplied provenance, a deterministic safe sidecar, and private evidence; the provider merges before the frozen web consumer becomes merge-ready. Live combat presentation remains unchanged, and production promotion is separate work.

**Tech Stack:** React 19, TypeScript 5.8, React Three Fiber 9, Three.js 0.181, Vitest/Testing Library, Playwright, Python 3 standard library/pytest, Git/GitHub CLI, Project 19.

**Spec:** `ideas/attack-die-3d/design.md`

## Global Constraints

- `AttackResolved.attackRoll` is the sole result authority. Visual code must not generate, choose, reroll, clamp, infer, or return a result.
- Do not modify production `src/components/game/combatPresentation/CombatPresentation.tsx`, `src/components/ui/dice/DiceTray.tsx`, or `src/components/game/EncounterView.tsx`; do not add rules, proto, API, toolkit, physics, damage-die, generic-dice, or live queue work.
- Use `/models/synty/props/SM_Prop_D20_Lightning_01.glb` and `/models/synty/dice/d20-lightning/attack-die-contract.json` directly. Web must not add or use a generic dungeon-palette resolver for these URLs.
- Preserve the existing d20 entry in assets `library/prop-role-map.json`; its current classification is outside this concept.
- A token snapshots already-validated readiness at its start. Not-ready means SVG for that whole token; late readiness affects only later tokens. The only current-token transition is irreversible `3d -> svg` on failure.
- `AttackDie3D` is visual-only: no `onComplete`, `onResultRelease`, queue callback, result callback, or timer extension. Existing sequencer timing and once-only completion remain authoritative.
- Animated and reduced-motion paths observe sign-invariant angular error `<= 0.25°`, then copy and hold the exact target quaternion. Missing the threshold fails closed without delaying the beat.
- Never reveal the `Canvas` before the requested result has a validated initial pose; load, hash, selector, WebGL, shader, context-loss, invalid-result, and missing-map failures show the authoritative SVG without a wrong-face flash.
- The Canvas is `aria-hidden`; the mounted SVG/`BeatStage` surface remains the sole semantic result source. Reduced motion skips tumble and animated lightning but preserves result, target, hold, timing order, and fallback.
- Private licensed GLBs stay out of the web diff and public evidence. Asset evidence uses the private repository's existing tracked render convention; PR text may reference safe PNG/contact-grid paths but must not expose binaries, private paths, provenance details, or reviewer identity.
- One web issue/branch/PR accumulates the concept; one asset issue/branch/PR accumulates the provider and evidence. Web starts from fetched `origin/dev`; assets starts from fetched `origin/main`; Kirk alone merges provider first and consumer second.
- Human provenance, appearance approval, face identity, and readability cannot be authored, inferred, or marked PASS by an agent. Guessed camera/light/material values and face orientations are never described as verified.
- Assets has no workflow framework: add no `.github/workflows/validate-attack-die-contract.yml`. Run local and independent gates and record absent asset CI as a residual risk.
- Every GitHub issue, PR, or comment body created during execution ends with `— asset-pipeline agent, on behalf of KirkDiggler`.

### Shared contracts

Use these names and shapes in all four tasks (readonly fields may be serialized as ordinary JSON arrays):

```ts
export type AttackDieMaterialMode = 'raw' | 'magical';
export type QuaternionTuple = readonly [number, number, number, number]; // xyzw
export type AttackDieRenderer = '3d' | 'svg';
export type Vector3Tuple = readonly [number, number, number];

export interface CameraContract {
  type: 'perspective'; fov: number; near: number; far: number;
  position: Vector3Tuple; target: Vector3Tuple; up: Vector3Tuple;
}

export interface AttackDie3DProps {
  result: number;
  presentationToken: number;
  phase: DiceTrayPhase;
  materialMode: AttackDieMaterialMode;
  reducedMotion: boolean;
  fallback: React.ReactNode;
  onTelemetry?: (event: AttackDieTelemetry) => void;
}

export interface AttackDieTelemetry {
  presentationToken: number;
  requestedResult: number;
  renderer: AttackDieRenderer;
  state: 'locked' | 'tumbling' | 'observed' | 'held' | 'failed' | 'disposed';
  mappedTarget?: QuaternionTuple;
  observedQuaternion?: QuaternionTuple;
  angularErrorDegrees?: number;
  exactTargetHeld: boolean;
  failureReason?: string;
}

export interface AttackDieRuntimeSidecar {
  schemaVersion: 1;
  kind: 'attack-die-runtime-contract';
  state: 'candidate' | 'verified';
  contractCoreSha256: string;
  asset: { url: '/models/synty/props/SM_Prop_D20_Lightning_01.glb'; sha256: string };
  coordinates: {
    quaternionOrder: 'xyzw'; handedness: 'right'; upAxis: '+Y';
    rootCorrection: QuaternionTuple; normalizationEpsilon: 0.000001;
  };
  selectors: {
    blenderSuffixPattern: '\\.\\d{3}$';
    node: 'D20_Lightning_preview_4pct';
    mesh: 'D20_Lightning_preview_4pct_Mesh';
    bodyMaterial: 'D20_Lightning_Material';
    numeralMaterial: 'Paint_Material';
    materialSlots: 2;
  };
  faces: ReadonlyArray<{ result: number; quaternion: QuaternionTuple }>;
  tuple: AttackDieEvidenceTuple;
  evidence: { machineRunSha256: string; humanReviewSha256: string; performanceSha256: string } | null;
}

export interface AttackDieEvidenceTuple {
  webCommit: string; webBuildSha256: string; glbSha256: string;
  contractCoreSha256: string; selectorRootRevision: string;
  topCamera: CameraContract; threeQuarterCamera: CameraContract;
  materialMode: AttackDieMaterialMode; shaderRevision: string;
  lightingRevision: string; environmentRevision: string;
  exposure: number; toneMapping: 'ACESFilmic'; outputColorSpace: 'sRGB';
  dieScale: number; viewportCss: readonly [number, number];
  outputPixels: readonly [number, number]; devicePixelRatio: number;
  toleranceDegrees: 0.25;
}
```

`contractCoreSha256` is SHA-256 over canonical JSON with recursively sorted object keys and order-preserving arrays. Its exact stable-runtime-core input is `{schemaVersion, kind, asset, coordinates, selectors, faces, tuple}` from `AttackDieRuntimeSidecar`, with `tuple.contractCoreSha256` omitted. Mutable lifecycle `state` and `evidence` are explicitly excluded: changing schema, kind, or any stable core member changes the digest, while the candidate-to-verified state transition and evidence binding do not. The full sidecar/evidence validator separately requires a valid lifecycle state and, for `state: 'verified'`, complete valid evidence references; stable core digest equality never makes missing/invalid evidence acceptable. Runtime accepts only `state: 'verified'`; development concept code may explicitly accept `candidate` or a provisional proposal. Unknown keys, duplicate JSON keys, non-integer/extra/missing face results, non-finite or zero quaternions, norm error above `1e-6`, a null/incorrect `webBuildSha256` in candidate or verified data, digest drift, tuple drift, and selector ambiguity are failures. Normalize by stripping one terminal `/\.\d{3}$/`; require exactly one normalized node, mesh, body material, and numeral material. Treat `q` and `-q` as equivalent and choose the shortest interpolation arc.

### Frozen build identity and preview lifecycle

Task 2 creates these package commands and keeps all manifests outside `dist/`:

```json
{
  "attack-die:build": "npm run build -- --mode development",
  "attack-die:hash-build": "node scripts/attack-die/hash-build.mjs",
  "attack-die:freeze-build": "bash scripts/attack-die/build-frozen.sh",
  "attack-die:preview": "node scripts/attack-die/serve-frozen.mjs",
  "attack-die:evidence": "node scripts/attack-die/capture-evidence.mjs",
  "perf:attack-die": "node scripts/perf/attack_die_paired.mjs"
}
```

`hash-build.mjs --dist dist --out <private-json>` walks regular files under `dist/`, sorts POSIX relative paths bytewise, and hashes canonical UTF-8 lines `<path>\0<size>\0<sha256>\n`. It writes `{schemaVersion:1, kind:"attack-die-web-build-manifest", files:[{path,size,sha256}], webBuildSha256}` outside `dist`; `webBuildSha256` is the SHA-256 of those exact canonical lines. The manifest cannot hash itself.

`build-frozen.sh --out <private-json>` requires a clean tracked/index state and non-empty `VITE_ATTACK_DIE_WEB_COMMIT`, temporarily moves ignored `public/models/synty/` to `mktemp -d`, installs an EXIT/INT/TERM trap that restores it, runs `npm run attack-die:build`, runs `npm run attack-die:hash-build -- --dist dist --out "$OUT"`, then restores the provider and clears the trap. It rejects output paths inside `dist/`. Thus build identity never contains the sidecar that stores the identity.

Run this exact deterministic freeze from one clean tracked `WEB_EVIDENCE_SHA` and unchanged provider inputs. Each `freeze-build` command recreates `dist`; B remains the served build. `build-frozen.sh` does not mutate provider inputs, so the second invocation consumes the same restored provider tree:

```bash
cd "$WEB_WT"
test "$(git rev-parse HEAD)" = "$WEB_EVIDENCE_SHA"
test -z "$(git status --porcelain --untracked-files=no)"
export VITE_ATTACK_DIE_WEB_COMMIT="$WEB_EVIDENCE_SHA"
rm -f /tmp/attack-die-build-a.json /tmp/attack-die-build-b.json \
  /tmp/attack-die-web-build-manifest.json
npm run attack-die:freeze-build -- --out /tmp/attack-die-build-a.json
npm run attack-die:freeze-build -- --out /tmp/attack-die-build-b.json
cmp -s /tmp/attack-die-build-a.json /tmp/attack-die-build-b.json
test "$(jq -er '.webBuildSha256' /tmp/attack-die-build-a.json)" = \
  "$(jq -er '.webBuildSha256' /tmp/attack-die-build-b.json)"
cp /tmp/attack-die-build-b.json /tmp/attack-die-web-build-manifest.json
```

Both manifests remain outside `dist/`. If either `cmp` or digest equality fails, stop before freeze; do not use a merely similar build. `serve-frozen.mjs` serves the exact hashed `dist/`, mounts the separately hash-bound provider tree at `/models/synty/`, and exposes the private manifest read-only at `/__attack-die-build-manifest.json`.

Every evidence/performance run uses port `4173` and this non-blocking lifecycle; never put `npm run dev` or a foreground preview before later commands:

```bash
PREVIEW_PID=
stop_frozen_preview() {
  if test -n "${PREVIEW_PID:-}" && kill -0 "$PREVIEW_PID" 2>/dev/null; then
    kill "$PREVIEW_PID"; wait "$PREVIEW_PID" || true
  fi
}
trap stop_frozen_preview EXIT INT TERM
npm run attack-die:preview -- --dist dist --asset-root public/models/synty \
  --build-manifest "$BUILD_MANIFEST" --host 127.0.0.1 --port 4173 \
  >/tmp/attack-die-preview.log 2>&1 &
PREVIEW_PID=$!
READY=0
for _ in $(seq 1 80); do
  if curl -fsS http://127.0.0.1:4173/__attack-die-build-manifest.json >/dev/null; then READY=1; break; fi
  kill -0 "$PREVIEW_PID" 2>/dev/null || { cat /tmp/attack-die-preview.log; exit 1; }
  sleep 0.25
done
test "$READY" = 1
# Run the evidence/performance commands while this process is alive.
stop_frozen_preview; PREVIEW_PID=; trap - EXIT INT TERM
```

The preview server and evidence driver reject path traversal and symlinks. The driver verifies every served hashed JS/CSS/asset path and byte digest that has a direct URL, verifies entry-document references for the remainder, separately verifies the GLB/sidecar digests, and injects the manifest's `webBuildSha256` into final proposal/evidence JSON. Unfrozen `npm run dev` exploration may export `webBuildSha256: null`; candidate/verified asset contracts and graduation evidence reject null.

---

### Task 1: Web production-intent renderer and four-stage shell

**Artifact:** Clean owning worktrees plus a synthetic-contract-tested `AttackDie3D` and registered development shell, with no production presentation edits.

**Files:**

- Create: `src/components/ui/dice/attackDieContract.ts`, `attackDieMotion.ts`, `attackDieRuntime.ts`, `attackDieMaterial.ts`, `AttackDie3D.tsx`, and adjacent `*.test.ts(x)` files.
- Create: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx` and `AttackDie3DConcept.test.tsx` (stage navigation shell only).
- Modify: `src/concepts/ConceptsView.tsx`, `public/themes/base.css`.
- Must remain unchanged: `src/components/game/combatPresentation/CombatPresentation.tsx`, `src/components/ui/dice/DiceTray.tsx`, `src/components/game/EncounterView.tsx`.

**Interfaces:** Produces the shared types above plus these executable boundaries.
The snapshot is immutable; `AttackDie3D` reads it once for each
`presentationToken`.

```ts
export interface AttackDieRuntimeSnapshot {
  status: 'idle' | 'loading' | 'ready' | 'failed';
  sidecar?: AttackDieRuntimeSidecar;
  failureReason?: string;
}
export interface AttackDieMotionInput {
  elapsedMs: number; reducedMotion: boolean;
  current: QuaternionTuple; target: QuaternionTuple;
}
export interface AttackDieMotionFrame {
  quaternion: QuaternionTuple; observeNow: boolean;
  exactTargetHeld: boolean; failed: boolean;
}
export function preloadAttackDieRuntime(): Promise<void>;
export function getAttackDieRuntimeSnapshot(): AttackDieRuntimeSnapshot;
export function angularDistanceDegrees(a: QuaternionTuple, b: QuaternionTuple): number;
export function stepAttackDieMotion(input: AttackDieMotionInput): AttackDieMotionFrame;
```

- [ ] **Step 1: Create and board both owning issues before any branch.** Run from a neutral directory; every `jq -e` must pass exactly once or execution stops before `fetch`/`worktree add`:

```bash
set -euo pipefail
OWNER=KirkDiggler; PROJECT_NUMBER=19
WEB_REPO=$OWNER/rpg-dnd5e-web; ASSET_REPO=$OWNER/rpg-game-assets
SIG='— asset-pipeline agent, on behalf of KirkDiggler'
WEB_ISSUE_URL=$(gh issue create --repo "$WEB_REPO" --title 'feat: prove staged 3D authoritative attack die' --body "Implements the approved concept tracked by https://github.com/KirkDiggler/rpg-project/pull/217. Concept and graduation evidence only; live CombatPresentation, DiceTray, EncounterView, and production promotion are excluded.

$SIG")
ASSET_ISSUE_URL=$(gh issue create --repo "$ASSET_REPO" --title 'asset: publish verified lightning d20 attack contract' --body "Owns human provenance receipt validation, GLB-bound selectors, the sole face map, safe runtime sidecar, and private evidence for https://github.com/KirkDiggler/rpg-project/pull/217.

$SIG")
WEB_ISSUE_NUMBER=${WEB_ISSUE_URL##*/}; ASSET_ISSUE_NUMBER=${ASSET_ISSUE_URL##*/}
PROJECT_ID=$(gh project view "$PROJECT_NUMBER" --owner "$OWNER" --format json | jq -er '.id')
FIELDS=$(mktemp); gh project field-list "$PROJECT_NUMBER" --owner "$OWNER" --format json >"$FIELDS"
STATUS_FIELD=$(jq -er '[.fields[]|select(.name=="Status" and .type=="ProjectV2SingleSelectField")]|if length==1 then .[0].id else error("Status field mismatch") end' "$FIELDS")
TODO_OPTION=$(jq -er '[.fields[]|select(.name=="Status")|.options[]|select(.name=="Todo")]|if length==1 then .[0].id else error("Todo option mismatch") end' "$FIELDS")
TEAM_FIELD=$(jq -er '[.fields[]|select(.name=="Team" and .type=="ProjectV2SingleSelectField")]|if length==1 then .[0].id else error("Team field mismatch") end' "$FIELDS")
ASSETS_OPTION=$(jq -er '[.fields[]|select(.name=="Team")|.options[]|select(.name=="Assets")]|if length==1 then .[0].id else error("Assets option mismatch") end' "$FIELDS")
for URL in "$WEB_ISSUE_URL" "$ASSET_ISSUE_URL"; do
  ITEM_ID=$(gh project item-add "$PROJECT_NUMBER" --owner "$OWNER" --url "$URL" --format json | jq -er '.id')
  gh project item-edit --id "$ITEM_ID" --project-id "$PROJECT_ID" --field-id "$TEAM_FIELD" --single-select-option-id "$ASSETS_OPTION"
  gh project item-edit --id "$ITEM_ID" --project-id "$PROJECT_ID" --field-id "$STATUS_FIELD" --single-select-option-id "$TODO_OPTION"
done
ITEMS=$(mktemp); gh project item-list "$PROJECT_NUMBER" --owner "$OWNER" --limit 1000 --format json >"$ITEMS"
for URL in "$WEB_ISSUE_URL" "$ASSET_ISSUE_URL"; do
  jq -e --arg url "$URL" '[.items[]|select(.content.url==$url and .team=="Assets" and .status=="Todo")]|length==1' "$ITEMS"
done
```

Expected: two issue URLs and two uniquely verified board items with `Team=Assets`, `Status=Todo`. On auth, schema, option, or verification failure, stop and repair the existing items; do not create duplicates or branches.

- [ ] **Step 2: Fetch exact bases and create one clean worktree per issue.** Record `WEB_BASE` and `ASSET_BASE` in the implementation log:

```bash
WEB_ROOT=/home/kirk/game-dev/rpg-dnd5e-web; ASSET_ROOT=/home/kirk/game-dev/rpg-game-assets
WEB_WT="$WEB_ROOT/.worktrees/attack-die-$WEB_ISSUE_NUMBER"; ASSET_WT="$ASSET_ROOT/.worktrees/attack-die-$ASSET_ISSUE_NUMBER"
WEB_BRANCH="feat/$WEB_ISSUE_NUMBER-attack-die-3d-concept"; ASSET_BRANCH="asset/$ASSET_ISSUE_NUMBER-attack-die-contract"
git -C "$WEB_ROOT" fetch origin dev; git -C "$ASSET_ROOT" fetch origin main
WEB_BASE=$(git -C "$WEB_ROOT" rev-parse origin/dev); ASSET_BASE=$(git -C "$ASSET_ROOT" rev-parse origin/main)
git -C "$WEB_ROOT" worktree add -b "$WEB_BRANCH" "$WEB_WT" "$WEB_BASE"
git -C "$ASSET_ROOT" worktree add -b "$ASSET_BRANCH" "$ASSET_WT" "$ASSET_BASE"
test -z "$(git -C "$WEB_WT" status --porcelain)"; test -z "$(git -C "$ASSET_WT" status --porcelain)"
```

Expected: clean web at fetched `origin/dev`, clean assets at fetched `origin/main`, and no second branch/PR for either wave.

- [ ] **Step 3: Record required performance-profile availability before renderer code.** A human supplies `/tmp/attack-die-required-profiles.json` with exactly three entries whose `category` values are `desktop-chromium`, `desktop-discord-activity`, and `mobile-low-gpu`. Each has `status: "available"` plus non-empty `clientOrBrowser`, `os`, `hardwareGpu`, `powerState`, `viewport`, and numeric positive `dpr`; only `mobile-low-gpu` may instead have `status: "blocked"` plus a non-empty `reason`. Never infer values. Validate and post those human-supplied bytes to the owning web issue:

```bash
jq -e '(.profiles|length)==3 and ([.profiles[].category]|sort)==["desktop-chromium","desktop-discord-activity","mobile-low-gpu"] and all(.profiles[]; if .status=="available" then ([.clientOrBrowser,.os,.hardwareGpu,.powerState,.viewport]|all(type=="string" and length>0)) and (.dpr|type=="number" and .>0) else .category=="mobile-low-gpu" and .status=="blocked" and (.reason|type=="string" and length>0) end)' /tmp/attack-die-required-profiles.json
{
  printf 'Human-confirmed required profile availability before implementation:\n\n```json\n'
  cat /tmp/attack-die-required-profiles.json
  printf '\n```\n\n— asset-pipeline agent, on behalf of KirkDiggler\n'
} >/tmp/attack-die-profile-comment.md
test "$(tail -n 1 /tmp/attack-die-profile-comment.md)" = '— asset-pipeline agent, on behalf of KirkDiggler'
gh issue comment "$WEB_ISSUE_NUMBER" --repo "$WEB_REPO" --body-file /tmp/attack-die-profile-comment.md
```

All commands exit `0`. A blocked mobile/low-GPU category permits Tasks 1–2 concept exploration, but it blocks Task 3 verified provider publication/graduation and all Task 4 completion.

- [ ] **Step 4: Write strict synthetic red tests.** Fixtures use fake GLB bytes/digests and explicit 1–20 quaternions only; they must not encode claims about the licensed die. Cover exact schema/key rejection; stable core digest changes for schema/kind/core mutation but not candidate-to-verified state/evidence binding; separate rejection of verified sidecars with missing/invalid evidence; selectors; `q/-q`; shortest arc; invalid result without clamping; `<=0.25°` observation then exact repeated copy; reduced motion; token readiness snapshot; late-readiness exclusion; irreversible failure; stale callback/cleanup; hidden-until-truthful Canvas; forced load/hash/WebGL/shader/context failures; single mounted semantic fallback; and absence of completion/release props.

```bash
cd "$WEB_WT"
npm run test:run -- src/components/ui/dice/attackDieContract.test.ts src/components/ui/dice/attackDieMotion.test.ts src/components/ui/dice/attackDieRuntime.test.ts src/components/ui/dice/attackDieMaterial.test.ts src/components/ui/dice/AttackDie3D.test.tsx src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
```

Expected red: non-zero with missing modules/components, not a test-environment failure.

- [ ] **Step 5: Implement the minimal green renderer.** Hash fetched GLB bytes with Web Crypto before `GLTFLoader.parse`; cache only a completely validated bundle. Give each token its own cloned scene/materials and animation lifecycle, use a component-owned overlay `<Canvas aria-hidden="true">`, patch only the uniquely normalized body material, preserve numeral material properties, and dispose token work/listeners/RAF while retaining the bounded shared cache until its owner is destroyed. Run a 2,000 ms decorative throw, reserve the final 800 ms for shortest-arc convergence, observe by 1,900 ms, then copy target on every settled frame; reduced motion applies target before first visible paint, observes next RAF, and fixes shader time at zero. Never expose Canvas before pose validation.

- [ ] **Step 6: Register `attack-die-3d` only in the existing development Concepts Lab.** The shell has keyboard-operable Appearance, Calibrate, Roll, and Verify tabs and renders `AttackDie3D` with a `DiceTray`/`BeatStage` fallback fixture. Do not add production navigation or a feature flag.

- [ ] **Step 7: Prove green and sequencing non-regression, then commit.**

```bash
npm run test:run -- src/components/ui/dice/attackDieContract.test.ts src/components/ui/dice/attackDieMotion.test.ts src/components/ui/dice/attackDieRuntime.test.ts src/components/ui/dice/attackDieMaterial.test.ts src/components/ui/dice/AttackDie3D.test.tsx src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run test:run -- src/components/ui/dice/DiceTray.test.tsx src/components/game/combatPresentation/CombatPresentation.test.tsx src/components/game/EncounterView.test.tsx
git diff --name-only "$WEB_BASE"...HEAD -- src/components/game/combatPresentation/CombatPresentation.tsx src/components/ui/dice/DiceTray.tsx src/components/game/EncounterView.tsx
git add src/components/ui/dice/attackDie* src/components/ui/dice/AttackDie3D* src/concepts/attack-die-3d src/concepts/ConceptsView.tsx public/themes/base.css
git commit -m 'feat: add authoritative 3D attack die renderer'
```

Expected per command: the focused renderer/concept test command exits `0`; the existing presentation-regression test command exits `0`; the protected-production-file diff prints no paths; `git add` stages only the listed concept files; and the commit succeeds.

### Task 2: Web appearance, calibration, roll, verification, and performance tooling

**Artifact:** The single web PR provides real-browser authoring/evidence tools and a frozen reviewed web commit, without queue authority or private tracked assets.

**Files:**

- Create: `src/concepts/attack-die-3d/attackDieExperiment.ts` and test; expand `AttackDie3DConcept.tsx` and test.
- Create: `src/dev/AttackDiePerfHarness.tsx` and test; `scripts/attack-die/frozenBuildManifest.ts` and test.
- Create: `scripts/attack-die/hash-build.mjs`, `scripts/attack-die/build-frozen.sh`, `scripts/attack-die/serve-frozen.mjs`, `scripts/attack-die/capture-evidence.mjs`, `scripts/perf/attack_die_paired.mjs`, `docs/how-to/attack-die-3d-concept.md`.
- Modify: `src/App.tsx`, `package.json`, `public/themes/base.css`.

**Interfaces:** The exporter emits the exact proposal below. `AttackDiePerfHarness` exposes only a development `window.__attackDiePerf` driver (`runSample({mode,result,reducedMotion,token})`, `readCounters()`, `unmountDie()`); it never imports or receives encounter queue callbacks.

```ts
interface AttackDieCalibrationProposal {
  schemaVersion: 1;
  kind: 'attack-die-calibration-proposal';
  warning: 'PROVISIONAL — NOT AN ASSET CONTRACT';
  webCommit: string;
  asset: AttackDieRuntimeSidecar['asset'];
  coordinates: AttackDieRuntimeSidecar['coordinates'];
  selectors: AttackDieRuntimeSidecar['selectors'];
  webBuildSha256: string | null;
  tupleDraft: Omit<AttackDieEvidenceTuple, 'contractCoreSha256' | 'webBuildSha256'>;
  faces: ReadonlyArray<{ result: number; quaternion: QuaternionTuple }>;
}
```

- [ ] **Step 1: Add red reducer/component/harness/driver/build-hash tests.** Require proposal export for 0–20 unique normalized mappings, exact warning text, no provenance/human-PASS fields, same-pose camera switching, fine `0.1°` controls, forced fallbacks, keyboard focus order, narrow view, reduced-motion suppression, real-route harness development gate, 20-per-mode alternating samples, 8-second post-unmount window, frozen performance budget calculations, canonical sorted build-manifest hashing, manifest self-exclusion, and changed-byte/path invalidation.

```bash
cd "$WEB_WT"
npm run test:run -- src/concepts/attack-die-3d/attackDieExperiment.test.ts src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx src/dev/AttackDiePerfHarness.test.tsx
npm run test:run -- scripts/attack-die/frozenBuildManifest.test.ts
```

Expected red: both commands are non-zero because the experiment/harness and build-manifest module do not exist.

- [ ] **Step 2: Implement the four tools with explicit provisional visual defaults.** Appearance shows Raw/Magical, animation pause, actual digest/selectors, and identical top/three-quarter captures. Calibrate starts with zero saved faces, supports result 1–20, local-axis coarse/fine controls, normalized save/reset, and camera changes that do not move the die. Roll shows input, target, measured error, renderer lock/fallback, and repeatable decorative variation. Verify drives fixed 1→20 animated/reduced runs and separates machine observations from human review. Begin with these visibly labelled **unverified provisional defaults**: FOV `35`, near/far `0.1/100`; top position `[0,4,0]`, target `[0,0,0]`, up `[0,0,-1]`; three-quarter position `[3,2.4,3]`, target `[0,0,0]`, up `[0,1,0]`; viewport `320x320`, output `640x640`, DPR `2`; scale `0.75`; ACESFilmic/sRGB/exposure `1`, no environment; ambient `0.65`, key `[4,6,5]` intensity `3`, fill `[-4,2,-3]` intensity `1.2`; shader `attack-die-magical-v1` with restrained cyan-white body emission and untouched gold numerals.

- [ ] **Step 3: Implement the frozen-build, real-browser, and paired-performance drivers.** Put canonical path sorting/record encoding/digest logic in `frozenBuildManifest.ts`; keep `hash-build.mjs` as its CLI and `build-frozen.sh` as the provider-withholding wrapper. Add the six package commands and exact build identity/preview lifecycle defined above. `attack-die:build` runs the existing `npm run build` with Vite mode `development`, so the optimized frozen build contains the development-only concept without changing ordinary production builds. `attack-die:evidence` accepts `--url`, `--out`, `--build-manifest`, `--all-results`, `--animated`, and `--reduced-motion`; it captures actual Chromium telemetry/screenshots and can force load, WebGL, shader, context-loss, hash, invalid-result, and unmapped failures. `perf:attack-die` accepts `--base-url`, `--build-manifest`, `--profile-file`, `--out`, `--samples-per-mode`, and `--post-unmount-ms`; it runs on the real `EncounterView` route with the independent overlay harness: same frozen build/profile/encounter/viewport/DPR, warmed GLB/shader, exactly 20 SVG and 20 3D throw-through-verdict windows in alternating order, then a fixed 8,000 ms post-unmount sample. Record per-sample/median/p95 frame time, >50 ms long tasks, request bytes/count, cold/warm ready/decode, contexts/losses, draw calls/triangles, heap, and GPU bytes or explicit `renderer.info` proxies. Enforce candidate p95 `<=110%` of SVG, no new attributable >50 ms task, and post-unmount p95 `<=110%` of SVG. The perf harness never invokes, replaces, receives, or imports queue callbacks; Task 4 separately exercises real FIFO behavior.

- [ ] **Step 4: Green the focused tests and perform human visual freeze before evidence.** Sync assets only from the controlled asset worktree into ignored public files, run Chromium, and exercise keyboard, narrow viewport, reduced motion, cameras, raw/magical, and every forced fallback. A human approves or changes camera/light/material settings; commit approved values before evidence. Do not label initial guesses verified, and do not record face readability PASS here.

```bash
npm run test:run -- src/concepts/attack-die-3d/attackDieExperiment.test.ts src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx src/dev/AttackDiePerfHarness.test.tsx
npm run test:run -- scripts/attack-die/frozenBuildManifest.test.ts
rsync -a --delete "$ASSET_WT/harness/models/synty/" "$WEB_WT/public/models/synty/"
git check-ignore -q public/models/synty/ && test -z "$(git ls-files public/models/synty/)"
```

Expected: both focused test commands and the ignore/leak checks exit `0`; human approval is recorded in the web PR history, not invented in JSON. This step uses unfrozen development exploration only; it may export `webBuildSha256: null` and does not create graduation evidence.

- [ ] **Step 5: Commit, run the full web gate, reconcile Copilot, obtain an independent gate, and freeze.** Valid Copilot findings receive tests/fixes and another full gate; every finding gets validity/action/rationale. The independent reviewer receives exact HEAD, diff, focused/full outputs, leak check, and browser evidence. No Critical/Important finding may remain.

```bash
git add package.json src/App.tsx src/concepts/attack-die-3d src/dev/AttackDiePerfHarness* scripts/attack-die scripts/perf/attack_die_paired.mjs docs/how-to/attack-die-3d-concept.md public/themes/base.css
git commit -m 'feat: add attack die calibration and evidence tools'
npm run ci-check
test -z "$(git diff --name-only "$WEB_BASE"...HEAD | grep -E '(^public/models/synty/|\.(glb|fbx|blend)$)' || true)"
test -z "$(git diff --name-only "$WEB_BASE"...HEAD -- src/components/game/combatPresentation/CombatPresentation.tsx src/components/ui/dice/DiceTray.tsx src/components/game/EncounterView.tsx)"
git push -u origin "$WEB_BRANCH"
WEB_PR_URL=$(gh pr create --repo "$WEB_REPO" --head "$WEB_BRANCH" --base dev --title 'feat: prove staged 3D authoritative attack die' --body "Closes $WEB_ISSUE_URL. Implements the approved development concept from rpg-project PR #217. Live combat wiring and production promotion are excluded.

— asset-pipeline agent, on behalf of KirkDiggler")
WEB_PR_NUMBER=${WEB_PR_URL##*/}
gh api "repos/$WEB_REPO/pulls/$WEB_PR_NUMBER/comments" --paginate
npm run ci-check
WEB_EVIDENCE_SHA=$(git rev-parse HEAD)
test -z "$(git status --porcelain --untracked-files=no)"
```

Expected: web PR remains open, all gates exit `0`, no tracked code changes after `WEB_EVIDENCE_SHA`. Run the exact deterministic double-build commands above; A/B manifests and hashes must match byte-for-byte, B remains in `dist`, provider assets are restored, and `/tmp/attack-die-web-build-manifest.json` designates B for later evidence. If reproduction fails, stop before freeze.

### Task 3: Assets canonical contract and graduation evidence

**Artifact:** One ready asset PR contains the human provenance receipt, sole canonical map, deterministic safe sidecar, and private immutable graduation evidence; Kirk merges it before Task 4.

**Files:**

- Human supplies: `library/custom-dice/d20-lightning/provenance.json`.
- Create: `library/custom-dice/d20-lightning/attack-die-contract.json`, `scripts/build_attack_die_contract.py`, `scripts/test_build_attack_die_contract.py`, `harness/models/synty/dice/d20-lightning/attack-die-contract.json`, `evidence/attack-die-3d/**`.
- Modify/Test: `library/custom-dice/d20-lightning/asset.json`, `library/custom-dice/d20-lightning/README.md`, `README.md`, `scripts/test_build_mesh_stats.py`, `scripts/test_build_web_asset_catalog.py`, `harness/models/synty/mesh-stats.json`, `harness/catalogs/synty-complete-inventory.json`.
- Must remain unchanged: every `*.glb`, `*.fbx`, `*.blend`, `library/prop-role-map.json`, and `.github/workflows/**`.

**Interfaces:** Canonical JSON has exactly `{canonicalSchemaVersion:1, kind:'attack-die-canonical-contract', runtime:AttackDieRuntimeSidecar, privateRefs:{provenanceReceiptSha256:string, machineRunPath:string, humanReviewPath:string, performancePath:string}}`. The generator validates private references, computes the core digest, and emits canonical `runtime` only; the emitted sidecar contains no `privateRefs`, provenance, private paths, attester, or reviewer identity.

- [ ] **Step 1: Stop for human provenance.** A human must supply a `status: "confirmed"` receipt with non-empty owner/licensor, grant basis, permitted private-repository and game-build uses, public-redistribution restriction, attester/date, source-evidence references, exact runtime hash, and derived hashes. The agent validates and commits those supplied bytes verbatim; it must not write, infer, complete, or change confirmation fields.

- [ ] **Step 2: Use the frozen web calibration UI to create the complete provisional proposal.** Verify web HEAD equals `WEB_EVIDENCE_SHA`, sync the current asset tree into ignored web public files, and start the exact frozen `dist` with `BUILD_MANIFEST=/tmp/attack-die-web-build-manifest.json` and the reusable background preview lifecycle. Manually calibrate results 1–20 at `http://127.0.0.1:4173/?concept=attack-die-3d` without inferred defaults; export `/tmp/attack-die-calibration-proposal.json`. It is still non-contract and contains no human PASS. Reject it unless its measured GLB SHA is `8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa`, its `webBuildSha256` equals the manifest, its normalized selectors match the shared contract, and results are exactly `[1..20]`.

```bash
cd "$WEB_WT"
test "$(git rev-parse HEAD)" = "$WEB_EVIDENCE_SHA"
rsync -a --delete "$ASSET_WT/harness/models/synty/" "$WEB_WT/public/models/synty/"
BUILD_MANIFEST=/tmp/attack-die-web-build-manifest.json
# Start/ready the frozen preview with the reusable port-4173 lifecycle above.
jq -e --arg build "$(jq -er '.webBuildSha256' "$BUILD_MANIFEST")" '.schemaVersion==1 and .kind=="attack-die-calibration-proposal" and .warning=="PROVISIONAL — NOT AN ASSET CONTRACT" and .webBuildSha256==$build and .asset.sha256=="8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa" and ([.faces[].result]==[range(1;21)])' /tmp/attack-die-calibration-proposal.json
# Stop/clean the preview with the reusable lifecycle above.
```

Expected: `jq` exits `0`; any unreadable/uncertain face returns to calibration rather than being guessed. The frozen preview is always cleaned up by trap.

- [ ] **Step 3: Write the deterministic validator/generator red tests.** Cover duplicate JSON keys, exact integer 1–20 set, finite normalized/canonical-sign quaternions, `q/-q`, GLB/library byte identity and hash, normalized selector cardinality (including current `.001` mesh/`.010` materials), root/camera/material/render tuple drift, schema/kind/stable-core digest drift, identical core digest across candidate-to-verified state/evidence binding, separate rejection of missing/invalid verified evidence, deterministic candidate/verified projection, human receipt requirements, and safe-projection leakage.

```bash
cd "$ASSET_WT"
python3 scripts/test_build_attack_die_contract.py
```

Expected red: non-zero because `build_attack_die_contract.py` does not exist.

- [ ] **Step 4: Implement the generator and import only the validated proposal.** Use Python standard library GLB parsing and duplicate-key-rejecting JSON loads. `build_attack_die_contract.py` supports normal write, `--check`, `--require-verified`, and `--verify-evidence`; canonical source is the sole authored map and the harness sidecar is always regenerated. Candidate output is accepted only by development web code and rejects null build identity. Keep existing `propKey`/classification metadata and add direct URL, schema, selector normalization, kinematic-only, generation, and invalidation documentation. Run `build_mesh_stats.py` before asserting the existing d20's 1,906 triangles, two primitives/materials, zero textures/animations, and no prop-budget warning. In `scripts/test_build_web_asset_catalog.py`, replace PR #46's stale fixed `2,143` file count/tree hash and comment with this exact generated-tree consistency assertion: `document, expected = stage.load_complete_inventory(COMPLETE_INVENTORY)`; `actual = stage.inventory(MODELS)`; `self.assertEqual(expected, actual)`; `self.assertEqual(document["fileCount"], len(actual))`; and `self.assertEqual(document["treeSha256"], stage.tree_sha256(actual))`, while retaining a known-file assertion and arbitrary-deletion test. This bounded prerequisite repair is not a new subsystem. Regenerate complete inventory last; do not edit the licensed GLB, prop-role map, or workflows.

```bash
cd "$ASSET_WT"
python3 scripts/build_attack_die_contract.py
python3 scripts/build_mesh_stats.py
python3 scripts/test_build_attack_die_contract.py
python3 scripts/test_build_mesh_stats.py
python3 scripts/build_synty_complete_inventory.py
python3 scripts/test_build_web_asset_catalog.py
python3 scripts/build_synty_complete_inventory.py --check
git add README.md library/custom-dice/d20-lightning \
  scripts/build_attack_die_contract.py scripts/test_build_attack_die_contract.py \
  scripts/test_build_mesh_stats.py scripts/test_build_web_asset_catalog.py \
  harness/models/synty/dice harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-complete-inventory.json
git commit -m 'feat: validate calibrated attack die asset contract'
ASSET_CANDIDATE_SHA=$(git rev-parse HEAD)
```

Expected per command: contract generation, mesh-stat regeneration, and focused contract/mesh tests exit `0`; complete inventory regeneration is the last tree-mutating command; the subsequent web-catalog test and inventory `--check` are read-only and exit `0`; staging includes every listed generated output; commit succeeds; candidate sidecar is deterministic and web-safe.

- [ ] **Step 5: Re-sync the committed candidate and capture actual frozen-renderer evidence.** The candidate commit may have changed the sidecar, mesh stats, and complete inventory, so replace ignored web assets from that exact candidate before capture. Start the exact frozen build with the reusable port-4173 background lifecycle, then run the exact command below; it writes machine JSON, screenshots, face views, and pending human-review request only under the private asset worktree.

```bash
cd "$WEB_WT"
test "$(git -C "$ASSET_WT" rev-parse HEAD)" = "$ASSET_CANDIDATE_SHA"
rsync -a --delete "$ASSET_WT/harness/models/synty/" "$WEB_WT/public/models/synty/"
BUILD_MANIFEST=/tmp/attack-die-web-build-manifest.json
# Start/ready the frozen preview with the reusable port-4173 lifecycle above.
npm run attack-die:evidence -- \
  --url 'http://127.0.0.1:4173/?concept=attack-die-3d' \
  --out "$ASSET_WT/evidence/attack-die-3d" \
  --build-manifest "$BUILD_MANIFEST" --all-results --animated --reduced-motion
# Keep preview alive for Step 6, or stop it with the reusable cleanup.
```

Expected: exit `0`; every one of 40 machine observations (20 animated, 20 reduced-motion) is `<=0.25°` and reports at least three subsequent exact-target-held frames. Output includes raw/magical top and three-quarter PNGs, `faces/top/01.png`–`20.png`, `faces/three-quarter/01.png`–`20.png`, contact grids, `verification-run.json`, and pending `human-review.json`. A human—not the script/agent—records 40 explicit numeral identity/readability decisions and appearance approval bound to the exact tuple/build hash.

- [ ] **Step 6: Capture paired performance on the same frozen app.** Copy the exact human-confirmed `/tmp/attack-die-required-profiles.json` bytes to `$ASSET_WT/evidence/attack-die-3d/performance/profiles.json` and confirm all three required categories are now available. A BLOCKED/missing mobile/low-GPU profile stops provider graduation/publication here even though Tasks 1–2 exploration was allowed. Never invent browser/client, OS, hardware/GPU, power, viewport, or DPR. With the same background server still at `4173` (or a fresh instance started by the reusable lifecycle), run exactly 20 alternating SVG/3D samples per profile and write the tracked private result below. An unexposed metric needs an explicit proxy/limitation; input/underlying-dungeon regression, context/lifecycle failure, retained-resource leak, or budget failure blocks graduation.

```bash
cd "$WEB_WT"
cp /tmp/attack-die-required-profiles.json "$ASSET_WT/evidence/attack-die-3d/performance/profiles.json"
jq -e 'all(.profiles[]; .status=="available")' "$ASSET_WT/evidence/attack-die-3d/performance/profiles.json"
BUILD_MANIFEST=/tmp/attack-die-web-build-manifest.json
npm run perf:attack-die -- --base-url http://127.0.0.1:4173 \
  --build-manifest "$BUILD_MANIFEST" \
  --profile-file "$ASSET_WT/evidence/attack-die-3d/performance/profiles.json" \
  --out "$ASSET_WT/evidence/attack-die-3d/performance/paired-results.json" \
  --samples-per-mode 20 --post-unmount-ms 8000
# Stop/clean the preview with the reusable lifecycle above.
```

Expected: exit `0` only when every required profile and frozen budget passes; preview cleanup exits without leaving a server.

- [ ] **Step 7: Bind evidence, run focused/full local asset gates, and independently review.** Stop first unless all three required performance categories, including mobile/low-GPU, are available and passed. Set runtime state to `verified`, bind content digests (not identities/paths) in the safe sidecar, and assert its `contractCoreSha256` still equals the candidate core digest even though the full validator now requires verified evidence. Regenerate every affected output, run `build_mesh_stats.py` before d20 assertions, and regenerate complete inventory last before its check. The asset repo has no CI workflow; do not add one.

```bash
cd "$ASSET_WT"
CANDIDATE_CORE_SHA=$(git show "$ASSET_CANDIDATE_SHA:harness/models/synty/dice/d20-lightning/attack-die-contract.json" | jq -er '.contractCoreSha256')
python3 scripts/build_attack_die_contract.py
test "$(jq -er '.contractCoreSha256' harness/models/synty/dice/d20-lightning/attack-die-contract.json)" = "$CANDIDATE_CORE_SHA"
python3 scripts/build_prop_manifest.py
python3 scripts/build_mesh_stats.py
python3 scripts/test_build_attack_die_contract.py
python3 scripts/test_build_mesh_stats.py
python3 scripts/build_web_asset_catalog.py --check
python3 scripts/build_synty_complete_inventory.py
python3 scripts/test_build_web_asset_catalog.py
python3 scripts/build_attack_die_contract.py --check --require-verified --verify-evidence
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/verify_web_asset_stage.py --verify-only
python3 -m pytest scripts/ -q
sha256sum library/custom-dice/d20-lightning/SM_Prop_D20_Lightning_01.runtime.glb harness/models/synty/props/SM_Prop_D20_Lightning_01.glb
cmp -s library/custom-dice/d20-lightning/SM_Prop_D20_Lightning_01.runtime.glb harness/models/synty/props/SM_Prop_D20_Lightning_01.glb
test -z "$(git diff --name-only "$ASSET_BASE"...HEAD -- '*.glb' '*.fbx' '*.blend' library/prop-role-map.json '.github/workflows/**')"
git diff --check
```

Expected: every command exits `0`; contract, prop manifest, mesh stats, and other provider outputs are regenerated before complete inventory; complete inventory regeneration is the last tree-mutating command; every following validator/check is read-only; both SHA lines equal the approved digest; and the forbidden-diff check is empty. Independent review compares canonical/projection bytes, actual screenshots and observations, human receipt/review, profile availability/performance, generated-tree consistency repair, leak boundary, and all command exits; no Critical/Important finding remains.

- [ ] **Step 8: Commit final evidence, push, and open the one asset PR.** Reference only safe render paths/summaries in the PR body. Leave it open for Kirk.

```bash
git add README.md library/custom-dice/d20-lightning evidence/attack-die-3d \
  scripts/build_attack_die_contract.py scripts/test_build_attack_die_contract.py \
  scripts/test_build_mesh_stats.py scripts/test_build_web_asset_catalog.py \
  harness/models/synty/dice harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-complete-inventory.json
git commit -m 'asset: publish verified attack die contract and evidence'
git status --short
git push -u origin "$ASSET_BRANCH"
ASSET_PR_URL=$(gh pr create --repo "$ASSET_REPO" --head "$ASSET_BRANCH" --base main --title 'asset: publish verified lightning d20 attack contract' --body "Closes $ASSET_ISSUE_URL. Publishes the private verified provider contract and safe sidecar for rpg-project PR #217; no licensed binary changes are included. Kirk alone merges.

— asset-pipeline agent, on behalf of KirkDiggler")
ASSET_PR_NUMBER=${ASSET_PR_URL##*/}
```

Expected: one open asset PR, no staged files, no licensed binary diff. **Stop until Kirk merges it.** Then fetch `origin/main`, obtain the actual merge SHA from the PR, and prove it is an ancestor of `origin/main`; agents do not merge.

### Task 4: Verify the frozen web consumer against the merged provider

**Artifact:** The unchanged web concept passes the merged-provider graduation gate and remains open, ready for Kirk; production promotion is explicitly deferred.

**Files:** No planned tracked changes. Sync only into gitignored `public/models/synty/`; any required source fix invalidates the frozen tuple and blocks readiness.

**Interfaces:** Consume the exact merged `AttackDieRuntimeSidecar`; derive `WEB_EVIDENCE_SHA` and `WEB_BUILD_SHA` from its tuple and require them to equal web HEAD, a freshly recomputed frozen build manifest, and the accepted private Task 3 evidence tuple.

- [ ] **Step 1: Sync only the exact merged provider commit.** Stop unless PR state is merged, merge SHA exists, and it is on fetched `origin/main`:

```bash
git -C "$ASSET_ROOT" fetch origin main
ASSET_MERGE_SHA=$(gh pr view "$ASSET_PR_NUMBER" --repo "$ASSET_REPO" --json state,mergeCommit --jq 'select(.state=="MERGED")|.mergeCommit.oid')
test -n "$ASSET_MERGE_SHA"; git -C "$ASSET_ROOT" merge-base --is-ancestor "$ASSET_MERGE_SHA" origin/main
MERGED_ASSET_WT="$ASSET_ROOT/.worktrees/attack-die-merged-${ASSET_MERGE_SHA:0:12}"
git -C "$ASSET_ROOT" worktree add --detach "$MERGED_ASSET_WT" "$ASSET_MERGE_SHA"
rsync -a --delete "$MERGED_ASSET_WT/harness/models/synty/" "$WEB_WT/public/models/synty/"
SIDECAR="$WEB_WT/public/models/synty/dice/d20-lightning/attack-die-contract.json"
WEB_EVIDENCE_SHA=$(jq -er '.tuple.webCommit' "$SIDECAR")
test "$(git -C "$WEB_WT" rev-parse HEAD)" = "$WEB_EVIDENCE_SHA"
cmp -s "$SIDECAR" "$MERGED_ASSET_WT/harness/models/synty/dice/d20-lightning/attack-die-contract.json"
WEB_BUILD_SHA=$(jq -er '.tuple.webBuildSha256' "$SIDECAR")
test -n "$WEB_BUILD_SHA"; test "$WEB_BUILD_SHA" != null
```

- [ ] **Step 2: Run tests/gate first, then recreate the frozen build identity.** Run focused tests and `npm run ci-check` first. `ci-check` may create production-mode `dist`; intentionally discard that output by running the frozen Task 2 development build afterward. No build-producing command may run after the frozen build/manifest is created and before serving/evidence verification.

```bash
cd "$WEB_WT"
npm run test:run -- src/components/ui/dice/attackDieContract.test.ts src/components/ui/dice/attackDieMotion.test.ts src/components/ui/dice/attackDieRuntime.test.ts src/components/ui/dice/attackDieMaterial.test.ts src/components/ui/dice/AttackDie3D.test.tsx src/concepts/attack-die-3d/attackDieExperiment.test.ts src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx src/dev/AttackDiePerfHarness.test.tsx scripts/attack-die/frozenBuildManifest.test.ts src/components/ui/dice/DiceTray.test.tsx src/components/game/combatPresentation/CombatPresentation.test.tsx src/components/game/EncounterView.test.tsx
npm run ci-check
# Discard any production dist emitted by ci-check; recreate exact frozen dev dist.
export VITE_ATTACK_DIE_WEB_COMMIT="$WEB_EVIDENCE_SHA"
npm run attack-die:freeze-build -- --out /tmp/attack-die-merged-build-manifest.json
test "$(jq -er '.webBuildSha256' /tmp/attack-die-merged-build-manifest.json)" = "$WEB_BUILD_SHA"
test "$(jq -er '.webBuildSha256' "$MERGED_ASSET_WT/evidence/attack-die-3d/verification-run.json")" = "$WEB_BUILD_SHA"
git check-ignore -q public/models/synty/
test -z "$(git ls-files public/models/synty/)"
test -z "$(git diff --name-only "$WEB_BASE"...HEAD | grep -E '(^public/models/synty/|\.(glb|fbx|blend)$)' || true)"
test -z "$(git diff --name-only "$WEB_BASE"...HEAD -- src/components/game/combatPresentation/CombatPresentation.tsx src/components/ui/dice/DiceTray.tsx src/components/game/EncounterView.tsx)"
```

Expected: focused tests and `ci-check` exit `0`; the subsequent frozen development build/manifest exits `0` and matches both `WEB_BUILD_SHA` checks; the remaining commands are read-only, leak/protected-file checks print no paths, and no build-producing command runs before Task 4 serves `dist`.

- [ ] **Step 3: Re-run actual merged-provider evidence to private temporary output.** The sole asset PR is already merged, so create no second asset PR and no tracked post-merge evidence. Stop unless the merged provider's profile record shows all three categories available; a blocked/missing mobile/low-GPU profile prevents Task 4 completion. Start the exact frozen app with `BUILD_MANIFEST=/tmp/attack-die-merged-build-manifest.json`, the merged provider tree, and the reusable port-4173 lifecycle; write only to the declared untracked directory below.

```bash
MERGED_RECHECK=/tmp/attack-die-merged-provider-recheck
BUILD_MANIFEST=/tmp/attack-die-merged-build-manifest.json
rm -rf "$MERGED_RECHECK"; mkdir -p "$MERGED_RECHECK/evidence" "$MERGED_RECHECK/performance"
jq -e 'all(.profiles[]; .status=="available")' "$MERGED_ASSET_WT/evidence/attack-die-3d/performance/profiles.json"
# Start/ready the frozen preview with the reusable port-4173 lifecycle above.
npm run attack-die:evidence -- \
  --url 'http://127.0.0.1:4173/?concept=attack-die-3d' \
  --out "$MERGED_RECHECK/evidence" \
  --build-manifest /tmp/attack-die-merged-build-manifest.json \
  --all-results --animated --reduced-motion
npm run perf:attack-die -- --base-url http://127.0.0.1:4173 \
  --build-manifest /tmp/attack-die-merged-build-manifest.json \
  --profile-file "$MERGED_ASSET_WT/evidence/attack-die-3d/performance/profiles.json" \
  --out "$MERGED_RECHECK/performance/paired-results.json" \
  --samples-per-mode 20 --post-unmount-ms 8000
# Stop/clean the preview with the reusable lifecycle above.
```

Both commands must exit `0`; compare recheck tuple/core/GLB/build digests and results with tracked Task 3 evidence. Require results 1–20 animated/reduced, `<=0.25°`, exact-target holds, no healthy fallback, and all required profile budgets. In real Chromium force every failure and confirm no wrong-face flash, irreversible SVG, one semantic source, hidden Canvas, keyboard/focus/narrow-view operation, reduced-motion lightning suppression, repeated paths with identical endpoint, and hit/miss/natural-1/critical fixture displays. Outputs remain private under `/tmp`.

- [ ] **Step 4: Run the separate real repeated-FIFO normal-game-path playtest.** With the actual local API/game stack and normal `EncounterView` UI—not `AttackDiePerfHarness`, fixture callbacks, or direct callback invocation—drive repeated attacks through the real server/UI path. Create the private output directory, begin a timestamped observation log, and capture browser screenshots there:

```bash
FIFO_OUT=/tmp/attack-die-merged-provider-recheck/real-fifo
rm -rf "$FIFO_OUT"; mkdir -p "$FIFO_OUT"
cat >"$FIFO_OUT/observation-request.json" <<EOF
{"web":"$WEB_EVIDENCE_SHA","asset":"$ASSET_MERGE_SHA","build":"$WEB_BUILD_SHA","assertions":{"ordering":"PENDING","onceOnlyRelease":"PENDING","verdictImpactTiming":"PENDING","responsiveness":"PENDING"},"eventIds":[],"notes":"Human reviewer completes this after normal UI/server observation."}
EOF
# Human drives repeated normal-UI attacks, records authoritative event IDs and
# observations, replaces each PENDING with PASS or FAIL, and saves observation.json.
jq -e '.assertions.ordering=="PASS" and .assertions.onceOnlyRelease=="PASS" and .assertions.verdictImpactTiming=="PASS" and .assertions.responsiveness=="PASS" and (.eventIds|type=="array" and length>1) and (.notes|type=="string" and length>0)' "$FIFO_OUT/observation.json"
! grep -R -n '"FAIL"' "$FIFO_OUT/observation.json"
find "$FIFO_OUT" -maxdepth 1 -name '*.png' -type f -size +0c | grep -q .
```

This JSON is a human observation through the normal UI/server path, not auto-generated by `AttackDiePerfHarness` or an evidence driver. `ordering: PASS` means observed presentation order equals authoritative event order; `onceOnlyRelease: PASS` means FIFO never skips, reorders, stalls, or double-releases; `verdictImpactTiming: PASS` means miss release is at Verdict and hit result/damage release is at Impact; `responsiveness: PASS` means movement/keyboard/pointer controls remain responsive during/after repetitions. If the local path cannot be driven, any marker is absent/not PASS, any FAIL exists, event IDs are insufficient, or screenshots are absent, graduation stops.

- [ ] **Step 5: Reconcile reviews and leave the web PR ready.** Compare merged-sidecar runs with tracked Task 3 evidence on all required categories and budgets. Reconcile every new Copilot finding with validity/action/rationale and obtain a fresh independent gate over merged sidecar, web/build diff, evidence, real FIFO notes, performance, and command exits. A valid finding requiring tracked web changes makes `WEB_EVIDENCE_SHA` stale: stop and do not mark the PR ready; never conceal the change or add a second asset PR within this wave.

Confirm PR head equals local frozen HEAD, body still describes concept-only scope, state is OPEN, no staged files exist, and worktrees are clean except ignored synced web assets and declared `/tmp` recheck output. Record explicitly: production integration into `CombatPresentation`/`DiceTray` is a separate future issue, written design, plan, branch, and PR after Kirk accepts graduation; this plan creates none of them.

## Design coverage and execution order

- Summary, goals, product experience, data flow, component boundaries, failure,
  accessibility, and reduced motion map to Task 1 and the shared contracts.
- Architecture/ownership, material approach, concept stages, and the incremental
  appearance/calibrate/roll/verify path map to Tasks 1–2.
- Face-map/asset contract and private provenance map to Task 3.
- Performance, validation/evidence, testing/playtest (including normal-path
  repeated FIFO), graduation, and provider ordering map to Tasks 2–4.
- Non-goals, promotion criteria, and scope risks are enforced by Global
  Constraints and Task 4's separate-promotion handoff.

Reviewer corrections are fully mapped: Task 1 owns profile availability; Task 2 owns deterministic frozen-build identity and the reusable server lifecycle; Task 3 owns generated-output repair/order and candidate evidence; Task 4 owns merged-provider temporary recheck and normal-path FIFO.

Execute Tasks 1→4 in order. Stop rather than weaken the contract when provenance or human review is absent; a face is guessed/unreadable; any tuple member changes; a settle misses the threshold/exact hold; SVG can upgrade late or 3D can recover within a failed token; a wrong face flashes; accessibility duplicates authority; required performance/profile evidence is absent or over budget; private material leaks; provider is unmerged; or a frozen web change is required. Each task's commit is an independently reviewable checkpoint, but only Kirk merges.
