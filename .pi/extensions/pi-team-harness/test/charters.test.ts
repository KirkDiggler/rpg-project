import assert from "node:assert/strict";
import test from "node:test";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { loadCanonicalCharters, resolveCanonicalCharterPaths } from "../src/charters";
import { roleAdapter, type RoleAdapter } from "../src/roles";

const testDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = resolve(testDir, "../../../..");

test("canonical charter loader resolves and reads the current charter at runtime", async () => {
  const role = roleAdapter("rpg-dnd5e-web-member-ui-ux");
  assert.ok(role);

  const paths = resolveCanonicalCharterPaths(projectRoot, role);
  assert.deepEqual(paths.map((path) => path.replace(`${projectRoot}/`, "")), role.charterPaths);

  const charters = await loadCanonicalCharters(projectRoot, role);
  assert.equal(charters.length, role.charterPaths.length);
  assert.match(charters[0]?.content ?? "", /\n# /);
});

test("canonical charter resolver rejects paths outside docs/teams/roles", () => {
  const outsideRole: RoleAdapter = {
    id: "outside",
    kind: "support",
    team: "Cross-team",
    charterPaths: ["README.md"],
  };

  assert.throws(
    () => resolveCanonicalCharterPaths(projectRoot, outsideRole),
    /escapes canonical roles directory/,
  );
});
