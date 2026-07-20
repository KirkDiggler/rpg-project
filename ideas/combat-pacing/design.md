---
name: Combat Pacing & Dice
description: The attack loop needs beats — theatrical suspense choreographed over already-resolved server rolls, so "swing, miss, end turn" stops feeling flat
updated: 2026-07-20
confidence: high on the wire contract (read events_pb.ts v0.1.108 + encounterStreamDispatch.ts in full); design positions are proposals for Kirk
status: Proposal — ideation, no issue yet (rpg-dnd5e-web#525-adjacent, Game-UX charter)
---

# Combat Pacing & Dice

## The Need (Kirk, verbatim)

> "we need something when we attack. the swing, miss, end turn, get hit, swing, miss,
> end turn needs some pacing. I am big on rolling dice and think a 1 and a crit can be
> fun. i think the suspense is nice rolling dice and we can get good engagement around it."

Today an attack resolves as three stream events that land in the same server tick and
get printed to the combat log instantly. There is no *moment*. A hit and a miss look the
same: a line of text appears, HP maybe changes, the turn passes. The loop is mechanically
correct and emotionally flat.

## The one architectural fact that shapes everything

**The server has already rolled.** The attack story arrives as a correlated triple sharing
one `correlation_id` on the `EncounterEvent` envelope:

```
ActionResolved   (the umbrella beat: actor, action ref, economy consumed)
  → AttackResolved  (roll, bonus, target AC, hit / miss / CRIT — fires on a MISS too, the #594 fix)
    → EntityDamaged (HP delta + damage breakdown — hit only)
```

All three are fully resolved before they hit the wire. There is **no roll-started event** —
nothing is undecided by the time the web sees it.

So suspense is not "will the die land?" — the die has landed. Suspense is **theatrical: we
choreograph the *reveal* of a result no player has seen yet.** This is exactly what a DM does
when they've already read the die behind the screen and say "…and that's a *hit*." For 4-player
co-op it is not a compromise — no player ever had secret information, so a paced reveal is
*indistinguishable from* true suspense to everyone at the table. We lean into this rather than
apologize for it. (§Wire covers why "true" two-phase rolls would cost more and buy nothing here.)

## 1. The beat model

Decompose one attack into named beats. Durations are *Cinematic-mode* defaults, all tunable,
all skippable (tap-to-skip jumps to the verdict — never trap a player in an animation).

| # | Beat | Duration | What happens | Suspense? |
|---|------|----------|--------------|-----------|
| 0 | **Declare** | instant | Player clicks Attack + target. The click *is* the commit. (Already exists — the action menu.) | — |
| 1 | **Wind-up** | 150–250ms | Target token highlights, attacker "readies." Says *something is coming* so the reveal isn't a value popping from nowhere. | building |
| 2 | **The Roll** | 600–900ms | The d20 tumbles. Outcome hidden until it settles. Lands on the real `attack_roll` — the die shows the server's number, never a fake. | **peak** |
| 3 | **The Verdict** | 200–400ms | Die lands, total assembles (`roll + bonus vs AC`), stamp: HIT / MISS / **CRIT** / **nat-1**. The frame-breaker. | release |
| 4 | **Impact** | 300–500ms *(hit only)* | Damage number flies off the target, HP bar drains. **Skipped on a miss** — the whiff is the punctuation. | payoff |
| 5 | **Reaction** | variable *(if prompted)* | Shield / etc. A real interrupt, already modeled by `InputRequired` prompts. | interrupt |
| 6 | **Hand-off** | 200–300ms | Initiative tracker slides, next actor's pill lights. A breath between turns. | reset |

**Budget:** routine hit ≈ 1.2–1.6s · routine miss ≈ 1.0s (skips Impact) · **crit ≈ 2.5s (earned)**.

**How crits/nat-1s break the frame.** The routine loop is deliberately snappy so the exceptions
land. A **crit** (`critical === true`) stretches the Verdict: die glows gold, a screen sheen fires
(`SPR_FX_FantasyWarrior_Sheen01/02`, `Glow01–03`, `Beams01` all exist in the Synty FX sheet), the
damage number is oversized and gold. A **nat-1** (`attack_roll === 1 && !hit`) cracks the die red
with a short comedic fumble stamp — **short**, because a fumble should be funny, not a punishment to
sit through.

**How the miss stays snappy — this is the actual fix.** Kirk's complaint isn't that misses are too
fast; it's that they have *no weight*. But a *drawn-out* miss is worse than a flat one. The fix is a
**short-but-legible** miss: same Roll tumble, a quick MISS stamp, no Impact beat. Enough that the
whiff reads; not so much that the loop drags. Repeat misses in one turn compress further (§4).

## 2. Dice as an object

| Option | Cost | Wow | Trade-off |
|--------|------|-----|-----------|
| A — Log-flourish only | trivial | low | Die animates inside the log line. Suspense in the periphery isn't a *shared* moment — a ticker, not a table beat. |
| **B — 2D / CSS-3D die, focal** | **medium** | **high** | Die tumbles in a focal overlay, lands on the value, verdict assembles beside it. Cheap to animate (Framer Motion), crisp, theme-able with the FX sheets, trivial to "land on a predetermined number." |
| C — 3D physics die on the map | high | highest | A real rigid-body d20 thrown on the hex map. Forcing a physics die to settle on a predetermined face looks janky; camera framing fights the tactical map read. |

**Recommendation: B, anchored over the acting token, promoting to center-stage for crit/nat-1.**
Tying the die to *whose token is rolling* is what makes it a shared-table read — everyone sees
*Dave's* die tumble over *Dave's* fighter. Crits/fumbles earn a brief center-screen promotion.
This gets ~80% of C's drama for ~20% of the cost, keeps the map legible, and sidesteps the
"physics die must land on a fixed number" problem entirely (in 2D you just show the number).
**Build the die behind a beat interface so a 3D die can swap in later** without touching choreography.

**Where it renders:** over the acting token (primary) → center-stage (crit/fumble). *Not* buried in
the dock (too peripheral for the suspense beat) and *not* only in the log — the log stays exactly as
it is: the permanent, scrollable record.

**Click-to-roll vs auto-roll.** Kirk explicitly loves *rolling* dice, so agency matters:
- **Your own attack → click-to-roll (default).** After you commit the attack, a die appears and *you
  tap/flick it* to throw. The result is already server-decided; the *act of throwing* is yours. This
  is the engagement ritual. Keep it one satisfying tap — **do not** fake a timing/charge meter that
  appears to affect the outcome; that would lie about client authority and violate the boundary.
- **Spectating another player → auto-play.** You watch their die; you don't throw someone else's.
- **NPCs → auto-play**, paced by tier (§4).
- A **"quick/auto" preference** lets a speed-first player auto-roll their own turns too.

## 3. Shared witnessing

Spectating a teammate's turn is half the co-op experience — Kirk named it. Every viewer receives the
same correlated triple on their per-viewer stream, so **every client plays the same choreography**:
the roller *throws*; spectators *watch the same die, the same verdict, the same celebration*. Dave's
crit lands for the whole table.

**Do not hard-sync clients.** Natural near-simultaneity (all clients get the events within network
jitter) reads as "live." A few hundred ms of skew is fine and even feels alive; lockstep is a rabbit
hole with no payoff for turn-based co-op.

**The hand-off gets its own beat** (Beat 6): the initiative tracker advancing is the natural breath
between turns, and it's where "waiting for Dave…" lives so a slow player's turn never reads as a hang.

## 4. Pacing knobs

- **Repeat-roll compression.** First roll of a turn gets full ceremony; extra-attack / two-weapon /
  a grind of NPC swings compress after the first (shorter tumble, faster stamp). This is the direct
  antidote to "swing, miss, swing, miss."
- **Quick-mode preference** (per player, behind the dock's existing settings gear):
  **Cinematic** (full beats) · **Brisk** (compressed) · **Instant** (log-only = today's behavior).
  Serves both the drama-lover and the "just let me play" player.
- **NPC pacing — currently instant, and that's wrong for the shared table.** Four goblins acting
  invisibly means players miss the story; full ceremony on every grunt drags. **Tier it: grunts get
  Brisk, bosses/elites get full ceremony.** The boss's crit should land like a player's.
- **What NEVER slows down:** movement, targeting, menu open/close, mode transitions, and *your turn
  starting* (the menu must be live the instant it's yours). Choreography paces *resolution reveals*,
  never *input latency*. Tap-to-skip is always available.

## 5. Engagement mechanics (scope-fenced)

**Cheap wins (round 1):** crit gold-flash + oversized damage pop (FX sheets exist) · nat-1 red-crack +
comedic log flourish · escalate the log's newest crit line with a one-frame flash (the log already
colors crit gold / hit red / miss gray in `CombatLog.tsx`) · directional hit indicator
(`SPR_FX_FantasyWarrior_DamageDirection*` exists) · **design an SFX slot into every beat** even if
audio ships later — audio is the single biggest cheap game-feel multiplier.

**Expensive wins (later, fenced out):** streak/momentum tracking ("3 crits this fight!") · per-class
die skins (barbarian's chipped-bone d20) · 3D physics dice (option C, behind the same interface) ·
cinematic camera moves on crits · party-wide table-reaction FX.

## 6. Wire implications

**Almost all of this is client-side today.** Precise asks, each with a why:

1. **The dispatch discards the envelope — fix that first (client-side, no proto change).** `correlation_id`,
   `sequence`, and `timestamp` live on the `EncounterEvent` envelope, but `dispatchEncounterStreamEvent`
   passes only `payload.value` to each callback. To buffer the triple and play it as *one* beat sequence,
   the dispatch must thread `correlationId` (and ideally `sequence`) through to the combat callbacks.
   **This is the single load-bearing prerequisite** — a small refactor, no wire change.
2. **Do NOT ask for two-phase (roll-started / roll-resolved) events.** True two-phase would let the die
   be genuinely unknown mid-tumble, but it (a) doubles event volume on a hot path, (b) forces the server
   to hold the result and emit twice — added latency + a "resolved never arrives" failure mode, (c) buys
   *nothing* in co-op because no player has secret information anyway. Theatrical over the resolved triple
   is the right tool. Stated so Kirk can overrule if a future PvP mode ever needs true-secret rolls.
3. **Roll-metadata gaps (flag, don't ask yet):**
   - `AttackResolved` already carries `attack_roll`, `attack_bonus`, `target_ac`, `hit`, `critical`,
     `has_advantage/disadvantage` + source refs — enough for the full "20+4 vs AC 14, ADV from Dodging"
     story. Good.
   - **Not** carried: raw *damage* dice faces (`EntityDamaged.damage_breakdown` is by source —
     "shortsword:5, sneak_attack:2" — not "2d6→[4,3]"). A *damage-dice tumble* would need those. Not for
     round 1 (the d20 is the star); a real proto ask if wanted later.
   - **Not** carried: the *discarded* die on advantage/disadvantage. Showing two d20s with one greyed is a
     lovely advantage visual but needs the dropped value. Real proto ask if wanted; flag only.

## 7. Prototype plan — `/concepts` "Dice & Pacing"

Fixture-first, per the concepts-route convention (the fixture shape *is* the draft of what the
reassembly layer will produce).

**Round 1 proves:**
- A beat sequencer plays a reassembled triple as timed beats from a **typed fixture**, not a live stream.
- The 2D die tumbles over a mock token, lands on the fixture's `attack_roll`, verdict stamps, damage pops.
- Crit / nat-1 / clean-hit / clean-miss / hit-with-advantage all render distinctly.
- Cinematic / Brisk / Instant visibly differ.
- Click-to-roll (self) and auto-play (spectator) both drive the same sequencer.

**Fixture shape:**
```ts
type AttackFixture = {
  correlationId: string;
  action: ActionResolved;          // umbrella
  attack: AttackResolved;          // roll / hit / crit
  damage?: EntityDamaged;          // hit only
  status?: StatusApplied;          // optional condition
};
// + toggles: viewerRole: 'self' | 'spectator';  paceMode: 'cinematic' | 'brisk' | 'instant'
```
Cases: clean hit · clean miss · nat-20 crit · nat-1 fumble · hit-with-advantage · multi-attack-same-turn
(compression) · NPC grunt swing.

**Acceptance checks (eyeball a rendered frame each):**
- A crit is visibly *more of an event* than a hit (duration + FX).
- A miss resolves in ≤~1s yet the whiff reads (doesn't cut to nothing).
- Instant mode ≈ today (log line, no choreography) — proves the escape hatch.
- The sequencer consumes the *exact* shape the reassembly layer will emit → promotion = wire reassembly +
  swap fixture for stream, zero shape drift.
- A spectator view of another token's crit plays the same celebration.

**Round 2 (not round 1):** land the dispatch envelope change (§6.1), swap fixtures for the live stream,
promote into `EncounterView`, tune durations against a real fight via MCP playtest.

## 8. Open questions for Kirk

1. **Click-to-roll default?** Throwing your own die is the agency you named — my rec is it's the default
   for your turn, Brisk/auto opt-in. Or is the extra tap friction, and auto should be default?
2. **NPC pacing tiers** — good with "grunts Brisk, bosses full ceremony," or should *every* enemy roll
   carry weight (risks drag with many enemies)?
3. **Fumble tone** — comedic (table laughs) or ominous (a fumble is a real cost)? Changes the nat-1
   FX/SFX language. I lean comedic — matches co-op energy.
4. **Damage dice** — is the d20 attack roll enough drama for round 1, or do you also want the *damage*
   dice to tumble? The latter is a proto ask (§6.3). I lean d20-only first.
5. **Die's home** — over the acting token (my rec: spatially tied, shared-table legible) or center-stage
   always (more cinematic, covers the map)? Crit promotes to center either way.
6. **Audio in scope for the playtest**, or visual-only for now? Design leaves SFX slots regardless, but
   audio is the biggest cheap multiplier — worth knowing.

## Pointers

- Wire contract: `rpg-api-protos` `dnd5e/api/v1alpha2/encounter/events_pb.ts` (`EncounterEvent` envelope,
  `AttackResolved`, `EntityDamaged`) · web `src/api/encounterStreamDispatch.ts` (the envelope-discarding dispatch)
- Render targets: web `src/components/game/{EncounterView,CombatLog,EncounterMap}.tsx`, `src/hooks/useCombatLog.ts`
- FX assets: `rpg-game-assets/harness/models/synty/ui/library/` — `fx/` (Sheen, Glow01–03, Beams01,
  DamageDirection*, Sparkle01), plus `banners/`, `frames/` for verdict stamps
- Prototype home: web `/concepts` route (`src/concepts/`), fixture-first per `docs/how-to/concepts-route.md`
- Charter: rpg-dnd5e-web#525 (Game-UX), north-star `ideas/encounter/v1alpha2/design.md`
</content>
</invoke>
