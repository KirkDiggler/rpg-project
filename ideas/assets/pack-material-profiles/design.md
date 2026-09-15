# Editable pack material profiles

Status: proposed design following Kirk's agreed operator workflow.

Tracking: [project #450](https://github.com/KirkDiggler/rpg-project/issues/450),
[Assets #217](https://github.com/KirkDiggler/rpg-game-assets/issues/217).

## Outcome

**Edit a pack profile → run the preparation script → inspect grouped options in
Asset Review Lab → adjust/approve → reuse.**

The product here is the reusable preparation process, not a textured floor or a
published batch. Kirk makes material/appearance decisions. Scripts discover files,
pair textures, preserve mesh assignments, generate previews and validate results.
No operator should need to hunt for FBXs/GLBs or assign shaders object by object.
Blender is an optional exception inspector, not the primary workbench.

## What the source tells us

Dark Fortress v3 has 82 texture files: four numbered atlases with A/B/C variants,
four floor color/normal pairs, and additional environment, emission, FX and misc
maps. Its vendor declaration has 1,458 slot references and 60 distinct material
labels (not 60 proven unique shaders). The same declared material appears across
many objects. Asset names are useful hints; declared/actual material-slot names
are stronger evidence. For example, Brick_Large_01 points naturally toward the
Brick_Large_Texture_01 / Brick_Large_Normals_01 pair.

The current pack config understands atlas01 A/B/C and a few exact per-model
overrides. The missing mechanism is shared material definitions, proposed options
and human-editable choices—not a new converter or more handwritten overrides.

## Three records with different owners

1. **Generated pack inventory and proposals (Assets).** Audited source identities,
   exact object/slot/UV facts, material-definition groups, texture sets, compatible
   options, recommendation reasons and affected uses. Hashes and expanded member
   lists are machine bookkeeping, not fields the operator maintains.
2. **Editable pack profile (human choices, Assets contract).** Versioned JSON with
   profile ID, pack slug/version and material-family-to-option selections. Keys
   are stable, readable IDs supplied by discovery. A null selection means
   unresolved; absence is not approval. No source-root paths, generated member
   lists, hand-copied digests or `trusted: true` switches.
3. **Resolved recipe and review evidence (generated).** Exact bindings for the
   existing converters, input/profile/definition fingerprints, outputs and the
   explicit human approval. The resolver—not Web—expands family choices to
   per-object slots. Editing the profile does not edit historical evidence.

The profile is the one editable choice document. Browser state is only a draft;
it does not become a second authoritative profile. Existing pack configuration
continues to own source discovery and approved conversion definitions. Compiled
bindings must not become a second independently hand-edited mapping table.

## Discovery and option generation

- Audit the source pack before building trusted review output. Reuse material
  declarations, FBX inspection and existing default/explicit binding support.
- Generate deterministic candidate sets using reviewed bindings first, then
  declared material names, texture naming tokens/numbered variants, folders and
  color/normal pairs. Asset-name clues are weaker suggestions, not proof.
- Each proposal explains its evidence and any missing facts. Ambiguous matches
  remain choices. A label such as "recommended" is not visual approval.
- Group by pack-scoped material definition and supported interpretation. Equal
  names alone do not merge conflicting definitions; importer suffixes/aliases
  require verification. Material identity conflicts form explicit exceptions.
- Separate atlas-layout families, supported tiling materials and special shader
  cases. Numbered atlases are not interchangeable because both contain stone.
  Alternative palettes/layout-compatible sets can be offered when supported;
  tiling substitutions must also respect UV scale, sampling and shader needs.
- Do not offer the entire texture directory as a menu. Show the prepared options
  relevant to that family. A set bundles color, optional normal and the supported
  material settings. Missing normal/shader information is disclosed, not invented.
- Static approximations of vendor custom shaders are labelled as such. Unsupported
  transparency, animation or other behavior stays unresolved instead of being
  silently represented as an ordinary opaque material.

A reviewed definition is reusable for new uses that satisfy its verified contract.
A new mesh does not require re-approving an unchanged material merely because the
member count grew. Changed definitions, interpretation or option compatibility do
require a new decision. Individual asset calibration/Ready review still applies.

## Operator experience

Extend the existing preparation/`asset-review` flow with a profile input. With a
registered workspace, the script already knows source/cache locations. It can
create a proposed profile with readable available IDs or read one the operator
has edited. Creating this local profile must not require repository/branch setup.

The script prepares a **review-only** material catalogue and staged GLB options,
then opens the existing Asset Review Lab in its material-review view:

- Organize by material family, with useful category filters such as floors,
  buildings, wood and props. Categories are navigation, not compatibility rules.
- Show recommended authored matches and compatible alternatives, their evidence,
  missing facts, representative previews and the complete affected-use list/count.
- Preview choices on the same representative pieces, including relevant multi-slot
  cases. Preserve other slots and identify any provisional context materials.
- Choose once per family, not once per object. Changing a choice updates the family
  draft. A deliberate override needs the same validation as a recommendation.
- Keep unresolved/unsupported cases visible. An optional Blender action prepares
  the selected context; ordinary review never requires manual source-file hunting.
- Import/export the same profile JSON. Initially reuse the Lab's existing file
  import/download pattern rather than adding a privileged browser filesystem API.
  The preparation script accepts the exported profile directly; no manual JSON
  merging, folder copying, or digest transfer is required.

Prepare representative options, not every possible combination across every
model. Choose samples deterministically and expose coverage/extra examples. Full
selected output is generated from the profile, not copied from screenshot samples.
Changing preview choices cannot mark all members Ready or publish them.

## Conversion and activation boundaries

Material resolution belongs **after source extraction/indexing and before trusted
conversion, calibration/Ready and runtime promotion**. Already-ingested sources
can be reused; a fresh download is not required.

Reuse the original FBX and its slot/UV assignments for conversion. Replacing a
texture on an already flattened neutral GLB is not the repair mechanism. Reuse
`synty_material_preflight.py`, `synty_material_bindings.py`, `fbx_to_glb.py`, review
preparation and verified cache refresh rather than forking their responsibilities.
Web consumes prepared option identities/GLBs; it does not guess bindings or claim
browser texture substitution proves the converter's final output.

After a human approves the exact selected definitions/profile, compile the choices
through the pack's normal material authority. Any canonical configuration update
uses the normal worktree/PR/human-merge boundary, with the script doing mechanical
preparation rather than asking the operator to edit expanded slot mappings.

Then prepare and validate a new cache generation and offer explicit activation
through the existing transition mechanism. Bind effective profile/compiled recipe
identity into the existing material/source/output checks. Unregistered preview
profiles must not bypass those checks to become trusted inputs. If that integration
is not implemented yet, the feature must report preview-only, not pretend it is
ready for ordinary ingestion.

Retain previous caches and approvals/evidence. Unchanged effective bindings reuse
verified outputs; keep source-quality images and advisory performance estimates.
Do not change previously published bytes incidentally. A profile change affecting
published identities must stop ordinary activation and identify the required
explicit revision/appearance decision, not silently rewrite earlier releases.
Active-release/cache transition guards remain in force.

## Ownership and delivery slices

- **Assets:** profile schema, audited discovery, matching/compatibility, recipe
  compilation, staged previews, launcher orchestration and cache/promotion binding.
- **Web:** material-family review UI, options/coverage display, profile editing and
  import/export. Existing model rendering and source/load safety remain reusable.
- **Project:** this cross-repo intent and boundary record, not runtime task state.

Build in visible steps, without an autonomous agent/review campaign:

1. Profile validation + deterministic resolver/proposal contract + fixture tests.
2. Scripted staged options and family review in the existing viewer; profile
   round-trip. This is the first human-usable slice and remains preview-only.
3. Verified authority/cache activation integration, completing normal preparation
   from an approved profile. Do not claim the whole workflow from slice 2 alone.

No new floors export, special tile binding or wall publication is an acceptance
shortcut. Kirk will supply his own input and exercise the operator himself.

## Acceptance evidence

- A fresh profile yields explained options without manually finding source files.
- A supported family selection resolves all and only its compatible slots;
  multi-material meshes keep other assignments and original UVs.
- Ambiguous pairs, conflicting material labels, unknown IDs, incompatible atlases
  and unsupported shaders stay explicit and unapproved.
- Profile export/import/direct edit/rerun preserves the same choices. Changed
  profile or source facts invalidate affected evidence; unchanged definitions and
  outputs are reused. Cancellation does not overwrite the existing profile/cache.
- Synthetic multi-family/variant fixtures exercise the resolver and actual
  conversion; tests do not just assert names or mocked readiness.
- Viewer tests exercise family choices and profile round-trip; Kirk evaluates the
  interaction early rather than receiving a polished final-only asset result.
- Final integration proves cache/source/profile binding and preservation of
  existing published outputs. No manual checkpoint insertion substitutes for the
  operator's own execution.
