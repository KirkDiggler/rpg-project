---
name: build-slice
description: Use when building one slice of an agreed plan across the RPG repositories — worktrees, base branches, module discipline, pseudo-version to tag, TDD against the plan's named tests, gates, commits, PR bodies, stacked PRs and review fix lists. The builder's contract; a fresh builder follows it from the plan alone.
---

# Build a slice

The plan names the tasks, files, interfaces and tests. This skill is how a
builder carries one task to a ready PR without inventing process.

## Worktrees and git

- One plain git worktree per line of work, under `<repo>/.worktrees/<name>`:
  `git -C <repo> fetch origin <base> && git -C <repo> worktree add <repo>/.worktrees/<name> -b <branch> origin/<base>`.
- Address every git command with an absolute `git -C <path>`. The shell's
  working directory resets between calls.
- A builder does not use a harness's own worktree switcher; it cannot target
  the sibling repositories reliably.
- Base branches: rpg-api-protos and rpg-toolkit build on `main`; rpg-api and
  rpg-dnd5e-web build on `dev`.
- Commit on top. Never rebase, never force-push, never `--no-verify`. A
  branch catches up with `git merge origin/<base>`.

## Modules and versions

- One nearest-`go.mod` module per toolkit PR.
- Develop outside-in on pushed-commit pseudo-versions of the provider
  (`go get <module>@<pushed sha>`). A PR on a pseudo-version is a **draft**.
- When the provider's tag exists: re-pin with `go get <module>@<tag>` and
  `go mod tidy`, re-run the gates, and mark the PR ready. A ready PR never
  carries a pseudo-version or a `replace` directive.
- Work providers to green before building more consumers on them.

## Protos

- Changes are additive within a package. A deliberate break follows the
  protos repository's breaking-change workflow and its label.
- No hand-written tests. The evidence is `buf lint`, `buf breaking` and code
  generation compiling (`make test`, `make generate`).

## rpg-api

A new gRPC service or RPC in rpg-api is classified in the role policy
(`GameMethodPermission` in `internal/auth/role_policy.go`: `PermissionPlay`,
`PermissionBuild`, and so on), with a test in `role_policy_test.go`, in the
same PR. With world access enforced, an
unclassified RPC is refused as PermissionDenied ("RPC has no configured
access policy"), and local stacks are ungated, so a walk will not catch it.

## Tests and gates

- Write the plan's named tests first and see them fail for the reason the
  plan states. Then implement.
- Run gates in the foreground under a timeout. Never wait on `tail -f` or a
  background monitor; a gate that is not watched finishing is not finished.
- Commit in the same turn a gate turns green.
- A test earns its place by what must break for it to fail. A count pin or
  an assertion that cannot fail is replaced with one that states the fact.

## Facts across a port

When a port or rewrite leaves the UI unable to show something it showed
before, that is a dropped fact. Carry it at its source, in the beat and every
layer above (event-stream Law 10). It is never an accepted loss.

## Attribution and PR body

- Commits, PR bodies and comments carry the team signature only: no session
  link, no model name, no co-author trailer.
- The PR body says what changed, lists the named tests, states every
  deviation from the plan and why, carries `Part of <repo>#<n>` and
  `Closes <repo>#<n>` for the issues it finishes, and ends with the
  signature.
- Scratch files (PR bodies, drivers) live in the job's temporary directory
  under a name carrying the repo, the PR and the builder.

## Stacked PRs

A PR based on another PR's branch is re-targeted to its final base before
that base merges, because GitHub closes a PR whose base branch is deleted:
`gh api -X PATCH repos/<owner>/<repo>/pulls/<n> -f base=<final base>`.
After the base lands squashed, the child catches up with
`git merge origin/<final base>`.

## Fix lists and review threads

When the lead sends a fix list, acknowledge it in one line first, then do it.
Reply in every review thread with what changed or why it did not, and the
commit that answers it.
