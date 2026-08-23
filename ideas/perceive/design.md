# Slice 4 — you hear what you can perceive

Journey #253 · follows #257 (the stream is whole) · closes rpg-toolkit#940.

## 1. Why

Every story beat in `rulebooks/dnd5e/encounter` is addressed to the whole roster. `record.SliceFor` honours `Audience`, `play/intel` models who currently sees whom, and `play/clock` knows who is in which fight — but no append site asks any of them. Kirk filed #940 (2026-08-13) when the session SDK turned the story from a query into a push channel: an over-broad audience is now a fog-of-war leak no client can undo. Slice 3's two-player proof (rpg-toolkit#1206) pinned it live: a walled-off member — and therefore a second human in another room — receives every `moved`/`struck`/`turn_ended` of a fight they are not in and cannot see.

Pure composition slice: toolkit only. rpg-api and web change nothing; the web's feed (#740) simply shows fewer lines to the player who shouldn't have them.

## 2. The rule, stated once

**A beat is addressed to the members who would know it happened.** Three audiences, by what the beat is about:

| Audience | Beats | Who |
|---|---|---|
| **Subject** | `moved`, `struck`, `missed`, `downed`/standing, `joined` | the subjects themselves (actor, targets, the mover, the joiner — you always know what you did and what was done to you) **∪** every member holding a *current* sight channel on a subject, measured **both before and after** the action for movement (watching someone walk out of view includes the step that takes them out) |
| **Bubble** | `fight_started`/formed, `turn_ended`, `transferred`, `dissolved`/`fight_ended` | everyone in that fight (`bubble.Order()`), sight or not — whose turn it is is fight knowledge, not line-of-sight knowledge |
| **Table** | `scene-opened`, `tick`, `exited`, `ended` | everyone (`allMemberIDs` / roster), as today |

Door beats already compute an audience through `refreshSight` (doorverbs.go) — the one site that got it right; it stays.

What a member who *can't* see gets: **nothing** for the subject beat. They still receive the session's sighting deltas (`seen`/`lost`) from the sight refresh that follows — "you lost sight of the skeleton" is the fog-of-war cue, and it already exists. No "something moved nearby" summaries in v1.

## 3. Where it changes (encounter)

| Site | Today | Becomes |
|---|---|---|
| `outcome.go` `Record` (struck/missed, and the downed beat that follows in `standing.go`) | `rosterIDs()` | subject audience over {actor} ∪ targets |
| `step.go` `Step` → `appendMovementBeat` | roster | subject audience over {mover}, sight measured before the step ∪ after |
| `clocks.go` `executeTurnIntent` Move case → `appendMovementBeat` | roster | same helper as Step (the driven monster's walk is a walk) |
| `encounter.go` tick-driven move (`:1256`) | roster | same helper |
| `encounter.go` `joined` (`:1622`) | roster | subject audience over {joiner} after the joiner's own sight refresh |
| `clocks.go` `appendClockBeat` (`:235`) | roster | `bubble.Order()` of the bubble the beat is about; `formed` uses the order it just rolled |
| `encounter.go` scene-opened/tick/exited/ended | roster/all | unchanged |

One helper does the subject rule: `audienceFor(subjects ...MemberID) []MemberID` — subjects, plus each member `m` of `everMembers` for which `intel.On{Observer: m, Subject: s}` returns `Status == Current` on the sight channel for any subject. O(members × subjects) per beat; roster is single digits. No reverse index in `play/intel` until a roster is large enough to need one (then it's a `HeldBy`-shaped `HoldersOf`, not a change here).

Ordering matters for movement: sight is refreshed per step today; the helper takes the union of holders from the refresh *before* the step and the refresh *after*. Joined: refresh first, then address.

## 4. What the session sees

Nothing changes in `rulebooks/dnd5e/session` except tests: `projectEvents` already addresses per recipient from each member's own `Story`. The #1206 negative control (`TestTwoPlayersOneSession`, walled skeleton receives beats) flips to zero; `TestOneBeatBecomesPerRecipientEvents` (`carol sealed in another room is told where alice moved — that is #940`) flips too. Scene 7 in `docs/ideas/session-sdk/scenes.md` loses its disclaimer.

Pin: encounter tag → session PR that only re-pins and flips the tests.

## 5. Rulings asked of Kirk

1. **Subject audience includes sight on the actor OR the target** (you see the archer loose, or you see the arrow land). Recommendation: yes — either suffices.
2. **Downed beat** follows the subject rule (you learn a teammate dropped only if you can see them or it was you/your attacker). Recommendation: yes in v1; a "party knows" channel (shouting) is content for later, not composition.
3. **Clock beats to the bubble only** means a player outside the fight never hears "a fight begins" until they see someone in it. Recommendation: yes — that is what walking into a room mid-fight feels like.

## 6. Gate

Toolkit tests are the gate (no walk needed, but the feed from #740 shows it): in the tomb fixture with fighter + barbarian, barbarian sealed behind the wall — barbarian's story holds no fight beats, no skeleton moves, no struck; the instant the door opens and the barbarian sees in, `seen` arrives and subsequent beats do. The skeleton behind its wall hears nothing of the fighter's fight. Mutation check on the helper: dropping the "after" refresh in movement must fail a test (the walk-out-of-view step).

## 7. Order

1. encounter: `audienceFor` helper + the seven sites, RED tests first per site (one test per row of §3), tag.
2. session: re-pin, flip the two negative controls, Scene 7 text, tag.
3. rpg-api: pin bump only.
