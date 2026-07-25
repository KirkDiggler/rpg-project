export type Team = "Platform" | "UI/UX" | "Assets" | "Cross-team";

export interface RoleAdapter {
  readonly id: string;
  readonly team: Team;
  readonly charterPaths: readonly string[];
}

export const ROLE_ADAPTERS: readonly RoleAdapter[] = [
  { id: "director", team: "Cross-team", charterPaths: ["docs/teams/roles/director/prompt.md"] },
  {
    id: "director-platform",
    team: "Platform",
    charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/platform.md",
    ],
  },
  {
    id: "director-ui-ux",
    team: "UI/UX",
    charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/ui-ux.md",
    ],
  },
  {
    id: "director-assets",
    team: "Assets",
    charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/assets.md",
    ],
  },
  { id: "rpg-toolkit-member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-toolkit-member/prompt.md"] },
  { id: "rpg-api-member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-api-member/prompt.md"] },
  { id: "rpg-api-protos-member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-api-protos-member/prompt.md"] },
  { id: "rpg-deployment-member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-deployment-member/prompt.md"] },
  { id: "rpg-game-assets-member", team: "Assets", charterPaths: ["docs/teams/roles/rpg-game-assets-member/prompt.md"] },
  {
    id: "rpg-dnd5e-web-member-ui-ux",
    team: "UI/UX",
    charterPaths: [
      "docs/teams/roles/rpg-dnd5e-web-member/prompt.md",
      "docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md",
    ],
  },
  {
    id: "rpg-dnd5e-web-member-assets",
    team: "Assets",
    charterPaths: [
      "docs/teams/roles/rpg-dnd5e-web-member/prompt.md",
      "docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md",
    ],
  },
  { id: "toolkit-fixer", team: "Platform", charterPaths: ["docs/teams/roles/toolkit-fixer/prompt.md"] },
  { id: "api-fixer", team: "Platform", charterPaths: ["docs/teams/roles/api-fixer/prompt.md"] },
  {
    id: "bug-fix-coordinator",
    team: "Cross-team",
    charterPaths: ["docs/teams/roles/bug-fix-coordinator/prompt.md"],
  },
  {
    id: "platform-simplifier",
    team: "Cross-team",
    charterPaths: ["docs/teams/roles/platform-simplifier/prompt.md"],
  },
  {
    id: "project-manager",
    team: "Cross-team",
    charterPaths: ["docs/teams/roles/project-manager/prompt.md"],
  },
  { id: "web-fixer", team: "UI/UX", charterPaths: ["docs/teams/roles/web-fixer/prompt.md"] },
  { id: "explore", team: "Cross-team", charterPaths: ["docs/teams/roles/explore/prompt.md"] },
  { id: "janitor", team: "Cross-team", charterPaths: ["docs/teams/roles/janitor/prompt.md"] },
  { id: "independent-gate", team: "Cross-team", charterPaths: ["docs/teams/roles/independent-gate/prompt.md"] },
] as const;

export function roleAdapter(roleId: string): RoleAdapter | undefined {
  return ROLE_ADAPTERS.find((role) => role.id === roleId);
}
