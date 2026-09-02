---
status: approved in conversation
journey: https://github.com/KirkDiggler/rpg-project/issues/352
decide: https://github.com/KirkDiggler/rpg-project/issues/353
provider: https://github.com/KirkDiggler/rpg-game-assets/issues/117
web: https://github.com/KirkDiggler/rpg-dnd5e-web/issues/897
predecessor: https://github.com/KirkDiggler/rpg-project/issues/346
team: Assets
area: Party Assembles
---

# Profile-driven hair customization for every supported race

## Outcome

Human, Elf, Dwarf, Half-Elf, Tiefling, Halfling, Gnome, and Half-Orc use one
production creation/runtime path for independent scalp and facial hair, shared
arbitrary color, and roughness across Barbarian, Fighter, Monk, and Rogue.

This is a generalization of the accepted Dwarf vertical, not eight independent
race deliveries. Proto/API persistence and public roster projection are already
provider-neutral. Delivery therefore requires one provider PR and one web PR.

## Approved product rulings

- All eight race profiles ship together.
- Human comes from the same Modular Fantasy Hero kit used by this wave: Head 00
  with its native Human ears and no modular ear attachment, proportions
  `[1,1,1]`, palette 01-A, and outfits 01/16/08/10.
- Every profile exposes all 38 scalp and 18 facial-hair options.
- An absent customization preserves the race's current fixed look. Defaults are
  provider-owned selections and may be either an exact style or `none`.
- Existing defaults remain Hair 01 for Elf, Hair 16 for Half-Elf/Halfling/Gnome,
  Hair 03 for Tiefling, Hair 08 for Half-Orc, and Hair 04 + Facial Hair 02 for
  Dwarf. Existing profiles without facial hair default to `none`.
- Human defaults to Hair 16 and facial hair `none`, accepted by Kirk at the
  corrected early visual checkpoint with the verdict “perfect”.
- Tiefling horns and every other race-identity mesh are body identity, not hair;
  removing scalp/facial meshes must not remove them.
- Existing Dwarf v1 runtime, manifest, thumbnails, hashes, defaults, and behavior
  remain immutable.
- Creation remains the only write surface. Color/roughness, failure isolation,
  owner/peer rendering, movement, downed, and hand-equipment behavior retain the
  Dwarf contract.

## Profile truth

| Race | Head/identity | Proportions | Palette | Existing scalp | Existing facial |
| --- | --- | --- | --- | --- | --- |
| Human | Head 00 native ears; no ear attachment | `[1,1,1]` | 01-A | Hair 16 | none |
| Elf | Head 00, Ear 03 | `[1,1,1]` | 01-A | Hair 01 | none |
| Dwarf | Head 00, Ear 01 | `[1.08,0.78,1.08]` | 01-A | Hair 04 | Facial Hair 02 |
| Half-Elf | Head 00, Ear 01 | `[1,1,1]` | 01-A | Hair 16 | none |
| Tiefling | Head 00, Ear 02, Helmet Attachment 07 horns | `[1,1,1]` | crimson 02-A | Hair 03 | none |
| Halfling | Head 00, Ear 01 | `[0.84,0.52,0.84]` | 01-A | Hair 16 | none |
| Gnome | Head 00, Ear 01 | `[0.76,0.64,0.76]` | 01-A | Hair 16 | none |
| Half-Orc | Head 03, Ear 02 | `[1.08,1.05,1.08]` | olive 01-A | Hair 08 | none |

Provider generation validates these facts against the existing promoted
race-class authority rather than copying this table as an unverified new source.

## Provider architecture

Existing Dwarf v1 remains one immutable profile manifest. Seven new versioned
profile manifests use a normalized default-selection shape:

```json
{
  "slot": "facial-hair",
  "defaultSelection": { "kind": "none" },
  "options": [
    {
      "styleRef": "modular-fantasy-hero:facial-hair:01",
      "path": "...",
      "sha256": "...",
      "thumbnailPath": "...",
      "thumbnailSha256": "..."
    }
  ]
}
```

A style default uses `{ "kind": "style", "styleRef": "..." }`. The
aggregate manifest maps exact race refs to exact profile refs and
manifest-path/SHA-256 pairs. The web generator normalizes immutable Dwarf
schema-v2 defaults and new manifests into one generated consumer type; it never
rewrites Dwarf authority.

One configuration matrix owns source identity, head/features, proportions,
palette, current defaults, four class recipes, existing complete-body
fallbacks, and output roots. Human uses exact generic class fallbacks only if
structural/visual inspection proves they are the intended neutral Human look;
otherwise the provider publishes explicit complete Human fallback artifacts in
this transaction and binds them immutably.

The transaction adds seven profiles:

- 28 hairless active bodies;
- 392 profile-bound accessories;
- 392 race-specific thumbnails;
- seven profile manifests and one aggregate manifest.

Across Dwarf plus the new profiles, the aggregate authority proves 32 bodies,
448 accessories, 448 thumbnails, and 1,792 body/accessory compatibility checks.
Every existing provider file—including all Dwarf customization bytes—must remain
byte-identical.

## Web architecture

The Dwarf-specific generated catalog becomes a generated profile map:

```text
raceRef + classRef
  -> exact profile
  -> active body + immutable complete-body fallback
  -> scalp/facial option maps
  -> default selection per slot
  -> shared surface defaults
```

The picker receives the selected race label/profile and continues using one
WebGL preview, lazy thumbnails, local edits, response-authoritative Apply, and
Cancel-without-RPC. The title becomes `Customize <Race> Hair`. The preview uses
the actual race/class body.

The renderer keeps one pure resolver and one staged attachment implementation.
A missing hair message resolves each slot's profile default; a profile default
of `none` mounts nothing. Exact refs remain opaque map keys. Unknown refs never
become URLs. Unsupported races/classes continue ignoring hair data.

Owner `Appearance.hair` and peer roster `Customization.hair` remain the only
session inputs. No proto/API changes, finalized editor, live customization
event, or roster invalidation are introduced.

## Failure and compatibility

- Aggregate/profile manifest failure blocks catalog generation before asset sync.
- Active body failure uses the exact profile/class complete fallback, then the
  existing generic fallback.
- Accessory/thumbnails fail per slot exactly as in Dwarf production.
- Existing Dwarf output and behavior are regression-pinned.
- Race-specific accessories remain separate bytes; common rig topology does not
  make baked proportion/bind outputs universal.
- Human identity must be visually approved before the seven-profile bulk apply.

## Verification and human checkpoints

### Early Human checkpoint

Render one neutral Human four-class body sheet and front/profile scalp/facial
catalogs. A controlled no-ear/Ear01/Ear02/Ear03 diagnostic established that
Head 00 already contains the native Human ears and that Ear 01 is a pointed
Half-Elf-style attachment. After regeneration with no ear attachment, Kirk
accepted Hair 16 + facial hair `none` and the corrected identity as “perfect”.

### Provider gate

Run the complete deterministic transaction, 1,792 compatibility checks,
Dwarf/unrelated byte-preservation checks, structural/path/license gates, one
aggregate visual overview, one full suite, and one GLM 5.3 whole-PR review.

### Provider outcome

Provider PR `KirkDiggler/rpg-game-assets#118` merged as
`0c837a801d97c98e50a336fb07e3b50d08d54df1` from reviewed head
`587d67c3859274cfe21bd57af03d788da967eb75`. It published the exact
941-file aggregate customization authority: 484 GLBs, 448 thumbnails, and nine
manifests. The aggregate manifest SHA-256 is
`2457ee61b15cb0ef1ca8cd9b42bc30d84d5286510f91e44d8437a6efbc80efac`.
All 1,792 body/accessory compatibility pairs passed. Two clean 424-GLB builds
were byte-identical across all 848 GLB/report files. Dwarf remained 117/117
byte-identical with manifest SHA-256
`10ba18b4281ea65b757d959ab7caa888adced2b106e2dc3b2e6ae0d19688ba4a`.

Kirk accepted the 8-race x 4-class provider overview with “looks great”. The
final current-head provider suite passed 1,065 tests with 53 established skips
and zero failures. GLM 5.3 moved from Ready 0/0/4 to Ready 0/0/0 after
interrupted-swap recovery, governed-tree temp-view placement, and deterministic
font authority were hardened; the single-operator concurrency-lock suggestion
was dispositioned as a sound non-blocking deferral.

### Web gate

Parameterize catalog/resolver/picker/session tests across all eight profiles.
Use focused tests while iterating, then one full CI run and one GLM 5.3 whole-PR
review. Normal-game evidence may use two automated four-member sessions; Kirk's
single final checkpoint reviews the aggregate creation/session result rather
than repeating a manual workflow eight times.

## Sustainable process

- Work inline without subagents.
- Keep one provider worktree/PR and one web worktree/PR.
- Report at useful checkpoints rather than running silently for hours.
- Do not run full suites repeatedly during implementation; focused tests first,
  one final full suite per PR, rerun only after review fixes.
- Use one whole-PR GLM review per repository; every finding still receives a fix
  or reasoned disposition.

## Gear appearance seam

Gear customization follows this Journey but does not enter its payloads. Rules
identity remains separate from item-instance appearance:

```text
dnd5e:equipment:longsword
  + provider styleRef
  + provider-named color channels
  + player/shop entitlement
```

Hair and gear reuse exact provider lookup, opaque refs, instance-owned material
treatment, and failure isolation. Gear receives its own appearance/entitlement
contract so a cosmetic sword remains mechanically a longsword. Initial gear
scope is named color channels; purchased alternate styles and ornamental
“bling” extend the same item-instance appearance later.

## Non-goals

- Race-by-race delivery.
- Proto/API changes for hair.
- One universal accessory GLB across profiles.
- Finalized-character editing or live customization events.
- Skin, eye, outfit, or independent slot colors.
- Gear implementation or shop entitlements in this Journey.
- Raw licensed source publication.
