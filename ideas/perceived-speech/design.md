# Perceived speech

## Shape

```mermaid
flowchart TD
    Line[An utterance: speaker, spoken-to, words, place, loudness, roll] --> Reach
    subgraph encounter[encounter: the truth, per observer]
      Reach[Loudness sets the word reach in rooms<br/>closed door costs one room]
      Sight[Who sees the speaker<br/>the one sight answer]
      Words[Who catches the words<br/>flat by level, no check]
      Notice[Who only notices a voice<br/>passive Perception, as today]
      Reach --> Words
      Reach --> Notice
      Sight --> Facets
      Words --> Facets
      Notice --> Facets
      Facets[Each observer's facets<br/>speaker? spoken-to? words? roll?]
      Facets --> Beats[One speech beat per facet group,<br/>one utterance id across all of them]
    end
    Beats --> NPC[NPC hearers: the voice on the sounds channel,<br/>facts per the open question]
    Beats --> Assemble
    subgraph session[session: a player's view]
      Assemble[Read the player's own story]
      Dial{Sharing dial}
      Assemble --> Dial
      Dial -->|each their own| Own[Own versions only]
      Dial -->|shared view| Union[Union of the party's versions,<br/>merged per utterance, marked shared]
    end
    Own --> Wire
    Union --> Wire
    Wire[One Speech message + the Perceived part] --> Client[web: bubble, log, toast;<br/>no name without sight, 'someone' rolls]
```

[Story #930](https://github.com/KirkDiggler/rpg-project/issues/930) carries
the operator's direction and every ruling's record. [The walkthrough](README.md)
explains owners, the measured state today, the wire field by field and one
utterance walked to the screen. [The slices](slices.md) are the working plan.
Rows marked **settled** below are agreed law; rows marked **open** are this
design's proposals, awaiting the operator, and are not implementation authority.

## Law

### R1 — Sight tells who; hearing tells what — settled

An observer is told who spoke only if they see the speaker, by the one sight
answer every deed and line already uses. An observer is told the words only if
the words reach them (R2: reach by loudness). An unseen speaker — out of sight, beyond a wall,
invisible, beaten by a sneak — is heard and not named. No reader of the record
learns a speaker's identity from a speech it was not shown seeing.

### R2 — Loudness reaches in rooms; a closed door costs one room — settled

Every utterance has a loudness. The words reach:

| loudness | the words reach |
|---|---|
| whisper | only the member addressed, and only from beside them |
| speak | every member in the speaker's room |
| shout | the speaker's room and the next room over |
| scream | two rooms over |

A room is the engine's wall partition; rooms join through doors. A closed door
on the way costs one room more, the rule every sound already follows. So a
shout crosses an open doorway and stops at a closed door; a scream passes one
closed door or two open doorways. An authored region is presentation and is
never a room.

Catching the words is flat by level: no roll and no Perception. Noticing that
somebody spoke, without the words, stays on passive Perception against the
voice's difficulty plus one step per room, as every sound today. A member who
is down catches and notices nothing. The speaker always perceives their own
line.

This amends the shout ruling recorded on rpg-project#925 ("hearing stays
passive … a shout carries further"): the words of a shout reach by level, not
by passive Perception.

Lead's calls recorded on #930 and not vetoed: catching the words is flat by
level; noticing stays passive; a whisper needs the speaker beside the person
addressed; a room is the wall partition, not an authored region.

### R3 — The roll travels with the words — settled

Whoever catches the words of a line sees its dice. The roll is the speaker's
when the observer also sees the speaker, and belongs to "someone" when they do
not. An observer who sees the speaker but does not catch the words sees no
roll. A speaker always sees their own roll.

### R4 — Who was spoken to is seen, not heard — settled

An observer learns who a line was spoken to only if they can see that person,
or are that person. Catching the words does not name the addressee.

Lead's call recorded on #930 and not vetoed.

### R5 — Sharing is one dial per encounter, three stops — settled

| stop | what a player is shown | what their character can act on |
|---|---|---|
| each their own | only what their own character perceives | only that |
| shared view (default) | what any party member perceives | only what their own character perceives |
| shared knowledge | what any party member perceives | what any party member has learned |

One setting per encounter, authored in the studio. An encounter that does not
author it is at shared view. At the first two stops a character learns from a
friend only in the world: telling, whispering, shouting. At no stop does party
sharing change what any other character holds: a guard recognising one member
is his knowledge about that member, and a party that shares knowledge teaches
the guard nothing.

Shared view changes only what a player is shown; the encounter's truth, each
character's knowledge and every rule's input stay per character. Shared
knowledge reaches rules and is built after the first two stops.

### R6 — One speech message, with a reusable "how you perceived it" part — settled

Every spoken thing rides one speech message on the wire: a player's line, a
creature's answer, a hail, a shout, an overheard word. Its parts are present or
absent: speaker, spoken-to, words, place, loudness, roll. Beside it, a small
part says how it reached this observer: seen, heard, shared by a friend. That
part is its own message so other families can carry it later, one family at a
time, when a use calls for it.

Other sounds (a clang, a crash, a boom) keep today's sound message. `SpeakAct`,
the speech fields of `TableAnswer` and the voice kind of `Sound` keep working,
marked retired, until the client has moved; then they stop being sent and stay
in the contract as deprecated.

### R7 — Speech goes in first; the castle's call from the gate uses it — settled

This lands before more is layered on the gate. The castle's use: a straggler
asks a gate guard to call inside; the guard shouts; a recognised friend in
earshot catches the words, not who said them, and can come back and vouch
(rpg-project#929).

### R8 — The encounter assembles every observer's version — open

The encounter computes, once per utterance, who sees the speaker, who catches
the words, who only notices a voice and who sees the addressee, and appends one
speech beat per distinct set of facets, each audienced to exactly the members
that set describes (the per-reader shape the `sighted` and `recognized` beats
already have). Every version of one utterance carries the same utterance id.
A version never carries a part its readers did not perceive; absent is absent,
never blanked.

| sees speaker | catches words | the version carries |
|---|---|---|
| yes | yes | speaker, words, roll, place, loudness; spoken-to when seen |
| no | yes | words, roll, place, distance in rooms, loudness; spoken-to when seen |
| yes | no | speaker, place, loudness; spoken-to when seen |
| no | no, but noticed | place, distance in rooms, loudness |
| no | no, not noticed | nothing |

Lead's assumptions, open to veto: the member a line is spoken to always catches
its words (a line is only said within Speak reach); a whisper makes no sound
anyone else notices; noticing difficulty by level is content tuned on the walk,
starting at speak 5, shout 0, scream 0, one step of 5 per room as today.

### R9 — The record each utterance leaves — open

The `spoke` beat keeps the check and the deed for the verb; what any observer
is told of the line moves to the speech beats. A creature's spoken answer or
reaction stops writing its words onto a beat every member reads: the
`answered` beat of a pick that says something is audienced to those who see
the creature, and its words travel only on the speech beats. Non-player
hearers keep holding a voice on the sounds channel, kind and place only, so
every `heard: { kind: voice }` table keeps working. Every line said makes its
voice, a player's included.

### R10 — Authored loudness and the authored dial — open

A line's loudness is authored where its words are: on a creature's option and
on an answer row's `say`, as `loudness: whisper | speak | shout | scream`.
Unmarked is speak. The dial is authored once on the encounter document as
`party: { sharing: own | view | knowledge }`; unmarked is view. The compiler
refuses any other word. A whispered line is offered and admitted only from
beside the member it is said to. The studio round-trips both fields.

### R11 — Shared view is assembled where a player's view is read — open

The session assembles a player's story from their own character's versions,
and at shared view from every party member's versions, merged per utterance:
each part present in any party member's version is present, and the Perceived
part names the friends whose senses contributed. It never asks the encounter
to widen an audience, and it never feeds a merged version back into any rule.

### R12 — Shared knowledge folds facts at the party's grain — open

At shared knowledge, a fact a party member comes to know is a fact the party
knows: the encounter audiences that member's knowledge facts (learned facts,
learned cells, pierced concealments) to the party group, the knowledge model's
group grain, so every party member's fold carries them. It never changes what a
non-party member holds, never shares a character's live sight, and never moves
an NPC's records. Acting on a creature only a friend can see is outside this
ruling and returns to the operator with the slice that builds it.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Sight tells who, hearing tells what; an unseen speaker is heard and not named | KirkDiggler | 2026-11-12 |
| R2 | settled | Whisper / speak / shout / scream reach by rooms; closed door costs one room; words flat by level; noticing stays passive; room is the wall partition; amends the #925 shout ruling | KirkDiggler | 2026-11-12 |
| R3 | settled | The roll travels with the words; named when the speaker is seen, "someone" otherwise | KirkDiggler | 2026-11-12 |
| R4 | settled | Spoken-to is shown only to an observer who sees that person | KirkDiggler | 2026-11-12 |
| R5 | settled | One sharing dial per encounter: each their own / shared view (default) / shared knowledge; first two built in this story; party sharing never changes others' holdings | KirkDiggler | 2026-11-12 |
| R6 | settled | One speech message with parts present or absent, plus a reusable Perceived part; other sounds keep `Sound`; old messages deprecated, not broken | KirkDiggler | 2026-11-12 |
| R7 | settled | Speech goes in before more gate work; the castle's guard shouts inside for a straggler | KirkDiggler | 2026-11-12 |
| R8 | open | The encounter appends one speech beat per facet group with one utterance id; addressee always catches; whisper makes no noticed sound; noticing DCs per level | — | — |
| R9 | open | `spoke` keeps the check; a spoken pick's `answered` beat goes to who sees the creature; NPC hearers keep the voice on the sounds channel; a player's line makes a voice | — | — |
| R10 | open | `loudness` on options and answer rows (default speak); `party.sharing` on the encounter (default view); studio round-trip for both | — | — |
| R11 | open | Shared view is the session's per-utterance merge of the party's versions, read-only | — | — |
| R12 | open | Shared knowledge audiences a party member's knowledge facts to the party group; never NPC holdings, never live sight | — | — |

## Open

- **What an NPC catches (R9: the record each utterance leaves):** whether a fact a spoken line teaches, and the
  `heard` channel of recognition, reach those who catch the words rather than
  those who see the speaker. Changes who learns an alarm in the castle.
- **What shared view covers in this story (R11: shared view in the session):** speech and every
  per-observer perception act only, or also the map a friend sees.
- **What a player chooses when speaking (R10: authored loudness and dial):** the line's authored loudness
  only, or a loudness the player picks per line.
- **Speaking to someone who cannot see you:** Speak today refuses a line to a
  creature that does not see the speaker; whether a line may be said to a
  creature that would only hear it.
- **Where a spoken pick's table arithmetic rides (R6, one speech message; R9, the record each utterance leaves):** on the retired
  `TableAnswer` for those who see the creature, or inside the speech message.
- **Acting on what only a friend perceives (R12: shared knowledge at the party grain):** targeting at shared
  knowledge; settled with the slice that builds that stop.
