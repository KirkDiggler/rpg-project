# Named roles for authored assets — one part, one word

**Status:** DESIGN 2026-09-18, for review. Tracking issue: rpg-project#470.
**First consumer:** authored doors (rpg-project#467 / PR #468), via
rpg-dnd5e-web#1117.

## The one sentence

An authored asset declares the semantic roles of its named parts, so a
consumer can move or grow one part without knowing the file, and no code
names `Door_Left` or `Door_Wall_Above`.

## Why now — two requirements blocked on the same missing thing

The approved door, `dnd5e:env:dark-fortress:wall_door_double_01`, is four
named meshes under one `AuthoredCalibration` wrapper: `Door_Frame`,
`Door_Left`, `Door_Right`, `Door_Wall_Above`. Two requirements need them,
and they are not the door design's to solve:

1. **The leaves swing** on open, each about its own hinge. (PR #468.)
2. **The upper section absorbs wall height**, leaving the door opening at
   its authored size. Kirk, 2026-09-18: *"in the world builder I can set
   the height. that new door has a section at the top that would be used to
   increase, leaving the door size alone."*

Requirement 2 is **not** satisfiable today, and the reason is worth stating
precisely because it looks like it already works. `WorldAssetModel.tsx:37-59`
clones the whole scene and applies ONE group scale:

```tsx
const cloned = useMemo(() => scene.clone(true), [scene]);
...
scale={[SYNTY_SCALE, SYNTY_SCALE * heightScale, SYNTY_SCALE]}
```

So a door's `heightScale` grows the **frame and the leaves** along with
everything else. The control's own copy is right for a wall and wrong for a
door: *"Grounded at each piece base; width, spacing, authored position stay
unchanged."* Raising a wall should stretch it; raising a door should grow
only the masonry above it.

## What already exists (verified 2026-09-18)

| Piece | State | Where |
|---|---|---|
| Four named meshes in the shipped GLB | EXISTS | node names under the `AuthoredCalibration` wrapper |
| Capture records the node list | EXISTS | `scripts/authored_world_asset_source.py` (`inspect_authored_glb`), node list in the capture receipt |
| Promotion records the named pivots | EXISTS | `scripts/normalize_authored_world_asset.py`, `scripts/promote_world_assets.py` → `preservationEvidence.namedPivots` |
| A semantic role map for env pieces | **EXISTS — and it is the pattern to copy** | `library/polygon-dungeon/env-role-map.json`, merged by `scripts/build_env_manifest.py` |
| Parts in the provider catalog | **NOTHING** | `harness/catalogs/synty-world-assets.json` carries 9 keys: `boundsMeters, category, displayName, file, glbSha256, ref, sizeBytes, supportsDecoration, tags` |
| Parts in the web catalog | **NOTHING** | `rpg-dnd5e-web/scripts/generate-world-asset-catalog.mjs` enforces exactly those 9 as `ASSET_KEYS`; `GeneratedWorldAsset` mirrors them |
| A consumer that can address a part | **NOTHING** | `WorldAssetModel.tsx` is `scene.clone(true)`; no node lookup anywhere |

The precedent is the important row. This repo **already** has a
hand-maintained answer to "which converted piece plays which semantic role":
`env-role-map.json` declares a `$schemaVersion` and a per-role `{fit, notes,
pieces}`, and `build_env_manifest.py` merges it against the GLBs actually
present and **fails loudly** when a role names a file that is not there. Env
pieces have this. Authored world assets do not.

This design does not invent a mechanism. It applies the one already here to
the authored world-asset pipeline, and it takes that pipeline's own rule —
`env-role-map.json` is *"the hand-maintained source of truth … the pipeline
never touches it, so it survives re-conversion"* — as the shape of the
declaration.

## The contract

A **hand-maintained, reviewed role assignment per authored asset**, declared
where the asset's other reviewed facts already live (the promotion recipe,
beside `calibration yawDegrees`), emitted into the catalog, and validated
against the GLB.

The catalog entry gains one optional key:

```jsonc
"roles": [
  { "role": "frame", "node": "Door_Frame" },
  { "role": "leaf",  "node": "Door_Left" },
  { "role": "leaf",  "node": "Door_Right" },
  { "role": "above", "node": "Door_Wall_Above" }
]
```

A **list**, not a map, because `leaf` legitimately appears more than once and
order is part of what the reviewer wrote.

### The vocabulary is sealed, and grows one word per use case

| Word | What it means | Consumer may |
|---|---|---|
| `frame` | the fixed surround | leave it exactly where the asset put it |
| `leaf` | a moving panel; may appear more than once | move it, hinged per its own geometry |
| `above` | masonry above the opening that absorbs height | grow it to take the height, and nothing else |

Named and **not built**: `below`, `inner`, `handle`, `chain`. They keep their
refusal until a use case pays for them — the shenanigans vocabulary's rule.

**Which leaf is which is deliberately NOT in the vocabulary.** For a double
door the two leaves hinge on opposite sides, and the consumer must know that
— but it must derive it from each leaf's own geometry and position within the
asset, which is what PR #468 already requires (*"derives each leaf's rest
local transform, hinge pivot, and axis from that instance's actual
hierarchy"*). A `leaf-left`/`leaf-right` word would encode one asset's
orientation as if it were universal, and the second door asset with its
hinges the other way would need a new word.

### Names, never numbers

The catalog carries **node names and semantic roles**. It carries no pivots,
no transforms, no rest poses and no angles. The measured pivots stay where
they are — pipeline evidence in the promotion receipts — and the runtime
derives pose from the loaded hierarchy.

This is not tidiness. A copied number is a number that silently disagrees
with the file after the next re-export, and nothing would catch it; a name
that no longer resolves is a loud failure. PR #468 states the same law for
the authored binding (*"no numeric GLB transform is copied into YAML, proto,
or a second registry"*), and this contract holds it one layer down.

### Absence is a single-part asset

No `roles` key means exactly what every asset in the catalog means today: one
whole object, cloned and scaled as one. Every existing entry keeps its bytes
and its behaviour. Absent is not a gap to be filled later — it is the truth
about a single-mesh asset.

## Validation, and failing loudly

- **At promotion** the role assignment is checked against the GLB actually
  being promoted: a role naming a node the file does not contain is a
  failure that stops the promotion, naming the asset, the role and the node.
  This is `build_env_manifest.py`'s existing rule — *"a role is never allowed
  to point at a file"* that is not present — applied to the same problem.
- **At generation** the web catalog generator refuses a malformed `roles`
  shape, an empty role list, a duplicate node, and a role word outside the
  sealed vocabulary. `ASSET_KEYS` and `GeneratedWorldAsset` gain the one key.
- **At render time** a declared node that is absent from the loaded GLB is a
  **named failure**, never a silent fallback to the whole object — PR #468's
  rule for the binding, held here for the roles.

## What this makes possible next

- The door's two requirements, with no door-specific code: swing every
  `leaf`, grow only `above`.
- The next multi-part asset — a portcullis, a gate, a drawbridge, a door with
  a transom — declares its parts and gets the same behaviour, with no
  renderer change.
- A consumer that has never heard of a door can still ask "what are this
  asset's parts, and which one grows?" That is the tool worth having.

## Non-goals

- **No door semantics.** State, obstruction, interaction, the offered verbs,
  and the authored binding are rpg-project#467 / PR #468. This contract says
  nothing about them.
- **No material, UV or texture behaviour.** The role word `above` says a part
  grows; it says nothing about how it is shaded.
- **No renderer knowledge of node names.** If the renderer ever contains
  `Door_Wall_Above`, this contract has been bypassed.
- **No numeric geometry in the catalog** (§ names, never numbers).
- **No general scene-graph exposure.** Only the reviewed roles, not every
  node — the asset stays one object with named handles, not a hierarchy the
  consumer wanders.

## Ownership

| Owner | Narrow responsibility |
|---|---|
| `rpg-game-assets` | the hand-maintained role assignment; merge it into the catalog; validate node names against the GLB and fail loudly |
| `rpg-dnd5e-web` | accept `roles` in `ASSET_KEYS` and `GeneratedWorldAsset`; resolve roles to nodes per instance; grow `above`, swing `leaf`; name a missing node |
| `rpg-project` | this contract |

## Done when

- The door asset's catalog entry names its four roles, and its two
  requirements are met by consuming them: leaves swing on their own derived
  hinges, and `above` grows while the opening keeps its authored size.
- An asset with no `roles` renders and behaves exactly as it does today.
- A role naming a node the GLB does not contain fails the promotion, naming
  the asset, role and node.
- No file in `rpg-dnd5e-web` names `Door_Frame`, `Door_Left`, `Door_Right` or
  `Door_Wall_Above`. That absence is the deliverable.
- Two independent instances of the same asset behave independently.
- Every claim above is a test; the browser is the walk.

## Residual risk, named

Two, both deliberate:

- **The vocabulary is judgement.** `frame`/`leaf`/`above` are words chosen
  against one asset. A second multi-part asset is the first real test of
  whether they hold, and adding a word is cheap by design — but if the second
  asset needs a *structural* word this table cannot express, that is evidence
  this shape was too narrow, and it should be said plainly rather than
  routed around.
- **`roles` is new catalog surface**, so it is a contract the provider and
  the consumer must agree on in the same wave. It is additive and optional,
  which is why it can land before either feature needs it.