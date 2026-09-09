# Palette-aware, Resumable Asset Release Design

Status: human approved; planning complete; implementation is not complete

Journey: [rpg-project#365](https://github.com/KirkDiggler/rpg-project/issues/365)

Design slice: [rpg-project#412](https://github.com/KirkDiggler/rpg-project/issues/412)

Builds on: [world-asset review #394](../world-asset-review/design.md) and the private Assets/Web delivery from that slice

## Purpose

Connect the delivered local Asset Review Lab to a repeatable release path without replacing its trust, validation, or transaction engines.
An operator selects one audited palette appearance for a trusted source, explicitly marks the resulting bytes Ready, exports once, and lets an Assets-owned runner advance that frozen export to two human merge gates.
The result is less shell choreography, not unattended publication.

```text
registered private cache -> optional review preparation -> palette selection -> Ready export
  -> release start -> Assets stage/validate/apply/check -> Provider PR -> human merge
  -> explicit resume -> verify exact merged provider -> Web sync/check/proof -> Web PR -> human merge
```

## Operator outcome

1. Open a named local workspace; no global shell variables are required.
2. Optionally prepare or refresh the ignored Lab queue before review.
3. Choose a prebuilt compatible palette in a simple Palette dropdown, inspect the model, and mark it Ready.
4. Set or explicitly generate the batch ID in the Lab, then export Ready JSON without editing JSON by hand.
5. Start a release from that unchanged Ready file; start never prepares candidates, regenerates variants, or resets the review queue.
6. Let the runner create/reconcile isolated Assets and Web issue worktrees, run existing checks, commit, push, and open the Provider PR under the requested policy.
7. Merge the Provider PR manually, then resume; the runner queries GitHub, validates the exact merged provider snapshot, prepares proof, and opens the Web PR.
8. Merge the Web PR manually. `status` and reruns report the same completed checkpoints without duplicate writes or PRs.

## Existing versus new

| Concern | Existing authority/seam | New narrow behavior |
| --- | --- | --- |
| Pack appearance | Dark Fortress config names A/B/C and keeps C as `defaultAtlas` | Offer only explicitly configured **and audited compatible** alternatives |
| Variant output | `build_synty_palette_variants.py` builds isolated comparison GLBs | Publish a strict source-matched alternatives descriptor for prebuilt verified GLBs |
| Review | Manifest-backed queue, local Lab, Keep/Ready, Ready-only schema-v1 export | One Palette dropdown; selection participates in identity and Ready validation |
| Materials | Preflight and exact slot overrides distinguish main atlas from auxiliary materials | Swap only the main atlas; preserve auxiliary base color, normal, wrapping, and slot assignment |
| Promotion | `promote_world_assets.py` owns stage/validate/apply/check and atomic installation | Resolve and promote the exact selected variant through those same engines |
| Consumer | Exact refs, generated provider commit binding, strict sync/check | One selected appearance becomes the bytes behind each promoted ref |
| Thumbnails | Key is already `[ref, glbSha256]` | No contract change; selected bytes naturally produce a new thumbnail key |
| Release operations | Human runbook composes Git, GitHub, promotion, and Web commands | Thin Assets CLI journals, reconciles, and resumes the same operations |
| Work authority | GitHub issues and PRs | Remains GitHub; local receipts are execution evidence only |

## Slice 1 — palette-aware review and export

A pack exposes an alternative only when its reviewed config names the palette and material audit proves that the source is `ready-default` or `ready-explicit` under that main-atlas substitution.
Generation retains the physical identity `(packSlug, packVersion, sourcePath)` and every reviewed auxiliary binding.
It produces one verified GLB per offered palette; arbitrary texture upload, shader editing, and silent material fallback are forbidden.
The Lab dropdown points at those local prebuilt GLBs; it does not rebuild assets in the browser. Each alternative carries its own measured bounds, eligibility and budget facts; the original candidate's eligibility cannot approve different selected bytes.
Exactly one appearance is selected per review entry and promoted ref; this is not simultaneous runtime variants.

Current palette comparison output is preview-only: its derived conversion manifest uses a suffixed pack slug such as `polygon-dark-fortress-palette-A`, while review preparation requires one original pack slug.
Implementation must adapt generation/preparation to emit and consume a source-matched alternatives record; it must not claim current comparison directories are already promotable provider authority.
Variant files remain outside canonical `glbs/`, and the canonical default-C cache is not rewritten.

Changing palette or any palette descriptor hash invalidates the loaded-preview success flag and demotes Ready to Keep.
Imported progress restores Ready only when source and complete selected-appearance identities match current catalog bytes.
A stale import may report its old decision, but never approves different bytes.
The Lab supplies an editable `batchId` field plus an explicit collision-safe Generate action, so manual JSON edits are exceptional rather than routine.

### Versioned palette-selection descriptor

New palette-aware review/provider exports use schema version 2 at the document level, retaining the batch envelope and requiring `paletteSelection` on each entry. Explicit null selects its original/default hash-bound source; an object selects a verified alternative. This permits mixed original/alternate batches and candidates with no available alternates without forcing a texture change. The following is an illustrative entry fragment, not a complete export; bracketed hashes/paths describe fields rather than literal values:

```json
{
  "source": {
    "packSlug": "polygon-dark-fortress",
    "packVersion": "v3",
    "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Example_01.fbx",
    "glbSha256": "<established default/source GLB hash>"
  },
  "paletteSelection": {
    "descriptorVersion": 1,
    "comparisonId": "<stable generated set id>",
    "palette": "A",
    "paletteDescriptor": {"path": "<cache-relative palette record>", "sha256": "<record hash>"},
    "packConfigSha256": "<reviewed config hash>",
    "atlas": {"path": "<configured source-relative atlas>", "sha256": "<atlas hash>"},
    "selectedGlb": {"path": "<cache-relative alternative path>", "sha256": "<GLB hash>"}
  }
}
```

The provider resolves the descriptor through the registered cache, then verifies original pack/source identity, config hash, configured palette name, audited compatibility, atlas path/hash, selected GLB path/hash, and containment before normalization.
The nested descriptor preserves physical source identity while making chosen generated bytes unambiguous.
Schema-v1 review/provider exports retain their established meaning: absence of `paletteSelection` means the canonical default, including Dark Fortress palette C.
Existing schema-v1 recipes, default-C outputs, refs, and exact bytes are not migrated or regenerated merely to adopt schema v2.

## Slice 2 — Assets-owned local release runner

The runner lives in `rpg-game-assets` and wraps existing preparation, promotion validators/transactions, complete-inventory generation, Web sync/check, Git, and GitHub operations.
It does not belong to Pi or the team harness, and it is not a second implementation of those engines.
An operator provides a named workspace plus an already Ready JSON file.
Versioned workspace config records named Assets/Web repository roots and remotes, base branches, workspace/output bases, registered cache roots, optional licensed source roots, source/cache registration facts, and PR automation policy.
It does not require precreated per-batch branches or worktrees; the runner creates issue branches/worktrees under configured bases and reconciles existing ones.
Provider promotion and Web work use separate clean worktrees; neither canonical checkouts nor cache roots are mutated, and cache copies never use hard links.

Proposed command surface:

```bash
python3 scripts/world_asset_release.py review-prepare --workspace dark-fortress --match 'SourceFiles/DarkFortress/FBX/SM_Prop_*.fbx'
python3 scripts/world_asset_release.py start --workspace dark-fortress --ready ~/asset-releases/tiny-dark-fortress-ready.json
python3 scripts/world_asset_release.py status --workspace dark-fortress --batch tiny-dark-fortress
python3 scripts/world_asset_release.py resume --workspace dark-fortress --batch tiny-dark-fortress
```

`review-prepare` is optional and separate; it may prepare the ignored queue but cannot start a release.
`start` freezes and hashes the existing Ready export, obtains a per-batch execution lock, creates/reconciles issue worktrees, invokes stage/validate/apply/check, prepares scripted evidence, and—when workspace policy requests it—commits, pushes, and opens the Provider PR.
Commit and PR bodies include Team/operator attribution, exact inputs, outputs, warnings, commands, and links.
`resume` advances only the next verified checkpoint; there is no daemon, scheduled polling, auto-approval, or auto-merge.
`start` takes the batch identity from the frozen Ready JSON; `status` and `resume` explicitly select that batch and never guess which export is active. The example file above carries `batchId: tiny-dark-fortress`.

A schema-versioned receipt and append-only checkpoint journal live under the configured local receipt base, outside the tracked provider and Web trees.
They record workflow/tool versions, frozen Ready hash, selected source/palette/config/atlas/GLB hashes, warnings and acceptance binding, stage/apply/check results, commits, remote heads, PR links, merge commits, Web catalog hash, and proof outputs.
They are not approval, project status, or merge authority: GitHub remains durable issue/PR truth, and validators plus actual bytes remain artifact truth.

At the Provider PR checkpoint the runner stops.
Explicit `resume` reads the actual PR and merge commit from GitHub, fetches that commit, and validates its exact recipe, runtime GLBs, receipts, catalog, mesh stats, and complete inventory against the frozen release.
If unrelated `main` commits landed later, Web binding uses an isolated checkout of the verified merge commit rather than chasing current `main` or overwriting a canonical root.
If the merge is not present verbatim because provider changes overlap or are incompatible, resume stops for explicit reconciliation and revision.
Only after verified binding does it sync/check the Web worktree, run scripted proof, commit/push/open the Web PR, and stop again.

## Human gates, warnings, and evidence

Human decisions remain: visual review and Ready, acceptance of each new warning, disposition of unexpected failures, Provider PR merge, and Web PR merge.
Normal validators, regression checks, browser/visual proof capture, and concise PR/evidence summaries are scripted; premium-agent availability is not a prerequisite.
Warning approval binds the exact sorted warning set, Ready/input hashes, selected artifact hashes, stage hash, and validator versions.
Any warning delta reopens the gate; there is no blanket `accept-all`, count threshold, or hard-limit bypass.
When warning approval is needed, status prints the affected assets, reasons, and an exact approval command: `resume --workspace NAME --batch ID --approve-warnings DIGEST`. The digest identifies the complete warning/input/artifact binding above; the runner revalidates it before apply and records the operator's explicit approval.
Human documentation includes a concise worked outcome with commands, receipt link, Provider/Web PR links, merge commits, warnings, and visible result—not hundreds of shell assertions.

## Retry and checkpoint rules

| Interrupted point | Reconciliation before retry | Safe next action |
| --- | --- | --- |
| Before frozen input | Rehash Ready export and palette descriptors | Start only if current; otherwise export a revision |
| Stage/validate | Verify frozen hashes and stage seal | Reuse exact valid stage or create a fresh stage; never edit it |
| Apply | Compare canonical targets and transaction result to intended hashes | Validate completed atomic apply or restage; never repeat a partial write blindly |
| Commit | Inspect worktree/index and exact HEAD/tree | Reuse matching commit or stop on unrelated/dirty state |
| Push | Fetch exact remote branch head | Push only missing matching commit; never force over divergence |
| PR creation timeout | Query GitHub by repo/base/head and verify returned PR | Reuse one exact PR or stop on ambiguity; never create duplicates blindly |
| Provider merge wait | Query PR and fetch recorded merge commit | Stay paused, or validate the exact merged snapshot |
| Web preparation/PR | Verify provider binding, Web HEAD/diff, remote branch, and PR | Continue once or reuse exact completed state |

One per-batch lock prevents concurrent mutating executions.
`status` is read-only and derives checkpoint state from journal plus live filesystem, Git, and GitHub observations.
A receipt alone never skips validation or proves a remote mutation.
Any Ready file, descriptor, cache registration, selected GLB, config, branch base, or governed output drift after `start` requires an explicit revision/restage record; it cannot be blessed by editing a receipt.

## Cumulative authority and minimal integration

Today world-asset promotion intentionally regenerates the cumulative catalog, mesh stats, complete inventory, and receipts for all world-asset recipes.
Current Assets tests also hard-pin cumulative inventory facts (`3225` files and tree SHA-256 `4726075ec7b9f1c2f0d5f6eefa1d562a2ecc4f0be1ac99639c9d76bc2f9b10b3`) and contain older path-partition/count seals.
Historical weapon/evidence assertions and their original hashes remain immutable.
`evidence/117-all-race-hair/verification.json` also carries the live `providerMetadata.inventory` and `providerMetadata.meshStats` pointers used by later provider closure checks while retaining its accepted #117 visual/hair authority; a normal world-asset batch must not hand-edit or reinterpret that historical evidence.

Minimal implementation makes the world-asset governed target set and cumulative evidence derive deterministically from tracked recipes/current bytes, validates the derived result independently, and updates only those explicitly designated live cumulative pointers.
A normal new batch must not require teaching old tests a new literal count/hash, editing historical weapon seals, or rewriting accepted evidence to agree with itself.
This is not permission for a broad repository test redesign: focused generator/validator seams should remove the recurring manual pin only where additive world-asset releases touch it, while preserving independent non-tautological completeness checks.

## End-to-end acceptance

Use a tiny, as-yet-unpromoted trusted Dark Fortress source selection and a compatible configured nondefault palette.
The Lab previews the exact alternative with auxiliary materials intact, exports schema-v2 Ready JSON, and visibly demotes Ready after any palette/hash change.
`start` reaches a checked Provider PR and pauses; the PR/receipt agree on physical source, palette/config/atlas/selected GLB hashes, normalized output bytes, warnings, and cumulative metadata.
After human merge, `resume` verifies the actual merge commit from an isolated checkout, binds Web metadata to it, runs sync/check and scripted visual/regression proof, opens the Web PR, and pauses.
World Builder bytes and existing hash-keyed thumbnails agree with the selected GLB; one exact ref has one appearance.
Repeated `status`, `start`, and `resume` reconcile to the same commits and PRs without queue reset, canonical mutation, duplicate remote objects, or changed output.
The worked human doc records receipt and both PR links plus the visible accepted outcome.

## Non-goals

- External shared-cache relocation or a new content-addressed cache service; register the existing cache as a compatibility bridge.
- Hosted ingestion/review services, daemons, agent orchestration requirements, automatic merges, or a second task tracker.
- Arbitrary texture upload, shader editing, unknown material fallback, or simultaneous palette variants for one ref.
- NPC, Toolkit/rulebook, item/equipment, collision, or gameplay-behavior binding.
- Multi-hex occupancy or work from paused #400/#975.
- Mutation of canonical checkouts, licensed source/cache bytes, legacy schema-v1 recipes, default palette-C bytes, historical weapon seals, or thumbnail identity.

## Initial operating defaults

- Named workspace config: `${XDG_CONFIG_HOME:-$HOME/.config}/rpg-game-assets/workspaces/NAME.json`; explicitly created by a setup command, never silently installed globally. It stores paths and allowed repositories, not credentials.
- Local evidence/journal: `${XDG_STATE_HOME:-$HOME/.local/state}/rpg-game-assets/releases/WORKSPACE/BATCH_ID/`. No automatic expiry or deletion; explicit cleanup is separate from release execution and retains the final receipt/PR links.
- Workspace `publicationMode: pr` enables the requested commit/push/PR automation. An explicit `--local-only` stops before push/PR and cannot change policy for later invocations. Both modes prohibit automatic merge.
- Assets branch: `asset/ISSUE-world-assets-BATCH_ID`; Web branch: `asset/ISSUE-world-assets-BATCH_ID` in its separate repository. Names are validated, existing branches are reconciled, and unrelated branches are never reused or overwritten.

These are technical defaults for detailed review, not additional product decisions or a broader configuration framework.
