# Reactions / #316 — implementation

What the build found, what it cost, and what would change the design next time.
Written against the slice as landed; PR links in `plan.md`.

**Status 2026-08-30:** written under the rpg-project#201 tag freeze. All four
toolkit trains are built, green, and mutation-proven; P1 (#1320) and P2 (#1321)
are open with HOLD banners, P3/P4 are pushed branches whose PRs open at "freeze
released" with real-tag bumps and the baseline stated in each body.

## Outcome

The opportunity attack fires again, on both walk paths, and the log says why.

```
--- PASS: TestWalkingOutOfReachProvokesAnOpportunityAttack
--- PASS: TestDisengagingWalksAwayUntouched
--- PASS: TestAnOpportunityAttackThatDropsTheWalkerStopsTheWalk
```

The first is the gap survey F2 named: **the first test anywhere in which the
real `OpportunityAttackCondition` fires inside a real fold.** Everything in its
path is production code — the monster loader's free grant at Attach, the
condition's own predicate, resolution's fold and reaction Request, the strike,
`enc.Record`, and the new `Reaction` identity. The beat it produces:

```
actor: skel-1, beat: struck, roll: 10, total: 14, against: 12, amount: 12,
attack:   {ref: dnd5e:monster_actions:skeleton-shortsword, name: shortsword, ...},
reaction: {ref: dnd5e:conditions:opportunity_attack, name: Opportunity Attack}
```

---

## What the design did not predict

### 1. R6 had no home on the monster's path

R6 ruled that an opportunity attack dropping the walker stops the walk, and the
plan assigned it entirely to session's `runWalk` — because the survey framed it
around session's batched `discoveryStanding`. But the wolf in the Done-when
sentence never goes through `runWalk`. It walks in `executeTurnIntent`'s `Move`
case, inside `encounter`.

Verified rather than assumed: `stepTo`/`stepMember` have **no standing gate at
all** — `Standing` is consulted when building a `MonsterView` and by
`attackIsInReach`, and nowhere else. So the plan as written would have shipped a
player who stops when dropped and a wolf who keeps walking while downed: the
exact asymmetry the `Mover` capability exists to prevent, one ruling later.

Folded into P2 by ruling (the director's — mechanism inside Kirk's R6,
flagged for veto in the P2/P3 PR bodies). The ordering is `announce → ask standing → step only
if still up`, and the mover **falls in the cell they were leaving** — which
follows from why the reaction fired at all. On the player's path this means a
two-cell walk takes *zero* steps when the first announcement is fatal.

**Design lesson:** a ruling scoped to the path the survey happened to examine
does not automatically cover the path the Done-when sentence travels. R1 created
a second path; every subsequent ruling needed asking against both.

### 2. Disengage never worked on this stack

The design said the condition's "Disengage short-circuit and its six-case suite
all survived intact because they were never the part that broke." The suite
survived. **The short-circuit never worked on this stack.**

`resolution/movement.go` publishes the chain — every *subscriber* runs — and
only then Executes it, which is when the *stages* run. `DisengagingCondition`
writes `OAPreventionSources` in a **stage**. `OpportunityAttackCondition` read
`IsOAPrevented()` in its **subscriber**, strictly earlier. It was reading a
field nothing had written yet, on every step, forever. The machine then set
`out.OAPrevented` from the folded event, so it ended up holding both facts —
"prevention happened" and "here is the OA that fired" — and reconciling neither.

Found by the first E2E to walk both real conditions through one real fold.

Fixed: prevention is applied by the machine post-fold, keyed on `TriggerKind`
so it is exactly as narrow as the rule ("Disengage stops opportunity attacks",
not "silences reactions"), and the condition's dead check is deleted. A
predicate that reads state written later is unfixable at its own layer — see
learning 4.

### 3. The OA suite's entire negative half could not fail

This is the most valuable thing the slice found, and it is not a bug in the
product.

The dead check above was guarded by `TestNoTriggerWhenOAPrevented`, which
claimed to cover exactly it and was **hollow**: it never installed a cast, so
`canReact` fell closed and no trigger fired regardless of what the prevention
sources said. Deleting the check left it green.

Following that shape rather than stopping at the one test found that **all four
negative cases were hollow, every one for the same single omission.** Measured
under the mutation each names, not suspected:

| guard removed | test that names it | before | after repair |
|---|---|---|---|
| readiness gate | `TestNoTriggerWhenReadinessOff` | still passed | fails |
| not-self gate | `TestNoTriggerOnSelfMovement` | still passed | fails |
| leaving-reach | `TestNoTriggerWhenStillInReach` | still passed | fails |
| leaving-reach | `TestNoTriggerWhenMoverNeverInReach` | still passed | fails |

**Root cause: one missing line each.** `canReact` reads the reactor's reaction
slot off the cast and falls closed when there is none. No negative case
installed a cast — the positive case always did — so the predicate never
reached the guard under test, and every assertion was satisfied by an unrelated
mechanism upstream of the thing being tested.

**What it cost.** This is why a six-case suite could be cited — by the design
*and* by the survey — as proof the opportunity attack worked, while the game
had never fired one. The suite was not weak evidence; in its negative half it
was no evidence at all. It also built the hiding place the ordering bug sat in
for months: the one test that would have caught a dead check was structurally
incapable of failing.

Repaired: each installs the cast, and each was re-measured under its own
mutation. Same 2333 PASS, same test names — now able to fail for their stated
reasons.

**The detector, which belongs in every review from here:** *can this test still
fail for its stated reason?* Answer it by removing the guard the test names and
watching it go red. Tests asserting that **nothing happened** need this most, because
every unrelated failure upstream satisfies them.

**And the measurement lesson that nearly buried it.** Deleting
`isLeavingMyThreatRange` leaves `room` unused, which is a *build* failure — and
a build failure prints no `--- FAIL` line. Grading "no FAIL line" as a pass
reported those two tests as still hollow when the harness had simply failed to
compile. **Neuter the guard (`if false && ...`), do not delete it**, and assert
the mutant BUILDS before trusting any result it gives you.

### 4. `resolution` CAN seat real conditions — the file said otherwise

`movement_test.go` said this module "cannot seat one" and stood a hand-published
trigger in for the condition. It can: resolution is the layer that attaches the
cast, so a participant carrying real condition JSON is all it takes.

That mistaken belief is *why* the only Disengage coverage lived somewhere that
fed a pre-populated event straight to a subscriber — a path production never
runs. The belief created the hiding place.

### 5. Lighting the economy at formation had a discriminator nobody stated

R2's ignition must skip monsters (no economy to light). The obvious
implementation keys on "is there a monster sheet for this id" — and an existing
pin, `TestAMemberWithNoSheetIsUp` ("a sheetless monster is not a broken world"),
caught it immediately: a roster member with no stored sheet went down the
character path and failed the fight it was standing in.

Kind is the ownership-true discriminator; store-presence was a proxy that lied.
The pin earned its keep.

### 6. Two behaviour changes the design did not name

- **A walk now fails if any roster character's sheet will not load.** `castFor`
  is strict for characters and tolerant for monsters ("content with no stored
  sheet contributes nothing"). Attack already carried that strictness; Kirk's
  load-everything law extends it to Move. Fail-closed is right — a world that
  cannot answer for its own member should say so at the verb rather than
  silently at the fold — but it is a change, and rpg-api fixtures with
  rosterless characters will meet it at adoption.
- **Formation writes every player's sheet.** The no-clobber pin's "a move that
  starts a fight touches nothing" case is false by design now; it was inverted
  to assert the economy appears (`reactions_remaining: 1`), keeping the
  ordinary-walk half intact.

### 7. The dead parameter the loop exposed

Once standing went per-step, `runWalk`'s batched `down` argument was overwritten
before first use — staticcheck SA4009. The parameter and `Manager.Move`'s
pre-walk ask both went. Per-step *is* R6's ordering, so the batched copy had
nothing left to be.

---

## What the design got right first try

- **The seam.** "A step is an interaction, a third sibling of `NewBoundary` and
  `NewActivation`" survived contact completely. `NewMovement` needed no
  reshaping — the only change to it was the prevention filter, which is a bug
  fix, not a shape change.
- **`Mover` as Striker's twin.** Required at construction, `RefusingMover` for
  construction-only worlds, `context.Background()` at the call site with the
  same comment. The pattern transferred verbatim, including to `resolve.go`,
  where the argument for `RefusingStriker` and `RefusingAnnouncer` already
  written twice applied to a third capability unchanged.
- **Announce-before-step.** Documented as a contract before anything called it,
  and the mutation reproduces the doc's own sentence verbatim (below).
- **One `ReactionAttacks` for both kinds.** No pressure emerged to split it.
- **The label.** One additive proto field carried the whole log requirement;
  `Missed` carrying it too needed no revisiting.
- **Kirk's load-everything law.** `castFor` with no selection was right, and the
  composability test holds: a new condition that wants to notice a step
  subscribes to `MovementChain` and needs no `runWalk` change.

---

## Mutation ledger — 20 planted, 20 killed

Every pin was shown failing under the exact breakage it guards, then restored
from `cp` backups with `diff` confirming byte-identical restoration.

| Module | Mutant | Killed by |
|---|---|---|
| dnd5e | caller of `character.EndTurn`, in-package | `TestNothingCallsEndTurn` |
| dnd5e | caller in a separate in-module package | `TestNothingCallsEndTurn` |
| dnd5e | prevention check re-added to the condition | `TestPreventionIsNotThisConditionsToApply` |
| dnd5e | readiness gate removed | `TestNoTriggerWhenReadinessOff` |
| dnd5e | not-self gate removed | `TestNoTriggerOnSelfMovement` |
| dnd5e | leaving-reach gate neutered | `TestNoTriggerWhenStillInReach` |
| dnd5e | leaving-reach gate neutered | `TestNoTriggerWhenMoverNeverInReach` |
| encounter | step-then-announce | `TestTheStepIsAnnouncedBeforeItIsTaken` |
| encounter | announce once per intent, not per cell | `TestAWalkIsAnnouncedCellByCell` |
| encounter | dropped `NewEncounter` Mover door | `TestSetupRefusesAnEncounterWithNoMover` |
| encounter | dropped `Reaction.Ref` presence check | `TestAReactionWithNoRefOrNameIsRefused/empty_ref` |
| encounter | reaction never written to the payload | `TestTheBeatSaysWhichReactionItWas` |
| encounter | standing check removed | `TestAReactionThatDropsTheMoverStopsTheWalk` |
| encounter | standing asked AFTER the step | same, on the leaving-cell assertion |
| resolution | prevention filter removed | `TestAPreventedOpportunityAttackDoesNotSwing` |
| resolution | filter widened to every trigger kind | `TestPreventionIsPreciseToOpportunityAttacks` |
| resolution | filter deleted | `TestBothRealConditionsOnOneRealFold` |
| session | `sheet.EndTurn` probe | `TestNothingHereEndsACharactersTurn` |
| session | announce moved after the step | two of the three E2E tests |
| session | formation ignition removed | the inverted no-clobber pin |

### The exemplar

Moving the announce to *after* the step reproduces the failure
`resolution/movement.go`'s doc predicts, word for word:

```
move: no target in reach: requested skel-1 reacts to fighter:
resolution: target is out of range: distance 2 cells exceeds maximum 1 cells (5 feet)
```

The doc had said: *"a caller that walks the member first and announces
afterwards hands the strike a target who has already gone: the swing is refused
as out of range and, because a refused reaction fails the interaction, the whole
walk dies with it."* The mutation turned that sentence into an executable
statement.

---

## Method notes worth keeping

**Two false-green shapes, both met in one slice.**

1. A `-run` filter matching nothing prints `PASS` with no subtest line. Guarded
   by grepping the subtest name out of `-v` output.
2. **New:** measuring *absence of a `--- FAIL` line* as a pass reads a **build
   failure** as a passing test — **a mutation harness must assert the mutant
   BUILDS before trusting its result.** Full account in finding 3, where it
   nearly buried the hollowness measurement itself.

**A blanket rename is a worse tool than the compiler.** `#1319` renamed
`CharacterID` → `MemberID` in conditions — but only on the *Data* structs, not
every runtime struct, and never on `combat.ACChainEvent`. A regex pass made 14
edits to fix 8 real errors; the compiler, driven one error at a time, made
exactly 8. `ACChainEvent` keeps `CharacterID`, and the next person to touch that
seam will reach for the same regex.

**Pseudo-versions are the freeze-safe development pin.** With a tag freeze
active on all five rulebook modules, a downstream module cannot pin its
provider. Pinning the *pushed branch commit* works and mints nothing:

```
dnd5e     v0.122.3-0.20260830050339-debb117d7d8a
encounter v0.39.3-0.20260830052928-049d68cef5b2
```

Go derives the base from the highest tag in the commit's ancestry, so these sort
**above** the frozen baselines and MVS selects them — verified by reading the
resolved pins back, not assumed. A naive `v0.0.0-*` would have sorted below and
been silently ignored. Every commit builds standalone, no `replace`, no
committed `go.work`, and the swap for real tags is written into the commit
message so it cannot be lost.

Corollary: doing the pin bump *during* the freeze surfaced #1319's rename drift
early, instead of under time pressure after the release.

---

## What we learned that would change the design

1. **Ask every ruling against every path the slice creates.** R1 added a second
   walk path; R6 was written for one. The mechanical form: after any ruling that
   adds a seam, re-ask each prior ruling "and on the new path?"

2. **A suite is not evidence until its negative half has been mutated.** The
   design and the survey both treated six green OA tests as proof the condition
   worked. Four of them could not fail. Before citing coverage as evidence that
   something works, remove the guard and watch it go red — especially for tests
   that assert *nothing happened*, which are satisfied by every unrelated
   failure upstream of the thing under test.

3. **"Never the part that broke" needs the same proof as "broken".** The design
   asserted the condition survived the rip-out intact. Half of that was true.
   The claim was inherited from the code's appearance, not from a test that
   exercised it, and no one had walked it since the driver was removed. When a
   slice is framed as re-growing a seam, the surviving half deserves one
   end-to-end walk *before* it is designed around.

4. **A predicate that reads state written later is unfixable at its own layer.**
   The OA condition could not have checked prevention correctly no matter how it
   was written, because subscribers run before stages. The general rule: when a
   check depends on the *completed* result of a fold, it belongs to whoever owns
   the fold, and any per-participant version of it is either order-dependent or
   dead. Worth a look at other conditions reading fold state from a subscriber.

5. **Hand-fed test paths are where bugs hide.** Every OA test published the chain
   by hand because a belief ("this module cannot seat one") said the real path
   was unavailable. The belief was wrong and went unchecked, and the hand-fed
   path both hid the ordering bug and made it unreproducible. When a test harness
   simulates a path production runs, verify the simulation is still necessary.

## Shelf

Unchanged from the design's "Not now", plus:

- **Per-cell movement metering.** The two paths still disagree about *when*
  movement is charged: the player walk pays for the whole path up front, the
  monster turn pays per step after the fact (F8). Prone and difficult terrain
  have nowhere to live until that is one answer.
- **The free-roam pump does not announce.** Deliberate — it is the world clock,
  nobody is in a fight, no threatened square to leave — and the code says so at
  the line that would change. A trap that wants to notice a wanderer is what
  changes it.
- ~~**`character.EndTurn` still zeroes the reaction.**~~ **RULED AND FIXED
  2026-08-30** — Kirk: "I see that as a bug and simply needs to be fixed."
  EndTurn now spares the reaction (1da332b on #1320); both source-reading pins
  retired for one behavioural test on the method, which covers every caller
  including unwritten ones. Bonus find: `TestEndTurn_ResetsButStaysInCombat`
  had been asserting the bug was correct — the bug carried positive coverage
  on top of the pin guarding its unreachability.
