# Weapon Presentation Wave Delivery Playbook

## Purpose

This is the durable delivery contract for adding reviewed player-equipment presentation through the existing runtime hand sockets. It prevents each weapon batch from reopening settled technical questions.

A wave-specific design supplies only the changing facts: exact refs, supported slots, special visual constraints, deferred items, and acceptance matrix. Everything else defaults to this playbook.

Parent program: [rpg-project#302](https://github.com/KirkDiggler/rpg-project/issues/302).

## Working agreement

Kirk owns:

- wave scope and product priority;
- visual candidate, silhouette, size, and placement judgment;
- licensing/security decisions requiring owner authority;
- genuinely balanced scope/cost choices;
- irreversible publication decisions; and
- PR merge approval.

The assets agent owns:

- source-query completeness and semantic classification;
- coordinate math, normalization, derivative authority, and socket compatibility;
- deterministic provider mechanics, validation, rollback, and tests;
- exact-ref consumer design and owner-authority boundaries;
- Gallery and browser evidence mechanics;
- independent technical review, finding disposition, and published verdicts;
- state restoration and cleanup; and
- cross-repository sequencing and Project 19 status.

The agent does **not** ask Kirk to repeatedly approve these settled technical mechanics. Kirk is interrupted during delivery only for a visual verdict, an unanticipated scope/licensing/security decision, a truly balanced tradeoff without a dominant engineering answer, or merge approval.

A new wave needs one design approval only when it changes the established contract. A list of exact refs using existing slots and this lifecycle may proceed from a concise wave brief plus Kirk's visual checkpoints.

## Invariants

1. Character GLBs remain unarmed; equipment attaches dynamically beneath cloned rig bones.
2. Existing rig-family sockets are immutable unless a dedicated calibration journey replaces them.
3. Provider assets own scale, axes, grip, roll, atlas, normals, tangents, and any left/right derivative.
4. Web contains no item × class × race transform table and does not mirror a provider asset at runtime.
5. Exact owner-private `CharacterData.equipped` refs drive presentation. Attack refs and peer guesses do not.
6. Unknown, attack-shaped, unavailable, and unsupported slot/ref combinations render nothing.
7. Main and off slots fail and clean up independently; one failure cannot remove the character or other slot.
8. Existing accepted GLBs and resolver behavior remain byte/behavior compatible.
9. Licensed sources, derivatives, checkpoints, and promoted GLBs stay private. Public web tracks no licensed GLBs.
10. API, proto, toolkit, inventory, legality, AC, damage, and persistence changes are out of scope unless the wave explicitly becomes a gameplay journey.
11. Git evidence outranks prose: tracked files, branch topology, commit IDs, manifests, hashes, and fresh command output are authoritative.
12. All integration is PR-only from isolated worktrees.

## Wave brief

Before Learn starts, record:

- exact `dnd5e:item:*` refs;
- supported semantic slots for each ref;
- current rig families and socket profiles;
- visual/product constraints;
- explicit deferred behavior;
- representative authority sequences; and
- any exception to this playbook.

An exact ref omitted from a slot allowlist remains unsupported there even if another slot has a model. Absence is preferable to a misleading substitution.

## Stage 1 — Learn

### Entry

- Parent Journey and wave brief are linked.
- Current provider `origin/main` and required predecessor merges are exact.
- A clean isolated `rpg-game-assets` worktree exists.
- No other operator's local continuity file is overwritten.

### Work

1. Query licensed inventories broadly and deterministically.
2. Classify every match exactly once as candidate or explicit rejection.
3. Record private source/provenance, dimensions, hierarchy, material, atlas, and semantic facts.
4. Generate source and grip review surfaces without promoting runtime mappings.
5. Preserve Kirk's active Blender viewport; experiments use separate files/windows.
6. Kirk selects or rejects each candidate and owns final placement judgment.
7. Derive normalization and any left/right variant through deterministic provider code.
8. Verify current rig families in Idle and Walk.
9. Rebuild established authored derivatives byte-for-byte.
10. Preserve every existing runtime GLB hash.

### Exit

- Candidate universe is closed for every target.
- Each accepted target has Kirk's recorded visual verdict.
- Each derivative has sealed deterministic authority.
- Provider-ready receipts are complete and private.
- Existing provider bytes remain unchanged.
- Full private tests pass.
- A current-head independent review verdict is published.

A rejected target may exit Learn as explicitly unaccepted. It does not receive a substitute merely to complete the batch.

## Stage 2 — Provider Build

### Entry

- Build starts only from the exact merged Learn commit.
- Tracked HEAD is clean.
- Inputs and expected outputs are sealed before staging.
- Existing provider manifests and canonical hashes are captured as the baseline.

### Work

1. Stage all cumulative outputs away from the canonical tree.
2. Generate normalized/provider-baked GLBs and manifests deterministically.
3. Validate the complete staged release before apply.
4. Render review evidence from immutable validated stage bytes.
5. Seal every release-tree record, not only newly promoted targets.
6. Apply transactionally with rollback on any failure.
7. Rebuild from clean tracked HEAD and compare byte-for-byte.

### GLB contract

Unless a wave design explicitly strengthens it, every standalone weapon is:

- glTF 2.0;
- static with identity root;
- true-meter canonical grip space;
- finite and deterministic;
- equipped with `POSITION`, `NORMAL`, `TEXCOORD_0`, and `TANGENT`;
- backed by one 1024×1024 source-compatible atlas;
- no more than 4.5 MiB base RGBA8 decoded texture memory; and
- free of skin, animation, camera, and light payloads.

### Exit

- New refs/hashes and manifest hash are exact.
- Every previous canonical GLB hash is unchanged.
- Stage, apply, rollback, complete-tree seal, and clean rebuild checks pass.
- Kirk approves sealed visual evidence.
- Full provider tests pass.
- A current-head independent review verdict is published.

## Stage 3 — Weapon Gallery

The stable Weapon Gallery is the normal-game authority bench.

### Requirements

- Use repository/service-backed state, never raw Redis/storage patching.
- Keep a stable character identity.
- Normalize idempotently.
- Preserve unrelated inventory and equipment.
- Add exact catalog refs without destructive replacement.
- Supply inventory identity/quantity required by the wave's normal equip sequence.
- Do not implement proficiency, legality, damage, AC, or presentation rules.

### Exit

- Catalog count and exact refs match the provider release.
- Repeated seeding converges without duplication or equipment loss.
- Focused and full API checks pass.
- Existing production behavior remains untouched.

## Stage 4 — Web Consumer

### Entry

- Provider and Gallery predecessor PRs are merged.
- Web work starts from fresh `origin/dev` in an isolated worktree.
- Ignored assets sync from a detached checkout of the exact provider merge.
- Manifest and every exercised GLB hash are verified before mapping.

### Work

1. Add exact refs only to the supported semantic slot catalogs.
2. Preserve existing URL, socket, hash, and resolver behavior.
3. Keep owner-only routing from private character data.
4. Exercise the real shared renderer in a production-backed Concept.
5. Cover Idle, Walk, rig families, relevant classes/races, facings, and replacement/cleanup.
6. Probe every exercised provider asset through HTTP and compare exact hash/size.
7. Use normal visible UI for equip, reconnect, unequip, and reconnect authority proof.
8. Restore exact initial owner state in a fresh context.

### Evidence

Evidence receipts bind:

- web code head and evidence-only head;
- exact provider merge and manifest;
- exact Gallery fixture merge;
- HTTP status, byte size, and SHA-256 for exercised GLBs;
- Concept observation matrix and browser error arrays;
- normal-game RPC statuses and authority snapshots;
- initial/final equipment, AC, HP, speed, and damage parity;
- media byte hashes, decoded-pixel hashes, dimensions, and metadata checks; and
- expected context-close stream aborts separately from unexpected failures.

Evidence images and text must expose no private character IDs, join codes, tokens, absolute local paths, licensed source paths, or licensed binaries.

### Exit

- Focused tests, typecheck, full suite, lint, format, build, and project CI pass.
- Normal-game presentation and reconnect restoration pass.
- Initial owner state is restored exactly.
- Tracked licensed GLB count is zero.
- Final evidence follows the code head with no source drift.
- A current-head independent review verdict is published directly on the PR.
- All required GitHub checks pass.

## Review policy

Substantive final reviews use a fresh independent session and inspect the complete exact range, source, tests, manifests, evidence receipts, images, authority records, privacy boundary, and verification output.

The published verdict includes:

- reviewed base and head;
- Critical, Important, and Minor counts;
- readiness;
- verification scope; and
- disposition of prior findings after any fix.

A stale-head or off-PR verdict does not satisfy the gate. Valid findings are fixed with targeted regressions, evidence is rebound when behavior changes, full verification is rerun, and a fresh rereview is published.

Model assignment follows current project policy in `active.md`; model names are not permanently frozen in this playbook. Copilot is used only when Kirk explicitly enables it.

## GitHub and Project 19

- The parent Journey stays In Progress across the wave.
- Create only the next actionable issue when its predecessor provides the exact contract.
- Link each issue beneath the Journey and add it to Project 19.
- Status flows Todo → In Progress → In Review → Done from real work state.
- Every issue/PR/comment uses the selected Team charter signature with the authenticated login.
- Merge order is Learn → provider → Gallery → web.
- No consumer PR claims readiness before predecessor merges and exact-byte sync.

## Failure and recovery

| Failure | Required response |
| --- | --- |
| Project 19 unavailable | Label it unavailable, continue only with human direction, and do not invent board state. |
| Active operator differs | Preserve the file and stop. |
| Candidate is visually or semantically weak | Reject it; do not substitute. |
| Asset needs a web correction | Return to provider normalization. |
| Input seal or tracked HEAD is dirty | Block promotion. |
| Stage validation fails | Do not apply. |
| Apply fails | Roll back to exact pre-build bytes. |
| Asset or bone load fails in web | Preserve character and other slot; render the failed slot empty. |
| Authority or reconnect proof fails | Diagnose the actual state/RPC/stream boundary; do not add timing guesses. |
| Evidence changes after code review | Rebind receipt and obtain current-head review. |
| Initial owner state cannot be restored | Do not publish or close the wave. |

## Completion and cleanup

After merge:

1. Verify exact merge commits and GitHub checks.
2. Close child issues and move Project 19 items to Done.
3. Post the exact provider/web hashes and verification summary to the Journey.
4. Restore and freshly verify normal-game owner state.
5. Stop only wave-specific previews or disposable services.
6. Remove merged worktrees, local branches, detached provider checkouts, temporary evidence files, and private review windows.
7. Preserve managed shared services unless the operator explicitly requests shutdown.
8. Update `active.md` with merged truth and the next focus.

A wave is not complete merely because code merged; authority restoration, published review, board state, and cleanup are part of delivery.

— assets agent, on behalf of KirkDiggler
