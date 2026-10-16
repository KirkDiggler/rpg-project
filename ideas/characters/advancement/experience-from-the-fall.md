# Experience from the fall

**Status:** RULED 2026-09-22 (Kirk, monster-ai session). The first in-toolkit
source of experience, the one [level-up-system.md](level-up-system.md) §4.3
said would bring `AddExperience` with it. Slice one: monster kills. Slice two
(a dungeon's reward at an ending) has its shape reserved here and is not
scheduled.

## What Kirk said

> I would like to get the ability to gain XP to trigger a level up. We have
> the advancement method but we need a way to get XP for killing monsters.
> Eventually we will also want a grant if so configured per dungeon in the
> success criteria being hit.

And, on the defaults below: "I like the defaults, let's proceed."

## What exists today (why this is small)

- The character stores a cumulative total (`character.Data.Experience`),
  derives the entitled level from the 2014 threshold table
  (`character/experience.go`), and `Advance` refuses an unearned level and
  refuses in combat. There is no mutator, by R4.8's own design: the first
  source brings it.
- A monster carries no experience value and no challenge rating anywhere,
  not on `monster.Data`, not in content. CR exists only as English in
  constructor doc comments.
- The fall of a monster is already noticed by the engine, once, at the
  standing choke point, whatever caused it: the `down` outcome beat
  (`encounter/standing.go appendDownBeat`), member id only, no striker, by
  ruling (rpg-toolkit#959). Session maps it to `EventDowned`; a fall is also
  a trigger (`TriggerMemberDown`) and a predicate (`{ down: <placement> }`).
- Success criteria already exist as endings: content-string keys from
  scenarios, the boss flag, or an authored `endings:` block, all closed
  through `Encounter.closeWith`. No ending carries a reward.
- Sheets are written per verb inside the act (`session saveCharacterRecord`
  over `DirtyCharacters`). `Manager.End` writes no character.
- The wire reads `Character.experience_points`, `entitled_level`,
  `next_level_threshold`. No RPC writes experience and no stream carries an
  experience event.

## Rulings

**R1. The monster's worth is authored on the monster.** `Experience` on
`monster.Data` / `monster.Config`, the 2014 SRD number for the stat block's
challenge rating, set on every shipping constructor. Zero means the monster
is worth nothing, and that is the truth for a monster nobody valued. No
challenge-rating field and no CR table: nothing else needs CR yet, and the
day something does (an encounter budget, the danger journey) it brings CR
with its own use case. A builder variant on a base monster inherits the
base's worth; an authored override is a later slice.

**R2. Granted on the fall, divided among the party.** When a `KindMonster`
member falls, every `KindPlayer` on the roster at that moment receives the
monster's worth divided by their count, floored (`ExperienceShare`). This is
the 2014 rule's equal division (DMG "Experience Points", PHB p.15) with the
timing moved from the end of the encounter to the fall. The move costs
nothing the letter cared about and buys two things: it rides the existing
per-verb sheet write, and an abandoned or lost run still credits its kills.
Every player on the roster receives a share whatever their life state: RAW
pays everyone who took part, and a dying character took part.

**R3. No attribution.** The fall is anonymous by an earlier ruling and the
equal division does not need a killer. A monster felled by another monster
still pays the party. That is the one honest gap, and it becomes a slice
the day two hostile factions share a room (rpg-toolkit#1870's world), which
is also the day the `down` beat may need to say who.

**R4. dnd5e owns the rule, session applies it.** `character.AddExperience`
is the first and only mutator: the total only grows, zero or negative is
refused. `ExperienceShare` holds the division. The session SDK observes a
monster's fall in the act it happened, applies the share to each sheet, and
saves them in the same commit. Entitlement stays derived (R4.9, R4.10):
"level up available" is still the gap on the next character read, and
`Advance` still refuses in combat, so a character crossing a threshold
mid-run levels between runs through the verbs that exist.

**R5. The wire gains one beat and no write.** `EVENT_KIND_EXPERIENCE_GAINED`
on the session stream, whole-party, one per fallen monster: the member that
fell and, per character, the amount and the new total. Experience remains
read-only over the wire (R4.12); no RPC writes it, the API maps and
projects. Entitlement is not on the beat because it is derived.

**R6. Slice two, shape reserved.** A dungeon pays an authored reward when an
ending fires: a `rewards:` block keyed by ending key, so scenario endings,
the boss flag and authored endings are all covered by one grammar:

```yaml
rewards:
  hold-out: { experience: 300 }
```

Paid at close, through the same `AddExperience`, to everyone ever admitted
(`EverMembers`, players only). Authored per participant, not divided,
because the author cannot know the party's size. Not built until a dungeon
asks for it.

## Costs stated

- A four-player party levels four times slower per kill than a solo player.
  That is RAW's pace; the "equal, not divided" alternative was offered and
  declined.
- No striker on the fall means no per-character bonus is possible without a
  later change to the beat (R3).
- A monster with `Experience` unset pays nothing and says nothing. That is
  the rule, not a bug; the value table in the dnd5e PR names every shipping
  monster.

## Ownership

| noun | owner | notes |
|---|---|---|
| monster worth | dnd5e `monster` (content) | authored on the definition |
| the total, the mutator, the share | dnd5e `character` | `AddExperience`, `ExperienceShare` |
| noticing the fall | encounter | unchanged, `down` beat |
| applying and saving the grant | session SDK | in the act, before commit |
| the beat on the wire | rpg-api-protos | `ExperienceGained` |
| mapping | rpg-api | transport only |
| showing it | rpg-dnd5e-web | web lane, issue filed |

## Done when

- A goblin falls on the local stack; each player on the roster gains its
  share; the character read shows the new total and, when a threshold is
  crossed, the entitled level flips; one `experience_gained` beat reaches the
  stream with the member and the grants.
- A character seeded just under a threshold crosses it on one kill and then
  levels between runs through the existing `LevelUp` verb.
- `AddExperience` refuses zero and negatives; `Advance` still refuses in
  combat and still refuses an unearned level.

## Later slices

- The dungeon reward at an ending (R6).
- Attribution on the fall when two hostile factions can fight (R3).
- An authored worth override on a builder variant.
- Equal instead of divided, only if play shows the pace is wrong.
