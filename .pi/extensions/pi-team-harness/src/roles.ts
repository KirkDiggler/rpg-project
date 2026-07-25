export type Team = "Platform" | "UI/UX" | "Assets" | "Cross-team";
export type RoleKind = "lead" | "member" | "fixer" | "explore" | "janitor" | "gate" | "support";

/** A runtime lookup record, not a copy of role policy. */
export interface RoleAdapter {
  readonly id: string;
  readonly kind: RoleKind;
  readonly team: Team;
  readonly charterPaths: readonly string[];
}

export const ROLE_ADAPTERS: readonly RoleAdapter[] = [
  { id: "director", kind: "lead", team: "Cross-team", charterPaths: ["docs/teams/roles/director/prompt.md"] },
  {
    id: "director-platform", kind: "lead", team: "Platform", charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/platform.md",
    ],
  },
  {
    id: "director-ui-ux", kind: "lead", team: "UI/UX", charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/ui-ux.md",
    ],
  },
  {
    id: "director-assets", kind: "lead", team: "Assets", charterPaths: [
      "docs/teams/roles/director/prompt.md",
      "docs/teams/roles/director/overlays/assets.md",
    ],
  },
  { id: "rpg-toolkit-member", kind: "member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-toolkit-member/prompt.md"] },
  { id: "rpg-api-member", kind: "member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-api-member/prompt.md"] },
  { id: "rpg-api-protos-member", kind: "member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-api-protos-member/prompt.md"] },
  { id: "rpg-deployment-member", kind: "member", team: "Platform", charterPaths: ["docs/teams/roles/rpg-deployment-member/prompt.md"] },
  { id: "rpg-game-assets-member", kind: "member", team: "Assets", charterPaths: ["docs/teams/roles/rpg-game-assets-member/prompt.md"] },
  {
    id: "rpg-dnd5e-web-member-ui-ux", kind: "member", team: "UI/UX", charterPaths: [
      "docs/teams/roles/rpg-dnd5e-web-member/prompt.md",
      "docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md",
    ],
  },
  {
    id: "rpg-dnd5e-web-member-assets", kind: "member", team: "Assets", charterPaths: [
      "docs/teams/roles/rpg-dnd5e-web-member/prompt.md",
      "docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md",
    ],
  },
  { id: "toolkit-fixer", kind: "fixer", team: "Platform", charterPaths: ["docs/teams/roles/toolkit-fixer/prompt.md"] },
  { id: "api-fixer", kind: "fixer", team: "Platform", charterPaths: ["docs/teams/roles/api-fixer/prompt.md"] },
  { id: "web-fixer", kind: "fixer", team: "UI/UX", charterPaths: ["docs/teams/roles/web-fixer/prompt.md"] },
  { id: "explore", kind: "explore", team: "Cross-team", charterPaths: ["docs/teams/roles/explore/prompt.md"] },
  { id: "janitor", kind: "janitor", team: "Cross-team", charterPaths: ["docs/teams/roles/janitor/prompt.md"] },
  { id: "independent-gate", kind: "gate", team: "Cross-team", charterPaths: ["docs/teams/roles/independent-gate/prompt.md"] },
  { id: "bug-fix-coordinator", kind: "support", team: "Cross-team", charterPaths: ["docs/teams/roles/bug-fix-coordinator/prompt.md"] },
  { id: "platform-simplifier", kind: "support", team: "Cross-team", charterPaths: ["docs/teams/roles/platform-simplifier/prompt.md"] },
  { id: "project-manager", kind: "support", team: "Cross-team", charterPaths: ["docs/teams/roles/project-manager/prompt.md"] },
] as const;

export function roleAdapter(roleId: string): RoleAdapter | undefined {
  return ROLE_ADAPTERS.find((role) => role.id === roleId);
}

export function roleCanServeTeam(role: RoleAdapter, team: Team): boolean {
  return role.team === team;
}
