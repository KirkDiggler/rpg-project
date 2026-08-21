## Files Retrieved

1. `CLAUDE.md` (lines 46-76, 168-212) — issue/board law, integration branches, cross-repo design workflow, one-branch-per-wave rule.
2. `.opencode/skills/project-board-workflow/SKILL.md` (lines 6-30) — Project 19 lifecycle, review gates, closing-keyword rules.
3. `ideas/opencode-team-workflow/design.md` (lines 30-83, 502-533) — exact Project 19 fields, `game-dev` routing, gate policy.
4. `docs/teams/roles/README.md` (lines 18-79) — standing repository owners and supporting roles.
5. `docs/teams/roles/game-dev-member/prompt.md` (lines 8-65) — workstation/bootstrap ownership and cross-repo refusal boundary.
6. `docs/teams/roles/{rpg-toolkit-member,rpg-api-member,rpg-dnd5e-web-member}/prompt.md` — rules/API/web ownership boundaries.
7. `docs/howto/run-the-game-locally.md` (lines 1-42, 101-180) — current dev-image, local override, and lab conventions.
8. `ideas/game-screen-rebuild/lobby-surface.md` (lines 25-107, 182-188) — approved real lobby flow and identity/ownership constraints.
9. `ideas/dungeon-builder/design.md` (lines 1-33) — canonical Dungeon Builder authority and layer responsibilities.
10. Sibling evidence: `rpg-api/scripts/toolkit-local-override.sh` (lines 1-149), `rpg-api/cmd/devseed/main.go` (lines 1-18, 204-250, 326-451), and `rpg-dnd5e-web/src/{concepts/ConceptsView.tsx,author/AuthorView.tsx}` — current gaps the new work must replace or reuse.

## Key Code

### Canonical routing brief

No existing issue or design exactly covers this approved direction. Create these as one linked wave on [Project 19](https://github.com/users/KirkDiggler/projects/19):

| Owning repo | Required issue | Team / Feature / Kind / initial Status | Owner and base |
|---|---|---|---|
| `rpg-project` | Umbrella/design tracker: **Toolkit contributor onboarding + live sandbox loop** | **Cross-team / Infra / Build / Todo** | Coordinating lead; `origin/main` |
| `rpg-api` | Make the single-module local override select `rulebooks/dnd5e`; add an idempotent seed client that creates the Protection Fighter and Barbarian through the running `CharacterService` draft/choice/finalize RPCs | **Platform / Infra / Build / Todo** | `rpg-api-member`; `origin/dev` |
| `game-dev` | WSL2-facing bootstrap/start/refresh/seed commands and contributor documentation; delegate API/web/runtime work rather than reimplement it | **Cross-team / Infra / Build / Todo** | `game-dev-member`; `origin/main` |
| `rpg-dnd5e-web` | Dev-only contributor sandbox: reuse Dungeon Builder with one template; compose 1–2 seats in either order; call real `PutDungeon` and lobby create/join/ready/start APIs | **UI/UX / Infra / Build / Todo** | `rpg-dnd5e-web-member` + UI/UX overlay; `origin/dev` |
| `rpg-project` | Final clean-WSL2, issue-only end-to-end verification | **Cross-team / Infra / Verify / Todo** | Independent verification owner; no branch unless a defect requires a separate owning-repo issue |

Make the implementation and verification issues sub-issues/checklist entries of the umbrella. Each implementation PR closes only its own same-repository issue.

**Do not pre-file MVP issues in `rpg-toolkit`, `rpg-api-protos`, or `rpg-deployment`:**

- The toolkit already exposes Protection as a selectable/implemented fighting style; implementation evidence is [rpg-toolkit PR #438](https://github.com/KirkDiggler/rpg-toolkit/pull/438). The stale broad [toolkit #148](https://github.com/KirkDiggler/rpg-toolkit/issues/148) is not this work.
- Existing contracts already provide `CharacterService` draft/finalize, `AuthoringService.PutDungeon`, and the lobby RPCs.
- `game-dev` already starts the compose stack. File a deployment issue only if the plan chooses to modify the isolated lab itself; [rpg-deployment #65](https://github.com/KirkDiggler/rpg-deployment/issues/65) / [PR #66](https://github.com/KirkDiggler/rpg-deployment/pull/66) currently hardcode the `encounter` module.

### Canonical design path

Use:

- `ideas/toolkit-contributor-sandbox/design.md`
- `ideas/toolkit-contributor-sandbox/plan.md`

The umbrella issue backs one ready `rpg-project` PR. Per `CLAUDE.md:178-189`, approve `design.md` first, then add `plan.md` to that same PR; keep the PR open until all implementation and clean-WSL2 verification are complete.

### Ordering

1. File/board the umbrella and open the design PR.
2. Record the supplied approved direction as fixed scope; do not reopen generic-scenario or Redis-write alternatives.
3. After design approval, add and approve the plan; then create/board the repository implementation issues.
4. Develop the web composition early/parallel, but merge dependencies in this order:
   1. `rpg-api` override + real CharacterService seeder.
   2. `game-dev` command façade.
   3. Web sandbox against the landed real path.
5. Run the clean-WSL2 Verify issue, including one-seat Fighter, one-seat Barbarian, Fighter→Barbarian, and Barbarian→Fighter using the same single template/scenario.
6. Merge the idea PR and close the umbrella only after verification.

Only the web PR clearly requires the product-behavior Sol gate. The docs, wrappers, override helper, seed client, and evidence-only verification are workflow/setup tooling unless they alter production server behavior.

## Architecture

`rpg-project` owns design and policy; `game-dev` owns the WSL2 cockpit; `rpg-api` owns the developer adapter and real API seed client; the web owns the dev-only interaction surface. `rpg-toolkit` remains the edited rules product, not the owner of orchestration, lobby state, or sandbox infrastructure. Existing protos remain the contract.

## Review Findings

- **Blocker — process:** No matching umbrella/design or Project 19 item exists. Repository branches cannot be cut until the issue/board/design-plan gate exists (`CLAUDE.md:54-58,178-189`).
- **High:** `rpg-api/scripts/toolkit-local-override.sh:15-18,120-133` permits only `rpg-toolkit/encounter`; the approved `rulebooks/dnd5e` loop needs an API-owned change.
- **High:** `rpg-api/cmd/devseed/main.go:1-4,326-451` explicitly bypasses draft/finalize and writes character/encounter Redis records directly. Do not extend this path for the new seed command.
- **High:** `rpg-dnd5e-web/src/concepts/ConceptsView.tsx:122-130` mounts the current dev Dungeon Builder with `forceFixtures`, explicitly preventing `PutDungeon`. Reuse the component tree, not that fixture-only behavior.
- **Medium:** Lobby membership validates character ownership against authenticated players (`ideas/game-screen-rebuild/lobby-surface.md:82-85`). The plan must name the two dev identities and keep seeded character ownership aligned; it must not weaken auth to support seat ordering.
- **Medium — policy drift:** API/web role charters still say branch from `main`, but current root policy requires `origin/dev` (`CLAUDE.md:65-75`). Root policy governs.
- **Low — routing hazard:** Legacy [rpg-api #477](https://github.com/KirkDiggler/rpg-api/issues/477) and [rpg-api-protos #140](https://github.com/KirkDiggler/rpg-api-protos/issues/140) concern an unused `SandboxRoom` subsystem. They must not be reused for this dev sandbox.

## Start Here

Start with `CLAUDE.md:178-189`: it determines the canonical artifact and tracking surface. Then use the issue table above to build the plan’s per-repository units.