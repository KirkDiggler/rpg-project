# Idle Animation Review Harness Design

**Tracking issue:** rpg-project#112

## Purpose

Add an `Idle Review` tab to the `rpg-dnd5e-web` Concepts Lab (`/concepts`) as an internal raw-asset QA bench. It reviews the synced runtime GLBs one class and one manifest-declared idle clip at a time. It is not a player-facing idle picker, production asset editor, or production animation policy change.

The first slice covers the current `rpg-dnd5e-web/public/models/synty/characters/manifest.json` contract fetched at the runtime URL `/models/synty/characters/manifest.json`:

| Class | Model | Ordered idle clips |
| --- | --- | --- |
| Fighter | `characters/fighter.glb` | `Idle_Relaxed`, `Idle_Stretch`, `Idle_Drinking` |
| Barbarian | `characters/barbarian.glb` | `Idle_Relaxed`, `Idle_ChinScratch`, `Idle_Drinking` |
| Monk | `characters/monk.glb` | `Idle_Relaxed`, `Idle_Meditative`, `Idle_Drinking` |
| Rogue | `characters/rogue.glb` | `Idle_Relaxed`, `Idle_CheckWatch`, `Idle_Drinking` |

The manifest has no explicit default-idle field. The bench must say `No manifest default declared`, rather than infer one from array order. It also reports whether the selected clip is the current production resolver selection, calculated only for display from the actual GLB clip names using `resolveIdleClipName` in `rpg-dnd5e-web`. The stage never calls that resolver to choose what to play. Production fallback behavior remains a later policy decision informed by review findings.

## Approach Decision

Use a dedicated bench rather than extending Class Selection or making a static evidence gallery.

- A `rpg-dnd5e-web` Class Selection extension would blur a raw QA workflow with a player-facing concept that currently uses hardcoded Monk guidance; it also encourages product-picker behavior and masks per-clip diagnostics.
- A static rendered gallery gives quick visual evidence but cannot verify exact playback, loop behavior, load failures, cameras, or recoverable reviewer state.
- A dedicated `Idle Review` tab keeps the task isolated from production encounter paths, supports active inspection against synced GLBs, and follows the established `rpg-dnd5e-web` Concepts Lab model of an isolated evaluation surface. This is the chosen approach.

The game-like view is representative only: its camera, scale, ground, and lighting approximate an encounter reading without recreating the full encounter scene.

## Architecture

`rpg-dnd5e-web/src/concepts/ConceptsView.tsx` registers `Idle Review` alongside the existing concept pages and mounts a self-contained `rpg-dnd5e-web/src/concepts/idle-review/IdleAnimationConcept`.

| Boundary | Responsibility |
| --- | --- |
| `IdleAnimationConcept` | Fetch-state orchestration, class/clip navigation, request identity coordination, draft revalidation, explanatory copy, review-state coordination, progress, reset action, and export trigger. |
| Validated catalog loader | Fetches the runtime manifest response as bytes, computes its SHA-256 with Web Crypto, parses those bytes, and validates them. It derives ordered class entries and their `idleClips` without a hidden hardcoded catalog or fallback, exposing per-class validation failures without discarding valid classes. |
| `IdleReviewStage` | Fetches the exact selected GLB once as bytes, computes its SHA-256, parses those same bytes with `GLTFLoader`, creates a skinned clone, finds and plays only the requested clip, controls playback and cameras, renders studio/game-like contexts, and reports actual GLB facts and request-scoped load status. |
| `IdleReviewForm` | Displays class, requested clip, production-resolver status, manifest-default status, the review checklist, verdict controls (`Keep`, `Fix`, `Reject`), and note input. It enables verdict input only for matching exact-clip readiness and requires a nonblank actionable note for `Fix` and `Reject`; a `Keep` note is optional. |
| Pure export utility | Produces deterministic, versioned review JSON from validated catalog identity, current target outcomes, diagnostics, and stored review state; it has no browser, Three.js, or localStorage dependency. |

`IdleReviewStage` reuses the production-proven `SkeletonUtils.clone` and `useAnimations` mechanics in `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.tsx`. Fetching bytes once and calling `GLTFLoader.parse` directly is a deliberate bench-specific seam, not a mechanic from that component: it is required to hash and render identical bytes. The stage intentionally does not use `useGLTF`, because the bench must preserve that auditable asset identity. It does not reuse `resolveIdleClipName` from `rpg-dnd5e-web/src/components/hex-grid/classCharacterModels.ts` for playback. The exact requested name must be present in the loaded GLB animation names; otherwise the stage is an error state. The stage releases actions and disposes stage-owned cloned resources when a request is superseded or unmounts; no object URL is needed for byte parsing, and any object URL introduced by an implementation must be revoked at the same lifecycle boundary.

## Data Flow And Review State

1. On tab entry, the `rpg-dnd5e-web` loader fetches `/models/synty/characters/manifest.json` with no alternate manifest, embedded catalog, or model fallback. It reads the exact response bytes once, computes `manifestSha256` with `crypto.subtle.digest('SHA-256', bytes)`, then parses those same bytes.
2. It validates `mapping` entries independently: a usable entry has a class key, non-empty `model`, and ordered non-empty `idleClips` strings. It derives the model URL by prefixing the manifest-relative model path with `/models/synty/`. An unusable entry produces an export-level catalog diagnostic because it cannot yield trustworthy target records.
3. The concept lists valid classes and their clips in manifest order. Each derivable target has immutable collision-safe `targetId = JSON.stringify([classKey, modelPath, clipName])`, produced from that validated ordered tuple; concatenated identity strings are not used. A malformed class is shown as failed and does not prevent review of other valid classes.
4. Every selection, including a return to the same `targetId`, receives a monotonically unique load/selection request ID. The stage receives both identities, fetches that exact GLB once as bytes, computes `modelSha256`, and passes those same bytes to `GLTFLoader.parse` before cloning. Stage readiness and error events carry both `targetId` and request ID; orchestration accepts an event only when both equal the active selection, so an older request can never win a rapid or repeated selection race.
5. Only after matching exact-clip lookup succeeds does the stage create an action, reset it to time zero, configure looping, and expose enabled review controls. It reports the requested clip name, full actual GLB clip-name list, selected-clip duration, model path, `manifestSha256`, `modelSha256`, and current load status. Verdict controls enable only from matching readiness.
6. A verdict and note are stored by `targetId` with the manifest and model hashes observed when it was confirmed. Changing class, clip, or visual context resets playback to time zero and applies that context's default camera preset. Playback speed, camera preset, zoom, and free orbit are ordinary inspection controls and do not reset playback. All changes preserve judgments and notes.
7. Draft state is saved to a versioned localStorage key after review-state changes and rehydrated only when its schema version is valid. On tab entry, after loading the stored draft and current manifest, the harness enters a visible transient `revalidating` state. It fetches and hashes every unique exact GLB referenced by persisted records before treating any prior verdict as reviewed, calculating final progress, or enabling export. During revalidation, those records do not count as reviewed and export is disabled with a reason that asset revalidation is in progress. For each persisted record, matching manifest and model hashes restore its verdict; a manifest or model mismatch becomes visible `STALE` and requires explicit reconfirmation; a model fetch, hash, or parse/load failure becomes blocked with the applicable diagnostic. The revalidation pass completes even when the manifest already differs, so no persisted model escapes verification. A stale verdict is visible, does not count as reviewed, is exported as stale, and is never silently attached to a changed manifest or GLB. `revalidating` is UI-only and never appears in an export because export remains disabled until the pass finishes. An explicit `Reset draft` action clears only this harness's stored state after confirmation; it never edits assets or manifest data.
8. After revalidation completes, export derives an outcome for every currently derivable target and gathers catalog diagnostics. Its persistent outcomes are only `reviewed`, `unreviewed`, `blocked`, or `stale`. It does not send data to a backend or GitHub.

Each target record contains `targetId`, its canonical `[classKey, modelPath, clipName]` identity tuple, `assetPath`, `class`, `clip`, `outcome` (`reviewed`, `unreviewed`, `blocked`, or `stale`), `manifestSha256`, and the available `modelSha256`. A reviewed record additionally contains `verdict`, `note`, and `timestamp`; every reviewed record has both hashes after startup revalidation. A blocked record contains the requested class/model/clip identity, available hashes, a machine-readable diagnostic code, a human message, and the actual GLB clip-name list when loading reached clip inspection. `catalogDiagnostics` records manifest-entry validation failures that cannot derive targets. The export envelope includes a fixed schema version, export timestamp, and a completion summary whose total, reviewed, unreviewed, blocked, stale, keep, fix, and reject counts are calculated from those records. Entries are ordered by manifest class order and then `idleClips` order so equal review state produces byte-stable JSON apart from its deliberate timestamps.

## Review Experience

The tab shows a class selector and an ordered clip selector, with visible progress for all 12 current review targets. The selected target explains:

- class, requested clip, and manifest model path;
- whether that clip is the production resolver selection for the actual GLB clip list, and `No manifest default declared` when applicable;
- the exact review checklist: personality/readability, anatomy and deformation, baked-weapon interaction, feet and ground contact, and loop quality;
- actual clip diagnostics and load status before review controls.

The form requires one verdict (`Keep`, `Fix`, or `Reject`). `Keep` accepts an optional note; `Fix` and `Reject` require a nonblank actionable note before save or export can classify the target as reviewed. A target counts as reviewed only when it has a valid verdict for the currently hashed manifest and model after startup revalidation completes. Failed targets count as blocked, stale targets count as stale, revalidating records count as neither, and none can receive a verdict until matching readiness is restored.

The visual stage provides both a neutral studio context and a representative game-like encounter context. Both render the same actual model and exact selected animation. The game-like context imports the production `SYNTY_SCALE` calibration rather than duplicating a scale literal. Its named calibration fixture records the current `HexGrid` seam: orthographic camera position `[8, 10, 8]`, zoom `80`, near `0.1`, far `1000`; XZ ground at `y=0`; and base ambient/directional lighting intensities `0.6` and `0.8`, with the directional light at `[10, 10, 5]`. It may render a review-appropriate visible ground treatment, but that treatment remains aligned to the production ground plane and does not claim to recreate the encounter. Controls provide play, pause, restart, 0.25x, 0.5x, and 1x speeds; fixed front, three-quarter, side, and top camera presets; zoom; and free orbit. Selecting a new clip/model or context resets playback to zero, stops the prior action, applies the selected context's default camera preset, and retains stored judgments. Changing speed or any camera/inspection control does not reset playback. Restart resets only the active action time to zero and resumes playback.

## Raw-Truth Failures

The harness must never silently substitute a class, model, clip, animation, or fallback pose. Its visible diagnostics distinguish loading from truth failures and name the requested values.

| Failure | Required behavior |
| --- | --- |
| Manifest request fails, bytes cannot hash, or JSON cannot parse | Show the request URL and browser, hash, or parse failure. No catalog is fabricated. |
| Manifest entry lacks a usable model or ordered idle clips | Mark that class unavailable with the field-level validation error; continue with valid classes and add a machine-readable catalog diagnostic to export. |
| GLB request, hash, or load fails | Show exact class, requested model URL, available hashes, and loader error. The derivable target exports as blocked; other classes remain reviewable. |
| GLB loads but requested clip is absent | Show `Requested clip <name> was not found in <model URL>` plus the full actual GLB clip-name list. Export the target as blocked, do not play another clip, and disable verdict controls. |
| Requested clip exists but cannot create/play an action | Show exact class, model, clip, available hashes, and action error. Export the target as blocked and disable verdict controls. |
| Persisted-record revalidation is in progress | Show that revalidation is checking all unique persisted model paths. Do not count stored verdicts as reviewed, and disable export until the pass reaches a final outcome for every persisted record. |
| Stale persisted verdict | Show the target identity and the persisted versus current manifest/model hash that differs. Export as stale, do not count it reviewed, and require explicit reconfirmation. |

Actual GLB clips and duration are diagnostics from the loaded asset, not claims copied from the manifest. This intentionally diverges from production `resolveIdleClipName`, whose fallback behavior is useful to the game renderer but invalid for an evidence bench.

## Non-Goals

- Editing manifests, GLBs, clip order, defaults, or production resolver behavior.
- Retargeting animations, modifying weapons, or invoking asset-pipeline tools.
- A character-creation idle picker or any player-facing control.
- Backend calls, authentication, persistence beyond browser localStorage, or GitHub writes.
- Full encounter recreation in the game-like view.

## Tests And Verification

Unit tests cover manifest byte hashing and parsing; independent per-class manifest failures and catalog diagnostics; canonical tuple `targetId` generation, including delimiter-like tuple values that prove distinct tuples cannot collide; exact requested-clip lookup, including an absent clip as an error rather than a substitute; model byte hashing before parsing; stale rehydration when either manifest or model hash changes; startup revalidation of every unique GLB referenced by persisted records; review-state persistence and localStorage recovery; required actionable notes for Fix/Reject and optional Keep notes; progress and explicit reset; and deterministic export ordering/schema. Export tests cover every derivable target as reviewed, unreviewed, blocked, or stale; blocked diagnostic shape and available hashes; reviewed hash presence after revalidation; disabled export and zero reviewed count while revalidation is active; and summary counts aligned with records.

Component tests mock the WebGL boundary as appropriate and verify tab registration, loading/error states, disabled verdicts until the exact clip is available for the active `targetId` and request ID, verdict/note retention across selection changes, playback control state, camera preset changes, and export invocation. They simulate rapid and out-of-order selections, including a return to the same target, and prove that stale readiness/error events cannot replace the active stage state. They also rehydrate stored verdicts and prove that progress/export remain disabled through revalidation, then distinguish matching hashes (restored verdict), changed hashes (stale), and failed model fetch/load (blocked).

Browser verification in `rpg-dnd5e-web` uses the synced real assets and confirms all 12 manifest targets: each requested clip loads from hashed bytes and advances during playback; studio and representative game-like contexts render; the game-like model scale and ground reading match the production calibration seam; all four camera presets, zoom, and free orbit work; playback reset behavior is predictable; judgments survive refresh; and an immediate post-refresh export remains disabled while every persisted model is revalidated. The browser check covers matching hashes restoring verdicts, changed manifest/model bytes producing visible stale records requiring reconfirmation, and failed model fetch/load producing blocked diagnostics without allowing export first. After revalidation, export contains the required target records, catalog diagnostics, hashes, and aligned summary; diagnostics expose the actual asset facts; class-specific failures do not block other classes; and the browser console has no unexpected errors.

## Acceptance Criteria

- `rpg-dnd5e-web` Concepts Lab has an isolated `Idle Review` tab registered through `src/concepts/ConceptsView.tsx`.
- The `rpg-dnd5e-web` runtime manifest is the sole catalog source, and its four current classes and 12 ordered idles are derived rather than hidden in code.
- Every review displays requested clip, actual GLB clips, duration, model path, load status, production-resolver status, manifest/model SHA-256 values when available, and the honest absence of a manifest default.
- The stage hashes and parses the exact manifest and model response bytes, plays only the exact requested animation on a correctly cloned skinned model, and releases stage-owned resources on replacement/unmount; a missing clip produces an actionable blocked failure and cannot receive a verdict.
- Selection events are scoped by immutable target ID and monotonically unique request ID, so stale asynchronous loads cannot enable controls or replace the active stage.
- Target IDs use canonical structured encoding of the validated class/model/clip tuple, so delimiter-like values cannot collide; request IDs remain separate selection instances.
- Reviewers can inspect each target in both visual contexts using the specified playback and camera controls, record Keep/Fix/Reject with a required actionable note for Fix/Reject, see progress, recover drafts, explicitly reset, and export versioned JSON.
- Changed manifest or model bytes make persisted verdicts visibly stale, not reviewed, and reconfirmation-only; they are never silently attached to changed assets.
- On refresh, every unique GLB referenced by persisted records is fetched and hashed before any persisted verdict can count as reviewed, influence final progress, or enable export; the visible transient revalidating state disables export until final reviewed/unreviewed/blocked/stale outcomes exist.
- Export includes every derivable target with a reviewed/unreviewed/blocked/stale outcome, hashes and diagnostics appropriate to its state, catalog diagnostics for non-derivable manifest failures, and summary counts aligned with records.
- The representative game-like context imports production character scale and uses the named production-derived camera, ground, and lighting calibration fixture without claiming full encounter recreation.
- No production assets, manifest values, defaults, backend data, or GitHub records are changed by the harness.
