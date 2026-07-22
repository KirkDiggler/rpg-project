# Combat Pacing & Dice — Round-One `/concepts` Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **Plan location note:** this plan lives beside the approved design at
> `ideas/combat-pacing/design.md` in `rpg-project` (matching the
> `ideas/equipment/item-icons/plan.md` precedent for this workspace's
> Cross-Repo Design Workflow, `rpg-project/AGENTS.md` §"Cross-Repo Design
> Workflow" — design approved first, plan added to the same PR after).
> Implementation itself happens in **`rpg-dnd5e-web`**, on its own branch,
> against **rpg-dnd5e-web#561** (Board 19, Feature=Game Screen, Team=UI/UX).

**Goal:** Build the round-one `/concepts` bench for combat pacing
(rpg-dnd5e-web#561): a fixture-driven beat sequencer (Cue → Throw →
Verdict → Impact → Release) that choreographs already-resolved
`AttackResolved`/`EntityDamaged`/`ActionResolved` events into a
theatrical reveal, rendered in **both** token-anchored and center-stage
die placements against **identical fixtures** so Kirk can compare them
side by side, plus an event/intent inspector and a `CONTRACT.md` gap log
— following the equipment concept's fixture-first pattern
(rpg-dnd5e-web#557). Planning only: no code changes to `rpg-api`,
`rpg-toolkit`, `rpg-api-protos`, or the live encounter route.

**Architecture:** Three layers, same separation the equipment concept
proved out. (1) `fixtures.ts` — pure data: wire-shaped `*Like` event
types (field-for-field against the real `AttackResolved`/`EntityDamaged`/
`ActionResolved` proto messages) plus the 8 round-one scenarios and a
pure `groupByCorrelation` helper. (2) `useBeatSequencer.ts` — a pure
timing state machine (no rendering) driving one scenario's correlation
groups through the five named beats plus the two non-timed states
(`armed` waiting for a throw, `done`), with pace-derived durations, crit
stretch, auto-timeout, tap-to-skip, and repeat-roll compression. (3)
`BeatStage.tsx` — a presentational component that takes only
beat-shaped props (beat name, attack, damage, placement, reduced-motion
flag) and renders the die/verdict/damage — it is never told about
fixtures, correlation ids, or scenarios. `CombatPacingConcept.tsx` wires
all three plus a scenario/pace/reduced-motion/viewport-frame switcher and
an event/intent inspector, registered into `ConceptsView.tsx`.

**Tech Stack:** TypeScript 5.8 (strict mode), React 19 function
components + hooks, Vitest + `@testing-library/react` with
`vi.useFakeTimers()` for beat-timing tests, no new runtime dependencies
(no animation library — `BeatStage` uses plain CSS classes toggled by
`data-*` attributes, matching this repo's existing inline-style
convention in `src/concepts/equipment/` and `src/concepts/combat-panel/`).

## Global Constraints

- `npm run ci-check` MUST pass before every push (format, lint,
  typecheck, build, tests) — non-negotiable per `rpg-dnd5e-web/CLAUDE.md`.
- Never use `git commit --no-verify` or `git push --no-verify`.
- **Scope is round-one `/concepts` only.** No changes to `rpg-api`,
  `rpg-toolkit`, `rpg-api-protos`, `src/api/encounterStreamDispatch.ts`,
  `src/api/useEncounterStream.ts`, or any live-encounter component
  (`EncounterView.tsx`, `EncounterMap.tsx`, `CombatLog.tsx`,
  `useCombatLog.ts`). Round one is fixture-first and never touches the
  live stream (design.md §"What we know about the wire", §7).
- **No audio implementation.** Every beat may reserve an SFX-slot comment
  in `BeatStage.tsx`, but no sound library, asset, or playback code ships
  (design.md §5, "Decided this revision").
- **No damage-dice tumble.** `AttackResolvedLike` is the only die that
  animates; `EntityDamagedLike.amount` is a display number, never an
  animated die face (design.md "Decided this revision": d20-only).
  the wire's real `EntityDamaged.damage_breakdown` gap stays a CONTRACT.md
  flag, not something this round implements.
- **No handoff/promotion.** This plan does not wire `CombatPacingConcept`
  into `EncounterView` or any production path — it stays under
  `/concepts` (design.md §7, "does not file any platform request on its
  own").
- **`CONTRACT.md` records evidence/candidate gaps only** — it must never
  read as a pre-authored Platform feature request (design.md §6, PR #557
  lifecycle). No issue gets filed on Platform's board lane by this plan.
- **Visual verification is mandatory before calling round one done**: the
  token-anchored vs. center-stage comparison must be checked against
  identical fixtures at a 1024×768 floor, at least one larger frame, and
  a non-breaking narrow fallback below the floor (design.md §8).
- One issue per PR, branch from latest `origin/main`, no direct commits
  to `main` — this is player-facing-adjacent web work (`rpg-project`'s
  board rules, `AGENTS.md` §"Project Board").
- Any GitHub issue/PR comment this work produces ends with
  `— asset-pipeline agent, on behalf of KirkDiggler` (this workspace's
  convention, `game-dev/CLAUDE.md`).

## File Map

| File | Responsibility |
| --- | --- |
| `src/concepts/combat-pacing/fixtures.ts` (new) | Wire-shaped `*Like` types (`ActionResolvedLike`, `AttackResolvedLike`, `EntityDamagedLike`), `PacingFixtureEvent`, `CombatPacingScenario`, `Pace`, `groupByCorrelation`, `BeatGroupResult`, and the `SCENARIOS` array (8 round-one cases from design.md §7). |
| `src/concepts/combat-pacing/fixtures.test.ts` (new) | Scenario-count/shape assertions: exactly 8 scenarios, the OA scenario has no `actionResolved` event, the nat-1/crit scenarios carry the right `AttackResolved` flags, the repeated-attacks scenario has 2 correlation groups, every scenario's `sequence` values are strictly increasing, `groupByCorrelation` groups and orders correctly. |
| `src/concepts/combat-pacing/useBeatSequencer.ts` (new) | Pure timing state machine: `BeatName`, `BeatDurations`, `CINEMATIC`/`BRISK` constants, `CRIT_VERDICT_EXTRA_MS`/`CRIT_IMPACT_EXTRA_MS`, `AUTO_THROW_TIMEOUT_MS`, `REDUCED_MOTION_THROW_MS`, and the `useBeatSequencer` hook (`throwDie`, `skip`, per-group compression, crit stretch, reduced-motion Throw collapse). |
| `src/concepts/combat-pacing/useBeatSequencer.test.ts` (new) | Fake-timer tests: full cinematic-hit budget (1450ms), miss skips Impact, crit stretches to exactly 2500ms, Brisk/NPC-grunt budget, second correlation group compresses to Brisk regardless of scenario pace, `armed` waits for `throwDie()` or `AUTO_THROW_TIMEOUT_MS`, `skip()` from `cue`/`armed`/`throw` jumps to `verdict`, `skip()` from `verdict`/`impact`/`release` finishes the group, `instant` pace goes straight to `done`, reduced motion collapses Throw to `REDUCED_MOTION_THROW_MS`. |
| `src/concepts/combat-pacing/BeatStage.tsx` (new) | Presentational component: `Placement` (`token-anchored` \| `center-stage`), `BeatStageProps` (`beat`, `placement`, `attack?`, `damage?`, `reducedMotion`), token-anchored-promotes-to-center-stage-on-crit/nat-1 logic, verdict label derivation. Beat-shaped props only — no fixture/scenario/correlation knowledge. |
| `src/concepts/combat-pacing/BeatStage.test.tsx` (new) | Render tests per beat (`cue`/`throw`/`armed`/`verdict`/`impact`/`release`), placement promotion on crit and on nat-1, reduced-motion class swap, verdict label for hit/miss/crit/nat-1. |
| `src/concepts/combat-pacing/CombatPacingConcept.tsx` (new) | Top-level concept page: scenario switcher (8 buttons), pace-override selector (`default`/`cinematic`/`brisk`/`instant`), reduced-motion toggle, viewport-frame selector (`narrow`/`floor`/`typical`/`full`, mirroring `combat-panel/CombatPanelConcept.tsx`'s `FRAMES` pattern), side-by-side `BeatStage` pair (token-anchored + center-stage sharing one `useBeatSequencer` instance), throw-die button (visible only while `armed`), skip button, and an event/intent inspector panel printing each fixture event's envelope fields (`sequence`, `correlationId`, `case`). |
| `src/concepts/combat-pacing/CombatPacingConcept.test.tsx` (new) | Fake-timer interaction tests: switching scenarios resets the sequencer, pace override drives `useBeatSequencer` with the overridden pace, clicking throw-die during `armed` advances immediately, letting the timeout elapse without clicking advances automatically, skip button jumps beats, both `beat-stage` elements render simultaneously with the correct `data-placement`. |
| `src/concepts/combat-pacing/CONTRACT.md` (new) | Gap log per design.md §6, following `src/concepts/equipment/CONTRACT.md`'s exact structure (numbered observations, each with a why, explicit "not asks yet" framing). |
| `src/concepts/ConceptsView.tsx` (modify) | Add `'combat-pacing'` to `ConceptPage`, add its `CONCEPT_PAGES` entry, import `CombatPacingConcept`, add its render branch. |
| `docs/how-to/concepts-route.md` (modify) | Add a `combat-pacing/` row to the "Current concepts" table. |
| `docs/architecture/components/concepts-route.md` (modify) | Add a `src/concepts/combat-pacing/` paragraph under "Current contents". |
| `docs/evidence/` (new) | Real-route (dev-server) before/after-style screenshots from the mandatory visual-verification task, plus a short write-up markdown file following the `wall-fittings-536.md` convention. |

---
## Task 1: Fixtures — wire-shaped event types + the 8 round-one scenarios

**Files:**
- Create: `src/concepts/combat-pacing/fixtures.test.ts`
- Create: `src/concepts/combat-pacing/fixtures.ts`

**Interfaces:**
- Consumes: `RefLike` from
  `../../components/game/equipment/equipmentTypes` (already exported,
  unchanged) — reused for `Ref`-shaped fields (`damageType`, `actionRef`,
  `advantageSources`, `disadvantageSources`) exactly as the real proto
  does, per design.md's "typed fixtures... matching the wire we want".
- Produces (consumed by Tasks 2-4): `Pace`, `ActionResolvedLike`,
  `AttackResolvedLike`, `EntityDamagedLike`, `HitPointsLike`,
  `PacingFixtureEvent`, `CombatPacingScenario`, `BeatGroupResult`,
  `groupByCorrelation(events: PacingFixtureEvent[]): BeatGroupResult[]`,
  `SCENARIOS: CombatPacingScenario[]` (length 8).

- [ ] **Step 1: Set up the isolated workspace (folded into this task — no standalone setup task)**

Create a project-local worktree from latest `origin/main`, tied to
rpg-dnd5e-web#561:

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin
git worktree add .worktrees/561-combat-pacing -b feat/561-combat-pacing-concept origin/main
cd .worktrees/561-combat-pacing
npm install
```

Expected: `npm install` completes without error; `git status` reports
`On branch feat/561-combat-pacing-concept`, clean.

Verify the baseline is green before writing anything:

```bash
npm run test:run
```

Expected: the full existing suite passes (0 failures) — the clean
baseline the rest of this plan's steps diff against.

- [ ] **Step 2: Write the failing test file**

Create `src/concepts/combat-pacing/fixtures.test.ts`:

```ts
import { describe, expect, it } from 'vitest';
import { groupByCorrelation, SCENARIOS } from './fixtures';

describe('SCENARIOS (design.md §7 round-one cases)', () => {
  it('has exactly 8 round-one scenarios', () => {
    expect(SCENARIOS).toHaveLength(8);
  });

  it('every scenario has a unique id', () => {
    const ids = SCENARIOS.map((s) => s.id);
    expect(new Set(ids).size).toBe(ids.length);
  });

  it('every scenario has at least one event with strictly increasing sequence numbers', () => {
    for (const s of SCENARIOS) {
      expect(s.events.length).toBeGreaterThan(0);
      for (let i = 1; i < s.events.length; i++) {
        expect(
          s.events[i].sequence,
          `${s.id}: sequence must strictly increase`
        ).toBeGreaterThan(s.events[i - 1].sequence);
      }
    }
  });

  it('player-hit resolves as a plain hit, no crit/nat-1', () => {
    const s = SCENARIOS.find((x) => x.id === 'player-hit')!;
    const attack = s.events.find((e) => e.case === 'attackResolved')!.value;
    expect(attack).toMatchObject({ hit: true, critical: false });
  });

  it('player-miss has no entityDamaged event (EntityDamaged fires hit-only)', () => {
    const s = SCENARIOS.find((x) => x.id === 'player-miss')!;
    expect(s.events.some((e) => e.case === 'entityDamaged')).toBe(false);
    const attack = s.events.find((e) => e.case === 'attackResolved')!.value;
    expect(attack).toMatchObject({ hit: false, critical: false });
  });

  it('player-crit carries critical: true', () => {
    const s = SCENARIOS.find((x) => x.id === 'player-crit')!;
    const attack = s.events.find((e) => e.case === 'attackResolved')!.value;
    expect(attack).toMatchObject({ hit: true, critical: true });
  });

  it('player-nat1 is attackRoll 1, a miss, not critical (design.md §1: nat-1 !== crit)', () => {
    const s = SCENARIOS.find((x) => x.id === 'player-nat1')!;
    const attack = s.events.find((e) => e.case === 'attackResolved')!.value;
    expect(attack).toMatchObject({ attackRoll: 1, hit: false, critical: false });
  });

  it('opportunity-attack has AttackResolved with NO ActionResolved (design.md §"What we know about the wire")', () => {
    const s = SCENARIOS.find((x) => x.id === 'opportunity-attack')!;
    expect(s.events.some((e) => e.case === 'actionResolved')).toBe(false);
    expect(s.events.some((e) => e.case === 'attackResolved')).toBe(true);
  });

  it('npc-grunt-swing is tiered Brisk (design.md §4: grunts get Brisk)', () => {
    const s = SCENARIOS.find((x) => x.id === 'npc-grunt-swing')!;
    expect(s.pace).toBe('brisk');
    expect(s.npcTier).toBe('grunt');
  });

  it('npc-boss-swing is tiered Cinematic and crits (design.md §4: boss crit lands like a player\'s)', () => {
    const s = SCENARIOS.find((x) => x.id === 'npc-boss-swing')!;
    expect(s.pace).toBe('cinematic');
    expect(s.npcTier).toBe('boss');
    const attack = s.events.find((e) => e.case === 'attackResolved')!.value;
    expect(attack.critical).toBe(true);
  });

  it('repeated-attacks has exactly 2 distinct correlation ids (design.md §4: compression after the first)', () => {
    const s = SCENARIOS.find((x) => x.id === 'repeated-attacks')!;
    const ids = new Set(s.events.map((e) => e.correlationId));
    expect(ids.size).toBe(2);
  });
});

describe('groupByCorrelation', () => {
  it('groups events by correlationId, preserving first-seen order', () => {
    const s = SCENARIOS.find((x) => x.id === 'repeated-attacks')!;
    const groups = groupByCorrelation(s.events);
    expect(groups).toHaveLength(2);
    expect(groups[0].correlationId).not.toBe(groups[1].correlationId);
  });

  it('collects action/attack/damage onto the same group by shared correlationId', () => {
    const s = SCENARIOS.find((x) => x.id === 'player-hit')!;
    const groups = groupByCorrelation(s.events);
    expect(groups).toHaveLength(1);
    expect(groups[0].action).toBeDefined();
    expect(groups[0].attack).toBeDefined();
    expect(groups[0].damage).toBeDefined();
  });

  it('a group with no ActionResolved still groups its AttackResolved (opportunity-attack)', () => {
    const s = SCENARIOS.find((x) => x.id === 'opportunity-attack')!;
    const groups = groupByCorrelation(s.events);
    expect(groups).toHaveLength(1);
    expect(groups[0].action).toBeUndefined();
    expect(groups[0].attack).toBeDefined();
  });
});
```

- [ ] **Step 3: Run the test to verify it fails**

Run: `npm run test:run -- src/concepts/combat-pacing/fixtures.test.ts`

Expected: FAIL — Vitest cannot resolve the import; e.g. `Error: Failed to
resolve import "./fixtures" from
"src/concepts/combat-pacing/fixtures.test.ts". Does the file exist?`

- [ ] **Step 4: Implement the fixture types + scenario data**

Create `src/concepts/combat-pacing/fixtures.ts`:

```ts
/**
 * Combat-pacing fixtures (rpg-dnd5e-web#561) — the outside-in contract
 * bench for the attack-loop beat sequencer, same method as the equipment
 * concept's fixtures.ts (rpg-dnd5e-web#557/#531).
 *
 * `ActionResolvedLike`/`AttackResolvedLike`/`EntityDamagedLike` match the
 * generated `dnd5e.api.v1alpha2.encounter` proto messages field-for-field
 * (same names, same camelCase shapes) rather than importing the generated
 * classes directly — same rationale as equipmentTypes.ts: `/concepts` can
 * keep feeding plain fixture data without constructing real proto
 * messages. `PacingFixtureEvent` additionally carries `sequence` and
 * `correlationId`, mirroring the real `EncounterEvent` envelope
 * (events_pb.ts) and its `event: { value, case }` oneof shape — so a
 * fixture reads exactly like a captured wire event, per design.md §7's
 * "event-shaped fixture lab".
 *
 * Round-one cases (design.md §7): player hit/miss/crit/nat-1, an
 * opportunity attack with no ActionResolved, an NPC grunt swing (Brisk),
 * an elite/boss swing (Cinematic), and repeated attacks in one turn
 * (compression). `groupByCorrelation` is the concept adapter's one piece
 * of logic — explicitly a scenario-boundary convenience, not a
 * production reassembler (design.md §7's own caveat).
 */

import type { RefLike } from '../../components/game/equipment/equipmentTypes';

export type Pace = 'cinematic' | 'brisk' | 'instant';

/** Matches ActionResolved (events_pb.ts) field-for-field. */
export interface ActionResolvedLike {
  actorEntityId: string;
  actionRef: RefLike;
  targetEntityId: string;
}

/** Matches AttackResolved (events_pb.ts) field-for-field. */
export interface AttackResolvedLike {
  attackerEntityId: string;
  targetEntityId: string;
  hit: boolean;
  critical: boolean;
  attackRoll: number;
  attackBonus: number;
  targetAc: number;
  hasAdvantage: boolean;
  advantageSources: RefLike[];
  hasDisadvantage: boolean;
  disadvantageSources: RefLike[];
}

/** Matches HitPoints (types_pb.ts) field-for-field; `temp` omitted from
 * every fixture below (optional, unused round one). */
export interface HitPointsLike {
  current: number;
  max: number;
  temp?: number;
}

/** Matches EntityDamaged (events_pb.ts) field-for-field; `sourceEntityId`/
 * `damageBreakdown` omitted from fixtures below — not needed for the beat
 * sequencer's display (it renders `amount` + `hpAfter` only). */
export interface EntityDamagedLike {
  entityId: string;
  amount: number;
  damageType: RefLike;
  hpAfter: HitPointsLike;
}

/** One envelope-like fixture event — `sequence`/`correlationId` mirror
 * the real `EncounterEvent` envelope; `case`/`value` mirrors its `event`
 * oneof discriminant naming exactly. */
export type PacingFixtureEvent =
  | {
      sequence: number;
      correlationId: string;
      case: 'actionResolved';
      value: ActionResolvedLike;
    }
  | {
      sequence: number;
      correlationId: string;
      case: 'attackResolved';
      value: AttackResolvedLike;
    }
  | {
      sequence: number;
      correlationId: string;
      case: 'entityDamaged';
      value: EntityDamagedLike;
    };

export interface CombatPacingScenario {
  id: string;
  label: string;
  description: string;
  viewerEntityId: string;
  role: 'self' | 'spectator';
  npcTier?: 'grunt' | 'elite' | 'boss';
  pace: Pace;
  events: PacingFixtureEvent[];
}

const slashing: RefLike = { module: 'dnd5e', type: 'damage', id: 'slashing' };
const attackRef: RefLike = {
  module: 'dnd5e',
  type: 'combat_abilities',
  id: 'attack',
};

const PLAYER = 'char-aldric';
const GOBLIN = 'npc-goblin-1';

const action = (
  sequence: number,
  correlationId: string,
  targetEntityId: string
): PacingFixtureEvent => ({
  sequence,
  correlationId,
  case: 'actionResolved',
  value: { actorEntityId: PLAYER, actionRef: attackRef, targetEntityId },
});

const npcAttack = (
  sequence: number,
  correlationId: string,
  attackerEntityId: string,
  targetEntityId: string,
  attack: Omit<AttackResolvedLike, 'attackerEntityId' | 'targetEntityId'>
): PacingFixtureEvent => ({
  sequence,
  correlationId,
  case: 'attackResolved',
  value: { attackerEntityId, targetEntityId, ...attack },
});

const attack = (
  sequence: number,
  correlationId: string,
  targetEntityId: string,
  a: Omit<AttackResolvedLike, 'attackerEntityId' | 'targetEntityId'>
): PacingFixtureEvent =>
  npcAttack(sequence, correlationId, PLAYER, targetEntityId, a);

const damage = (
  sequence: number,
  correlationId: string,
  entityId: string,
  amount: number,
  hpAfter: HitPointsLike
): PacingFixtureEvent => ({
  sequence,
  correlationId,
  case: 'entityDamaged',
  value: { entityId, amount, damageType: slashing, hpAfter },
});

const NO_ADV = { hasAdvantage: false, advantageSources: [] as RefLike[] };
const NO_DISADV = {
  hasDisadvantage: false,
  disadvantageSources: [] as RefLike[],
};

const playerHit: CombatPacingScenario = {
  id: 'player-hit',
  label: 'Player — hit',
  description: 'Declared strike, routine hit (roll 14+5=19 vs AC 16).',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    action(1, 'corr-hit', GOBLIN),
    attack(2, 'corr-hit', GOBLIN, {
      hit: true,
      critical: false,
      attackRoll: 14,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
    damage(3, 'corr-hit', GOBLIN, 7, { current: 8, max: 15 }),
  ],
};

const playerMiss: CombatPacingScenario = {
  id: 'player-miss',
  label: 'Player — miss',
  description:
    'Declared strike, routine miss (roll 8+5=13 vs AC 16) — no EntityDamaged at all.',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    action(1, 'corr-miss', GOBLIN),
    attack(2, 'corr-miss', GOBLIN, {
      hit: false,
      critical: false,
      attackRoll: 8,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
  ],
};

const playerCrit: CombatPacingScenario = {
  id: 'player-crit',
  label: 'Player — crit',
  description: 'Declared strike, critical hit (roll 20+5=25 vs AC 16).',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    action(1, 'corr-crit', GOBLIN),
    attack(2, 'corr-crit', GOBLIN, {
      hit: true,
      critical: true,
      attackRoll: 20,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
    damage(3, 'corr-crit', GOBLIN, 14, { current: 1, max: 15 }),
  ],
};

const playerNat1: CombatPacingScenario = {
  id: 'player-nat1',
  label: 'Player — nat-1',
  description:
    'Declared strike, natural 1 (roll 1+5=6 vs AC 16) — playful, non-punitive, NOT critical.',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    action(1, 'corr-nat1', GOBLIN),
    attack(2, 'corr-nat1', GOBLIN, {
      hit: false,
      critical: false,
      attackRoll: 1,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
  ],
};

const opportunityAttack: CombatPacingScenario = {
  id: 'opportunity-attack',
  label: 'Opportunity attack (no ActionResolved)',
  description:
    'NPC opportunity attack triggered by movement: AttackResolved + EntityDamaged, no ActionResolved — the toolkit resolves NPC OAs inline (design.md §"What we know about the wire").',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    npcAttack(1, 'corr-oa', 'npc-goblin-2', PLAYER, {
      hit: true,
      critical: false,
      attackRoll: 16,
      attackBonus: 4,
      targetAc: 14,
      ...NO_ADV,
      ...NO_DISADV,
    }),
    damage(2, 'corr-oa', PLAYER, 5, { current: 20, max: 25 }),
  ],
};

const npcGruntSwing: CombatPacingScenario = {
  id: 'npc-grunt-swing',
  label: 'NPC grunt swing (Brisk)',
  description:
    'A goblin grunt attacks and misses — tiered Brisk so four goblins acting doesn\'t drag (design.md §4).',
  viewerEntityId: PLAYER,
  role: 'spectator',
  npcTier: 'grunt',
  pace: 'brisk',
  events: [
    action(1, 'corr-grunt', PLAYER),
    attack(2, 'corr-grunt', PLAYER, {
      hit: false,
      critical: false,
      attackRoll: 11,
      attackBonus: 3,
      targetAc: 15,
      ...NO_ADV,
      ...NO_DISADV,
    }),
  ],
};

const npcBossSwing: CombatPacingScenario = {
  id: 'npc-boss-swing',
  label: 'Elite/boss swing (Cinematic)',
  description:
    "A troll boss lands a crit — full Cinematic ceremony, the boss's crit should land like a player's (design.md §4).",
  viewerEntityId: PLAYER,
  role: 'spectator',
  npcTier: 'boss',
  pace: 'cinematic',
  events: [
    npcAttack(1, 'corr-boss', 'npc-troll-boss', PLAYER, {
      hit: true,
      critical: true,
      attackRoll: 19,
      attackBonus: 7,
      targetAc: 17,
      ...NO_ADV,
      ...NO_DISADV,
    }),
    damage(2, 'corr-boss', PLAYER, 22, { current: 3, max: 25 }),
  ],
};

const repeatedAttacks: CombatPacingScenario = {
  id: 'repeated-attacks',
  label: 'Repeated attacks (compression)',
  description:
    'Extra Attack: two swings in one turn — the first gets full ceremony, the second compresses (design.md §4).',
  viewerEntityId: PLAYER,
  role: 'self',
  pace: 'cinematic',
  events: [
    action(1, 'corr-rep-1', GOBLIN),
    attack(2, 'corr-rep-1', GOBLIN, {
      hit: true,
      critical: false,
      attackRoll: 15,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
    damage(3, 'corr-rep-1', GOBLIN, 6, { current: 9, max: 15 }),
    action(4, 'corr-rep-2', GOBLIN),
    attack(5, 'corr-rep-2', GOBLIN, {
      hit: false,
      critical: false,
      attackRoll: 9,
      attackBonus: 5,
      targetAc: 16,
      ...NO_ADV,
      ...NO_DISADV,
    }),
  ],
};

export const SCENARIOS: CombatPacingScenario[] = [
  playerHit,
  playerMiss,
  playerCrit,
  playerNat1,
  opportunityAttack,
  npcGruntSwing,
  npcBossSwing,
  repeatedAttacks,
];

export interface BeatGroupResult {
  correlationId: string;
  action?: ActionResolvedLike;
  attack?: AttackResolvedLike;
  damage?: EntityDamagedLike;
}

/** Splits a scenario's ordered fixture events into correlation groups,
 * preserving first-seen order. This is the concept adapter's one piece of
 * logic — explicitly a scenario-boundary convenience (it can assume "this
 * fixture list is one complete story"), never a production reassembler
 * (design.md §7's own caveat: a live reassembler has no such boundary to
 * lean on). */
export function groupByCorrelation(
  events: PacingFixtureEvent[]
): BeatGroupResult[] {
  const order: string[] = [];
  const byId = new Map<string, BeatGroupResult>();
  for (const e of events) {
    if (!byId.has(e.correlationId)) {
      byId.set(e.correlationId, { correlationId: e.correlationId });
      order.push(e.correlationId);
    }
    const group = byId.get(e.correlationId)!;
    if (e.case === 'actionResolved') group.action = e.value;
    if (e.case === 'attackResolved') group.attack = e.value;
    if (e.case === 'entityDamaged') group.damage = e.value;
  }
  return order.map((id) => byId.get(id)!);
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `npm run test:run -- src/concepts/combat-pacing/fixtures.test.ts`

Expected: PASS — `Tests  14 passed (14)`.

- [ ] **Step 6: Typecheck**

Run: `npm run typecheck`

Expected: no errors.

- [ ] **Step 7: Commit**

```bash
git add src/concepts/combat-pacing/fixtures.ts src/concepts/combat-pacing/fixtures.test.ts
git commit -m "feat(concepts): combat-pacing round-one fixtures (#561)"
```

---
## Task 2: `useBeatSequencer` — the beat-timing state machine

**Files:**
- Create: `src/concepts/combat-pacing/useBeatSequencer.test.ts`
- Create: `src/concepts/combat-pacing/useBeatSequencer.ts`

**Interfaces:**
- Consumes: `CombatPacingScenario`, `PacingFixtureEvent`, `BeatGroupResult`,
  `groupByCorrelation` from `./fixtures` (Task 1).
- Produces (consumed by Task 4): `BeatName`, `BeatDurations`, `CINEMATIC`,
  `BRISK`, `CRIT_VERDICT_EXTRA_MS`, `CRIT_IMPACT_EXTRA_MS`,
  `AUTO_THROW_TIMEOUT_MS`, `REDUCED_MOTION_THROW_MS`,
  `UseBeatSequencerOptions`, `BeatSequencerState`, and the
  `useBeatSequencer(scenario, options?)` hook itself.

- [ ] **Step 1: Write the failing test file**

Create `src/concepts/combat-pacing/useBeatSequencer.test.ts`:

```ts
import { act, renderHook } from '@testing-library/react';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { SCENARIOS } from './fixtures';
import {
  AUTO_THROW_TIMEOUT_MS,
  REDUCED_MOTION_THROW_MS,
  useBeatSequencer,
} from './useBeatSequencer';

const scenario = (id: string) => SCENARIOS.find((s) => s.id === id)!;

beforeEach(() => {
  vi.useFakeTimers();
});

afterEach(() => {
  vi.useRealTimers();
});

describe('useBeatSequencer', () => {
  it('a cinematic hit runs cue -> throw -> verdict -> impact -> release -> done over exactly 1450ms', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-hit')));
    expect(result.current.beat).toBe('cue');

    act(() => {
      vi.advanceTimersByTime(150);
    });
    expect(result.current.beat).toBe('armed'); // role: 'self' waits for a throw

    act(() => {
      result.current.throwDie();
    });
    expect(result.current.beat).toBe('throw');

    act(() => {
      vi.advanceTimersByTime(600);
    });
    expect(result.current.beat).toBe('verdict');

    act(() => {
      vi.advanceTimersByTime(200);
    });
    expect(result.current.beat).toBe('impact');

    act(() => {
      vi.advanceTimersByTime(300);
    });
    expect(result.current.beat).toBe('release');

    act(() => {
      vi.advanceTimersByTime(200);
    });
    expect(result.current.beat).toBe('done');
  });

  it('a cinematic miss skips impact entirely', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-miss')));
    act(() => {
      vi.advanceTimersByTime(150);
      result.current.throwDie();
      vi.advanceTimersByTime(600);
    });
    expect(result.current.beat).toBe('verdict');
    act(() => {
      vi.advanceTimersByTime(200);
    });
    expect(result.current.beat).toBe('release'); // NOT 'impact'
  });

  it('a cinematic crit stretches verdict + impact to a total budget of exactly 2500ms (design.md §1)', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-crit')));
    act(() => {
      vi.advanceTimersByTime(150); // cue
      result.current.throwDie();
      vi.advanceTimersByTime(600); // throw
    });
    expect(result.current.beat).toBe('verdict');
    act(() => {
      vi.advanceTimersByTime(1199); // verdict is 200 + 1000 = 1200ms
    });
    expect(result.current.beat).toBe('verdict');
    act(() => {
      vi.advanceTimersByTime(1);
    });
    expect(result.current.beat).toBe('impact');
    act(() => {
      vi.advanceTimersByTime(349); // impact is 300 + 50 = 350ms
    });
    expect(result.current.beat).toBe('impact');
    act(() => {
      vi.advanceTimersByTime(1);
    });
    expect(result.current.beat).toBe('release');
    act(() => {
      vi.advanceTimersByTime(200); // release
    });
    expect(result.current.beat).toBe('done');
    // total: 150 + 600 + 1200 + 350 + 200 = 2500ms exactly.
  });

  it('spectating an NPC grunt auto-plays through Brisk timing with no armed wait', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('npc-grunt-swing')));
    expect(result.current.beat).toBe('cue');
    act(() => {
      vi.advanceTimersByTime(75); // Brisk cue
    });
    expect(result.current.beat).toBe('throw'); // auto-played, no 'armed'
    act(() => {
      vi.advanceTimersByTime(300); // Brisk throw
    });
    expect(result.current.beat).toBe('verdict');
  });

  it('spectating an NPC boss crit still gets Cinematic timing (design.md §4)', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('npc-boss-swing')));
    act(() => {
      vi.advanceTimersByTime(150); // Cinematic cue
    });
    expect(result.current.beat).toBe('throw');
    act(() => {
      vi.advanceTimersByTime(600);
    });
    expect(result.current.beat).toBe('verdict');
  });

  it('auto-throws after AUTO_THROW_TIMEOUT_MS if the player never calls throwDie (design.md §2)', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-hit')));
    act(() => {
      vi.advanceTimersByTime(150); // cue -> armed
    });
    expect(result.current.beat).toBe('armed');
    act(() => {
      vi.advanceTimersByTime(AUTO_THROW_TIMEOUT_MS);
    });
    expect(result.current.beat).toBe('throw');
  });

  it('the second correlation group of repeated-attacks compresses to Brisk even though the scenario pace is cinematic', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('repeated-attacks')));
    // finish group 0 (cinematic hit, 1450ms) via skip to avoid re-asserting Task 1's timing
    act(() => {
      result.current.skip(); // cue/armed/throw -> verdict
      result.current.skip(); // verdict/impact/release -> finishes the group
    });
    expect(result.current.groupIndex).toBe(1);
    expect(result.current.beat).toBe('cue');
    act(() => {
      vi.advanceTimersByTime(75); // Brisk cue, NOT Cinematic's 150
    });
    expect(result.current.beat).toBe('throw');
  });

  it('skip() from cue/armed/throw jumps straight to verdict (design.md §1: "jumps to the verdict")', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-hit')));
    expect(result.current.beat).toBe('cue');
    act(() => {
      result.current.skip();
    });
    expect(result.current.beat).toBe('verdict');
  });

  it('skip() from verdict/impact/release finishes the current group', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-hit')));
    act(() => {
      result.current.skip(); // -> verdict
      result.current.skip(); // -> done (only one group)
    });
    expect(result.current.beat).toBe('done');
  });

  it('instant pace goes straight to done with no intermediate beats (design.md §4 escape hatch)', () => {
    const instantScenario = { ...scenario('player-hit'), pace: 'instant' as const };
    const { result } = renderHook(() => useBeatSequencer(instantScenario));
    expect(result.current.beat).toBe('done');
    expect(result.current.group?.attack?.hit).toBe(true);
  });

  it('reduced motion collapses Throw to REDUCED_MOTION_THROW_MS instead of the full tumble', () => {
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'), { reducedMotion: true })
    );
    act(() => {
      vi.advanceTimersByTime(150);
      result.current.throwDie();
    });
    expect(result.current.beat).toBe('throw');
    act(() => {
      vi.advanceTimersByTime(REDUCED_MOTION_THROW_MS);
    });
    expect(result.current.beat).toBe('verdict'); // not still 'throw' at the full 600ms mark
  });

  it('throwDie() is a no-op outside the armed beat', () => {
    const { result } = renderHook(() => useBeatSequencer(scenario('player-hit')));
    expect(result.current.beat).toBe('cue');
    act(() => {
      result.current.throwDie();
    });
    expect(result.current.beat).toBe('cue'); // unchanged
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test:run -- src/concepts/combat-pacing/useBeatSequencer.test.ts`

Expected: FAIL — `Error: Failed to resolve import "./useBeatSequencer"
from "src/concepts/combat-pacing/useBeatSequencer.test.ts". Does the file
exist?`

- [ ] **Step 3: Implement the state machine**

Create `src/concepts/combat-pacing/useBeatSequencer.ts`:

```ts
/**
 * useBeatSequencer (rpg-dnd5e-web#561) — the pure timing state machine for
 * the attack-loop beat model (design.md §1). Drives one scenario's
 * correlation groups through Cue -> [Armed] -> Throw -> Verdict ->
 * [Impact] -> Release -> Done, with pace-derived durations, crit
 * stretch, a short auto-throw timeout, tap-to-skip, and repeat-roll
 * compression (every group after the first runs at Brisk regardless of
 * the scenario's declared pace).
 *
 * This hook renders nothing and knows nothing about fixtures beyond the
 * `CombatPacingScenario` shape — `BeatStage.tsx` (Task 3) is the only
 * consumer that turns `beat`/`group` into pixels.
 */

import { useEffect, useRef, useState } from 'react';
import type { CombatPacingScenario } from './fixtures';
import { groupByCorrelation, type BeatGroupResult } from './fixtures';

export type BeatName =
  | 'idle'
  | 'cue'
  | 'armed'
  | 'throw'
  | 'verdict'
  | 'impact'
  | 'release'
  | 'done';

export interface BeatDurations {
  cue: number;
  throw: number;
  verdict: number;
  impact: number;
  release: number;
}

/** Cinematic-mode defaults (design.md §1) — the LOW end of each stated
 * range, chosen so the routine-hit/miss budgets land inside design.md's
 * stated bands: hit = 150+600+200+300+200 = 1450ms (design: "≈1.2-1.6s");
 * miss = 150+600+200+200 = 1150ms, skipping Impact (design: "≈1.0s" —
 * close; every value here is tunable, none is a placeholder). */
export const CINEMATIC: BeatDurations = {
  cue: 150,
  throw: 600,
  verdict: 200,
  impact: 300,
  release: 200,
};

/** Repeat-roll / grunt-tier compression (design.md §4): exactly half of
 * every CINEMATIC duration ("shorter tumble, faster stamp"). */
export const BRISK: BeatDurations = {
  cue: 75,
  throw: 300,
  verdict: 100,
  impact: 150,
  release: 100,
};

/** Crit stretches Verdict (the frame-breaker) and slightly oversizes
 * Impact, applied ONLY at Cinematic pace so a Cinematic-tier crit's total
 * budget lands at exactly 150+600+(200+1000)+(300+50)+200 = 2500ms —
 * design.md §1's "crit ≈ 2.5s (earned)". A Brisk-pace crit (a grunt's
 * lucky roll) does NOT stretch — grunts stay Brisk even on a crit; only
 * Cinematic actors (players, elites/bosses) get the frame-break, matching
 * design.md §4's "the boss's crit should land like a player's" (the boss
 * scenario is itself tiered Cinematic, not Brisk-with-an-exception). */
export const CRIT_VERDICT_EXTRA_MS = 1000;
export const CRIT_IMPACT_EXTRA_MS = 50;

/** A player must never be able to stall the table sitting on an
 * un-thrown die (design.md §2, "a short auto-timeout"). No number is
 * given in the design doc — 3000ms is this round's concrete, tunable
 * choice. */
export const AUTO_THROW_TIMEOUT_MS = 3000;

/** Reduced motion drops the tumble itself but keeps every beat's
 * semantics (design.md §4) — collapse Throw to a brief settle instead of
 * removing the beat outright. */
export const REDUCED_MOTION_THROW_MS = 80;

export interface UseBeatSequencerOptions {
  reducedMotion?: boolean;
}

export interface BeatSequencerState {
  beat: BeatName;
  groupIndex: number;
  groupCount: number;
  group?: BeatGroupResult;
  /** Throws the player's own die during `armed`; a no-op in any other beat. */
  throwDie: () => void;
  /** Jumps straight to `verdict` from `cue`/`armed`/`throw` (design.md §1:
   * "tap-to-skip jumps to the verdict"); from `verdict`/`impact`/`release`,
   * finishes the current group instead (advances to the next group, or
   * `done` if none remains) — round-one's extension of the same "never
   * trap a player in an animation" principle (design.md §8). */
  skip: () => void;
}

export function useBeatSequencer(
  scenario: CombatPacingScenario,
  options: UseBeatSequencerOptions = {}
): BeatSequencerState {
  const { reducedMotion = false } = options;
  const [beat, setBeat] = useState<BeatName>('idle');
  const [groupIndex, setGroupIndex] = useState(0);
  const groupsRef = useRef<BeatGroupResult[]>(
    groupByCorrelation(scenario.events)
  );
  const timerRef = useRef<ReturnType<typeof setTimeout>>();

  const clearTimer = () => {
    if (timerRef.current !== undefined) clearTimeout(timerRef.current);
    timerRef.current = undefined;
  };

  const after = (ms: number, run: () => void) => {
    clearTimer();
    timerRef.current = setTimeout(run, ms);
  };

  /** Reference-equality against the module-level CINEMATIC/BRISK singleton
   * (not a duration-value heuristic) — every group after the first
   * compresses to BRISK regardless of the scenario's own pace. */
  const durationsFor = (index: number): BeatDurations =>
    scenario.pace === 'brisk' || index > 0 ? BRISK : CINEMATIC;

  const finishGroup = (index: number) => {
    const next = index + 1;
    if (groupsRef.current[next]) {
      setGroupIndex(next);
      startGroup(next);
    } else {
      setBeat('done');
    }
  };

  const startGroup = (index: number) => {
    const group = groupsRef.current[index];
    if (!group || scenario.pace === 'instant') {
      setBeat('done');
      return;
    }
    const d = durationsFor(index);
    setBeat('cue');
    after(d.cue, () => {
      // Only the FIRST roll of a turn waits for the player's own throw —
      // repeat-roll compression (design.md §4) auto-plays every
      // subsequent group in the same turn, matching the speed the
      // compressed Brisk durations already imply.
      if (scenario.role === 'self' && index === 0) {
        setBeat('armed');
        after(AUTO_THROW_TIMEOUT_MS, () => runThrow(index));
      } else {
        runThrow(index);
      }
    });
  };

  const runThrow = (index: number) => {
    clearTimer();
    setBeat('throw');
    const d = durationsFor(index);
    const throwMs = reducedMotion ? REDUCED_MOTION_THROW_MS : d.throw;
    after(throwMs, () => runVerdict(index));
  };

  const runVerdict = (index: number) => {
    const group = groupsRef.current[index];
    const d = durationsFor(index);
    const cinematic = d === CINEMATIC;
    const critical = group?.attack?.critical ?? false;
    const hit = group?.attack?.hit ?? false;
    setBeat('verdict');
    const verdictMs =
      critical && cinematic ? d.verdict + CRIT_VERDICT_EXTRA_MS : d.verdict;
    after(verdictMs, () => {
      if (hit) runImpact(index);
      else runRelease(index);
    });
  };

  const runImpact = (index: number) => {
    const group = groupsRef.current[index];
    const d = durationsFor(index);
    const cinematic = d === CINEMATIC;
    const critical = group?.attack?.critical ?? false;
    setBeat('impact');
    const impactMs =
      critical && cinematic ? d.impact + CRIT_IMPACT_EXTRA_MS : d.impact;
    after(impactMs, () => runRelease(index));
  };

  const runRelease = (index: number) => {
    const d = durationsFor(index);
    setBeat('release');
    after(d.release, () => finishGroup(index));
  };

  useEffect(() => {
    groupsRef.current = groupByCorrelation(scenario.events);
    setGroupIndex(0);
    startGroup(0);
    return clearTimer;
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [scenario]);

  const throwDie = () => {
    if (beat === 'armed') runThrow(groupIndex);
  };

  const skip = () => {
    if (beat === 'idle' || beat === 'done') return;
    if (beat === 'cue' || beat === 'armed' || beat === 'throw') {
      runVerdict(groupIndex);
    } else {
      clearTimer();
      finishGroup(groupIndex);
    }
  };

  return {
    beat,
    groupIndex,
    groupCount: groupsRef.current.length,
    group: groupsRef.current[groupIndex],
    throwDie,
    skip,
  };
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `npm run test:run -- src/concepts/combat-pacing/useBeatSequencer.test.ts`

Expected: PASS — `Tests  12 passed (12)`.

- [ ] **Step 5: Typecheck**

Run: `npm run typecheck`

Expected: no errors.

- [ ] **Step 6: Commit**

```bash
git add src/concepts/combat-pacing/useBeatSequencer.ts src/concepts/combat-pacing/useBeatSequencer.test.ts
git commit -m "feat(concepts): combat-pacing beat sequencer state machine (#561)"
```

---
## Task 3: `BeatStage` — the presentational die/verdict/damage component

**Files:**
- Create: `src/concepts/combat-pacing/BeatStage.test.tsx`
- Create: `src/concepts/combat-pacing/BeatStage.tsx`

**Interfaces:**
- Consumes: `AttackResolvedLike`, `EntityDamagedLike` from `./fixtures`
  (Task 1); `BeatName` from `./useBeatSequencer` (Task 2).
- Produces (consumed by Task 4): `Placement`, `BeatStageProps`, the
  `BeatStage` component.

- [ ] **Step 1: Write the failing test file**

Create `src/concepts/combat-pacing/BeatStage.test.tsx`:

```tsx
import { render, screen } from '@testing-library/react';
import { describe, expect, it } from 'vitest';
import { BeatStage } from './BeatStage';
import type { AttackResolvedLike, EntityDamagedLike } from './fixtures';

const hitAttack: AttackResolvedLike = {
  attackerEntityId: 'char-aldric',
  targetEntityId: 'npc-goblin-1',
  hit: true,
  critical: false,
  attackRoll: 14,
  attackBonus: 5,
  targetAc: 16,
  hasAdvantage: false,
  advantageSources: [],
  hasDisadvantage: false,
  disadvantageSources: [],
};

const critAttack: AttackResolvedLike = { ...hitAttack, critical: true, attackRoll: 20 };
const nat1Attack: AttackResolvedLike = {
  ...hitAttack,
  hit: false,
  critical: false,
  attackRoll: 1,
};

const dmg: EntityDamagedLike = {
  entityId: 'npc-goblin-1',
  amount: 7,
  damageType: { module: 'dnd5e', type: 'damage', id: 'slashing' },
  hpAfter: { current: 8, max: 15 },
};

describe('BeatStage', () => {
  it('renders the cue beat', () => {
    render(
      <BeatStage
        beat="cue"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-cue')).toBeTruthy();
    expect(screen.queryByTestId('beat-verdict')).toBeNull();
  });

  it('renders a tumbling die during throw, unless reducedMotion', () => {
    render(
      <BeatStage
        beat="throw"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-die').className).toContain('tumbling');
  });

  it('renders a settled die during throw when reducedMotion is set', () => {
    render(
      <BeatStage
        beat="throw"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion
      />
    );
    expect(screen.getByTestId('beat-die').className).toContain('settled');
  });

  it('shows HIT for a plain hit verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-verdict').textContent).toContain('HIT');
  });

  it('shows MISS for a miss verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={{ ...hitAttack, hit: false }}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-verdict').textContent).toContain('MISS');
  });

  it('shows CRIT for a critical verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={critAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-verdict').textContent).toContain('CRIT');
  });

  it('shows NAT-1 for attackRoll 1 + miss, distinct from CRIT', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={nat1Attack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-verdict').textContent).toContain('NAT-1');
  });

  it('renders the damage number only during impact', () => {
    render(
      <BeatStage
        beat="impact"
        placement="token-anchored"
        attack={hitAttack}
        damage={dmg}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-damage').textContent).toContain('7');
  });

  it('does not render damage during release', () => {
    render(
      <BeatStage
        beat="release"
        placement="token-anchored"
        attack={hitAttack}
        damage={dmg}
        reducedMotion={false}
      />
    );
    expect(screen.queryByTestId('beat-damage')).toBeNull();
  });

  it('a pure center-stage placement stays center-stage even on a plain hit', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="center-stage"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-stage').getAttribute('data-placement')).toBe(
      'center-stage'
    );
  });

  it('token-anchored promotes to center-stage on a crit (design.md §2)', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={critAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-stage').getAttribute('data-placement')).toBe(
      'center-stage'
    );
  });

  it('token-anchored promotes to center-stage on a nat-1 too', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={nat1Attack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-stage').getAttribute('data-placement')).toBe(
      'center-stage'
    );
  });

  it('token-anchored stays token-anchored on a plain hit (no promotion)', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-stage').getAttribute('data-placement')).toBe(
      'token-anchored'
    );
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test:run -- src/concepts/combat-pacing/BeatStage.test.tsx`

Expected: FAIL — `Error: Failed to resolve import "./BeatStage" from
"src/concepts/combat-pacing/BeatStage.test.tsx". Does the file exist?`

- [ ] **Step 3: Implement the component**

Create `src/concepts/combat-pacing/BeatStage.tsx`:

```tsx
/**
 * BeatStage (rpg-dnd5e-web#561) — the presentational die/verdict/damage
 * surface for one beat. Takes ONLY beat-shaped props: no fixture,
 * scenario, or correlation-id knowledge, so promotion later (swapping
 * fixtures for a live-stream data source) touches only the data source,
 * never this component (design.md §7, "reusable presentation components
 * accept only presentation props").
 *
 * Every SFX-worthy moment below is marked with an "SFX slot" comment
 * (design.md §5: "audio hooks only — no actual sound implementation ships
 * round one").
 */

import type { AttackResolvedLike, EntityDamagedLike } from './fixtures';
import type { BeatName } from './useBeatSequencer';

export type Placement = 'token-anchored' | 'center-stage';

export interface BeatStageProps {
  beat: BeatName;
  placement: Placement;
  attack?: AttackResolvedLike;
  damage?: EntityDamagedLike;
  reducedMotion: boolean;
}

function verdictLabel(attack?: AttackResolvedLike): string {
  if (!attack) return '';
  if (attack.critical) return 'CRIT';
  if (attack.attackRoll === 1 && !attack.hit) return 'NAT-1';
  return attack.hit ? 'HIT' : 'MISS';
}

function verdictModifier(label: string): string {
  return label.toLowerCase().replace('-', '');
}

export function BeatStage({
  beat,
  placement,
  attack,
  damage,
  reducedMotion,
}: BeatStageProps) {
  // Token-anchored promotes to center-stage for a crit/nat-1 frame-break
  // (design.md §2) — a pure center-stage placement never moves.
  const isFrameBreak =
    !!attack && (attack.critical || (attack.attackRoll === 1 && !attack.hit));
  const effectivePlacement: Placement =
    placement === 'token-anchored' && isFrameBreak ? 'center-stage' : placement;

  const label = verdictLabel(attack);

  return (
    <div
      data-testid="beat-stage"
      data-beat={beat}
      data-placement={effectivePlacement}
      className={`beat-stage beat-stage--${effectivePlacement}`}
    >
      {beat === 'cue' && (
        // SFX slot: a soft "readying" cue sting.
        <div data-testid="beat-cue" className="beat-cue">
          {attack?.attackerEntityId ?? '…'} readies…
        </div>
      )}

      {(beat === 'armed' || beat === 'throw') && (
        // SFX slot: die-tumble rattle (throw) / none (armed, silent wait).
        <div
          data-testid="beat-die"
          className={
            beat === 'armed' || reducedMotion
              ? 'beat-die beat-die--settled'
              : 'beat-die beat-die--tumbling'
          }
        >
          🎲
        </div>
      )}

      {(beat === 'verdict' || beat === 'impact' || beat === 'release') &&
        attack && (
          // SFX slot: verdict stamp (gold chime on CRIT, comedic honk on
          // NAT-1, a duller thud on MISS, a clean stamp on HIT).
          <div
            data-testid="beat-verdict"
            className={`beat-verdict beat-verdict--${verdictModifier(label)}`}
          >
            {label} ({attack.attackRoll}+{attack.attackBonus} vs AC{' '}
            {attack.targetAc})
          </div>
        )}

      {beat === 'impact' && damage && (
        // SFX slot: impact thud, oversized/gold-tinted for a crit (the
        // gold tint itself is a future visual pass — round one's plan
        // only reserves the slot, per design.md §5).
        <div data-testid="beat-damage" className="beat-damage">
          -{damage.amount}
        </div>
      )}
    </div>
  );
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `npm run test:run -- src/concepts/combat-pacing/BeatStage.test.tsx`

Expected: PASS — `Tests  12 passed (12)`.

- [ ] **Step 5: Typecheck + lint**

Run: `npm run typecheck && npm run lint`

Expected: no errors.

- [ ] **Step 6: Commit**

```bash
git add src/concepts/combat-pacing/BeatStage.tsx src/concepts/combat-pacing/BeatStage.test.tsx
git commit -m "feat(concepts): combat-pacing BeatStage presentation component (#561)"
```

---
## Task 4: `CombatPacingConcept` — wiring, comparison layout, and registration

**Files:**
- Create: `src/concepts/combat-pacing/CombatPacingConcept.test.tsx`
- Create: `src/concepts/combat-pacing/CombatPacingConcept.tsx`
- Modify: `src/concepts/ConceptsView.tsx`

**Interfaces:**
- Consumes: `SCENARIOS`, `Pace`, `CombatPacingScenario` from `./fixtures`
  (Task 1); `useBeatSequencer`, `BeatName` from `./useBeatSequencer`
  (Task 2); `BeatStage`, `Placement` from `./BeatStage` (Task 3).
- Produces: `CombatPacingConcept` component, exported and registered into
  `ConceptsView.tsx`'s `ConceptPage` union — no other file consumes
  anything further from this task.

- [ ] **Step 1: Write the failing test file**

Create `src/concepts/combat-pacing/CombatPacingConcept.test.tsx`:

```tsx
import { act, fireEvent, render, screen } from '@testing-library/react';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { CombatPacingConcept } from './CombatPacingConcept';

beforeEach(() => {
  vi.useFakeTimers();
});

afterEach(() => {
  vi.useRealTimers();
});

describe('CombatPacingConcept', () => {
  it('renders both placements side by side for the same scenario', () => {
    render(<CombatPacingConcept />);
    const stages = screen.getAllByTestId('beat-stage');
    expect(stages).toHaveLength(2);
    const placements = stages.map((s) => s.getAttribute('data-placement'));
    expect(placements).toContain('token-anchored');
    expect(placements).toContain('center-stage');
  });

  it('shows a throw-die button only while armed, for a self-role scenario', () => {
    render(<CombatPacingConcept />);
    // default scenario is player-hit (role: self) — cue is 150ms.
    expect(screen.queryByTestId('throw-die-button')).toBeNull();
    act(() => {
      vi.advanceTimersByTime(150);
    });
    expect(screen.getByTestId('throw-die-button')).toBeTruthy();
    fireEvent.click(screen.getByTestId('throw-die-button'));
    expect(screen.queryByTestId('throw-die-button')).toBeNull();
  });

  it('switching scenarios resets the sequencer to cue', () => {
    render(<CombatPacingConcept />);
    act(() => {
      vi.advanceTimersByTime(150); // player-hit reaches 'armed'
    });
    fireEvent.click(screen.getByTestId('scenario-button-npc-boss-swing'));
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('cue');
  });

  it('a pace override drives the sequencer at the overridden pace', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('pace-override-instant'));
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('done');
  });

  it('the skip button advances the beat immediately', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('skip-button'));
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('verdict');
  });

  it('the event/intent inspector lists every fixture event with its sequence and correlationId', () => {
    render(<CombatPacingConcept />);
    const inspector = screen.getByTestId('event-inspector');
    // player-hit has 3 events: actionResolved, attackResolved, entityDamaged.
    expect(inspector.textContent).toContain('seq 1');
    expect(inspector.textContent).toContain('seq 2');
    expect(inspector.textContent).toContain('seq 3');
    expect(inspector.textContent).toContain('corr-hit');
  });

  it('the reduced-motion toggle is wired to both beat stages', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('reduced-motion-toggle'));
    act(() => {
      vi.advanceTimersByTime(150); // cue -> armed
    });
    fireEvent.click(screen.getByTestId('throw-die-button'));
    act(() => {
      vi.advanceTimersByTime(80); // REDUCED_MOTION_THROW_MS, not 600
    });
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('verdict');
  });

  it('every viewport-frame button is present, including the narrow fallback (design.md §8)', () => {
    render(<CombatPacingConcept />);
    expect(screen.getByTestId('frame-button-narrow')).toBeTruthy();
    expect(screen.getByTestId('frame-button-floor')).toBeTruthy();
    expect(screen.getByTestId('frame-button-typical')).toBeTruthy();
    expect(screen.getByTestId('frame-button-full')).toBeTruthy();
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test:run -- src/concepts/combat-pacing/CombatPacingConcept.test.tsx`

Expected: FAIL — `Error: Failed to resolve import "./CombatPacingConcept"
from "src/concepts/combat-pacing/CombatPacingConcept.test.tsx". Does the
file exist?`

- [ ] **Step 3: Implement the concept page**

Create `src/concepts/combat-pacing/CombatPacingConcept.tsx`:

```tsx
/**
 * CombatPacingConcept (rpg-dnd5e-web#561) — the round-one design-review
 * bench for the attack-loop beat model (design.md). Fixture-first, same
 * method as the equipment concept (#557): real presentation components
 * (`BeatStage`) fed by typed fixtures (`fixtures.ts`), driven by the pure
 * `useBeatSequencer` state machine, plus an event/intent inspector.
 *
 * Placement is NOT pre-decided (design.md §2/§7/§8) — this page renders
 * BOTH token-anchored and center-stage placements side by side against
 * the SAME `useBeatSequencer` state for every scenario, so Kirk can
 * compare them directly instead of toggling between two separate views.
 */

import { useState } from 'react';
import { BeatStage } from './BeatStage';
import type { Pace } from './fixtures';
import { SCENARIOS } from './fixtures';
import { useBeatSequencer } from './useBeatSequencer';

const FRAMES = {
  narrow: { label: '480×640 (below floor)', width: 480, height: 640 },
  floor: { label: '1024×768 (floor)', width: 1024, height: 768 },
  typical: { label: '1440×900 (typical)', width: 1440, height: 900 },
  full: { label: '1920×1080', width: 1920, height: 1080 },
} as const;
type FrameId = keyof typeof FRAMES;

type PaceOverride = 'default' | Pace;
const PACE_OVERRIDES: PaceOverride[] = ['default', 'cinematic', 'brisk', 'instant'];

function chipStyle(active: boolean): React.CSSProperties {
  return {
    padding: '4px 10px',
    borderRadius: 6,
    fontSize: 13,
    cursor: 'pointer',
    background: active ? 'var(--accent-primary)' : 'var(--bg-secondary)',
    color: 'var(--text-primary)',
    border: `1px solid ${active ? 'var(--accent-primary)' : 'var(--border-primary)'}`,
  };
}

export function CombatPacingConcept() {
  const [scenarioId, setScenarioId] = useState(SCENARIOS[0].id);
  const [paceOverride, setPaceOverride] = useState<PaceOverride>('default');
  const [reducedMotion, setReducedMotion] = useState(false);
  const [frame, setFrame] = useState<FrameId>('floor');

  const scenario = SCENARIOS.find((s) => s.id === scenarioId) ?? SCENARIOS[0];
  const effectiveScenario =
    paceOverride === 'default' ? scenario : { ...scenario, pace: paceOverride };

  const seq = useBeatSequencer(effectiveScenario, { reducedMotion });

  const selectScenario = (id: string) => setScenarioId(id);

  return (
    <div>
      <p style={{ color: 'var(--text-muted)', marginBottom: 12, fontSize: 14 }}>
        Round-one bench for the attack-loop beat model (web#561). Both
        placements render side by side against the SAME fixture — token-
        anchored (left) promotes to center-stage on a crit/nat-1; the pure
        center-stage placement (right) never moves. Pace override and
        reduced motion apply to both. Tests use fake timers; a `reload`
        step is not needed here since `/concepts` needs no backend.
      </p>

      <div style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 8 }}>
        {SCENARIOS.map((s) => (
          <button
            key={s.id}
            data-testid={`scenario-button-${s.id}`}
            style={chipStyle(s.id === scenarioId)}
            onClick={() => selectScenario(s.id)}
          >
            {s.label}
          </button>
        ))}
      </div>

      <div style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 8 }}>
        {PACE_OVERRIDES.map((p) => (
          <button
            key={p}
            data-testid={`pace-override-${p}`}
            style={chipStyle(paceOverride === p)}
            onClick={() => setPaceOverride(p)}
          >
            Pace: {p}
          </button>
        ))}
        <button
          data-testid="reduced-motion-toggle"
          style={chipStyle(reducedMotion)}
          onClick={() => setReducedMotion((v) => !v)}
        >
          Reduced motion: {reducedMotion ? 'on' : 'off'}
        </button>
      </div>

      <div style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 12 }}>
        {(Object.keys(FRAMES) as FrameId[]).map((id) => (
          <button
            key={id}
            data-testid={`frame-button-${id}`}
            style={chipStyle(frame === id)}
            onClick={() => setFrame(id)}
          >
            {FRAMES[id].label}
          </button>
        ))}
      </div>

      <p style={{ color: 'var(--text-muted)', marginBottom: 8, fontSize: 13 }}>
        <em>{scenario.description}</em>
      </p>

      <div
        style={{
          width: FRAMES[frame].width,
          maxWidth: '100%',
          minHeight: FRAMES[frame].height / 2,
          border: '2px solid var(--border-primary)',
          borderRadius: 8,
          padding: 16,
          display: 'flex',
          gap: 24,
          background: 'var(--bg-primary)',
        }}
      >
        <div style={{ flex: 1 }}>
          <div style={{ fontSize: 12, opacity: 0.7, marginBottom: 4 }}>
            Token-anchored (promotes on crit/nat-1)
          </div>
          <BeatStage
            beat={seq.beat}
            placement="token-anchored"
            attack={seq.group?.attack}
            damage={seq.group?.damage}
            reducedMotion={reducedMotion}
          />
        </div>
        <div style={{ flex: 1 }}>
          <div style={{ fontSize: 12, opacity: 0.7, marginBottom: 4 }}>
            Pure center-stage
          </div>
          <BeatStage
            beat={seq.beat}
            placement="center-stage"
            attack={seq.group?.attack}
            damage={seq.group?.damage}
            reducedMotion={reducedMotion}
          />
        </div>
      </div>

      <div style={{ display: 'flex', gap: 8, marginTop: 12 }}>
        {seq.beat === 'armed' && (
          <button
            data-testid="throw-die-button"
            style={chipStyle(false)}
            onClick={seq.throwDie}
          >
            🎲 Throw
          </button>
        )}
        <button data-testid="skip-button" style={chipStyle(false)} onClick={seq.skip}>
          Skip
        </button>
        <span style={{ fontSize: 12, opacity: 0.7, alignSelf: 'center' }}>
          Group {seq.groupIndex + 1} / {seq.groupCount} — beat: {seq.beat}
        </span>
      </div>

      <div
        data-testid="event-inspector"
        style={{
          marginTop: 12,
          fontFamily: 'monospace',
          fontSize: 12,
          background: 'var(--bg-secondary)',
          borderRadius: 8,
          padding: 10,
          maxHeight: 160,
          overflowY: 'auto',
        }}
      >
        <div style={{ opacity: 0.7, marginBottom: 4 }}>
          Event/intent inspector — this scenario's fixture events verbatim
        </div>
        {scenario.events.map((e) => (
          <div key={e.sequence}>
            seq {e.sequence} · corr {e.correlationId} · {e.case}
          </div>
        ))}
      </div>
    </div>
  );
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `npm run test:run -- src/concepts/combat-pacing/CombatPacingConcept.test.tsx`

Expected: PASS — `Tests  8 passed (8)`.

- [ ] **Step 5: Register the concept page in `ConceptsView.tsx`**

In `src/concepts/ConceptsView.tsx`, add the import:

```ts
import { CombatPacingConcept } from './combat-pacing/CombatPacingConcept';
```

Extend the `ConceptPage` union:

```ts
type ConceptPage =
  | 'class-selection'
  | 'encounter-dock'
  | 'combat-panel'
  | 'equipment'
  | 'combat-pacing';
```

Add its `CONCEPT_PAGES` entry (after `equipment`):

```ts
const CONCEPT_PAGES: { id: ConceptPage; label: string }[] = [
  { id: 'class-selection', label: 'Class Selection' },
  { id: 'encounter-dock', label: 'Encounter Dock' },
  { id: 'combat-panel', label: 'Combat Panel' },
  { id: 'equipment', label: 'Equipment' },
  { id: 'combat-pacing', label: 'Combat Pacing' },
];
```

Add its render branch (after the `equipment` branch):

```tsx
{activePage === 'equipment' && <EquipmentConcept />}
{activePage === 'combat-pacing' && <CombatPacingConcept />}
```

- [ ] **Step 6: Run the full test suite + typecheck + lint**

Run: `npm run test:run && npm run typecheck && npm run lint`

Expected: all pass, 0 regressions in any pre-existing file.

- [ ] **Step 7: Commit**

```bash
git add src/concepts/combat-pacing/CombatPacingConcept.tsx \
  src/concepts/combat-pacing/CombatPacingConcept.test.tsx \
  src/concepts/ConceptsView.tsx
git commit -m "feat(concepts): combat-pacing concept page + register in ConceptsView (#561)"
```

---
## Task 5: `CONTRACT.md`, docs, full CI, visual evidence, and PR flow

**Files:**
- Create: `src/concepts/combat-pacing/CONTRACT.md`
- Modify: `docs/how-to/concepts-route.md`
- Modify: `docs/architecture/components/concepts-route.md`
- Create: `docs/evidence/combat-pacing-561-token-anchored.png`
- Create: `docs/evidence/combat-pacing-561-center-stage.png`
- Create: `docs/evidence/combat-pacing-561-narrow.png`
- Create: `docs/evidence/combat-pacing-561.md`

**Interfaces:**
- Consumes: nothing new — this task verifies and documents Tasks 1-4's
  already-landed behavior; no further component/hook/fixture code
  changes.

- [ ] **Step 1: Write `CONTRACT.md`**

Create `src/concepts/combat-pacing/CONTRACT.md`, following
`src/concepts/equipment/CONTRACT.md`'s exact structure (numbered
observations, each with a why, explicit "not asks yet" framing) and
porting design.md §6 verbatim into that shape:

```markdown
# Combat-pacing contract log — evidence, not asks yet (rpg-dnd5e-web#561)

Produced by the fixture-first combat-pacing concept (`/concepts` →
Combat Pacing). Following the equipment concept's lifecycle (#557):
during the concept this file records evidence, observations, and
candidate gaps as the fixture lab is built and exercised — it is
explicitly NOT a pre-authored feature request. Filing anything on the
Platform lane of board #19 happens only after Kirk reviews this concept
and confirms a candidate is a real, scoped need (design.md §6 intro).

Almost all of round one is client-side and fixture-driven — there is
nothing to ask the platform team for yet.

## 1. Correlation/cardinality/completeness (candidate, not a request)

The event spine supports several real shapes, and this concept's
fixtures are deliberately authored to match each one rather than assume
a single fixed triple (design.md §"What we know about the wire"):

- A declared strike: `ActionResolved` → `AttackResolved` → `EntityDamaged`
  (hit only — a miss has no damage event at all).
- An opportunity attack: `AttackResolved` (+ optional `EntityDamaged`),
  with **no** `ActionResolved` — the `opportunity-attack` fixture
  (`fixtures.ts`) models exactly this shape.
- A non-attack `ActionResolved` (dodge, dash, potion, …) can have no
  attack event at all — not exercised by a round-one fixture (round one
  is attack-only per design.md §7's case list), flagged here as a gap in
  fixture COVERAGE, not a wire gap.
- No documented, universal completion marker for a correlation group —
  only the actor-only, TakeAction-only `TurnStateChanged` that can
  incidentally close a declared strike's group. This concept's own
  `groupByCorrelation` (`fixtures.ts`) sidesteps the question entirely by
  relying on a closed, authored fixture list — explicitly a
  scenario-boundary convenience, NOT a production reassembler (design.md
  §7's own caveat). Whether `TurnStateChanged` is reliable enough for a
  REAL reassembler to build on is exactly what a later round's
  CONTRACT.md work would need to verify against live behavior — this
  round observes nothing new here beyond what design.md already logged.

## 2. The dispatch layer discards the envelope (already-logged, re-confirmed)

`dispatchEncounterStreamEvent` (`src/api/encounterStreamDispatch.ts`)
passes only `payload.value` to each callback — `correlation_id`,
`sequence`, and `timestamp` on `EncounterEvent` are dropped before they
reach any combat callback. Real, observed, **not a round-one
prerequisite** — round one plays fixtures, not the live stream, so
nothing here blocks the concept (design.md §6.2). This concept's own
`PacingFixtureEvent` type (`fixtures.ts`) carries `sequence`/
`correlationId` precisely so a future reassembler round has a fixture
shape to promote against once this dispatch gap is closed.

## 3. Roll-metadata gaps (flag, don't ask yet)

- `AttackResolved` already carries everything this concept's beat
  sequencer needs (`attackRoll`, `attackBonus`, `targetAc`, `hit`,
  `critical`, `hasAdvantage`/`hasDisadvantage` + source refs) — no gap
  here, confirmed by this concept's own fixture types matching the wire
  field-for-field with zero invented fields.
- **Not** carried: raw damage-dice faces (`EntityDamaged.damage_breakdown`
  is by source, not by die). Not relevant round one (d20-only, design.md
  "Decided this revision") — `EntityDamagedLike` in this concept
  deliberately has no per-die breakdown field.
- **Not** carried: the discarded die on advantage/disadvantage. Flag
  only, not asked for — this concept's `advantageSources`/
  `disadvantageSources` fixture fields already match what the wire DOES
  carry; showing the dropped die itself is future work (design.md §6.3).

## 4. Do NOT ask for two-phase (roll-started/roll-resolved) events

Restated from design.md §6.4: true two-phase rolls would double event
volume, add server-side hold-and-emit-twice latency, and buy nothing in
co-op (no player has secret information anyway). This concept's entire
beat model is built as theater over an already-resolved event
specifically to avoid needing this — every fixture's `AttackResolved`
already carries a final, resolved roll.

## No Platform issue filed

Nothing above is a request. It restates design.md §6's candidates in the
shape the equipment concept's CONTRACT.md used, adding only the concrete
confirmation that this concept's own fixture/hook/component code needed
no invented wire field to build the round-one bench — filing anything on
Platform's board-#19 lane happens only after Kirk reviews this concept.
```

- [ ] **Step 2: Add the `combat-pacing/` row to `docs/how-to/concepts-route.md`**

Under `## Current concepts`, add a row to the table (after the
`combat-panel` row):

```markdown
| `combat-pacing/`   | Design review (web#561)  | Round-1 beat-sequencer bench (`useBeatSequencer` + `BeatStage`) comparing token-anchored vs. center-stage die placement on identical fixtures; fixture-first exemplar. |
```

Also update the frontmatter `updated:` field at the top of the file to
today's date (the date this step is actually executed).

- [ ] **Step 3: Add the `combat-pacing` paragraph to `docs/architecture/components/concepts-route.md`**

Under `## Current contents`, add a paragraph (after the `equipment/`
paragraph):

```markdown
`src/concepts/combat-pacing/` — the attack-loop beat-sequencer bench
(web#561): `fixtures.ts` (8 event-shaped round-one scenarios matching the
real `AttackResolved`/`EntityDamaged`/`ActionResolved` field shapes),
`useBeatSequencer.ts` (the pure Cue→Throw→Verdict→Impact→Release timing
state machine — pace-derived durations, crit stretch, auto-throw
timeout, tap-to-skip, repeat-roll compression), `BeatStage.tsx` (the
presentational die/verdict/damage surface, beat-shaped props only,
token-anchored-promotes-to-center-stage on crit/nat-1), and
`CombatPacingConcept.tsx` (renders both placements side by side against
the same fixture, plus a pace-override/reduced-motion/viewport-frame
switcher and an event/intent inspector). `CONTRACT.md` restates
design.md §6's wire-shape candidates in the equipment concept's
gap-log convention — no Platform issue filed by this round.
```

Also update the frontmatter `updated:` field at the top of the file.

- [ ] **Step 4: Run the full CI check**

Run: `npm run ci-check`

Expected: all steps pass — format check clean, lint clean, typecheck
clean, build succeeds, full test suite green (no regressions in any file
outside the ones touched in Tasks 1-4 plus this task's two doc edits).

- [ ] **Step 5: MANDATORY visual verification — both placements at the floor, larger, and the narrow fallback**

`/concepts` needs no backend (fixture-first, per design.md §7) — this is
simpler than a live-route check. Start the dev server:

```bash
npm run dev
```

Using the chrome-devtools MCP tools (or equivalent), navigate to
`http://localhost:5173/concepts`, click the "Combat Pacing" tab, and for
EACH of the following, capture a screenshot and view it directly (not
just "the test passed" — an actual look, per this repo's real-route
evidence convention):

1. **1024×768 floor** (`frame-button-floor`, the default): click the
   `player-crit` scenario button, click skip once or twice until the
   verdict/impact beats are visible, and confirm BOTH `BeatStage`
   columns are visible side by side without horizontal scrolling or
   clipping, and the token-anchored column shows `data-placement`
   promoted to `center-stage` (visually indistinguishable from the pure
   center-stage column, confirming the promotion actually renders, not
   just passes a unit assertion). Save as
   `docs/evidence/combat-pacing-561-token-anchored.png` (crop or
   annotate to the token-anchored column) and
   `docs/evidence/combat-pacing-561-center-stage.png` (the center-stage
   column) — same capture, split for the write-up below.
2. **1440×900 typical and 1920×1080 full**: click each frame button in
   turn and confirm the layout breathes upward (no fixed/clipped
   elements) rather than staying pinned at the floor's pixel size.
3. **Narrow fallback below the floor** (`frame-button-narrow`, 480×640):
   confirm the two `BeatStage` columns still render — wrapped/stacked is
   an acceptable non-breaking fallback, an overlapping or clipped column
   is not. Save as `docs/evidence/combat-pacing-561-narrow.png`.

If either placement column fails to render, clips, or the token-anchored
promotion is not visually apparent on the `player-crit`/`player-nat1`
scenarios at ANY of the four frames, that is a real regression — stop
and debug before writing the evidence file below; do not paper over a
layout problem with a passing unit test (the `BeatStage.test.tsx` tests
only assert the `data-placement` attribute, not actual visual layout).

- [ ] **Step 6: Write the evidence file**

Create `docs/evidence/combat-pacing-561.md`, following the
`wall-fittings-536.md` convention (environment, what was viewed, an
honest statement of what was and was not confirmed):

```markdown
# Real-route evidence: combat-pacing round-one bench (#561)

Verifies the round-one `/concepts` → Combat Pacing bench (design.md §8's
acceptance bar): token-anchored vs. center-stage placement compared
against identical fixtures at the 1024×768 floor, a larger frame, and a
non-breaking narrow fallback below the floor.

## Environment

`npm run dev` from this branch's worktree, `http://localhost:5173/concepts`
→ Combat Pacing tab. No backend required — the bench is fixture-first
(design.md §7); nothing here touches `rpg-api`/the live encounter route.

## What I viewed

[Fill in after Step 5: which scenario(s)/frame(s) were captured, what
the token-anchored promotion looked like on `player-crit`/`player-nat1`
at each frame width, and an honest note on anything that read cluttered,
clipped, or otherwise not ready — matching the honesty bar the
`wall-fittings-536.md` precedent set for this repo's evidence docs.]

## Screenshots

- `combat-pacing-561-token-anchored.png`
- `combat-pacing-561-center-stage.png`
- `combat-pacing-561-narrow.png`
```

The bracketed paragraph above is a real, required section to fill in
with the actual observations from Step 5 — it is not a stand-in the
implementer skips; leaving it as literal bracketed text is a plan
violation, not a valid evidence doc.

- [ ] **Step 7: Commit**

```bash
git add src/concepts/combat-pacing/CONTRACT.md \
  docs/how-to/concepts-route.md \
  docs/architecture/components/concepts-route.md \
  docs/evidence/combat-pacing-561-token-anchored.png \
  docs/evidence/combat-pacing-561-center-stage.png \
  docs/evidence/combat-pacing-561-narrow.png \
  docs/evidence/combat-pacing-561.md
git commit -m "docs(concepts): combat-pacing CONTRACT.md, docs, real-route evidence (#561)"
```

- [ ] **Step 8: Open the PR**

```bash
git push -u origin feat/561-combat-pacing-concept
gh pr create \
  --repo KirkDiggler/rpg-dnd5e-web \
  --title "feat(concepts): combat pacing & dice — round-one beat-sequencer bench (#561)" \
  --body "Round-one \`/concepts\` bench for the attack-loop beat model, following the equipment concept's fixture-first pattern (#557). Fixture-driven \`useBeatSequencer\` (Cue→Throw→Verdict→Impact→Release, pace-derived durations, crit stretch, auto-throw timeout, tap-to-skip, repeat-roll compression) plus \`BeatStage\` (beat-shaped presentational props only) rendered in BOTH token-anchored and center-stage placements side by side against identical fixtures, an event/intent inspector, and a CONTRACT.md gap log (no Platform ask filed). Design: rpg-project's \`ideas/combat-pacing/design.md\` + \`plan.md\`. Closes no issue by itself — round one does not touch the live encounter route (rpg-dnd5e-web#561, Board 19).

— asset-pipeline agent, on behalf of KirkDiggler"
```

Expected: PR opens against `main`, CI runs and passes (same checks as
`npm run ci-check`), no direct commits to `main`.

---

## Self-review (performed by the plan author, not a task for the implementer)

- **Spec coverage:** every one of the task's required elements is
  present: required sub-skill header ✓; Goal/Architecture/Tech
  Stack/Global Constraints ✓; exact file map ✓ (table above); 5
  independently-testable TDD tasks (Tasks 1-4 are strict red/green TDD;
  Task 5 is the equipment-concept-precedented docs/CI/evidence/PR
  wrap-up task, matching `item-icons/plan.md`'s own Task 3 shape) ✓;
  exact current web paths/imports/types verified by reading the live
  `rpg-dnd5e-web` checkout (not guessed) ✓; test commands
  (`npm run test:run -- <path>`, `npm run typecheck`, `npm run lint`,
  `npm run ci-check`) match `package.json` exactly ✓; concrete code in
  every step, no vague instructions ✓; expected fail/pass output stated
  per step ✓; commit steps after every task ✓; `CONTRACT.md` lifecycle
  matches the #557 precedent exactly (evidence-only, no pre-authored
  ask) ✓; visual verification at 1024×768 floor + larger + narrow
  fallback is a mandatory, gated step (Task 5 Step 5) ✓. Scope
  exclusions (no live-stream reassembly, no Platform/proto/API/toolkit
  changes, no actual audio, no damage dice, no handoff/promotion) are
  each called out explicitly in Global Constraints and never crossed by
  any task's file list.
- **Placeholder scan:** no "TBD"/"add appropriate handling"/"similar to
  Task N" language anywhere in Tasks 1-5's code steps. The one
  intentionally-unfilled prose block (Task 5 Step 6's evidence-doc "What
  I viewed" section) is explicitly flagged as a required fill-in tied to
  Step 5's own observations, not a silent gap.
- **Type/name consistency:** traced every cross-task interface by hand —
  `PacingFixtureEvent`/`CombatPacingScenario`/`BeatGroupResult`/
  `groupByCorrelation` (Task 1) are the exact names imported in Task 2;
  `BeatName`/`BeatSequencerState`/`useBeatSequencer` (Task 2) are the
  exact names imported in Tasks 3-4; `Placement`/`BeatStageProps`/
  `BeatStage` (Task 3) are the exact names imported in Task 4. No
  renamed field slipped through (e.g. `attackRoll` is spelled identically
  everywhere it appears, matching the real proto's camelCase).
- **Path/command accuracy:** every file path was confirmed to exist (or,
  for new files, confirmed its parent directory's sibling convention)
  by reading the live `rpg-dnd5e-web` checkout during authoring — not
  assumed. `npm run test:run`/`typecheck`/`lint`/`ci-check` copied
  verbatim from this repo's `package.json`.
- **Scope:** no task touches `rpg-api`, `rpg-toolkit`, `rpg-api-protos`,
  `encounterStreamDispatch.ts`, `useEncounterStream.ts`, or any live
  encounter component; no audio asset/library added; no damage-dice
  animation; no wiring into `EncounterView`.
