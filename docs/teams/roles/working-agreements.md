# Working agreements

Cross-cutting rules for **every** dispatched agent, regardless of role. A charter
(`<role>/prompt.md`) says who you are; this says how we all work.

Each item below was paid for. The date is when it bit us.

---

## 1. Dispatch briefs must point here — fresh agents inherit nothing

A spawned subagent starts with none of this repo in context. Omit it and you get a
teammate who derives process from the code, which is how a branch lands on the wrong
base or a "dead" file turns out to be load-bearing.

Every dispatch prompt names, before its task description:

- `rpg-project/AGENTS.md` — the boundary rule and the vocabulary
- `rpg-project/CLAUDE.md` — base branches, board rules, proto versioning
- `rpg-project/sessions/active.md` — the living handoff
- the relevant `docs/teams/roles/<role>/prompt.md`

**Corollary: a process fact that is not in rpg-project does not exist.** The web
`development` branch flow lived only in rpg-dnd5e-web#630, so a session that
correctly searched rpg-project found nothing and briefed a teammate onto `main`
(2026-07-26). When a fact surfaces from an issue or from Kirk in conversation, write
it here.

## 2. Across a seam, the consumer names the interface

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

## 3. Flag a wrong spec; never silently implement it

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

## 4. When a contract changes: moved vs gone

- **Data MOVED** (`Space.walls` → `HexRecord.edges`, `Entity.position` →
  `HexRecord.contents`) → **remap it from the new location.** Always in scope. A
  migration that strips the read without following the data is a half-migration.
- **Data is GONE** with no new home → strip it, note it in the PR body, keep going.
- **Only stop and ask** if remapping would require inventing information the server
  did not send. That is the one line not to cross — that is how a leak gets built.

## 5. Evidence: a green run can prove nothing

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

## 6. Tests that encode a bug get rewritten, not deleted

A test asserting behavior the new design deliberately removes was pinning the old
truth — rewrite it to assert the new one and comment why. Delete only when the
behavior itself is gone, so the test guards nothing.

Before deleting, check the test isn't *also* covering something that survives. In
the fog wave one deleted test also covered reconnect-replace behavior; the right
move was a narrowed replacement, not a deletion.

## 7. Shell that does not stall on permission prompts

A background agent that hits a prompt waits forever, because nobody is watching.

- **Never chain commands.** `cd /path && cmd` prompts. One command per call.
- **Never rely on the working directory.** Do not assume a bare `cd` persists to your
  next call — for a dispatched agent it may not, and you will not notice, because the
  next command runs happily in the *wrong repo*. Address everything absolutely:
  `git -C /abs/path status`, absolute paths to every file. (An earlier version of
  this doc claimed a bare `cd` persists. It was wrong, and it cost us — see below.)
- **Never run a destructive command without an explicit path.** `git clean -fd` and
  `rm -rf` with no argument act on whatever the cwd happens to be — which, per the
  bullet above, is not reliably where you think you are. On 2026-07-27 a pathless
  `git clean -fd`, run while confused about cwd, wiped an unrelated worktree's
  untracked `.claude/` directory. Nothing version-controlled was lost, but only by
  luck. Always name the target: `git clean -fd /abs/path/to/target`.
- **No bare `rm`:** `git rm -f <abs-path>` for tracked, `git clean -fd <abs-path>`
  for untracked.
- **Never bare `git stash`** — the stash stack is shared across worktrees and
  sessions. Use a WIP commit.
- **Never force-push.** Squash-merge means extra commits cost nothing, so add a
  commit instead of amending. (`--force-with-lease` is *not* a safe substitute here:
  any background fetch re-arms the lease.)

If something prompts anyway, route around it with an equivalent that does not, and
note the substitution in your report.

## 8. Model tiers

Pin the model explicitly on every dispatch; never inherit by accident.

| Work | Model |
|------|-------|
| Coordinating / directing | Opus — stays thin, orchestrates and verifies |
| Implementation, refactors, test rewrites | Sonnet |
| Broad read-only search | Haiku |

The point is to keep bulk file-by-file work out of the coordinating context, where
it is most expensive and least useful.
