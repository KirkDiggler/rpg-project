---
name: Combat Pacing & Dice
description: The attack loop needs beats — theatrical suspense choreographed over already-resolved server rolls, so "swing, miss, end turn" stops feeling flat
updated: 2026-07-22
confidence: high on the beat model and pacing decisions (Kirk-approved); §1's beat durations are the implemented-and-measured values Kirk accepted live on 2026-07-22 for this round-one concept iteration, not locked production timing; the wire's correlation/cardinality shape (§6 item 1) is still an open contract question round one defers to CONTRACT.md, not a fact this doc assumes; the envelope-metadata delivery path is now split into two approved, independently landable Platform prerequisites (§9: rpg-api#701 projects toolkit metadata onto effect envelopes; rpg-dnd5e-web#582 preserves envelope metadata at typed callbacks)
status: Round 1 concept, in progress — tracked as rpg-dnd5e-web#561 (Board 19, Feature=Game Screen, Team=UI/UX). Broader charter: rpg-dnd5e-web#525 (Game-UX). Beat-timing concept iteration accepted by Kirk 2026-07-22 (§1); concept-stage only. Live promotion rpg-dnd5e-web#581 requires both Platform prerequisites in §9: rpg-api#701 and rpg-dnd5e-web#582.
---

# Combat Pacing & Dice

## The Need (Kirk, verbatim)

> "we need something when we attack. the swing, miss, end turn, get hit, swing, miss,
> end turn needs some pacing. I am big on rolling dice and think a 1 and a crit can be
> fun. i think the suspense is nice rolling dice and we can get good engagement around it."

Today an attack resolves as stream events that land in the same server tick and get
printed to the combat log instantly. There is no *moment*. A hit and a miss look the
same: a line of text appears, HP maybe changes, the turn passes. The loop is mechanically
correct and emotionally flat.

## Ownership & routing

- **Primary:** rpg-dnd5e-web#561 (Board 19, Feature = Game Screen, Team = UI/UX). Link
  this issue in any PR that touches the round-one concept.
- **UI/UX owns round-one concept and presentation** — the beat sequencer, the die object,
  placement comparison, fixtures, and CONTRACT.md all live in this lane per the Board 19
  ownership rule ("UI/UX owns presentation/interaction once state arrives").
- **Platform owns any eventual web stream/envelope/state seam** ("Platform owns
  wire-and-state correctness end to end — including the client half of stream/reconnect/
  snapshot plumbing"), but **no Platform implementation issue gets created from this idea
  until Kirk reviews the concept's `CONTRACT.md` and confirms a concrete requirement**
  (§6). Round one is fixture-first and does not touch the live stream, so there is
  nothing to file yet.
- **Assets is consultation-only** for this round — the 2D die and beat FX use existing
  Synty FX-sheet pointers (§Pointers). Assets only gets pulled in as an implementer if a
  future round chooses a 3D physics die or bespoke new art (option C, fenced out below).

## What we know about the wire — and what we don't

**What's true regardless of shape:** whenever an event carries a resolved die roll, the
roll already happened before the web sees it. The server never emits a "roll started"
event and lets the client watch it land — everything on the wire is already decided.
So suspense is not "will the die land?" — the die has landed. Suspense is **theatrical:
we choreograph the *reveal* of a result no player has seen yet.** This is exactly what a
DM does when they've already read the die behind the screen and say "…and that's a
*hit*." For 4-player co-op it is not a compromise — no player ever had secret
information, so a paced reveal is *indistinguishable from* true suspense to everyone at
the table. We lean into this rather than apologize for it. (§Contract questions covers
why "true" two-phase rolls would cost more and buy nothing here.)

**What is NOT true: that every attack arrives as one fixed, correlated triple.** The
event spine (`rpg-api-protos` `dnd5e/api/v1alpha2/encounter/events.proto`) supports
several real shapes, and the visual prototype must not assume only one of them:

- **A declared strike** (player clicks Attack + target via `TakeAction`) can produce
  `ActionResolved` → `AttackResolved` → `EntityDamaged` — but `EntityDamaged` fires **hit
  only**; a miss is `ActionResolved` + `AttackResolved` with no damage event at all.
- **An opportunity attack** (triggered by movement, not a declared action) can produce
  `AttackResolved` and an optional `EntityDamaged`, with **no `ActionResolved` at all** —
  there was no umbrella action to resolve; the toolkit resolves NPC OAs inline and
  publishes the attack events directly.
- **A non-attack `ActionResolved`** — e.g. a dodge, a dash, drinking a potion, or any
  other declared action that isn't a strike — can have **no attack event following it**
  at all: not every `ActionResolved` implies an attack happened.
- **Status events are not guaranteed to correlate** with an attack at all —
  `StatusApplied`/`StatusRemoved` share the envelope's `correlation_id` mechanism in
  principle, but nothing guarantees a status event is present, or that it arrives paired
  with an attack, for any given case.
- **There is no documented, universal completion marker for a correlation group** — no
  wire contract says "this `correlation_id` is now complete" for every case. In practice,
  for a declared `TakeAction` strike, a same-correlation `TurnStateChanged` (the actor's
  refreshed economy/menu) often follows the triple and can incidentally read as "this
  group is done" — but that's **observed implementation behavior, not a guarantee**:
  `TurnStateChanged` is projected to the acting client only (it carries the actor's own
  menu, not a broadcast), so it says nothing to spectators, and an opportunity attack has
  no `ActionResolved`/`TurnStateChanged` pair to close at all. Whether this incidental
  actor-only signal is reliable enough to build on, and what a spectator or an OA case
  needs instead, is exactly what CONTRACT.md must verify against real behavior — not a
  production completion contract this design gets to assume.

Treat correlation shape, cardinality, and completeness as **contract questions for
round one's CONTRACT.md to log**, not facts this design — or the visual prototype — gets
to assume. The beat model below is deliberately built to require nothing about
cardinality: it plays whatever roll-bearing event it's handed as one fixture-driven
scenario, and every round-one fixture is authored to match one of the real shapes above
(including the OA-without-`ActionResolved` case).

**The invariant that must never be violated:** authoritative HP, economy, and turn state
is applied immediately on receipt, exactly as it is today. Choreography **delays
presentation only — it never delays or reorders state.** A theater layer that sat on
state changes to "play them out" would be a real bug, not a design choice.

**Discontinuities (eventual production integration only — not built or proven round
one):** a snapshot delivery, a reconnect, a sequence gap, or a mode/turn transition
mid-choreography should flush any pending theater immediately rather than let stale
animation play against state that has since moved on. Any event shape that arrives
unknown, incomplete, or uncorrelated falls back to an **immediate, readable result** —
never blocks, never guesses, never stalls waiting for a sibling event that may not come.
This is a design invariant to build the eventual reassembly layer against; round one's
fixtures are authored, not streamed, so this behavior is stated here, not exercised by
the prototype (see §Round-one success below).

## 1. The beat model

Decompose one attack into named beats. Durations below are the **implemented-and-
measured Cinematic values, accepted by Kirk in a live concept review on 2026-07-22**
(round-one `/concepts` bench, PR rpg-dnd5e-web#579's "faceted d20 + suspenseful reveal
timing" iteration) — a substantial, deliberate lengthening from this doc's earlier
proposed ranges, chosen for suspense rather than snappiness. All beats remain tunable
and skippable (tap-to-skip jumps to the verdict — never trap a player in an animation).
**This acceptance is for the current round-one concept-stage iteration, not locked
production timing** — see the note after the budget line below.

| # | Beat | Duration (Cinematic) | What happens | Suspense? |
|---|------|----------|--------------|-----------|
| 0 | **Declare** | instant | Player clicks Attack + target. The click *is* the commit. (Already exists — the action menu.) | — |
| 1 | **Cue** | 300ms | Target token highlights, attacker "readies." Says *something is coming* so the reveal isn't a value popping from nowhere. | building |
| 2 | **Throw** | 2000ms | The d20 tumbles. Outcome hidden until it settles. Lands on the real `attack_roll` — the die shows the server's number, never a fake. | **peak** |
| 3 | **Verdict** | 1600ms | Die lands, total assembles (`roll + bonus vs AC`), stamp: HIT / MISS / **CRIT** / **nat-1**. The frame-breaker. | release |
| 4 | **Impact** | 900ms *(hit only)* | Damage number flies off the target, HP bar drains. **Skipped on a miss** — the whiff is the punctuation. | payoff |
| — | **Reaction** | variable *(if prompted)* | Shield / etc. A real interrupt, already modeled by `InputRequired` prompts. Not one of the five timed beats — its duration is whatever the reactor takes. | interrupt |
| 5 | **Release** | 300ms | A short breath before the next actor's turn. **Not** the full initiative-tracker slide-and-highlight treatment — that's future polish, out of round one's scope. | reset |

**Brisk** (repeat-roll/grunt-tier compression, §4) is the exact half of every Cinematic
value above: Cue 150ms, Throw 1000ms, Verdict 800ms, Impact 450ms, Release 150ms.

**Budget — measured wall-clock totals (Cue through Release, `performance.now()` in a
real browser against the round-one `/concepts` bench, per
`rpg-dnd5e-web` `docs/evidence/combat-pacing-561.md`), target vs. observed:**

- Cinematic routine **hit**: target 5100ms, observed ≈5130ms
- Cinematic routine **miss** (skips Impact): target 4200ms, observed ≈4229ms
- Cinematic **crit** (earned — Verdict/Impact stretch further): target 6600ms, observed
  ≈6629ms
- **Brisk**, exact-half of Cinematic, routine hit: target 2550ms, observed ≈2582ms

All observed values land within ~1% of target. **These are objective, stopwatch-style
measurements of the full beat sequence — not the same thing as how long a beat *feels*
to a player.** Kirk's own qualitative read, from his 2026-07-22 live review of this
exact iteration: *he can get behind this for the stage we are in* — and, on the routine
miss specifically, counting subjectively from when rolling starts, *it felt just under
about 3 seconds, and felt right.* That ~3s felt-impression and the 4200/4229ms measured
miss total are **two different measurements, not a contradiction** — a stopwatch counts
every beat uniformly; a person's felt sense of "how long until I knew" compresses
against clock time for a short, eventful interval. Both are recorded here deliberately:
the measured totals are the objective/regression evidence, Kirk's felt read is the
product-feel judgment call, and neither is being adjusted to match the other.

**Status of this acceptance:** Kirk's 2026-07-22 review accepts these values for the
**current round-one concept iteration** — it clears the `/concepts` bench to stand as-is
on timing. It is **not** a sign-off on production choreography, and these numbers are
not locked: they may change again in a later concept-iteration pass, and definitely
before (if ever) this pacing model is wired into the live `EncounterView` route against
real stream events.

**How crits/nat-1s break the frame.** The routine loop is deliberately kept *shorter
than* the crit/nat-1 exceptions so they land — "snappy" here is relative and felt, not a
sub-second wall-clock claim; per the accepted concept-iteration values above, routine
measures longer in raw ms than round one's earlier draft envisioned, and Kirk's own felt
read of a routine miss (~3s, "felt right") confirms the relative pacing holds even at
these longer absolute durations. A **crit** (`critical === true`) stretches the Verdict — the longest
frame-breaker in the set: die glows gold, a screen sheen fires
(`SPR_FX_FantasyWarrior_Sheen01/02`, `Glow01–03`, `Beams01` all exist in the Synty FX
sheet), the damage number is oversized and gold. A **nat-1** (`attack_roll === 1 &&
!hit`) is **playful and non-punitive by decision** — the die cracks red with a short
comedic fumble stamp, kept short because a fumble should be funny, not a punishment to
sit through.

**How the miss stays snappy relative to a hit or crit — this is the actual fix.**
Kirk's complaint isn't that misses are too fast; it's that they have *no weight*. But a
*drawn-out* miss is worse
than a flat one. The fix is a **short-but-legible** miss: same Throw tumble, a quick
MISS stamp, no Impact beat — readable, but it skips Impact. Repeat misses in one turn
compress further (§4).

## 2. Dice as an object

| Option | Cost | Wow | Trade-off |
|--------|------|-----|-----------|
| A — Log-flourish only | trivial | low | Die animates inside the log line. Suspense in the periphery isn't a *shared* moment — a ticker, not a table beat. |
| **B — 2D / CSS-3D die, focal** | **medium** | **high** | Die tumbles in a focal overlay, lands on the value, verdict assembles beside it. Cheap to animate (Framer Motion), crisp, theme-able with the FX sheets, trivial to "land on a predetermined number." |
| C — 3D physics die on the map | high | highest | A real rigid-body d20 thrown on the hex map. Forcing a physics die to settle on a predetermined face looks janky; camera framing fights the tactical map read. Fenced out — see §5. |

**Recommendation: B.** Build the die behind a beat interface so a 3D die can swap in
later without touching choreography.

**Placement is not pre-decided — it's what round one's prototype exists to compare.**
The design leans toward *token-anchored, promoting to center-stage for crit/nat-1* (tying
the die to *whose token is rolling* is what makes it a shared-table read — everyone sees
*Dave's* die tumble over *Dave's* fighter, with crits/fumbles earning a brief
center-screen promotion). But **round one builds both token-anchored and pure
center-stage placement against identical fixtures** so Kirk can compare them side by
side rather than take the recommendation on faith (see §7 and §8). *Not* buried in the
dock (too peripheral for the suspense beat) and *not* only in the log — the log stays
exactly as it is: the permanent, scrollable record.

**Click-to-roll vs auto-play.** Kirk explicitly loves *rolling* dice, so agency matters:

- **Your own attack → tap/flick to throw (default).** After you commit the attack, a die
  appears and *you tap/flick it* to throw. The result is already server-decided; the
  *act of throwing* is yours. This is the engagement ritual — one satisfying gesture.
  **Do not** fake a timing/charge meter that appears to affect the outcome; that would
  lie about client authority and violate the boundary. **A short auto-timeout throws the
  die for you if you don't act** — a player must never be able to stall the table by
  sitting on an un-thrown die.
- **Spectating another player → auto-play.** You watch their die; you don't throw
  someone else's.
- **NPCs → auto-play**, paced by tier (§4).
- A **"quick/auto" preference** lets a speed-first player auto-roll their own turns too.

## 3. Shared witnessing

Spectating a teammate's turn is half the co-op experience — Kirk named it. In production,
every viewer would receive the same roll-bearing events on their per-viewer stream, so
*in principle* every client can play the same choreography: the roller *throws*;
spectators *watch the same die, the same verdict, the same celebration*. Dave's crit
lands for the whole table. Round one's fixtures model this as a `viewerEntityId`/`role`
toggle (`self` vs `spectator`) per scenario rather than a live multi-client stream —
whether every viewer's real stream actually stays synchronized enough for this to hold
is a contract question, not something this doc or the prototype proves.

**Do not hard-sync clients.** Natural near-simultaneity (all clients get the events
within network jitter) reads as "live." A few hundred ms of skew is fine and even feels
alive; lockstep is a rabbit hole with no payoff for turn-based co-op.

**The hand-off gets its own beat** (Beat 5, Release): the breath between turns is where
"waiting for Dave…" lives so a slow player's turn never reads as a hang. The fuller
initiative-tracker hand-off treatment (tracker sliding, next pill lighting) is future
work — out of round one (§8).

## 4. Pacing knobs

- **Repeat-roll compression.** First roll of a turn gets full ceremony; extra-attack /
  two-weapon / a grind of NPC swings compress after the first (shorter tumble, faster
  stamp). This is the direct antidote to "swing, miss, swing, miss."
- **Pace preference** (per player, behind the dock's existing settings gear):
  **Cinematic** (full beats) · **Brisk** (compressed) · **Instant** (log-only = today's
  behavior, no theater — a real escape hatch that always shows a readable result).
  Serves both the drama-lover and the "just let me play" player.
- **NPC pacing — tiered by decision.** Four goblins acting invisibly means players miss
  the story; full ceremony on every grunt drags. **Grunts get Brisk; elites/bosses get
  Cinematic.** The boss's crit should land like a player's.
- **What NEVER slows down:** movement, targeting, menu open/close, mode transitions, and
  *your turn starting* (the menu must be live the instant it's yours). Choreography paces
  *resolution reveals*, never *input latency*. Tap-to-skip is always available, and
  auto-timeout means no beat can block the table indefinitely (§2).
- **Reduced motion.** With reduced motion enabled, every beat retains its cue / result /
  verdict / impact *semantics* — the target still highlights, the verdict still stamps
  HIT/MISS/CRIT, damage still pops — but drops the tumble/travel animation itself (no die
  spin, no flying damage number arcing across the screen). The result reads
  instantly; nothing about *what happened* is lost, only the motion.

## 5. Engagement mechanics (scope-fenced)

**Cheap wins (round 1):** crit gold-flash + oversized damage pop (FX sheets exist) ·
nat-1 red-crack + comedic log flourish, playful not punitive · escalate the log's newest
crit line with a one-frame flash (the log already colors crit gold / hit red / miss gray
in `CombatLog.tsx`) · directional hit indicator
(`SPR_FX_FantasyWarrior_DamageDirection*` exists) · **audio hooks only** — design an SFX
slot into every beat, but no actual sound implementation ships round one; audio is the
single biggest cheap game-feel multiplier and worth reserving the seam for, later.

**Expensive wins (later, fenced out):** streak/momentum tracking ("3 crits this
fight!") · per-class die skins (barbarian's chipped-bone d20) · 3D physics dice (option
C, behind the same interface) · cinematic camera moves on crits · party-wide table-reaction
FX · the fuller initiative-tracker hand-off animation (§3).

## 6. Contract questions (not asks yet)

**Almost all of round one is client-side and fixture-driven — there is nothing to ask
the platform team for yet.** `src/concepts/combat-pacing/CONTRACT.md` follows the same
lifecycle the equipment concept's contract file proved out (rpg-dnd5e-web#557): **during
the concept it records evidence, observations, and candidate gaps** as the fixture lab is
built and exercised — not proto asks written from assumption. **After Kirk reviews the
concept**, whichever candidate gaps are confirmed as real needs become the concrete
feature requests filed on Platform's lane on board 19 — that review is the trigger, not
this design doc and not round one's code completion. What follows are the candidate
questions to carry into that log:

1. **Correlation/cardinality/completeness.** §"What we know about the wire" lists the
   real event shapes a reassembly layer would need to handle (declared strike, OA
   without `ActionResolved`, non-attack `ActionResolved`, uncorrelated status events, no
   documented universal completion marker — only the actor-only, TakeAction-only
   `TurnStateChanged` incidental signal). Whether that incidental signal is reliable
   enough to build on, whether spectators and OAs need something else, or whether the
   wire should grow an explicit completion contract, is a real question — but it's
   premature until a production reassembler is being built, which is explicitly not
   round one's job. CONTRACT.md should record what the concept actually observes here,
   not assume an answer.
2. **The dispatch layer currently discards the envelope — CONFIRMED, now filed as
   web#582 (§9).** `dispatchEncounterStreamEvent` (`src/api/encounterStreamDispatch.ts`)
   passes only `payload.value` to each callback — `correlation_id`, `sequence`, and
   `timestamp` on `EncounterEvent` are dropped before they reach any combat callback. This
   was a round-one candidate, not a round-one prerequisite — round one played fixtures,
   not the live stream, so nothing here blocked the concept. It became relevant the moment
   a later round (web#581) proposed promoting the concept into the live game: Kirk
   reviewed this exact candidate on 2026-07-22 and confirmed it as a real, scoped need,
   filed as rpg-dnd5e-web#582 (Board 19, Team=Platform). End-to-end verification then
   confirmed a separately scoped API prerequisite, rpg-api#701, for correlated effect
   envelopes (§9). Both issues are required before web#581 can group live attack effects.
   **§9 records the approved contract**; this item's status here is historical (what
   round one observed and logged), not the current source of truth.
3. **Roll-metadata gaps (flag, don't ask yet):**
   - `AttackResolved` already carries `attack_roll`, `attack_bonus`, `target_ac`, `hit`,
     `critical`, `has_advantage/disadvantage` + source refs — enough for the full "20+4
     vs AC 14, ADV from Dodging" story. Good.
   - **Not** carried: raw *damage* dice faces (`EntityDamaged.damage_breakdown` is by
     source — "shortsword:5, sneak_attack:2" — not "2d6→[4,3]"). Not relevant round one
     (d20-only, see Decided below); flag only if a later round wants damage dice to
     tumble too.
   - **Not** carried: the *discarded* die on advantage/disadvantage. Showing two d20s
     with one greyed is a lovely advantage visual but needs the dropped value. Flag
     only, not asked for.
4. **Do NOT ask for two-phase (roll-started / roll-resolved) events.** True two-phase
   would let the die be genuinely unknown mid-tumble, but it (a) doubles event volume on
   a hot path, (b) forces the server to hold the result and emit twice — added latency +
   a "resolved never arrives" failure mode, (c) buys *nothing* in co-op because no player
   has secret information anyway. Theatrical over already-resolved events is the right
   tool. Stated so Kirk can overrule if a future PvP mode ever needs true-secret rolls.

**No Platform implementation issue got filed from this list during round one.** These
were candidates for the CONTRACT.md log, not requests — filing happens only after Kirk
reviews the concept's evidence and confirms a candidate is a real, scoped need (§6 intro).
Item 2 has since cleared that gate as web#582, with the separately verified rpg-api#701
effect-envelope prerequisite (§9); items 1, 3, and 4 remain open candidates, not filed.

## 7. Round-one `/concepts` scope — following the PR #557 pattern

**Goal: feel and placement comparison, not live stream integration.** Round one proves
the beats read right and lets Kirk compare die placements side by side. It does **not**
reassemble a live stream, does not touch reconnect/snapshot behavior, and does not file
any platform request on its own.

Following the fixture-first pattern the equipment concept (rpg-dnd5e-web#557) proved out:

- **Fixture-first, real components over concept data.** The concept renders the actual
  shared presentation components fed by typed fixtures kept beside the concept
  (`fixtures.ts`), plus an **event/intent inspector** panel (mirrors the equipment
  concept's intent log) and a **`CONTRACT.md` gap log** (§6) instead of hard-coded mockup
  data.
- **Event-shaped fixture lab.** Each scenario gets its own **local/concept context**:
  `viewerEntityId` / `role` (`self` | `spectator`), an optional `npcTier` (`grunt` |
  `elite` | `boss`), and a `pace` (`cinematic` | `brisk` | `instant`) — plus an **ordered
  list of envelope-like event fixtures**, each carrying `sequence`, `correlationId`, and
  payload-shaped data matching one of the real event shapes from §"What we know about
  the wire" (including shapes with no `ActionResolved`, no `EntityDamaged`, or no
  correlation at all).
- **The concept adapter may use scenario boundaries — it is explicitly not a production
  reassembler.** It's allowed to know "this scenario's fixture list is one complete
  story" in a way a live-stream reassembler never could (a live reassembler has no
  scenario boundary to lean on — that's exactly the completeness question §6 defers).
  Don't let the adapter's convenience leak into an assumption the real reassembler could
  rely on.
- **Reusable presentation components accept only presentation props.** The beat
  sequencer, the die, the verdict stamp, the damage pop — none of them know about
  streams, correlation ids, fixture types, or game rules. They take beat-shaped props
  and render. This is what makes "promotion = swap the data source" true later.
- **`CONTRACT.md` records evidence and candidate gaps as the concept is built, not
  pre-authored proto asks** (§6) — the equipment concept's `CONTRACT.md` is the model to
  follow: numbered observations, each with a why. Filing anything on Platform happens
  only after Kirk reviews the concept and confirms a candidate is real — not during
  round one.

**Round-one cases** (each an event-shaped fixture per the shapes in §"What we know about
the wire"):

- Player hit, miss, crit, nat-1
- Opportunity attack with `AttackResolved` (+ optional `EntityDamaged`) and **no**
  `ActionResolved`
- NPC grunt swing (Brisk pace)
- Elite/boss swing (Cinematic pace)
- Repeated attacks in one turn (compression, §4)

**Placement comparison:** build both **token-anchored** placement (die over the acting
token, with crit/nat-1 promoting to center-stage) and **pure center-stage** placement
against the *same* fixtures, so Kirk can flip between them and judge feel directly rather
than read a recommendation (§2).

## 8. Round-one success / verification

- Kirk can compare the token-anchored and center-stage placement treatments against
  **identical fixtures**, at a 1024×768 floor and larger, with a non-breaking narrow
  fallback below that floor.
- No input blocking anywhere in the sequence; auto-timeout and tap-to-skip both work; the
  authoritative outcome is always shown, even in Instant mode or if a beat is skipped.
- **Tests:** fake-timer component tests covering beat timing, skip, and auto-timeout;
  fixture tests covering every round-one scenario (§7); reduced-motion checks (§4);
  real-route evidence (before/after renders + a viewed statement, per this repo's
  `docs/evidence/` convention) before calling round one done.
- **What round one explicitly does NOT prove:** live-stream reassembly, reconnect
  behavior, snapshot-flush behavior, or sequence-gap handling. Those are the
  discontinuity invariants stated in §"What we know about the wire" for a future
  reassembler to be built against — round one's fixtures are authored, not streamed, so
  none of that is exercised here.

## 9. Envelope metadata delivery — confirmed Platform prerequisites (rpg-api#701 + web#582)

**Status: approved.** §6 item 2 logged callback-level envelope discard as a round-one
candidate, not a request. Kirk reviewed it on 2026-07-22 and confirmed web#582 as the
client-side contract. End-to-end root-cause verification then confirmed that the API also
drops toolkit-authored correlation/occurrence metadata for effect envelopes, filed
separately as **rpg-api#701**. The two changes are independent, may land in parallel, and
both are required before **rpg-dnd5e-web#581** can promote the beat sequencer into
`EncounterView` and group live attack effects. This section records those production
prerequisites, not #581's reassembler — see "Scope boundary" below.

**Verified current state.** The proto already carries the fields and the toolkit already
authors them. The two delivery seams are distinct:

- `EncounterEvent` (`events_pb.ts`) already declares `sequence: bigint` (`int64`,
  field 1), an optional `timestamp?: Timestamp` (`google.protobuf.Timestamp`, field 2),
  and `correlationId: string` (field 3) at the envelope level — these are not new
  fields.
- `dispatchEncounterStreamEvent` (`src/api/encounterStreamDispatch.ts`) destructures
  `event.event` and invokes each callback with only `payload.value`; the envelope's three
  fields are discarded before any callback runs. This is **web#582's** client-only scope.
- `translateDamageDealtEvent` and `translateConditionAppliedEvent` construct
  `EntityDamaged`/`StatusApplied` envelopes without their embedded toolkit event's
  `CorrelationID()` and stamp the handler clock rather than `OccurredAt()`. This is
  **rpg-api#701's** API-only scope; it projects those existing toolkit values onto the
  already-defined envelope fields.

**Approved contract shape.** An exported `EncounterEventMetadata` type, defined as a
`Pick` off the wire's own `EncounterEvent` fields (`Pick<EncounterEvent, 'sequence' |
'timestamp' | 'correlationId'>` or equivalent) so the metadata type can never drift from
the wire it is picked from. Every `EncounterStreamOptions` callback — every currently
handled event case, not only the three combat ones — changes from `(payload) => void` to
`(payload, metadata: EncounterEventMetadata) => void`. This is additive and
backward-compatible: a caller's existing single-parameter callback keeps compiling and
running unchanged (JavaScript ignores an extra call-time argument a function doesn't
declare), so nothing currently subscribed to the stream breaks the day this lands.

**Values pass through verbatim.** The dispatcher forwards `event.sequence`,
`event.timestamp`, and `event.correlationId` exactly as received alongside
`payload.value` — no numeric coercion of `sequence` (stays `bigint`), no `Date`
conversion of `timestamp` (stays `Timestamp | undefined`), no defaulting an empty
`correlationId` to something else. No conversion, validation, grouping, dedup,
completion-marker synthesis, or game-rule interpretation of any kind — that is
explicitly #581's (or a later reassembler's) job, never this passthrough's.

**Field semantics — verified against `rpg-api`'s translator
(`internal/handlers/dnd5e/v2/encounter/translate.go`), not assumed:**

- **`correlationId` ties an action's cause/effect chain.** `translateActionResolvedEvent`,
  `translateAttackResolvedEvent`, and `translateTurnStateChangedEvent` all set the
  envelope's `CorrelationId` from the toolkit event's own `CorrelationID()` (the shared
  `eventMeta` accessor) — confirming design.md's existing claim that `TurnStateChanged`
  can "incidentally close" a declared strike's group: it shares the *same*
  `correlation_id`, not merely adjacent timing. `TurnStateChanged`'s own doc comment notes
  the field is legitimately **empty** for turn-start refreshes (Invariant 8) — a real,
  intentional value this passthrough must forward as-is, not treat as an error.
  **Confirmed API prerequisite: rpg-api#701.** `DamageDealtEvent` and
  `ConditionAppliedEvent` embed the same `eventMeta` and therefore already expose
  `CorrelationID()` and broker-authored `OccurredAt()`, but their current translators
  (`translateDamageDealtEvent`, `translateConditionAppliedEvent`) do not project either:
  `EntityDamaged`/`StatusApplied` arrive with an empty `correlation_id` and a handler-clock
  timestamp. #701 corrects only that API translation, preserving empty IDs unchanged for
  genuinely uncorrelated events. Without it, #582 correctly forwards metadata but cannot
  make a client tie hit-only damage or status effects to the responsible attack.
- **`sequence` is ordering information, not a global-uniqueness guarantee.** Two
  precedents already in the codebase prove it: `TranslateSnapshot` and
  `BuildReplayEvents` currently stamp `Sequence: 0` on `SnapshotDelivered` and every
  replayed `EntityAppeared`/`GeometryRevealed` sent alongside it — several envelopes,
  one connect, the same `0`. And historically, before rpg-toolkit#765, `rpg-api`
  deliberately stamped a synthesized `InitiativeRolled` envelope with the **same**
  `sequence` as its paired `ModeChanged` envelope ("seq is shared with the paired
  ModeChangedEvent: this envelope is that transition's effect, not an
  independently-caused event of its own" — since retired; `InitiativeRolled` now carries
  its own real sequence number). A consumer may use `sequence` to order events it already
  knows are related; it must never assume a `sequence` value uniquely identifies one
  event.
- **`timestamp` is the toolkit's own occurrence time, not `rpg-api`'s wall clock** — for
  the correlated action/attack events this live feature needs. `translateActionResolvedEvent`'s
  doc comment is explicit: "the event's game-time `OccurredAt` (Invariant 5, **NOT**
  rpg-api's wall clock)". #701 applies that same existing toolkit occurrence time to
  `EntityDamaged`/`StatusApplied`; #582 then forwards the resulting envelope timestamp
  without changing it. Some non-combat translators, e.g. `ModeChanged`, currently stamp
  the handler's own `now` instead — a difference neither prerequisite needs to resolve.

**Why every handled callback gets metadata, not only the three combat events.** A
production consumer needs sequence/correlation/timestamp context around encounter-level
boundaries that are not combat events themselves: `SnapshotDelivered` (the connect/
reconnect sync barrier), `ModeChanged`/`InitiativeRolled` (the FREE_ROAM→TURN_BASED
transition), and `TurnStarted`/`TurnEnded`/`TurnStateChanged` (turn boundaries). A
discontinuity at any of these is exactly what this design's own "flush pending theater
immediately" invariant (§"What we know about the wire", Discontinuities) needs to
detect — scoping metadata to only `ActionResolved`/`AttackResolved`/`EntityDamaged` would
leave a reassembler blind to the boundary events it must react to. One uniform
`(payload, metadata)` shape for every case is simpler to implement, test, and reason
about than a special-cased subset, and it costs nothing extra per call.

**The invariant does not move.** This is a passthrough, not new behavior: authoritative
HP, economy, and turn state continues to apply immediately on receipt, exactly as stated
above and unchanged by #582. The change is what the callback signature *carries*, never
when or how state updates — no new delay, no buffering, no reordering.

**TDD / acceptance shape for web#582 (matches its own "Add dispatch tests" bar):**

- A parameterized test across every currently handled `EncounterEvent` case asserting
  each registered callback receives the exact `payload.value` it receives today *plus*
  the exact `{ sequence, timestamp, correlationId }` the fixture envelope carries — one
  assertion shape, run over the full case list, not one bespoke test per case.
- Explicit, named-case tests for `ActionResolved`, `AttackResolved`, and `EntityDamaged`
  — the callback-level combat metadata #582 exposes — each proving `metadata` is present
  and correct alongside the unchanged payload.
- An explicit empty-string `correlationId` case (the real, legitimate turn-start-refresh
  value above) and an explicit `undefined` `timestamp` case (the wire's own optional-field
  shape) both passed through unmodified — not defaulted, not coerced, not dropped.
- Run the targeted file first — `npm run test:run -- src/api/encounterStreamDispatch.test.ts`
  — then the full `npm run ci-check` before any PR, matching this repo's own non-negotiable
  gate (plan.md's Global Constraints).

**Dependency and scope boundary.** #582 is a client-only passthrough: it changes typed
web callbacks to carry envelope metadata and remains independently landable; it contains
no `rpg-api` code. #701 is an API-only translator correction: it projects toolkit-authored
`CorrelationID()`/`OccurredAt()` onto `EntityDamaged`/`StatusApplied` envelopes and contains
no web callback code. Neither issue implements reassembly, completion, buffering, or
live-route presentation. They may be implemented and merged in parallel, but **#581
requires both** before live grouping: #701 supplies correct effect-envelope metadata and
#582 delivers it to the typed callback boundary. Round one's `/concepts` bench (§7) remains
fixture-driven and unaffected by either landing. §6 item 1 (correlation/cardinality/
completeness for a real reassembler) remains open and undecided — the two prerequisites
deliver raw fields; neither decides how #581 groups or completes them.

## Decided this revision

- **Click-to-roll is the default** for your own turn, with a short auto-timeout so a
  player can never stall the table (§2).
- **NPC pacing tiers:** grunts Brisk, elites/bosses Cinematic (§4).
- **Fumble tone:** comedic, non-punitive (§1).
- **Damage dice:** d20-only for round one; damage dice do not tumble (§5, §6.3).
- **Audio:** hooks only round one — every beat reserves an SFX slot, but no sound ships
  (§5).
- **Initiative hand-off:** the fuller tracker-slide treatment is future work, outside
  round one; Release (Beat 5) is a short generic breath, not that animation (§1, §3).

## Still open — what round one's prototype resolves

- **Die's home:** token-anchored (with crit/nat-1 center promotion) vs pure center-stage.
  Round one builds both against identical fixtures so Kirk picks by looking, not by
  reading a recommendation (§2, §7, §8).

## Deferred beyond round one

- Correlation/cardinality/completeness contract work for a real reassembler (§6 item 1)
  — still open, still undecided; unlike §6 item 2 (§9, web#582), this candidate has not
  cleared Kirk's review gate. Any resulting Platform issue is gated on that review.
- Live-stream reassembly, reconnect/snapshot-flush behavior (§"What we know about the
  wire", §8) — rpg-api#701 and web#582 (§9) together deliver the raw effect-envelope and
  callback metadata a reassembler would need; neither builds the reassembler.
- Damage-dice tumble, discarded-advantage-die visual (§6.3).
- 3D physics dice (option C) and any Assets-implemented new art (§2, §5).
- The fuller initiative-tracker hand-off animation (§1, §3).

## Pointers

- Wire contract: `rpg-api-protos` `dnd5e/api/v1alpha2/encounter/events.proto`
  (`EncounterEvent` envelope, `ActionResolved`, `AttackResolved`, `EntityDamaged`,
  `StatusApplied`) · web `src/api/encounterStreamDispatch.ts` (the envelope-discarding
  dispatch, confirmed and filed as web#582 — §9) · `rpg-api`
  `internal/handlers/dnd5e/v2/encounter/translate.go` (the envelope-field source of truth
  §9 verifies against: `Sequence`/`Timestamp`/`CorrelationId` per translator)
- Pattern precedent: `src/concepts/equipment/` (rpg-dnd5e-web#557) — fixture-first
  components, intent/event inspector, `CONTRACT.md` gap-log convention this round
  follows
- Render targets: web `src/components/game/{EncounterView,CombatLog,EncounterMap}.tsx`,
  `src/hooks/useCombatLog.ts`
- FX assets: `rpg-game-assets/harness/models/synty/ui/library/` — `fx/` (Sheen, Glow01–03,
  Beams01, DamageDirection*, Sparkle01), plus `banners/`, `frames/` for verdict stamps
- Prototype home: web `/concepts` route (`src/concepts/combat-pacing/`), fixture-first
  per `docs/how-to/concepts-route.md`
- Tracking: rpg-dnd5e-web#561 (Board 19, Feature=Game Screen, Team=UI/UX). Broader
  charter: rpg-dnd5e-web#525 (Game-UX). North-star: `ideas/encounter/v1alpha2/design.md`.
  Live promotion: rpg-dnd5e-web#581 (requires both rpg-api#701 and rpg-dnd5e-web#582).
  Platform prerequisites: rpg-api#701 (effect-envelope projection) and rpg-dnd5e-web#582
  (callback metadata passthrough; §9).
</content>
