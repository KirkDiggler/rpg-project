# The hold-out — how it works

Team-facing explainer, one mechanism at a time. Each section flips from
*designed* to *landed* with the branch head that made it true. The design is
`design.md`; this page is the picture.

The law behind all four: **the run's world is the only state.** Content
declares it, verbs append to it, readers fold over it, the projection presents
it. No reader keeps a copy.

## 1. Sides — who fights whom · *file + Compiled landed (encounter/hold-out d5bcd4e7: factions, dispositions, predicate grammar, fixture) · readers pending · authoring landed (web/hold-out 97c0a298: Factions + Dispositions forms, predicate editor)*

Every member belongs to a faction; players to `party`, unauthored monsters to
`monsters`. A disposition is an edge between two factions. Fight formation
stops counting kinds and asks the graph one question.

```mermaid
flowchart LR
  subgraph file[the file]
    F["factions: goblins (mind: chief)"]
    D["dispositions: goblins × party = hostile until {fact}"]
    P["place: chief, scout → faction goblins"]
  end
  subgraph world[the run's world · graph]
    party((party)) -- "hostile-to" --> gob((goblins))
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

## 2. Knowledge — what a member knows · *designed*

Knowledge is a fact with an audience. A record's `reveals` is applied when the
record changes hands; a fact reveal writes `known:fact:<id>` with the receiver
as its audience. The faction knows what its mind knows.

```mermaid
sequenceDiagram
  participant L as letter (prop, holds wisemans-letter)
  participant H as holder (party)
  participant W as the run's world · journal
  participant C as chief (mind of goblins)
  H->>L: Hold
  L-->>W: known:fact:saved-wiseman · audience {holder}
  H->>C: walks into the hut (presence, on the sweep every verb runs)
  W-->>W: record copies to the chief
  W-->>W: known:fact:saved-wiseman · audience {chief}
  Note over W: knowsFact(chief, saved-wiseman) = true<br/>knowsFact(scout, saved-wiseman) = false
```

A scout who learns it flips nothing: the fold for the faction reads the
mind's facts, and only the mind's.

## 3. The flip — a fact changes a side · *kernel landed (world/pair-settle 640402ed), encounter pending*

A declared reducer (`Raise`) flags the mind when a `known:fact` with the mind as
subject appears; a pair projection (`Settle`, new in the kernel — Kirk's R11:
"the graph should tell the truth") drops the hostile edges between the two
factions in both directions while the flag holds. `HasEdge` is the only reader.
Formation now sees no hostile edge, so the encounter dissolves the fight
between those two factions and the hold-out ending fires.

```mermaid
flowchart TD
  K["known:fact:saved-wiseman · audience {chief}"] --> R["reducer: until {fact: saved-wiseman} holds for goblins"]
  R --> A["projection AdoptStance: goblins × party → neutral (both directions)"]
  A --> F["formation folds: no hostile-to edge"]
  F --> X["dissolveBubble(ByStance) for the goblins–party fight"]
  F --> E["TriggerStance: hold-out ended"]
  X --> B["beats: stance {goblins, party, neutral} · ended"]
```

Nothing is stored as a stance. Save after the flip and load again: the facts
are there, the fold runs, the camp is still neutral.

## 4. Reserve and arrival — something enters the run · *designed (step B)*

A placement with `arrives` is spawned at launch but held in reserve: no cell,
no turn, in no pair, absent from every projection for every member. When its
predicate holds, it is placed and the same verb's sight refresh treats it like
anyone walking into view.

```mermaid
stateDiagram-v2
  [*] --> reserve : launch (content resolved, nothing projected)
  reserve --> placed : predicate holds<br/>{round: 6} at RoundStarted<br/>{down: chief} at noticeDown
  placed --> inContact : sight refresh
  inContact --> fight : formation (hostile edge)
  placed --> [*]
```

The messenger's letter and the reinforcements are the same mechanism with
different predicates. The predicate grammar is the encounter's existing
Trigger set with three new members: `round`, `fact`, `stance`.
