import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { rename, rm, writeFile } from "node:fs/promises";
import test from "node:test";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const testDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = resolve(testDir, "../../../..");
const verifier = resolve(projectRoot, "scripts/verify-pi-team-harness.mjs");
const packageManifest = resolve(projectRoot, ".pi/extensions/pi-team-harness/package.json");
const hiddenManifest = `${packageManifest}.temporary`;
const installedPiDirectory = resolve(projectRoot, ".pi/extensions/pi-team-harness/node_modules/@earendil-works/pi-coding-agent");
const hiddenInstalledPiDirectory = `${installedPiDirectory}.temporary`;
const prohibitedSource = resolve(projectRoot, ".pi/extensions/pi-team-harness/src/prohibited.ts");

function runVerifier(): string {
  return execFileSync(process.execPath, [verifier], { cwd: projectRoot, encoding: "utf8" });
}

test("verifier passes the configured local-only skeleton", () => {
  assert.match(runVerifier(), /PASS/);
});

test("verifier discriminates missing required configuration", async () => {
  await rename(packageManifest, hiddenManifest);
  try {
    assert.throws(
      () => runVerifier(),
      (error: unknown) => /missing required file: .pi\/extensions\/pi-team-harness\/package.json/.test(String(error)),
    );
  } finally {
    await rename(hiddenManifest, packageManifest);
  }

  assert.match(runVerifier(), /PASS/);
});

test("verifier discriminates an absent installed Pi baseline", async () => {
  await rename(installedPiDirectory, hiddenInstalledPiDirectory);
  try {
    assert.throws(
      () => runVerifier(),
      (error: unknown) => /missing node_modules Pi baseline/.test(String(error)),
    );
  } finally {
    await rename(hiddenInstalledPiDirectory, installedPiDirectory);
  }

  assert.match(runVerifier(), /PASS/);
});

test("verifier discriminates prohibited external runtime structure", async () => {
  await writeFile(prohibitedSource, "fetch(\"https://example.test\");\n");
  try {
    assert.throws(
      () => runVerifier(),
      (error: unknown) => {
        const output = error instanceof Error ? `${error.message}\n${(error as { stderr?: string }).stderr ?? ""}` : String(error);
        return /external fetch is prohibited/.test(output);
      },
    );
  } finally {
    await rm(prohibitedSource);
  }

  assert.match(runVerifier(), /PASS/);
});
