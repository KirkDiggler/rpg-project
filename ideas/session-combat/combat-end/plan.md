# Combat end — the build ledger

What was built, and **what the building changed about the design**. Written as
it happened rather than reconstructed, because the corrections are the part
worth keeping.

Slice: rpg-project#295 part 1 · design: `design.md` · PRs: rpg-toolkit
[#1262], [#1263], [#1264], [#1265]

[#1262]: https://github.com/KirkDiggler/rpg-toolkit/pull/1262
[#1263]: https://github.com/KirkDiggler/rpg-toolkit/pull/1263
[#1264]: https://github.com/KirkDiggler/rpg-toolkit/pull/1264
[#1265]: https://github.com/KirkDiggler/rpg-toolkit/pull/1265

---

## The tasks

| # | module | PR | state |
|---|---|---|---|
| 1 | `rulebooks/dnd5e` — `SubjectID`, dead publisher documented | #1262 | open |
| 2 | `encounter` — `CombatEnded`, fan-out, announce | #1263 | open |
| 3 | `resolution` — one topic-table entry | #1264 | open, **pin swap** |
| 4 | `session` — acceptance tests only | #1265 | open, **pin swap** |
| 5 | `rpg-api` — version bump | — | waits on real tags |

---

## What changed from the design

### 1. `announce` split in two — not in the design, and the design was wrong to omit it

`dissolveBubble` has **no milestone to translate**. `boundariesFrom` takes
`[]clock.Milestone`; the fan-out produces `[]Boundary` directly. So the single
`announce(ms)` helper could not serve both, and it split:

- `announce(ms []clock.Milestone)` — translates, then delegates
- `announceBoundaries(crossed []Boundary)` — skips an empty set, calls the capability

Small, but it is the shape the design should have predicted: **the moment
boundaries can come from somewhere other than a milestone, the translating step
and the announcing step are two different things.** No behaviour change at the
three existing call sites.

### 2. Q1 answered as leaned — and pinned rather than left implicit

Monsters get the boundary. The test asserts it **by name** rather than letting
it be incidental, because "which members an ending is about" is exactly the
kind of question that gets quietly narrowed later by someone adding a filter
that looks like an optimization. R3's answer — pass everyone in, let the
effect's own predicate decide — is now a failing test if reversed.

### 3. Q2 answered as leaned, and the debt written where it will be found

`Character.EndCombat` stays, documented as dead **in its own godoc** rather
than in an issue. Same for the reasoning: `combat.TurnManager` is in the
identical position after #294, and the two should go in one pass.

An issue would have been the tidier-looking choice and the worse one — the
person who needs this fact is reading the method, not the backlog.

### 4. The economy-ordering test is a guard, not a proof, and says so

§3.1 argued the two writes are safe because `fetchCharacterData` is uncached.
The test pins it. But it **cannot fail today for the reason it exists** — there
is no cache to break it. Mutation-checking it would have meant inventing a
cache to knock down.

So it is labelled in the test as a guard against a future change rather than
dressed up as a verified claim. **An untested branch stated honestly is worth
more than a passing test that implies more than it checks.**

---

## What the building found that the design did not

### The mutant that survives every behavioural test

Reading the round off the **first** milestone instead of the dissolution passes
every test driven through a real `Dissolve` — because a real dissolve returns
exactly *one* milestone. The behavioural suite is structurally blind to it.

Only the internal test catches it, and only because it hands in a **decoy**: a
`TurnEnded` carrying a different round, sitting ahead of the dissolution.

This is the same lesson as #1259's ordering test (which survived its own mutant
because one monster after alice made the drive loop irrelevant), arriving from a
different direction: **a behavioural test can only distinguish implementations
that production inputs distinguish.** Where production hands in a degenerate
case — one milestone, one member, one round — the discriminating input has to be
constructed by hand.

### Forming a fight already announces, so a failing announcer cannot build one

`TestAnAnnouncerMalfunctionAbortsTheDissolve` failed on its first run at
**construction**, not at the assertion: #294 made `form` announce the fight's
first turn, so `journalAnnouncer{fail: boom}` refuses to build the fight whose
ending was under test.

Fixed with a `failAfterForming` that is switched on after setup — and the
construction succeeding first is its own small proof that the form-time
announcement is real.

### The suite default made green meaningless

Every encounter test supplies `quietAnnouncer{}`. All three packages stayed
green the instant the announce was wired in, which said **nothing** about
whether it fired.

Worth recording as a pattern rather than an anecdote: **a tolerant test double
converts "did this happen" into "did this not crash."** The check that caught it
was asking why a change this behavioural broke nothing.

### Pinning backwards is a cheap red-before-green proof

`session` never names `encounter.CombatEnded`, so pinning its three
dependencies back to their pre-slice tags **still compiles**. Doing so fails
exactly the three new tests and leaves #294's five passing.

That is a stronger statement than mutation testing gives for an integration
suite: the tests are red against the real previous release, and they do not
accidentally re-test the previous slice. **Worth reaching for whenever a slice
spans modules and the consumer does not reference the new symbol by name.**

### Copilot on #1262: a cross-repo path with no repo is worse than no path

`ideas/session-combat/…` reads as a path in the repo you are looking at. A
reader follows it, finds nothing, and concludes the comment is stale — so the
pointer costs a search *and then* misleads. `events.go` already said
"rpg-project's …" and the two test sites did not; the inconsistency was the
tell.

Two older sites (`resolution/cast.go:68`, `gamectx/cast.go:38`) carry the same
ambiguity and were left for one sweep rather than a drive-by.

---

## Verification

- Build, vet, lint clean in every module; 27 packages green in `rulebooks/dnd5e`,
  3 in `encounter`, `resolution` and `session` green.
- **Mutation testing, five mutants, all killed**: the owner guard in
  `onCombatEnd`; the announce removed from `dissolveBubble`; the fan-out reduced
  to one member; the round read off the first milestone; the missing-dissolution
  error softened to an empty return.
- **Red-before-green** against the pre-slice tags, as above.
- Structural: `dissolveBubble` has exactly two callers and announces itself, off
  the AST — a behavioural suite cannot notice a third being added, which is how
  this vocabulary reached the state the slice found it in.

## Open at hand-off

- **Pin swaps** on #1264 and #1265 once #1262/#1263 merge and tag, then the
  rpg-api bump.
- **Q1/Q2 unruled by Kirk.** Built on the leanings, both cheap to flip: Q1 is
  the absence of a filter, Q2 is the absence of a deletion.
