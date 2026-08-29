# Character Presentation — implementation plan (v1)

## Status: Plan — follows the ruled design (design.md, PR #264). Three module-isolated units, proto merges first.

Wire-anchored issues live under journey rpg-project#253 (this is the same
"play the dungeon together" wire the kind wave shipped on). One in-flight PR
per module; each unit lands whole with its own evidence.

## Unit 1 — rpg-api-protos: the wire

`dnd5e/api/session/v1alpha1`: `GetRoster` rpc on SessionService;
`GetRosterRequest{session}`; `GetRosterResponse{repeated PublicMemberInfo}`;
`PublicMemberInfo{id, kind, name, class_ref, race_ref, monster_ref,
customization}`; `message Customization {}` — the deliberately empty shelf,
with a doc comment saying exactly why it is empty and what may ever land in
it (the Synty customization contract, `ideas/characters/customization/`).

Evidence: buf lint / format / breaking + generation compiling — no
hand-written proto tests (standing rule).

## Unit 2 — rpg-api: assemble the roster (no toolkit work)

The session SDK deliberately has no roster enumeration and does not grow one
— presentation is not rules data. rpg-api owns the facts already:

- **Launch writes a roster record.** `start_encounter_session_stack.go`
  already walks every member (ids double as character ids) and every
  `dungeon.Monsters` spawn (id + ref) — after the spawns succeed, persist
  one roster row keyed by encounter id: `{members: [character ids],
  monsters: [{id, ref}]}`. A new small redis repo beside the existing ones.
- **`GetRoster` handler** loads the roster row; for each player member,
  loads the character record and projects the PUBLIC subset only — name,
  class ref, race ref, kind PLAYER; each monster row projects id, ref, kind
  MONSTER, display name. Customization: empty message, always set.
- Authorization: same posture as the service's other read verbs (the
  member-verb authorization sweep is rpg-api#803's scope, not this unit's).

Evidence: handler convert test (player and monster rows, public-subset-only
pinned), orchestrator test that launch persists the roster row, acceptance
test that GetRoster returns every member of a started tomb with refs.

## Unit 3 — rpg-dnd5e-web: reference at render

- `useSessionRoster` hook: fetch once at session mount; refetch on a
  `joined` event (pull — ruled). Map keyed by member id.
- `SessionEncounterView`: sighted PLAYER entities get `classRefId` from the
  map — the same prop the local player path already takes, so `HexEntity`
  needs no new machinery for the body. Missing roster entry degrades to
  today's neutral placeholder (never blocks rendering).
- Monster path: use the roster's `monster_ref` and retire the
  ordinal-stripping derivation in `sightingEntities.ts` (keep it only as
  the missing-entry fallback).

Evidence: hook test (fetch-once + joined refetch), canvas test pinning a
sighted player renders down the class-model path with the roster's ref, and
a live two-browser walk on the local stack before merge.

## Sequencing

protos PR → merge → npm/Go tags → api PR (pin + feature) → web PR (pin +
feature) → stack refresh → Kirk's two-browser walk: the other player is
THEIR character. Bloodied/beat-up state is the next resident of this folder
when picked (sight-channel unit; runtime-mesh armor dings consume it).

— asset-pipeline agent, on behalf of KirkDiggler
