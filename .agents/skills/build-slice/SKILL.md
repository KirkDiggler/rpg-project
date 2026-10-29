---
name: build-slice
description: Use when building one slice of an agreed plan across the RPG repositories — worktrees, base branches, module discipline, pseudo-version to tag, TDD against the plan's named tests, gates, commits, PR bodies, stacked PRs and review fix lists. The builder's contract; a fresh builder follows it from the plan alone.
---

# Build a slice

The plan names the tasks, files, interfaces and tests. This skill is how a
builder carries one task to a ready PR without inventing process.

## Slice order

1. **Build** the slice.
2. **Boot it:** the stack comes up on the builder's own ports and its pages
   load with no errors.
3. **Report the head** to the lead for the operator's early view.
4. **Only then** walk it and ask for review.

The builder starts neither a walk nor a review request until the lead says
the operator has looked, or that the slice has nothing to see. The operator
may pivot on the early view, and a pivot then costs nothing.

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

The rpg-api gate is `make test` (`go test -short -race ./...`), never CI's
package list. CI's `test.yml` excludes `cmd/server` (rpg-api#1133), so a green
CI can hide a failing world-access rollout test; the builder's local gate
covers every package.

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

## The walk site

A slice with a visible change ships a walk site, `<slice-short-name>-walk.yaml`.
It exists so the operator's QA look verifies the mechanic, not the route to
it: a small open place where the mechanic shows and the operator can walk
around it.

- Cut it from the-mad-king where the castle already has the place, so the
  fold back is trivial. The gate and its two doors need no courtyard.
- It is never castle navigation, and never a corridor that forces a fight to
  reach the mechanic. A fight on the site has a way past it.
- It lives untracked in the slice's rpg-api worktree `content/` and in the
  walk env's mounted content dir. It is never committed to the rpg-api shelf.
- The Try-it names the site key in its first line. Its last step is the
  mechanic folded into the-mad-king, which the same slice does so the castle
  stays the one living reference.
- The engine side carries its own one-off: a site under the module's
  `testdata/` per mechanic with one scenario test on it, plus one route
  assertion on the castle.

This states the spirit; the builder shapes each site to its mechanic. A site
built from a template is the tell that the rule went prescriptive.

## Attribution and PR body

- Commits, PR bodies and comments carry the team signature only: no session
  link, no model name, no co-author trailer.
- The PR body says what changed, lists the named tests, states every
  deviation from the plan and why, carries `Part of <repo>#<n>` and
  `Closes <repo>#<n>` for the slice sub-issue it finishes (story skill), and
  ends with the signature. A slice with no sub-issue stops here: the builder
  asks the lead for one before opening the PR.
- **Try it**, required in every PR with a visible change: where to point
  (the operator's stack on :3030/:8115, or the studio), then numbered steps
  written for the operator, each "do this → expect that" and each tied to the
  box of the slice sub-issue's verification checklist it proves — the
  checklist is authored there and copied here, never the other way round. A PR with no visible change says "Nothing to
  try (api internals)" or "Nothing to try (toolkit)".
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
commit that answers it: "fixed by <sha>", plus "mutant fails" where the
finding was about a seam test.
