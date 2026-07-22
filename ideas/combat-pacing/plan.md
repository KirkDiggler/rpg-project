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
`beatStageTypes.ts` — presentation-owned `BeatAttackView`/
`BeatDamageView` types (NOT imported from `fixtures.ts` — `fixtures.ts`'s
wire-shaped types are a structural superset, so they satisfy these
without an adapter). `BeatStage.tsx` — a presentational component that
takes only beat-shaped props (beat name, attack, damage, placement,
reduced-motion flag) and renders the die/verdict/damage with real CSS
(tumble/settle, crit gold, nat-1 red-crack, hit/miss color, oversized
crit damage, placement/promotion) — it is never told about fixtures,
correlation ids, or scenarios. `CombatPacingConcept.tsx` wires all of the
above plus a scenario/pace/reduced-motion/viewport-frame switcher and an
event/intent inspector, registered into `ConceptsView.tsx`.

**Tech Stack:** TypeScript 5.8 (strict mode), React 19 function
components + hooks, Vitest + `@testing-library/react` with
`vi.useFakeTimers()` for beat-timing tests, no new runtime dependencies.
Styling is real CSS added to `public/themes/base.css` (this repo's
existing global, dynamically-theme-loaded stylesheet — see
`src/hooks/useTheme.ts`), reusing two already-shipped conventions rather
than inventing new ones: the `@keyframes dice-roll`/`.animate-dice-roll`
tumble (already live in `src/components/DiceRoller.tsx`) and the crit-
gold/hit/miss color palette `src/components/game/CombatLog.tsx` already
renders combat text in. No animation library, no CSS-in-JS.

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
  animated die face (design.md "Decided this revision": d20-only). The
  wire's real `EntityDamaged.damage_breakdown` gap stays a CONTRACT.md
  flag, not something this round implements.
- **Visual styling DOES ship round one** — die tumble/settle, crit gold,
  nat-1 red-crack, hit/miss color differentiation, oversized crit damage,
  and placement/promotion are real CSS in Task 3, not deferred. Only the
  SOUND that could accompany them is deferred (the point above).
- **No handoff/promotion.** This plan does not wire `CombatPacingConcept`
  into `EncounterView` or any production path — it stays under
  `/concepts` (design.md §7, "does not file any platform request on its
  own").
- **`CONTRACT.md` starts evidence-only, never a pre-authored ask.** This
  is a correction from an earlier draft: the COMMITTED
  `src/concepts/equipment/CONTRACT.md` (PR #557) is itself already
  ask-shaped — literally titled "Equipment contract request — what the
  wire needs" — because it was written and committed AFTER Kirk's design
  review confirmed the gaps were real (its own fixtures.ts header even
  notes the asks it made "landed as rpg-api-protos#188/rpg-toolkit#812/
  rpg-api#682"). Combat-pacing's `CONTRACT.md` borrows that file's
  STRUCTURE (numbered items, each with a why) but starts life evidence-
  only, matching design.md §6's stated lifecycle for THIS round — it only
  becomes a concrete request in a LATER revision, after Kirk reviews this
  concept and confirms a candidate gap is real (design.md §6 intro). No
  issue gets filed on Platform's board lane by this plan.
- **Visual verification is mandatory before calling round one done**: the
  token-anchored vs. center-stage comparison must be checked against
  identical fixtures at a 1024×768 floor, at least one larger frame, and
  a non-breaking narrow fallback below the floor (design.md §8).
- One issue per PR, branch from latest `origin/main`, no direct commits
  to `main` — matching `rpg-dnd5e-web`'s own observed convention on its
  recent merged PRs (verified via `gh pr list`): `feat/531-equipment-
  slot-concept` (#557), `feat/571-equipment-live` (#575),
  `feat/559-crypt-prop-keys` (#567), `feat/558-crypt-room-spike` (#566) —
  every one a single-issue branch off `origin/main`, merged to `main`.
- Any GitHub issue/PR comment this work produces ends with
  `— asset-pipeline agent, on behalf of KirkDiggler` (this workspace's
  convention, `game-dev/CLAUDE.md`).

## File Map

| File | Responsibility |
| --- | --- |
| `src/concepts/combat-pacing/fixtures.ts` (new) | Wire-shaped `*Like` types (`ActionResolvedLike`, `AttackResolvedLike`, `EntityDamagedLike`), `PacingFixtureEvent`, `CombatPacingScenario`, `Pace`, `groupByCorrelation`, `BeatGroupResult`, and the `SCENARIOS` array (8 round-one cases from design.md §7). |
| `src/concepts/combat-pacing/fixtures.test.ts` (new) | Scenario-count/shape assertions: exactly 8 scenarios, the OA scenario has no `actionResolved` event, the nat-1/crit scenarios carry the right `AttackResolved` flags, the repeated-attacks scenario has 2 correlation groups, every scenario's `sequence` values are strictly increasing, `groupByCorrelation` groups and orders correctly. |
| `src/concepts/combat-pacing/useBeatSequencer.ts` (new) | Pure timing state machine: `BeatName`, `BeatDurations`, `CINEMATIC`/`BRISK` constants, `CRIT_VERDICT_EXTRA_MS`/`CRIT_IMPACT_EXTRA_MS`, `AUTO_THROW_TIMEOUT_MS`, `REDUCED_MOTION_THROW_MS`, and the `useBeatSequencer` hook (`throwDie`, `skip`, per-group compression, crit stretch, reduced-motion Throw collapse). Internal decisions read `beatRef`/`groupIndexRef` (plain refs), not the `beat`/`groupIndex` React state — state exists only to trigger re-renders — so two `skip()` calls in the same synchronous tick each see the other's effect immediately. |
| `src/concepts/combat-pacing/useBeatSequencer.test.ts` (new) | Fake-timer tests: full cinematic-hit budget (1450ms), miss skips Impact, crit stretches to exactly 2500ms, Brisk/NPC-grunt budget, second correlation group compresses to Brisk AND auto-plays with no armed wait, `armed` waits for `throwDie()` or `AUTO_THROW_TIMEOUT_MS`, `skip()` from `cue`/`armed`/`throw` jumps to `verdict`, two synchronous `skip()` calls in one tick finish the group (verdict -> done), `instant` pace goes straight to `done`, reduced motion collapses Throw to `REDUCED_MOTION_THROW_MS`, `throwDie()` no-op outside `armed`. |
| `src/concepts/combat-pacing/beatStageTypes.ts` (new) | Presentation-owned `BeatAttackView`, `BeatDamageView`, `VerdictLabel`, `verdictLabel()` — split into their own module (not inline in `BeatStage.tsx`) because a component file that also exports plain functions/types trips this repo's `react-refresh/only-export-components` lint rule, the same reason `equipmentTypes.ts` is split from `EquipmentSlots.tsx`. |
| `src/concepts/combat-pacing/BeatStage.tsx` (new) | Presentational component: `Placement` (`token-anchored` \| `center-stage`), `BeatStageProps` (`beat`, `placement`, `attack?: BeatAttackView`, `damage?: BeatDamageView`, `reducedMotion`), token-anchored-promotes-to-center-stage-on-crit/nat-1 logic. Imports its attack/damage types from `./beatStageTypes`, NOT from `./fixtures` — see Task 3's header. Real CSS (in `public/themes/base.css`, this task): die tumble/settle, crit gold, nat-1 red-crack wobble, hit/miss color, oversized crit damage, placement border treatment, reduced-motion suppression. |
| `src/concepts/combat-pacing/BeatStage.test.tsx` (new) | Render tests per beat (`cue`/`throw`/`armed`/`verdict`/`impact`/`release`), placement promotion on crit and on nat-1, reduced-motion class swap on the die AND the container, verdict class (`beat-verdict--hit`/`--miss`/`--crit`/`--nat1`) per outcome, `beat-damage--crit` only on a crit. |
| `public/themes/base.css` (modify) | Append `.beat-stage`/`.beat-cue`/`.beat-die`/`.beat-verdict`/`.beat-damage` rules + `@keyframes nat1-crack`, reusing the file's existing `@keyframes dice-roll`/`.animate-dice-roll` and `color-mix()` conventions (Task 3 Step 4). |
| `src/concepts/combat-pacing/CombatPacingConcept.tsx` (new) | Top-level concept page: scenario switcher (8 buttons), pace-override selector (`default`/`cinematic`/`brisk`/`instant`), reduced-motion toggle, viewport-frame selector (`narrow`/`floor`/`typical`/`full`, EXTENDING `combat-panel/CombatPanelConcept.tsx`'s `FRAMES` pattern with the added `narrow` frame), side-by-side `BeatStage` pair (token-anchored + center-stage sharing one `useBeatSequencer` instance), throw-die button (visible only while `armed`), skip button, and an event/intent inspector panel printing each fixture event's envelope fields (`sequence`, `correlationId`, `case`). `effectiveScenario` is `useMemo`'d on `[scenario, paceOverride]` — see Task 4's header for the object-identity bug this fixes. |
| `src/concepts/combat-pacing/CombatPacingConcept.test.tsx` (new) | Fake-timer interaction tests: switching scenarios resets the sequencer, instant pace override drives straight to `done`, a cinematic/Brisk pace override still progresses PAST `cue` (the object-identity regression guard), a compound reduced-motion+override case, clicking throw-die during `armed` advances immediately, skip button jumps beats, both `beat-stage` elements render simultaneously with the correct `data-placement`, the inspector lists every event, all 4 frame buttons exist. |
| `src/concepts/combat-pacing/CONTRACT.md` (new) | Evidence-only gap log per design.md §6, borrowing `src/concepts/equipment/CONTRACT.md`'s STRUCTURE (numbered observations, each with a why) but not its ask-shaped CONTENT — see the corrected Global Constraints bullet above. |
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
    expect(attack).toMatchObject({
      attackRoll: 1,
      hit: false,
      critical: false,
    });
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

  it("npc-boss-swing is tiered Cinematic and crits (design.md §4: boss crit lands like a player's)", () => {
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
 * classes directly — same rationale as equipmentTypes.ts. Two deliberate
 * exceptions, both noted at their field:
 *   - `sequence` is `number`, not the wire's `bigint` (int64) — every
 *     round-one scenario has under 10 events, far inside `number`'s exact
 *     range, and the event/intent inspector template-literals `seq
 *     ${e.sequence}` directly; a `bigint` would force every call site to
 *     `.toString()` it for no round-one benefit.
 *   - `ActionResolvedLike` omits `economyConsumed` — the beat sequencer
 *     never displays economy cost (that's the live action menu/dock's
 *     job, not this bench's), so it is not part of what this fixture
 *     needs to carry, matching the equipment concept's `ItemFixture`
 *     precedent of only including fields a consumer actually reads.
 *
 * `PacingFixtureEvent` carries `sequence` and `correlationId`, mirroring
 * the real `EncounterEvent` envelope's `event: { value, case }` oneof
 * shape — a fixture reads like a captured wire event (design.md §7).
 */

import type { RefLike } from '../../components/game/equipment/equipmentTypes';

export type Pace = 'cinematic' | 'brisk' | 'instant';

/** Matches ActionResolved (events_pb.ts) field-for-field, EXCEPT
 * `economyConsumed` — see file header. */
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

/** Matches HitPoints (types_pb.ts) field-for-field, INCLUDING `temp`
 * (required `int32` on the wire, not optional) — every fixture below
 * supplies `temp: 0` explicitly rather than making it optional here,
 * so this type stays a true field-for-field match. */
export interface HitPointsLike {
  current: number;
  max: number;
  temp: number;
}

/** Matches EntityDamaged (events_pb.ts) field-for-field, except
 * `sourceEntityId`/`damageBreakdown` — omitted because the beat
 * sequencer's display only reads `amount` + `hpAfter` (same
 * only-what's-read rationale as `ActionResolvedLike` above). */
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
    damage(3, 'corr-hit', GOBLIN, 7, { current: 8, max: 15, temp: 0 }),
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
    damage(3, 'corr-crit', GOBLIN, 14, { current: 1, max: 15, temp: 0 }),
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
    damage(2, 'corr-oa', PLAYER, 5, { current: 20, max: 25, temp: 0 }),
  ],
};

const npcGruntSwing: CombatPacingScenario = {
  id: 'npc-grunt-swing',
  label: 'NPC grunt swing (Brisk)',
  description:
    "A goblin grunt attacks and misses — tiered Brisk so four goblins acting doesn't drag (design.md §4).",
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
    damage(2, 'corr-boss', PLAYER, 22, { current: 3, max: 25, temp: 0 }),
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
    damage(3, 'corr-rep-1', GOBLIN, 6, { current: 9, max: 15, temp: 0 }),
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
 * logic — explicitly a scenario-boundary convenience (design.md §7's own
 * caveat: a live reassembler has no such boundary to lean on). */
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'))
    );
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-miss'))
    );
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-crit'))
    );
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('npc-grunt-swing'))
    );
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('npc-boss-swing'))
    );
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'))
    );
    act(() => {
      vi.advanceTimersByTime(150); // cue -> armed
    });
    expect(result.current.beat).toBe('armed');
    act(() => {
      vi.advanceTimersByTime(AUTO_THROW_TIMEOUT_MS);
    });
    expect(result.current.beat).toBe('throw');
  });

  it('the second correlation group of repeated-attacks compresses to Brisk and auto-plays with no armed wait, even though the scenario role is self', () => {
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('repeated-attacks'))
    );
    // Finish group 0 via two SYNCHRONOUS skip() calls in the same tick —
    // this is the exact case that requires skip()/throwDie() to read a
    // ref rather than the closured `beat` state variable (see
    // useBeatSequencer.ts's file header); both calls happen inside one
    // act(), with no render between them.
    act(() => {
      result.current.skip(); // cue/armed/throw -> verdict
      result.current.skip(); // verdict/impact/release -> finishes the group
    });
    expect(result.current.groupIndex).toBe(1);
    expect(result.current.beat).toBe('cue');
    act(() => {
      vi.advanceTimersByTime(75); // Brisk cue, NOT Cinematic's 150
    });
    expect(result.current.beat).toBe('throw'); // auto-played, group index > 0
  });

  it('skip() from cue/armed/throw jumps straight to verdict (design.md §1: "jumps to the verdict")', () => {
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'))
    );
    expect(result.current.beat).toBe('cue');
    act(() => {
      result.current.skip();
    });
    expect(result.current.beat).toBe('verdict');
  });

  it('two synchronous skip() calls in one tick finish the current group (verdict -> done)', () => {
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'))
    );
    act(() => {
      result.current.skip(); // -> verdict
      result.current.skip(); // -> done (only one group)
    });
    expect(result.current.beat).toBe('done');
  });

  it('instant pace goes straight to done with no intermediate beats (design.md §4 escape hatch)', () => {
    const instantScenario = {
      ...scenario('player-hit'),
      pace: 'instant' as const,
    };
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
    const { result } = renderHook(() =>
      useBeatSequencer(scenario('player-hit'))
    );
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
 * the scenario's declared pace, and auto-plays without an armed wait).
 *
 * Internal decisions (skip/throwDie/finishGroup) read from `beatRef`/
 * `groupIndexRef`, NOT the `beat`/`groupIndex` React state variables —
 * `useState` here exists only to trigger a re-render; the refs are the
 * single source of truth for "what beat are we actually in right now",
 * updated synchronously in the same tick a transition happens. This
 * matters because `skip()` can be called twice back-to-back in the same
 * event-handler tick (a fast double-click, or a single `act()` block in
 * a test) — reading React state there would see the PRE-update value
 * both times (state only commits on the next render), so two skips in
 * one tick would both take the same branch instead of advancing twice.
 *
 * This hook renders nothing and knows nothing about fixtures beyond the
 * `CombatPacingScenario` shape — `BeatStage.tsx` (Task 3) is the only
 * consumer that turns `beat`/`group` into pixels, and it does so through
 * its OWN presentation-owned types, not this module's fixture-shaped
 * ones (see `BeatStage.tsx`'s header).
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
   * trap a player in an animation" principle (design.md §8). Synchronous
   * and safe to call twice in the same tick — see file header. */
  skip: () => void;
}

export function useBeatSequencer(
  scenario: CombatPacingScenario,
  options: UseBeatSequencerOptions = {}
): BeatSequencerState {
  const { reducedMotion = false } = options;
  const [beat, setBeatState] = useState<BeatName>('idle');
  const [groupIndex, setGroupIndexState] = useState(0);
  const beatRef = useRef<BeatName>('idle');
  const groupIndexRef = useRef(0);
  const groupsRef = useRef<BeatGroupResult[]>(
    groupByCorrelation(scenario.events)
  );
  const timerRef = useRef<ReturnType<typeof setTimeout> | undefined>(undefined);

  const setBeat = (b: BeatName) => {
    beatRef.current = b;
    setBeatState(b);
  };
  const setGroupIndex = (i: number) => {
    groupIndexRef.current = i;
    setGroupIndexState(i);
  };

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
    if (beatRef.current === 'armed') runThrow(groupIndexRef.current);
  };

  const skip = () => {
    const current = beatRef.current;
    if (current === 'idle' || current === 'done') return;
    if (current === 'cue' || current === 'armed' || current === 'throw') {
      runVerdict(groupIndexRef.current);
    } else {
      clearTimer();
      finishGroup(groupIndexRef.current);
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
## Task 3: `BeatStage` — the presentational die/verdict/damage component + its styling

**Files:**
- Create: `src/concepts/combat-pacing/beatStageTypes.ts`
- Create: `src/concepts/combat-pacing/BeatStage.test.tsx`
- Create: `src/concepts/combat-pacing/BeatStage.tsx`
- Modify: `public/themes/base.css`

**Interfaces:**
- Consumes: `BeatName` from `./useBeatSequencer` (Task 2). Deliberately
  does NOT consume `AttackResolvedLike`/`EntityDamagedLike` from
  `./fixtures` (Task 1) — this component owns its own, narrower
  `BeatAttackView`/`BeatDamageView` types instead (`beatStageTypes.ts`,
  this task). `fixtures.ts`'s types are a structural superset (same
  field names/types, plus more), so `CombatPacingConcept.tsx` (Task 4)
  passes a `BeatGroupResult`'s `attack`/`damage` straight into this
  component's props with no adapter function and no cast — verified by
  `npm run typecheck` passing with zero errors once Task 4 wires them
  together.
- Produces (consumed by Task 4): `BeatAttackView`, `BeatDamageView`,
  `VerdictLabel`, `verdictLabel()` (`beatStageTypes.ts`); `Placement`,
  `BeatStageProps`, the `BeatStage` component (`BeatStage.tsx`).

- [ ] **Step 1: Create the presentation-owned types module**

Create `src/concepts/combat-pacing/beatStageTypes.ts`:

```ts
/**
 * Presentation-owned types for `BeatStage.tsx` (rpg-dnd5e-web#561),
 * split into their own module for the same reason
 * `src/components/game/equipment/equipmentTypes.ts` is split out from
 * `EquipmentSlots.tsx`/`InventoryLight.tsx`: a component file that also
 * exports plain functions/types trips this repo's
 * `react-refresh/only-export-components` lint rule (verified — moving
 * `verdictLabel` out of `BeatStage.tsx` and into this file is what
 * cleared `npm run lint`).
 *
 * `BeatAttackView`/`BeatDamageView` are deliberately NOT imported from
 * `./fixtures` — see `BeatStage.tsx`'s header for why. `fixtures.ts`'s
 * `AttackResolvedLike`/`EntityDamagedLike` are a structural superset of
 * these (same field names/types, plus more), so passing one where a
 * `BeatAttackView`/`BeatDamageView` is expected needs no adapter or cast.
 */

/** The ONLY fields `BeatStage` reads off an attack. */
export interface BeatAttackView {
  attackerEntityId: string;
  hit: boolean;
  critical: boolean;
  attackRoll: number;
  attackBonus: number;
  targetAc: number;
}

/** The ONLY field `BeatStage` reads off a damage event. */
export interface BeatDamageView {
  amount: number;
}

export type VerdictLabel = 'HIT' | 'MISS' | 'CRIT' | 'NAT-1' | '';

export function verdictLabel(attack?: BeatAttackView): VerdictLabel {
  if (!attack) return '';
  if (attack.critical) return 'CRIT';
  if (attack.attackRoll === 1 && !attack.hit) return 'NAT-1';
  return attack.hit ? 'HIT' : 'MISS';
}
```

- [ ] **Step 2: Write the failing test file**

Create `src/concepts/combat-pacing/BeatStage.test.tsx`:

```tsx
import { render, screen } from '@testing-library/react';
import { describe, expect, it } from 'vitest';
import { BeatStage } from './BeatStage';
import type { BeatAttackView, BeatDamageView } from './beatStageTypes';

const hitAttack: BeatAttackView = {
  attackerEntityId: 'char-aldric',
  hit: true,
  critical: false,
  attackRoll: 14,
  attackBonus: 5,
  targetAc: 16,
};

const critAttack: BeatAttackView = {
  ...hitAttack,
  critical: true,
  attackRoll: 20,
};
const nat1Attack: BeatAttackView = {
  ...hitAttack,
  hit: false,
  critical: false,
  attackRoll: 1,
};

const dmg: BeatDamageView = { amount: 7 };

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
    expect(screen.getByTestId('beat-die').className).toContain(
      'beat-die--tumbling'
    );
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
    expect(screen.getByTestId('beat-die').className).toContain(
      'beat-die--settled'
    );
  });

  it('shows HIT with the beat-verdict--hit class for a plain hit verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    const el = screen.getByTestId('beat-verdict');
    expect(el.textContent).toContain('HIT');
    expect(el.className).toContain('beat-verdict--hit');
  });

  it('shows MISS with the beat-verdict--miss class for a miss verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={{ ...hitAttack, hit: false }}
        reducedMotion={false}
      />
    );
    const el = screen.getByTestId('beat-verdict');
    expect(el.textContent).toContain('MISS');
    expect(el.className).toContain('beat-verdict--miss');
  });

  it('shows CRIT with the gold beat-verdict--crit class for a critical verdict', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={critAttack}
        reducedMotion={false}
      />
    );
    const el = screen.getByTestId('beat-verdict');
    expect(el.textContent).toContain('CRIT');
    expect(el.className).toContain('beat-verdict--crit');
  });

  it('shows NAT-1 with the red beat-verdict--nat1 class, distinct from CRIT and MISS', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={nat1Attack}
        reducedMotion={false}
      />
    );
    const el = screen.getByTestId('beat-verdict');
    expect(el.textContent).toContain('NAT-1');
    expect(el.className).toContain('beat-verdict--nat1');
    expect(el.className).not.toContain('beat-verdict--crit');
    expect(el.className).not.toContain('beat-verdict--miss');
  });

  it('renders the damage number, oversized/gold (beat-damage--crit) only on a crit', () => {
    render(
      <BeatStage
        beat="impact"
        placement="token-anchored"
        attack={critAttack}
        damage={dmg}
        reducedMotion={false}
      />
    );
    const el = screen.getByTestId('beat-damage');
    expect(el.textContent).toContain('7');
    expect(el.className).toContain('beat-damage--crit');
  });

  it('renders a plain (non-crit) damage number without beat-damage--crit', () => {
    render(
      <BeatStage
        beat="impact"
        placement="token-anchored"
        attack={hitAttack}
        damage={dmg}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-damage').className).not.toContain(
      'beat-damage--crit'
    );
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
    expect(
      screen.getByTestId('beat-stage').getAttribute('data-placement')
    ).toBe('center-stage');
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
    expect(
      screen.getByTestId('beat-stage').getAttribute('data-placement')
    ).toBe('center-stage');
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
    expect(
      screen.getByTestId('beat-stage').getAttribute('data-placement')
    ).toBe('center-stage');
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
    expect(
      screen.getByTestId('beat-stage').getAttribute('data-placement')
    ).toBe('token-anchored');
  });

  it('applies beat-stage--reduced-motion on the container when reducedMotion is set', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={critAttack}
        reducedMotion
      />
    );
    expect(screen.getByTestId('beat-stage').className).toContain(
      'beat-stage--reduced-motion'
    );
  });

  it('does not apply beat-stage--reduced-motion when reducedMotion is false', () => {
    render(
      <BeatStage
        beat="verdict"
        placement="token-anchored"
        attack={hitAttack}
        reducedMotion={false}
      />
    );
    expect(screen.getByTestId('beat-stage').className).not.toContain(
      'beat-stage--reduced-motion'
    );
  });
});
```

**Note on what these tests do and don't prove:** jsdom (Vitest's test
environment here) does not load `public/themes/base.css` or apply real
CSS — these tests assert that the RIGHT class names land on the right
elements, not that the CSS behind those class names looks correct. That
second half is exactly what Task 5's mandatory visual-verification step
covers with real screenshots; treat both as required, not either/or.

- [ ] **Step 3: Run the test to verify it fails**

Run: `npm run test:run -- src/concepts/combat-pacing/BeatStage.test.tsx`

Expected: FAIL — `Error: Failed to resolve import "./BeatStage" from
"src/concepts/combat-pacing/BeatStage.test.tsx". Does the file exist?`

- [ ] **Step 4: Implement the component**

Create `src/concepts/combat-pacing/BeatStage.tsx`:

```tsx
/**
 * BeatStage (rpg-dnd5e-web#561) — the presentational die/verdict/damage
 * surface for one beat. Takes ONLY beat-shaped props via its OWN
 * presentation-owned `BeatAttackView`/`BeatDamageView` types
 * (`beatStageTypes.ts`, split into its own module because a component
 * file that also exports plain functions/types trips this repo's
 * `react-refresh/only-export-components` lint rule — the same reason
 * `equipmentTypes.ts` is split from `EquipmentSlots.tsx`). This file does
 * NOT import `AttackResolvedLike`/`EntityDamagedLike` from `./fixtures`.
 * `fixtures.ts`'s real wire-shaped types are a structural SUPERSET of
 * `beatStageTypes.ts`'s (every field there also exists here, same
 * name/type), so `CombatPacingConcept.tsx` can pass a `BeatGroupResult`'s
 * `attack`/`damage` straight through with no adapter function and no
 * cast — TypeScript's structural typing accepts a wider object wherever
 * a narrower shape is declared. This keeps the promotion story true
 * (design.md §7, "reusable presentation components accept only
 * presentation props"): swapping the data source later (fixtures -> a
 * live-stream reassembler) never requires touching this file, because
 * this file was never coupled to the fixture module's types in the
 * first place — only to its own.
 *
 * Styling reuses two already-shipped `public/themes/base.css`
 * conventions rather than inventing new ones:
 *   - `@keyframes dice-roll` / `.animate-dice-roll` (already used by the
 *     live `src/components/DiceRoller.tsx`) for the Throw beat's tumble.
 *   - The crit/hit/miss color palette `src/components/game/CombatLog.tsx`
 *     already renders combat text in (`#facc15` gold / `#f87171` hit /
 *     `#9ca3af` miss, see its `lineStyle()`) — reused verbatim so the log
 *     and this beat stage never disagree about what a color means. This
 *     also fixes an earlier draft's incorrect claim that gold-for-crit
 *     was "future work" — it ships in this task, defined in
 *     `public/themes/base.css` (see Task 3 Step 4).
 *
 * Every SFX-worthy moment is marked with an "SFX slot" comment (design.md
 * §5: "audio hooks only — no actual sound implementation ships round
 * one").
 */

import type {
  BeatAttackView,
  BeatDamageView,
  VerdictLabel,
} from './beatStageTypes';
import { verdictLabel } from './beatStageTypes';
import type { BeatName } from './useBeatSequencer';

export type Placement = 'token-anchored' | 'center-stage';

export interface BeatStageProps {
  beat: BeatName;
  placement: Placement;
  attack?: BeatAttackView;
  damage?: BeatDamageView;
  reducedMotion: boolean;
}

function verdictModifier(label: VerdictLabel): string {
  switch (label) {
    case 'CRIT':
      return 'crit';
    case 'NAT-1':
      return 'nat1';
    case 'HIT':
      return 'hit';
    case 'MISS':
      return 'miss';
    default:
      return '';
  }
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
  const modifier = verdictModifier(label);
  const isCrit = label === 'CRIT';

  const stageClassName = [
    'beat-stage',
    `beat-stage--${effectivePlacement}`,
    reducedMotion ? 'beat-stage--reduced-motion' : '',
  ]
    .filter(Boolean)
    .join(' ');

  return (
    <div
      data-testid="beat-stage"
      data-beat={beat}
      data-placement={effectivePlacement}
      className={stageClassName}
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
            className={`beat-verdict beat-verdict--${modifier}`}
          >
            {label} ({attack.attackRoll}+{attack.attackBonus} vs AC{' '}
            {attack.targetAc})
          </div>
        )}

      {beat === 'impact' && damage && (
        // SFX slot: impact thud, oversized/gold-tinted on a crit.
        <div
          data-testid="beat-damage"
          className={`beat-damage${isCrit ? ' beat-damage--crit' : ''}`}
        >
          -{damage.amount}
        </div>
      )}
    </div>
  );
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `npm run test:run -- src/concepts/combat-pacing/BeatStage.test.tsx`

Expected: PASS — `Tests  16 passed (16)`.

- [ ] **Step 6: Add the real styling to `public/themes/base.css`**

Append to `public/themes/base.css` (verified against this repo's actual
file — it currently ends right after a `.dice-modifier` rule; append
directly below that):

```css

/* Combat-pacing beat stage (rpg-dnd5e-web#561) — reuses this file's own
   .animate-dice-roll/@keyframes dice-roll (already shipped for
   src/components/DiceRoller.tsx) for the Throw beat's tumble, and
   CombatLog.tsx's existing crit-gold (#facc15) / hit (#f87171) / miss
   (#9ca3af) palette (src/components/game/CombatLog.tsx's lineStyle())
   for verdict differentiation — NOT this file's unrelated equipment-
   rarity --legendary token, which is theme-varying and renders RED in
   dark-fantasy.css, the wrong semantic for a gold crit. Adds only what's
   new: a nat-1 red "crack" wobble (distinct from both a gray miss and a
   gold crit), an oversized-crit damage pop, and the token-anchored/
   center-stage placement treatment. No @media query — this codebase has
   none (verified); narrow-width behavior instead comes from flexible
   sizing (clamp()/max-width: 100%) plus the concept page's own flex-wrap
   layout, the same pattern combat-panel/equip-bench already use. */

.beat-stage {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 0.5rem;
  min-height: 96px;
  padding: 0.75rem;
  border-radius: 0.5rem;
  background-color: var(--card-bg);
  border: 2px solid var(--border-primary);
  max-width: 100%;
  transition: border-color 0.2s ease;
}

.beat-stage--center-stage {
  border-color: var(--accent-primary);
}

.beat-cue {
  font-size: clamp(0.75rem, 2.5vw, 0.9375rem);
  color: var(--text-secondary);
  animation: armed-pulse 1.2s ease-in-out infinite;
}

.beat-die {
  font-size: clamp(1.5rem, 6vw, 2.5rem);
  line-height: 1;
}

.beat-die--tumbling {
  animation: dice-roll 0.6s cubic-bezier(0.68, -0.55, 0.265, 1.55) infinite;
}

.beat-die--settled {
  animation: none;
  transform: none;
}

.beat-verdict {
  font-family: 'Cinzel', serif;
  font-weight: 700;
  font-size: clamp(0.875rem, 3vw, 1.25rem);
  padding: 0.25rem 0.75rem;
  border-radius: 0.375rem;
  text-align: center;
}

/* Palette matches CombatLog.tsx's lineStyle() exactly so the log and this
   beat stage never disagree on what a color means. */
.beat-verdict--hit {
  color: #f87171;
  background-color: color-mix(in srgb, #f87171 12%, transparent);
}

.beat-verdict--miss {
  color: #9ca3af;
  background-color: color-mix(in srgb, #9ca3af 10%, transparent);
}

/* Crit gold treatment (design.md §1: "die glows gold... damage number is
   oversized and gold"). */
.beat-verdict--crit {
  color: #facc15;
  background-color: color-mix(in srgb, #facc15 18%, transparent);
  box-shadow: 0 0 12px color-mix(in srgb, #facc15 55%, transparent);
  animation: glow 1.4s ease-in-out infinite;
}

/* Nat-1: playful, non-punitive (design.md §1) — a red "crack" wobble
   distinct from both a plain miss (gray, no animation) and a crit (gold,
   glow). Short and comedic, not a long frame-break. */
.beat-verdict--nat1 {
  color: #ef4444;
  background-color: color-mix(in srgb, #ef4444 12%, transparent);
  animation: nat1-crack 0.4s ease-in-out 1;
}

@keyframes nat1-crack {
  0%,
  100% {
    transform: rotate(0deg);
  }
  25% {
    transform: rotate(-6deg);
  }
  75% {
    transform: rotate(6deg);
  }
}

.beat-damage {
  font-family: 'Cinzel', serif;
  font-weight: 700;
  font-size: clamp(1rem, 4vw, 1.5rem);
  color: #f87171;
}

/* Oversized + gold for a crit's damage pop (design.md §1). */
.beat-damage--crit {
  font-size: clamp(1.5rem, 6vw, 2.25rem);
  color: #facc15;
  text-shadow: 0 0 10px color-mix(in srgb, #facc15 60%, transparent);
}

/* Reduced motion (design.md §4): keeps every beat's semantics (colors,
   labels, the crit glow's box-shadow) but drops motion — the tumble
   (handled by .beat-die--settled above), the nat-1 wobble, and the crit
   glow's pulsing all stop; a static gold/red state remains. */
.beat-stage--reduced-motion .beat-verdict--crit {
  animation: none;
}

.beat-stage--reduced-motion .beat-verdict--nat1 {
  animation: none;
}
```

There is no automated test for this step — Vitest's jsdom environment
does not load `public/themes/base.css` (Step 2's note above). Confirm
this file still parses as valid CSS by starting the dev server and
checking the browser console for a stylesheet parse error:

```bash
npm run dev
```

Expected: no CSS parse errors in the browser console at
`http://localhost:5173/concepts`. The actual look (tumble, gold crit,
red nat-1, etc.) is verified with real screenshots in Task 5 Step 5 —
this step only confirms the file is syntactically valid before that.

- [ ] **Step 7: Typecheck + lint**

Run: `npm run typecheck && npm run lint`

Expected: no errors. If `verdictLabel`/`BeatAttackView`/`BeatDamageView`
were defined directly inside `BeatStage.tsx` instead of
`beatStageTypes.ts`, this step is where `npm run lint` would fail with
`Fast refresh only works when a file only exports components
(react-refresh/only-export-components)` — confirmed by deliberately
trying that during this plan's own verification pass; Step 1's file
split is what keeps this step clean.

- [ ] **Step 8: Commit**

```bash
git add src/concepts/combat-pacing/beatStageTypes.ts \
  src/concepts/combat-pacing/BeatStage.tsx \
  src/concepts/combat-pacing/BeatStage.test.tsx \
  public/themes/base.css
git commit -m "feat(concepts): combat-pacing BeatStage + styling (#561)"
```

---
## Task 4: `CombatPacingConcept` — wiring, comparison layout, and registration

**Files:**
- Create: `src/concepts/combat-pacing/CombatPacingConcept.test.tsx`
- Create: `src/concepts/combat-pacing/CombatPacingConcept.tsx`
- Modify: `src/concepts/ConceptsView.tsx`

**Interfaces:**
- Consumes: `SCENARIOS`, `Pace`, `CombatPacingScenario` from `./fixtures`
  (Task 1); `useBeatSequencer` from `./useBeatSequencer` (Task 2);
  `BeatStage` from `./BeatStage` (Task 3) — `seq.group?.attack`/
  `seq.group?.damage` (typed `AttackResolvedLike`/`EntityDamagedLike` per
  Task 1) are passed directly into `BeatStage`'s `attack`/`damage` props
  (typed `BeatAttackView`/`BeatDamageView` per Task 3) with no adapter —
  see Task 3's header for why that's safe.
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

  it('the instant pace override drives the sequencer straight to done', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('pace-override-instant'));
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('done');
  });

  it('a cinematic pace override progresses beyond cue (object-identity regression guard)', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('scenario-button-npc-grunt-swing')); // spectator, no armed wait
    fireEvent.click(screen.getByTestId('pace-override-cinematic'));
    act(() => {
      vi.advanceTimersByTime(150); // Cinematic cue, not stuck resetting at cue
    });
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('throw');
  });

  it('a brisk pace override progresses beyond cue too (object-identity regression guard)', () => {
    render(<CombatPacingConcept />); // default scenario player-hit, role: self
    fireEvent.click(screen.getByTestId('pace-override-brisk'));
    act(() => {
      vi.advanceTimersByTime(75); // Brisk cue
    });
    const stage = screen.getAllByTestId('beat-stage')[0];
    expect(stage.getAttribute('data-beat')).toBe('armed'); // NOT stuck at 'cue'
  });

  it('toggling reduced motion while a cinematic override is active still progresses (compound regression guard)', () => {
    render(<CombatPacingConcept />);
    fireEvent.click(screen.getByTestId('pace-override-cinematic'));
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

  it('the reduced-motion toggle is wired to both beat stages (no override active)', () => {
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
 *
 * Viewport frames EXTEND `combat-panel/CombatPanelConcept.tsx`'s FRAMES
 * pattern (floor/typical/full) with one addition: a `narrow` frame below
 * the 1024×768 floor, to prove the non-breaking fallback design.md §8
 * requires.
 *
 * `effectiveScenario` is `useMemo`'d on `[scenario, paceOverride]` — both
 * primitive-or-stable-reference deps — rather than recomputed as a fresh
 * object literal every render. Without this, selecting any NON-default
 * pace override would spread a brand-new `{ ...scenario, pace }` object
 * on every render; `useBeatSequencer`'s own `useEffect(() => {...},
 * [scenario])` would then see a "changed" argument on every re-render
 * (including ones the sequencer's OWN internal timers trigger) and reset
 * back to `cue` in a loop, so the beat would never progress past `cue`
 * whenever an override was active. `useBeatSequencer.test.tsx`'s
 * dedicated regression tests below cover this directly.
 */

import { useMemo, useState } from 'react';
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
const PACE_OVERRIDES: PaceOverride[] = [
  'default',
  'cinematic',
  'brisk',
  'instant',
];

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

  // See file header — this useMemo is load-bearing, not decorative.
  const effectiveScenario = useMemo(
    () =>
      paceOverride === 'default'
        ? scenario
        : { ...scenario, pace: paceOverride },
    [scenario, paceOverride]
  );

  const seq = useBeatSequencer(effectiveScenario, { reducedMotion });

  const selectScenario = (id: string) => setScenarioId(id);

  return (
    <div>
      <p style={{ color: 'var(--text-muted)', marginBottom: 12, fontSize: 14 }}>
        Round-one bench for the attack-loop beat model (web#561). Both
        placements render side by side against the SAME fixture — token-
        anchored (left) promotes to center-stage on a crit/nat-1; the pure
        center-stage placement (right) never moves. Pace override and reduced
        motion apply to both. `/concepts` needs no backend.
      </p>

      <div
        style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 8 }}
      >
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

      <div
        style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 8 }}
      >
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

      <div
        style={{ display: 'flex', gap: 8, flexWrap: 'wrap', marginBottom: 12 }}
      >
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
          flexWrap: 'wrap',
          background: 'var(--bg-primary)',
        }}
      >
        <div style={{ flex: 1, minWidth: 160 }}>
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
        <div style={{ flex: 1, minWidth: 160 }}>
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
        <button
          data-testid="skip-button"
          style={chipStyle(false)}
          onClick={seq.skip}
        >
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

Expected: PASS — `Tests  11 passed (11)`.

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

Expected: all pass, 0 regressions in any pre-existing file. Verified
directly during this plan's authoring against a disposable worktree off
`origin/main` (commit `29bdce1`): `Test Files  63 passed (63)`,
`Tests  1096 passed (1096)` — the pre-existing suite plus this plan's 53
new tests (14 fixtures + 12 sequencer + 16 BeatStage + 11 concept), zero
regressions.

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

Create `src/concepts/combat-pacing/CONTRACT.md`, borrowing
`src/concepts/equipment/CONTRACT.md`'s STRUCTURE (numbered observations,
each with a why) — but NOT that file's CONTENT stance. The committed
`equipment/CONTRACT.md` is itself already ask-shaped (literally titled
"Equipment contract request — what the wire needs"), because it was
written and committed AFTER Kirk's review confirmed the gaps were real
(its `fixtures.ts` header even notes the asks it made "landed as
rpg-api-protos#188/rpg-toolkit#812/rpg-api#682"). This file instead
starts evidence-only, matching design.md §6's stated lifecycle for THIS
round — porting design.md §6 into that numbered-with-a-why shape:

```markdown
# Combat-pacing contract log — evidence, not asks yet (rpg-dnd5e-web#561)

Produced by the fixture-first combat-pacing concept (`/concepts` →
Combat Pacing). Borrows `src/concepts/equipment/CONTRACT.md`'s
STRUCTURE (numbered observations, each with a why) — but unlike that
file (which is already ask-shaped because it was committed AFTER Kirk's
review confirmed real gaps), THIS file records evidence, observations,
and candidate gaps during the concept, before any such review — it is
explicitly NOT a pre-authored feature request. Filing anything on the
Platform lane of board #19 happens only after Kirk reviews this concept
and confirms a candidate is a real, scoped need (design.md §6 intro) —
at which point a LATER revision of this file would read like the
equipment one does today.

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
numbered-with-a-why STRUCTURE the equipment concept's (now-ask-shaped)
CONTRACT.md uses, adding only the concrete confirmation that this
concept's own fixture/hook/component code needed no invented wire field
to build the round-one bench — filing anything on Platform's board-#19
lane happens only after Kirk reviews this concept, at which point this
file's next revision would read like the equipment one does today.
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
gap-log STRUCTURE, but stays evidence-only (not yet ask-shaped like the
equipment file) — no Platform issue filed by this round.
```

Also update the frontmatter `updated:` field at the top of the file.

- [ ] **Step 4: Run the full CI check**

Run: `npm run ci-check`

Expected: all steps pass — format check clean, lint clean, typecheck
clean, build succeeds, full test suite green (no regressions in any file
outside the ones touched in Tasks 1-4 plus this task's two doc edits).

- [ ] **Step 5: MANDATORY visual verification — styling, placement, and manual pace control**

`/concepts` needs no backend (fixture-first, per design.md §7) — this is
simpler than a live-route check. Start the dev server:

```bash
npm run dev
```

Using the chrome-devtools MCP tools (or equivalent), navigate to
`http://localhost:5173/concepts`, click the "Combat Pacing" tab, and
work through EVERY item below, capturing a screenshot and viewing it
directly (not just "the test passed" — an actual look, per this repo's
real-route evidence convention). Unit tests only proved the right class
names land on the right elements (Task 3 Step 2's note) — this step is
what proves the CSS behind those classes actually looks right:

1. **Die tumble (Throw beat).** On `player-hit` (default scenario, 1024×768
   floor), advance to `armed`, click 🎲 Throw, and watch the die during
   `throw` — confirm it visibly spins/scales (the `dice-roll` keyframe),
   not a static emoji.
2. **Crit gold (Verdict/Impact beats).** Click the `player-crit` scenario
   button, click Skip once to reach `verdict` — confirm the verdict stamp
   reads **CRIT** in gold with a visible glow, then skip again to
   `impact` and confirm the damage number is oversized AND gold (not the
   plain red `beat-damage` color).
3. **Nat-1 red-crack (Verdict beat).** Click `player-nat1`, skip to
   `verdict` — confirm the stamp reads **NAT-1** in red with a brief
   wobble, VISUALLY DISTINCT from both the crit's gold glow and a plain
   miss's flat gray.
4. **Hit vs. miss differentiation.** Click `player-hit` and `player-miss`
   in turn, skip each to `verdict` — confirm HIT renders in the red-ish
   hit color and MISS renders in flat gray, matching
   `CombatLog.tsx`'s existing palette (open the live combat log
   component or its screenshot from a prior evidence doc side by side if
   useful, to visually cross-check the colors agree).
5. **Placement/promotion.** Still on `player-crit` or `player-nat1` at
   `verdict`, confirm the LEFT (token-anchored) column visually matches
   the RIGHT (center-stage) column — the promotion border/treatment
   should read the same on both, proving `data-placement="center-stage"`
   isn't just an attribute but an actual visual match. Then switch to
   `player-hit` (no promotion) and confirm the left column now reads
   differently from the right (still token-anchored, not promoted).
   Capture both states as
   `docs/evidence/combat-pacing-561-token-anchored.png` and
   `docs/evidence/combat-pacing-561-center-stage.png`.
6. **Reduced motion.** Click "Reduced motion: on", re-run the
   `player-crit` sequence — confirm the die shows settled (no spin)
   during what would be `throw`, and the crit glow/nat-1 wobble no
   longer animate (a static gold/red state remains, per design.md §4:
   "nothing about what happened is lost, only the motion"). Toggle back
   off and confirm the animations return.
7. **Manual pace-control verification (not just the unit-test regression
   guard).** Click each of the four pace-override buttons in turn
   (`default`, `cinematic`, `brisk`, `instant`) on at least one scenario
   and confirm the beat visibly progresses through cue/throw/verdict/etc
   rather than appearing frozen at "cue" — this is the by-eye check for
   the object-identity bug Task 4's `useMemo` fixes; a regression here
   would look like the beat display never leaving its "Group 1 / 1 —
   beat: cue" line no matter how long you wait.
8. **1024×768 floor / 1440×900 typical / 1920×1080 full / 480×640
   narrow.** Click each frame button in turn — confirm the layout
   breathes upward at the larger frames (no fixed/clipped elements) and,
   at the narrow fallback, the two `BeatStage` columns still render
   (wrapped/stacked is an acceptable non-breaking fallback; an
   overlapping or clipped column is not). Save the narrow view as
   `docs/evidence/combat-pacing-561-narrow.png`.

If any item above fails — the die doesn't visibly move, the crit doesn't
read gold, the nat-1 doesn't read distinctly red, hit/miss look the
same, promotion doesn't visually match center-stage, reduced motion
doesn't actually stop the animations, a pace override leaves the display
stuck at "cue", or a column clips/overlaps at any frame — that is a real
regression. Stop and debug before writing the evidence file below; do
not paper over a layout or styling problem with a passing unit test.

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

[Fill in after Step 5, covering each of that step's 8 checks: die
tumble during Throw, crit gold verdict/damage, nat-1 red-crack (visually
distinct from both crit gold and a plain miss), hit-vs-miss color
differentiation against `CombatLog.tsx`'s palette, token-anchored vs.
center-stage promotion match on `player-crit`/`player-nat1`, reduced
motion actually suppressing the tumble/wobble/glow, manual pace-override
clicking visibly progressing (not stuck at "cue") on at least one
non-default pace, and all 4 frames (narrow/floor/typical/full) — plus an
honest note on anything that read cluttered, clipped, or otherwise not
ready, matching the honesty bar the `wall-fittings-536.md` precedent set
for this repo's evidence docs.]

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
  --body "Round-one \`/concepts\` bench for the attack-loop beat model, following the equipment concept's fixture-first pattern (#557). Fixture-driven \`useBeatSequencer\` (Cue→Throw→Verdict→Impact→Release, pace-derived durations, crit stretch, auto-throw timeout, tap-to-skip, repeat-roll compression) plus \`BeatStage\` (its own presentation-owned types, real CSS: die tumble/settle, crit gold, nat-1 red-crack, hit/miss color, oversized crit damage) rendered in BOTH token-anchored and center-stage placements side by side against identical fixtures, an event/intent inspector, and an evidence-only CONTRACT.md gap log (no Platform ask filed). Design: rpg-project's \`ideas/combat-pacing/design.md\` + \`plan.md\`. Closes no issue by itself — round one does not touch the live encounter route (rpg-dnd5e-web#561, Board 19).

— asset-pipeline agent, on behalf of KirkDiggler"
```

Expected: PR opens against `main`, CI runs and passes (same checks as
`npm run ci-check`), no direct commits to `main`.

---

## Revision note (independent review, this pass)

This plan was revised against an independent review that found real
issues in the first version. Rather than list them abstractly, every
fix below was actually executed against real code in a disposable
worktree off `origin/main` (deleted afterward, no residue) before being
folded back into this document — see each item for what was verified:

1. **Task 3 had no real styling deliverable.** Added `beatStageTypes.ts`
   + full CSS in `public/themes/base.css` (die tumble via the existing
   `dice-roll` keyframe, crit gold / hit / miss via `CombatLog.tsx`'s
   existing palette, a new nat-1 red-crack keyframe, oversized crit
   damage, placement border treatment, reduced-motion suppression). The
   earlier draft's comment claiming gold-for-crit was "future work" was
   wrong and is removed. `BeatStage.test.tsx` grew from 12 to 16 tests
   covering the new classes; Task 5 Step 5 grew to 8 explicit visual
   checks (tumble, gold crit, red nat-1, hit/miss, promotion, reduced
   motion, manual pace control, all 4 frames).
2. **The double-`skip()` test was a real synchronization bug, not just a
   test bug.** Fixed by having `useBeatSequencer`'s internal decisions
   (`skip`/`throwDie`/`finishGroup`) read `beatRef`/`groupIndexRef` (plain
   refs, updated synchronously) instead of the closured `beat`/
   `groupIndex` React state — state now exists only to trigger
   re-renders. Verified: the exact planned test file
   (`useBeatSequencer.test.ts`, 12 tests including the two-synchronous-
   skips case) passes 12/12 against the fixed hook.
3. **The pace-override object-identity bug was real.** `effectiveScenario`
   is now `useMemo`'d on `[scenario, paceOverride]`. Verified TWICE: the
   fixed version passes `CombatPacingConcept.test.tsx`'s 11 tests
   (including 3 new regression tests for cinematic/Brisk overrides and a
   compound reduced-motion case); temporarily reverting to the un-memoized
   version and re-running the SAME test file failed exactly those 3 tests
   (8/11 passing) — confirming the tests actually catch the bug, not just
   coincidentally pass.
4. **`BeatStage` no longer imports fixture types.** `beatStageTypes.ts`
   now owns `BeatAttackView`/`BeatDamageView`/`VerdictLabel`/
   `verdictLabel()`; `fixtures.ts`'s `AttackResolvedLike`/
   `EntityDamagedLike` satisfy these structurally (verified: `npm run
   typecheck` passes with zero errors once Task 4 wires a
   `BeatGroupResult`'s fields straight into `BeatStage`'s props with no
   cast). This split was also load-bearing for a real lint rule, not
   just a style preference — see item 6.
5. **`CONTRACT.md` precedent wording corrected.** The committed
   `equipment/CONTRACT.md` is itself already ask-shaped (post-review);
   combat-pacing's file borrows its STRUCTURE only and stays
   evidence-only per design.md §6's lifecycle for this round. Fixed in
   Global Constraints, the File Map, Task 5 Step 1, and Task 5 Step 3's
   docs paragraph.
6. **Test counts recounted for real.** Final, verified totals: 14
   (`fixtures.test.ts`) + 12 (`useBeatSequencer.test.ts`) + 16
   (`BeatStage.test.tsx`) + 11 (`CombatPacingConcept.test.tsx`) = 53 new
   tests, plus the pre-existing suite: `npm run test:run` reports `Test
   Files  63 passed (63)`, `Tests  1096 passed (1096)` with all four new
   files present, zero regressions. Every "Expected: PASS — Tests N
   passed (N)" line in Tasks 1-4 matches these counts exactly.
7. **Minor accuracy fixes:** `HitPointsLike.temp` is now `temp: number`
   (required, matching the wire exactly), with every fixture supplying
   `temp: 0` explicitly. `ActionResolvedLike`'s omission of
   `economyConsumed` and `PacingFixtureEvent.sequence`'s use of `number`
   instead of the wire's `bigint` are both now explained inline in
   `fixtures.ts`'s header docstring (ergonomics for a fixture bench with
   under 10 events per scenario, not a claim that they're unimportant).
8. **Branch/PR convention citation corrected.** No longer attributes the
   "one issue per PR, branch off `main`" convention to `rpg-project`'s
   `AGENTS.md` (which governs boards #11/#13, not `rpg-dnd5e-web`'s board
   #19). Now cites `rpg-dnd5e-web`'s own OBSERVED convention, verified via
   `gh pr list`: #557, #575, #567, #566 are all single-issue branches off
   `origin/main`.
9. **Viewport-frame wording corrected** from "mirroring" to "EXTENDING"
   `combat-panel/CombatPanelConcept.tsx`'s `FRAMES` pattern (this plan
   adds a 4th, `narrow`, frame that pattern doesn't have) — fixed in the
   File Map and Task 4's header.
10. A build-only TypeScript error surfaced during verification and is
    folded into Task 2 Step 3's code directly (not left as a separate
    fixup step): `useRef<ReturnType<typeof setTimeout>>()` needs an
    explicit `| undefined` type argument and initial value under this
    repo's `tsc -b` project-reference build (stricter than the
    `typecheck` script's plain `tsc --noEmit`) — verified `npm run build`
    passes with the corrected `useRef<... | undefined>(undefined)`.

## Self-review (performed by the plan author, not a task for the implementer)

- **Spec coverage:** every one of the task's required elements is
  present: required sub-skill header ✓; Goal/Architecture/Tech
  Stack/Global Constraints ✓; exact file map ✓ (table above, now
  including `beatStageTypes.ts` and the `base.css` modification); 5
  independently-testable TDD tasks (Tasks 1-4 are strict red/green TDD;
  Task 5 is the equipment-concept-precedented docs/CI/evidence/PR
  wrap-up task, matching `item-icons/plan.md`'s own Task 3 shape) ✓;
  exact current web paths/imports/types verified by reading the live
  `rpg-dnd5e-web` checkout AND by actually running the code in a
  disposable worktree (not guessed) ✓; test commands
  (`npm run test:run -- <path>`, `npm run typecheck`, `npm run lint`,
  `npm run ci-check`) match `package.json` exactly, and were themselves
  run for real ✓; concrete code in every step, no vague instructions ✓;
  expected fail/pass output stated per step and matches real, executed
  counts ✓; commit steps after every task ✓; `CONTRACT.md` lifecycle
  correctly distinguished from the equipment file's (now ask-shaped, not
  evidence-only) precedent ✓; visual verification covers every one of
  the 6 critical-item styling/behavior requirements plus manual pace
  control, at 1024×768 floor + larger + narrow fallback, as a mandatory,
  gated step (Task 5 Step 5) ✓. Scope exclusions (no live-stream
  reassembly, no Platform/proto/API/toolkit changes, no actual audio, no
  damage dice, no handoff/promotion) are each called out explicitly in
  Global Constraints and never crossed by any task's file list.
- **Placeholder scan:** no "TBD"/"add appropriate handling"/"similar to
  Task N" language anywhere in Tasks 1-5's code steps. The one
  intentionally-unfilled prose block (Task 5 Step 6's evidence-doc "What
  I viewed" section) is explicitly flagged as a required fill-in tied to
  Step 5's own 8-item checklist, not a silent gap.
- **Type/name consistency:** traced every cross-task interface by hand,
  re-verified after introducing `beatStageTypes.ts` specifically —
  `PacingFixtureEvent`/`CombatPacingScenario`/`BeatGroupResult`/
  `groupByCorrelation` (Task 1) are the exact names imported in Task 2;
  `BeatName`/`BeatSequencerState`/`useBeatSequencer` (Task 2) are the
  exact names imported in Tasks 3-4; `BeatAttackView`/`BeatDamageView`/
  `VerdictLabel`/`verdictLabel()` (Task 3's `beatStageTypes.ts`) are the
  exact names `BeatStage.tsx`/`BeatStage.test.tsx` import, and are NEVER
  imported from `fixtures.ts`; `Placement`/`BeatStageProps`/`BeatStage`
  (Task 3) are the exact names imported in Task 4, which passes
  `AttackResolvedLike`/`EntityDamagedLike`-typed values into
  `BeatAttackView`/`BeatDamageView`-typed props with no adapter — checked
  by actually running `npm run typecheck` against the wired-together
  code, zero errors. No renamed field slipped through (e.g. `attackRoll`
  is spelled identically everywhere it appears).
- **Path/command accuracy:** every file path was confirmed to exist (or,
  for new files, confirmed its parent directory's sibling convention) by
  reading the live `rpg-dnd5e-web` checkout, AND every code block in
  Tasks 1-4 is the exact content that was written to a disposable
  worktree and run for real (`npm run test:run`, `typecheck`, `lint`,
  `format:check`, `build`, `ci-check` — all green). `public/themes/
  base.css`'s append point (right after the existing `.dice-modifier`
  rule) was confirmed against the live file, not assumed.
- **Scope:** no task touches `rpg-api`, `rpg-toolkit`, `rpg-api-protos`,
  `encounterStreamDispatch.ts`, `useEncounterStream.ts`, or any live
  encounter component; no audio asset/library added; no damage-dice
  animation; no wiring into `EncounterView`; no new runtime dependency
  (styling is plain CSS, no animation library).
