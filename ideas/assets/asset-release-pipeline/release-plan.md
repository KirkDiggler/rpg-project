# Resumable Local Asset Release Runner Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
**Goal:** Add an Assets-owned local CLI that freezes an existing Ready export, safely prepares/reconciles Provider and Web PRs, and resumes across two human merge gates without replacing current validators or GitHub authority.
**Architecture:** A small checkpoint state machine records hash-chained local execution evidence under an XDG state root and delegates artifact work to existing review, promotion, Web sync/check, Git, and GitHub commands. Every mutating step is guarded by a per-batch lock and live reconciliation; status performs only read operations, and exact merged snapshots are validated in isolated checkouts rather than following a moving main branch.
**Tech Stack:** Python 3 stdlib (`argparse`, `dataclasses`, `fcntl`, `hashlib`, `json`, `pathlib`, `subprocess`, `urllib` only if needed); existing Assets Python engines; Git and GitHub CLI; Web Node/npm/Playwright already pinned by the configured Web checkout.
**Spec:** `ideas/assets/asset-release-pipeline/design.md` (human approved; planning complete; implementation is not complete)

## Global Constraints

- Begin only after the palette Assets contract from `palette-plan.md` has merged to Assets `main` and the compatible Lab has landed on Web `dev`. Implement the runner under a new Assets issue/branch from current `origin/main`.
- `review-prepare` is separate and optional. `start` consumes an unchanged, non-empty Ready JSON, takes its `batchId`, freezes it once, and never prepares candidates, generates variants, resets the Lab queue, or guesses an active batch.
- Named workspace setup is explicit at `${XDG_CONFIG_HOME:-$HOME/.config}/rpg-game-assets/workspaces/NAME.json`. Local journals/receipts are outside both repositories at `${XDG_STATE_HOME:-$HOME/.local/state}/rpg-game-assets/releases/WORKSPACE/BATCH_ID/`.
- `publicationMode: "pr"` may create/reconcile issues, worktrees, commits, pushes, and PRs. It never merges. `--local-only` is per call, stops before the next remote mutation, and cannot change or bless publication state for a later resume.
- GitHub issues/PRs remain work and merge authority. Local journal/receipt/summary files are execution evidence only; they never authorize skipping validation or become a second task store.
- Use one Assets release branch/worktree and one Web release branch/worktree per batch, each named `asset/ISSUE-world-assets-BATCH_ID`. Do not touch canonical checkout bytes, cache/source bytes, unrelated worktrees, or branch per finding.
- No force push, automatic merge, shell interpolation, `shell=True`, daemon, hosted service, cache relocation, hard link, or automatic warning acceptance. No premium-agent dependency is permitted in normal use.
- Preserve stage authority: only `promote_world_assets.py stage` recreates a stage. The runner never manually removes/edits `release/`; interrupted apply recovery compares recorded prestate/intended hashes before any retry.
- Validate the exact Provider merge commit and exact Web merge commit. Do not require a later unrelated Assets `main` head and do not mutate a canonical `main` checkout to prove a merge.
- Tests use synthetic Ready files, fake command executors/GitHub responses, temporary local repositories, and bare local Git remotes. Tests never contact GitHub or create real issues/PRs.

---

## File Responsibility Map

| File | Responsibility |
| --- | --- |
| `scripts/world_asset_release.py` | Thin CLI/parser for `setup`, `review-prepare`, `start`, `status`, `resume`, and `revise`; no workflow logic. |
| `scripts/world_asset_release_state.py` | Strict workspace schema, path/cache registration validation, frozen revisions, per-batch lock, hash-chained journal, derived receipt/summary, canonical hashing. |
| `scripts/world_asset_release_commands.py` | `CommandExecutor` protocol/real implementation, tool version probes, safe argv/env handling, local Git/worktree/remote reconciliation, read/write GitHub API adapters, Team/operator signature loading. |
| `scripts/world_asset_release_workflow.py` | Checkpoint state machine, stage/warning/apply recovery, Provider/Web preparation, exact merge validation, local-only stops, and read-only status observations. |
| `scripts/promote_world_assets.py` | Small machine-readable stage-seal/report seam over the existing stage/validate/apply/check engine; artifact behavior stays in the existing functions. |
| `scripts/capture_world_asset_release_evidence.mjs` | Bounded Playwright proof against the configured local Web server; writes screenshot and non-licensed JSON evidence only under release state. |
| `scripts/test_world_asset_release_{state,commands,workflow}.py` | Config/journal/locking/digest, argv/Git/GitHub reconciliation, checkpoint/recovery/idempotency tests. |
| `scripts/test_capture_world_asset_release_evidence.mjs` | Node argument/report normalization tests without loading licensed assets. |
| `scripts/test_promote_world_assets.py` | Stage-seal/target/prestate report regression tests. |
| `docs/human/asset-ingestion/world-asset-batch-runbook.md` | Replace shell choreography with setup/start/status/resume/revise commands, receipt protocol, recovery, and two explicit merge pauses. |
| Local generated `STATE/WORKSPACE/BATCH/{receipt.json,journal.jsonl,summary.md,evidence/*}` | Execution facts and concise human outcome; never committed automatically and never work authority. |

## Task 1: Strict workspace, command, lock, journal, and revision foundations

**Files:**
- Create: `scripts/world_asset_release_state.py`
- Create: `scripts/world_asset_release_commands.py`
- Create: `scripts/test_world_asset_release_state.py`
- Create: `scripts/test_world_asset_release_commands.py`
- Create: `scripts/world_asset_release.py` with `setup`, `status` parsing only in this task
**Interfaces:**
- Consumes: XDG config/state environment, canonical Assets Team charter, authenticated `gh api user`, and configured repository/cache roots.
- Produces: `WorkspaceConfig.load(name: str) -> WorkspaceConfig`, `ReleaseStore.open(workspace: str, batch_id: str) -> ReleaseStore`, `ReleaseStore.lock(exclusive: bool) -> ContextManager`, `freeze_ready(path: Path, store: ReleaseStore) -> FrozenRevision`, `revise_ready(path: Path, store: ReleaseStore) -> FrozenRevision`, `CommandExecutor.run(argv: Sequence[str], *, cwd: Path, env: Mapping[str, str] | None = None, stdin: bytes | None = None) -> CommandResult`, and `load_actor(config, executor) -> Actor(team: str, login: str, signature: str)`.
- [ ] **Step 1: Write failing workspace, journal, and command-safety tests**

```python
def test_journal_is_append_only_hash_chained_and_receipt_is_derived(self):
    store.append("input-frozen", {"readySha256": "a" * 64})
    store.append("stage-valid", {"stageSha256": "b" * 64})
    events = store.read_verified_events()
    self.assertEqual([1, 2], [event.sequence for event in events])
    self.assertEqual(events[0].event_sha256, events[1].previous_event_sha256)
    self.assertEqual("b" * 64, store.derive_receipt()["stage"]["sha256"])
def test_executor_never_uses_a_shell_or_unvalidated_tool_arguments(self):
    executor.run([self.git, "status", "--porcelain=v1"], cwd=self.repo)
    self.assertEqual(False, self.recorded_kwargs["shell"])
    with self.assertRaisesRegex(ValueError, "absolute executable"):
        WorkspaceConfig.parse(self.config_with_tool("git; rm -rf /"))
```

Also test exclusive lock contention, shared status lock, symlink/overlap roots, bad names/remotes/base branches, unregistered cache facts, source/cache mutation, charter without the exact signature line, login lookup failure, truncated/tampered journal, and config file mode/path behavior.
- [ ] **Step 2: Run the foundation tests red**

```bash
python3 -m unittest -v scripts.test_world_asset_release_state scripts.test_world_asset_release_commands
```

Expected: imports fail because the state/command modules do not exist.
- [ ] **Step 3: Implement explicit setup and the exact workspace schema**
`setup` writes schema version 1 only after resolving real, non-symlink, pairwise-disjoint roots and confirming repository top levels/remotes. Use this concrete shape; tools are absolute executable paths, never command strings or embedded arguments:

```json
{
  "schemaVersion": 1,
  "publicationMode": "pr",
  "projectRoot": "/absolute/rpg-project",
  "teamCharter": "docs/teams/roles/assets/prompt.md",
  "assets": {"root": "/absolute/rpg-game-assets", "repo": "KirkDiggler/rpg-game-assets", "remote": "origin", "base": "main"},
  "web": {"root": "/absolute/rpg-dnd5e-web", "repo": "KirkDiggler/rpg-dnd5e-web", "remote": "origin", "base": "dev"},
  "worktreeBase": "/absolute/worktrees/asset-releases",
  "stageBase": "/absolute/state/asset-release-stages",
  "receiptBase": "/absolute/state/rpg-game-assets/releases",
  "cache": {"root": "/absolute/rpg-game-assets-cache", "packSlug": "polygon-dark-fortress", "packVersion": "v3", "registrationSha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
  "source": {"root": "/optional/absolute/licensed-source", "registrationSha256": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
  "tools": {"git": "/usr/bin/git", "gh": "/usr/bin/gh", "node": "/usr/bin/node", "npm": "/usr/bin/npm", "blender": "/absolute/blender"}
}
```

The cache registration digest binds real root identity, pack/version, checked manifest hashes, and pack-config hash. Source is optional for normal runner use and is used only by `review-prepare`. Setup stores no credentials.
- [ ] **Step 4: Implement evidence-only state and safe commands**
Use `fcntl.flock`: exclusive for every mutating command and shared for local status reads. Store immutable Ready copies at `inputs/revision-NNNN.json`; fsync copy and directory, then rehash. Journal events are canonical one-line JSON with sequence, previous hash, event hash, RFC-3339 observation time, workflow version, revision number, and payload; command events include exact argv, cwd, tool version, exit code, and output hashes. Mutating workflows rewrite `receipt.json`/`summary.md` only as deterministic projections of verified events and live observations; `status` never rewrites them, and neither file can create a checkpoint.
Probe and record `python`, `git --version`, `gh --version`, `node --version`, `npm --version`, and `blender --version`. Invoke every process as an argv list with `shell=False`, validated cwd, closed stdin unless supplied, captured stdout/stderr, and a minimal explicit environment. Do not concatenate refs, paths, titles, or warning digests into shell text.
Load the exact signature template from the configured canonical Team charter and replace `<github-login>` with `gh api user --jq .login`; never store or assume a login in source.
- [ ] **Step 5: Implement a real immutable revision command and rerun green**

```bash
python3 scripts/world_asset_release.py revise \
  --workspace dark-fortress --batch tiny-dark-fortress \
  --ready "$HOME/asset-releases/tiny-dark-fortress-ready-r2.json"
```

`revise` requires the same `batchId`, records old/new Ready hashes and invalidated checkpoints, copies a new revision, and never edits the old copy. It is allowed before Provider merge; after merge it fails and requires a new batch. It invalidates stage/warning/apply/commit proof and does not reset browser review state or silently change a remote branch.

```bash
python3 -m unittest -v scripts.test_world_asset_release_state scripts.test_world_asset_release_commands
python3 scripts/world_asset_release.py --help
```

Expected: tests pass and help lists all six commands.
- [ ] **Step 6: Commit the independently reviewable foundation**

```bash
git add scripts/world_asset_release.py scripts/world_asset_release_state.py \
  scripts/world_asset_release_commands.py scripts/test_world_asset_release_state.py \
  scripts/test_world_asset_release_commands.py
git diff --cached --check
git commit -m "feat: add asset release workspace state"
```

## Task 2: Frozen start, stage authority, warning approval, and apply recovery

**Files:**
- Modify: `scripts/world_asset_release.py`
- Create: `scripts/world_asset_release_workflow.py`
- Create: `scripts/test_world_asset_release_workflow.py`
- Modify: `scripts/promote_world_assets.py`
- Modify: `scripts/test_promote_world_assets.py`
**Interfaces:**
- Consumes: Task 1 state/executor, palette-plan schema-v2 loader/resolver and cumulative targets, and existing `stage_batch`, `validate_stage`, `apply_stage`, `check_batch`.
- Produces: `stage_seal(repo_root: Path, provider_root: Path, recipe_path: Path, stage_root: Path) -> StageSeal`, `warning_approval_digest(revision: FrozenRevision, stage: StageSeal, warnings: tuple[WarningRecord, ...], tools: ToolVersions) -> str`, `ReleaseWorkflow.start(ready_path: Path, *, local_only: bool = False) -> StopReason`, and `ReleaseWorkflow.resume(batch_id: str, *, approve_warnings: str | None = None, local_only: bool = False, publish: bool = False) -> StopReason`.
- [ ] **Step 1: Add failing start/warning/recovery tests**

```python
def test_warning_delta_requires_exact_digest_before_apply(self):
    stop = workflow.start(self.ready)
    self.assertEqual("warning-approval-required", stop.code)
    self.assertIn("resume --workspace ws --batch batch --approve-warnings", stop.command)
    self.assertFalse(self.commands.called_with("promote_world_assets.py", "apply"))
    with self.assertRaisesRegex(ValueError, "warning approval digest"):
        workflow.resume(approve_warnings="0" * 64)
def test_interrupted_apply_reconciles_only_known_prestate_or_intended_bytes(self):
    self.fixture.install_mixed_known_apply_state()
    workflow.resume()
    self.assertTrue(self.fixture.every_target_matches_stage())
    self.fixture.corrupt_one_target()
    with self.assertRaisesRegex(ValueError, "unknown canonical target state"):
        workflow.resume()
```

Also prove start rejects non-Ready/empty/malformed input, freezes before mutation, takes batch ID only from JSON, never calls prepare/reset, reuses an exact stage, restages only through the engine, and detects input/descriptor/cache/config/tool drift.
- [ ] **Step 2: Run workflow tests red**

```bash
python3 -m unittest -v scripts.test_world_asset_release_workflow scripts.test_promote_world_assets
```

Expected: failure because stage reports and workflow transitions do not exist.
- [ ] **Step 3: Expose a small machine-readable stage seal, not another engine**
Extend promotion results with sorted install/removal targets and records `{path,state,sizeBytes,sha256}`. `stage_seal` calls current validation, hashes the validated release targets, records recipe/catalog/inventory/mesh/live-#117 pointer hashes, and extracts warning rows from staged mesh stats. Add `--result-json PATH` to promotion commands for the runner; ordinary CLI behavior remains compatible.
The runner invokes, in order, existing `stage`, `validate`, `apply`, and canonical `check` argv. It never deletes stage itself. The stage digest is SHA-256 of canonical target/removal records plus revision/input/variant/config hashes and promotion/normalizer versions.

Implement `review-prepare` as a separate delegation that requires registered cache facts and revalidates optional source facts when present, then runs `prepare_asset_review.py prepare --pack-root CACHE/library/PACK/VERSION --source-match MATCH --web-root WEB`. It forwards `--reset` only when the operator supplied that flag, never opens a release store, and cannot call `start`.

```bash
python3 scripts/world_asset_release.py review-prepare \
  --workspace dark-fortress --match 'SourceFiles/DarkFortress/FBX/SM_Prop_*.fbx'
```

- [ ] **Step 4: Implement start and exact warning approval**
`start --workspace NAME --ready FILE` acquires the lock, validates/freeze-copies once, and advances toward the Provider PR without preparing review data or resetting queues. Collect warnings only for incoming runtime paths, as sorted `(ref,file,reasons)` records, and compare against canonical pre-stage warnings so an unchanged old cumulative warning is not presented as new.
Bind approval to canonical JSON containing workflow/tool versions, Ready hash, all source/descriptor/config/atlas/original/selected GLB hashes, stage digest, and complete warning records. Status prints affected refs/reasons and this real command:

```bash
python3 scripts/world_asset_release.py resume \
  --workspace dark-fortress --batch tiny-dark-fortress \
  --approve-warnings 4d6f000000000000000000000000000000000000000000000000000000000000
```

There is no `accept-all`, count threshold, or hard-error bypass. A valid approval appends `warnings-approved` with the digest, complete binding, exact command, and current charter-derived actor; the receipt merely projects that event. A revision, tool/version change, restage, artifact drift, or warning delta changes the digest and reopens the gate.
- [ ] **Step 5: Reconcile interrupted apply from recorded authority**
Immediately before apply, journal every canonical target as exact hash/size or absent. If no apply-complete event exists on resume:
1. Revalidate the frozen revision and exact stage.
2. Compare every canonical install/removal target with its recorded prestate and intended state.
3. If all intended, run canonical `check` and record recovered completion.
4. If every target is either exact prestate or exact intended, invoke existing atomic `apply` once to converge, then `check`.
5. On any third state, dirty unrelated path, missing stage, or rollback uncertainty, stop and preserve logs/transaction directories; never `rm` or blindly repeat.
After apply, require the palette plan's generated cumulative inventory/catalog/stats/receipts and #117 live pointers in the stage target seal. Do not edit historical tests/seals or calculate acceptance from generated metadata alone.
- [ ] **Step 6: Run green and commit**

```bash
python3 -m unittest -v scripts.test_world_asset_release_workflow scripts.test_promote_world_assets
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
git add scripts/world_asset_release.py scripts/world_asset_release_workflow.py \
  scripts/test_world_asset_release_workflow.py scripts/promote_world_assets.py \
  scripts/test_promote_world_assets.py
git commit -m "feat: checkpoint local asset promotion"
```

## Task 3: Idempotent issues, worktrees, commits, pushes, and Provider gate

**Files:**
- Modify: `scripts/world_asset_release_commands.py`
- Modify: `scripts/world_asset_release_workflow.py`
- Modify: `scripts/test_world_asset_release_commands.py`
- Modify: `scripts/test_world_asset_release_workflow.py`
**Interfaces:**
- Consumes: Task 1 actor/executor and Task 2 verified Provider target seal.
- Produces: `reconcile_issue(repo, marker, title, body) -> IssueIdentity`, `reconcile_worktree(spec: WorktreeSpec) -> WorktreeIdentity`, `reconcile_push(spec: PushSpec) -> str`, `reconcile_pr(repo, base, head, title, body) -> PullRequestIdentity`, and Provider stop reason `provider-merge-required`.
- [ ] **Step 1: Add fake-GitHub and local-bare-remote tests**

```python
def test_pr_timeout_reconciles_by_repo_base_and_head_without_duplicate_create(self):
    fake.queue_create_timeout_then_list(existing_pr(base="main", head=self.branch))
    pr = reconcile_pr(self.repo, "main", self.branch, self.title, self.body)
    self.assertEqual(413, pr.number)
    self.assertEqual(1, fake.mutation_count("pr create"))
def test_diverged_remote_branch_stops_without_force_push(self):
    self.advance_local_and_remote_differently()
    with self.assertRaisesRegex(ValueError, "remote branch diverged"):
        reconcile_push(self.push_spec)
    self.assertNotIn("--force", self.executor.all_argv_text())
```

Cover issue-create timeout with exact hidden marker reconciliation, ambiguous matches, wrong repo/base/head, pre-existing matching worktree/branch/commit/remote/PR reuse, dirty/unrelated branches, remote fast-forward, local-only behavior, and signature derived from fake charter/login.
- [ ] **Step 2: Implement exact identity and read-after-write reconciliation**
Issue bodies contain a hidden stable marker keyed by workflow/workspace/batch/role, plus Ready hash, selected hashes, warnings, commands, and Team/operator signature. Query all states and verify exact marker/repo before create; after a timeout, query again and reuse one exact issue or stop on zero/ambiguity.
Create branches only after issue identity exists. Fetch explicit base refs, verify configured repo/remote, then create or reconcile the one branch/worktree under `worktreeBase`. Refuse canonical roots, cache roots, wrong branch/base, dirty/index/untracked state, or unrelated HEAD.
Commit only the exact stage-reported governed Assets paths and generated #117 live-pointer update. Verify staged names/hashes, `git diff --cached --check`, commit tree, and clean status. Push `HEAD:refs/heads/BRANCH` without force only when remote is absent or a strict ancestor; read `ls-remote` afterward.
PR identity is exact `(repo, base, head owner, head branch)`. Query before create and after any uncertain response; verify title/body marker/head SHA and reuse one exact open PR. Never merge or request a premium reviewer automatically.
- [ ] **Step 3: Enforce publicationMode and local-only crossings**
In `publicationMode: pr`, normal `start` may create/reconcile remote objects. With `--local-only`, stop immediately before the next issue/push/PR mutation and record only `local-only-stop`; do not create a fake issue number or alter workspace policy. A later invocation that would cross that stopped mutation requires explicit acknowledgement:

```bash
python3 scripts/world_asset_release.py resume \
  --workspace dark-fortress --batch tiny-dark-fortress --publish
```

`--publish` is valid only for that call, cannot approve warnings, and is rejected unless config still says `publicationMode: pr`. This prevents a local-only run from silently blessing publication on ordinary resume.
- [ ] **Step 4: Run green and commit the Provider gate**

```bash
python3 -m unittest -v scripts.test_world_asset_release_commands scripts.test_world_asset_release_workflow
# Tests use temporary git init --bare remotes and a fake gh executor only.
git diff --check
git add scripts/world_asset_release_commands.py scripts/world_asset_release_workflow.py \
  scripts/test_world_asset_release_commands.py scripts/test_world_asset_release_workflow.py
git commit -m "feat: reconcile asset release pull requests"
```

Expected: repeated start/resume yields the same issue, worktree, commit, remote head, and Provider PR, then stops with `provider-merge-required`.

## Task 4: Exact Provider merge, scripted Web proof, Web gate, and completion

**Files:**
- Create: `scripts/capture_world_asset_release_evidence.mjs`
- Create: `scripts/test_capture_world_asset_release_evidence.mjs`
- Modify: `scripts/world_asset_release_workflow.py`
- Modify: `scripts/test_world_asset_release_workflow.py`
- Modify: `docs/human/asset-ingestion/world-asset-batch-runbook.md`
**Interfaces:**
- Consumes: exact Provider PR identity/merge commit, isolated snapshot root, existing Web `world-assets:sync`, `world-assets:check`, typecheck/tests, Web Playwright installation, and Task 3 GitHub reconciliation.
- Produces: `validate_provider_merge(pr: PullRequestIdentity, frozen: FrozenRevision) -> ProviderBinding`, `captureBrowserEvidence(options: BrowserEvidenceOptions) -> Promise<BrowserEvidence>`, Web stop reason `web-merge-required`, and final status `complete` bound to exact Provider/Web merge commits.
- [ ] **Step 1: Add failing merge/binding/status tests**

```python
def test_provider_resume_validates_recorded_merge_not_newer_main(self):
    github.merge_provider(self.provider_merge)
    github.advance_main_with_unrelated_commit()
    workflow.resume()
    self.assertEqual(self.provider_merge, store.receipt()["provider"]["mergeCommit"])
    self.assertEqual(self.provider_merge, store.receipt()["web"]["providerCommit"])
def test_status_is_read_only_and_completes_only_after_exact_web_merge(self):
    before = snapshot_state_and_repositories()
    result = workflow.status()
    self.assertEqual(before, snapshot_state_and_repositories())
    self.assertNotEqual("complete", result.phase)
    github.merge_web(self.web_merge)
    self.assertEqual("complete", workflow.status().phase)
```

Cover closed-unmerged PR, wrong merge/base/head, merge tree without exact recipe/selected output, newer unrelated main, dirty canonical roots, duplicate Web PR ambiguity, and local receipt that falsely claims a remote mutation.
- [ ] **Step 2: Validate the exact merged Provider snapshot in isolation**
Read Provider PR state/merge commit from GitHub; do not infer merge from local receipt. Create/reuse a disposable snapshot clone under batch state, fetch the exact merge SHA, and detach at it. Run canonical provider `check` from that snapshot against registered cache and frozen recipe. Additionally compare frozen recipe bytes and each incoming selected source/output/receipt record with the validated stage binding. Allow unrelated files incorporated by a concurrent base update only when cumulative generators validate them at that exact merge.
Web sync must use this clean detached snapshot as `RPG_GAME_ASSETS_PATH`, so generated metadata binds the recorded merge even if `origin/main` later advances. Do not fast-forward or alter the configured canonical Assets checkout.
- [ ] **Step 3: Add bounded browser evidence without a hosted/premium dependency**
The Assets-owned `.mjs` script resolves Playwright from the configured Web root, checks its package/version, and accepts validated argv values `--web-root`, `--url`, repeated `--ref`, and `--output-root`. It listens for console/page/request/WebGL/asset-load errors, searches the World Builder's `Search assets` input, requires exactly one `[data-asset-ref="REF"]`, drags it to `world-building-canvas`, waits for `Real models loaded 1/1`, exports scene JSON, verifies the ref, saves/reloads/reopens, and verifies it remains. Store screenshot/video only under local state; write license-safe JSON with URL, refs, HTTP statuses, error arrays, visible labels, generated provider commit/catalog hash, and screenshot hash.

```javascript
test('normalizes evidence without embedding image bytes or local roots', () => {
  const report = browserEvidenceReport(fixtureObservation);
  assert.deepEqual(report.errors, []);
  assert.equal(report.assets[0].ref, 'dnd5e:props:dark-fortress:brazier_01');
  assert.ok(!JSON.stringify(report).includes('/home/'));
  assert.ok(!('screenshotBytes' in report));
});
```

The workflow starts Vite as a bounded child on loopback, waits for readiness, runs the script, and terminates the child in `finally`; it never leaves a daemon. Fake executor tests prove cleanup on success/failure. Require a zero-error report before Web commit/PR.
- [ ] **Step 4: Prepare Web once, stop at the second human gate, and complete read-only**
Create/reconcile the Web issue/worktree from configured `origin/dev`. Run exact argv/environment for:

```text
npm run world-assets:sync
npm run world-assets:check
npm test -- --run src/components/hex-grid/WorldAssetModel.test.tsx src/concepts/world-building/WorldBuildingViewport.test.tsx
npm run typecheck
```

Commit only generated license-safe metadata/integration files; synchronized GLBs and screenshots remain ignored/untracked and are never pushed. Reconcile the Web remote branch/PR exactly as Task 3 and stop with `web-merge-required`. No command merges either PR.
After human Web merge, `status` uses GitHub read APIs to verify exact repo/base/head, merged state/commit, expected generated-file blob and Provider commit binding; it does not fetch, write journals, touch worktrees, or acquire an exclusive lock. `resume` may append the final checkpoint and regenerate local `receipt.json`/`summary.md`. `summary.md` contains commands, warning digest/disposition, receipt link, both PR URLs/merge commits, selected hashes, and visible browser result—no licensed pixels or local source paths.
- [ ] **Step 5: Run the complete test/validation suite and commit**

```bash
python3 -m unittest -v \
  scripts.test_world_asset_release_state \
  scripts.test_world_asset_release_commands \
  scripts.test_world_asset_release_workflow \
  scripts.test_promote_world_assets
node --test scripts/test_capture_world_asset_release_evidence.mjs
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
git status --short
git add scripts/capture_world_asset_release_evidence.mjs \
  scripts/test_capture_world_asset_release_evidence.mjs \
  scripts/world_asset_release_workflow.py scripts/test_world_asset_release_workflow.py \
  docs/human/asset-ingestion/world-asset-batch-runbook.md
git commit -m "feat: resume asset releases across merge gates"
```

## Tiny Real End-to-end Acceptance — Human-owned, Two Merge Pauses

Use one as-yet-unpromoted trusted Dark Fortress source and a configured compatible nondefault palette. Run setup once, optionally run `review-prepare`, review/export in the Lab, then:

```bash
python3 scripts/world_asset_release.py start --workspace dark-fortress \
  --ready "$HOME/asset-releases/tiny-dark-fortress-ready.json"
python3 scripts/world_asset_release.py status --workspace dark-fortress --batch tiny-dark-fortress
```

If status prints a warning digest, inspect the named refs/reasons and run its exact `resume --approve-warnings DIGEST` command. Confirm Provider PR/receipt physical source, palette/config/atlas/selected hashes, normalized bytes, warnings, cumulative catalog/inventory/stats, and #117 live pointers. **Pause 1: a human reviews and merges Provider PR.**

```bash
python3 scripts/world_asset_release.py resume --workspace dark-fortress --batch tiny-dark-fortress
```

Confirm exact Provider merge binding, Web generated diff, zero-error scripted browser evidence, selected appearance, auxiliary material behavior, placement/export/reload, and no new thumbnail pipeline. **Pause 2: a human reviews and merges Web PR.**

```bash
python3 scripts/world_asset_release.py status --workspace dark-fortress --batch tiny-dark-fortress
python3 scripts/world_asset_release.py resume --workspace dark-fortress --batch tiny-dark-fortress
```

Expected: `complete`, the same two PRs/commits on repetition, and a concise local `summary.md`. No queue reset, duplicate issue/PR, canonical/cache mutation, force push, automatic merge, or newer-main rebinding occurs.

## Validation and Self-review Gate

| Requirement | Primary proof |
| --- | --- |
| Named setup; frozen start; separate review prepare | Tasks 1–2 CLI/config/call tests |
| Lock/journal/revision and hash bindings | Task 1 state tests; Task 2 approval tests |
| Stage authority and interrupted apply recovery | Task 2 seal/prestate/convergence tests |
| Automated pr-mode issue/worktree/commit/push/PR | Task 3 fake-GitHub/local-remote tests |
| Idempotent repo/base/head reconciliation; no force/merge | Task 3 argv and timeout tests |
| Local-only does not bless later publication | Task 3 `--publish` crossing tests |
| Exact merged Provider, not newer main | Task 4 isolated-snapshot test |
| Scripted warning/browser evidence; no premium agent | Tasks 2 and 4 evidence tests |
| GitHub authority; receipts evidence only; status read-only | Tasks 1, 3, and 4 tamper/snapshot tests |
| Final Web merged completion | Task 4 GitHub/blob/provider-binding test |
| Cumulative count/hash/#117 updates without historical edits | Palette Task 2 engine tests plus runner stage seal |

Before opening the runner PR, compare every checkpoint/event/property name across state, workflow, CLI, docs, and tests; search for placeholder language; validate local Markdown links/code fences; run `git diff --check`; verify the index contains only the listed Assets files; and ensure no real GitHub objects or licensed/cache files were created by tests. Planning completion is not implementation completion.
