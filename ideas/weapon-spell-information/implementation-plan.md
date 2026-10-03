# Effect information — implementation handoffs

**Readiness: partial.** C1 is implemented in draft
[toolkit#1932](https://github.com/KirkDiggler/rpg-toolkit/pull/1932) at `9b127988`;
its owning-module checks pass, but repository-wide gate/review/release readiness
is not claimed. The shared assessment and target-aware read wave is **not** ready
to execute from this document: required context producer contracts below are not
closed. C1 is not a substitute for that component and does not complete #520.

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
Raging explains the eligible melee-Strength damage benefit, STR check/save
advantage and physical resistance; do not bake its current instance's level-based
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

The PR remains draft; no independent review, merge, released provider adoption,
API or real browser tooltip acceptance is claimed.

## Remaining wave — not executable handoffs yet

These are required workstreams, **not approved deferrals or complete task briefs**.
The source files and interfaces have been measured, but an implementer must not be
told to invent the open contracts while coding.

| Owner / source | Required deliverable | Readiness dependency |
|---|---|---|
| Root dnd5e: `events/roll_trace.go`, `conditions`, `features`, `monstertraits`, weapon/cast assembly | Shared values, detached assessments, per-facet coverage, normalization and lifecycle-preserving adapters for the inventory | Close concrete frame/change/binding interfaces and full coverage tasks; C1 only supplies three descriptions |
| Encounter: `testimony.go`, `world.go`, `encounter.go:View`, observed projections | Bounded observer context, measured distances, permitted three-party relationships, explicit missing facts | Exact observation source/fields for relationships and any target-carried effects/senses; no hidden-truth fallback |
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
| Permitted selected-target context | Required, blocked on exact observation contract | Hidden-state noninterference and unknown-versus-negative proofs; not covered by C1 |
| Independent read, freshness and command legality | Proposed read contract; transport tasks not ready | Off-turn/spent/frozen reads, exact action identity and stale response cases |
| Active/gray/conditional/opportunity UI and accessible tooltips | R15 settled; implementation handoff pending transport | Pointer/keyboard/touch acceptance and zero command calls on inspection |
| End-to-end normal character acquisition/reload/effect changes | Required; integration task not ready | Real API/native browser proof, not seeded-provider tests alone |

### Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|
| C1 / conditions | Existing character status projection | `Display.Detail` → `ConditionView.Detail`, same canonical ref | Existing compiled interface; no new dependency | New status-detail equality and no-mutation test |
| Root variant/assessment work | Resolution and session | Typed action identity, frames, decisions and coverage | Proposed, not implemented/handoff-ready | Must compare exact signatures and execute joined provider/consumer tests before coding downstream |
| Encounter observed context | Resolution information read | Bounded permitted facts, not full participant sheets | Source gaps named in read contract | Hidden-world variations with identical permitted inputs must give identical outputs |
| Session → protos/API → web | Effect indicators and tooltips | `InspectActions` / `ActionInformation` / `EffectInformation` semantics | Proposed; generated SDKs and published provider pins needed for integration | Real mapping tests and browser effect/tooltip proof remain to be specified |

**Check findings:** the previous source inventory was not an implementation plan;
this document does not relabel it as one. C1 consumes only inspected existing APIs.
The remaining rows still have unproduced required inputs and are not ready. No
user-visible scope has been deferred to make the readiness table look complete.
