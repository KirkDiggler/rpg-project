# Perceived speech — measured start, open calls and slices

Working plan for [#930](https://github.com/KirkDiggler/rpg-project/issues/930),
beside [the design](design.md) and [the walkthrough](README.md). It changes as
slices move; the law does not live here.

## Measured state

Read 2026-11-12 at `rpg-toolkit` main, `rpg-api-protos` main, `rpg-api` dev
and `rpg-dnd5e-web` dev.

**A player's line travels by sight and makes no sound.**

- `Speak` asks the one door, whose witnesses are `audienceOf`: every placed
  member the sight reach says sees the speaker, plus the speaker
  (`encounter/deed.go:223`, `encounter/interaction_hold.go:376`). A target
  that does not see the speaker is refused `ErrUnwitnessed`
  (`interaction_hold.go:397`).
- `appendSpokeBeat` writes actor, target, option, line, skill, DC, total,
  beaten and the calculation onto one `spoke` beat audienced to those
  witnesses (`encounter/option.go:895`). Nothing calls `makeSound` for a
  player's own line.
- Speak reach is the session's `InteractionReach = 3` cells
  (`session/offers.go:974`); loudness exists nowhere. `SpeakRequest` carries
  session, member, target, option id and revision only
  (`rpg-api-protos` `session/v1alpha1/service.proto:1050`).

**A creature's answer to a line goes to the speaker's witnesses, words and
all, and is also a voice.**

- `answer` teaches a row's fact to `in.witnesses` — those who see the
  *player* (`encounter/answer.go:212`) — writes `answered` to the same
  witnesses (`answer.go:336`), then `makeSound(noiseSay)` for a `say`
  (`answer.go:235`).

**A creature's own-time line, hail or bump answer goes to every member.**

- `appendPickBeat` audiences the `answered` beat, `say` and `to` included, by
  `audienceFor(subjectBeat, member)` (`encounter/clocks.go:803`, `:828`), and
  `audienceFor` returns the whole roster for every subject beat
  (`encounter/encounter.go:1846`). A hail or a barred answer therefore names
  its speaker and its words to everyone.
- `answerTime` makes the voice and teaches an alarm's fact to the creature's
  sight witnesses together with every voice hearer by passive Perception
  (`answer.go:259`).

**Hearing is a kind and a place, by passive Perception over wall partitions.**

- Voice DC 5, clang 5, crash 5, boom 0; `dcPerHop = 5`
  (`encounter/noise.go:83`, `:88`). `hopsFrom` is a Dijkstra over every wall
  partition, a closed door costing one hop more, with an unbounded search
  that scans its frontier map each step (`noise.go:232`, `:239`).
- `hearersOf` skips the maker and down members and requires passive
  Perception ≥ DC + 5 × hops (`noise.go:290`). `makeSound` lands the sound on
  every hearer's sounds channel through `sound.Land` and writes a per-player
  `heard {listener, kind, position, hops}` beat (`noise.go:366`, `:417`).
  `mind/behavior/sound` carries no actor by construction.

**The per-reader beat pattern exists.** `tellRecognition` groups readers by
what each perceives and writes one beat per group
(`encounter/recognition.go:528`); `sighted` is one beat per observer.

**Knowledge folds per member; the party is a graph group.**

- `learnFact` audiences each learned fact to the learner alone
  (`encounter/flip.go:52`); members are declared `GrainIndividual`
  (`encounter/world.go:285`) under the graph's group-grain default
  (`world/graph/graph.go:47`). Readers of the fold: option `requires` facts
  (`option.go:432`), the machine's `knows` (`encounter/machine.go:76`), cells
  learned and pierced concealments (`world.go`).
- The party is `FactionParty` (`encounter/field.go:68`), a group every player
  `belongs-to` in the graph.

**The session reads each player's story alone.** `Acts` and the live commit
both read `enc.Story(Audience: member)` (`session/acts_read.go:82`, `:128`).
`spokePiece` builds `SpeakAct` (`session/act_family.go:737`);
`answeredPiece` builds `TableAnswer` and folds it into an open `SpeakAct`
(`act_family.go:772`, `:827`); `heard` becomes `Perception{Sound}`
(`act_family.go:1090`). The launch takes the compiled dungeon
(`session/launch.go:78`), whose `Field` is persisted as field data.

**The wire has three messages for one thing.** `SpeakAct`
(`act.proto:532`), `TableAnswer` with `say` and `to` (`act.proto:707`) and
`Sound` (`act.proto:834`) inside `Perception` (`act.proto:885`); the api
converts each (`rpg-api` `convert_act.go:681`, `:778`, `:840`).

**The client renders speech from three places.** `speechAtSpeakers.ts` puts a
`TableAnswer.say` over `who`; `toastPolicy.ts` decides `own-speech`,
`speech-to-you`, `answer-to-your-line`, `answer-to-your-bump`, `line-to-you`
and `heard-sound` (`toastPolicy.ts:225`–`275`); `describeAct.ts` writes the
log lines; `heardBeat.ts` phrases `voice` as "a voice". The studio's
encounter home has sections for intel, factions, endings, concealments,
interactions, scenarios and facts, and no encounter-wide setting today
(`EncounterStudioWorkspace.tsx`).

### What contradicts a ruling or makes it expensive

- **Reaction lines reach everyone** (`appendPickBeat` through `audienceFor`):
  directly against R1 (sight tells who). R9 (the record each utterance leaves) narrows that one beat at its call site rather than
  flipping the shelf for every beat.
- **A creature's answer is audienced by who sees the player, not who
  perceives the creature**, and its fact teaches the same set. Every answer
  path moves.
- **"Each their own" is not honest outside speech.** Moves, casts, barred
  bumps and picks still reach every member through `audienceFor`; the dial's
  first stop narrows only what this story makes per-observer.
- **The #925 shout ruling put a shout's words on passive Perception**; R2 (reach by loudness)
  makes them flat by level. The design records the amendment.
- **The ruling names `TableAnswer` retired**, but `TableAnswer` also carries
  temperament deals and silent table picks; only its speech fields can retire
  without losing those (Open).
- **Speak refuses a target that cannot see the speaker** — eavesdropping on a
  player is possible after this story, speaking unseen is not (Open).
- **Shared knowledge reaches every knowledge reader at once**: option
  requirements, machine `knows`, cells and concealments. It is the expensive
  stop, which is why it is built last.
- **Unbounded partition search per sound**: harmless today, multiplied by
  every utterance after this story; the utterance search is bounded.

## Open calls, with recommendations

1. **What an NPC catches.** A: facts a line teaches, and recognition's `heard`
   channel, go to whoever catches the words. B: keep the speaker's sight
   witnesses (today). C: both. **Recommend A**: it is R1 (sight tells who) applied to minds;
   the castle's alarm rows then need `loudness: shout` to reach the yard.
2. **What shared view covers now.** A: speech and every per-observer
   perception act (sounds, sightings, rolls) in the story. B: A plus the map —
   a friend's sight on the board through `View`. C: speech only. **Recommend
   A**, with B as its own slice after the castle if wanted.
3. **What a player chooses when speaking.** A: the line's authored loudness.
   B: the player picks a loudness per line. C: authored default the player
   may change. **Recommend A** until a player-to-player telling verb brings
   the use case.
4. **Speaking to someone who cannot see you.** A: keep Speak's refusal.
   B: admit a line to a target that catches the words. **Recommend A** for
   this story; B with a use case.
5. **Where a spoken pick's table arithmetic rides.** A: `TableAnswer` keeps
   the pick's arithmetic (die, candidates, entry, temperament) for those who
   see the creature; `say` and `to` retire; the speech carries the words and
   the die. B: the speech carries the arithmetic too and `TableAnswer` stops
   being sent for spoken picks. **Recommend A**: the decision is seen, the
   words are heard, and silent picks keep one message.
6. **Acting on what only a friend perceives** at shared knowledge: settled
   with the slice that builds that stop, from its own measurement.

## Slices

Toolkit is one module per PR. "Review" marks a slice that gets an independent
review round.

| # | Deliverable | Repos and modules | Review |
|---|---|---|---|
| S1 | Words travel by loudness; each observer gets their version | toolkit `encounter`, then `session` | yes, both |
| S2 | One speech message on the wire, and the client shows it | protos, toolkit `session`, api, web | yes, `session` |
| S3 | The sharing dial: each their own, shared view | toolkit `encounter`, `session`; api; web studio | yes, `session` |
| S4 | The gate guard shouts inside | toolkit `encounter` (castle content), session, api, web repins | no, unless it finds an engine gap |
| S5 | Shared knowledge | toolkit `encounter`, `session`; web studio | yes, `encounter` |

### Perceived speech S1 — words travel by loudness; each observer gets their version

**What it is.** The engine's utterance: loudness on lines and answer rows,
word reach over the wall partitions, the facet table, one speech beat per
facet group with one utterance id, a voice for every line said.

**What it does when done.** A player beyond a speak line's room no longer
reads its words because they could see the speaker; a guard's hail or bump
answer no longer reaches players who neither see nor hear him; a player who
catches words without sight is told a voice and no name through today's
messages.

**How it works.** R1 (sight tells who), R2 (reach by loudness), R4 (spoken-to is seen), R8 (the encounter assembles each version), R9 (the record each utterance leaves), R10 (loudness only). `encounter`:
`dungeonspec` compiles `loudness`; `utter` replaces the answer and pick paths'
direct audiences; a bounded partition search; `spoke` keeps the check; a
spoken pick's `answered` goes to those who see the creature. `session`:
decodes the speech beat into today's messages honestly (no `SpeakAct` or
`TableAnswer.say` to a reader without sight and words; a voice `Sound` to one
with words and no sight until S2 (the speech message on the wire)).

**Done means.** Encounter and session merged on tags; api and web repinned on
dev; tests for every row of the facet table, each loudness through open and
closed doors, the addressee, a down member, a sneak-beaten watcher, the
studio round-trip of `loudness`; looked at by the operator.

**Verification checklist** (two clients, HTTPS).

- [ ] A speaks to the gate guard; B stands one room away through an open door, in sight of A → B is not shown A's words or roll.
- [ ] Same, B in A's room out of sight behind a pillar → B is shown a voice from A's place, no name.
- [ ] B walks up to a guard and is hailed; A is two closed doors away → A is shown nothing of the hail.
- [ ] A studio line marked `shout` saves, reloads and still reads `shout`.

### Perceived speech S2 — one speech message on the wire, and the client shows it

**What it is.** `Speech`, `Loudness`, `Perceived` in protos; the session's
speech act; the api conversion; the web rendering; the old messages marked
deprecated.

**What it does when done.** Words heard without sight show as words from
someone at a place; a roll heard without sight is "someone rolls 1"; the
addressee is named only when seen; a bubble sits over a seen speaker, and a
voice cue sits at the place of an unseen one.

**How it works.** R3 (the roll travels with the words), R4 (spoken-to is seen), R6 (one speech message). Protos merge first (contract). The session emits
`Speech` beside the retired messages; the web moves every renderer
(`speechAtSpeakers`, `toastPolicy`, `describeAct`, `ConversationPanel`,
`RecentActs`, `heardBeat`) to `Speech`; then the session stops sending the
retired ones.

**Done means.** Protos merged and released; session merged on a tag; api and
web merged on dev; the retired messages no longer sent; looked at by the
operator.

**Verification checklist** (two clients, HTTPS).

- [ ] A speaks a checked line to a guard; B is in the room out of sight → B's log shows the words and "someone rolled N", no name.
- [ ] B steps into sight and A speaks again → B's log names A and shows the roll as A's.
- [ ] A whispers a line to a guard beside them; B watches from across the room → B sees A whispering to the guard, no words, no roll.
- [ ] A speaks to a guard B cannot see → B's line names no addressee.

### Perceived speech S3 — the sharing dial: each their own, shared view

**What it is.** `party: { sharing }` authored in the studio; the session's
per-utterance merge at shared view; `Perceived.via`.

**What it does when done.** At shared view (the default) a player is shown
what any party member perceives, marked as shared by that friend; at each
their own, only their own character's versions. No rule changes at either
stop.

**How it works.** R5 (the sharing dial), R10 (the dial), R11 (shared view in the session); the coverage per Open call 2.
`encounter` carries the compiled setting on its field data; `session` reads
it and merges party members' stories; web adds the dial to the encounter
home and its round-trip test.

**Done means.** Encounter and session merged on tags; api and web on dev;
tests for the merge (parts unioned, own version preferred, `via` named),
reload keeps the setting, a merged version never reaches `Afford` or
`Speak`; studio round-trip; looked at by the operator.

**Verification checklist** (two clients, HTTPS).

- [ ] Shared view: A speaks to a guard while B is two rooms away → B is shown A's line and roll, marked shared by A.
- [ ] Each their own (set in the studio): same → B is shown nothing.
- [ ] Shared view: A overhears a line that unlocks a fact → B's Speak menu does not gain the line that fact unlocks.
- [ ] The setting survives save, reload and a studio round-trip.

### Perceived speech S4 — the gate guard shouts inside

**What it is.** The castle's ask line for a straggler and the gate guard's
shouted answer, the last bullet of rpg-project#929.

**What it does when done.** B, a straggler at the gate, asks a guard to call
inside; the guard shouts; A, admitted and out of sight in the yard, is shown
the shouted words with no name, walks back and vouches.

**How it works.** R2 (reach by loudness), R7 (speech first, the gate shout); S1 (words by loudness) to S3 (the sharing dial). Castle content only: the ask option on
the gate guards, the answer row with `loudness: shout`; castle tests.

**Done means.** Encounter merged on a tag (castle content and test); session,
api, web repinned; looked at by the operator.

**Verification checklist** (two clients, HTTPS).

- [ ] B asks a gate guard to call inside; A is in the yard, out of sight → A is shown the shout's words with no name.
- [ ] A is in the great hall behind a closed door → A is shown nothing.
- [ ] A returns to the gate and vouches with B in the guard's sight → B walks through.

### Perceived speech S5 — shared knowledge

**What it is.** The dial's third stop: a party member's knowledge facts folded
at the party's grain.

**What it does when done.** At shared knowledge, a fact one member learns
offers its lines to every member; at the other stops it does not; no guard,
captain or other creature learns anything from the party's sharing.

**How it works.** R12 (shared knowledge at the party grain) and Open call 6, settled first. `encounter` audiences
learned facts, cells and pierced concealments to `FactionParty` when the
field's dial is knowledge; `session` and web enable the third stop.

**Done means.** Encounter and session merged on tags; api and web on dev;
tests for every fold reader at each stop and for no change to non-party
holdings; looked at by the operator.

**Verification checklist** (two clients, HTTPS).

- [ ] Shared knowledge: A learns the password from a guard's answer; B, elsewhere, is offered the password line at another guard.
- [ ] Shared view: same → B is not offered it.
- [ ] Shared knowledge: the guard who recognised A does not recognise B because of it.
