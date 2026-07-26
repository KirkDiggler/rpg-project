import { loadCanonicalCharters, type LoadedCharter } from "./charters";
import type { GitHubIssue, GitHubReader, Project19Item } from "./github";
import { roleAdapter, roleCanServeTeam, type Team } from "./roles";

export interface WorktreeInspection {
  readonly exists: boolean;
  readonly dirty: boolean;
}

/** Injected boundary: PIH-2 never creates, removes, or changes a worktree. */
export interface WorktreeInspector {
  inspect(path: string): Promise<WorktreeInspection>;
}

export interface DispatchRequest {
  readonly github: GitHubReader;
  readonly worktrees: WorktreeInspector;
  readonly projectRoot: string;
  readonly issueNumber: number;
  readonly roleId: string;
  readonly branch: string;
  readonly worktreePath: string;
}

export interface ValidDispatch {
  readonly issue: GitHubIssue;
  readonly projectItem: Project19Item;
  readonly roleId: string;
  readonly branch: string;
  readonly worktreePath: string;
  readonly charters: readonly LoadedCharter[];
}

export type DispatchRefusal =
  | `missing issue #${number}`
  | `issue #${number} is not open`
  | `missing Project 19 item for issue #${number}`
  | `incomplete Project 19 fields: ${string}`
  | `unknown role: ${string}`
  | `role ${string} is incompatible with Team ${string}`
  | `missing fresh branch name`
  | `missing isolated worktree path`
  | `assigned worktree is dirty: ${string}`
  | `worktree collision: ${string} already exists`;

export type DispatchResult =
  | { readonly ok: true; readonly contract: ValidDispatch }
  | { readonly ok: false; readonly refusal: DispatchRefusal };

function present(value: string | undefined): boolean {
  return value !== undefined && value.trim() !== "";
}

function projectTeam(item: Project19Item): Team | undefined {
  const team = item.fields.team;
  return team === "Platform" || team === "UI/UX" || team === "Assets" || team === "Cross-team"
    ? team
    : undefined;
}

/**
 * Validates only dispatch prerequisites and produces an in-memory contract. It
 * does not spawn, write, post a comment, mutate Project 19, or retain state.
 */
export async function validateDispatch(request: DispatchRequest): Promise<DispatchResult> {
  const issue = await request.github.getIssue(request.issueNumber);
  if (issue === undefined) return { ok: false, refusal: `missing issue #${request.issueNumber}` };
  if (issue.state.toLowerCase() !== "open") {
    return { ok: false, refusal: `issue #${request.issueNumber} is not open` };
  }

  const projectItem = await request.github.getProjectItem(request.issueNumber);
  if (projectItem === undefined) {
    return { ok: false, refusal: `missing Project 19 item for issue #${request.issueNumber}` };
  }

  const missingFields = (["status", "team", "feature", "kind"] as const)
    .filter((field) => !present(projectItem.fields[field]));
  if (missingFields.length > 0) {
    return { ok: false, refusal: `incomplete Project 19 fields: ${missingFields.join(", ")}` };
  }

  const role = roleAdapter(request.roleId);
  if (role === undefined) return { ok: false, refusal: `unknown role: ${request.roleId}` };
  const team = projectTeam(projectItem);
  if (team === undefined || !roleCanServeTeam(role, team)) {
    return { ok: false, refusal: `role ${role.id} is incompatible with Team ${projectItem.fields.team}` };
  }
  if (request.branch.trim() === "") return { ok: false, refusal: "missing fresh branch name" };
  if (request.worktreePath.trim() === "") return { ok: false, refusal: "missing isolated worktree path" };

  const inspection = await request.worktrees.inspect(request.worktreePath);
  if (inspection.exists) {
    return { ok: false, refusal: `worktree collision: ${request.worktreePath} already exists` };
  }
  if (inspection.dirty) {
    return { ok: false, refusal: `assigned worktree is dirty: ${request.worktreePath}` };
  }

  // This is intentionally last: content is read from canonical paths only when
  // a valid dispatch is actually being formed, never cached in an adapter.
  const charters = await loadCanonicalCharters(request.projectRoot, role);
  return {
    ok: true,
    contract: {
      issue,
      projectItem,
      roleId: role.id,
      branch: request.branch,
      worktreePath: request.worktreePath,
      charters,
    },
  };
}
