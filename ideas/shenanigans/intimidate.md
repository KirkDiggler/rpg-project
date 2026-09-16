# Intimidate — a scared goblin can be told to run

## Status: SHIPPED 2026-09-16 (walked by Kirk: "saw intimidate take effect. looks good to me.") — rulebooks/dnd5e v0.176.0 · mind/behavior v0.4.0 · encounter v0.85.0 · dnd5e/behavior v0.6.0 · session v0.92.0 · rpg-api-protos #339 · rpg-api #998 · rpg-dnd5e-web #1102. Decisions the build corrected are marked CORRECTED below and gathered in "What the build corrected". Originally resolved in session with Kirk 2026-09-16 (design PR #455). First customer of [README.md](README.md). Tracking issue: rpg-project#454.

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
2. **The target must be able to perceive the actor.** The audience is `witnessesOf(actor cell)` exactly as `landAttack` computes it; the target must be in that set or the verb is refused. CORRECTED: with the session's own `ErrUnwitnessed`, not `ErrOutOfReach` — Attack's sentinel means "get closer", and a threat has no range: closing on a goblin that cannot see you changes nothing. The wire maps it to FAILED_PRECONDITION. No distance cap beyond sight. Everyone in the set learns what happened, not only the target.
3. **The roll is the authored check.** `stageCheck` + `MakeCheck` with the monster's approaches, posing for offers the way Unlock does. CORRECTED (Kirk, on protos #339): the response and the beat were two copies of the truth. The wire response carries only what is the caller's alone — `paused`, the `roll` while an offer window is open, and the save/delivery reports. The outcome is the beat `Intimidated{actor, target, dc, total, beaten}`, read by everyone, the actor included, the way Search's find is read. While paused the pre-offer total is carried and the DC withheld, Unlock's rule shared. Unlock's `beaten/total/dc` on its response is the older shape, not a precedent.
4. **The DC is the monster's, authored on the placement, derived when absent.** Placement key `intimidate:` takes a `CheckSpec` (the same list Unlock and Search author): `intimidate: [{ ability: intimidation, dc: 12 }]`. Absent means the derived default: one approach, Intimidation, DC = **passive Insight = 10 + the listed Insight total when the stat block lists Insight, else 10 + Wisdom modifier** (CORRECTED during the build: an SRD block lists a skill as a TOTAL — the goblin's Stealth +6 is more than DEX + proficiency — so a listed number is the whole number and the proficiency bonus is never added on top) — the living-world §3 rule "passive is derived, never stored", applied to the monster. Goblin 9, thug 10. The author may list other routes (`{ ability: str, dc: 15 }` flexes a muscle) and the checker's best applies; that is the check machine's existing law, not a new one.
5. **Success lands a deed `intimidate` on the witnesses.** `DeedIntimidate` beside `DeedAttack` in the encounter; `Actor` = the character, `Target` = the monster. Failure lands nothing (see open items for the provocation question). Same stage, same rewrite per witness, same clock high-water.
6. **The preset decides what the deed is worth — this is the mind capability the slice pays for.**
   - **Coward: fear.** A fresh `intimidate` deed against me by X makes `Keep` answer *no distance is far enough* for X (CORRECTED: the view carries no sight range; rung 0 only asks about a creature currently held, so fear ends the moment X becomes a memory — better at the edge than a literal range, where a fighter at the boundary would be shot at), for `Fear.Patience` turns (default 3, the retaliator's number, for the same reason: a fight's length). Rung 0 then fires whenever X is seen: the goblin runs while it can see the fighter and stops when it cannot. Nothing else in the ladder moves. "How far" stays a claim the ladder makes, not a profile field.
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

## What this makes possible — the outcome Kirk named (2026-09-16)

> "Intimidate one goblin and it runs away from us into another room, possibly to get its friends, and once around its friends it can lose the intimidation and come back for us with reinforcements."

That is three slices, and slice one is shaped so the next two are additions, not rewrites.

| Slice | The claim the ladder gains | What exists | What is new |
|---|---|---|---|
| 1. **Cowed** (this doc) | Away from the one who scared me, as far as I can see it | rung 0 Away; `AwayPath` is the encounter's `routeAway`, one cell, corner-aware | `Fear{Patience}` on the preset; `Keep` answers per creature |
| 2. **Regroup** | Toward my friends while I am afraid, and courage in their company | `Remembered`/`Seen` on the view already carry allies; Toward exists for enemies | Toward an *ally* (a new use of rung 2); `Fear.Company`: fear lapses early when this many live allies are in sight. Zero value = never lapses early |
| 3. **Alarm** | Tell my friends where the party was | `deed.Deed` carries `Where`; a perception `Report` moves the channel without sustaining it — "a rumour is not a sighting" is the package's own law | The runner lands a rumour of the party's last-known position on allied witnesses; they rank a rumoured target and go Toward it. This pays "word spreads" for the mind's half; the world's half (the faction mind learning a fact) stays its own |

Two things slice one must not foreclose, and does not: `Fear` is a struct, not a number, so `Company` is a field added later; and fear is keyed by the deed's actor, so "afraid of X" and "toward Y" can hold at once.

One thing to watch on the walk: whether `routeAway` crosses a doorway into the next room, or only backs into the same room's far corner. If it stops at the door, "into another room" is a `routeAway` finding, not a mind finding, and it goes to the encounter (rpg-toolkit#1758 is the sibling: a fleeing coward orbits its pursuer).

## What the build corrected (2026-09-16)

Recorded so the reasoning survives, per rpg-project/CLAUDE.md "Mistakes are not the problem".

- **Five toolkit PRs, not four.** `Keep` had to become per creature (fear is per creature or it is not fear), and the ladder lives in `mind/behavior`, its own module. rpg-toolkit#1786 gave `KeepInput` a `Contact` and moved the `Keep` call inside rung 0's loop.
- **The response is not a copy of the beat** (decision 3), **the sentinel is the session's own** (decision 2), **passive Insight is the listed total** (decision 4), **fear has no range** (decision 6) — each marked in place above.
- **`Judge` reads no verb at all.** A verb-filtered `Judge` left a coward's threat unattached because its grudge is the zero value. Whether two holdings are one figure is a perception claim, true whatever she did. The provocation set lives on the grudge only.
- **The session had to type the beat, and the spawn had to carry the authoring.** Integration found the encounter publishing `intimidated` while `kindFor` had no case (the beat crossed as `EventUnknown`, so the roll would have reached no client), and `SpawnInput` with no field for a placement's `intimidate:` or `on:` (every threat would have resolved against the derived DC and the fixture's DC 12 would have been a lie). Both fixed in session with proofs; the guard that would have caught the first at build time is rpg-toolkit#1792.
- **Saving a cowed run was refused** until the placement's authored fact joined the trust boundary's minted list. Fixed in encounter; the proof cows the goblin before the round trip.
- **`encounter.Witnesses` is a public read**, so Afford's candidate list and the verb's refusal agree about who can be shouted at without spending the action to find out.
- **rpg-api verbs go handler → manager**; there is no orchestrator method for any session verb. The brief said otherwise.
- **Web needed no picker.** Intimidate is a priced verb, so the affordance panel compiled its own member-targeted row. The work was the six hand-written verb lists that turn a forgotten entry into a dead button (rpg-dnd5e-web#1104).
- **Two CI traps, both fixed in flight:** repo-wide GOPRIVATE broke the base-graph load on a squashed-away pseudo-version (rpg-toolkit#1791, per module now), and a tag cut on main mid-wave outranked the branch pseudo-versions so a dev merge would have dropped the feature (merge main into the toolkit branch first, then repin).

Findings filed: rpg-toolkit#1792 (beat completeness guard), #1793 (`DoorBody.beaten` omitempty), #1796 (`place()` input struct), rpg-dnd5e-web#1104 (verb registry).

## Unpaid, on purpose
- **Failure is free.** A missed roll lands no deed. Whether a failed threat provokes a berserker is an open item, not a hidden default.
- **No monster Insight roll.** The DC is passive Insight; the live contest waits for a monster checker (rpg-project#351's shelf).
- **No `Frightened`.** The condition ref stays unwired; the mind is the outcome.
- **No word spreads.** A witness learns; nobody tells anyone. Regroup and Alarm above are slices two and three, not this PR.
- **No per-character stance.** `on: intimidated` flips a faction pair, the only edge the stance table has. Disguise needs a per-character edge and brings it itself.

## Open items (for Kirk on the PR)
1. Does a failed Intimidate provoke the berserker? (Proposed later: land the deed with `Beaten=false` carried, let the provocation set read it. Not in v1.)
2. `Fear.Patience` default 3 — right number, or the coward's own?
3. Is an action the right price, or should the first Intimidate of a fight be free the way a warning shout is at the table?
