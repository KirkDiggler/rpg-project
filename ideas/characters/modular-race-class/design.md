---
name: Modular Race-Class Character Presentation
issue: https://github.com/KirkDiggler/rpg-project/issues/321
journey: https://github.com/KirkDiggler/rpg-project/issues/320
team: Assets
status: approved in conversation; pending written review
---

# Modular Race-Class Character Presentation

## Purpose

Make a character's public race participate in its class silhouette on the real
dungeon route, beginning with one fixed-look Elf Fighter. The first release
proves a reusable provider recipe and exact `race_ref + class_ref` client
resolution without introducing player-selectable customization, runtime modular
assembly, or backend work.

The expansion target is additional Elf class combinations produced from the
same modular rig. The first combination must therefore leave behind a recipe,
manifest contract, animation path, atlas seam, and rig-family socket—not a
manual GLB and a one-off client condition.

## Evidence and current reality

A disposable Blender spike against
`POLYGON_Modular_Fantasy_Hero_SourceFiles_v2.zip` established:

- 720 skinned modular parts and 720 static authoring copies;
- one combined FBX with all 720 meshes bound to a 63-bone armature;
- coordinated male outfit assemblies, including provisional Fighter 16,
  Barbarian 01, Monk 08, and Rogue 10 recipes;
- one shared 1024x1024 atlas layout with alternate source palettes and masks;
- a 13-mesh Elf Fighter at 3,750 triangles and about 725 KiB with its two
  animations embedded;
- successful transfer of the provider's accepted `Idle_Relaxed` and
  `Walk_Forward` clips through 48 mapped deform bones;
- clean sampled idle/walk renders for the Elf Fighter and a proportion-adjusted
  Dwarf Rogue; and
- successful export, re-import, and material rendering in Blender 5.0.1.

This proof is discovery output under `/tmp`; none of its GLBs, scripts, or
licensed source bytes are production artifacts.

Production already has the public data needed for exact resolution.
`PublicMemberInfo` carries `class_ref`, `race_ref`, and an intentionally empty
`Customization` shelf. `useSessionRoster` loads those facts once. The web
currently forwards only `classRef` to `HexEntity`, and
`resolveClassCharacterModelUrl` maps class alone to the four Townfolk aliases.
The first release needs no proto, API, or toolkit change.

The current weapon presentation resolves item identity separately but embeds
the Townfolk socket in `MainHandPresentation`. Modular Fantasy Hero uses the
same `Hand_R` semantic name but a different rest frame, so reusing the Townfolk
socket is an evidence question rather than an assumption.

## Approved decisions

1. **Provider recipe plus composite resolver.** No manual binary promotion and
   no runtime 720-part assembly.
2. **First exact combination: Elf Fighter.** One standing model with one fixed
   default look.
3. **Class-only downed fallback.** A downed Elf Fighter temporarily resolves to
   the current Fighter downed GLB.
4. **Color later.** The embedded atlas remains replaceable, but
   `Customization` stays empty and no palette selector or runtime recolor ships.
5. **One modular rig-family socket.** Reuse Townfolk only if visual evidence
   passes; otherwise calibrate one `modular-fantasy-hero-main-hand-v1` profile
   shared by every modular combination.
6. **Current animation source.** The provider's accepted Townfolk
   `Idle_Relaxed` and `Walk_Forward` clips are the pinned transfer source. Auto
   Rig Pro is not a production dependency.
7. **Fallbacks remain load-bearing.** Missing exact combinations use the
   current class model; missing/unloadable class models use `MediumHumanoid`.
8. **No separate Concept or Learn PR.** The accepted Blender spike already
   answers assembly and animation feasibility. Production begins with one
   provider Build and one web Build under journey #320.

## Provider architecture

### Declarative recipe

The private provider adds a versioned recipe at:

```text
scripts/configs/character-promotion/modular-race-class-v1.json
```

Its top-level contract records:

- schema/workflow version;
- source archive display name and SHA-256;
- combined-FBX archive member path;
- default-atlas archive member path and SHA-256;
- accepted animation-source provider path and SHA-256;
- rig family `modular-fantasy-hero-v1`;
- output root and manifest path; and
- combinations keyed by exact normalized race and class ids.

The initial `elf:fighter` entry records the selected mesh names for head, ears,
hair, torso, hips, paired upper/lower arms, hands, and legs. It also records the
output path and default palette key. It does not carry an absolute source-root
path, generated timestamps, web revisions, or future customization values.

A versioned provider command reads the recipe and an explicit private source
root, verifies archive/member/source hashes, selectively stages only the
combined FBX and atlas, assembles the selected meshes on the shared armature,
transfers the two accepted clips, normalizes the output, and writes the GLB plus
manifest. Raw FBX and texture staging remains ignored and disposable.

### Runtime output and manifest

The first output lives under:

```text
harness/models/synty/characters/race-class/elf-fighter.glb
harness/models/synty/characters/race-class/manifest.json
```

The generated manifest maps the exact key `elf:fighter` to:

- standing runtime path and SHA-256;
- rig family;
- default palette key and atlas dimensions;
- animation names;
- geometry/material/texture facts; and
- accepted socket profile id.

The manifest is provider authority and evidence; the web follows its existing
pattern of compiling reviewed manifest values into typed constants rather than
fetching private provider JSON at runtime.

### Output invariants

The provider gate requires:

- one 63-bone armature;
- exactly the 13 reviewed Elf Fighter meshes and no unused modular inventory;
- no importer `Icosphere` artifact;
- one embedded 1024x1024 PNG atlas;
- exactly `Idle_Relaxed` and `Walk_Forward` on standing output;
- world scale and forward orientation compatible with the shared
  `SYNTY_SCALE` and player-facing path, with no web correction matrix;
- finite bounds and mesh-budget reporting;
- byte-preserving atlas replacement through the existing embedded-image swap
  path; and
- a provider-accepted main-hand socket profile.

The modular multi-mesh rig receives a narrowly named structural profile rather
than weakening the existing Townfolk Root-wrapper checks. The validator proves
its top-level transform, armature, skins, mesh set, atlas, and clips directly.

### Atlas seam

A temporary alternate-palette run must replace the one embedded image while
preserving nodes, skins, geometry, and animation bytes. The alternate GLB is
private evidence only; A promotes the default palette alone.

This proves future color or texture selection can choose another reviewed atlas
without rebuilding the character. It does not decide whether future selection
uses named baked palettes or a runtime material, and it does not add fields to
`Customization`.

### Main-hand socket

Provider evidence first applies `townfolk-main-hand-v1` to the modular rig with
one representative normalized weapon through idle and walk. If grip and
orientation pass, the manifest reuses that profile. If they do not, the
provider calibrates exactly one `modular-fantasy-hero-main-hand-v1` profile
against `Hand_R` and verifies the same weapon through both clips.

The correction belongs to the rig profile. No race, class, combination, or item
receives its own transform.

## Web architecture

### Composite model resolution

Replace the URL-only class resolver with a typed player-model resolution:

```text
resolvePlayerCharacterModel(raceRefId, classRefId, isDowned)
  -> { url, rigFamily, source } | undefined
```

Resolution order is:

1. when standing, exact normalized `race:class` entry;
2. current normalized class entry;
3. `undefined`, preserving `MediumHumanoid` fallback.

The initial exact table contains only `elf:fighter`. Downed resolution skips the
standing-only exact entry and returns the existing Fighter downed Townfolk
model. Unknown races, empty refs, and malformed casing/whitespace never produce
a guessed asset path.

### Public race data flow

`SessionEncounterView` reads `ownRoster.raceRef` beside its existing
`classRef`. `SessionCanvas` receives `raceRefId` for the local player and reads
both refs from the roster for visible peers. `HexEntity` passes both refs to the
composite resolver.

No peer sheet read is introduced. Race and class remain public identity loaded
once from `GetRoster`, exactly as the existing character-presentation design
requires.

### Rig-family socket override

The existing item lookup continues to resolve authoritative equipment ref to
weapon URL. `HexEntity` uses the resolved model's rig family to select an
optional socket override before `ClassCharacterModel` mounts
`MainHandAttachmentSlot`:

- Townfolk and class fallback retain `TOWNFOLK_MAIN_HAND_SOCKET` unchanged;
- exact modular output uses the provider-accepted modular socket only if it is
  distinct; and
- downed class fallback resolves as Townfolk, so it cannot accidentally receive
  a modular socket.

This is a rig-family boundary, not gameplay logic. Missing bone or invalid
socket continues to degrade to the existing attachment status without breaking
the character model.

## Failure behavior

- Missing or unknown race/class combination: use the current class GLB.
- Missing or unknown class: use `MediumHumanoid`.
- Exact GLB load failure: the existing ErrorBoundary uses `MediumHumanoid`.
- Missing race in a roster row: class fallback; never assume Human.
- Missing/invalid modular socket: render the character and leave the weapon
  unattached with the existing diagnostic status.
- Failed source/hash/structure/atlas validation: write no provider runtime
  replacement.
- Failed candidate render: retain the last accepted provider bytes; do not
  publish the web mapping.
- Licensed source bytes, temporary extracts, and private source paths never
  enter `rpg-project` or the public web repository.

## Verification

### Provider

- focused recipe/schema tests with fixture archives;
- exact source/hash/member refusal tests;
- real Blender assembly and GLB re-import;
- structural readback of armature, selected mesh names, clips, texture, bounds,
  and absence of importer artifacts;
- idle/walk sampled renders;
- default-atlas and temporary alternate-atlas semantic comparison;
- main-hand socket renders across idle and walk;
- mesh stats and complete-provider inventory refresh; and
- full provider script suite once before review.

### Web

- resolver table tests for exact standing, class fallback, downed fallback,
  missing refs, normalization, and unknown combinations;
- local and visible-peer race propagation tests through SessionCanvas;
- rig-family socket override tests while preserving Townfolk defaults;
- GLB load/fallback behavior through the existing ErrorBoundary;
- full `npm run ci-check`; and
- real local session evidence showing an authoritative Elf Fighter idle,
  walking, and holding its authoritative main-hand weapon.

## Delivery and ceremony

The tracking shape is deliberately small:

1. this Decide slice and design PR under journey #320;
2. one `rpg-game-assets` Build issue/PR from `origin/main` for the reusable
   recipe, generated manifest/output, atlas proof, and socket evidence;
3. one `rpg-dnd5e-web` Build issue/PR from `origin/dev` for race propagation,
   composite resolution, socket override, and real-route evidence; and
4. the design PR merges after both implementation PRs and the integrated walk.

There is no issue per modular part, race, class, render, or palette. Each
implementation PR receives one final review round; there are no task-level
review loops or a second discovery project.

## A completion gate

A is complete when an Elf Fighter whose public roster row carries
`race_ref=elf` and `class_ref=fighter`:

- resolves the reviewed modular standing GLB on the production session route;
- plays `Idle_Relaxed` while stationary and `Walk_Forward` while moving;
- holds the authoritative main-hand weapon through the accepted rig socket;
- falls back to the current Fighter downed GLB when downed;
- remains recoverable through class and `MediumHumanoid` fallbacks; and
- demonstrates that its embedded atlas can be replaced without changing
  geometry or animation.

## Expansion to B

Additional Elf classes add recipe entries and provider manifest rows using the
same source archive, rig, clips, atlas family, normalization, and socket. The web
adds exact table entries but no new renderer or data flow. Each combination
still receives visual review because coordinated outfit selection and clipping
are art judgments, but B introduces no new architecture.

## Explicitly deferred

- player-selectable or persisted customization;
- fields in the `Customization` proto shelf;
- runtime palette shaders or modular-part assembly;
- a matching modular downed Elf, portrait, or character-creation preview;
- races beyond Elf and classes beyond the current four;
- arbitrary combinatorial outfit mixing;
- combat-specific animation families;
- Auto Rig Pro installation or pipeline dependency; and
- changes to rules, equipment authority, API orchestration, or toolkit data.
