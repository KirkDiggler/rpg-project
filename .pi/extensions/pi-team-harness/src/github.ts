export interface GitHubIssue {
  readonly number: number;
  readonly state: string;
  readonly url: string;
}

export interface Project19Fields {
  readonly status?: string;
  readonly team?: string;
  readonly feature?: string;
  readonly kind?: string;
}

export interface Project19Item {
  readonly id: string;
  readonly issueNumber: number;
  readonly fields: Project19Fields;
}

export interface GitHubReader {
  getIssue(issueNumber: number): Promise<GitHubIssue | undefined>;
  getProjectItem(issueNumber: number): Promise<Project19Item | undefined>;
}

/**
 * The command boundary is injected so PIH-2 can prove fixture-only reads without
 * starting a process or making GitHub mutations. A later runtime adapter owns the
 * actual process execution and must preserve this read-only surface.
 */
export interface GhCommandRunner {
  run(args: readonly string[]): Promise<string>;
}

export interface GhCliOptions {
  readonly repository: string;
  readonly projectOwner: string;
  readonly projectNumber: number;
}

type JsonObject = Record<string, unknown>;

function object(value: unknown): JsonObject | undefined {
  return value !== null && typeof value === "object" && !Array.isArray(value)
    ? value as JsonObject
    : undefined;
}

function string(value: unknown): string | undefined {
  return typeof value === "string" && value.trim() !== "" ? value : undefined;
}

function number(value: unknown): number | undefined {
  return typeof value === "number" && Number.isInteger(value) ? value : undefined;
}

function parseJson(output: string): unknown {
  return JSON.parse(output) as unknown;
}

export class GhCliGitHubClient implements GitHubReader {
  constructor(
    private readonly runner: GhCommandRunner,
    private readonly options: GhCliOptions,
  ) {}

  async getIssue(issueNumber: number): Promise<GitHubIssue | undefined> {
    const output = await this.runner.run([
      "issue",
      "view",
      String(issueNumber),
      "--repo",
      this.options.repository,
      "--json",
      "number,state,url",
    ]);
    const value = object(parseJson(output));
    const numberValue = number(value?.number);
    const state = string(value?.state);
    const url = string(value?.url);
    return numberValue === undefined || state === undefined || url === undefined
      ? undefined
      : { number: numberValue, state, url };
  }

  async getProjectItem(issueNumber: number): Promise<Project19Item | undefined> {
    const output = await this.runner.run([
      "project",
      "item-list",
      String(this.options.projectNumber),
      "--owner",
      this.options.projectOwner,
      "--limit",
      "500",
      "--format",
      "json",
    ]);
    const response = object(parseJson(output));
    const items = Array.isArray(response?.items) ? response.items : [];

    for (const rawItem of items) {
      const item = object(rawItem);
      const content = object(item?.content);
      if (number(content?.number) !== issueNumber) continue;
      const id = string(item?.id);
      if (id === undefined) return undefined;
      return {
        id,
        issueNumber,
        fields: {
          status: string(item?.status),
          team: string(item?.team),
          feature: string(item?.feature),
          kind: string(item?.kind),
        },
      };
    }
    return undefined;
  }
}
