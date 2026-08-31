---
name: Character Customization Concept
issue: https://github.com/KirkDiggler/rpg-project/issues/338
team: Assets
status: approved design; implementation not started
---

# Character Customization Concept

## Purpose

Build a durable, provider-backed `/concepts` laboratory that answers whether
separately loaded skinned hair and facial-hair parts can share an animated
production character skeleton, and whether per-instance runtime material
controls are a sound foundation for later hair, cloth, leather, and metal
presentation.

The first proof is deliberately one Dwarf Fighter. It is a Learn, not character
creation: no persistence or public wire shape is frozen until the actual mesh,
material, animation, and browser behavior is visible.

North star: **a player can switch scalp and facial-hair geometry independently,
including no geometry, and apply one shared arbitrary hair color while the body,
animation, proportions, equipment sockets, cached assets, and other character
instances remain unchanged.**

## Current facts

- The Modular Fantasy Hero source contains 38 scalp meshes
  (`Chr_Hair_01`–`38`) and 18 facial-hair meshes
  (`Chr_FacialHair_Male_01`–`18`) on the shared 63-bone rig.
- Production race/class GLBs preserve identity parts as separate named skinned
  mesh nodes, but each complete GLB currently contains only one approved fixed
  look and all meshes share one embedded `SyntyAtlas` material.
- The current Dwarf default is `Chr_Hair_04` plus
  `Chr_FacialHair_Male_02`, baked with proportions `[1.08, 0.78, 1.08]`
  and the provider-owned arm correction.
- `ClassCharacterModel` clones each skinned GLB with `SkeletonUtils`, plays
  `Idle_Relaxed` or `Walk_Forward`, and owns instance-local selection, ghost,
  memory, and main-hand material/attachment behavior. It does not accept
  separately loaded skinned parts today.
- The legacy creation contract already stores `Appearance` with skin, primary,
  secondary, and eye hex strings. Those controls target the old
  `MediumHumanoid` marker-color renderer and do not customize the production
  race/class GLBs.
- Session `PublicMemberInfo.customization` already provides an intentionally
  empty public shelf. Its design explicitly requires the new Synty contract to
  be learned before fields are added.
- `/concepts` is durable fixture-first infrastructure. A Concept uses shared
  production components against provisional local data; `CONTRACT.md` records
  observed evidence before a cross-repository wire request exists.

## Decisions

1. **Learn outside-in before protos.** Prove the consumer and asset behavior,
   then transcribe accepted fixture semantics into a later proto-first
   production journey.
2. **One Dwarf Fighter vertical slice.** Small first slices reveal rig,
   material, and visual problems before multiplying them across races/classes.
3. **Separate runtime skinned accessories.** Test the most modular future path:
   a hairless animated body plus independently loaded scalp and facial-hair
   GLBs rebound to its skeleton by exact bone name.
4. **Independent style selections.** Scalp and facial hair each support
   approved default, explicit `none`, and an opaque style ref.
5. **One shared hair color for now.** Scalp and facial hair use the same
   arbitrary sRGB color. A future independent facial-hair override can be
   additive.
6. **Capability first, curation later.** The Concept exposes an unrestricted
   color picker and raw PBR controls. A production UI may later show curated
   choices without weakening the underlying capability.
7. **Durable only when useful.** If rebinding succeeds, provider assets, shared
   renderer primitives, Concept, tests, and evidence remain. A failed mechanism
   is recorded honestly and is not merged as a knowingly broken interactive
   surface.
8. **Production remains unchanged.** Existing race/class GLBs, resolver rows,
   fallbacks, animations, sockets, and licensed-source boundaries stay intact.

## Approaches considered

### A. One complete GLB containing every selectable hair mesh

The web would toggle bundled nodes. This avoids rebinding and combinatorial
outputs, but every character downloads all options and it does not prove the
independent skinned-part system that can later serve other modular body parts.
It remains the fallback if the selected approach fails.

### B. One complete GLB per scalp × facial-hair combination

This has the simplest runtime, but combinations grow multiplicatively, model
switches reload the whole character, and every new style regenerates many full
bodies. Runtime color still needs a material seam. Rejected as the long-term
learning target.

### C. Separate skinned accessory GLBs — selected

A hairless body supplies the only animated skeleton. Each accessory supplies a
skinned mesh, weights, inverse bind data, and material, then maps onto the
body's same-named bones at runtime. This has the highest technical risk and the
highest learning value; `/concepts` is the correct place to settle it.

## Provider Concept contract

Provider work is additive under a clearly Concept-only runtime root. It
produces:

- one hairless Dwarf Fighter body derived from the current production recipe;
- provisional scalp candidates `Chr_Hair_04`, `Chr_Hair_08`, and
  `Chr_Hair_16`;
- provisional facial candidates `Chr_FacialHair_Male_01`, `02`, and `03`;
- a small versioned manifest; and
- visual and structural evidence.

The candidate set is deliberately varied and provisional. The Concept may
show clipping or poor visual fit; that is a finding, not a reason to hide a
technically valid candidate. Production curation occurs only after the runtime
mechanism is accepted.

The body preserves the production Dwarf Fighter's rig family, canonical root,
proportions, arm correction, `Idle_Relaxed`, `Walk_Forward`, scale/orientation,
and socket profile. It contains no scalp or facial-hair mesh.

Each accessory output contains exactly one skinned identity mesh plus the bind
information needed to reconstruct its skin. It carries no animation and is
built with the same Dwarf proportion transform as the body. The web must not
run a second mixer or retain a second live armature after attachment.

The manifest records, for body and each option:

- schema/workflow version;
- slot (`scalp` or `facial-hair`);
- opaque public style ref and human label;
- source mesh identity;
- runtime path and SHA-256;
- rig family and weighted-bone inventory;
- bind-contract version;
- default-selection facts; and
- supported surface-treatment mode.

The initial material mode supports per-instance base color, roughness, and
metalness. Provider evidence determines whether a neutral texture/material can
retain useful authored detail under arbitrary tint. The contract must report
what is actually supported; it must not call a flat-color proof a cloth or
leather texture system.

Promotion validates exact source identity, one mesh, finite geometry, weights,
bind matrices, material/texture facts, no animation, no private path leakage,
and deterministic rebuilds. Existing production GLBs remain byte-identical.
Licensed source FBX/textures remain private; only approved game-ready GLBs and
manifests synchronize to the ignored web runtime tree.

## Shared web runtime

The Concept extends a production-owned character rendering seam rather than
building a parallel Concept renderer.

A shared skinned-accessory unit receives:

- the cloned live body root;
- one manifest-backed accessory presentation;
- an instance-local surface treatment; and
- a status callback.

It loads and skeleton-clones the accessory source without mutating the shared
`useGLTF` cache. For every bone referenced by the accessory skin, it finds the
body bone by exact semantic name. It refuses duplicate names, missing weighted
bones, incompatible rig family, invalid bind data, or non-finite transforms.
On success it constructs the accessory skin against the body bones, mounts
only the accessory mesh under the body instance, and leaves animation entirely
with the body's existing mixer.

The body remains renderable when either accessory is absent, unknown, loading,
or rejected. An accessory failure does not select a plausible-looking fallback
and does not take down the Canvas.

Surface treatment clones only the accessory instance's material resources.
Base color, roughness, and metalness may change without altering the body,
another accessory instance, another character, or cached source materials.
Created materials/textures are disposed when a style changes or the character
unmounts. Existing selection, ghost, remembered, and attachment treatments must
compose without resetting customization.

The production session and creation routes pass no accessory props in this
journey, so their behavior remains byte-for-byte/data-flow equivalent.

## Concept fixture and controls

Deep link: `?concept=character-customization`.

The provisional fixture distinguishes three states per slot:

- **default/unset** — current provider default (`Hair 04` or Facial Hair `02`);
- **none** — mount no accessory for that slot; and
- **style ref** — resolve one exact manifest option.

The Concept includes:

- the actual provider-backed Dwarf Fighter body;
- independent scalp and facial-hair selectors;
- one unrestricted shared sRGB base-color control plus useful swatches;
- raw roughness and metalness controls;
- clearly provisional hair, cloth-like, leather-like, and metal-like PBR
  presets that may set all three treatment values on the accessories;
- idle/walk controls;
- close, orbit, and tactical views;
- an optional canonical main-hand weapon as a socket-regression witness; and
- a contract inspector.

The PBR presets are comparison aids, not production surface semantics. This
slice does not promise fabric weave, leather grain, rust, normal/detail maps,
or per-outfit material zones.

The inspector shows current fixture JSON, body/accessory refs and hashes,
load/bind status, mapped and missing bones, body skeleton identity, duplicate
runtime-armature count, material values, asset byte sizes, and observation
coverage. Unknown refs remain visibly unmapped.

## Success gate

The selected approach succeeds only when browser evidence proves all of the
following:

- every candidate loads and binds through exact weighted-bone mapping;
- scalp and facial hair independently support default, `none`, and style refs;
- both accessories follow the head/face through sampled idle and walk motion;
- the body skeleton is the sole live animation authority;
- rapid style changes do not leave stale meshes, materials, mixers, or bones;
- one arbitrary color updates both slots and no non-hair surface;
- raw and preset PBR changes remain instance-local;
- a second character instance preserves its own treatment;
- Dwarf proportions, body root, animations, and canonical weapon socket remain
  unchanged;
- failures degrade to the body plus explicit diagnostics;
- asset size, draw-call, and decoded-texture observations are recorded; and
- proof windows have no unexpected console, page, or request failures.

Provider verification includes focused contract tests, real deterministic
exports, structural GLB readback, clean rebuild comparison, sampled Blender
idle/walk renders, inventory/private-path checks, and preservation of all
existing production GLB hashes.

Web verification includes pure synthetic-skeleton tests for exact mapping and
refusal paths, cache/material isolation tests, lifecycle/disposal tests,
Concept interaction tests, focused renderer tests, full `npm run ci-check`, and
human browser review through the reproducible deep link.

## Failure outcome

If separate rebinding cannot satisfy the success gate after one bounded Dwarf
Fighter Learn, stop. Record the exact failure and measured facts in the design
and provider evidence. Do not invent per-style transforms, retain duplicate
animated armatures, patch animation every frame, or merge a broken Concept.
Reconsider the bundled selectable-node approach using the same fixture and
material findings.

## Proto and production direction after acceptance

No proto changes belong to this Concept journey. A successful Concept produces
the desired consumer semantics; a following production journey develops
outside-in from those accepted fixtures and then merges provider-first.

The current likely wire shape is one shared hair-customization message carrying
optional scalp style ref, optional facial-hair style ref, and one optional
24-bit sRGB value. Absence preserves provider defaults; explicit none must stay
distinct from absence. The message would be persisted through the existing
draft appearance boundary and projected into the existing public session
`Customization` shelf. This is a hypothesis to test against the Concept's
actual option, failure, color, and fallback behavior—not a schema decision made
by this design.

Creation-only selection is the first production target. Editing finalized
characters, public customization-changed events/roster invalidation, separate
facial-hair color, skin/outfit/eye customization, and API-owned asset-catalog
validation remain later decisions. Toolkit remains outside presentation data.

## Delivery order

1. Keep this cross-repository design PR open as the review/tracking surface.
2. Provider candidate: generate and validate Concept-only body/accessories on
   one provider branch without changing production outputs.
3. Web Concept: iterate against that local exact candidate until the consumer
   either proves or rejects the mechanism.
4. On success, merge the provider contract first; sync its exact merged bytes
   into the web branch and repeat the web/browser gates before merging the
   durable Concept. On failure, merge neither a useless provider payload nor a
   broken interactive Concept; record the failed Learn in this design.
5. Record Kirk's visual/runtime verdict in `CONTRACT.md` and this design.
6. Merge the design record after the implementation outcome is known.
7. If accepted, open a separate production customization journey beginning
   with the evidence-derived proto contract.

## Non-goals

- No proto, API, toolkit, Redis, draft, finalized-character, or roster change.
- No production character-creation or dungeon-route integration.
- No all-race/all-class expansion.
- No runtime body/outfit assembly.
- No skin, eye, armor, or clothing color UI.
- No independent scalp/facial color.
- No authoritative cloth/leather/metal taxonomy or per-zone masks.
- No replacement of current race/class GLBs, downed fallbacks, or portraits.
