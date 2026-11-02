---
status: DESIGN, R1–R3 RULED 2026-11-02 on rpg-project#785; building
story: rpg-project#785
---

# Acting in free roam — the bar is yours on the world clock

**Kirk, 2026-11-02:** "let's see what we can do about attacking in free roam.
our bar should be available to us. if i want to cast a spell I should be able
to. light, heal etc are valid. attacking anyone I guess is also valid."

## Law

**The world clock is a place to act.** Every verb the turn clock prices is
offered on the world clock. What differs is the price: the turn clock prices
in slots of a budget; the world clock prices in rounds of the world, one per
action, the same round a walk pays every pace (the-world-runs §5).

**A hostile act makes a hostile pair.** Hostile intent across a neutral pair
turns the pair's factions hostile before the deed's testimony lands (the
aggression law, rpg-project#493 R3, `deed.go`). The fight then forms through
the same sight refresh a step into an enemy's view forms it. No verb forms a
fight by itself; seeing an enemy does.

**A cast that hurts nobody forms nothing.** Light, a heal, a buff: a round of
the world passes, witnesses hear a voice if the cast speaks, and nothing else
changes. The sound of a damaging cast travels as it does today.

**The door is the session's; the engine already answers.** The encounter
prices actions on the world clock today (`spendWorldAction` in cast, loot,
interact, option, the attack outcome, door verbs) and runs the aggression law.
The session's `Afford` is what hands the bar one row; the web draws only what
`Afford` returns. Opening the door is a session change; the dock follows.

## What is true today, read not guessed

- `afford.go`: on a non-turn clock, `Afford` returns the Speak row alone,
  priced at `SlotNone` with no shortfall ("THE WORLD CLOCK OFFERS THE SOCIAL
  VERB, AND NOTHING ELSE"). The front-room goblin's R3 (rpg-project#457)
  brought the social verb to the world clock; "nothing else" was the
  implementation's line, not the ruling's.
- `ActionDock.tsx`: on the world clock it draws `executableDeclarations`
  minus Move: "ONLY WHAT AFFORD RETURNED, and no local decision about which."
- `outcome.go`, `cast.go`, `interact.go`, `loot.go`, `option.go`,
  `doorverbs.go`: each pays `spendWorldAction(actor)` on the world clock.
- `deed.go`: "THE AGGRESSION LAW RUNS FIRST: hostile intent across a neutral
  pair makes it hostile" before the deed's testimony lands.
- Every verb's session door compares the clock's active member on the turn
  clock ("not your turn"); on the world clock there is no active member. S1
  reads each door and prices it as Speak's is priced.

## Rulings

| # | question | options | recommendation | status |
|---|---|---|---|---|
| R1 | What does an action cost on the world clock? | A one round of the world per action, as the engine charges today; the bar shows no slot and no shortfall. B nothing. C a budget per round like the turn clock. | A | ruled 2026-11-02 (Kirk: "A") |
| R2 | What does a hostile act on a neutral do? | A the aggression law as ruled (#493 R3): the two factions turn hostile, the fight forms with everyone in view. B only the struck creature turns hostile. | A | ruled 2026-11-02 (Kirk: "A. I suppose it's an NPC had no faction or was a faction of 1 then only they would turn": the unit is the faction; a creature with none, or alone in one, is the whole of its faction) |
| R3 | Does the first swing from free roam land before initiative? | A the deed lands on the world clock (as a cast does today), then the fight forms: the swinger gets the first blow for free, the surprise flag records who was caught. B declaring the swing forms the fight first; the swing becomes the first turn, initiative decides who is first. | A | ruled 2026-11-02 (Kirk: "A") |

## Slices

- **S1 the bar on the world clock** (session; api and web unchanged): `Afford`
  on a non-turn clock compiles every verb's row the turn-clock path compiles,
  priced `SlotNone`, no shortfall, availability by the verb's own facts
  (range, target, sheet). Each verb's door accepts a world-clock declaration
  and the encounter charges the round. Walk: on the castle in free roam the
  bar shows Attack, Cast, Activate and the rest; cast Light and watch a world
  round pass; swing at the captain and watch the garrison turn hostile and the
  fight form (R2, R3).
- **S2 rows and the walk**: garrison keep in free roam, a cast and a swing;
  rows on #785; the sneak and clock stories' walks gain the free swing as the
  way to make a clang outside a fight.

## Non-goals

Reactions on the world clock (an opportunity attack needs a turn clock; none
is taken against a free-roam swing). A surprise round as a rule. Changing
what the turn clock prices.

— cross-team agent, on behalf of KirkDiggler
