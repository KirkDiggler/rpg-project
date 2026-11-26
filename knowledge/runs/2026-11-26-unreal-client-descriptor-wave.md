# Run — 2026-11-26 — The descriptor fields, renamed in one wave

Ruling R2 ([rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155#issuecomment-7185702703)), in the operator's words: before playtest the pieces go back together in one pass.

## Shipped

- The contract: both fields `npc_descriptor`, tag 1 untouched, the old name reserved — [rpg-api-protos#457](https://github.com/KirkDiggler/rpg-api-protos/pull/457).
- The api: two handlers and five test reads, pinned to the published commit — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292).
- The web: six reads and fifteen fixtures on the new key, pinned to `v0.1.292` — [rpg-dnd5e-web#1493](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1493).
- The client: its fetch script generates the contract as the protos carry it, and refuses a checkout from before — [rpg-dnd5e-unreal#1](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/1).

## Decided

- In place under the breaking-change label, by the protos repository's workflow for an alpha package — [the workflow](https://github.com/KirkDiggler/rpg-api-protos/blob/main/docs/how-to/breaking-change-workflow.md).
- Each leg was proven before its pin existed, against a local copy of the renamed contract, and pushed only when the pin did — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292).
- The api branch caught up with `dev` before merging, so CI built the combination that merged — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292).
- The client's script states its cause where the cause is, not in a generated header — [rpg-dnd5e-unreal#1](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/1).

## Found

- C++ from `protocolbuffers/cpp` v23.4: `origin/main` failed at `service.pb.h:14733` and `:16400`; with the rename it compiles — [rpg-api-protos#457](https://github.com/KirkDiggler/rpg-api-protos/pull/457).
- protoc-gen-go's mangled accessor and the web's loose fixtures hid the field from every check but the runtime: five test reads in the api, thirteen vendor tests in the web — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292), [rpg-dnd5e-web#1493](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1493).
- The web consumes the tag's tarball from GitHub, not the public npm registry — [proto-updates](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/how-to/proto-updates.md).
- An unprotected `BEHIND` branch merges untested; one commit had landed on `dev` under the api leg — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292).

## Mistaken

- A publish was called done on the previous merge's run; it was re-read by the run's own commit, and the tag was `v0.1.292` — [rpg-api-protos#457](https://github.com/KirkDiggler/rpg-api-protos/pull/457).
- "No test reads the field" was said before the tests were compiled — [rpg-api#1292](https://github.com/KirkDiggler/rpg-api/pull/1292).
- Numbers were written for pull requests before they existed, three times in one session; each was corrected, and the rule now is to write none — [game-dev#140](https://github.com/KirkDiggler/game-dev/pull/140).
- A gate's summary was read through a filter that hid the failing stage — [rpg-dnd5e-web#1493](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1493).
