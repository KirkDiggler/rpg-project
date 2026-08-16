# API session integration — plan

Waves for rpg-project#227. Each wave: one issue per repo on board 19, one
branch, one PR, the Copilot loop, and live verification per the
test-before-merge discipline. This triplet's PR stays open as the tracking
surface until cutover completes.

## W0 — triplet (this PR)

Design ratified on the PR; `sessions/active.md` brought current.

## W1 — protos (rpg-api-protos, base `origin/main`)

`dnd5e/api/session/v1alpha1/`: `SessionService` per design §1–2, the `Event`
message per MUST-3, request/response messages mirroring the SDK's exported
inputs/outputs. Additive — `buf breaking` stays green. Done when: reviewed
against the design's rules, merged, released; the release note names the SDK
stack version the shapes were read from (session/v0.8.0).

## W2 — rpg-api (base `origin/dev`, one branch)

- `internal/orchestrators/session/`: `session.Manager` construction (all
  capabilities supplied), Redis `SessionRepository`/`EncounterRepository`,
  `CharacterRepository` adapter, `EventStream` → broker.
- `internal/handlers/dnd5e/session/v1alpha1/`: translation handlers, the
  error table (design rule 7), `StreamEvents`.
- Lobby re-point behind server config (design §3 coexistence): authored
  dungeon content → `EncounterData` → `StartSession`/`Join`/`Spawn`.
- Delete the unwired legacy components (`internal/components/dungeon/`,
  `spawner/`) — unreachable from the server binary, free.
- Integration test: the acceptance loop (design §6.1) headless.

Done when: acceptance loop passes in CI and is demonstrated live locally.

## W3 — web (rpg-dnd5e-web, base `origin/dev`, one branch)

Starts when W1 merges (Kirk). New game route on `SessionService` +
`StreamEvents` with story resync; old route untouched. UI/UX lane owns
shape and slicing. Done when: the acceptance loop is playable in the browser
against the local stack.

## W4 — cutover (the rip-out)

Coordinated swap per design §5: web deletes the old route; rpg-api deletes
the old handler/orchestrator/repo/tests and flips the creation default;
rpg-api-protos removes `dnd5e.api.v1alpha2.encounter` in place. Evidence:
live walkthrough of §6.1 on the new-only build; §6.2 and §6.3 checked on the
PRs. This triplet's PR merges here as ratification.

## W5+ — the capability train (post-cutover, out of this plan's scope)

Each lands toolkit-first, then flows through the thin pipe as its own small
visible wave: monster attacks back (behavior work), fights end by defeat
(toolkit#1024), action economy (toolkit#1035), asymmetric perception /
stealth and surprise (toolkit#1020). Filed here so nobody re-derives the
order; sequenced when they arrive.
