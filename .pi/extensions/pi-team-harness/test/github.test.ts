import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { GhCliGitHubClient, type GhCommandRunner } from "../src/github";

const testDir = dirname(fileURLToPath(import.meta.url));
const fixturePath = resolve(testDir, "fixtures/github.json");

async function fixture(): Promise<{ issue: unknown; projectItems: unknown }> {
  return JSON.parse(await readFile(fixturePath, "utf8"));
}

test("GitHub adapter reads issue and Project 19 facts through injected gh", async () => {
  const data = await fixture();
  const calls: readonly string[][] = [];
  const runner: GhCommandRunner = {
    async run(args) {
      (calls as string[][]).push([...args]);
      if (args[0] === "issue") return JSON.stringify(data.issue);
      return JSON.stringify(data.projectItems);
    },
  };
  const github = new GhCliGitHubClient(runner, {
    repository: "KirkDiggler/rpg-project",
    projectOwner: "KirkDiggler",
    projectNumber: 19,
  });

  assert.deepEqual(await github.getIssue(141), {
    number: 141,
    state: "OPEN",
    url: "https://github.com/KirkDiggler/rpg-project/issues/141",
  });
  assert.deepEqual(await github.getProjectItem(141), {
    id: "PVTI_fixture_141",
    issueNumber: 141,
    fields: { status: "In Progress", team: "Cross-team", feature: "Infra", kind: "Build" },
  });
  assert.deepEqual(calls, [
    ["issue", "view", "141", "--repo", "KirkDiggler/rpg-project", "--json", "number,state,url"],
    ["project", "item-list", "19", "--owner", "KirkDiggler", "--limit", "500", "--format", "json"],
  ]);
});

test("GitHub adapter treats absent fixture facts as absent rather than inventing state", async () => {
  const runner: GhCommandRunner = { async run() { return "null"; } };
  const github = new GhCliGitHubClient(runner, {
    repository: "KirkDiggler/rpg-project",
    projectOwner: "KirkDiggler",
    projectNumber: 19,
  });

  assert.equal(await github.getIssue(141), undefined);
  assert.equal(await github.getProjectItem(141), undefined);
});
