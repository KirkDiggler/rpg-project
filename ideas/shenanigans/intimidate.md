# Intimidate — a scared goblin can be told to run

## Status: DESIGN, resolved in session with Kirk 2026-09-16; this PR is the review surface. First customer of [README.md](README.md) (the shenanigans primitive). Builds on the monster minds (rpg-toolkit#1745, `rulebooks/dnd5e/behavior` v0.5.0), the authored check (rpg-project#350/#351) and the hold-out's facts and dispositions (rpg-project#375). Tracking issue: rpg-project#454.

North star (Kirk): **we want to see somebody make the roll, and the outcome shapes the encounter.** The author plants the setup; the player finds the interesting way to win.

Driving acceptance case: **in The Three Minds, the fighter walks up to the archer goblin, rolls Intimidation in front of the table, beats its DC, and the goblin runs as far as it can see the fighter instead of shooting — without anyone touching Go.** The thug, told the same thing, takes it personally and charges.

## Context — what exists today (verified 2026-09-16)

- **The check machine is built.** `resolution.MakeCheck` takes a character record and a list of approaches (`encounter.CheckApproach{Ability, Tool, DC}`; the Ability field carries a skill ref or a bare ability ref), picks the checker's best route, rolls on the ability-check chain, and poses for offers. Session runs it through `stageCheck`; Unlock returns `Beaten/Total/DC` and a `Roll` while paused. Search's seam cannot pose and declines on the checker's behalf.
- **Only characters check.** `stageCheck` refuses a member with no character record (rpg-project#351). A monster rolling Insight against you is inexpressible today.
- **A deed is `{Verb, Actor, Target, Where}` and nothing more.** No weight, no count. The deed package's own doc names `"intimidate"` as an example verb. The encounter is the only publisher: `landAttack` builds one on a hit or a miss and lands it, through the stage, on every witness whose sight reaches the actor's cell. Each witness keeps an actor's latest deed only.
- **Every mind consults only the attack deed.** The retaliator's `Judge` and `grudge` both test `Verb == DeedAttack`. `scenarios.md` records provocation as unpaid: "an attack on me is hardcoded".
- **The coward is `Room: 2` and no grudge.** It has no ladder of its own: rung 0 of the shared ladder backs away from any live named creature nearer than `Keep`, and the coward's `Keep` answers two steps. So the coward already flinches from anyone adjacent, and at range it shoots (rpg-toolkit#1758: a fleeing coward orbits its pursuer).
- **The world remembers facts, not deeds.** A fact is an authored id that an intel record `reveals`; a member learns it by presence or by taking the record; a disposition `until: { fact: … }` flips the stance once the faction's mind knows. The raider camp fixture is the live example.
- **Two records, two purposes.** Deeds live in the play record as holdings — what happened in the run, the mind's memory. Facts live in the world journal — what carries out of the run. Both are records; neither is the other's cache.
- **Search costs nothing; Attack, Cast, Move, Activate are priced** through `Afford` and `CostOf*` spend profiles. `VERB_*` on the wire has no social entry.
- **A monster has Wisdom** (the goblin's is 8) and a private skill map with no getter. `Frightened` exists as a condition ref with no behavior; this design does not use it.

## The two cuts that broke, and why (recorded so they are not proposed again)

1. **"On success, set the goblin to fleeing."** A flee flag on the member is state the driver would have to read live, which rule A2 forbids, and it would make the outcome the verb's instead of the mind's. A berserker told to run does not run. The preset must decide.
2. **"One fact that both the mind and the disposition read."** Proposed in session, withdrawn: the play record (deeds, what happened in the run) and the world journal (facts, what carries out of the run) are both records with different purposes, by design — Kirk's correction, 2026-09-16. Intimidate writes to each from one seam and copies nothing; there is no missing bridge to build.

## Decisions (resolved in the 2026-09-16 session)

1. **Intimidate is a session verb, an action, one target.** `Intimidate{Session, Member, Target}`. It costs the standard action (`CostOfIntimidate`, `Slots: {ActionStandard: 1}`), joins `Afford`'s priced set as `VerbIntimidate`, and is refused outside the actor's turn the way Attack is.
2. **The target must be able to perceive the actor.** The audience is `witnessesOf(actor cell)` exactly as `landAttack` computes it; the target must be in that set or the verb is refused with `ErrOutOfReach`. No distance cap beyond sight. Everyone in the set learns what happened, not only the target.
3. **The roll is the authored check.** `stageCheck` + `MakeCheck` with the monster's approaches, posing for offers the way Unlock does. The response returns `Beaten, Total, DC, Paused, Roll?`, and a beat `Intimidated{actor, target, dc, total, beaten}` goes down the log so the table sees the die whether it beat or not.
4. **The DC is the monster's, authored on the placement, derived when absent.** Placement key `intimidate:` takes a `CheckSpec` (the same list Unlock and Search author): `intimidate: [{ ability: intimidation, dc: 12 }]`. Absent means the derived default: one approach, Intimidation, DC = **passive Insight = 10 + Wisdom modifier (+ proficiency if the definition lists Insight)** — the living-world §3 rule "passive is derived, never stored", applied to the monster. Goblin 9, thug 10. The author may list other routes (`{ ability: str, dc: 15 }` flexes a muscle) and the checker's best applies; that is the check machine's existing law, not a new one.
5. **Success lands a deed `intimidate` on the witnesses.** `DeedIntimidate` beside `DeedAttack` in the encounter; `Actor` = the character, `Target` = the monster. Failure lands nothing (see open items for the provocation question). Same stage, same rewrite per witness, same clock high-water.
6. **The preset decides what the deed is worth — this is the mind capability the slice pays for.**
   - **Coward: fear.** A fresh `intimidate` deed against me by X makes `Keep` answer *my sight range* for X, for `Fear.Patience` turns (default 3, the retaliator's number, for the same reason: a fight's length). Rung 0 then fires whenever X is seen: the goblin runs while it can see the fighter and stops when it cannot. Nothing else in the ladder moves. "How far" stays a claim the ladder makes, not a profile field.
   - **Berserker: provocation.** `grudge` generalises from `Verb == DeedAttack` to a *provocation set* `{attack, intimidate}`; `ExcuseNever` means an intimidation counts. The thug charges whoever threatened it. This pays the "provocation is hardcoded" debt scenarios.md names.
   - **Retaliator: excused.** Its excuse is about hands, and a threat is not a swing; the deed is held and ignored. Zero value tells the truth: a mind with no `Fear` and no provocation entry for `intimidate` does nothing with it.
7. **The world half is opt-in, authored beside the check.** Placement key `on: { intimidated: { fact: sergeant-cowed } }`: on success, every witness learns the fact through the existing `learnFact` path, and `until: { fact: sergeant-cowed }` on a disposition does the rest. The `on:` map is keyed by verb so the second shenanigan adds a key, not a field. Absent means no fact and the camp does not care.
8. **Proficiency is the checker's, as always.** Intimidation proficiency, expertise, Guidance and Bardic Inspiration all reach the roll through the check machine untouched.

## Components

### rpg-api-protos — session/v1alpha1
`rpc Intimidate(IntimidateRequest{session, member, target}) returns (IntimidateResponse{beaten, total, dc, paused, optional roll})` mirroring Unlock; `VERB_INTIMIDATE` appended to `Verb`; beat `Intimidated{actor, target, dc, total, beaten}` in events.proto. Additive only. Opens READY.

### rpg-toolkit — rulebooks/dnd5e (root)
`monster.PassiveInsight()` (10 + WIS mod + Insight proficiency if listed), the derived default DC. One module, one PR.

### rpg-toolkit — rulebooks/dnd5e/behavior
`Fear{Patience}` on the preset beside `Grudge`; coward gets `Fear{Patience: 3}`; `Keep` answers per creature (sight range for a feared actor within patience). The provocation set replaces the hardcoded `DeedAttack` in `Judge` and `grudge`. Proofs: (a) a cowed coward at range runs instead of shooting, and stops when the actor is out of sight; (b) patience expires and it shoots again; (c) the berserker ranks the intimidator first; (d) the retaliator's ranking is unchanged by an intimidate deed. Mutation: make `Keep` ignore the deed and show (a) fails.

### rpg-toolkit — rulebooks/dnd5e/encounter (includes dungeonspec)
`DeedIntimidate`; an `Intimidate` composition op: witnesses, land the deed, learn the fact if authored; `PlaceSpec.Intimidate CheckSpec` and `PlaceSpec.On map[string]OnSpec` (known-key list updated, monsters only, fact ids join `mintedFactIDs`). Proofs: field errors `place[i].intimidate[j]`, `place[i].on.intimidated.fact` unknown to the run; the deed reaches exactly the witnesses.

### rpg-toolkit — rulebooks/dnd5e/session
`Manager.Intimidate`, `VerbIntimidate` priced, `CostOfIntimidate`, target-in-witnesses refusal, `stageCheck` with the placement's or the derived approaches, the pose window (Unlock's shape), the beat. Proofs: refused off-turn and out of sight; a beaten check lands the deed and the next goblin turn is `Away`; a missed check lands nothing and the goblin still shoots; the derived DC for a goblin is 9; the authored list overrides it.

### rpg-api
Handler + orchestrator for `Intimidate`, beat translation, `content/reference-minds.yaml` gains the sergeant line as the walk fixture.

### rpg-dnd5e-web
An Intimidate row in the ActionDock with a target pick; the roll surfaces where Unlock's does. Builder: an `intimidate` DC field and an `on: intimidated` fact picker on the monster placement card, after the release.

## Order of work
1. Protos, READY, merges first.
2. rulebooks/dnd5e (root) and rulebooks/dnd5e/behavior in parallel — no shared pin.
3. encounter on their pseudo-versions, then session on all three.
4. rpg-api on the session pseudo-version; web on the api branch.
5. `dev-env.sh up local/mind` on the branches; Kirk walks; merge inside-out; repin; tags.

## The walk (The Three Minds)
1. The fighter closes on the archer goblin at [4,2] and picks Intimidate. The table sees the d20, the total and DC 9.
2. On a beat: the goblin's next turn is a run to the edge of sight, no shot. Three turns later, out of sight or out of patience, it shoots again.
3. The fighter tells the thug the same thing. The thug's next turn is a charge at the fighter, whoever else is closer.
4. Optional, on the raider camp: the sergeant placement carries `on: { intimidated: { fact: sergeant-cowed } }` and the camp's disposition `until: { fact: sergeant-cowed }`; cowing the sergeant in front of the camp turns it.

## Unpaid, on purpose
- **Failure is free.** A missed roll lands no deed. Whether a failed threat provokes a berserker is an open item, not a hidden default.
- **No monster Insight roll.** The DC is passive Insight; the live contest waits for a monster checker (rpg-project#351's shelf).
- **No `Frightened`.** The condition ref stays unwired; the mind is the outcome.
- **No word spreads.** A witness learns; nobody tells anyone. The runner is a separate primitive.
- **No per-character stance.** `on: intimidated` flips a faction pair, the only edge the stance table has. Disguise needs a per-character edge and brings it itself.

## Open items (for Kirk on the PR)
1. Does a failed Intimidate provoke the berserker? (Proposed later: land the deed with `Beaten=false` carried, let the provocation set read it. Not in v1.)
2. `Fear.Patience` default 3 — right number, or the coward's own?
3. Is an action the right price, or should the first Intimidate of a fight be free the way a warning shout is at the table?
