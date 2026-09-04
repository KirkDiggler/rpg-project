---
status: RULED 2026-09-04 (R1–R4) — the first tool cut under the north star; plan.md beside this file
journey: rpg-project#326 (Living World); issue rpg-project#372 (framing + addendum)
predecessor: recover-the-artifact/design.md (slice 2; holdings, Loot, Hold, `knows`)
north star: "we are here to build tools that can be used to tell stories" — rpg-project#326
---

# The intel record — authored knowledge you place in a monster

The tool: an author configures a piece of intel (what it reveals) and places
it in a monster, through a form. The test case: the heirloom tomb, re-authored
through that form, walked once by path 2. No new scenario; the one we have
proves it.

## 0. Vocabulary

- **intel record** — an authored thing in the dungeon file: an id and what it
  reveals. Today: one door. Later, by its own use cases: a region, a location,
  a parchment body, a reading check.
- **holds** — a monster placement's list of intel record ids. The monster
  carries them from spawn; Loot transfers them (a prop moves, intel copies).
- **the panel** — the designer's intel form: create a record, choose what it
  reveals, assign it to a holder.

## 1. Ruled (Kirk, 2026-09-04, in session)

- **R1 — `knows` goes.** "if we don't have a use case for it then it goes.
  when a use case does arrive for it it will define the shape without any
  baggage." One spelling of knowledge: the record. Deleted from dungeonspec
  and refused by name; the only fixture spelling it is re-authored.
- **R2 — Intel is assigned through a form.** "the form should assign the
  Intel to something though and that could use a form." The author never
  types a property on a monster.
- **R3 — Intel is a general builder capability**, like a concealed door:
  declared in the dungeon file, placed on a monster, read by the engine as
  holdings. Never scenario machinery; the scenario form binds what its quest
  needs and nothing else.
- **R4 — Step one proves the tool on the scenario we have.** No dropdown, no
  description, no second scenario in this cut (those are rpg-project#372's
  and #371's).

## 2. The file — dungeonspec v2

```yaml
intel:
  - id: vault-map
    reveals: { door: vault }

place:
  - { id: captain, ref: "dnd5e:monsters:skeleton-captain", at: [23,5],
      targeting: closest, holds: [vault-map] }
```

- `intel[].id` — unique; refused on collision naming both lines.
- `intel[].reveals` — exactly one target in this cut: `door: <door id>`, refused
  when the door does not exist. A declared-but-unconcealed door is legal and
  inert. The map of targets grows one key per use case (region, …).
- `place[].holds` — intel ids; monsters only (refused on a prop, for
  `blocks_movement`'s reason); refused when the id does not exist; the same
  record may be held by several monsters (intel copies).
- `knows` — refused by name, pointing at `intel`/`holds`.
- `Compiled` exposes `Intel []IntelRecord{ID, Reveals}` and `Holds` on each
  monster placement.

## 3. The engine — encounter + session

- `MemberInput.Holds` and `JoinInput.Holds` ([]IntelID) replace `.Knows`;
  seeding writes `holds:intel:<record id>` on the member — the RECORD id, not
  the door: what a record reveals is read at transfer time from the field's
  intel table, so the same fact kind serves every future `reveals` target.
- Loot: unchanged in shape. For each intel holding of the body, apply its
  `reveals` to the looter — a door → `learnDoor(looter, door, "loot")` and the
  looter's own DOOR_REVEALED beat, exactly as today; copies, never moves.
- Secrecy (design slice 2, P3): holdings are never projected; the probe law
  holds; a monster with an empty `holds` and one with intel are
  byte-identical to every observer until a loot.
- Persistence: the intel table is field structure (construction-truth); the
  holdings are journal facts; Load replays, never re-seeds.
- `session.SpawnInput.Holds` replaces `.Knows` and forwards to
  `JoinInput.Holds`. Hosts forward COMPILED intel ids.
- Trust boundary at load (slice 2's gap 10) extends: a `holds:intel:` fact
  naming a record the field does not declare is refused.

## 4. The wire — no change

`PutDungeon` ships verbatim YAML; the atlas never carries intel; the reveal
reaches the looter as the DOOR_REVEALED beat that already exists. rpg-api
translates nothing new: it forwards `Holds` where it forwarded `Knows`.

## 5. The designer — the panel (web)

- **Intel panel** in the inspector, beside Regions/Props: **New intel** →
  a record with a suggested id; **Reveals** = a dropdown of the dungeon's
  doors (the only kind in this cut; the dropdown is the entity_ref picker
  filtered by kind, as everywhere); **Held by** = a multi-pick of the
  dungeon's monsters by id.
- Selecting a monster shows what it holds, read-only, with a link back to
  the record — the monster is not where you edit intel.
- YAML emit/parse round-trip byte-exact for `intel` and `holds`; `knows` is
  refused by the parser with the compiler's own sentence.
- The scenario form is untouched: "who knows the way in" never becomes a
  scenario field (R3).

## 6. Ownership

| noun | holder | why |
|---|---|---|
| intel record | dungeonspec (author) | declared structure, like a door |
| `holds` on a placement | dungeonspec (author) → encounter holdings at spawn | the monster carries it from birth |
| what a record reveals | encounter, read at transfer | one fact kind, many targets later |
| the panel | web | the form that assigns intel to something (R2) |
| scenario form | untouched | binds the quest's nouns only (R3) |

Charter checks: dungeonspec gains declarations and refusals it decides alone
(holds); encounter's holdings gain a record indirection and no new writer
(holds); session forwards one renamed field (holds); rpg-api forwards
(holds); web gains a form (holds — the designer is the product).
Singularity: knowledge is spelled once (the record); who-has-what is
answered once (holdings).

## 7. Acceptance

| item | proof |
|---|---|
| a record held by the captain reveals its door to the looter alone | scene: two members; loot; DOOR_REVEALED to one; the other's atlas bytes unchanged |
| a monster holding nothing and one holding intel are byte-identical to every observer until a loot | pinned atlas + story bytes |
| two monsters holding the same record: looting either reveals; looting both reveals once | intel copies; no double narration |
| `knows:` is refused by name in the file and in the panel | dungeonspec test; parser test |
| a `holds:intel:` fact naming an undeclared record is refused at load | trust-boundary scene |
| the heirloom tomb re-authored through the panel round-trips byte-exact and walks path 2 | fixture pinned in three repos; Kirk's walk |

## 8. Shelves — named, empty

- **Reveals a region / a location** — the next `reveals` key, when a scenario
  wants it; the fact kind already fits.
- **The parchment body and its reading check** — the record grows a body and
  a check; Loot yields the record as an item; reading applies `reveals`.
- **Hands** — a held record may cost a hand (slice 2 §9).
- **Intel on props and NPCs** — a chest or a vendor holding intel; `holds` is
  monsters-only until a use case says otherwise.
- **The scenario dropdown, description, kill-the-captain** — rpg-project#372 /
  #371, the next cut.
