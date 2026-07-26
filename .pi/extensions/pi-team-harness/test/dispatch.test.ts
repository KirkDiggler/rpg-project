import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { validateDispatch, type WorktreeInspector } from "../src/dispatch";
import { GhCliGitHubClient, type GhCommandRunner } from "../src/github";

const testDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = resolve(testDir, "../../../..");
const fixturePath = resolve(testDir, "fixtures/github.json");

type Fixture = { issue: Record<string, unknown> | null; projectItems: { items: Array<Record<string, unknown>> } | null };

async function readFixture(): Promise<Fixture> {
  return JSON.parse(await readFile(fixturePath, "utf8")) as Fixture;
}

function githubFromFixture(fixture: Fixture) {
  const runner: GhCommandRunner = {
    async run(args) {
      return JSON.stringify(args[0] === "issue" ? fixture.issue : fixture.projectItems);
    },
  };
  return new GhCliGitHubClient(runner, {
    repository: "KirkDiggler/rpg-project",
    projectOwner: "KirkDiggler",
    projectNumber: 19,
  });
}

function worktree(state: "clean" | "dirty" | "collision"): WorktreeInspector {
  return {
    async inspect() {
      return {
        exists: state === "collision",
        dirty: state === "dirty",
      };
    },
  };
}

async function dispatch(overrides: Partial<Parameters<typeof validateDispatch>[0]> = {}) {
  const fixture = await readFixture();
  return validateDispatch({
    github: githubFromFixture(fixture),
    worktrees: worktree("clean"),
    projectRoot,
    issueNumber: 141,
    roleId: "director",
    branch: "feat/141-pi-github-dispatch-contract",
    worktreePath: "/tmp/rpg-project-141",
    ...overrides,
  });
}

test("valid fixture-only GitHub contract resolves current canonical charter content at dispatch time", async () => {
  const result = await dispatch();

  assert.equal(result.ok, true);
  if (!result.ok) return;
  assert.equal(result.contract.issue.number, 141);
  assert.equal(result.contract.projectItem.fields.team, "Cross-team");
  assert.equal(result.contract.charters[0]?.path, resolve(projectRoot, "docs/teams/roles/director/prompt.md"));
  assert.match(result.contract.charters[0]?.content ?? "", /You are the \*\*director\*\*/);
  assert.equal(Object.hasOwn(result.contract, "localTaskRecord"), false);
});

test("dispatch refuses every missing GitHub prerequisite", async (t) => {
  await t.test("missing issue", async () => {
    const fixture = await readFixture();
    fixture.issue = null;
    const result = await dispatch({ github: githubFromFixture(fixture) });
    assert.deepEqual(result, { ok: false, refusal: "missing issue #141" });
  });

  await t.test("missing Project 19 item", async () => {
    const fixture = await readFixture();
    fixture.projectItems = { items: [] };
    const result = await dispatch({ github: githubFromFixture(fixture) });
    assert.deepEqual(result, { ok: false, refusal: "missing Project 19 item for issue #141" });
  });

  for (const field of ["status", "team", "feature", "kind"] as const) {
    await t.test(`incomplete ${field}`, async () => {
      const fixture = await readFixture();
      fixture.projectItems?.items[0] && (fixture.projectItems.items[0][field] = "");
      const result = await dispatch({ github: githubFromFixture(fixture) });
      assert.deepEqual(result, { ok: false, refusal: `incomplete Project 19 fields: ${field}` });
    });
  }
});

test("dispatch refuses role, dirty-worktree, and collision hazards without creating local task state", async () => {
  const mismatch = await dispatch({ roleId: "rpg-toolkit-member" });
  assert.deepEqual(mismatch, { ok: false, refusal: "role rpg-toolkit-member is incompatible with Team Cross-team" });

  const dirty = await dispatch({ worktrees: worktree("dirty") });
  assert.deepEqual(dirty, { ok: false, refusal: "assigned worktree is dirty: /tmp/rpg-project-141" });

  const collision = await dispatch({ worktrees: worktree("collision") });
  assert.deepEqual(collision, { ok: false, refusal: "worktree collision: /tmp/rpg-project-141 already exists" });
});
