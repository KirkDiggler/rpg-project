# Game Context — plan (the HOW)

**Status:** Auto-approved per Kirk's plan ruling (2026-08-28): a plan is an
agent-handoff artifact; the **design** is what Kirk rules, and this plan is
approved exactly as far as it honestly represents [design.md](./design.md).
If implementation reality diverges from the design, **stop and surface on the
design PR (rpg-project#318)** — never patch the divergence locally. Each
phase appends findings to `implementation.md` (the mini retro).

## Ground rules for implementing agents

- One in-flight PR per module; every phase lands with its tests green.
- Phase 0 may start immediately; Phases 1+ start after toolkit#1284 merges
  (D2).
- Parity is pinned by value-formula tests — assert the number, never only a
  round-trip (symmetric bugs hide from round-trips).
- Docs follow truth the same day (record hygiene).
- A new gamectx tenant found "necessary" mid-build is a design moment (M5):
  stop and surface.

## Phases

**Phase 0 — spike (evidence, no merge).** Enumerate every path that folds a
chain or attaches an effect outside `Resolve` (EffectiveAC callers,
standing/preflight attaches, each session verb); confirm the verb-level
install is the ~one-line-per-verb it looks like; determine where the door
function lives so session can call it without an import cycle; report to the
design PR **before Phase 2 begins**.

**Phase 1 — the door (R5). DONE — rpg-toolkit#1286.** Extract `installTruth` in `resolution` from the
three inline `With*` calls (`resolve.go:347/374/395`). No behavior change;
`TestNoCodePathProduces*lessInteraction` stays green; add a structural pin
that the door is the only caller of any `gamectx.With*`.

**Phase 2 — the projection entry (R6/D6).** Resolution exports a small
projection entry: attach the one character, install the truth, fold the AC,
tear down. `session.Join`'s `projectCharacter` reroutes through it instead of
calling `Character.EffectiveAC` itself. Session gains no gamectx calls and no
imports of resolution internals. Pin: a character with Unarmored Defense
joining a session reports correct AC — the base-AC-barbarian test.
`compileResolutionCast` is parked to Phase 3 (D6).

**Phase 3 — reader migration (R1).** Unarmored Defense, Martial Arts,
Unarmored Movement read via `CastOf(ctx).Member(ownID)`; delete the
per-condition structural owner interfaces; parity tests assert identical
fold results before/after.

**Phase 4 — read-only member surface (D5).** The cast returns a read-facing
interface (no `ApplyDamage`, no `MarkClean`); the compiler now enforces R2.

**Phase 5 — request-shaped writes (D3/D4).** MarkDirty request event +
keeper handling; OA publishes its spend instead of reaching through the
purse; sheets go dirty exactly as today, pinned.

**Phase 6 — deletion + docs.** Remove `OwnerAware`/`SetOwner` and both
bespoke loader wirings; update `gamectx/doc.go` and `cast.go`; add the
superseded pointer to effect-context's design; ADR for the channel law if
Kirk rules he wants one (open question carried from toolkit#1285).
