---
status: approved in conversation; written review pending
journey: https://github.com/KirkDiggler/rpg-project/issues/346
decide: https://github.com/KirkDiggler/rpg-project/issues/347
predecessor: https://github.com/KirkDiggler/rpg-project/issues/338
team: Assets
area: Party Assembles
---

# Production four-class Dwarf hair customization

## Outcome

A player creating a Dwarf Barbarian, Fighter, Monk, or Rogue can choose scalp
hair, facial hair, one shared arbitrary color, and hair roughness; reload and
finalize the draft; and see that exact public look for both owner and peers in
the normal dungeon session.

This is the first production customization vertical. It replaces the unused
legacy appearance surface with the behavior proven by the provider-backed
Character Customization Concept. It is not a general all-race customization
platform.

## Approved product rulings

- All four Dwarf starter classes ship together because they use the same head,
  `modular-fantasy-hero-v1` armature, bind profile, and Dwarf proportions.
- Each class keeps its own approved outfit: Barbarian 01, Fighter 16, Monk 08,
  and Rogue 10. Hair modularity does not put Fighter clothing on another class.
- Production resolution moves every Dwarf to a hairless class body plus shared
  accessories. There is no live-character migration requirement.
- One provider catalog exposes all 38 approved scalp styles and all 18 approved
  facial-hair styles for all four class bodies.
- Scalp and facial hair select independently. Each slot supports provider
  default, explicit none, or one exact provider style ref.
- One optional arbitrary 24-bit sRGB value colors both slots.
- One optional finite roughness value in `[0,1]` overrides the provider default.
  Hair metalness remains provider-controlled and is not persisted or exposed.
- The production picker uses lightweight style thumbnails plus the actual
  selected class body in a live orbitable preview.
- Customization is creation-only. There is no finalized-character editor or
  customization-changed session event in this vertical.

## Evidence carried forward

Journey #338 and merged design PR #339 established the mechanism rather than
merely proposing it:

- provider merge `4c208fad5a950d2103d763a9c8aac96d3bb342b1`;
- web Concept merge `1a3342ed95be2e6b3d9a685c430634c6d64e527f`;
- canonical Concept record `755dfbdff4a93a1e82a9249f4b51fa57d287901a`;
- exact rebinding to the animated body's 63 Bone objects;
- zero mounted source armatures and no second animation mixer;
- independent scalp/facial selection and explicit none;
- instance-owned arbitrary color, roughness, and metalness capability;
- stable in-place treatment updates without accessory remounts;
- coexistence with idle/walk and main/off-hand attachments; and
- deterministic provider generation, exact hash locking, and explicit failure
  diagnostics.

The local licensed Blender catalog at
`/home/kirk/synty-review/character-customization-catalog/` proves the source
geometry across seven race identities, all four class outfits, 38 scalp styles,
and 18 facial-hair styles. Kirk accepted the Dwarf and broader front/profile
catalog: **“those all look great.”** Production still performs exact
four-outfit clipping and runtime-bind gates against the promoted bytes.

## Architecture

The provider owns visual capability and byte identity. The API owns public
cosmetic data, draft/final-character persistence, and session projection. The
web owns the creation interaction and deterministic rendering of provider
refs. Toolkit owns none of this presentation data.

```text
provider customization manifest
  ├─ four classRef → hairless body URI mappings
  ├─ 38 scalp styleRef → accessory + thumbnail mappings
  ├─ 18 facial styleRef → accessory + thumbnail mappings
  └─ profile/material defaults + exact hashes
                   │ exact provider merge and web sync
                   ▼
creation modal ── UpdateAppearance ── draft API-owned envelope
                                           │ FinalizeDraft
                                           ▼
                                  character API-owned envelope
                                           │ GetRoster projection
                                           ▼
                                PublicMemberInfo.customization
                                           │
                                           ▼
                shared ClassCharacterModel body + accessories
```

A style ref is an opaque lookup key, never a path. The API does not know catalog
membership. The web may load only a URI obtained by exact manifest lookup; it
never interpolates a persisted ref into a URL.

## Shared proto contract

Create a neutral package such as
`dnd5e.api.customization.v1alpha1` so character creation and the public session
surface import one semantic type without making either API package own the
other.

The normative shape is:

```proto
message StyleSelection {
  oneof selection {
    string style_ref = 1;
    google.protobuf.Empty none = 2;
  }
}

message HairCustomization {
  StyleSelection scalp = 1;
  StyleSelection facial_hair = 2;
  optional uint32 color_srgb = 3;
  optional float roughness = 4;
}
```

Presence semantics are load-bearing:

- absent `StyleSelection` message: use the provider default for that slot;
- present `style_ref`: use that exact opaque catalog entry;
- present `none`: mount no accessory in that slot;
- absent `color_srgb`: use provider default base color;
- present `color_srgb`: interpret bits `0xRRGGBB` as 24-bit sRGB;
- absent `roughness`: use provider default roughness; and
- present `roughness`: use the exact finite normalized override.

A message-valued `none` avoids a false boolean that is present but contradictory.
A string sentinel such as `"none"` is forbidden because provider refs are an
open string space.

The existing character `Appearance` message removes its unused fields 1–4 and
permanently reserves both their numbers and names:

```proto
message Appearance {
  reserved 1 to 4;
  reserved "skin_tone", "primary_color", "secondary_color", "eye_color";

  dnd5e.api.customization.v1alpha1.HairCustomization hair = 5;
}
```

The public session shelf becomes:

```proto
message Customization {
  dnd5e.api.customization.v1alpha1.HairCustomization hair = 1;
}
```

The old numbers and names are not reused even though no player data migration
is needed. Reservation protects generated and unknown-field behavior from
future accidental reinterpretation.

## API ownership and validation

`entities.Appearance` becomes an API-owned hair envelope and remains outside
toolkit `character.Data`. Draft Redis storage, final character Redis storage,
character Get/List responses, and session roster projection preserve the same
semantic values.

`UpdateAppearance` remains the creation-only write seam. It validates before
mutation:

- each present style ref is non-empty and bounded in encoded length;
- `color_srgb <= 0xFFFFFF`;
- roughness is finite and in `[0,1]`; and
- the generated oneof shape is respected.

Invalid input returns `InvalidArgument` and leaves the draft unchanged. The API
does not validate that a ref belongs to Dwarf, query an asset service, duplicate
the provider manifest, or construct an asset URL. Unknown but well-shaped refs
can persist and will fail closed at the renderer's exact catalog lookup.

Finalization copies Appearance from draft to the API-owned character envelope.
Session SDK save paths continue replacing only toolkit `Data`, preserving the
API-owned Appearance pointer. `GetRoster` reads that envelope for player rows
and maps `hair` into the always-present public `Customization` shelf. Monster
rows retain an empty shelf. No private sheet fields enter the roster.

Because writes stop at finalization, the existing load-roster-on-join behavior
is sufficient. No customization-changed event, roster invalidation, or live
editor protocol is introduced.

## Provider production contract

The provider publishes a versioned production customization manifest under the
private canonical character tree. Existing complete Dwarf GLB bytes remain
immutable historical/fallback artifacts, while public `raceRef + classRef`
resolution moves to new versioned hairless body outputs. “Replace the bodies”
means replace active resolution, not mutate an already published file in place.

The manifest contains:

- schema/workflow version;
- profile ref `modular-fantasy-hero-v1:dwarf`;
- exact race ref and the four supported class refs;
- class-ref-to-body URI and SHA-256 mappings;
- one exact class-ref-to-immutable-complete-body fallback URI/hash mapping for
  each new modular body;
- skeleton, inverse-bind, animation, proportions, atlas, and socket profile
  facts;
- scalp and facial slot definitions;
- opaque style ref, display label, accessory URI/hash, thumbnail URI/hash, and
  source identity for every option;
- default scalp `modular-fantasy-hero:hair:04`;
- default facial hair `modular-fantasy-hero:facial-hair:02`;
- default sRGB color `0x5A3825`;
- default roughness `0.72`; and
- provider-controlled metalness `0.0`.

Style refs extend the proven Concept vocabulary:

- `modular-fantasy-hero:hair:01` through `:38`; and
- `modular-fantasy-hero:facial-hair:01` through `:18`.

Each body preserves its current head 00, ear 01, class outfit, Dwarf proportions
`[1.08, 0.78, 1.08]`, palette `01-a`, `Idle_Relaxed`, `Walk_Forward`, 63-bone
hierarchy, arm correction, downed behavior, and modular hand socket semantics.
Only Hair 04 and Facial Hair 02 are removed from the body recipe.

Each accessory contains exactly one skinned mesh, no animation, one opaque
instance-replaceable uniform PBR material, and no texture. It is finite,
weight-normalized, joint-range-safe, and exactly compatible with every one of
the four body bind profiles. No source accessory armature is runtime authority.

Each option gets a deterministic lightweight front-view thumbnail rendered on
the approved neutral Dwarf head/material witness. Default and None tiles can be
client-generated; provider thumbnails represent exact styles. Thumbnail failure
must not make a style unselectable.

Provider promotion remains deterministic, source/archive/path safe,
hash-bound, atomic, and structurally read back. It proves the complete manifest
and output tree, exact source identities, four class bodies, 56 accessories, 56
thumbnails, preserved pre-existing file hashes, and no licensed source leakage.

## Web creation experience

The unused legacy color editor is removed. Once both Dwarf race and one of the
four supported classes are selected, the existing Appearance action opens the
production workspace:

- scalp grid: Default, None, and 38 provider thumbnail tiles;
- facial-hair grid: Default, None, and 18 provider thumbnail tiles;
- arbitrary shared color picker;
- roughness slider in `[0,1]`; and
- the actual selected class body, idling in an orbitable close preview.

Barbarian shows outfit 01, Fighter 16, Monk 08, and Rogue 10. The picker never
uses a Fighter body as a stand-in for another selected class.

Edits remain local to the modal. Style changes stage the requested accessory
and swap only after it binds, avoiding a hairless loading flash. Color and
roughness update the mounted instance material in place and preserve mesh,
material, body, and Skeleton identity. **Apply** sends one whole
`UpdateAppearance`; **Cancel** sends nothing and restores persisted state.

The grid preloads lightweight thumbnails, not all 56 GLBs. Only defaults and
currently selected accessories need model preloading. Missing thumbnails use a
labeled fallback tile. The live 3D preview remains the final visual authority.

For non-Dwarves the ineffective legacy Appearance controls disappear and this
Dwarf-only picker is not offered. A stale or manually supplied hair payload may
remain public data, but an unsupported race/class renderer ignores it and keeps
its existing fixed-look model behavior.

## Production renderer

The existing composite resolver first resolves `raceRef + classRef`. All four
Dwarf mappings now select their class-appropriate hairless body and the shared
Dwarf customization profile. Every Dwarf uses provider defaults when the hair
message or any individual override is absent, so an untouched creation still
matches the approved Hair 04 / Facial Hair 02 / brown default look.

The accepted Concept binder becomes production support rather than a parallel
implementation. It maps accessory joints to the cloned body's authoritative
Bone objects, accepts equivalent body Skeleton wrappers only under exact Bone
identity/inverse/bind equality, owns instance materials, and mounts no source
armature. Main hand, off hand, scalp, and facial hair remain independent
siblings.

Owner and peer paths consume the same resolved public shape. Character creation
uses draft/character Appearance; the session uses
`PublicMemberInfo.customization`. Both normalize to one renderer input so there
is no owner-only visual shortcut.

## Failure behavior

- Body resolution/load failure first uses the manifest's exact preserved
  complete Dwarf class artifact, then follows the existing generic class/model
  fallback chain. The web does not guess either fallback path.
- Unknown style ref fails exact manifest lookup. It never becomes a URL.
- Missing, malformed, or bind-incompatible accessory leaves the body and the
  other valid slot mounted and emits a slot-specific diagnostic.
- A rejected replacement keeps the previously attached valid style until the
  requested selection is resolved as failed; persisted invalid data then
  degrades to body plus any valid sibling rather than mounting untrusted bytes.
- Thumbnail failure renders a labeled tile and does not block selection.
- Material failure cannot mutate cached GLTF material or another character.
- Provider default absence is a contract defect, not an invitation for the web
  to guess a style.

Diagnostics name profile, slot, requested style ref, manifest outcome, asset
URI/hash when known, body Skeleton identity, mapped-bone count, and refusal
reason without leaking licensed source paths.

## Verification gates

### Proto

- Generated Go and TypeScript presence/oneof behavior.
- Reserved legacy numbers and names.
- Shared imports compile for character and session packages.
- No unrelated generated contract drift.

### API

- Handler/converter/entity round trips for default, none, exact styles, color
  boundaries, and roughness boundaries.
- NaN, infinity, out-of-range color/roughness, empty/oversized style refs, and
  missing requests refuse before repository mutation.
- Redis draft reload preserves presence distinctions.
- Finalization copies exact customization.
- Get/List character responses preserve it.
- Session SDK writes do not erase the API-owned envelope.
- GetRoster projects exact public customization for players and an empty shelf
  for monsters without exposing private sheet data.
- Integration coverage exercises create → appearance → reload → finalize →
  roster.

### Provider

- Clean deterministic rebuild and exact tree/hash manifest equality.
- Four class bodies preserve outfit identities, proportions, atlas, animations,
  socket witnesses, and finite 63-bone structures while excluding scalp/facial
  meshes.
- All 56 accessories structurally validate and bind to all four bodies: 224
  class/accessory compatibility checks.
- Exactly 56 manifest-bound thumbnails exist and are nonblank/deterministic.
- Four-class front/profile clipping evidence covers scalp and facial catalogs.
- Every pre-existing provider file remains byte-identical.

### Web and browser

- Pure contract tests cover defaults, none, exact refs, unknown refs, safe URI
  lookup, color conversion, roughness, and supported profiles.
- Binder/lifecycle tests cover all four body Skeletons, 56 accessories, staged
  style swap, treatment identity stability, disposal, StrictMode, and failure
  isolation.
- Creation tests cover class-appropriate preview, tile selection, thumbnail
  fallback, Apply/Cancel, draft reload, and no dead legacy controls.
- Session tests prove local and peer use the same public customization.
- Existing main/off-hand, movement, animation, downed, and non-Dwarf regression
  suites remain green.
- Browser publication uses exact merged provider hashes and has no unexpected
  console, page, request, or HTTP failures.

The final human walk creates four Dwarves—one per starter class—with visibly
distinct combinations, reloads/finalizes them, enters one four-player session,
checks owner/peer looks, walks each model, and verifies representative main- and
off-hand equipment. Contact sheets and the live picker establish visual fit;
runtime observations establish exact binding/material authority.

## Delivery order

1. Merge this design and its implementation plan.
2. Add and release the neutral proto contract, including reserved legacy
   Appearance fields.
3. Update API entities, validation, persistence, finalization, and roster
   projection against that exact proto release.
4. Generate, review, and merge the complete immutable provider contract.
5. Sync the exact provider merge into the web implementation; replace the dead
   editor, promote the Concept binder, and wire creation/session rendering.
6. Run provider, API, web, browser, normal-game, human, and independent-review
   gates. Web publication merges only after provider authority is exact.
7. Record exact merges and Kirk's verdict in this design before closing Journey
   #346.

Implementation slices may develop in parallel after the proto release where
their inputs are independent, but provider remains authoritative before web
publication. No client workaround may compensate for an API projection or
ownership defect.

## Non-goals

- Other race profiles or a universal accessory byte contract.
- Editing a finalized character.
- Customization-changed session events or roster invalidation.
- Independent scalp and facial-hair colors.
- Skin, eye, armor, outfit, or secondary color controls.
- Persisted or player-controlled metalness.
- A surface-preset taxonomy.
- Runtime body/outfit piece assembly.
- API-owned provider catalogs or style allowlists.
- Toolkit cosmetic fields or validation.
- Replacing existing non-Dwarf race/class outputs.
- Shipping licensed source files or local Blender review scenes.
