# RPG Project

We are building a multiplayer D&D 5e dungeon crawler that runs as a Discord
Activity. Players make characters, join a lobby, and fight through a dungeon
together.

That is the product. It is not the point.

## The lens — read this before anything else

**We are building architecture that makes a complex ruleset simple.** Every piece
of work in every repository is read through this, and nothing below overrides it.

> *"we are not here to make things work, we are here to build composeable,
> extensible components that we can build a solid foundation for our game to
> evolve on. the play and world packages we made fit that description. the seams
> in session — resolution and encounter are deliberate. we are not making
> independent features, we are architecting this thing so it can do a full DND
> implementation. that will never happen if we put a band aid in. our goal is
> never 1 feature, it is the game becoming more than we ever dreamed."*
>
> — Kirk, 2026-09-10

What that means when you are actually deciding something:

- **`play/*` and `world` are the model to copy.** One concern each, composable,
  deliberately ignorant of everything else. A new primitive should look like them.
- **The seams in `session`, `resolution` and `encounter` are deliberate.** They are
  not incidental module boundaries to route around when a feature is inconvenient.
  A change that wants to blur one is the thing that is wrong — not the seam.
- **A band-aid is a non-starter.** Not a cheaper option to be weighed against the
  proper fix and traded away under time pressure. When the fork is "special case
  above" versus "give the lower layer the primitive it lacks", take the primitive.
  Ask what a shortcut makes permanently **impossible**, not merely what it makes ugly.
- **Judge a slice by the tool it adds, never by the feature it closes.** One feature
  is never the goal. Work earns its place by what it makes possible next.
- **Full D&D is the target, and it is the test.** Read every design against it: does
  this shape still hold when the rest of the ruleset arrives, or does it only hold
  for the handful of spells implemented today?

The failure this exists to prevent is not ugliness. It is a shortcut that quietly
forecloses something the game was always going to need — the reason a fact an
observer can be wrong about must be snapshotted per observer rather than read live,
because a live read can only ever be true, and a game with no way to lie can never
have illusion in it.



## Mistakes are not the problem

> *"we make mistakes. that's fine. there's no problem making mistakes and it
> might actually be a good thing. mistakes show us ways that we can get better.
> we don't acknowledge them and learn from them. that's a problem but a mistake
> happening that can help us that can improve our process is a really good thing
> and should be treated as such. mistakes aren't the problem repeating mistakes
> is the problem."*
>
> — Kirk, 2026-09-10

The response to a mistake is never "be more careful next time." **Careful is not
a mechanism.** The response is to find what let it through, and change that.

- **Say it plainly.** A mistake nobody names is one nobody can learn from. Quietly
  rewriting a design so it looks like it was always right destroys the only
  valuable thing the mistake produced.
- **Fix what permitted it, not just the instance.** If a rule allowed it, the rule
  was testing the wrong thing. If a doc failed to prevent it, the doc was not
  clear enough to be load-bearing. Correcting only the instance leaves the cause
  in place, armed.
- **Leave the correction visible.** A record showing what was believed, and why it
  changed, is worth more than one showing only the conclusion — the reasoning is
  what stops the next person walking the same path.
- **A repeat is the real defect.** A second occurrence is evidence the first fix
  addressed the symptom.

The worked example this was written from: an area spell's target derivation was
designed into `session` for two drafts. The seam doc already said *"geometry,
placement → `encounter`"*, but the work did not read as *geometry* from inside —
it read as *deriving a target set* — and the predicate/producer rule beside it is
a **mechanism** test that a `session`-side fold passes cleanly. The rule licensed
the error rather than catching it. The fix was not "remember that placement is
encounter's"; it was to add an **ownership test** ahead of the mechanism test in
all three seam docs, so the wrong answer now looks wrong on the page.

This is also why the shape is worth getting right while the system is small. Today
a misplaced responsibility is visible in an afternoon. That will not last, and the
same mistake in a system three times this size is not caught by being careful — it
is caught by the seams and the rules being clear enough that the wrong answer
cannot look reasonable.

## What the foundation is

```
Client sends REFERENCES (keys, IDs) -> never calculations
API orchestrates by KEY             -> never knows what "rage" does
Toolkit implements RULES            -> returns rich breakdowns for rendering
```

If you see game logic in the API, say something. If you see calculations in the
client, say something. Those two sentences are most of the job.

The invariants that hold it up:

- **Ownership before mechanism.** Every noun has one owner. A fact about a repo
  lives in that repo; a rule about the game lives here.
- **Zero values tell the truth.** An absent value must mean what its author
  meant. Fail closed and loudly — never silently.
- **Input/Output types on every function.** Non-negotiable.
- **Never return `(nil, nil)`.** A valid object or an error.
- **Toolkit types are canonical.** The API stores them; one conversion point at
  the handler/proto boundary.
- **Events for multiplayer.** One player sees the RPC response; everyone sees
  the event.
- **A use case brings the mechanism.** We do not build what we think will be
  used. Absent is a cost not yet paid, not a gap.

We are pre-pre-alpha. Nobody is playing; breaking it is not a gate. That buys us
the one thing that makes this standard affordable — **we can still fix the
shape.** Spend it.

## The repos

| Name | Role | Key phrase | Its rules live in |
|------|------|-----------|-------------------|
| **rpg-toolkit** | Rules engine | "Knows what Rage does" | `rpg-toolkit/CLAUDE.md` |
| **rpg-api** | Game server / orchestrator | "Knows feature keys, not behavior" | `rpg-api/CLAUDE.md` |
| **rpg-dnd5e-web** | React UI in the Discord Activity | "Renders protos, sends intent" | `rpg-dnd5e-web/CLAUDE.md` |
| **rpg-api-protos** | Contract definitions | "Source of truth for API shape" | `rpg-api-protos/CLAUDE.md` |

Branch bases, build commands, test gates, proto versioning, grid choice — those
belong to the repo that lives with the consequence. **Read the owning repo's
instructions before you touch it.** Nothing here overrides them.

## How a wave is shaped

**Develop outside-in. Merge inside-out.**

```
develop:  web concept  ->  protos  ->  api  ->  toolkit
merge:                     toolkit ->  api  ->  web
```

Develop from the edge, because the consumer discovers the requirement — nobody
guesses at a layer they don't consume. Merge from the middle, because a consumer
cannot honestly pin a provider that hasn't shipped.

**One wave, one branch per repo — not one per bug found along the way.**
Integration keeps revealing things the provider must do; that is the method
working, not new scope. The provider's branch is done when the consumer driving
it stops asking. (Learned on Fog of War: four toolkit PRs for one feature, two
of which conflicted with each other because they were halves of the same change.)

Parallel waves legitimately produce many versions of a module — that is a studio
working. The mistake is three versions *of one thing*.

## Getting to work

1. **Project 19** — https://github.com/users/KirkDiggler/projects/19 — is the
   shared state. Boards #11–#13 are historical records, not work sources.
2. Read the Team lens for the assigned work in `docs/teams/roles/`.
3. Read the **owning repository's** `CLAUDE.md`/`AGENTS.md` and nearest scoped
   instructions. That is where the commands and invariants are.

- **Work in a worktree, one per line of work**, under `.worktrees/<name>` in
  every child repository.
- One issue per PR. No branch without an issue. No issue without a board entry.
- Publish the draft on the first working push and report the PR link immediately,
  with review explicitly pending. Visibility is not withheld while a reviewer runs.
- **Ready for review** means the declared scope is implemented and applicable
  checks are green. **Merge-ready** additionally requires completed review and
  release prerequisites. Neither label authorizes an automatic merge.
- Feature PRs and rules/engine changes get one independent review round from a
  session that did not implement the change; doc-only PRs, pin bumps and small
  mechanical fixes may skip it. The record is a verdict published on the PR — a
  local review file is not the review record.
- Anything on your own machine is yours and binds nobody. Shared work is the
  board and the PRs.

Cross-repo designs live here in `ideas/<topic>/design.md`, reviewed as an
rpg-project PR before implementation, and that PR stays open as the tracking
surface until the implementing repos have landed.

## When you hit a gap

A convention nobody wrote down, a doc that contradicts practice, a pointer that
resolves to nothing — that is a finding, and it is worth more than the workaround
you are about to apply. Open an issue naming the gap and what it cost, then carry
on with the work.

**Do not write the rule from the session that hit it.** That session is saturated
with its own incident and will shape a general rule around the one example in
front of it.

**The case belongs in the issue or PR; the rule belongs in the doc.** Evidence and
measurement stay permanent in git, one hop away. A doc every session loads carries
the instruction and as much of the spirit as the rule will need when it is
inconvenient — no more.

## Pointers

| For | Read |
|-----|------|
| Model/context budgets, PR sizing, branch naming | `docs/teams/roles/working-agreements.md` |
| Team charters and role lenses | `docs/teams/roles/` |
| Layer model, trust boundaries, data flow | `docs/architecture.md` |
| Extended glossary | `docs/vocabulary.md` |
| What each layer knows and doesn't | `docs/boundaries.md` |
| Running the game locally | `docs/howto/` |
| Cutting a release (`dev` -> `main`) | `game-dev/scripts/release.sh` |
