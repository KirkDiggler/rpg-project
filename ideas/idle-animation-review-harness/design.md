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
| `IdleAnimationConcept` | Fetch-state orchestration, class/clip navigation, explanatory copy, review-state coordination, progress, reset action, and export trigger. |
| Validated catalog loader | Fetches and validates the runtime manifest; derives the ordered class entries and their `idleClips` without a hidden hardcoded catalog or fallback. It exposes per-class validation failures without discarding valid classes. |
| `IdleReviewStage` | Loads the selected GLB, creates a skinned clone, finds and plays only the requested clip, controls playback and cameras, renders studio/game-like contexts, and reports actual GLB clip names, duration, resolved model path, and load status. |
| `IdleReviewForm` | Displays class, requested clip, production-resolver status, manifest-default status, the review checklist, verdict controls (`Keep`, `Fix`, `Reject`), and note input. It disables verdict input unless the exact requested animation is visibly loaded and playing or paused. |
| Pure export utility | Produces deterministic, versioned review JSON from validated catalog identity and stored review state; it has no browser, Three.js, or localStorage dependency. |

`IdleReviewStage` reuses the production-proven mechanics in `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.tsx`: `useGLTF`, `SkeletonUtils.clone` for skinned meshes, and `useAnimations` bound to the stable clone. It does not reuse `resolveIdleClipName` from `rpg-dnd5e-web/src/components/hex-grid/classCharacterModels.ts` for playback. The exact requested name must be present in the loaded GLB animation names; otherwise the stage is an error state.

## Data Flow And Review State

1. On tab entry, the `rpg-dnd5e-web` loader fetches `/models/synty/characters/manifest.json` with no alternate manifest, embedded catalog, or model fallback.
2. It validates `mapping` entries independently: a usable entry has a class key, non-empty `model`, and ordered non-empty `idleClips` strings. It derives the model URL by prefixing the manifest-relative model path with `/models/synty/`.
3. The concept lists valid classes and their clips in manifest order. A malformed class is shown as failed and does not prevent review of other valid classes.
4. Selecting a class and clip sends that exact class, model path, and clip name to the stage. The stage loads that exact GLB, clones it, inspects its animation clips, and looks up the requested name by exact string equality.
5. Only after that lookup succeeds does the stage create an action, reset it to time zero, configure looping, and expose enabled review controls. It reports the requested clip name, full actual GLB clip-name list, selected-clip duration, model path, and current load status.
6. A verdict and note are stored by the stable `classKey + modelPath + clipName` review identity. Changing class, clip, or visual context resets playback to time zero and applies that context's default camera preset. Playback speed, camera preset, zoom, and free orbit are ordinary inspection controls and do not reset playback. All changes preserve judgments and notes.
7. Draft state is saved to a versioned localStorage key after review-state changes and rehydrated only when its schema version is valid. An explicit `Reset draft` action clears only this harness's stored state after confirmation; it never edits assets or manifest data.
8. Export gathers the current stored judgments into a JSON download. It does not send data to a backend or GitHub.

Each review record contains `assetPath`, `class`, `clip`, `verdict`, `note`, and `timestamp`. The export envelope includes a fixed schema version, export timestamp, and a completion summary with total, reviewed, unreviewed, keep, fix, reject, and blocked counts. Entries are ordered by manifest class order and then `idleClips` order so equal review state produces byte-stable JSON apart from its deliberate timestamps.

## Review Experience

The tab shows a class selector and an ordered clip selector, with visible progress for all 12 current review targets. The selected target explains:

- class, requested clip, and manifest model path;
- whether that clip is the production resolver selection for the actual GLB clip list, and `No manifest default declared` when applicable;
- the exact review checklist: personality/readability, anatomy and deformation, baked-weapon interaction, feet and ground contact, and loop quality;
- actual clip diagnostics and load status before review controls.

The form requires one verdict (`Keep`, `Fix`, or `Reject`) and accepts an optional reviewer note. A target counts as reviewed only when it has a verdict. Failed targets count as blocked, not reviewed, and cannot receive a verdict.

The visual stage provides both a neutral studio context and a representative game-like encounter context. Both render the same actual model and exact selected animation. Controls provide play, pause, restart, 0.25x, 0.5x, and 1x speeds; fixed front, three-quarter, side, and top camera presets; zoom; and free orbit. Selecting a new clip/model or context resets playback to zero, stops the prior action, applies the selected context's default camera preset, and retains stored judgments. Changing speed or any camera/inspection control does not reset playback. Restart resets only the active action time to zero and resumes playback.

## Raw-Truth Failures

The harness must never silently substitute a class, model, clip, animation, or fallback pose. Its visible diagnostics distinguish loading from truth failures and name the requested values.

| Failure | Required behavior |
| --- | --- |
| Manifest request fails or JSON cannot parse | Show the request URL and browser error or parse failure. No catalog is fabricated. |
| Manifest entry lacks a usable model or ordered idle clips | Mark that class unavailable with the field-level validation error; continue with valid classes. |
| GLB request/load fails | Show exact class, requested model URL, and loader error. Other classes remain reviewable. |
| GLB loads but requested clip is absent | Show `Requested clip <name> was not found in <model URL>` plus the full actual GLB clip-name list. Do not play another clip and disable verdict controls. |
| Requested clip exists but cannot create/play an action | Show exact class, model, clip, and action error; disable verdict controls. |

Actual GLB clips and duration are diagnostics from the loaded asset, not claims copied from the manifest. This intentionally diverges from production `resolveIdleClipName`, whose fallback behavior is useful to the game renderer but invalid for an evidence bench.

## Non-Goals

- Editing manifests, GLBs, clip order, defaults, or production resolver behavior.
- Retargeting animations, modifying weapons, or invoking asset-pipeline tools.
- A character-creation idle picker or any player-facing control.
- Backend calls, authentication, persistence beyond browser localStorage, or GitHub writes.
- Full encounter recreation in the game-like view.

## Tests And Verification

Unit tests cover manifest parsing and validation, including independent per-class failures; exact requested-clip lookup, including an absent clip as an error rather than a substitute; review-state persistence and localStorage recovery; progress and explicit reset; and deterministic export ordering/schema.

Component tests mock the WebGL boundary as appropriate and verify tab registration, loading/error states, disabled verdicts until the exact clip is available, verdict/note retention across selection changes, playback control state, camera preset changes, and export invocation.

Browser verification in `rpg-dnd5e-web` uses the synced real assets and confirms all 12 manifest targets: each requested clip loads and advances during playback; studio and representative game-like contexts render; all four camera presets, zoom, and free orbit work; playback reset behavior is predictable; judgments survive refresh; export contains the required records and summary; diagnostics expose the actual asset facts; class-specific failures do not block other classes; and the browser console has no unexpected errors.

## Acceptance Criteria

- `rpg-dnd5e-web` Concepts Lab has an isolated `Idle Review` tab registered through `src/concepts/ConceptsView.tsx`.
- The `rpg-dnd5e-web` runtime manifest is the sole catalog source, and its four current classes and 12 ordered idles are derived rather than hidden in code.
- Every review displays requested clip, actual GLB clips, duration, model path, load status, production-resolver status, and the honest absence of a manifest default.
- The stage plays only the exact requested animation on a correctly cloned skinned model; a missing clip produces an actionable failure and cannot receive a verdict.
- Reviewers can inspect each target in both visual contexts using the specified playback and camera controls, record Keep/Fix/Reject plus a note, see progress, recover drafts, explicitly reset, and export versioned JSON.
- No production assets, manifest values, defaults, backend data, or GitHub records are changed by the harness.
