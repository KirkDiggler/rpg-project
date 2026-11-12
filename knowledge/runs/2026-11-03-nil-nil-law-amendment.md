# Run — 2026-11-03 — nilnil #2220 — the law gains its second half

processed: 2026-11-12T165555Z

Amends [rpg-project/AGENTS.md](https://github.com/KirkDiggler/rpg-project/blob/main/AGENTS.md) in service of [rpg-toolkit#2220](https://github.com/KirkDiggler/rpg-toolkit/issues/2220). Base: `rpg-project` `main` at 2a666a2. Ruling: Kirk, in session, [recorded on the issue](https://github.com/KirkDiggler/rpg-toolkit/issues/2220#issuecomment-6613107798).

## Shipped

- The invariant now says what the fixes build: "Exactly one of a valid object or an error — and an error may be a signal, not a failure (`io.EOF` is the model)." — [AGENTS.md](https://github.com/KirkDiggler/rpg-project/blob/main/AGENTS.md).

## Decided

- The amendment carries both halves of the ruling in one line — the exactly-one shape and the error-as-signal license — because a law that only bans `(nil, nil)` would leave "which of the two" unanswered at every fix site — [#2220](https://github.com/KirkDiggler/rpg-toolkit/issues/2220#issuecomment-6612643483).
- `io.EOF` names the tradition rather than a toolkit example, so the law does not point at code that a later wave may move — [AGENTS.md](https://github.com/KirkDiggler/rpg-project/blob/main/AGENTS.md).

## Found

- The invariant lives in the invariants list under "Zero values tell the truth" territory; the signal license does not weaken that: a sentinel says what happened, a missing value says nothing — [#2220](https://github.com/KirkDiggler/rpg-toolkit/issues/2220).

## Mistaken

- none observed.