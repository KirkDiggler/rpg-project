# Class outfit colors — production design

## Status

Approved by Kirk on 2026-09-03. Tracked by Journey
[rpg-project#362](https://github.com/KirkDiggler/rpg-project/issues/362).
Implementation has not started.

## Goal

Every supported hero can choose primary and secondary colors for the fixed
default outfit of Barbarian, Fighter, Monk, or Rogue during character creation,
reload and finalize that character, and render the same chosen class style for
the owner and peers in the normal dungeon session.

This wave customizes **class style**, not mechanical armor. Barbarian and Monk
are not necessarily wearing armor, and the current character bodies do not swap
geometry when equipment changes. Actual leather-armor and chain-mail models,
item-instance dyes, and finalized-character equipment editing are later work.

## Current facts

- The production profile matrix has eight races and four exact class outfits:
  Barbarian 01, Fighter 16, Monk 08, and Rogue 10.
- Each production body uses the Modular Fantasy Hero 1024×1024 palette `01-A`.
  Identity and outfit parts remain named skinned meshes, but one textured
  `MeshStandardMaterial` family serves the complete body.
- Hair is already separate skinned geometry with a uniform material. Its shared
  color and roughness treatment cannot color selected regions of the textured
  body atlas.
- Synty's licensed source includes
  `PolygonFantasyHero_Texture_Mask_01.png`, aligned with
  `PolygonFantasyHero_Texture_01_A.png`. It identifies replaceable palette
  regions and supplies useful source authority for deriving two explicit dye
  channels.
- Existing production body GLBs include immutable Dwarf v1 files. A color
  system must not rewrite those files or any other accepted body output.
- Character `Appearance` and public session `Customization` currently carry
  hair. The API currently owns a parallel Appearance entity, validates its
  contents, stores it beside toolkit data, and assembles public customization
  directly in `GetRoster`.
- Toolkit `character.Data` and `DraftData` currently carry no Appearance. The
  session SDK preserves API-owned fields only because the API wraps toolkit
  data around every SDK save.
- The four deprecated string colors on the character Appearance proto are
  intentionally inert. The API accepts and drops them. They describe the old
  marker-color renderer and are not an authority to reactivate.
- Redis character and draft state is ephemeral. No permanent player character
  migration is required; after the version transition, characters may be
  recreated from the standard seed/cast.

## Decisions

1. **Character-owned class style.** Outfit colors live with the character like
   hair. They are not attached to equipment in this wave.
2. **Two optional channels.** Primary and secondary are independently optional
   packed RGB24 values. Missing means preserve the provider's authored default;
   explicit black is a real override.
3. **Semantic dye regions.** Primary controls dominant dyeable cloth or leather
   panels. Secondary controls trim and accent panels. Skin, hair, fur, wood,
   and exposed metal preserve their authored colors.
4. **Class changes retain colors.** Changing class during creation applies the
   same selected colors to the new class's fixed outfit rather than resetting
   them.
5. **Provider-owned masks.** Licensed source evidence is normalized into four
   class-specific two-channel masks. The web does not infer dye regions from
   source pixels, mesh names, or class enums.
6. **No body rewrite.** Existing body GLBs and manifests remain byte-identical.
   Outfit treatment is additive sidecar authority.
7. **Blender first, without a durable Concept.** Kirk reviews editable mappings
   in a private Blender scene before provider publication. The payload and
   production route are sufficiently understood that no `/concepts` page is
   needed.
8. **Toolkit owns semantics.** The complete neutral Appearance model, including
   existing hair and new outfit colors, moves into toolkit ownership. The API
   is a transport, authentication, repository-host, and conversion boundary;
   it never interprets customization contents at runtime.
9. **One whole Appearance write.** Creation keeps the existing
   response-authoritative Apply/Cancel behavior. Finalized sheets remain
   read-only.
10. **Keep the type specific.** `OutfitCustomization` is reusable by a future
    item cosmetic envelope, but this wave does not generalize recoloring across
    props, obstacles, weapons, or arbitrary entities.

## Material approaches

### A. Provider mask with a standard-material shader extension — selected

The provider publishes normalized masks. The web extends per-character
`MeshStandardMaterial` instances at shader compilation, samples the shared
mask, and substitutes only selected dye channels before the rest of the
standard PBR lighting pipeline runs.

This preserves the original texture, skinning, animation, lighting, and entity
overlays. Color changes mutate uniforms in place. The body, Skeleton, mixer,
material, and texture identities remain stable.

### B. Rebuild the atlas per character

A client could copy and recolor a 1024×1024 atlas for each character. This keeps
stock materials but adds CPU work, one large GPU texture per customized
character, asynchronous update complexity, and texture lifecycle risk. It is
rejected.

### C. Split every body into separate materials

Provider exports could assign primary and secondary materials directly. This
adds draw calls, multiplies the release matrix, and rewrites accepted body
bytes, including immutable Dwarf v1. It is rejected.

## Architecture

```text
licensed atlas + source mask
        |
        v
provider class-mask curation ------> outfit-treatment manifest
        |                                      |
        | Blender approval                     | generated catalog
        v                                      v
four fixed outfit mappings              web material treatment

Appearance proto
        |
        v
API shape-only converter
        |
        v
toolkit Draft.SetAppearance
        |
        v
toolkit DraftData -> Character Data
        |
        v
API repository round-trips toolkit persistence data
        |
        v
toolkit session Roster reads public character identity
        |
        v
API shape-only converter -> public roster proto -> web renderer
```

The provider and toolkit are independent authorities. The provider decides how
an outfit looks; toolkit decides whether neutral customization intent is valid
and how it survives game state. Neither imports or duplicates the other.

## Provider and Blender contract

The provider adds an outfit-treatment root without modifying existing body
roots:

```text
harness/models/synty/characters/outfit-customization/v1/
  manifest.json
  masks/
    barbarian-01.png
    fighter-16.png
    monk-08.png
    rogue-10.png
```

Each runtime mask is a 1024×1024 image aligned to the embedded `01-A` atlas:

- red is the primary dye weight;
- green is the secondary dye weight;
- black preserves the original atlas;
- primary and secondary weights do not overlap; and
- unused channels have one canonical zero value.

The manifest names schema/workflow versions, exact class and outfit refs, mask
paths and SHA-256 hashes, original primary/secondary display defaults, atlas
identity, and the exact body/profile authority against which compatibility was
proved. Paths are provider-relative and never licensed source paths.

The provider validates mask dimensions, channel encoding, overlap, non-empty
coverage for both channels, exact inventory, deterministic bytes, and the
material/UV facts required by every corresponding race body. Treatment is
limited to provider-declared outfit mesh nodes; identity meshes are not guessed
by the web. Structural checks establish that race scaling did not alter the
outfit UV contract, avoiding 32 redundant visual copies.

The private Blender review scene has four Outliner collections:

- `Barbarian_01`
- `Fighter_16`
- `Monk_08`
- `Rogue_10`

Each contains clearly named `Original` and `Editable` variants. Primary and
secondary are exposed as material inputs and begin with deliberately loud
diagnostic colors. The viewport contains no labels or floating text; Kirk uses
the Outliner and ordinary orbit/material controls. Licensed source meshes,
textures, and the `.blend` remain outside public repositories. Provider
publication stops until Kirk approves all four mappings.

Promotion is fail-closed and atomic. A missing file, extra file, unexpected
channel, stale authority hash, nondeterministic output, symlink, traversal, or
body-byte change refuses publication. Existing GLBs and immutable Dwarf files
are byte-compared before and after.

## Wire contract

The provider-neutral customization package adds:

```proto
message OutfitCustomization {
  // Packed 0xRRGGBB. Absence preserves the provider's authored primary.
  optional uint32 primary_color_srgb = 1;
  // Packed 0xRRGGBB. Absence preserves the provider's authored secondary.
  optional uint32 secondary_color_srgb = 2;
}
```

Character creation adds `outfit` beside `hair` on `Appearance`; the public
session shelf adds the same `outfit` beside `hair` on `Customization`. New field
numbers are used. Deprecated `skin_tone`, `primary_color`, `secondary_color`,
and `eye_color` strings remain deprecated and inert.

Message absence and individual scalar absence both preserve provider defaults.
An empty present `OutfitCustomization` therefore has the same rendering intent
as an absent message. Present zero is black and must survive conversion and
storage.

The wire contains no class IDs, mask paths, shader modes, material names,
provider defaults, or asset URLs. Race and class identity already travel
through their existing fields; the web joins those facts against the pinned
provider catalog.

## Toolkit ownership

A low-dependency D&D customization package owns provider-neutral values and
validation so character creation, character persistence, session projection,
and a future item package can share one meaning without import cycles.

Toolkit owns:

- `Appearance`;
- `HairCustomization` and `StyleSelection` with their existing semantics;
- `OutfitCustomization`;
- RGB24, finite roughness, selection-shape, and bounded opaque-ref validation;
- deep-copy/presence behavior;
- `Draft.SetAppearance`;
- Appearance fields in `DraftData` and character `Data`;
- class-change preservation;
- draft-to-character finalization;
- strict load and `ToData` round trips; and
- the session SDK's public roster result and customization projection.

The toolkit does not query the provider catalog, validate that a style ref is
available for a race, choose visual defaults, construct URLs, or know shader
and mask details. A well-formed unknown style ref may persist and later fail
closed in provider lookup.

The session package adds an ID-based roster read. It loads current session
membership and character data through its existing host repositories, verifies
that the supplied authenticated player identity is seated, and returns
session-owned flat public-member values. API-owned duplicate roster state and
direct public-customization assembly retire once this SDK path is live.

This is a deliberate correction of the production-hair ownership decision.
Existing hair behavior does not change, but its runtime and persistence owner
does.

## API boundary

The API knows protobuf and toolkit shapes only where it converts between them.
Its Appearance path is:

1. authenticate the caller and validate transport-level request requirements;
2. convert protobuf presence and oneof shapes losslessly into toolkit input;
3. call the toolkit method;
4. translate the toolkit error category into the transport error category;
5. persist the toolkit data returned by the SDK; and
6. convert toolkit output back to protobuf.

The API does not inspect RGB values, roughness, style-ref contents, selection
validity, provider compatibility, defaults, masks, or class treatment rules.
Its tests prove conversion fidelity, SDK delegation, authentication,
repository adaptation, and error translation—not a duplicate semantic refusal
matrix.

The old API Appearance entity, API semantic validator, special sibling-envelope
preservation logic, and direct roster customization mapping are removed. Redis
stores toolkit persistence shapes. There is no compatibility migration for old
ephemeral records; restarting Redis and recreating the standard cast is the
supported transition.

## Web renderer

The generated catalog consumes the exact provider outfit-treatment manifest and
binds each supported class to its mask, defaults, declared mesh nodes, and
hashes. Sync validates all required provider files before mutating the ignored
web asset mirror.

Each rendered character owns cloned body materials while sharing immutable
source textures and the mask texture cache. A focused material unit extends
`MeshStandardMaterial` with a stable shader-program key and per-instance
primary/secondary uniforms. It applies mask substitution at the map-color seam
before ordinary PBR lighting. Existing color, emissive, opacity, and depth
properties continue to serve selected, ghost, remembered, and downed
presentation.

Changing colors updates only the existing uniform values and invalidates the
demand-rendered frame. It does not reload the body, recreate the Skeleton or
animation mixer, clone a new texture, or replace the material. Per-character
uniform ownership prevents one hero from recoloring another or the cached GLTF
source.

If the profile, declared node, material type, mask, or shader preparation is
missing or rejected, the exact original textured body remains mounted. Outfit
failure cannot remove the body, hair, hand equipment, or a valid sibling
presentation. Development diagnostics name the failed treatment fact without
exposing private provider paths.

## Creation and readonly UI

The existing Appearance modal becomes a single-open-section accordion:

1. **Hair** — scalp style and the shared hair-color control;
2. **Facial Hair** — facial-hair style and the same shared hair-color value;
3. **Gear Colors** — primary, secondary, and Reset controls.

Hair and Facial Hair may each expose a control for convenience, but both bind
to one state value and one wire field. Changing it in either place immediately
updates both controls and both hair slots. Collapsed headers summarize the
current style or swatches.

The full-body class preview remains visible. Primary and secondary use
unrestricted color controls; a curated dye palette is deferred. Reset clears
both optional overrides. Class changes retain the selected colors and switch to
the new class mask.

Apply sends one complete Appearance, preserving hair and outfit together, and
updates local state only from the returned draft. Cancel sends nothing and
restores persisted state. Finalized sheets remain read-only and show primary
and secondary swatches alongside the existing hair summary.

Owner rendering consumes character Appearance. Peer rendering consumes public
session Customization. Both normalize through one renderer input and one
provider resolver.

## Failure behavior

- RGB values above `0xFFFFFF`, malformed style selections, empty or oversized
  exact style refs, and non-finite/out-of-range roughness fail in toolkit before
  mutation.
- API translates toolkit refusals and does not restate their rules.
- Failed Apply preserves the prior draft and current persisted summary.
- Missing individual outfit colors preserve their corresponding original atlas
  channels.
- Missing or invalid provider treatment preserves the whole original outfit.
- Unknown hair refs retain the existing exact provider fallback behavior.
- Treatment failure for one character is isolated from every other character
  and shared cache entry.
- A class change with no treatment profile keeps the colors in Appearance and
  renders that class's untouched outfit until a compatible profile exists.

## Verification

### Provider

- Deterministic mask and manifest rebuilds are byte-identical.
- Every mask is exact-size, two-channel, non-overlapping, and exercises both
  semantic regions on its approved outfit.
- Declared outfit nodes and UV/material facts match every supported race
  profile for that class.
- Every pre-existing provider file, especially Dwarf v1, remains byte-identical.
- Kirk opens the Blender scene and approves all four mappings before promotion.

### Toolkit

- The full semantic refusal matrix lives here, including RGB24 boundaries,
  explicit zero, optional absence, roughness finiteness, and style-selection
  shape/ref limits.
- Appearance survives draft update, reload, class change, finalization,
  character load/save, and independent deep copies.
- Existing hair behavior remains unchanged after ownership moves.
- Session roster returns current public identity plus exact hair/outfit intent,
  excludes private sheet data, and enforces seated access.

### API

- Proto/toolkit converters preserve absent messages, optional scalar presence,
  explicit zero, both style-selection arms, and malformed shapes for toolkit
  refusal.
- Handlers delegate once, translate toolkit errors, and return SDK-authoritative
  state.
- Repositories round-trip toolkit data without a parallel Appearance model.
- Integration drives UpdateAppearance through reload/finalize/Get/List and the
  SDK-backed roster, without API semantic assertions.

### Web

- Accordion sections retain edits and share exactly one hair-color state.
- Primary/secondary defaults, independent overrides, Reset, class carryover,
  Apply/Cancel, failure retention, and readonly summaries are covered.
- Color updates preserve body mesh, Skeleton, mixer, material, texture, and
  hair/accessory identities.
- Two differently colored characters retain disjoint uniforms and identical
  shared source assets.
- Selection, ghost, memory, movement, downed state, hair, and main/off-hand
  equipment continue to compose correctly.
- Missing/rejected masks visibly preserve the original outfit.

### Integrated evidence

Normal character creation shows all four class outfits with approved distinct
palettes. A normal multiplayer dungeon session shows owner/peer agreement,
multiple independently colored characters, movement, and equipped weapons.
Structural provider checks cover the race matrix without requiring 32
redundant visual witnesses.

Each changed repository receives one implementation branch/PR for the wave,
one final full suite, and one independent whole-PR review when its repository
policy calls for it. Every finding is fixed or dispositioned before readiness.

## Delivery order

1. Provider mask curation and Kirk's Blender approval.
2. Provider publication.
3. Proto contract and toolkit implementation, developed against the approved
   boundary.
4. Toolkit merge and generated module tag.
5. API pin bump, conversion/delegation migration, and SDK-backed roster.
6. Web proto/provider pin, generated catalog, UI/runtime integration, and
   normal-game proof.
7. Canonical design outcome and Journey closure.

Provider, toolkit, and proto are dependencies of API/web in different ways;
their exact merge order may interleave, but no consumer merges against an
unpublished dependency.

## Non-goals

- Alternate leather, chain-mail, or other armor geometry.
- Mechanical equipment changes or armor-rule logic.
- Item-instance identity, dyes, styles, entitlements, shops, or ownership.
- Post-finalization appearance editing or customization-changed events.
- Curated palette restrictions.
- Outfit roughness, metalness, wear, damage, decals, or patterns.
- Skin or eye customization.
- Independent scalp and facial-hair colors.
- Runtime body/outfit assembly.
- Generic prop, obstacle, environment, or weapon recoloring.
- Rewriting existing body GLBs or immutable Dwarf artifacts.
- Publishing licensed source textures, meshes, or Blender scenes.

## Future item migration

When characters can actually change visible armor, an item cosmetic envelope
may embed the same neutral `OutfitCustomization`. At that point equipped item
appearance becomes renderer authority, the character-level outfit field can be
deprecated, and old records can map explicitly. That future Journey must also
define item-instance identity, ownership, entitlement, and edit lifecycle. None
of those concerns are prebuilt here.
