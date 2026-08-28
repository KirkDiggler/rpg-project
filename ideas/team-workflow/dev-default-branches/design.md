# Dev-default integration branches

**Date:** 2026-08-28

**Journey:** [rpg-project#312](https://github.com/KirkDiggler/rpg-project/issues/312)

**Documentation slice:** [rpg-project#313](https://github.com/KirkDiggler/rpg-project/issues/313)

## Summary

`rpg-api` and `rpg-dnd5e-web` already use `dev` as their integration branch in project policy: feature branches start from `origin/dev`, feature PRs target `dev`, and production is cut by promoting `dev` to `main`. GitHub still defaults both repositories to `main`, however, and the live repository settings do not safely support changing that default.

This change makes GitHub match the existing development model without changing the production trigger:

```text
feature/<issue> --squash--> dev --merge commit--> main --deploy--> production
```

`dev` becomes the default, protected integration branch. `main` remains an explicitly protected production branch. Docker `latest` and deployment remain explicitly tied to `main`, independent of whichever branch GitHub calls the default.

## Verified current state

The following facts were read back from GitHub on 2026-08-28.

### Shared repository settings

Both `KirkDiggler/rpg-api` and `KirkDiggler/rpg-dnd5e-web` currently have:

- default branch `main`;
- automatic deletion of merged head branches enabled;
- merge commits, squash merges, and rebase merges enabled at repository scope;
- an unprotected `dev` branch;
- one active ruleset named `Protect The President` with no bypass actors.

The active ruleset in each repository targets `~DEFAULT_BRANCH`, requires pull requests and strict required checks, blocks deletion and force pushes, and allows **squash only**. This differs from the statement in `rpg-project/CLAUDE.md` that the rulesets already target explicit `main` and allow merge commits only.

### Branch history

The latest production promotions were squash merges:

- rpg-api PR #854: `dev -> main`;
- rpg-dnd5e-web PR #838: `dev -> main`.

For each repository, `main` and `dev` currently point at different commits with the same Git tree. The code is identical, but the squash commits did not record `dev` as a parent. A merge of `main` into `dev` before the next production promotion will restore the ancestry relationship without changing files.

### Workflows

In both repositories, `.github/workflows/docker.yml` currently grants `latest` with:

```yaml
type=raw,value=latest,enable={{is_default_branch}}
```

Production deployment consumes `ghcr.io/kirkdiggler/rpg-api:latest` and `ghcr.io/kirkdiggler/rpg-dnd5e-web:latest`.

The deployment-dispatch steps are already explicitly restricted to `refs/heads/main` and remain unchanged.

Current CI branch coverage is otherwise suitable:

- rpg-api test and Docker workflows run for `main` and `dev`; a `dev` push intentionally publishes the `:dev` API image used by the local stack.
- rpg-dnd5e-web normal CI runs for `main` and `dev`; its Docker workflow runs only for `main` and manual dispatch.

## Goals

1. Make `dev` the GitHub default branch for rpg-api and rpg-dnd5e-web.
2. Prevent automatic branch cleanup from deleting either long-lived branch.
3. Keep `main` as the only branch that publishes production `latest` images and triggers deployment.
4. Make branch-specific merge methods enforceable:
   - squash feature PRs into `dev`;
   - merge-commit promotions into `main`.
5. Keep required CI checks appropriate to each target branch.
6. Reconcile living project and workspace instructions with the verified configuration.

## Non-goals

- No change to rpg-toolkit, rpg-api-protos, rpg-deployment, or their default branches.
- No change to application behavior, API contracts, or game rules.
- No replacement for the existing `latest`-based production deployment in this journey. Immutable deployment tags are a separate improvement.
- No mandatory staging deployment or preview environment.
- No rewrite of historical designs or plans that accurately record the workflow used at their time.
- No new source-branch enforcement workflow. Project policy and the default PR target establish the normal route; hotfix PRs must remain possible against `main`.

## Branch roles and flows

### Normal feature work

```bash
git fetch origin
git checkout -b feat/123-description origin/dev
```

The PR targets `dev` and is squash-merged after its required checks pass. Automatic deletion removes the short-lived feature branch.

No one commits directly to `dev`.

### Production promotion

A production promotion is a PR whose head is `dev` and base is `main`.

Before merge, update `dev` with the current `main` tip. For the first promotion in this rollout, that update is content-neutral except for recording the existing squash-release ancestry. GitHub's strict up-to-date requirement remains enabled and is the guard that forces this step.

The promotion is merged with a real merge commit. It is never squashed or rebased. The merge commit records `dev` as a parent, so the next comparison starts from the release that actually shipped rather than replaying already-shipped feature history.

Only Kirk merges promotion PRs. The existing rule that every merge to an auto-deploying `main` must be followed through the downstream deployment remains in force.

### Hotfix

An urgent production-only fix starts from `origin/main`, targets `main`, and uses a merge commit under the production ruleset. Once production is verified, merge `main` into `dev` before continuing normal integration work. Never rebase either long-lived branch.

## Docker safety change

Both Docker workflows replace the default-sensitive tag gate with an explicit production-branch gate:

```yaml
type=raw,value=latest,enable=${{ github.ref == 'refs/heads/main' }}
```

This yields the following behavior after `dev` becomes default:

| Event | API image behavior | Web image behavior | Deployment |
|---|---|---|---|
| PR into `dev` | build only; no push | normal CI only | never |
| push to `dev` | push `:dev` and SHA tags; never `:latest` | no Docker run | never |
| PR into `main` | build only; no push | Docker build only; no push | never |
| push to `main` | push branch/SHA/`latest` | push branch/SHA/`latest` | dispatch production |
| manual dispatch on `dev` | may push non-production dev/SHA tags; never `latest` | may push non-production dev/SHA tags; never `latest` | never |

The existing deployment condition remains:

```yaml
if: (github.event_name == 'push' || github.event_name == 'workflow_dispatch') && github.ref == 'refs/heads/main'
```

## Branch rulesets

The existing `Protect The President` ruleset in each repository is edited in place and renamed `Protect main (production)`. Its target changes from `~DEFAULT_BRANCH` to explicit `refs/heads/main` before the default branch changes.

A second active ruleset, `Protect dev (integration)`, targets explicit `refs/heads/dev`.

Both rulesets have no bypass actors and include:

- deletion protection;
- non-fast-forward protection;
- pull request requirement;
- strict required status checks;
- the existing Copilot code-review rule;
- zero required human approvals, preserving the current solo-maintainer setting.

They deliberately do not require linear history because production promotions use merge commits.

### rpg-api

| Target | Required checks | Allowed PR merge method |
|---|---|---|
| `dev` | `test`, `build` | squash |
| `main` | `test`, `build` | merge commit |

### rpg-dnd5e-web

| Target | Required checks | Allowed PR merge method |
|---|---|---|
| `dev` | `Test`, `Lint and Type Check`, `Security Audit` | squash |
| `main` | `Test`, `Lint and Type Check`, `Security Audit`, `build` | merge commit |

The web `dev` ruleset does not require Docker `build` because the Docker workflow intentionally does not run for `dev`. The normal `Lint and Type Check` job already performs `npm run build`. The production promotion still requires the Docker `build` check before `main` can move.

At repository scope:

- keep merge commits enabled;
- keep squash merges enabled;
- disable rebase merges;
- keep automatic head-branch deletion enabled.

Branch-specific rulesets decide which of the remaining merge methods is valid for each target.

## Ordered rollout

Order is load-bearing. The default branch changes last.

1. **Land Docker safety on `dev`.**
   - rpg-api#856: one workflow-line change, PR to `dev`.
   - rpg-dnd5e-web#844: one workflow-line change, PR to `dev`.
2. **Reconcile PR targets before tightening production.**
   - Retarget active API feature PR #853 from `main` to `dev` if its diff and base remain valid.
   - Leave old web PRs unmodified unless separately triaged; production protection will prevent accidental squash delivery.
3. **Install explicit rulesets while `main` is still default.**
   - Convert the existing ruleset to explicit `main` and merge-only.
   - Add the explicit `dev` squash-only ruleset with target-appropriate checks.
   - Verify both branches report protected.
4. **Merge the two safety PRs into `dev`.**
   - Kirk approves and squash-merges each PR.
   - Confirm the feature branches are deleted and `dev` remains.
5. **Promote with merge commits.**
   - rpg-api#855 owns the API `dev -> main` promotion PR.
   - rpg-dnd5e-web#843 owns the web `dev -> main` promotion PR.
   - Update each promotion branch with `main` first, wait for required checks, then Kirk merge-commits it.
   - Observe each main image build and downstream production deployment to terminal success.
6. **Change repository defaults to `dev`.**
   - Change both GitHub default branches only after both main workflows contain the explicit `latest` gate.
   - Disable repository-level rebase merges; leave automatic deletion enabled.
7. **Reconcile documentation.**
   - Update the live ruleset description in `rpg-project/CLAUDE.md` without changing the existing repo-aware base-branch table.
   - Update `/home/kirk/personal/CLAUDE.md` locally so its generic fresh-branch command does not override the repo-aware rule.
   - Keep the rpg-project design/plan PR open as the tracking surface until implementation and read-back verification are complete.

## PR and issue map

| PR | Owning issue | Base | Purpose |
|---|---|---|---|
| rpg-api safety | rpg-api#856 | `dev` | Make `latest` explicitly main-only |
| web safety | rpg-dnd5e-web#844 | `dev` | Make `latest` explicitly main-only |
| API promotion | rpg-api#855 | `main` (`dev` head) | Ship safety change and restore release ancestry |
| web promotion | rpg-dnd5e-web#843 | `main` (`dev` head) | Ship safety change and restore release ancestry |
| project workflow docs | rpg-project#313 | `main` | Design, plan, and living-instruction reconciliation |

Repository setting mutations are recorded on the owning issues and journey but do not create additional PRs.

## Verification

### Static workflow checks

For each Docker workflow:

- `latest` is gated by `github.ref == 'refs/heads/main'`;
- deployment is gated by the same explicit main ref;
- no `is_default_branch` remains in production tag metadata.

### GitHub settings read-back

For each repository, verify through `gh` after mutation:

- `defaultBranchRef.name == "dev"`;
- `deleteBranchOnMerge == true`;
- `rebaseMergeAllowed == false`;
- both explicit rulesets are active;
- `main` allows merge commits only;
- `dev` allows squash only;
- branch protection reports true for both `main` and `dev`;
- required checks match the tables above.

### PR-path checks

- A new feature PR defaults to base `dev`.
- Safety PR checks appear on `dev` and pass.
- Promotion PR checks appear on `main` and pass.
- The promotion UI offers the permitted merge-commit path, not squash.
- Merging a promotion does not delete `dev`.

### Deployment checks

For each promotion:

- the main Docker workflow reaches terminal success;
- the image metadata includes `latest`;
- the `Deploy RPG Platform` run reaches terminal success;
- no workflow run from `dev` triggers production deployment.

## Failure handling and rollback

- **A required check never appears on `dev`:** do not bypass. Compare the explicit ruleset check list with the workflow's `pull_request.branches` filters and job display names; correct the mismatch before merge.
- **A promotion says `dev` is behind `main`:** update `dev` with `main`, rerun checks, and keep the merge method as a merge commit. Do not rebase or force-push.
- **A ruleset blocks every merge method:** read back all active rulesets for overlapping targets. Disable the stale overlapping rule only after the explicit replacement is active.
- **A `dev` workflow can move `latest`:** stop before changing the default. Revert the workflow or restore the explicit main expression.
- **A main deployment fails:** do not describe the release as shipped. Follow the existing deployment recovery path; changing the default branch is not a deployment rollback.
- **The default change causes unexpected repository behavior:** switch the default back to `main`. Explicit branch rules and explicit Docker tagging remain safe in either default configuration.

## Alternatives considered

### Keep `main` as default and protect `dev`

This would stop automatic deletion with fewer setting changes, but new clones and PRs would continue to point at production by default. It preserves the exact accidental-target risk the integration branch is meant to remove.

### Deploy directly from `dev` and retire `main`

This removes promotion PRs but changes the deployment contract and eliminates the explicit production branch. It is larger than the problem and discards a useful release boundary.

### Disable automatic head-branch deletion

This prevents another `dev` deletion but leaves every feature branch behind and does not solve wrong PR defaults, Docker tagging, branch-specific merge methods, or production protection. Keeping cleanup enabled with explicit protection is simpler.

## Decision

Adopt `dev` as the GitHub default for rpg-api and rpg-dnd5e-web only after Docker tagging and explicit branch rulesets are in place. Keep `main` as the production branch, feature PRs squash into `dev`, and release PRs merge-commit from `dev` into `main`.
