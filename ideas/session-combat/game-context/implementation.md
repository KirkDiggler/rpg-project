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
