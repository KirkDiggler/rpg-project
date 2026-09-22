# The social offer comes from the NPC

**Status:** RULED 2026-09-22 (Kirk, monster-ai session). Slice one of dialogue.
Builds the R5 direction of
[v4-gameplay-shape.md](../../dungeon-authoring/world-builder/v4-gameplay-shape.md)
in its smallest cut. Not scheduled beyond this slice.

## What Kirk said

> I was thinking of just having the intimidation and persuasion entries we
> can make come from the npc. We can start with just that. I think that will
> allow us to get something simple and working in.

## What exists today (why this is small)

- Intimidate and Persuade are session verbs (`session/intimidate.go`,
  `session/persuade.go`). Afford offers both on the turn clock and, free, on
  the world clock (`session/afford.go socialRowsOnTheWorldClock`), to every
  witness of the actor.
- The DC is the creature's: the binding's `intimidate: [{ability, dc}]` /
  `persuade:` entries (`encounter.CheckApproach`) when authored, otherwise ONE
  derived approach, the skill against the stat block's passive Insight.
- The creature answers from its authored table (`on: intimidated |
  intimidate_failed | persuaded | persuade_failed`, rows `say/fact/flee`).

So the only thing NOT coming from the NPC is the offer itself, and the
derived DC that lets a creature nobody configured be threatened anyway.

## Rulings

**R1. No entry, no offer.** A creature is offered a social verb only when its
binding authors entries for that verb. A creature with no `intimidate:`
entries cannot be intimidated; the same for `persuade:`. The derived
passive-Insight approach is RETIRED. Zero values tell the truth: absence is
"this creature does not do that", not "use the default".

**R2. The offer is per target.** Afford's Intimidate row lists as candidates
only the witnesses that carry `intimidate:` entries; Persuade likewise. A row
whose candidate set is empty is not available and says why with a shortfall
of its own (`ShortfallNoSocialEntry`, text on the order of "nobody here can
be intimidated"), distinct from `ShortfallNoTargetInReach`. Both clocks.

**R3. The verb refuses what the offer withheld.** `Manager.Intimidate` /
`Manager.Persuade` at a target with no entries fails closed with a named
error (`ErrNoSocialEntry`), even when a stale client sends it. The offer and
the door agree.

**R4. Left exactly where it is, this slice.** Clocks and pricing (turn clock
costs the action, world clock free), stance gating (a hostile creature in
combat is still intimidable when authored), the answer table, the `intimidated`
/ `persuaded` / `answered` beats, and the wire. Nothing in protos changes.
"Out of combat only, non-hostile only" from R5 is NOT adopted here; it stays
a direction.

## What this costs, stated

- Every existing room whose monsters have no authored entries loses Intimidate
  and Persuade on them. The goblin at "DC 9 derived" is gone unless the author
  writes `intimidate: [{ability: intimidation, dc: 9}]`. Pre-release: no
  backcompat baggage; the reference rooms that ship with entries keep working,
  the ones that never authored them were never designed to be talked to.
- The World Builder becomes the only place a creature gains a social verb.
  That is the point.

## Ownership

- encounter owns the entries (already) and, if the derived approach lives
  there, its removal.
- session owns the offer (Afford) and the door (the verb). One law in one
  place: a single `socialEntriesOf(enc, target, verb)` read that both use.
- rpg-api carries nothing new; it repins.
- web: the dock button must follow Afford's availability and candidates per
  row rather than showing the verb for any target. File wire-anchored if it
  does not already.

## Done when

- A room with two goblins, one authored with `intimidate:` and one without:
  Afford lists only the first as a candidate; Intimidate at the second returns
  `ErrNoSocialEntry`; the shipped answer-table walk (front-room goblin) is
  unchanged.
- Kirk walks it on the local stack with the World Builder team.

## Later slices (named, not built)

- Authored options beyond the two skills: the line the player chooses, the
  check, the answer. Intimidate and Persuade become two options an author
  happened to write.
- Options gated on a known fact or a held item (intel as a key).
- Out-of-combat-only / non-hostile-only offer (R5 direction).
- A Give verb (an item to a creature). Presence transfer already covers the
  toy room, so it has no use case yet.

— monster-ai agent, on behalf of KirkDiggler
