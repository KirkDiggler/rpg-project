# The hold-out — how it works

Team-facing explainer, one mechanism at a time. Each section flips from
*designed* to *landed* with the branch head that made it true. The design is
`design.md`; this page is the picture.

The law behind all four: **the run's world is the only state.** Content
declares it, verbs append to it, readers fold over it, the projection presents
it. No reader keeps a copy.

## 1. Sides — who fights whom · *landed (encounter/hold-out 5263f098: one graph, formation asks it; 177 leaves fail when the table is forced never-hostile) · authoring landed (web/hold-out 97c0a298: Factions + Dispositions forms, predicate editor)*

Every member belongs to a faction; players to `party`, unauthored monsters to
`monsters`. A disposition is an edge between two factions. Fight formation
stops counting kinds and asks the graph one question.

```mermaid
flowchart LR
  subgraph file[the file]
    F["factions: raiders (mind: chief)"]
    D["dispositions: raiders × party = hostile until {fact}"]
    P["place: chief, scout → faction raiders"]
  end
  subgraph world[the run's world · graph]
    party((party)) -- "hostile-to" --> gob((raiders))
    gob -- "hostile-to" --> party
    chief[chief] -- "belongs-to" --> gob
    scout[scout] -- "belongs-to" --> gob
    pc[fighter] -- "belongs-to" --> party
  end
  F --> gob
  D --> party
  P --> chief
  form["formation / fight-decided / bubble-has-player<br/>«is there a hostile-to edge between a's faction and b's?»"] -.asks.-> world
```

Today's dungeons declare nothing and get the default table: `party` and
`monsters` mutually hostile, every faction allied with itself. Same answers,
read from data instead of computed from kind.

*API landed (api/hold-out e74995b):* each monster spawns with its faction, the
roster says the side, and the client hears STANCE_CHANGED → FIGHT_ENDED (by
stance) → ENDED hold-out, proved end to end through the production launch
and the gRPC handler on the shipped camp.

*Web landed (web/hold-out 8aeabbf5):* members on the map coloured by declared
faction with a sides legend; the stance beat narrated ("The raiders and the
party are no longer hostile."); a fight ended by stance says the sides stood
down; the hold-out's `convince` renders as a dropdown of declared factions;
the compiler's own refusals render inline beside the client's.

*Session landed (session/hold-out 25b2891b):* Spawn carries the
faction, the roster says whose side everyone is on, and after the flip every
player's stream reads `stance_changed` → `fight_ended (by stance)` → `ended:
hold-out`, walked through the real verbs on the fixture.

*Resolution landed too (resolution/hold-out 7a32b34d):* Sneak Attack and Pack
Tactics ask the reloaded run, `castRelations` and its two cast-side entities
are deleted, and a cast with no run answers "unknown" for everyone rather
than letting a default table step back in.

## 2. Knowledge — what a member knows · *landed (encounter/hold-out 5263f098: known:fact with the learner as subject and audience; presence teaches the mind on the sweep and at the end of Hold/Loot)*

Knowledge is a fact with an audience. A record's `reveals` is applied when the
record changes hands; a fact reveal writes `known:fact:<id>` with the receiver
as its audience. The faction knows what its mind knows.

```mermaid
sequenceDiagram
  participant L as letter (prop, holds wisemans-letter)
  participant H as holder (party)
  participant W as the run's world · journal
  participant C as chief (mind of raiders)
  H->>L: Hold
  L-->>W: known:fact:saved-wiseman · audience {holder}
  H->>C: walks into the hut (presence, on the sweep every verb runs)
  W-->>W: record copies to the chief
  W-->>W: known:fact:saved-wiseman · audience {chief}
  Note over W: knowsFact(chief, saved-wiseman) = true<br/>knowsFact(scout, saved-wiseman) = false
```

A scout who learns it flips nothing: the fold for the faction reads the
mind's facts, and only the mind's.

## 3. The flip — a fact changes a side · *landed (world/pair-settle e9da06e Settle; encounter/hold-out 5263f098 declares Raise + Settle per until, dissolves ByStance; 12 leaves fail without the Raise)*

A declared reducer (`Raise`) flags the mind when a `known:fact` with the mind as
subject appears; a pair projection (`Settle`, new in the kernel — Kirk's R11:
"the graph should tell the truth") drops the hostile edges between the two
factions in both directions while the flag holds. `HasEdge` is the only reader.
Formation now sees no hostile edge, so the encounter dissolves the fight
between those two factions and the hold-out ending fires.

```mermaid
flowchart TD
  K["known:fact:saved-wiseman · audience {chief}"] --> R["reducer: until {fact: saved-wiseman} holds for raiders"]
  R --> A["projection AdoptStance: raiders × party → neutral (both directions)"]
  A --> F["formation folds: no hostile-to edge"]
  F --> X["dissolveBubble(ByStance) for the raiders–party fight"]
  F --> E["TriggerStance: hold-out ended"]
  X --> B["beats: stance {raiders, party, neutral} · ended"]
```

Nothing is stored as a stance. Save after the flip and load again: the facts
are there, the fold runs, the camp is still neutral.

*Walk 1 fixes (encounter/hold-out 08469fa2):* a driven monster turn stops the
moment the fight it belongs to is gone — the chief who steps into the yard,
learns the fact and turns the camp no longer swings afterwards; and Exit
removes the leaver from the roster before it drives the next monster (a
pre-existing ordering bug on main, found by the walk).

## 4. Reserve and arrival — something enters the run · *landed (encounter/hold-out baea481d): projections byte-identical with and without the reserve; the letter at round 6; three zombies on the chief's fall; `endings[]` authorable and `convince` proven to be one of them*

A placement with `arrives` is spawned at launch but held in reserve: no cell,
no turn, in no pair, in nobody's story, absent from every projection for every
member. When its predicate holds it is placed where it was drawn, or the
nearest free cell of that room, everyone hears `arrived`, and the arrival's own
sight refresh treats it like anyone walking into view.

```mermaid
stateDiagram-v2
  [*] --> reserve : launch (content resolved, nothing projected)
  reserve --> placed : predicate holds<br/>{round: 6} at RoundStarted<br/>{down: chief} at noticeDown
  placed --> inContact : sight refresh
  inContact --> fight : formation (hostile edge)
  placed --> [*]
```

*Session and api landed (session/hold-out 055264d7, api/hold-out a2d2e40):* a
reserved monster is spawned into the wings, `SpawnOutput.Reserved` says so, no
roster or atlas shows it; the `arrived` beat crosses the wire as ARRIVED with
the placement kind and cell; an ending written in the file is declared beside
the scenario's at launch. End to end: the letter appears at round 6, three
zombies arrive on the chief's fall, and the authored ending ends the run.

The messenger's letter and the reinforcements are the same mechanism with
different predicates. The predicate grammar is the encounter's existing
Trigger set with three new members: `round`, `fact`, `stance`.
