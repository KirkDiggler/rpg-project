# Scout: design paper trail for NPC minds, behavior and selection

Read-only scout, 2026-11-09 local (GitHub timestamps are UTC, several are 2026-11-10).
Sources: rpg-project issues/PRs, `rpg-project/ideas/**`, `rpg-toolkit/docs/{ideas,adr}/**`,
`rpg-toolkit/knowledge/decisions.md`, `rpg-toolkit/mind/behavior/*.go`, PR #2351 worktree docs.
"inferred" marks my reading, not a recorded statement.

## 1. Timeline

| date | artifact | what it decided |
|---|---|---|
| 2025-07-19 | toolkit#71, ADR-0012 | `tools/selectables`: generic weighted random selection tool. |
| 2026-08-31 / 09-02 | toolkit generic npc foundation; `docs/ideas/world-npcs/design.md` | NPC content lives in session; disposition model for placed non-combat NPCs (#311). |
| 2026-09-12/13 | toolkit#1686 `mind/perception` v0.1.0; #1719 `mind/behavior` v0.1.0 | Per-observer intel pass; behavior as a `Mind` ladder (Judge/Rank/Keep) + deeds `stage.Land`. |
| 2026-09-18 | `ideas/creature-table/design.md` R1–R8 (#465/#466), toolkit#1819 | **Ladder retired.** "A creature decides by rolling on an authored weighted table"; temperament = weight profile only; every pick is a seen roll. |
| 2026-09-22 | rpg-project#491 (merged docs PR) | "a monster has no mind object any more — a table, a temper (a percent profile, nothing else) and holdings". |
| 2026-09-22 | `ideas/living-world/dialogue/social-offer-from-the-npc.md` R1–R2 (#494) | No entry, no offer: a creature is socially addressable only if authored. |
| 2026-09-28 | toolkit#1910 (encounter v0.109.0) | "Observed passage and admission before movement" — movement-sense admission, NOT #769's guarded entrance (name collision). |
| 2026-10-13 | `ideas/living-world/the-mad-king/use-case.md` §6 R1–R12 (#569) | Options flat, facts gate (R1); answer table lives on the option, one `Speak` verb (R2); deed is the skill (R3); `requires` judged against actor's own holdings (R9). Built as `encounter/option.go`. |
| 2026-10-13 | toolkit#2043 | `heard`, `fact`, list-form `when`, sounds channel. |
| 2026-10-23..27 | #667 smarter monsters, design PR #673 (still draft), lane #683 | R0 pick rule: "load = weight × the temperament's percent … one die of the summed loads; authored order only maps faces". Bundles = nested `use:` tables (R12–R15). S6 the pick explains itself. |
| 2026-10-27/28 | `rpg-toolkit/knowledge/decisions.md` lines 10–19 (#694) | Primitives never rulebook-built ("an actual law"); a mechanism may grow inside mind first, extraction follows use case; mind may draw on `tools/selectables`; machine = state machine over a bag, closed guard vocabulary, no expression language. |
| 2026-10-28 | rpg-project#700 (OPEN) | Design review asked: mind takes rulebook vocabulary as inputs; a check is a roll vs a bar on d{n}. No design doc exists. |
| 2026-10-30 | rpg-project#737 (OPEN) | BG3-style branching dialogue, shared party choice, tagged speaker lines, knowledge-gated options. No slices. |
| 2026-10-31 | #766 / lane #768 / `ideas/the-world-runs/design.md` R1–R8 | NPC = data state machine; wake only who is due; fresh rulebook-agnostic `mind/machine`; reproducible not seeded. |
| 2026-10-31 | #769 opened; toolkit#2207/#2208, #746 | Guarded entrance requirements; counted Say activity cues preserved through save/record. |
| 2026-11-01 | toolkit#2215 `mind/machine` v0.1.0; #796 | Machine primitive shipped; "intel in motion" brainstorm (no story). |
| 2026-11-02 | #766 S5 + R11 (toolkit#2229/#2230) | Replay from the record; R9 reaction = toolkit#2223, still OPEN/unbuilt. |
| 2026-11-03 | #737 / #769 priority notes | Shared voting is not a playtest prerequisite. |
| 2026-11-03..05 | #836 S1–S2, `ideas/the-guard-holds-the-gate/` | A posted body blocks; a post holds a passage; the bump speaks (guard's table answers). |
| 2026-11-04 | #859 | "Road builders": designer-authored behavior; evolve `mind/machine` in place; procedural generation is a goal. |
| 2026-11-05 | ADR-0049 Accepted; toolkit#2271 (machine v0.1.1), #2277 (behavior v0.19.0); #866 | `mind/behavior` owns composition of parameterized behavior instances; mechanical parameter metadata is enough (no quality-profile framework); patrol yields to investigation and resumes. |
| 2026-11-06 | #871 closed, toolkit#2284 (behavior v0.20.0); design PR #868 still OPEN | `ExpandSequence`: authored finite sequence → ordinary `machine.Recipe` + `behavior.Table` at bind time (R1–R3). |
| 2026-11-07/08 | toolkit#2292 (v0.21.0), #2293 (encounter v0.185.0); #866 closed; #879 | Live adoption of the composer. Profile: composer is 0.15% of a Move. |
| 2026-11-08/09 | #898/#899 closed; machine v0.2.0, behavior v0.22.0 | Inspection: configuration as symbols, decisions as a trace incl. losers. |
| 2026-11-10 | draft PR #916 `ideas/guarded-admission/design.md` | R1, R2, R11 settled; R3–R10 open. |
| 2026-11-10 03:44 | #769 comment "Exploration checkpoint — routines, profiles and selection" | Kirk exploring whether Say options and NPC choices share a selection primitive; explicitly "brainstorming, not authorization". toolkit#2350 filed (selectables docs overstate). |
| 2026-11-10 03:59 | #917 / #918; toolkit#2351 OPEN at aab48ec6; `docs/ideas/behavior-selection/design.md` R1–R7 | `mind/behavior/selection`: Evaluate → Inspect / Draw / Choose; Pick and Deal re-based on it. All 7 rulings "settled" same day; R3/R4 "under delegated implementation choice". |

## 2. Rulings in force that shape behavior/selection

- **No mind object.** "a monster has no mind object any more — a table, a temper … and holdings; one evaluator" (#491). Creature-table R3: "The table is the one surface for what a creature does; the mind ladder, the presets, the decider and `Pump` are deleted."
- **Temperament is weight only.** Creature-table R5: "Temperament is a weight profile only: no entries, no triggers, no memory". Code echoes it: "The moment it grows one of those it is the mind again under a new name" (`mind/behavior/table.go` ~L855). Bare Say/fact rows load at 100 (`Temper.factor`).
- **Pick rule.** #667 R0: candidates are rows whose `when` holds and whose word is affordable; load = weight × percent; one die; order maps faces, not priority. #866: "Row order maps die faces, **not role priority**." Selection R6 preserves this exactly.
- **NPCs are data state machines.** World-runs R1/R3: "a machine whose current state declares what it listens to"; "a fresh, rulebook-agnostic `mind/machine` … the encounter supplies the vocabulary."
- **No seed; replay from the record.** World-runs R6: "No seeded roller; the dice stay as random as they can be; every world roll is on the record and a replay reads them back." Creature-table R7: "Every pick is a roll with the creature as the die's entity, shown with its arithmetic." Selection Draw takes a supplied die and does not seed (consistent). ADR-0049 keeps content-generation seeding (keepgen) separate from gameplay rolls.
- **Per-observer testimony.** Creature-table: "the outcome of anything is testimony the creature holds, never a flag"; mad-king R9: "`requires` is judged against the actor's own holdings"; ADR-0049: "The composer receives lawful facts; it does not consult hidden world truth". #916 R11 adds: arrest by area OR "the arresting guards' own intel", never a live hidden position.
- **Rulebook-agnostic primitives.** decisions.md 2026-10-27: a primitive "works for any rulebook and is never rulebook-built"; the protected seam is primitive ↔ rulebook. 2026-10-28: grow a mechanism inside mind first, extract after the use case defines it. Selection R1 follows this (lives in behavior module, imports nothing game-shaped).
- **Composer boundary.** ADR-0049: `mind/behavior` composes; `mind/machine` owns transitions only; execution, knowledge and generation stay outside; "Weighted action selection remains a distinct operation"; "Role precedence is not manufactured by reinterpreting weighted-table row order."
- **Dialogue / Say / player choice.** Social-offer R1: "No entry, no offer." Mad-king R1: "options are flat, facts gate what shows up. An author tree is something to work toward"; R2: one `Speak` verb with an option id. #737 (goal, not ruled mechanics): shared approaches with character-specific lines, party vote with visible d20 tiebreak; "speaker and the check result are different choices". #916 R1 settled: first admission uses one acting character's Speak/check, no voting.
- **Admission blocking law.** #836: "block by state on the world, never by an invisible permit"; #916 R6 (open) restates "no invisible replacement wall".

## 3. Guarded admission #769

**What it asks** (#769 body): guard auto-initiates at an authored approach; party stops outside; Performance or Persuasion at DC 11; first fail = warning, second fail = arrest into an authored cell keeping equipment; attempt history per party per entrance survives speaker swap, walk-away, rejoin; second try loses first-try advantage, same DC; defeating all gate guards or alternate entry releases the gate; surviving guards relocate once to ballroom posts; everything consistent across players and reloads.

**Settled** (#916): R1 one acting character; R2 attempt memory per entrance for the run; R11 arrest set = entrance area OR arresting guards' intel. **Open**: R3 encounter-owned shared interaction on the existing machine; R4 addressed invitation lifecycle; R5 versioned choice + pending-check reservation; R6 permission ≠ knowledge; R7 consequences on existing actors; R8 claim/advantage mapping; R9 persistence/inspection; R10 delivery bounds; plus the R11 intel predicate (known-location vs Lost/Unknown).

**What it would consume from a selection primitive (inferred from #916):**
- *Requirements*: per-option eligibility from the speaker's holdings (already `encounter/option.go` `Requires`, mad-king R9) and the "first failed claim unavailable on retry" rule (R2). Selection's Met/Unmet/Unknown verdicts fit; nothing maps them yet.
- *Factors*: the guard's invitation answer, chosen by "existing composer selection" (#916 README L72). That path already runs through `behavior.Pick`, which #2351 re-bases on selection.
- *Explicit choice*: the player's reply = `Choose(id)`. But #916 R5 needs a revisioned offer, revalidation, reservation before dice, stale refusal. Selection R5 says the opposite layer: "A captured evaluation is not a current authorization token; execution revalidation remains the consumer's responsibility."
- *Arrest membership*: a set predicate (area ∪ intel), not a weighted or single choice. Selection does not serve it.

**Undesigned there:** the InteractionSelection/revision contract (R5), where the shared interaction lives (R3), the invitation trigger (R4), claim mapping (R8), the intel predicate (R11 open), and reconciliation of #916's "direct capture-set framing" with Kirk's newer search-routine idea (#769 comment 03:44: "Guard intel is therefore a search lead, not automatic physical capture").

**Verdict: the link is mostly nominal today.** Evidence: #917 itself says "focused foundation delivery supporting #769, not implementation of the open admission design"; #916's design text never names selection or #917; #916 settled rows (R1, R2, R11) need none of selection's features; selection R7 excludes "admission, routine handoff". The real connection is indirect: the idea arose in the #769 discussion, and the guard's answer path (Pick) is now built on it. The dialogue proof is a test example, not a consumer of `encounter/option.go` (inferred).

## 4. Proposed and unbuilt

| item | state | evidence |
|---|---|---|
| #700 mind vocabulary as inputs (keys at init; check = roll vs bar on d{n}) | OPEN, no `ideas/mind-vocabulary/` doc, no PR | sealed deed verbs still in `mind/behavior/table.go`; `TemperProfile` still has five fixed fields (Attack/Toward/Away/Flee/Hold) |
| #737 branching dialogue, party voting, tagged speaker | OPEN, "No implementation slices or architecture selected" | no toolkit PR titled dialogue |
| #769 admission R3–R10 + R11 predicate | draft PR #916, no product code | "no product implementation … is claimed" |
| #796 intel in motion (`say` act with fact payload, testimony travels) | brainstorm, "no story, no slice" | — |
| toolkit#2223 R9 "a reaction's pick is the creature's turn" | OPEN, unbuilt; bug toolkit#2281 OPEN | memory + issue state |
| #766 S6b studio `states:` editor, S7 area clocks, S8 plural fights | proposed | #766 table |
| #859 remaining: source/Studio/world adapter proof; procedural generation of bindings | open; ADR-0049 says generation profiles are optional future work | #859 "Remaining" |
| Studio behavior authoring UI | explicitly separate | #898 |
| #868 behavior-bindings design (R1–R3) | PR OPEN, unmerged, though its code shipped as toolkit#2284 | gh pr view 868 |
| #877 live Patrol consumer plan | draft PR, open | gh pr view 877 (live adoption was nonetheless reported via #2293 on #866; inferred stale) |
| #673 smarter-monsters design | draft PR, open, while story #667 lane says complete | design doc lives only on branch `design/667-smarter-monsters` |
| `tools/selectables` contextual weights / nested tables | unimplemented; docs overstate | toolkit#2350 OPEN |
| Routine handoff (another guard picks up a search), personality catalogs, dialogue wire/UI adoption, selectables migration | excluded by #917 R7 | #917 body |
| Selection follow-ups: fractional/additive factors, extraction to another home | "require an actual consumer and a separate design decision" | selection design "Open" |

**Authored-sequences lane:** shipped. `feat/871-authored-sequences` (3 commits, tip eba23e1d, 2026-11-05) sits in worktree `rpg-toolkit/.worktrees/behavior-bindings`, clean; its content was squash-merged as toolkit#2284 → c9fbb6bc, `mind/behavior/v0.20.0`. Only the companion doc PR #868 is unmerged.

**admission-discovery worktree:** `rpg-toolkit/.worktrees/admission-discovery`, detached at e4ac31f8 (toolkit main 2026-11-09), clean. It is the source snapshot cited by the #769 exploration comment and #916's R11 comment links (`testimony.go#L17-L58`, `perception.go#L103-L137`). Read-only discovery, no branch.

Other mind-adjacent toolkit worktrees: `say-activity-{behavior,encounter,session}` (shipped work, #2207/#2208), `review-behavior-selection`.

## 5. Vocabulary collisions

| concept | names used | where |
|---|---|---|
| Disposition that biases choices | temperament, temper, `Temper`, `TemperProfile`, `Tempered` beat, "percent profile", "personality profiles", soldier/coward words | creature-table R5; #491; `table.go`; #466 protos; #769 comment |
| "profile" meaning something else | generation profiles; quality-profile framework (rejected); dnd5e sequence profile (toolkit#1847); `dispositions[]` stance | ADR-0049; #859; resolution strike scripts; #491 |
| Ongoing NPC work | routine, sequence, patrol, duty, work, role, behavior instance, attachment, bundle, pattern | #917/#769 comment; #871 `ExpandSequence`; #866; ADR-0049; #667 bundles |
| "sequence" meaning something else | resolution sequence machine (strikes), session record sequence train | toolkit#1848, #2007 |
| "activity" | `ActivityCue` (animation with Say), NPC activities (animation sets), keep activities (minigames) | `activity.go`; `ideas/npc-activities`; #851 |
| Gate on a candidate | requirement + verdict (Met/Unmet/Unknown), `requires`, `When`/`when:`, fact, guard (machine guard tree), `PredicateSpec`, `pass: <fact>`, affordability, eligibility | selection R2; `option.go`; `table.go`; `mind/machine`; #491; #836 S1c; `Facts.affords` |
| Numeric preference | factor, percent (`Candidate.Percent`), load, weight, share (`Mix`, effective share), multiplier | selection R3; `table.go` L1489; #667 R0; Temper.Mix |
| The choosing act | pick (`Pick`), deal (`Deal`), draw (`Draw`), choose (`Choose`), roll, "selection" | `table.go`; selection |
| "selection / selector" | `selection` package; `tools/selectables`; `behavior.Selector` (target word: enemy/attacker/at); proposed `InteractionSelection` | #2351; ADR-0012; `table.go` L648–717; #916 |
| Candidate type | `behavior.Candidate` and `selection.Candidate`, two types in one module | `table.go:1481`; `selection.go:54` |
| What an NPC says | Say (row line), answer/`Answer`, `Answered` beat, reply, line, option (`encounter.Option`), Speak verb | `answer.go`; `option.go`; #916 |
| "guard" | NPC gate guard vs machine transition guard | everywhere |
| "admission" | toolkit#1910 movement admission vs #769 guarded entrance | encounter v0.109.0; #769 |

## 6. Open questions the trail itself raises

1. **Can a personality "filter" choices?** #769 comment explores "personality profiles that filter or weight choices", but creature-table R5 says temperament has "no entries", and selection R3 says a zero factor "is not a failed requirement". A filtering profile is either a requirement source or a ruling change. Unanswered.
2. **Who produces `Unknown`?** Selection keeps Unknown distinct; `When.holds` is boolean; the R11 intel predicate (known-location vs Lost/Unknown) is open on #916. No caller emits Unknown yet (inferred). Unanswered.
3. **Does `encounter.Option.Requires` migrate onto selection?** Mad-king R1/R9 built player options with their own gate; #917 R7 defers "actual player-dialogue wire/UI adoption". No issue tracks it (none found). Unanswered.
4. **Who owns choice revalidation and reservation?** #916 R5 (open) needs a revisioned offer; selection R5 disclaims authorization. The layer between is undesigned. Unanswered.
5. **How do votes become one Choose?** #737 needs party votes, visible d20 tiebreak and tagged speakers; #737 says "calibration, timing, disconnects and tagged-speaker eligibility still need design". Unanswered.
6. **Is `TemperProfile`'s five fixed words a #700 violation?** #700 asks for rulebook keys supplied at init; the profile struct seals attack/toward/away/flee/hold inside mind, though `Temper` says "the vocabulary [is] the caller's". Unanswered (#700 open, no design).
7. **Who hands a routine to another NPC?** #769 comment: a search routine "may interrupt a guard or be picked up by another guard"; ADR-0049 gives each instance private progress; machine-foundation README L211 says group outcomes "belong outside an individual machine". No owner chosen; #917 R7 excludes it. Unanswered.
8. **When does selection leave `mind/behavior`?** decisions.md says extraction follows a defined use case; selection design says extraction "require[s] an actual consumer and a separate design decision". A player-dialogue consumer would be outside monster-ai's charter ("intentional monster decisions"). Partly answered (wait for a consumer).
9. **What happens to `tools/selectables`?** decisions.md 2026-10-28 names it "the next tool [mind] draws on"; #917 declined to import it; toolkit#2350 says its docs overstate. Two weighted-choice implementations now coexist. Unanswered.
10. **Does an explicit Choose land on the record?** World-runs R6 and creature-table R7 put every roll on the record; inspection (#898) records decisions. A die-free player Choose has no recording contract yet (inferred). Unanswered.
