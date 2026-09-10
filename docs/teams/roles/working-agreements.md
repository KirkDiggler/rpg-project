# Working agreements

Cross-cutting rules for **every** dispatched agent, regardless of role. A charter
(`<role>/prompt.md`) says who you are; this says how we all work.

Each item below was paid for. The date is when it bit us.

---

## 1. Dispatch briefs must point here — fresh agents inherit nothing

A spawned subagent starts with none of this repo in context. Omit it and you get a
teammate who derives process from the code, which is how a branch lands on the wrong
base or a "dead" file turns out to be load-bearing.

Every new dispatch prompt names, before its task description:

- `rpg-project/AGENTS.md` — the boundary rule and the vocabulary
- `rpg-project/CLAUDE.md` — startup, base branches, board rules, proto versioning
- the Project 19 issue or PR that owns the slice, plus its parent journey when relevant
- the relevant `docs/teams/roles/<team>/prompt.md`
- the owning repository's AGENTS.md and nearest scoped instructions
- `.agents/skills/` only when a matching approved skill exists

`sessions/active.md` remains a shared handoff for cold-start orientation, but new
dispatch briefs do not depend on it as a per-agent task store. Local continuity
belongs in ignored `active.md`, copied from `docs/templates/local-active.md`, and
shared work remains on Project 19.

**Corollary: a process fact that is not in rpg-project does not exist.** The web
`development` branch flow lived only in rpg-dnd5e-web#630, so a session that
correctly searched rpg-project found nothing and briefed a teammate onto `main`
(2026-07-26). When a fact surfaces from an issue or from Kirk in conversation, write
it here.

## 2. Signatures use the Team and the operator

Derive the operator with `gh api user --jq .login`. GitHub activity must end with
the Team-derived signature from the loaded charter:

```text
— <team> agent, on behalf of <github-login>
```

Do not hard-code Kirk's login in new prompts. The point of the signature is to
make the accountable Team and the actual operator visible on every shared
comment.

## 3. Across a seam, the consumer names the interface

The layer that *needs* something defines the shape; the layer that *provides* it
implements that definition. Not the reverse, and not both guessing.

This is what made the Fog of War wave work (2026-07-26):

1. A playable fixture-driven **concept** proved the event shape.
2. **rpg-api-protos** transcribed the proven shape — transcription, not invention.
3. **rpg-api** named `ViewerKnowledge`, the interface it needs from the toolkit,
   and tested its own logic against a fake.
4. **rpg-toolkit** implements that stated requirement.

Each step is reviewable on its own and each correction is cheap, because it lands
before the layer beneath is built. Where the spec and the thing that actually ran
disagree, **the thing that ran wins.**

Implementation order and merge order are different questions. You can build
outside-in and still merge inside-out.

## 4. Flag a wrong spec; never silently implement it

If the instruction you were given is wrong, say so and stop — do not implement it
faithfully and let the bug ship with your name on it.

Both real bugs in the fog wave were caught this way, by implementers pushing back on
a director's spec:

- "Mark the hex REMEMBERED on any disappearance" — wrong when the hex is still
  visible; it dims a hex the player is standing next to.
- "Remove the `space.walls` read" — that read was the sole input to the entire wall
  rendering pipeline.

Equally: if you find the *mirror* of a bug you were sent to fix, fix both and say
you did. Fixing one side of a symmetric bug leaves a planted landmine.

## 5. When a contract changes: moved vs gone

- **Data MOVED** (`Space.walls` → `HexRecord.edges`, `Entity.position` →
  `HexRecord.contents`) → **remap it from the new location.** Always in scope. A
  migration that strips the read without following the data is a half-migration.
- **Data is GONE** with no new home → strip it, note it in the PR body, keep going.
- **Only stop and ask** if remapping would require inventing information the server
  did not send. That is the one line not to cross — that is how a leak gets built.

## 6. Evidence: a green run can prove nothing

Report **real command output**. Never paraphrase a pass you did not see.

Watch for green that is worthless:

- `npm run typecheck` in rpg-dnd5e-web checked **zero files** for its whole life —
  `tsc --noEmit` ignores project references (rpg-dnd5e-web#640). It shipped a type
  error to `main`.
- An implementer wrote `if edge != nil { assert ... }` that silently never fired,
  and caught it on self-review.

So: prefer the check that can fail. Run the real integration suite, not just unit
fixtures, when the thing under test is a wire contract. When you add a gate, prove
it fails on a deliberately broken input before trusting it.

## 7. Tests that encode a bug get rewritten, not deleted

A test asserting behavior the new design deliberately removes was pinning the old
truth — rewrite it to assert the new one and comment why. Delete only when the
behavior itself is gone, so the test guards nothing.

Before deleting, check the test isn't *also* covering something that survives. In
the fog wave one deleted test also covered reconnect-replace behavior; the right
move was a narrowed replacement, not a deletion.

## 8. Shell that does not stall on permission prompts

A background agent that hits a prompt waits forever, because nobody is watching.

- **Never chain commands.** `cd /path && cmd` prompts. One command per call; a bare
  `cd <abs-path>` as its own call persists for later calls.
- **Prefer `-C`:** `git -C /abs/path status` needs no `cd` at all.
- **No bare `rm`:** `git rm -f <path>` for tracked, `git clean -fd <path>` for
  untracked.
- **Never bare `git stash`** — the stash stack is shared across worktrees and
  sessions. Use a WIP commit.
- **Never force-push.** Squash-merge means extra commits cost nothing, so add a
  commit instead of amending. (`--force-with-lease` is *not* a safe substitute here:
  any background fetch re-arms the lease.)

If something prompts anyway, route around it with an equivalent that does not, and
note the substitution in your report.

## 9. Model tiers

Pin the model explicitly on every dispatch; never inherit by accident.

| Work | Model |
|------|-------|
| Coordinating / directing | Opus — stays thin, orchestrates and verifies |
| Implementation, refactors, test rewrites | Sonnet |
| Broad read-only search | Haiku |

The point is to keep bulk file-by-file work out of the coordinating context, where
it is most expensive and least useful.

## 10. Make work visible early; keep toolkit releases module-sized

**Drafts provide visibility, not a readiness claim.** Open and link a draft PR on
the first working push. Update it at meaningful checkpoints with what changed,
what decisions need the human, current validation, and known gaps. Do not wait
for the whole feature, all checks, or an agent review loop before the human can
see the work. Required checks and review still govern promotion to ready and
merge; they are not a gate on publishing an honest draft.

**One toolkit Go module per PR.** Toolkit modules build and tag independently in
CI. A feature spanning modules needs separate PRs, identified by the nearest
`go.mod`; directory nesting does not make child modules part of their parent.
Bane therefore has four toolkit PRs: `rulebooks/dnd5e`, `encounter`, `resolution`,
and `session` (the latter three nested under `rulebooks/dnd5e`). Repository-wide
instructions may accompany the relevant module, but another module's files may
not be bundled into that PR.

Draft consumers can expose work using verified pushed provider pseudo-versions.
Before merging a consumer, adopt the provider's actual successful CI-generated
tag in the consumer's own PR and rerun its checks. Never fabricate tags or merge
a multi-module batch to evade release sequencing. Preserve working branches
while dependency pins still reference them.

These are Kirk's Bane review corrections (2026-09-10): the implementation was
visible too late and the initial toolkit PR incorrectly bundled four release
units. The toolkit's AGENTS.md must state both rules explicitly.
