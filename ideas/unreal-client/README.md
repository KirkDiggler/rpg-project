# Idea: the Unreal client

- **Law:** [design.md](design.md). R1 is settled; R2, R3 and R4 are open.
- **Spike:** [spike/README.md](spike/README.md). What ran, what it showed, and how to run it again.
- **Run record:** [knowledge/runs/2026-11-24-unreal-client-design.md](../../knowledge/runs/2026-11-24-unreal-client-design.md).
- **Paper trail:** idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154); design PR in the first comment there; contract gap [rpg-api-protos#453](https://github.com/KirkDiggler/rpg-api-protos/issues/453).

## What this is

A second client of the running engine, built in Unreal, for a first- or
third-person way to play what the engine's stories already do. It is shaped
like the web client: generated code from the same protos, the same auth, the
same verbs in and record out. Nothing in the engine changes for it.

## Origin

Session of 2026-11-23. The web client is one way to interact with the
engine's story-building. The Synty assets have already stood in Unreal as a
first- and third-person experience. The question was whether to make the
toolkit a C-bound library or to re-implement it in C++. The answer was
neither: a second client over the existing contract (R1). The operator
approved a spike the same day, and it ran on Linux.

## Earlier cuts

- **Bind or port the engine.** The first framing of the question: a Go library
  inside the game's process, or a native C++ engine. It gave way when the
  operator moved to Unreal as another client, like the web. No second engine.
- **Portable combat core.** A separate product for game builders, with the
  composition core re-implemented in C++ for a deck-builder game and a UE
  host. Not this game and not this engine:
  [ideas/portable-combat-core](../portable-combat-core/design.md). It stays a
  parallel line.

## What the spike showed

A C++ client, linked against the protobuf and gRPC libraries that ship for
Unreal 5.6, read the record stream from the Go engine on Linux. It received
the retained beats in eight milliseconds, then live rounds as the engine
played its turns, through the production auth boundary. A caller with no seat
read the whole record in development mode.

It also found what has to be decided before the first Unreal leg builds: a
C++-only defect in the contract (R2) and a presence bug in the Blueprint
wrapper it surveyed (R3). Numbers, evidence and the recipe are in
[spike/README.md](spike/README.md).

## Status

- R1 is settled. R2, R3 and R4 are the operator's; the checklist is on the
  design PR.
- Nothing is built, and no repository changes until R2 to R4 rule. Then this
  folder gains its walkthrough, and the story skill cuts the first Unreal leg.
