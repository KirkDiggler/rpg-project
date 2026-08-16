# Active handoff — 2026-08-15

## Now

**W4 is CLOSED** (toolkit#966; tracker #959's body is the full record). The
session SDK walks, sees, fights, swings, and disengages with zero rules of
its own — stack version-aligned at `session/v0.8.0`, `encounter/v0.9.0`,
`resolution/v0.7.2`, `spatial/v0.9.1`, `dnd5e/v0.94.1`.

**New initiative: API session integration** (rpg-project#227, triplet at
`ideas/session-api/`). Kirk green-lit 2026-08-15: integrate rpg-api **now**
at today's capability — the parity gate set at W4's close is **retired**
("parity was a thought I had for about 10 minutes"). New proto surface
`dnd5e.api.session.v1alpha1.SessionService` mirroring the SDK verbs 1:1;
rpg-api thin by construction; lobby keeps creation; build new **beside** old;
**the cutover is the rip-out**. Web reimplements in parallel once protos are
decided.

## Solid — verified, don't re-derive

- **The seam's laws live in the code's own docs**: `session/doc.go` (S-laws,
  the 15 verbs), `encounter`/`resolution` godoc, toolkit
  `docs/adr/DECISIONS.md` (the digest — read it, not the ADR corpus).
- **`session.Event` was designed to map onto a proto message** — flat,
  per-recipient, audience-projected inside the SDK, monotonic gapless `Seq`
  keyed to the story log. The host never filters visibility; `GetStory` is
  the resync source of truth.
- **Old-path blast radius measured 2026-08-15** (recorded in
  `ideas/session-api/brainstorm.md` + issue #227): ~22.5k lines on the live
  path; lobby (~7.4k) re-points, not deleted; `internal/components/dungeon/`
  + `spawner/` (~11k) are unwired and free to delete; rpg-api pins old
  `encounter v0.53.0` / `dnd5e v0.72.0` with zero session/resolution refs —
  greenfield adoption.
- **Standing toolkit laws minted in W4**: capabilities are supplied, never
  defaulted (resolution#1033; #1036 tracks the remaining class); read a
  module's version from its own tags, never a consumer's go.mod; a wave's
  version is decided by its first merge (auto-tag on merge).
- **Day-one capability list of the new stack** (deliberate, not bugs —
  don't re-file): no monster attacking back (behavior work incoming), no
  action economy (#1035), fights end by decision not defeat (#1024's
  remaining half), no stealth/surprise (#1020).

## Open questions

- **No self-position read** (toolkit#933): a reconnecting client cannot
  learn its own position; SDK fix precedes the W1 read shapes (design
  rule 11) and rides the seam reshape below.
- **Chapter 2 / board 13 ("Combat Verbs" on the v1alpha2 route) is
  superseded by #227** — needs Kirk's confirmation, then the board note.
- **Coexistence flag shape** (which stack `StartEncounter` creates on) —
  settled at implementation, design pins "exactly one, never both".

## Next

**The SDK seam reshape** (toolkit) — Kirk's world-model ruling is IN
(2026-08-15, design §0: rooms internal to the encounter, seam projects
absolute geometry, wire = one map), so the remaining W1 preconditions are
toolkit-side: absolute positions on every seam output, `Traverse` retired,
the self-position read (#933), and the forcing case (reference tomb runs,
entrance → hall → tomb as one move surface). Sequencing vs the combat
capability push is Kirk's slotting. Un-gated proto messages (stream,
errors, fight verbs) may draft anytime. Then W1 merge → W2 rpg-api beside
the old path → W3 web in parallel → W4 cutover.

## Decision log

| Date | Decision | Visible at |
|---|---|---|
| 2026-08-16 | W4 closed: session verbs complete, stack aligned; monster attackers refused by name (earned by behavior work); `Turn.Yours` refused — "may I act now" is the economy's question (#1035) | toolkit#966, tracker #959 |
| 2026-08-16 | Adoption gate refined to capability parity — **superseded next day** | #959 → rpg-project#227 |
| 2026-08-15 | **Parity gate retired; integrate now at today's capability** | rpg-project#227 |
| 2026-08-15 | **New proto surface** `dnd5e.api.session.v1alpha1`; old `EncounterService` deleted at cutover, never reimplemented (a wrapper would dictate the contract) | `ideas/session-api/design.md` |
| 2026-08-15 | Creation stays the lobby's; `StartSession`/`Spawn` not exposed on the new service | design §2 rule 5 |
| 2026-08-15 | Stream = `session.Event` mirrored, bytes payload passthrough; host never filters visibility | design MUST-3/4 |
| 2026-08-15 | Build new beside old; **cutover = the rip-out**; web parallel implementation starts when protos are decided | rpg-project#227, plan.md |
| 2026-08-15 | **World-model ruling: one map at the seam** — "the encounter has rooms but projects the absolute geo of the dungeon so the session package sees it as all one map"; no `Traverse` on the wire, all wire positions dungeon-absolute, door/lock gap unaffected | design §0, #227 |

## Carried follow-ups — filed, none blocking

All re-verified OPEN 2026-08-15: toolkit#948 (LoadFromData drops conditions),
#933 (Members() without positions), #940 (beats to every member), #941 (story
tags coarser than beats), #934 (validation asymmetries), #951 (ErrNoMember
conflation), rpg-project#218 (hand-maintained lists fail open). Deliberate
W4 opens: #1020, #1024 (defeat half), #1035, #1036. (#1022 CLOSED.)
In every composition case the layering holds: fixing it in the composition
fixes the SDK for free.

## Other lanes

- **Semantic scope (#180)** — parked. Wave 0 live-verified; Wave 1 not
  started; issues cut when the lane picks back up.
- **rpg-api#793 wiring** — Platform's whenever; map surface live, the
  Locate→Move trap called out.

## Pointers

- **This initiative**: `ideas/session-api/{brainstorm,design,plan}.md`,
  issue rpg-project#227, board 19.
- **Decisions digest**: toolkit `docs/adr/DECISIONS.md` (CI-enforced).
- **Before designing anything touching turns/time/perception/story**: toolkit
  `play/README.md` + the relevant `play/*` doc.go — this lane once lost a
  full turn re-deriving `play/clock`.
- **SDK deep record**: toolkit `docs/ideas/session-sdk/{design,plan}.md`,
  `resolution/ARCHITECTURE.md`, journeys 051–054.
- **Doc ownership**: each repo owns its status/quality docs; rpg-project says
  how we work — except `docs/ideas`, which records the plan as it stood.
- **Full narrative history**: `git log -p sessions/active.md`.
