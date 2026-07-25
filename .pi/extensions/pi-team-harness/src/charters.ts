import { readFile } from "node:fs/promises";
import { resolve, relative, sep } from "node:path";

import type { RoleAdapter } from "./roles";

export interface LoadedCharter {
  readonly path: string;
  readonly content: string;
}

export function resolveCanonicalCharterPaths(projectRoot: string, role: RoleAdapter): readonly string[] {
  const canonicalRoot = resolve(projectRoot, "docs/teams/roles");
  return role.charterPaths.map((charterPath) => {
    const resolved = resolve(projectRoot, charterPath);
    const pathFromCanonicalRoot = relative(canonicalRoot, resolved);
    if (pathFromCanonicalRoot.startsWith("..") || pathFromCanonicalRoot === "" || pathFromCanonicalRoot.includes(`${sep}..${sep}`)) {
      throw new Error(`charter path escapes canonical roles directory: ${charterPath}`);
    }
    return resolved;
  });
}

export async function loadCanonicalCharters(projectRoot: string, role: RoleAdapter): Promise<readonly LoadedCharter[]> {
  return Promise.all(
    resolveCanonicalCharterPaths(projectRoot, role).map(async (path) => ({
      path,
      content: await readFile(path, "utf8"),
    })),
  );
}
