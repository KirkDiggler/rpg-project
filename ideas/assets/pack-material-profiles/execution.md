# Profile-and-preview delivery checkpoint

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
