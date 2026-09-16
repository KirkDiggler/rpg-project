# Profile-and-preview delivery checkpoint

## Operator visual follow-ups — paused for Blender inspection

After installation, Kirk tried the material screen and clarified that he has
**chosen no materials**. Some proposals look good from one angle, but edges become
transparent when orbiting and the interior becomes visible. White regions did not
change when selecting options. Neither symptom is diagnosed or approved.

Follow-up list:

- [ ] Angle-dependent visibility: distinguish material alpha handling, backface
  culling/sidedness and source geometry. Do not assume emissive settings or an
  atlas replacement fixes it.
- [ ] Persistent white regions: identify the actual object/slot and whether it is
  intentionally neutral context, missing material channels/images or another
  renderer/material interpretation problem. Kirk suspects emissive/image settings;
  that is a hypothesis, not established source-material authority.

Kirk will inspect difficult cases in Blender and keep this follow-up list rather
than start another repair campaign. He clarified that **this must not pause asset
ingestion**: continue reviewing eligible assets from the existing registered cache
in the normal Asset Review Lab, defer problematic rows, and export only human-marked
Ready rows. The new material-profile authority handoff remains unimplemented, but
it is not required to review existing trusted-cache candidates. No material choice,
new-profile approval or cache activation is authorized by these observations. The698 sources with a proposal for every declared slot are
only potential coverage, **not698 usable or visually approved assets**. Successful
loading/layout checks from the earlier proof do not establish correct appearance.

## Source-first correction — 2026-09-16

The original preview PRs219/1087 were merged and installed. Real pack use then
exposed what the synthetic fixture did not: a vendor prefab declaration is not
guaranteed to describe the raw FBX one-to-one. Missing declarations, mesh-data
aliases and a five-used-slot source with four declared slots invalidated the
assumption. Isolated fixes passing did not establish full workflow readiness;
PR223 was explicitly returned to draft after the full real-pack run failed.

Kirk approved the shared correction: audit original FBXs before assigning
proposals; preserve strict correspondence; show unresolved sources as exceptions
without blocking valid representatives; generate exception-only inspection blends.
Tool/import failures and corrupt evidence remain fatal. Blender supports human
judgment, not automatic shader-graph import or implicit approval.

Implemented in Assets [223](https://github.com/KirkDiggler/rpg-game-assets/pull/223)
at `6e792c5` and paired Web [1092](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1092)
at `d672a598`. Both are unmerged/uninstalled at this checkpoint; human review and
merge remain pending. No trusted-cache activation or asset publication is included.

Evidence from the full real pack, not a substituted synthetic run:

- 825 original sources audited before binding:811 reconciled layouts,14 explicit
 exceptions. Counts do not establish material/shader approval.
- Actual launcher completed all56 families and served48 prepared views from43 jobs.
 Browser rendered planks, atlas01_A and floor atlas04_C; verified all43 HTTP GLB
 hashes; showed source exceptions and blocked-family state; exported/imported
 profile choices without browser errors.
- Second normal launcher invocation consumed the browser-exported JSON: audit
 reuse,48 views,11 new context-dependent jobs. Second browser/hash/profile pass.
- Generated gate inspection blend reopened with original objects, material names,
 slots, face assignments and UVs identical to the audited source.
- 99 focused Assets checks;12 focused Web checks; complete local Web ci-check exit0.
 Synthetic tests protect editable blend files, cache receipts, source changes,
 fatal execution failures and valid alternate representatives.
- User profile and registered workspace config byte-identical; trusted cache and
 published outputs untouched. All proof servers stopped. Licensed proof outputs
 remain private; none committed to the public Web repository.

Merge the paired consumer and producer before installing the updated capability.
The new launcher refuses older Web checkouts before preparation. Kirk is holding
his normal command until explicit installation confirmation.

## Historical initial preview checkpoint

The approved first slice (plan Tasks1–4) is implemented. This is not trusted
profile activation or asset publication; design slice3 remains separate.

## Code PRs

- Assets217 → [PR219](https://github.com/KirkDiggler/rpg-game-assets/pull/219),
  `cd12dc879ac5b3726c786b2b969b6c03849cb7ff`, targets main.
- Web1086 → [PR1087](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1087),
  `60fce3f7cc6d7a99c57092cb12c9c662bedeae6a`, targets dev.

Both are open, non-draft and mergeable. Human review/merge remains the operator's
step. No automatic merge or installation has occurred.

## Verification

- 85 focused Assets checks passed, including actual Blender synthetic fixtures,
  legacy conversion control, slot/UV/layout protection, producer failure/reuse,
  profile creation and launcher/input compatibility.
- 23 focused Web profile/material-view/normal-Lab checks passed. Final full local
  `npm run ci-check` passed (format, lint, types, build/isolation, tests).
- All four hosted Web checks passed on the final head: Test, Lint and Type Check,
  Security Audit, Deploy Preview.
- An actual synthetic FBX was converted and served through the real launcher.
  Browser proof verified the served GLB SHA, loaded model, family choice,
  profile download/import, and a second launcher run consuming the exported JSON.
- The real browser check exposed a callback-identity loading loop missed by the
  original scene double. A stable callback fixed it, and the test now models the
  scene's load-effect dependency rather than an inert rendering placeholder.
- Blender's installed FBX export operator had incomplete RNA properties. The
  synthetic fixture uses the real exporter API directly with factory startup;
  the operator did not alter the Blender installation or use a licensed asset as
  substitute input.
- No real pack mappings, floors JSON, trusted cache pointers, published model
  bytes or release checkpoints were modified. No licensed files were committed.

## Review and installation

Work was inline under Kirk's explicit no-agent/no-heavy-review direction. Parent
source/integration inspection is recorded on the PRs; no independent-agent review
is claimed. Human review remains pending. Existing normal Lab behavior is covered
by its unchanged tests and default paths.

Merge Web1087, then Assets219. After both merges, deliberately update the clean
installed Assets checkout and the dedicated review Web checkout to the merged
capability. Preserve local changes and stop only an owned Lab before changing its
checkout. The launcher will refuse old Web code rather than reset it or pretend
material mode is available.

The command is then `asset-review --workspace PACK --material-profile PROFILE.json`.
An absent profile prompts before creating an unresolved template. Options and
profile changes remain preview-only. Kirk supplies his own pack/profile and
judges the interaction; his material decisions are not made by this session.
