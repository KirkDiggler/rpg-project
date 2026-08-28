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
