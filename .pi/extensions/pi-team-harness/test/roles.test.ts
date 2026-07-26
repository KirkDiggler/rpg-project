import assert from "node:assert/strict";
import { existsSync, readdirSync, readFileSync } from "node:fs";
import test from "node:test";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { ROLE_ADAPTERS, roleAdapter, roleCanServeTeam } from "../src/roles";

const testDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = resolve(testDir, "../../../..");

test("role adapters name canonical charter paths without copying charter prose", () => {
  assert.ok(ROLE_ADAPTERS.length > 0);
  assert.equal(new Set(ROLE_ADAPTERS.map((role) => role.id)).size, ROLE_ADAPTERS.length);

  const referencedPaths = new Set(ROLE_ADAPTERS.flatMap((role) => role.charterPaths));
  for (const role of ROLE_ADAPTERS) {
    assert.ok(role.charterPaths.length > 0, `${role.id} must name a canonical charter`);
    for (const charterPath of role.charterPaths) {
      assert.ok(charterPath.startsWith("docs/teams/roles/"));
      assert.ok(existsSync(resolve(projectRoot, charterPath)), `${role.id} path must resolve: ${charterPath}`);
    }
  }

  for (const entry of readdirSync(resolve(projectRoot, "docs/teams/roles"), { recursive: true, encoding: "utf8" })) {
    if (entry.endsWith("/prompt.md") || entry.includes("/overlays/")) {
      assert.ok(referencedPaths.has(`docs/teams/roles/${entry}`), `missing canonical adapter path: ${entry}`);
    }
  }

  const rolesSource = readFileSync(resolve(testDir, "../src/roles.ts"), "utf8");
  for (const charterPath of referencedPaths) {
    const heading = readFileSync(resolve(projectRoot, charterPath), "utf8").split("\n")[0];
    assert.equal(rolesSource.includes(heading), false, `adapter must not copy ${charterPath}`);
  }
});

test("role adapters are found by their literal identifier and retain the canonical roster kinds", () => {
  assert.equal(roleAdapter("director-platform")?.kind, "lead");
  assert.equal(roleAdapter("rpg-toolkit-member")?.kind, "member");
  assert.equal(roleAdapter("api-fixer")?.kind, "fixer");
  assert.equal(roleAdapter("explore")?.kind, "explore");
  assert.equal(roleAdapter("janitor")?.kind, "janitor");
  assert.equal(roleAdapter("independent-gate")?.kind, "gate");
  assert.equal(roleAdapter("rpg-dnd5e-web-member-ui-ux")?.charterPaths.at(-1), "docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md");
  assert.equal(roleAdapter("rpg-dnd5e-web-member-assets")?.charterPaths.at(-1), "docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md");
  assert.equal(roleCanServeTeam(roleAdapter("rpg-toolkit-member")!, "Platform"), true);
  assert.equal(roleCanServeTeam(roleAdapter("rpg-toolkit-member")!, "Cross-team"), false);
  assert.equal(roleAdapter("not-a-role"), undefined);
});
