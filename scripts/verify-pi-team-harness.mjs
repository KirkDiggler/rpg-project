#!/usr/bin/env node

import { existsSync, readdirSync, readFileSync, statSync } from "node:fs";
import { resolve, relative } from "node:path";

const root = resolve(import.meta.dirname, "..");
const extensionDir = resolve(root, ".pi/extensions/pi-team-harness");
const expectedPiVersion = "0.82.1";
const requiredFiles = [
  ".pi/extensions/pi-team-harness/package.json",
  ".pi/extensions/pi-team-harness/package-lock.json",
  ".pi/extensions/pi-team-harness/index.ts",
  ".pi/extensions/pi-team-harness/src/roles.ts",
  ".pi/extensions/pi-team-harness/src/charters.ts",
  ".pi/extensions/pi-team-harness/src/worker-policy.ts",
  ".pi/extensions/pi-team-harness/test/roles.test.ts",
  ".pi/extensions/pi-team-harness/test/charters.test.ts",
  ".pi/extensions/pi-team-harness/test/worker-policy.test.ts",
];
const forbiddenDependencyNames = new Set([
  "@modelcontextprotocol/sdk",
  "better-sqlite3",
  "express",
  "fastify",
  "ioredis",
  "redis",
  "socket.io",
  "sqlite3",
  "ws",
]);
const forbiddenSourcePatterns = [
  { name: "Pi session persistence", pattern: /\bappendEntry\s*\(/ },
  { name: "browser-style durable store", pattern: /\b(?:indexedDB|localStorage|sessionStorage|openDatabase)\b/ },
  { name: "daemon listener", pattern: /\b(?:createServer|listen)\s*\(/ },
  { name: "network listener import", pattern: /from\s+["']node:(?:dgram|http|https|net)["']/ },
  { name: "worker process", pattern: /\b(?:spawn|exec|execFile|fork)\s*\(/ },
  { name: "timer queue", pattern: /\b(?:setInterval|setTimeout)\s*\(/ },
  { name: "durable write", pattern: /\b(?:writeFile|appendFile|mkdir)\s*\(/ },
  { name: "external fetch", pattern: /\bfetch\s*\(/ },
];

const failures = [];
const fail = (message) => failures.push(message);

for (const file of requiredFiles) {
  if (!existsSync(resolve(root, file))) fail(`missing required file: ${file}`);
}

if (failures.length === 0) {
  const packagePath = resolve(extensionDir, "package.json");
  let manifest;
  try {
    manifest = JSON.parse(readFileSync(packagePath, "utf8"));
  } catch (error) {
    fail(`invalid package.json: ${error instanceof Error ? error.message : String(error)}`);
  }

  if (manifest) {
    if (manifest.private !== true) fail("package.json must set private: true");
    if (manifest.type !== "module") fail("package.json must set type: module");
    if (manifest.engines?.node !== ">=18") fail("package.json must require Node >=18");
    if (JSON.stringify(manifest.pi?.extensions) !== JSON.stringify(["./index.ts"])) {
      fail("package.json must declare ./index.ts as its Pi extension");
    }

    for (const dependency of [
      "@earendil-works/pi-coding-agent",
      "@earendil-works/pi-tui",
      "typebox",
    ]) {
      if (manifest.peerDependencies?.[dependency] !== "*") {
        fail(`peer dependency ${dependency} must use the Pi package range *`);
      }
    }

    const expectedDevDependencies = {
      "@earendil-works/pi-coding-agent": expectedPiVersion,
      "@earendil-works/pi-tui": expectedPiVersion,
      typebox: "1.1.38",
      tsx: "4.23.1",
      typescript: "5.9.3",
    };
    for (const [dependency, version] of Object.entries(expectedDevDependencies)) {
      if (manifest.devDependencies?.[dependency] !== version) {
        fail(`dev dependency ${dependency} must be pinned to ${version}`);
      }
    }

    const declaredDependencies = {
      ...manifest.dependencies,
      ...manifest.devDependencies,
      ...manifest.peerDependencies,
    };
    for (const dependency of forbiddenDependencyNames) {
      if (dependency in declaredDependencies) fail(`prohibited dependency: ${dependency}`);
    }
  }

  const installedPiPackage = resolve(extensionDir, "node_modules/@earendil-works/pi-coding-agent/package.json");
  if (!existsSync(installedPiPackage)) {
    fail("missing node_modules Pi baseline; run npm --prefix .pi/extensions/pi-team-harness ci");
  } else {
    try {
      const installed = JSON.parse(readFileSync(installedPiPackage, "utf8"));
      if (installed.version !== expectedPiVersion) {
        fail(`installed Pi baseline must be ${expectedPiVersion}, found ${installed.version ?? "unknown"}`);
      }
    } catch (error) {
      fail(`invalid installed Pi package metadata: ${error instanceof Error ? error.message : String(error)}`);
    }
  }

  const rolesSource = readFileSync(resolve(extensionDir, "src/roles.ts"), "utf8");
  const charterPathMatches = [...rolesSource.matchAll(/"(docs\/teams\/roles\/[^\"]+\.md)"/g)].map((match) => match[1]);
  if (charterPathMatches.length === 0) {
    fail("roles adapter must name literal canonical charter paths");
  }
  for (const charterPath of charterPathMatches) {
    if (!existsSync(resolve(root, charterPath))) fail(`canonical charter path does not resolve: ${charterPath}`);
  }

  const sourceRoots = [resolve(extensionDir, "index.ts"), resolve(extensionDir, "src")];
  const sourceFiles = [];
  const collectSourceFiles = (path) => {
    if (statSync(path).isFile()) {
      if (path.endsWith(".ts")) sourceFiles.push(path);
      return;
    }
    for (const entry of readdirSync(path, { withFileTypes: true })) {
      collectSourceFiles(resolve(path, entry.name));
    }
  };
  for (const sourceRoot of sourceRoots) collectSourceFiles(sourceRoot);

  for (const sourceFile of sourceFiles) {
    const source = readFileSync(sourceFile, "utf8");
    for (const forbidden of forbiddenSourcePatterns) {
      if (forbidden.pattern.test(source)) {
        fail(`${forbidden.name} is prohibited in ${relative(root, sourceFile)}`);
      }
    }
  }
}

if (failures.length > 0) {
  console.error("PI team-harness verifier: FAIL");
  for (const message of failures) console.error(`- ${message}`);
  process.exitCode = 1;
} else {
  console.log("PI team-harness verifier: PASS (Pi 0.82.1 skeleton is deterministic and local-only)");
}
