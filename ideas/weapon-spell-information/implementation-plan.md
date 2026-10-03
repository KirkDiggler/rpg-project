# Effect information — implementation handoffs

**Readiness: partial.** C1 is implemented in draft
[toolkit#1932](https://github.com/KirkDiggler/rpg-toolkit/pull/1932) at `f94ed5a7`;
its owning-module checks pass. C2
[toolkit#1933](https://github.com/KirkDiggler/rpg-toolkit/pull/1933) is merged as
`f06cacf2`; CI published `rulebooks/dnd5e/encounter/v0.111.0`, verified to point at
that merge. Its encounter-module race tests/lint and normal commit hook passed.
Consumer adoption and joined acceptance remain pending. The full shared
assessment/read/UI wave is not yet an executable handoff; remaining contracts
below stay explicit. These provider checkpoints do not complete #520.

## Goal, authority and constraints

Show provider-assessed effects for the current action/inspected target, with
canonical tooltips; preserve gray/ineligible, conditional and optional states.
Hover/focus/touch inspection is read-only, and clicking retains existing commands.
Authority: [design.md](design.md), R7–R15; [consumer acceptance](effect-info-delivery.md).
The [read contract](read-contract.md) and [assessment contract](assessment-contract.md)
are concrete proposals, not approved public APIs merely because they are named.

Non-goals: outcome prediction, a calculation dashboard, a full catalogue browser,
new spells, a new command confirmation, or correcting Sneak Attack in information
code. The owning-rule defect remains toolkit#1929.

Owning instructions: `rpg-toolkit/CLAUDE.md`, `rulebooks/dnd5e/CLAUDE.md`, the
character package's `CLAUDE.md`, and the consumer repositories' own instructions.
One nearest-Go-module per toolkit PR; publish provider commits for development,
then adopt CI release tags before consumer merges. No automatic merge.

Inspected baselines: toolkit `3955eaf7`; C1 worktree starts at `a088baa9`
(`origin/main`, identical root-dnd5e source to the inspected baseline); web
`af4ee892`; API `858c3e5a`; protos `e80efe0`; project design `e0cf1b9` plus the
current read-contract draft. Exact
source inventory and earlier baseline commands are linked from the consumer trace
and [coverage inventory](contribution-coverage.md).

## Task C1: Canonical tooltip content for the three motivating effects

**Delivers:** R7/R14 and the canonical-content portion of the effect-tooltip
acceptance: Sneak Attack, Raging and Blessed have actual descriptions under their
existing content owner. No eligibility calculation, contribution engine, or UI
integration is delivered by this checkpoint.

**Owner:** `rpg-toolkit`, root `rulebooks/dnd5e` module; conditions owns content and
character owns its existing status projection.

**Prerequisites:** existing `conditions.DisplayFor`, `character.StatusView` and
valid loaded condition fixtures. No new proto, consumer release, context provider,
assessment API or running stack is needed to execute this task.

**Files:**

- Modify `rulebooks/dnd5e/conditions/display.go`: the existing entries for
  `refs.Features.SneakAttack()`, `refs.Conditions.Raging()` and
  `refs.Conditions.Blessed()` only.
- Modify `conditions/display_test.go`: update existing exact expected displays
  for Raging/Sneak Attack and add the descriptor assertions below.
- Modify `character/status_view_test.go`: extend
  `TestStatusViewRogueProjectsSneakAttackCondition` to prove canonical detail is
  copied; add a named case for the Raging/Blessed projection.
- No handler, predicate, loader dispatch, price, resource, dependency pin or
  selection-list changes.

**Interfaces:** preserve `DisplayFor(core.Ref) (Display, bool)`, `Display{Name,
Detail}` and `Character.StatusView(*StatusViewInput) (*StatusViewOutput, error)`.
`ConditionView.Detail` must equal the returned canonical descriptor's `Detail`;
`SourceMember` behavior and the condition ref remain unchanged. Do not add a
parallel `tooltipDescriptions` registry or a new transport field for data already
represented by `Detail`.

**Behavior:** add original, truthful descriptions of the implemented benefits.
Sneak Attack explains extra damage, once-per-turn use and the advantage/nearby-
enemy alternatives without claiming its known weapon-eligibility defect is fixed.
Raging explains the eligible melee-Strength damage benefit, advantage on
Strength-based skill checks and Strength saves, and physical resistance; do not bake its current instance's level-based
bonus into universal copy. Blessed explains its d4 on attacks/saves and nonstacking
of duplicate Bless instances, not a damage bonus. Instance-specific amounts and
contextual eligibility reasons remain the future shared assessor's responsibility.
Unknown refs still return false; Shield's existing status-catalog exclusion stays
unchanged. Reading content mutates no condition or character.

**Tests:**

- New `TestEffectTooltipContentSuite`: all three canonical refs are found, names
  are unchanged and details are nonempty. Exact expected copy is pinned after
  checking it against the owning implementations; no test introduces a new
  gameplay-eligibility assertion.
- Existing `TestDisplayForKnownConditions` updates the intentional new detail
  rather than deleting its equality assertions.
- Existing rogue status-view test plus new
  `TestStatusViewProjectsCanonicalEffectDetails`: detail matches `DisplayFor`,
  refs/source identities remain correct and serialized character data before and
  after the read is equal, normalizing only `ToData`'s generated `UpdatedAt` stamp.
  That field is serialization time, not mutable character state. Use existing
  fixture construction patterns.
- Existing unknown-ref and Shield exclusion regressions remain green.

- [x] Add nonempty-detail assertions; demonstrate they fail against name-only entries.
- [x] Author checked canonical copy in the existing three entries.
- [x] Update exact display expectations and status projection tests.
- [x] Run focused and owning-module gates; record actual outputs on the PR body.
- [x] Publish a root-module checkpoint PR linked to #520; do not claim live tooltips.

**Verification:** from the task worktree's `rulebooks/dnd5e` directory:

```sh
GOWORK=off go test -mod=readonly ./conditions ./character \
  -run 'TestEffectTooltipContentSuite|TestDisplayFor|TestDisplayCatalogExcludesShieldSpell|TestStatusViewRogueProjectsSneakAttackCondition|TestStatusViewProjectsCanonicalEffectDetails' \
  -count=1
GOWORK=off go test -mod=readonly -race ./...
golangci-lint run ./...
```

Before commit, run `git diff --check` and the repository's required hooks/gates
without bypassing them. Full module/lint checks apply at the provider readiness
boundary, not after each prose edit.

**Observed checkpoint evidence:** toolkit#1932 at `9b127988`; descriptor/status
regressions red then green; full root-dnd5e race suite passed; root lint reported
zero issues; normal commit hook passed with cache disabled and no bypass. Tooling:
Go 1.24.6 and repo-pinned golangci-lint v2.3.1. No dependency changes.

Repository-wide `make pre-commit` failed at the documented core coverage parsing
problem (#769): package output enters `bc`, and the target reports 78.4%. This is
not a successful repository-wide gate. Log: `/tmp/effect-info-content-repo-gate.log`;
root test/lint logs: `/tmp/effect-info-content-race.log` and
`/tmp/effect-info-content-lint.log`. The read-state assertion initially included
`ToData`'s generated timestamp; correcting that measurement left the intended
missing-description regressions red before the content fix.

Independent review at `9b127988` reported no Critical/Important findings and one
Minor: the raw-Strength-check path does not receive Rage advantage. A real-chain
probe confirmed one d20 for an unset skill versus two for Athletics. Copy and its
expected string were narrowed at `f94ed5a7`; the underlying governing-ability gap
is tracked separately in toolkit#1934. Focused descriptor/status/Rage race tests
and the normal uncached commit hook passed at that head. The full root gate was
not repeated for the two-line copy correction. The finding has an Addressed
thread disposition; reviewer closure at the new head remains pending.

The PR remains draft; no merge, released provider adoption, API or real browser
tooltip acceptance is claimed.

## Task C2: Detached observed context from encounter

**Delivers:** R7/R13's spatial/relationship input to shared assessments: the
observer's own position, current sighted member snapshots and pairwise distance/
relationship facts. It never decides whether a game effect applies.

**Owner:** `rpg-toolkit`, independent `rulebooks/dnd5e/encounter` module.
Worktree `.worktrees/effect-observed-context`, branch `feat/effect-observed-context`,
base `a088baa9` (same encounter source as inspected `3955eaf7`). No dependency on
C1's root-module changes; no root rulebook import or dependency bump.

**Prerequisites:** existing `Encounter.View`, `DecodeSightTestimony`, `placementOf`,
`Distance` and the current `BelievedStance` policy. The latter already specifies
that, absent deception, a creature shows the derived stance. C2 shares that owner
with an explicit observer for a pair of other sighted members; it does not infer
relationships from two ring colors or query a target's private viewpoint.

**Files:** create `encounter/observed_context.go` and
`encounter/observed_context_test.go`; modify `encounter/world.go` only to factor
`BelievedStance` through its shared observer-aware pair helper; update
`encounter/doc.go` with the read's bounded contract. All paths are inside the
encounter module. Existing `believedstance_test.go` remains regression coverage.

**Interfaces:**

```go
// Proposed here; implemented by C2 before a consumer may import it.
func (e *Encounter) ObservedContext(in *ViewInput) (*ObservedContextOutput, error)

type ObservedContextOutput struct {
    Observer MemberID
    Position spatial.Position // Observer's own canonical placement.
    Members []ObservedContextMember // Other members in CURRENT sight only.
    Pairs []ObservedContextPair // Ordered distinct pairs over observer + Members.
}
type ObservedContextMember struct {
    ID MemberID
    Position spatial.Position
    Down *bool // Nil = unobserved, not standing or incapacitation inferred.
    Equipment *HeldEquipment // Nil = unobserved, not empty hands.
}
type ObservedContextPair struct {
    From MemberID
    To MemberID
    DistanceCells float64
    Stance *Stance // Nil = no known relation; present neutral is a real fact.
}
```

**Behavior:**

- Validate through the existing member read: nil input is `ErrNilInput`; an absent
  or nonmember observer is `ErrNotMember`. Return no partial output on error.
- Own placement comes through `placementOf`. Other positions, standing and hands
  come only from that observer's current sight testimony. Exclude memories,
  unknown remembered locations and non-sight channels; never refill missing
  observation fields from the roster or capability providers. Invalid current sight testimony
  is an `ErrInvalidData` failure, not a guessed position.
- Sort member IDs; pairs are sorted by From then To, omit self pairs, and range
  only over the observer and those current sightings. Distances use `Distance`
  over the projected positions, not live target positions. Each ordered pair
  carries the relationship answered by encounter's shared believed-stance owner
  for the original observer; no faction means nil, not neutral.
- Preserve the existing stance policy and the old `BelievedStance(viewer,subject)`
  answers. Factor a private `believedStanceBetween(observer,from,to)` helper so
  future observer-specific stance belongs at one owner, not inside information
  projection. The known participant set is what authorizes pair enumeration.
- Return detached values. Read no live Sight/Equipment/Participation capability,
  publish nothing, resurvey nothing and change no persisted encounter data.
- The result enumerates **observations, not a complete world neighborhood**. It
  cannot prove absence of unseen participants, reveal target effects/senses, or
  turn observed standing into general reaction eligibility. Those fields remain
  unavailable to the later assessment unless another permitted provider supplies
  them. C2 is not a complete target-effect observation system.

**Tests:** new `TestObservedContextSuite` in the external encounter test package:

- `TestCurrentMembersAndPairs`: observer at `cellAt(0,0)`, ally at `cellAt(1,0)`,
  hostile target at `cellAt(2,0)`, world NPC at `cellAt(4,0)`. Assert sorted member
  IDs, exact pair count/order, ally-target distance 1, observer-target distance 2,
  known hostile target/ally relation and nil NPC relation (not neutral).
- `TestSnapshotFactsWinOverLiveState`: snapshot hands/standing, change their live
  providers to different answers or errors, read twice; output stays identical and
  provider call counts do not increase. A loaded known sight position differing
  from the true target cell supplies the projected position and pair distances.
- `TestMemoryAndOtherChannelsAreExcluded`: reload a holding with CurrentVia empty,
  an unknown remembered location, or another channel; it appears in neither
  Members nor Pairs. Hidden subject changes do not alter the observed output.
- `TestUnknownObservedFieldsStayUnknown`: a current known location with nil Down
  and Equipment remains nil in the result; no fallback consult occurs.
- `TestDetachedAndReadOnly`: mutate returned positions/pointers/rows and verify a
  fresh read is unchanged; compare full serialized encounter data before/after.
- `TestInputErrors`: nil, empty observer and stranger return the existing errors
  with nil output. Existing strict testimony-load tests still reject malformed
  current records; no reader-side repair is introduced.
- Existing `TestBelievedStanceSuite` stays green, pinning unchanged relation rules.

- [x] Add the named tests and demonstrate the missing API fails to compile.
- [x] Implement the bounded projection and shared stance helper.
- [x] Run focused and full module/race/lint gates; check no other module changes.
- [x] Publish a draft encounter-provider PR linked from the #520 body.

**Verification:** from the worktree's `rulebooks/dnd5e/encounter`:

```sh
GOWORK=off go test -mod=readonly . -run 'TestObservedContextSuite|TestBelievedStanceSuite|TestPassageSuite' -count=1
GOWORK=off go test -mod=readonly -race ./...
golangci-lint run ./...
```

Also run the normal commit hook without bypasses and report the repository-wide
gate separately. No API, browser or released-consumer adoption is proven by these
provider tests. Do not merge before the wave's applicable review/integration gates.

**Observed checkpoint evidence:** toolkit#1933 at `01e429e6`. The new API's absence
was demonstrated red; the observed-context, existing stance and passage tests
passed. Full encounter-module race tests passed, full lint reported zero issues,
and the normal commit hook passed with cache disabled. No dependency changes.
The suite also pins known empty hands, an empty observed universe and another
observer's conflicting testimony. Logs: `/tmp/effect-observed-context-focused.log`,
`/tmp/effect-observed-context-race.log`, `/tmp/effect-observed-context-lint.log`.

The repository-wide gate first hit a disk/cache failure. Available space was
rechecked; no cleanup was needed. Retrying with task-local caches reached the same
known core coverage/parser failure (#769), not a successful gate. Retry log:
`/tmp/effect-observed-context-repo-gate-retry.log`. Tooling remains Go 1.24.6 and
repo-pinned golangci-lint v2.3.1; no hook was bypassed.

The operator merged C2 on 2026-10-03 as `f06cacf2`. CI's auto-tag run
[37160884903](https://github.com/KirkDiggler/rpg-toolkit/actions/runs/37160884903)
succeeded; the annotated `rulebooks/dnd5e/encounter/v0.111.0` tag dereferences to
that merge commit. No independent review record is claimed here. Consumer pin
adoption, integration and browser evidence remain pending; tagging is not proof
of those boundaries. Later consumers must mirror these fields without an inner
encounter type crossing session's host surface.

## Remaining wave — not executable handoffs yet

These are required workstreams, **not approved deferrals or complete task briefs**.
The source files and interfaces have been measured, but an implementer must not be
told to invent the open contracts while coding.

| Owner / source | Required deliverable | Readiness dependency |
|---|---|---|
| Root dnd5e: `events/roll_trace.go`, `conditions`, `features`, `monstertraits`, weapon/cast assembly | Shared values, detached assessments, per-facet coverage, normalization and lifecycle-preserving adapters for the inventory | Close concrete frame/change/binding interfaces and full coverage tasks; C1 only supplies three descriptions |
| Encounter: C2 above; remaining observed projections | C2 supplies current observed positions/distances and existing public relationship policy; target-carried effects/senses stay separate | C2 merged/tagged via #1933 (encounter v0.111.0); additional target-effect/sense knowledge still needs its own permitted source, never a hidden-truth fallback |
| Resolution: `strike.go`, `strike_pose.go`, `visibility.go`, new information read | Assemble/fold rule-owned assessments and return effect information without rolling/mutating; migrate execution consumers to the same decisions | Root and encounter contracts above; preserve per-boundary frozen state |
| Session: `offers.go`, `casts.go`, `read.go`, `types.go`, new inspection verb | Enumerate actual action variants independent of affordability; carry permitted context; project detached information refs/effects | Root variant enumerator, resolution read, final `InspectActions` semantics |
| Protos/API: session service/types and session handler/converters | Inspection request/response, informational action ref on declarations, authorization and one mapping point | Concrete SDK output and absence/error states; publish generated bindings before consumer integration |
| Web: `SessionEncounterView`, `SessionCanvas`, action dock/target surface, new read hook | Separate inspection state and effect tooltips, newest-generation responses, explicit touch inspection, unchanged click commands | Published wire/provider contract; R15 interaction is already settled |
| Joined stack | Normal acquisition, effect changes, target switching, reload and no-side-effect/native-tooltip proof | Implemented providers/consumers, module pins, fixtures and final exact verification commands |

No persistence/event expansion is assumed yet. If a newly required observation
must survive loss of sight/reload, that lifecycle belongs in the existing
observer testimony owner, with an explicit task and tests—not a hidden cache in
the information read. Existing multiplayer events can invalidate current reads;
new event types need an actual uncovered producer/consumer requirement.

## Visible checks

### Requirement coverage

| In-scope requirement / acceptance | Task or status | Concrete proof |
|---|---|---|
| Canonical descriptions for Sneak Attack/Raging/Blessed | C1 implemented in draft #1932 | Nonempty descriptor regression, unchanged names/refs, status detail equals canonical content |
| Rule-owned applicability/reason and shared execution | Required, blocked on full root assessment handoff | Inventory's paired consumer/execution and no-duplicate-predicate assertions; not covered by C1 |
| Explicit normalization and frozen/consumption custody | Required, blocked on root/resolution handoffs | Paired-rule and RNG-count/freeze tests in inventory; not covered by C1 |
| Permitted selected-target context | C2 merged/tagged via #1933 for positions/observed standing/equipment/distances/relations; target-effect/sense projections remain open | Snapshot, current-only, no-live-provider, unknown and pair-scope tests; not a claim of complete target knowledge |
| Independent read, freshness and command legality | Proposed read contract; transport tasks not ready | Off-turn/spent/frozen reads, exact action identity and stale response cases |
| Active/gray/conditional/opportunity UI and accessible tooltips | R15 settled; implementation handoff pending transport | Pointer/keyboard/touch acceptance and zero command calls on inspection |
| End-to-end normal character acquisition/reload/effect changes | Required; integration task not ready | Real API/native browser proof, not seeded-provider tests alone |

### Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|
| C1 / conditions | Existing character status projection | `Display.Detail` → `ConditionView.Detail`, same canonical ref | Existing compiled interface; no new dependency | New status-detail equality and no-mutation test |
| Root variant/assessment work | Resolution and session | Typed action identity, frames, decisions and coverage | Proposed, not implemented/handoff-ready | Must compare exact signatures and execute joined provider/consumer tests before coding downstream |
| C2 / encounter | Resolution information read | `ObservedContextOutput` members/pairs; optional observation facts remain optional | Published as encounter v0.111.0 via #1933; consumer handoff/adoption still pending | Snapshot/current-only/detachment tests at provider; joined information noninterference still required |
| Session → protos/API → web | Effect indicators and tooltips | `InspectActions` / `ActionInformation` / `EffectInformation` semantics | Proposed; generated SDKs and published provider pins needed for integration | Real mapping tests and browser effect/tooltip proof remain to be specified |

**Check findings:** the previous source inventory was not an implementation plan;
this document does not relabel it as one. C1 consumes only inspected existing APIs;
C2 produces the bounded observed-context API from inspected owners without changing
stance/visibility policy. The remaining rows still have unproduced required inputs
and are not ready. No
user-visible scope has been deferred to make the readiness table look complete.
