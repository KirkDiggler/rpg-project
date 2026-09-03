# Run Readiness and Activation Story — Implementation Decisions

## Slice B: first-admission LongRest persistence ordering

**Date:** 2026-09-01  
**Issues:** rpg-project#341, rpg-toolkit#1396

### Approved design before implementation

The first-admission Join design originally treated LongRest as tentative until projection, placement, and discovery checks had all succeeded. The rested character would be saved immediately before encounter commit, so a failed placement would leave both character and encounter unchanged.

### Implementation discovery

Session placement is not a closed local operation. `encounter.Join` may refresh sight, consult `standingSeam`, form combat, and drive a monster turn. Those callbacks read character records through `CharacterRepository`. Holding the rested record only in a Join-local variable therefore creates two simultaneous truths:

- projection and the eventual Join response see the rested record;
- standing/cast/driven-resolution callbacks see the old persisted record.

A zero-HP admission can consequently be observed as down during placement, and a driven result can be overwritten by the later rest save. Session commit preparation also has fallible stream-numbering work after placement.

### Final approved behavior

LongRest persistence is an independently valid between-runs transition:

1. On first-ever Join, Session calls `resolution.LongRest`.
2. Session immediately persists the rested `character.Data` and records `character:<id>` as written.
3. Projection, placement, standing/cast callbacks, fight formation, and driven turns all read the same rested repository truth.
4. Session commits the encounter normally.
5. If any step after the character save fails, the rest remains durable and the returned `SaveError`/`SaveReport` names every earlier write.
6. Retry is safe because LongRest is idempotent and a failed placement does not persist `EverMembers`.

### Rejected alternative

A transaction-style character overlay on `writeScope` was considered. It would require every standing, cast, striker, announcer, check, concealment, and dirty-save path to read and mutate staged records before a final flush. That broader Session transaction system is disproportionate to the current rule and obscures the future removal point.

The chosen early-save block is explicit and temporary. When town play introduces an in-world LongRest action, remove the first-admission `resolution.LongRest → SaveCharacter` block from Join; no Session-wide transaction infrastructure needs to be unwound.

## Slice B: LongRest exits stale prior-session action economy

**Date:** 2026-09-02
**Issues:** rpg-project#341, rpg-toolkit#1408, related rpg-toolkit#1223

### Live discovery

The first merged live proof restored Second Wind and removed temporary conditions correctly, but the second run persisted the prior session's spent turn state:

```text
run 1 end:      turn=1 actions=0 bonus=0
run 2 LongRest: turn=1 actions=0 bonus=0
first EndTurn:  turn=2 actions=1 bonus=1
```

The UI therefore showed Second Wind `1/1` but unavailable until the player ended a turn. `Character.InCombat` is defined by non-nil `ActionEconomy`; `LongRest` restored every D&D resource but left that old combat marker intact. When the new fight also began on round 1, `RefreshForTurn` treated the stale economy as current.

### Final approved behavior

A completed LongRest exits the prior combat economy through the existing `Character.ExitCombat` owner. `ToData().ActionEconomy` is nil after LongRest; when a new fight forms, normal `StartTurn` seeds fresh action, bonus action, reaction, movement, and an empty granted-capacity map. ShortRest does not clear economy.

This is a root D&D rule, not a Session or API patch. rpg-toolkit#1223 remains separately responsible for clearing economy immediately when `Manager.End`/`Exit` ends combat without a following LongRest, which matters for future town play.

### Local acceptance before publication

The fix was synced into the exact merged API checkout with the landed `rulebooks/dnd5e` local override, built through `Dockerfile.local-toolkit`, and deployed only to isolated lab1. A third run began with persisted `ActionEconomy:nil`; the first combat formed on round 1 and Second Wind was immediately available without EndTurn. The shared primary container start time remained unchanged.

## Slice C: activation results expose rolls and use the audience shelf

**Date:** 2026-09-02
**Issue:** rpg-project#342

### Approved behavior

Every successful `Activate` records one activation beat followed by its typed results. Healing results preserve the source, roll, modifier, requested amount, actual post-clamp amount, and HP before/after. Story as well as Debug exposes available roll arithmetic; Story remains readable while Debug includes canonical refs and every raw typed field.

This applies generically to healing produced inside the scoped `Activate` interaction. Second Wind is the current activated healing provider. Hit-die spending, LongRest, natural-20 death-save recovery, and other healing outside `Activate` remain outside activation-result logging.

Condition results likewise use server-authored identities. Story may say `Aldric begins Raging`; Debug includes `dnd5e:conditions:raging` and the toolkit-authored display name. API and web never switch on Rage, Second Wind, or another feature ref.

### Audience

Repository inspection after approval exposed an existing policy boundary: rpg-project#260 deliberately keeps every combat-log beat visible to the full roster until v1.0, and rpg-toolkit#940 owns the eventual perception-scoping flip. Adding an activation-only visibility rule would bypass the single `audienceFor` shelf; flipping the shelf here would expand this slice across attacks, movement, downed, clocks, and every other classified beat.

The final decision is therefore to defer the policy flip. Activation and result facts call `audienceFor(subjectBeat, ...)` with honest actor and affected-target subjects. They follow today's full-roster policy and are automatically ready for #940 without later API, web, payload, or append-site changes. Live and catch-up read the same recorded audience, and neither API nor web recalculates or broadens visibility.

### Delivered activation-result stack

Slice C shipped through the following immutable coordinates:

- root healing facts: `rulebooks/dnd5e/v0.128.0`;
- activation transaction: `rulebooks/dnd5e/encounter/v0.46.0`;
- activation effect capture: `rulebooks/dnd5e/resolution/v0.30.0`;
- durable Session projection: `rulebooks/dnd5e/session/v0.48.0`;
- generated activation proto: `f87c098979ce64f6c162a769e988c934a1fd80cc` / `v0.1.153`;
- API merge: `250057d927fd43c279de86a7e2a6a9f2fa0a16d6`;
- web merge: `2bee01584d8168173f047ddeb69e7ea9aa63c1e2`.

Real handler/Manager/Broker acceptance proved Activated followed by HealingApplied, requested-versus-applied clamping, persisted HP, full-roster receipt, and `proto.Equal` live/GetStory delivery. Kirk then confirmed that Second Wind appeared in the live log.

## Follow-up: reusable server-authored roll traces

**Date:** 2026-09-04<br>
**Issue:** rpg-project#361<br>
**Design:** rpg-project#363

The first live log exposed an ambiguity: `Second Wind rolled 6 + 1 = 7` did not say that the feature rolls its own `1d10 + Fighter level`, and the newer Session seam discarded Great Weapon Fighting's original faces and ordered reroll history even though the root damage chain still carried them.

The approved correction kept one result per domain—damage and healing remain distinct—while sharing provider-authored dice provenance and sourced modifier primitives. Toolkit authors notation, original/final faces, ordered indexed rerolls, source refs/names/labels, subtotals, and totals. Encounter validates before append; Session persists and strictly decodes new and legacy records; API maps fields only; web formats them without arithmetic or ref switches.

### Delivered roll-trace stack

The final consumer stack uses:

- root D&D `v0.137.0`;
- encounter `v0.53.0`;
- resolution `v0.32.1`;
- Session `v0.53.1`;
- generated proto `883dd221a6cdf724df8d5d993d897e0c8a3358ab`;
- API merge `2716d3e90315ad53e20beab601c63d6e9ec98619`;
- web merge `1188268486843085ed2fcc226f9a5f56be51fd38`.

Automated acceptance proved deterministic Second Wind (`1d10 [6] + 1 Fighter level = 7`, requested 7, applied 2, HP 8→10) and a persisted GWF Fighter (`[1,5]`, sourced `1→4`, final `[4,5]`, subtotal 9, +3 Strength, 12 damage) through real handler, Session Manager, Broker, Redis, live delivery, and GetStory. Web tests proved shared Story/Debug formatting, legacy fallback, all-or-nothing malformed handling, exact nested conflict identity, and no damage/healing 3D dice behavior. Final counts were 4,923 passing and 5 skipped; post-merge API test/Docker workflows and web CI passed.

After being told that only the isolated visual/reconnect pass remained, Kirk requested cleanup. No plan-specific lab, assets, or service was started for that final pass. The live/visual step is therefore explicitly waived rather than claimed as evidence; a real-stack visual defect remains the cost of that waiver.

### Process correction

The implementation initially created PRs at task-sized boundaries inside the same Go module. Kirk corrected the working agreement: publish one PR per module, not one PR per subtask. Internal TDD/review checkpoints may remain, but PR count is not a deliverable. The final API and web work followed that module-sized shape.

Separate follow-ups remain independent: rpg-toolkit#1460 (legacy fixture identities), #1466 (feature-resource failure semantics), #1467 (negative healing), #1490 (int32 wire bounds), and rpg-api#906 (destructive dirty-tree CI helper).
