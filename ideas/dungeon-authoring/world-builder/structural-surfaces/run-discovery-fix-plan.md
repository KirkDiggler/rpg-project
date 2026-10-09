# Encounter-owned discovery correction

Operator ruling: every playthrough creates a fresh dungeon and grants the normal
long rest; discoveries belong to that encounter. Reconnect/reload preserves the
same encounter. No playtest-only exception or manual data clearing.

## Existing ownership

EncounterData.Discovery already persists attempt/re-arm state; EncounterData.World
persists the per-observer learned facts. The defect is the SDK additionally saving
those facts in character ExplorationData.Checks and importing them at load/join.
There is no need for another store or for a new encounter snapshot format.

## Bounded changes

1. **Session #1947:** exploration.go / write.go stop exporting/importing discovery
   checks. ExplorationData retains Character and PrivateDiscoveries (preference)
   only. Existing profile JSON with a checks member is accepted but its obsolete
   check data has no authority. Preserve session guard, long rest, and encounter
   commit behavior. Update doc.go and the S2 persistence allow-list.
2. **Tests:** rewrite the cross-run-retention test to the corrected ownership:
   real Join/Move, successful and failed checks, fresh Manager over the same
   repositories, same-run reload/rejoin preserves state, second encounter of the
   SAME template/SAME character has neither attempts nor learned facts. It earns
   another check when approached. Preference survives. Decode old profile JSON
   with learned/used entries and prove it cannot initialize the new encounter.
3. **API #1077:** adopt the pushed SDK normally; repository tests preserve only
   preferences, accept old stored JSON without importing its checks, and show
   profiles remain detached. No deletion of current Redis data or active runs.
   Add a real fresh-run acceptance regression using normal launch/SDK paths.
4. **Authoring follow-through:** the existing attempt-lifetime selector advertises
   across-run retention. Audit/update that UI/contract so it does not promise
   behavior the game no longer permits. Do not silently use that selector to
   override the fresh-encounter rule. Keep unrelated wall gestures and prop
   appearance delivery tracked separately.

## Checks and release sequence

Session module Go1.24.1: targeted red→green lifecycle tests, full tests/race/vet/lint
and normal hook. Push checkpoint before API go get; no replace/go.work/source copy.
API: real repository/lifecycle regression and full make ci-check with compatible
linter. Verify old run reload and fresh playthrough in the named local stack.
Independent focused review is required for the lifecycle delta; no automatic merge.

| Requirement | Owner | Proof |
|---|---|---|
| Discovery has one persistence owner | encounter + session transport | persisted encounter state, no profile check field/export |
| New playthrough starts fresh | session launch/admission | same template/character, second encounter hidden + unused check |
| Resume is not reset | encounter load + SDK | reload/rejoin same encounter retains attempts/knowledge |
| Character preference survives | SDK profile / API repo | sharing preference retained without discovery facts |
| Existing saves remain usable | SDK JSON / encounter load | legacy profile checks ignored; active encounter snapshot unchanged |

Plan inspected against current session cc2bcf7c and API8b41b09a. The lifecycle tests
above are the discriminator; existing green tests encoded cross-run retention and
must be corrected, not cited as evidence of the operator's fresh-run contract.
