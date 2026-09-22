# Dispositions turn both ways: `until` on a neutral pair, and aggression as a law

**Status:** RULED 2026-09-22 (Kirk), BUILDING. Follows the hold-out design
(`../hold-out/design.md`, R2/R11) and the reference map
`ideas/dungeon-authoring/world-builder/minds-factions-intel.md`.

## The gap, as found

Today a pair turns exactly one way: hostile → neutral, on `until: { fact }`.
Nothing writes a hostile edge back. If the party attacks a neutral creature:
the swing lands (no verb reads opposition), no fight forms (fights form only
between opposed pairs), the victim alone swings back through its
`attacked within 3 → attack: attacker` row, and its faction reads
`enemy: none` and holds. A betrayed truce is one aggrieved goblin with no
initiative and no friends. That is fail-silent, not a design.

## Rulings

**R1 — `until` runs both ways.** A declared stance lasts until its predicate
holds, then the pair moves to the OTHER of hostile and neutral:
`{ stance: hostile, until: X }` → neutral (as today);
`{ stance: neutral, until: X }` → hostile (new). `until` on `allied` is
refused: there is nothing for an allied pair to become. One disposition per
pair, one `until`, so a pair turns at most once by authoring; no oscillation.

**R2 — `until` accepts every predicate form.** "Neutral until it learns
something we did" needs more than `{ fact }`: the things we DO are deeds,
which are per-creature testimony, never facts. So the honest spellings are
`{ down: scout }` ("until we kill its friend"), `{ round: N }` ("the guards
turn at midnight"), `{ stance: ... }` ("when the raiders turn, so do their
dogs"), and `{ fact }` as before. The "in this version a disposition turns
only on a fact" refusal (hold-out R11, `untilNotBuilt`) retires; the use
case has arrived. Grain: a `fact` on an `until` stays on the faction MIND's
knowledge (audience grain); `down`, `round` and `stance` are the world's
truth. A witnessed deed becoming a fact ("until it sees us loot the
shrine") is the word-spreads shelf and stays there.

**R3 — Aggression is a law, not an authored trigger.** When a member of one
faction attacks a member of a faction the pair is NEUTRAL with, the pair
turns HOSTILE: faction-wide, public, immediately, the same `stance` beat and
the same fold the hold-out flip uses. Fights form on the next sight refresh;
`enemy:` bands light up. An author never writes "if attacked, become
hostile" on a camp. Symmetric: two neutral monster factions that come to
blows turn on each other the same way. An allied pair is NOT turned by an
attack (friendly fire is not betrayal in this cut; it can be its own use
case).

**R4 — Monster refs only. NPC refs are not targetable.** R3 applies to
members whose ref is `dnd5e:monsters:*`. A member whose ref is
`dnd5e:npcs:*` cannot be the target of an attack at all: the verb refuses
with a sentence, fail closed. A merchant that should be able to turn is
authored as `dnd5e:monsters:merchant` (or similar) with a neutral
disposition; the monster configuration is what gives it a table to fight
with. NPC roles remain fadedpez's lane; this ruling only says the engine's
attack verb will not point at them.

## Two corrections from the build (2026-09-22, accepted)

- **"NPC ref" is `KindWorld` at this seam.** The encounter module carries no
  member ref and cannot import the rulebook's refs (C1). What a
  `dnd5e:npcs:*` becomes when placed is a member of Kind `world` (session's
  PlaceNPC). So R4 is implemented as "a member of Kind world cannot be the
  target of an attack", refused at the verb before anything is appended, and
  R3 then applies to any landed attack between two faction members. A
  literal ref check would need a member-ref wave through session and api;
  not brought.
- **A stance turn must FORM the fight; a later sight refresh will not.**
  Fight formation reads only first contact, by law: a subject already
  watched is not news. A camp the party was already looking at, turned
  hostile by aggression or by `until`, would therefore never go to
  initiative — a second fail-silent the design's opening paragraph did not
  see. So the stance site synthesizes the first contact that just became
  true (each member's currently-sighted members on the newly opposed side)
  and feeds it through the one classify/formation path: precedence,
  surprise, straggler-join and the formed beat stay one set of rules.
  Strangers becoming enemies is first contact with an enemy.
- Graph shape: a pair's PUBLIC stance is now a journal fact (two settled
  kinds, hostile declared after neutral so aggression re-turns a pair its
  own `until` made neutral — the betrayed truce, pinned); fact-untils stay
  on the mind's grain; one mind learning is enough in either direction.

## Free roam: the item is the trigger, not a swing (Kirk, 2026-09-22)

A party in free roam has no attack verb (Attack and Cast are fight-only at
the session seam), and the ruling is that it does not need one: in free
roam the thing that turns a camp is what the party DOES, and the first use
case is taking their thing. That is already spellable with no new key:

```yaml
intel:        [{ id: the-toy, reveals: { fact: stole-toy } }]
propBindings: { toy: { holdable: true, holds: [the-toy] } }
dispositions: [{ between: [goblins, party], stance: neutral, until: { fact: stole-toy } }]
```

Hold teaches the holder the fact; standing in the goblin mind's region
(a single room is one region) hands it to the mind by presence transfer;
R1 turns the pair hostile and the stance site forms the fight. The swing
path (R3) is for a fight already running with a neutral faction in the
room. No `{ held }` predicate and no free-roam Attack verb are brought.

## What this makes possible
- A neutral camp the party can provoke, and that provokes back as a camp.
- "The guards are civil until midnight."
- "The wolves are calm until you kill the alpha."
- Cascades through `until: { stance }`: the raiders' dogs turn when the
  raiders do.

## Out of scope, named
- Allied ↔ anything. Allied is still a static, authorable, inert stance.
- Directed dispositions (A hostile to B but not B to A).
- Witnessed deeds as facts (word spreads).
- Per-character stance.
- Anything about NPC behaviour beyond "not a target".

## Slice
rpg-toolkit `rulebooks/dnd5e/encounter` (+ dungeonspec, both dialects): the
`until` widening and both-direction settle; the aggression law in the attack
path; the NPC-ref refusal; liveness refusals updated (`stanceReachable` must
now say a neutral pair with an `until` can reach hostile, and a pair
declared hostile can be reached by an attack from neutral). Session and api
take pins; no proto change expected (the `stance` beat already exists).
