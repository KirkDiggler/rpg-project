# The mind, taken in whole — survey of 2026-11-09

**Status:** capture document. A survey of what exists under rpg-toolkit
`mind/` and around it, read against toolkit PR
[#2351](https://github.com/KirkDiggler/rpg-toolkit/pull/2351) (behavior
selection, story [#917](https://github.com/KirkDiggler/rpg-project/issues/917)).
It rules nothing. It exists so the next session that asks "what do we have
and what can we make" starts here instead of re-walking the code.

Three scout reports carry the evidence, with file and line anchors:

| scout | file | question it answered |
|---|---|---|
| inventory | [scouts/2026-11-09-inventory.md](scouts/2026-11-09-inventory.md) | what lives in `mind/`, who consumes it, what is unwired |
| live flow | [scouts/2026-11-09-live-flow.md](scouts/2026-11-09-live-flow.md) | how an NPC decides in the running keep, how a player talks to one, where #2351 lands |
| design trail | [scouts/2026-11-09-design-trail.md](scouts/2026-11-09-design-trail.md) | what is ruled, what is proposed and unbuilt, where names collide |
| gate S2 ground | [scouts/2026-11-10-gate-s2-ground.md](scouts/2026-11-10-gate-s2-ground.md) | what an encounter-owned interaction touches: option grammar, machine step, persistence, castle content, studio |

## 1. What we have

Three Go modules, all consumed only by `rulebooks/dnd5e`, almost entirely by
`encounter`.

| module | release | owns | consumed by |
|---|---|---|---|
| `mind/perception` | v0.3.1 (pins at v0.3.0) | per-observer testimony that may be false or stale; `Observe` and `Report` doors; `Held`/`On` reads | encounter (heavy), session, resolution, dungeonspec |
| `mind/machine` | v0.2.0 | a data state machine whose state is the set of triggers it listens to; `Recipe.Compile`, `Program.Step`, `TraceStep`, host-side `Due`/`Listeners` | encounter, dungeonspec; `Listens`/`DueAt`/`TraceStep` reached only through the composer |
| `mind/behavior` | v0.22.0 | the authored weighted table (`Pick`, `Deal`, `Layer`), the composer over behavior instances (`Decide`, `Observe`, `Inspect`, `StateOf`, `DelayWakes`), bind-time `ExpandSequence`, the deeds and sounds channels (`deed`, `sound`, `stage`) | encounter, dungeonspec |

The lineage matters for reading #2351. The four-judgment `Mind` ladder was
retired on 2026-09-18 for one reason: a ladder was a black box an author could
not open, and a table is a tool an author can hold. Everything since has kept
that rule. The pick explains itself (`answered` beat carries roll, die size,
path, candidates, every row's verdict), the composer explains itself
(`InstanceOutcome` traces, decision as a trace including the losers), and the
machine explains itself (`TraceStep`). The one law that binds all of it is
world-runs R6: no seed, every roll on the record, replay reads the faces back.

**How an NPC decides today, in one line each** (live flow §1):
a player verb raises the world clock; `wakePass` pops only who is due; each due
member gets one opportunity; `buildMonsterView` and `factsFromView` gather the
facts; the default driver rolls `behavior.Pick` on the `time` key, or a member
with authored instances goes through `Composer.Decide`, which takes the first
requesting instance by priority and rolls its table; the row becomes an intent;
`answered` and `behavior_decision` land on the record; the client receives a
`TableAnswer` with eligible candidates and the roll, never the ineligible rows
or the path.

**How a player talks to an NPC today** (live flow §2): a player-chosen reply
already exists. Afford's Speak row carries `speak_options`, the player sends
one `option_id`, `encounter.Speak` rolls the NPC's answer from that option's
success or failure rows, and the web `ConversationPanel` renders it. The wire
already has `SpeakOption.available` and `shortfall`. Both are always "available"
because the mad-king R9 probe law keeps a fact-gated line absent rather than
greyed. What does not exist: a reply chain (the NPC's answer cannot open a new
menu), any visible "why not" for a withheld line, and a shared party choice.

## 2. What #2351 is, and what it is not

**What it is.** A standard-library-only subpackage `mind/behavior/selection`:
`Evaluate` captures ordered candidates with named requirement verdicts
(`Met`/`Unmet`/`Unknown`) and named integer factors, folds eligibility and
checked integer shares; `Inspect` returns detached evidence; `Draw` makes one
weighted choice on a supplied die; `Choose` takes an eligible ID with no die.
`behavior.Pick` uses it per visited table level and `behavior.Deal` over the
sorted temperament mix, and the old `land` loop is gone. The code is clean, the
contract tests are real, the independent review found nothing, CI is green on
the exact head. Its design, plan and implementation record sit in the PR
(`docs/ideas/behavior-selection/`) with a module run record. That is the paper
trail this survey leans on.

**What it is not, yet.**

- *A second consumer.* `Choose` and `Unknown` are exercised only by the
  example test. The real explicit-choice consumer, `encounter/option.go` and
  the Speak door, is untouched and no issue tracks moving it (design trail §6
  Q3). The story's title promises routines and dialogue; the PR ships routines.
- *In service of guarded admission.* #916's settled rulings need nothing from
  selection, and its open R5 (a revisioned offer with a reservation before the
  dice) is exactly the layer selection R5 declines to be. The link is that the
  guard's answer path now runs on it (design trail §3).
- *Free.* Each judged level now runs `Evaluate` plus `Inspect` plus row clones:
  measured +1.2 KiB and +16 allocations per `ComposerDecide` on the two-role
  fixture. Picks per raise scale with due members times intents per member,
  and nobody has measured it on the keep (live flow §5).

**Three things to settle before encounter or session bumps past behavior
v0.22.0.** None is a defect in the PR's own module; all three bite at the bump.

1. **Test dice that ignore die size now fail.** The old `land` sent a face
   above the total to the last row. The PR refuses it (`ErrBadTable`,
   `ErrBadMix`), and nothing in encounter or session handles those errors, so
   the raising verb aborts. Confirmed case: the session launch template suite
   deals temperament from `temper: { soldier: 1 }` (die size 1) with a fixed
   face of 10. The flow scout lists six more such dice across session,
   encounter and rpg-api tests. The fix is on the consumer side: the dice were
   lying about their size, and fail-closed is right.
2. **The adapter pays for evidence it throws away.** `judge` calls `Inspect`
   only to read `Total` and each row's `Weight`, cloning every row to do it,
   and `Evaluate` builds the `byID` map that only `Choose` needs. Cheap
   accessors on `Evaluation` and a lazily built index would remove most of the
   measured cost without touching the contract. Worth doing, and measuring on
   the keep harness, before the bump.
3. **`validateTemper` drifts further.** Encounter's hand copy of the deal rules
   pre-checks share below one and unknown profile but not share overflow, which
   now fails at the deal. Exporting the validator from behavior (the PR's own
   stated follow-up) closes the gap.

**Three design notes, none blocking.**

- R3 (integer multiplicative factors only) and R4 were ruled by the
  implementing agent under delegation. R3 is the one that binds a future
  dialogue or personality consumer. Treat it as a hypothesis until a second
  consumer plays it.
- Two `Candidate` types now live in one module (`behavior.Candidate` carries
  entry, weight, percent, loaded; `selection.Candidate` carries ID, weight,
  requirements, factors). The adapter translates between them per level.
  Toolkit law says pick one representation; a later slice can let
  `RowVerdict` carry the selection `Row` instead of a parallel copy.
- The verb set for choosing is now Pick, Deal, Draw, Choose and the composer's
  Decide (design trail §5). Not wrong, but a glossary line in the behavior
  README would save the next reader an hour.

## 3. What we can make

Ordered by how much existing wiring each one reuses. Every item names the
evidence that says it is small or large.

1. **Speak options onto selection (the real second consumer).** `OptionsFor`
   returns a `selection.Evaluation` over the speaker's lines (requirements:
   `requires` verdict, reach, witness, price); the Speak door becomes
   `Choose(option_id)` so offer and door share one evaluation instead of
   agreeing by convention; session projects non-secret `Unmet` rows into the
   `available`/`shortfall` fields that already exist on the wire. Fact-gated
   lines stay absent under R9. Encounter and session only, no proto change
   (live flow §2 steps 1, 3, 4). This is the slice that makes #917's title
   true.
2. **A reply chain.** An `Answer` row that names the next option set
   (`opens:`), so an NPC's answer can hand the player a new menu. New
   behavior vocabulary, dungeonspec grammar, a studio round-trip test, and the
   web panel staying open after an answer. This is the first real slice of
   branching dialogue (#737), and it needs the item above first.
3. **The offer-version layer for guarded admission.** #916 R5 needs a
   revisioned offer, revalidation and a reservation before the dice. Selection
   says that is the consumer's job. The home is session (it already owns offer
   and door for social verbs, per the social-offer ruling). Design it there,
   once, for Speak and for the gate.
4. **Personality that filters.** Kirk's exploration comment on #769 asks for
   profiles that filter or weight choices. Creature-table R5 says temperament
   is weight only, and selection R3 says a zero factor is not a failed
   requirement. A filtering profile is therefore either a requirement source
   the caller supplies, or a ruling change. This is a call, not a build.
5. **A record contract for `Choose`.** Every roll lands on the record; a
   die-free explicit choice has no recording shape yet. Small, but it decides
   whether a player's reply is replayable the way a roll is.
6. **Routine handoff between NPCs.** A search routine that another guard picks
   up. Excluded by #917 R7, no owner, and the machine-foundation README says
   group outcomes belong outside one machine. Needs a design before a slice.
7. **Housekeeping with real value.** Resume `mind/behavior/knowledge/decisions.md`
   (it stops at 2026-10-29 while three weeks of decisions sit in run records);
   close or merge design PRs #868, #877 and #673 whose code shipped; give
   `behavior/deed` its first tests; prune the merged `behavior-bindings` and
   empty `admission-discovery` worktrees; decide `tools/selectables`' fate
   (toolkit#2350: its docs promise what it does not do, and it has no
   eligibility to lend).

## 4. Calls this survey surfaces

Recorded here so they can be walked one at a time and ruled on the owning
issue, never in this file.

- Merge #2351 as the provider now, with the three pre-bump items as the
  consumer bump's checklist, or hold the merge until the cost trim is in.
- Whether item 1 above becomes S2 of #917 or its own story.
- Whether a profile may filter (requirement source) or only weight.
- Whether an explicit `Choose` is recorded like a roll.
