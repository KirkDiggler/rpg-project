# Character Presentation — public identity, loaded once (v1)

## Status: Design — open questions RULED by Kirk 2026-08-25 (see Decisions); plan can follow.

`ideas/characters/` is the umbrella for character ideas — nested on purpose,
to break the flat one-folder-per-idea pattern (Kirk, 2026-08-25). This
document, `characters/presentation/`, is the first resident: the
public/private information split and the roster manifest. Named future
residents: `characters/customization/` (the Synty customization system —
TBD until the asset contract exists; today it is only this design's empty
shelf), bloodied/beat-up visual tiers, portraits, equipment visuals.
Related but already housed elsewhere: `ideas/character-facing/` (how a body
turns), `ideas/unified-entity-state/`.

North star: **every member of a session renders as themselves, because a
character's public face is session data loaded once and referenced at render —
while the sheet stays private.**

Driving acceptance case: two players walk the tomb together and sight each
other. Each renders as the other's actual character — right body, right
customization once customization exists — not a neutral placeholder. When one
of them is beat up, the other can see it. Neither can read the other's sheet.

## Context — what exists today (verified 2026-08-25 against origin/dev, web/api; origin/main, protos)

- `HexEntity`'s player path renders from two inputs: `classRefId` (picks the
  class GLB) and `Appearance` (four shader color strings: skin tone, primary,
  secondary, eye — `dnd5e/api/v1alpha1/character.proto`). The LOCAL player
  gets both from their own sheet. A SIGHTED player gets neither —
  `GetCharacterData` is owner-gated (rpg-api#814 ruling), and the #792 wave's
  comment in `SessionCanvas.tsx` records that this component deliberately
  never fetches a peer's sheet — so sighted players degrade to the
  `MediumHumanoid` neutral `'human'` variant. That is the "default person"
  on screen today.
- **The four color strings are the OLD system.** What customization looks
  like on the new Synty assets is TBD (Kirk, 2026-08-25). This design
  therefore ships a *shelf* for customization, not a payload.
- `Sighting` today: `subject`, `name`, `kind` (protos#245 — category, not
  identity), `seen{position, standing}`, channels. `standing` is prior art:
  public, *dynamic* state riding the sight channel.
- The monster render path derives its GLB by stripping the subject's trailing
  ordinal (`skel-1` → skeleton) — a naming convention doing a ref's job.
- Pre-v1 ruling ([[pre-v1: full data down the log]]): no perception limiting
  until v1.0. This design does not gate who can perceive whom; it defines
  what of a *character record* is public to fellow players.

## The spine: a character's information is public or private

Kirk's ruling, 2026-08-25: *each character has public and private info; if
they are bloodied or beat up, that is public knowledge.* The design follows
that line, and it turns out to also sort by transport:

| Class | Examples | Changes | Transport |
| --- | --- | --- | --- |
| **Public identity** | name, kind, class/race refs (the body), customization (the shelf) | rarely (join-time, editor) | **roster manifest — loaded once, referenced at render** |
| **Public state** | standing (already shipped), bloodied/beat-up tier | every hit | **sight channel (`Sighting.seen`) + events — already per-refresh** |
| **Private sheet** | exact HP, ability scores, inventory, resources, death saves | constantly | `GetCharacterData`, owner-gated — **#814 unchanged** |

The distinction earns its keep twice: it answers *who may see what* (the #814
gate keeps guarding exactly the third row), and it answers *how it should
travel* (stable data loads once; dynamic data rides the channel that already
refreshes; private data stays behind the gate).

## Wire sketch (session v1alpha1)

One new RPC, fetched at session mount and re-fetched on a `joined` event
(pull — the same refetch pattern every other event already uses; ruled, see
Decisions). The message is named `PublicMemberInfo` rather than anything
presentation-flavored so the wire itself states the ruling: this is the
public half of a character, and nothing private ever belongs in it:

```proto
rpc GetRoster(GetRosterRequest) returns (GetRosterResponse);

message GetRosterResponse {
  repeated PublicMemberInfo members = 1;
}

message PublicMemberInfo {
  string id = 1;              // the session member/subject id
  MemberKind kind = 2;
  string name = 3;
  // The body — players. Same refs the local player's own render path
  // already maps to GLBs; empty for monsters.
  string class_ref = 4;
  string race_ref = 5;
  // The body — monsters: the authored archetype ref, so the client stops
  // deriving GLBs by string-stripping subject ordinals.
  string monster_ref = 6;
  // THE SHELF. Empty message today, on purpose: what customization IS on
  // the new Synty assets is TBD. When that contract exists, its fields
  // land here and only here — no other message grows.
  Customization customization = 7;
}

message Customization {
  // Deliberately empty in v1. The old system's four shader color strings
  // (character.proto Appearance) are NOT copied forward; if any survive
  // into the Synty era they re-enter here under the new contract.
}
```

Bloodied/beat-up is **not** in the manifest — it is public *state*, so it
belongs beside `standing` in `Seen` when its unit of work comes. Recorded
here as the second resident of this folder, not built by this design.

## Sequencing (how-we-build: from the panel back, protos first)

1. **protos**: `GetRoster` + `MemberPresentation` + empty `Customization`.
2. **rpg-api**: assemble the manifest from the character records it already
   owns — the launch-restore loop already walks every member's sheet at
   session start; this is a read-only projection of the same rows, plus the
   spawned monsters' refs. Server-side assembly is what lets #814 stand:
   the server may read any sheet; clients only ever see the projection.
3. **web**: fetch once into a map keyed by member id; sighted PLAYER
   entities read `classRefId` from the map — literally the props the local
   player path already takes, so the render layer barely changes. Monster
   entities read `monster_ref` and retire the ordinal-stripping.

No toolkit work: presentation is not rules data, and the session module
never needs to know what anyone looks like.

## Non-goals (v1)

- No customization payload content (TBD with the Synty asset contract).
- No perception limiting (pre-v1 ruling stands).
- No bloodied implementation (designed-for, filed separately when picked).
- No portraits, no equipment visuals — future residents of this folder.

## Decisions (Kirk, 2026-08-25)

1. **`class_ref`/`race_ref` for now.** The client keeps its existing
   ref→GLB mapping; what customization will look like is unclear, so no
   server-authoritative model ref until it is.
2. **Knowing hurt is good — public damage state is wanted.** And it has a
   visual consumer beyond a HUD tint: the team is looking into models
   taking dings on the armor through a runtime mesh, which would read the
   same public hurt state. Granularity firms up when that unit is picked;
   the ruling here is that hurt-visibility is public and designed-for.
3. **Pull on join.** The `joined` event triggers a roster refetch; it does
   not carry the new member's info inline.
4. **Naming: `PublicMemberInfo`**, not `MemberPresentation` — explicit
   about the split rather than about rendering.

— asset-pipeline agent, on behalf of KirkDiggler
