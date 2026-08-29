# Game Context — implementation (the mini retro, accumulating per phase)

What we thought we were building vs. what we found. Appended as each phase
lands; merges with the design PR at slice end.

## Phase 0 — spike (2026-08-28, evidence only)

Thought: 21 verbs would each need an install line; the door might not have a
cycle-free home. Found: one choke point (`openForWrite`/`open` →
`loadWorldWithBaseline`) covers 17 verbs — only `Join` needs its own line;
session → resolution is one-directional so the door lives in `resolution`.
Four design-touching discoveries became D6–D9 on the PR (sealed
`Participants`, missing `HasShieldEquipped` on the member surface, the
encounter's deliberate ctx sever, seven `OwnerAware` implementors including
unnamed Fighting Style Protection). Bonus: `EffectiveAC` already takes ctx —
Phase 3's plumbing pre-exists; vestigial second room key `combat.WithRoom`
(reached only via dead `TurnManager` — `turn_manager.go:136` calls it, but
`capacity.go:16` records TurnManager itself is used by nothing) and
caller-less `MoveEntity`/`DealDamage` folds queued for Phase 6's sweep.

## Phase 1 — the door (2026-08-28, rpg-toolkit#1286)

Thought: the room's install-before-attach position would force a two-phase
door. Found: nothing in the attach path reads gamectx (all 11 read sites are
condition handlers + one in delivery), so the ordering constraint dissolved —
one unexported `installTruth(ctx, room, cast)` in `truth.go`, called once
after `attachAll`, with `freeReactions`/`defaultReadiness` moved beside it so
the door's file holds every fact and derivation.

Pins: the three `TestNoCodePathProduces*lessInteraction` AST tests re-pointed
in the same commit (they failed en masse mid-move — followed the code, as
they should). New `TestOnlyTheDoorInstallsGameContext` asserts BOTH halves of
R5: sole installer AND unconditionally reached — mutation evidence showed
deleting the door call left the three tenant pins green, so "only caller"
alone was satisfiable with the door never called. The pin deliberately
INCLUDES test files: in-package tests wanting context call `installTruth`
(the sanctioned hatch); `conditions/`' 43 hand-installs are a separate module
out of scan reach — not exempt, unreachable — and Phase 3 is what takes those
readers off the hand-installed path (#1251's lesson: a pin that exempts tests
is blind to "every test installed what production never installed").

Found, for Phase 3 and a future law: **the bus's two topic flavors hand
handlers different contexts.** Chained handlers receive the PUBLISHER's ctx
(`chained_topic.go:104`); typed handlers close over SUBSCRIBE-time ctx and
`Publish` discards the publisher's (`bus.go:99`). All 52 subscription sites
checked: every gamectx reader today is a chain handler, so nothing misbehaves
— but a gamectx read added to a typed handler would fail closed and log
nothing (#1251's shape). Candidate rule when reactions/typed-handler reads
arrive: forward the publisher's ctx through typed dispatch, or law that
gamectx readers must be chain handlers.

## Post-ratification amendments (2026-08-28, before Phase 2)

Thought: the ratified design was final. Found: three Kirk probes ("why does
X need Y") each shrank it — owner handle → cast reads; export-attachAll →
sealed cast + projection entry; verb installs → session installs nothing,
folds live in resolution (R6). Every intermediate had more mechanism than
the final form. This produced the ownership-interrogation discipline (the
who-holds-what table now in design.md) and the standing charter-check rule.

## Phase 1 addendum — the Copilot round (2026-08-28, merged in #1286 @ 891c860)

Thought: the R5 pin's call-expression scan was airtight. Found (Copilot):
aliasing (`with := gamectx.WithRoom`) bypasses a call scan — fixed by
scanning selector expressions ("the only file allowed to NAME an installer;
taking one as a value is still being one"). Rule-3 sweep found the same
shape's SILENT case: a second tenant install added by alias inside the door
kept every pin green while "installed in exactly one place" was false —
proven by mutation against the old pin, which passed it. All four pins now
count names through one helper. The finding's value was the place the
reviewer did not look.

## Phase 2 PR-A — the projection entry (2026-08-28, rpg-toolkit#1287)

Thought: entering the door without a room might force a variant door or a
nil-as-present lie. Found: an untyped nil reads as genuinely absent
(`gamectx.Room` guards ok && non-nil), so `installTruth(ctx, nil, cast)` is
M4-honest — "nobody knows the world here" is TRUE at join. The distinction
that mattered: absent room ("no world") vs empty room ("you are somewhere and
there is nothing there" — false and unfalsifiable). Pinned by
TestTheProjectionInstallsNoWorld. Shape: ProjectCharacter(record in, folded
AC out), cast sealed, attach through attachAll, Resolve/resolveOn split
mirrored. R6 pin grew a foldEntries completeness table: the two fold entries
are the only door callers; a third joins deliberately or fails. One
implementation decision flagged-not-decided-silently: a refusingRoller per
the RefusingStriker idiom (unreachable by construction, loud if wrong).
Honest limit, measured: until Phase 3 the AC VALUE doesn't depend on the door
(Unarmored Defense still reads the handle) — the suite pins the attach (15
vs 12) and holds the door structurally; parity asserted to 15 independently
on both paths so it cannot pass by two wrongs agreeing.

## Phase 2 PR-B — the strictness tripwire fired (2026-08-29)

Thought: rerouting Join through the projection was a pure re-homing. Found:
session loads leniently (drop-and-continue, pinned twice — one pin literally
documents itself as a tripwire for this event), resolution attaches strictly
(#948) — so the reroute changed a verb's observable contract, and the agent
parked the finished branch unopened rather than deciding. The tripwire test
fired exactly as designed. Kirk ruled D10: observable, not refused —
lenient+report on read entries, strict stays on write entries. Also: go get
right after a fresh tag mis-resolves via the proxy (GOPROXY=direct fixes);
the resolution bump forced rulebooks/dnd5e v0.105.3→v0.106.0 transitively.

## D10 PR-1 — the drops stop being silent (2026-08-29, rpg-toolkit#1288)

The toolkit's first deliberate log line: one `warnDropped` helper, all eight
lenient drop branches (seven in character/load.go plus the same apply-time
shape in sheet_keeper.go — flagged, not smuggled). slog to the default
logger, chosen as the boring stdlib convention, strikeable in review, stated
with its evidence in the PR body. The pins assert on what was SAID, not just
that the load succeeded — a load-only test passes before and after and
proves nothing. Absent ref stays absent (pinned both directions: truncated
blob → no ref; well-formed-unknown → the ref) — a placeholder would read
like a ref that exists. Copilot's round caught the capture handler enabling
all levels; the fix also sharpened a test's claim from "nothing logged" to
"nothing dropped", and the mutation was RE-RUN after the filter landed — a
filter is exactly the fix that can green a test for the wrong reason.

## D10 PR-2 — policy per entry (2026-08-29, rpg-toolkit#1289)

One mechanism, an argument apart: Resolve = refuseUnreadable (it persists
sheets — a silent drop is a condition deleted by whatever verb ran, #948);
ProjectCharacter = dropUnreadable (it only reads; the loader now warns by
name, #1288). The lenient branch CALLS LoadFromData rather than reimplementing
it. The contrast is pinned in one test that runs the same unreadable record
through both doors and mutation-fails in both directions. Found along the
way: the v0.109.0 bump necessarily adopted #1283+#1284 into resolution
(version ordering allows no cherry-pick) — monster write-back now includes
the granted OA condition; one count-derived test rewritten to read the end
of the list (what it pins is that the applied condition arrived, not how
many things attach grants on the way), and the agent verified the failure
came from the bump alone before absorbing it. Copilot quota exhausted — the
round DID NOT HAPPEN on this PR, reported as absence, not approval.

## D10 PR-2 addendum — the substitute gate's first round (2026-08-29)

With Copilot's quota out ~3 days, every PR now gets an independent review
(Sonnet, Copilot stance: diff and code only, barred from the design
conversation) before Kirk. First round: #1289 one real nit (a doc naming the
deleted loadPolicy symbol — the doc-claims-what-code-lacks pattern this
chain exists to kill), #1288 retrospectively clean including
no-raw-blob-leakage and no-double-warning checks. A lesson worth keeping
from the fix: a struct that documents one field's zero value as "the safe
answer" and says nothing about its neighbour TEACHES the reader an inference
("this struct thinks about zero values") that the silent field then betrays
— documentation asymmetry is itself a defect. Roller's field now says nil
surfaces deep in monstertraits and points at refusingRoller.

## D10 PR-3 — Join reroutes; Phase 2 closes (2026-08-29, rpg-toolkit#1290)

The parked branch's blocker closed exactly as D10 predicted: both tripwire
pins pass BYTE-IDENTICAL to main — no test edited to make it so. Found: the
barbarian value pin proves the fold (15, not stored-11 or unattached-12) but
NOT the reroute — the old path also answers 15 because Join attaches its own
sheet; measured, not assumed. Hence two pins: value + structural absence
(no session file names EffectiveAC or imports gamectx), because neither is
sufficient alone. The independent review then BUILT two escapes from the
structural pin: a wrapper subpackage (fixed — recursive walk, escape rebuilt
as a mutation and watched fail) and a renamed same-effect method (cannot be
name-matched by construction — the pin's doc now calls itself a literal-match
tripwire, not a proof of absence, with a forward reference to the real
guarantee: session losing its bus entirely, the Phase 3 frame Kirk's
"so session has the bus?" probe set). A pin that oversells itself is worse
than no pin. Also: session's v0.109.0 exposure verified nil — nothing
duck-types condition state; the Join "dual read" is one fetch, one value.


## Phase 3 — session asks, resolution answers (2026-08-29, rpg-toolkit#1291–#1297)

Thought: two independent halves under one rule — three readers move to
`Member(ownID)`, three session call-bus sites move behind entries. Found: the
halves were the same half. Every one of the six was a place holding a bus, and
what the phase actually removed was the bus, not the folds.

Seven PRs, each merged the day it opened. **#1291** put
`HasShieldEquipped` on the member surface (D7) — no reader, purely additive, and
the surface stayed decorative until #1294 consumed it. **#1292** moved Unarmored
Defense onto the cast and produced `member()`. **#1293** moved Martial Arts, both
handlers in one commit because attack and damage disagreed once already (#709)
and reading the same scores through the same channel is what makes agreement
structural. **#1294** moved Unarmored Movement and closed the reader half; four
`OwnerAware` implementors remain and all four are writers. **#1295** built the
three entries in resolution. **#1296** rerouted Join. **#1297** rerouted standing
and preflight, and deleted `newCallBus`.

### The door was correct for a whole phase before it was load-bearing

Phase 2 PR-A recorded an honest limit: the AC value did not depend on
`installTruth`, measured, because Unarmored Defense still read the owner handle.
That note named the migration that would close it. #1295 closed it — deleting
the door now fails SIX tests where it failed none, five value pins plus the AST
pin catching it structurally.

Worth keeping as a shape: **a structural pin can hold a law that is not yet
load-bearing, and writing down which phase makes it bite is what lets the
closure be verified rather than assumed.** The limit note was the most useful
comment in the slice precisely because it said what the test did NOT prove.

### One pattern, four instances: a comment claiming a check the code never made

- **#1291** — a generated mock header naming a `mockgen` command that had not
  run since #1254 deleted `CombatantLookup`.
- **#1292** — a test asserting IN ITS NAME that no game context was needed,
  after a migration that made game context the read channel.
- **#1293** — a scaling test whose comment said "we verify the damage string is
  upgraded to 1d4" and never asserted it; it seeded a roll of 1, which is a
  legal 1d4 result, so it passed identically whether the rule fired or not.
- **#1294** — a TODO describing a registry that no longer existed.

A fifth instance landed in the follow-up (#1298) on the author of this list,
minutes after writing it — and it ran TWO levels deep. The first
cumulative-attach test could not fail against the casts-of-one version; the
rewrite was reported as fixed and that was still false: it failed against a
mutant its author had WRITTEN, and passed against the restored prior
implementation (review caught it by byte-restoring the old blob under the new
tests). The review's own root cause was then also wrong — nothing can observe
a cast during attach in either version; the implementations were behaviorally
equivalent, and the true difference was structural (R4's ordering rule lived
in two copies that had to agree). The pin that shipped —
`TestOrderingIsDecidedInOnePlace` — is the first that verifiably fails against
the prior blob. **A mutant you write yourself tests your idea of the
difference; when a change claims to improve on existing code, the prior
implementation is the only honest mutant.** The process rule that survives:
restore the old code under the new tests before opening the PR. Awareness of
the pattern did not help; running the old code did.

Every one was found by mutation or by a compile break. **None was found by
reading**, including by the reader who had just written the surrounding code.
This is principle 6 (the record tells the truth same-day) seen from the failure
side, and the mechanism worth folding in is narrow: *a comment that asserts a
check is a claim about code, and only the code can be asked.* Prefer a comment
that states what is NOT covered — those aged correctly all phase.

### F2: a ruling exercised in both directions

Ruled lenient-for-all-participants. Implementation surfaced that
`DropUnreadable` is read on `attachAll`'s character branch and never reaches
the monster one — `monstertraits` has one loader with no lenient half — so the
ruling was not implementable in the module where it landed. Verified both ways
before reporting: the probe refuses, and `monster.Load` (what session used)
refuses the same record for the same reason, so nothing regressed. Amended:
premature, not queued, no real path to a corrupt record with short-lived
encounters; the asymmetry stays as a pinned fact.

Rulings carry their scope, and this is the reverse case the principle also
covers: **a ruling can be un-implementable where it lands, and the honest move
is to report that before building a half-version that reads like the whole
one.** The sibling of M5 (admission is a ruling): a ruling about POLICY needs a
check that the policy is expressible where it is ruled.

### Pins that could be walked past

The D11 walker shipped, then review defeated it twice: an interface field
(`struct{ Payload any }`) and a map KEY, both holding a live sheet, both walking
clean. Fixed by failing closed — refuse any interface not named with a reason,
walk both halves of a map. Three interfaces turned up, not the one named; two
are sealed (`Outcome`, `saves.DCSource`) and `Outcome`'s allow-list entry is
only honest because every implementor is walked separately behind an AST
completeness scan.

The reentrancy pin was defeated the same way, by an import alias, and rebuilt to
resolve the import path to its local name.

**The subtler find was in the fix, not the bug.** The walker's cycle guard was a
global seen-set, so a type reachable by two routes was attributed to whichever
field the walk reached first — making the reported PATH order-dependent while
the allow-list was keyed on paths. An unrelated field reordering could have
moved an entry out from under its allowance. **A pin whose key can drift is a
pin that can silently stop guarding**, and that is not visible from a green run.

### The module graph agreed

`go mod tidy` moved `rpg-toolkit/events` from a direct requirement to an
**indirect** one when the last import went. Not planned, and better evidence
than the pin: the dependency graph stating what the seam reaches for.

### Deferred, named

Fidelity follow-up (M2): `factsOf` reporting a non-compiling main hand as
`resolution.ErrBadAttack` rather than `ErrBadParticipant` — the sentinel exists
and is documented for exactly this, and the wrong wrap in #1295 narrowed Join's
vocabulary in #1296 (pinned as today's behaviour); `Preflight` attaching
cumulatively, restoring the one-cast semantics the session loop had; the
resolution-side reentrancy pin, which session honestly cannot hold because it
compiles against a published module. Then, riding the session PR: the
`translateResolution` case and `memberActionsFrom`'s drift-guard.

### What we learned that would change the design

1. **R6's enforceable form is about the bus, not the fold.** "Folds live in
   resolution" is the rule; "session holds no bus" is the version a test can
   hold, and it is strictly stronger — a fold cannot run without one, whatever
   it is named. Two of the three session sites never folded anything; they held
   a bus only because the loader demanded one. Had the design said the bus, the
   phase would have been the same work with a clearer target.
2. **A rule of the form "nothing with X crosses" should get its structural pin
   at ruling time.** D11 was pinnable by reflection, and doing so found an
   interface-shaped hole prose could not have. Written down at ruling time, the
   hole is a design question; written down at implementation time, it is a
   review finding.
3. **A "no attack" case was assumed and does not exist.** An empty hand is an
   unarmed strike. The output shape nearly carried a nil nothing could produce.
   Design-level shapes with optional fields want one probe each before the
   optionality is real.

### Two process misses, ours

- **Nearly hand-tagged a module.** CI auto-tags on merge; the brief said the
  director would tag. Neither was true and the module was already tagged. Caught
  before acting, but the near-miss is the record: *check what the pipeline
  already does before assigning the step to a human.*
- **Re-saved a memory that already existed.** A recall failure, not a record
  failure — the fact was written down and not retrieved. Worth distinguishing,
  because the fixes differ: one is better writing, the other better lookup.

## Phase 4 retro (2026-08-29 — toolkit#1300/#1301/#1302, merged same-day)

**D5 landed as written; the survey's headline held measurably.** "Zero
production call sites mutate a cast member" was proven, not asserted:
`go build` stayed clean through the whole interface split while `go vet`
broke in exactly four files, all test fakes. The phase was compiler
enforcement of an invariant that already held — principle 3's shape, with
the compile-break set as the evidence.

**The review earned its round.** The substitute reviewer defeated the
widening pin's sanctioned-door exemption with a same-named method
(`doorImpersonator.GetEffectiveAC` in the door's own file — matched every
axis the name-based rule checked, pin passed while missing a widening).
Fix: the door is a declaration OBJECT now (`*types.Func`, `Recv() == nil`,
compared via `info.Defs`). The honest-mutant rule ran on its second-ever
PR: the old pin byte-restored under the new test reported zero offenses —
review-defeated-it as a number.

**Second-order lesson — name vs object, twice in one PR.** The alias
escape (Phase 3) was the same mistake about types; the door was it about
declarations. Both fixes were "compare the object." That is now the rule
when writing pins, not a fix pattern.

**Third-order lesson — a prefilter on a pin is a second, unpinned rule.**
The door bug's deeper cause: `worthChecking` selected packages by "imports
combat," and combat does not import itself, so the one package containing
the door was never scanned. A performance prefilter silently narrowed the
pin's scope and nothing said so. Every pin has one; prefilters get the
same adversarial mutation treatment as the pin body. (Found only by
mutating the exemption — reading never would have.)

**"Read-only" is a property of the surface, not immutability.** Members
still alias the live sheets ("a view, not a copy" — deliberate). A rule
holding a member across a fold sees the keeper's writes land. Phase 5
must not read D5 as "the cast hands out snapshots"; one line added to
design.md so this isn't rediscovered.

**The zero-code closer was the seam's own law.** Session — the module
that holds records, hands them to resolution, and takes back answers —
never named `combat.Combatant` anywhere, so splitting the sheet surface
could not touch it. PR C was six lock-file lines. Also caught there: the
`-run`-filter-matches-nothing false green, live (two testify suite
methods; caught by grepping `--- PASS` lines, the recorded discipline).

**Docs age at the READING seam.** `gamectx/doc.go` was already false when
Phase 4 opened — Phase 3 falsified it and Phase 3's retro missed it,
because the stale paragraph lived in the package being read FROM, not the
one being changed. Generalisation: when a phase changes who reads what,
the reading seam's package doc is part of that phase's diff. Phase 6's
doc list should be re-checked now for paragraphs staled by Phases 2–4.

**Deferred ledger (Phase 6 seeds, evidence attached):**
- `MarkClean` is interface-vestigial: zero non-test callers repo-wide.
  `Combatant` is really Member + {ApplyDamage, IsDirty} + a vestige. Same
  sweep family as `combat.DealDamage` (no production caller) and
  `combat.WithRoom`. Beware `mechanics/features`' unrelated `MarkClean`.
- "Only the keeper names `combat.Combatant`" pin — the cross-module
  complement the widening pin explicitly cannot be (its doc says so).
  Pairs with toolkit#1119, the room tenant's version of the D5 argument.
- Type-resolving pins share one `sync.OnceValues` type-check cache (26s →
  15s for three tests); a future pin of this shape joins the cache.

**Process:** worktree torn down after per-PR content diffs against
origin/main came back empty (content, not ancestry — squash merges);
session/v0.40.1 patch-tagged off the `chore(session):` prefix, correctly
signalling "gains nothing, offers nothing new."

## Phase 5 retro (2026-08-29 — toolkit#1303/#1304/#1305/#1306, merged same-day)

**Measurable headline:** the last four direct writers (Raging, Sneak Attack,
Opportunity Attack, Fighting Style Protection) publish instead of write; the
owner handle is dead code (both loader `SetOwner` sites match nothing);
R2 — writes are requests — now holds everywhere by construction, not
convention. Tags: dnd5e v0.115.0 → v0.116.0, resolution v0.24.1, session
v0.40.2. Both dnd5e PRs reviewed clean under the substitute protocol; zero
blocking findings this phase (contrast Phase 4's door impersonation).

**Kirk's naming catch became a law-shaped lesson.** The draft event was
`MarkDirtyRequested` — command-shaped. Kirk's probe ("what changed? it seems
like marking something changed means it's dirty") exposed that every keeper
event is a FACT (ConditionApplied, HealingReceived), so the event became
`ConditionStateChangedEvent`: the condition states "my slice of your sheet
changed where you can't see it"; marking dirty is the keeper's own response.
Generalizable: a request event is named for the fact the requester states,
never for the response it wants.

**F7 — the lookup's third state, and a correction kept honest.** Deleting
the purse surfaced a state the ruling hadn't named: `member()` answering
(nil, false). Disposed fail-closed as M4/D10 applied at a new site, not a
new ruling. The build agent then CORRECTED its own scope claim on evidence —
the permissive mutant fails exactly one test, not two; the fork governs only
the castless fold, unreachable in production by the door pin. Four test
files now install the cast production always installs (the inverse-#1251
shape), and the handle-pinning test was repurposed as the fail-closed pin.

**The timing law held under fire.** Ruled F1: keepers apply synchronously at
publish — the bus is synchronous, so each request lands at the identical
instant its direct-write predecessor did. The proof is the movement suite
passing untouched: `movement.go:133-140`'s documented dependency on OA's
meter landing MID-fold (second trigger in the same walk sees the spent
slot). R7's buffering is for reaction ANSWERS, never sheet writes.

**Scope discipline ran both directions on the same defect class.** PR B
rewrote every comment its own change falsified (loader handoffs,
gamectx/doc.go's "four conditions still hold one") — record-truth-same-day.
PR D found five MORE stale comments in the same file it touched, checked
them against session's INCOMING pin, found them already false at v0.114.0,
and left them for Phase 6 — don't rewrite historical narrative about bugs
you never investigated. Kirk ratified. The rule: fix what YOUR change
falsifies; file what was already false.

**Smaller lessons.** The `chore(module):` prefix mints a PATCH tag (C
predicted v0.25.0, got v0.24.1 — correct: a pin bump changes no API). The
build agent self-caught the `-run`-matching-nothing trap (guessed test name,
cheerful PASS, looked up the real name, re-ran). The monster keeper gained
its missing `ConditionRemoved` row (F5) — the keepers are symmetric for the
first time. Non-compiling mutants prove nothing and two were discarded as
such, stated in the PR body rather than hidden.

**Phase 6 pile (consolidated from both phase surveys):**
`OwnerAware`/`SetOwner` + both loader handoffs (commented dead, kept to die
together); `ConditionRemovedEvent.CharacterID` → member naming; the
character keeper's ToJSON-based removal filter could match on `Ref()` like
the monster's now does; `AttackChainEvent.ReactionsConsumed` keep-or-delete
(F6); six stale `attack_test.go` references (pre-stale at v0.114.0, Kirk
ratified deferral); nil-`Ref()` guard in the monster removal loop
(noted-not-acted, contract says never-nil); `MarkClean`
interface-vestigial + the only-keeper-names-Combatant pin (Phase 4 seeds);
re-check Phase 6's doc list for paragraphs staled by Phases 2–5.

**From the build agent's closing notes (promoted, not paraphrased):**
- **Evidence rule, folklore → law:** any claim naming a test must quote its
  `--- PASS`/`--- FAIL` line — a `-run` filter matching nothing prints a
  package-level PASS, and a mutant that fails to COMPILE looks exactly like
  a caught mutant if you only read the exit code. Both bit this phase; both
  were caught by the habit this rule now names.
- **A survey question Phase 6 inherits:** find where a handle carried
  information its replacement doesn't. The phase's one real design fork
  (F7) existed because `purse != nil` silently encoded KIND ("I am a
  character") and the cast encodes no such thing — neither the survey nor
  the build brief had that fact; two disagreeing pins surfaced it.
- **Report the mutant result, not the reasoning.** The agent's fail-closed
  stakes were overstated until the permissive mutant ran and failed exactly
  one test — first-principles argument alone would have let the
  overstatement stand.
- **Dry-run downstream consumers during the FIRST PR of any
  interface-widening chain** (uncommitted `replace`, minutes of work). It
  turned C and D into zero-surprise steps — the one-fake delta was known a
  day before PR C existed. Standard practice now, not a reaction to a
  stop-and-report rule.

**Teardown:** all four build worktrees removed, branches deleted, prune run,
nothing stranded (`git diff origin/main` empty against the last branch);
review scratch worktrees removed by their reviewers.
