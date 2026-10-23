# Team map

Living document, corrected by running real work under it (first wave:
the mad king close-out, rpg-project#569). Initiative: rpg-project#680.
Not a law doc — change it by PR when friction says the seams are wrong.

**One distinction governs everything here: standing ownership ≠ living
context.** Umbrellas and disciplines are standing: they own seams and
accumulate craft knowledge. A *feature* — the mad king, monster-ai, the
dungeon builder — is living context: it has needs, state, and a finish
line, and it runs *hosted by* a team without being one. Monster-ai is
the first proof of the split: chartered in rpg-project#511 for an
ownership that never materialized, it is `feature:monster-ai`, hosted
by Platform, open past its first slice.

## The umbrellas

### Platform

**Owns: rules in the toolkit, hosts thin everywhere.** rpg-toolkit,
rpg-api, rpg-api-protos, rpg-deployment, game-dev workspace tooling.

| Discipline | Owns |
|---|---|
| engine-primitives | `core`, `dice`, `events`, `play/*`, `mind/perception`, `mechanics/*`, `tools/spatial` and friends — composable, game-agnostic machinery |
| dnd5e-rules | `rulebooks/dnd5e` rules content — character, combat, conditions, spells, monsters. RAW is a guide, not a law |
| game-layer | `rulebooks/dnd5e/{encounter,resolution,session}` — composition, turn flow, and the ID-based seam the API speaks |
| host-wire-and-infra | rpg-api handlers/orchestrators/repositories, rpg-api-protos wire contracts, rpg-deployment pipelines, game-dev tooling |

The dependency direction is acyclic — primitives → rules → game layer →
host — and a change that wants to point upstream has found a design gap,
not a refactor. Folds: platform charter; the toolkit/api/protos/
deployment/game-dev members; the api/toolkit/web fixer lenses (the
pre-member dispatch model, kept as compatibility inputs).

### UI programming

**Owns: web craft — render and call, never compute.** rpg-dnd5e-web:
surfaces, streams, concepts, three.js/pixi presentation. Folds:
rpg-dnd5e-web-member, web-fixer. The lesson corpus shows web craft
knowledge living embedded in cross-team notes — this umbrella's memory
starts thin by neglect, and the first waves under it should expect to
write more lines than other teams.

### UI-UX

**Owns: screens, interaction, legibility, accessibility — what a human
or an agent's output reads like.** Chartered rpg-project#511; routes
needs upstream rather than computing them. The encounter studio's
read-back principle (agent-written fields must survive human read-back)
is the umbrella's standing test.

### Assets

**Owns: the licensed pipeline from Synty source to rendered evidence.**
rpg-game-assets stays the private canonical store; nothing licensed is
committed to a public repo.

| Discipline | Owns |
|---|---|
| environment/props | buildings, props, environment kits, dungeon dressing |
| character-NPC modeling | characters, NPCs, monsters, animation and its evidence receipts |

Folds: assets charter, rpg-game-assets-member.

### QA

**Owns: adversarial review and the walk discipline — evidence over
prose, fresh-context verdicts, the castle steps run for real before a
wave merges.** Folds: independent-gate, explore.

### Orchestration (above the umbrellas)

Not a discipline: the layer that keeps the other five coherent.
**cross-team** (the director's successor) owns integration seams,
end-to-end verification, and process; **janitor** owns curation,
including the team-memory ingest-and-grooming duty (rpg-project#678).
Neither authors product code.

### Not standing: perf/testing

No role, charter, skill, or lesson references performance work anywhere
in the workspace. The label exists; the charter is created only when a
use case demands it (deferred, rpg-project#680).

## Retired

`docs/teams/roles/_archive/` holds the record; git history holds the
rest.

| Role | Successor |
|---|---|
| director | cross-team, once its `overlays/` fold into the named charters |
| project-manager | the initiative/journey issue flow on GitHub |
| platform-simplifier | the design skill + this map |
| bug-fix-coordinator | per-issue dispatch under working-agreements |
| monster-ai (charter) | `feature:monster-ai`, hosted by Platform |

## Labels

Every new issue carries at least one `team:` label; features carry
`feature:<name>` ad hoc. The names match the team-memory scope
vocabulary (rpg-project#678, R8) so a triaged issue and a captured note
name the same thing. Repos: all seven carry the six `team:*` labels;
`feature:mad-king` exists in rpg-project, rpg-toolkit, rpg-dnd5e-web,
rpg-api. Existing legacy labels stay; toolkit's duplicates and dead
phases are debt to clean once usage shows what sticks.
