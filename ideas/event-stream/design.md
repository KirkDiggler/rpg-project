# Event stream — the record and the act, on session v1alpha2

Session v1alpha2 carries two streams cut from one record. The **record stream**
shows every beat encounter writes, as written. The **act stream** shows what
happened as acts: who did what, to whom, how it was decided, what changed. The
package holds only the streams; the verbs stay in v1alpha1. The pass extends
the event-projection design (`ideas/event-projection/design.md`,
rpg-project#541): encounter states facts, the session projection shapes
delivery, rpg-api transports. Explanation: [walkthrough.md](walkthrough.md).
Tracking: rpg-project#631.

## Shape

```mermaid
flowchart LR
    subgraph encounter[toolkit encounter — facts]
        A[act: a verb or a driven activation<br/>mints one act id]
        B[record beats<br/>each carries its act id]
    end
    subgraph session[toolkit session SDK]
        R[record read<br/>beat name · act id · seq · at · fields]
        P[act projection<br/>beats to acts, delivered as segments]
        S1[per-step read<br/>for v1alpha1 until deleted]
    end
    subgraph api[rpg-api]
        CR[StreamRecord · GetRecord]
        CA[StreamEvents · GetStory]
        C1[v1alpha1 stream, untouched]
    end
    subgraph web[web]
        D[debug panel<br/>filter by type + act id]
        U[player UI<br/>one form per family]
    end
    A --> B
    B --> R --> CR --> D
    B --> P --> CA --> U
    B --> S1 --> C1
```

The act families, one body shape each:

| Family | Carries | Examples |
|---|---|---|
| act | a member's verb or a driven activation: who, what, to whom, the `Roll`, the outcome inside the body | walk, attack (hit, miss or warded), speak, cast, loot, rest |
| reaction | the same as act, nested under its trigger's act id | opportunity attack, a ward |
| table answer | word, say, option, candidates, the roll of N | a creature's answer, an idle pick |
| state | a standing change to one party | downed, stance, concentration ended, condition removed, experience |
| perception | what one observer came to know | sighted, heard, snuck, revealed, cells |
| clock and roster | the frame the acts happen in | turn, fight formed or dissolved, tick, joined, exited, arrived, scene, ended |

Two shared messages. `Roll` is the expression, the raw dice, the modifiers
with their sources, the total, what it was against, and whether it beat it.
`Party` is a kind and an id, for a member, a prop, a faction or the world.

```mermaid
flowchart LR
    E1[E1 envelope<br/>toolkit · #627] --> E2[E2 record stream<br/>protos + api + web debug · #628] --> E3[E3 act projection<br/>toolkit session SDK · #629] --> E4[E4 act stream on v1alpha2<br/>protos + api + web · #630] --> E5[E5 forms per family · scene movement<br/>web#1301 · web#1293] --> D[delete v1alpha1<br/>StreamEvents and GetStory]
```

The pass is done when the v1alpha1 StreamEvents and GetStory are deleted (R2).

## Law

1. **Two streams, one record.** The record stream and the act stream are both
   read from the record encounter writes; neither holds a fact the record
   lacks.
2. **An act names itself.** Encounter mints one act id per act, and every beat
   appended inside the act carries it as `correlation`. A reaction carries its
   trigger's act id.
3. **The envelope is `seq`, `at` and `correlation`.** `at` is the world-clock
   reading in rounds, shared by every beat in a round; order is `seq`, never
   `at`.
4. **The record stream is the beat as written:** beat name, act id, record
   seq, `at`, and the encounter fields as `google.protobuf.Struct`. It has no
   typed bodies. Its whole contract is that a reader can narrow it by act id
   and by beat name.
5. **The unit of the act stream is the act.** An act is delivered as segments
   under one act id and folds back into one act on replay; a walk is the
   model. Any other beat delivered to that recipient mid-act closes the
   segment (R19).
6. **An act says what happened.** Its body names who, what, to whom, how it
   was decided and what changed. The outcome lives inside the body; an attack
   is one act that hit, missed or was warded, never three kinds.
7. **A beat with no act appears only in the record stream.**
8. **The player UI reads acts; the debug panel reads the record** (R14). A
   form that needs a record detail means the detail belongs in the act body.
9. **Filtering is outside both shapes.** Neither stream's bodies encode who
   may see them; event-projection R5 stands before release.
10. **v1alpha2 imports nothing from v1alpha1.** v1alpha1's stream is left as
    is until it is deleted.
11. **Within a package, protos change additively:** a replaced field is
    `[deprecated = true]` and unwritten; it never changes type in place.

## Rulings

| ID | ruling | status | scope | ruled by | date |
|----|--------|--------|-------|----------|------|
| R1 | The pass extends event-projection R1–R5 and reopens none of them | settled | all | KirkDiggler | 2026-10-20 |
| R2 | v1alpha2 holds only the streams; verbs stay in v1alpha1 and move as their use cases arrive. The pass is done when the v1alpha1 StreamEvents and GetStory are deleted | settled | protos, api, web | KirkDiggler | 2026-10-20 |
| R3 | Every beat carries the act id of the verb or activation that caused it | settled | encounter, session | KirkDiggler | 2026-10-20 |
| R4 | `at`, `seq` and `correlation` all stay; `at` is the world-clock reading in rounds | settled | session, protos | KirkDiggler | 2026-10-20 |
| R5 | v1alpha2 carries two streams: the record stream (every beat, untyped) and the act stream (the projection, folded per act) | settled | session, protos, api, web | KirkDiggler | 2026-10-20 |
| R6 | The act stream's unit is the act, delivered as segments under one act id and folded on replay; the walk is the model (event-projection R4 generalized) | settled | session, protos, web | KirkDiggler | 2026-10-20 |
| R7 | Filtering is a later layer outside both shapes; event-projection R5 stands before release | settled | session, api | KirkDiggler | 2026-10-20 |
| R8 | The record stream is made manageable in the UI by act id and beat name; that is its whole contract | settled | web, protos | KirkDiggler | 2026-10-20 |
| R9 | The trading beats (`transferred`, `interacted`, `bought`, `sold`) need no kind: they appear in the record stream and produce no act | settled | session, protos | KirkDiggler | 2026-10-20 |
| R10 | Answered `fact` stays out; a learned fact needs a per-viewer projection, not a broadcast field | settled | protos, api | KirkDiggler | 2026-09-17 |
| R11 | An unmapped beat rides a raw arm of the act stream — replaced by R5 | superseded | protos | KirkDiggler | 2026-10-20 |
| R12 | Two RPC pairs, not one stream with two arms: StreamRecord and GetRecord, StreamEvents and GetStory. The record seq is the record's own, not per recipient; an act segment names the record seq range it folded | settled | protos, api | KirkDiggler | 2026-10-20 |
| R13 | Record access is an entitlement (operator and developer at release); before release every client receives both streams | settled | api, web | KirkDiggler | 2026-10-20 |
| R14 | The player UI reads acts only; the debug panel reads the record (Law 8) | settled | web | KirkDiggler | 2026-10-20 |
| R15 | A record catch-up read, GetRecord, sits beside GetStory so the panel keeps history on reconnect | settled | protos, api, web | KirkDiggler | 2026-10-20 |
| R16 | The act families of the Shape table, one body each: act, reaction, table answer, state, perception, clock and roster | settled | session, protos, web | KirkDiggler | 2026-10-20 |
| R17 | Shared `Roll` (expression, raw dice, modifiers with sources, total, against, beaten) and `Party` (kind and id: member, prop, faction, world) | settled | session, protos, web | KirkDiggler | 2026-10-20 |
| R18 | v1alpha2 defines every type it needs and imports nothing from v1alpha1; the duplication lasts until v1alpha1's stream is deleted. Not taken: importing the v1alpha1 types, which ties v2 to v1's names | settled | protos | KirkDiggler | 2026-10-20 |
| R19 | An open act (a walk paused for a window, a roll window) delivers as segments; any other beat delivered to that recipient mid-act closes the segment | settled | session | KirkDiggler | 2026-10-20 |
| R20 | Encounter mints the act id once per act as `<verb>:<first seq>`, the equip form | settled | encounter | KirkDiggler | 2026-10-20 |
| R21 | The session SDK keeps a per-step read for the v1alpha1 converter, so v1alpha1 keeps per-step MOVED until it is deleted | settled | session, api | KirkDiggler | 2026-10-20 |
| R22 | GetStory on v1alpha2 fails one act, not the read: the act keeps its seq and arrives marked refused. v1alpha1 keeps its whole-read failure | settled | api, protos | KirkDiggler | 2026-10-20 |
| R23 | Tags stay the record's own query surface and leave the session SDK event | settled | session | KirkDiggler | 2026-10-20 |
| R24 | A driven activation that walks then attacks is several acts: an act is one verb, and a walk's act id is event-projection R4's walk id | settled | encounter, session | KirkDiggler | 2026-10-20 |
| R25 | The record stays trimmed to encounter's retention window as it is today; a durable home for the record waits for its use case, and this pass is the learning step for it | deferred-until-use-case | encounter, session, api | KirkDiggler | 2026-10-20 |
| R26 | The record stream shows every viewer every beat, per-observer beats included, before release under event-projection R5; R13 closes it at release | settled | session, api, web | KirkDiggler | 2026-10-20 |

## Open

- **What the record must reach** (R25): how far back the debug panel and an
  act segment's seq range actually need to read, measured during this pass.
- **Act seq.** One per-recipient seq per segment or per act; confirm against
  story catch-up when building.
- **Family membership.** Where a beat folds when two families fit: a
  concentration break is a state change inside the act that caused it, or its
  own state act.
