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
  directly against R1 (sight tells who). R9 (a line teaches whoever catches its words) narrows that one beat at its call site rather than
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
  temperament deals and silent table picks; only its speech fields retire
  (R9, a line teaches whoever catches its words, lead's call).
- **Speak refuses a target that cannot see the speaker** — eavesdropping on a
  player is possible after this story, speaking unseen is not (R13, speaking
  unseen stays refused).
- **Shared knowledge reaches every knowledge reader at once**: option
  requirements, machine `knows`, cells and concealments. It is the expensive
  stop, which is why it is built last.
- **Unbounded partition search per sound**: harmless today, multiplied by
  every utterance after this story; the utterance search is bounded.
- **The castle's alarm spreads by voice today**: `answerTime` teaches
  `alarm-raised` to every passive-Perception hearer. Under R9 (a line teaches
  whoever catches its words) it reaches only the words' reach, so the alarm
  rows are authored `shout` in the first slice.

## Still open

- **Shared knowledge (R12, the party grain):** the fold and acting on what
  only a friend perceives are settled with the shared-knowledge slice, from
  its own measurement of every knowledge reader.

## Slices

Toolkit is one module per PR. "Review" marks a slice that gets an independent
review round. The sub-issues of #930 carry the live checklists; this table is
the plan they were cut from.

| # | Deliverable | Repos and modules | Review |
|---|---|---|---|
| S1 [#933](https://github.com/KirkDiggler/rpg-project/issues/933) | Words travel by loudness; each observer gets their version | toolkit `encounter` (with its `dungeonspec`), then `session`; api and web repin | yes, both |
| S2 [#934](https://github.com/KirkDiggler/rpg-project/issues/934) | One speech message on the wire, and the client shows it | protos, toolkit `session`, api, web | yes, `session` |
| S3 [#935](https://github.com/KirkDiggler/rpg-project/issues/935) | The sharing dial: each their own, shared view | toolkit `encounter`, `session`; api; web studio | yes, `session` |
| S4 [#936](https://github.com/KirkDiggler/rpg-project/issues/936) | The gate guard's hint is a shout | toolkit `encounter` (castle content); session, api, web repins | no, unless it finds an engine gap |
| S5 [#937](https://github.com/KirkDiggler/rpg-project/issues/937) | Shared knowledge | toolkit `encounter`, `session`; web studio | yes, `encounter` |

**Words by loudness (S1)** — the engine's utterance: `loudness` compiled on
options and answer rows; word reach over the wall partitions with a bounded
search; the facet table; one speech beat per facet group under one utterance
id; a voice for every line said, a player's included; a line's fact and
recognition's `heard` channel to whoever catches the words; a spoken pick's
`answered` beat to those who see the creature; the castle's `alarm-raised`
rows authored `shout`. The session decodes speech beats into today's messages
without leaking a part. Rulings: R1, sight tells who; R2, reach by loudness;
R4, spoken-to is seen; R8, the encounter assembles each version; R9, a line
teaches whoever catches its words; R10, the loudness is the author's.

**The speech message (S2)** — `Speech`, `Loudness` and `Perceived` in protos;
the session's speech act; the api conversion; every web renderer moved; the
retired messages deprecated and then no longer sent. Rulings: R3, the roll
travels with the words; R4, spoken-to is seen; R6, one speech message.

**The sharing dial (S3)** — `party.sharing` authored in the studio and carried
on the field data; the session's per-utterance merge over speech and every
per-observer perception act at shared view; `Perceived.via`. Rulings: R5, the
sharing dial; R10, the loudness is the author's (the dial's authored word);
R11, shared view shares what friends hear and notice.

**The gate shout (S4)** — the castle's gate guards answer the unrecognised
speaker's plain line (added by rpg-project#932) with a hint that a friend can
speak for them, authored `shout`, and the castle reads back. Rulings: R7,
speech first, the gate shout; R14, the hint is the author's tool.

**Shared knowledge (S5)** — the third stop: a party member's knowledge facts
folded at the party grain; targeting on a friend's sight settled first.
Rulings: R5, the sharing dial; R12, shared knowledge at the party grain.
