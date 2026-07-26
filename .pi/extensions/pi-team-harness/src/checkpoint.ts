export interface CheckpointInput {
  readonly role: string;
  readonly completedWork: string;
  readonly verification: string;
  readonly blockers: string;
  readonly nextAction: string;
  readonly nextOwner: string;
  readonly branch?: string;
  readonly prUrl?: string;
}

export class CheckpointValidationError extends Error {
  constructor(message: string) {
    super(message);
    this.name = "CheckpointValidationError";
  }
}

function required(label: string, value: string): string {
  const trimmed = value.trim();
  if (trimmed === "") throw new CheckpointValidationError(`${label} is required`);
  return trimmed;
}

function rejectSelfApproval(value: string): void {
  if (/MERGE-READY/i.test(value)) {
    throw new CheckpointValidationError("MERGE-READY is reserved for the applicable independent gate");
  }
}

/** Renders only a GitHub-comment body; it never posts, caches, or persists it. */
export function renderCheckpoint(input: CheckpointInput): string {
  const role = required("role", input.role);
  const completedWork = required("completed work", input.completedWork);
  const verification = required("verification", input.verification);
  const blockers = required("blockers", input.blockers);
  const nextAction = required("next action", input.nextAction);
  const nextOwner = required("next owner", input.nextOwner);
  const branch = input.branch === undefined ? undefined : required("branch", input.branch);
  const prUrl = input.prUrl === undefined ? undefined : required("PR URL", input.prUrl);

  for (const value of [role, completedWork, verification, blockers, nextAction, nextOwner, branch, prUrl]) {
    if (value !== undefined) rejectSelfApproval(value);
  }

  const lines = [
    "## Checkpoint",
    "",
    `- **Completed work:** ${completedWork}`,
    `- **Verification:** ${verification}`,
    `- **Blockers:** ${blockers}`,
    `- **Next action / owner:** ${nextAction} — ${nextOwner}`,
  ];
  if (branch !== undefined) lines.push(`- **Branch:** \`${branch}\``);
  if (prUrl !== undefined) lines.push(`- **PR:** ${prUrl}`);
  lines.push("", `— ${role}, on behalf of KirkDiggler`);
  return lines.join("\n");
}
