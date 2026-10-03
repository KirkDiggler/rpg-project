#!/usr/bin/env python3
"""Reproduce #508's local geometry/knowledge experiment; never edit provider source."""

import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("toolkit", type=Path, help="toolkit checkout/worktree at the investigated revision")
    parser.add_argument("api", type=Path, help="API checkout/worktree containing the actual Tomb content")
    parser.add_argument("--output", type=Path, help="new or existing local evidence directory")
    args = parser.parse_args()
    output = args.output.resolve() if args.output else Path(tempfile.mkdtemp(prefix="rd508-"))
    output.mkdir(parents=True, exist_ok=True)
    source = Path(__file__).resolve().parent
    toolkit = args.toolkit.resolve()
    api = args.api.resolve()
    encounter = toolkit / "rulebooks/dnd5e/encounter"
    env = os.environ.copy()
    env.update(GOWORK="off", RD508_TOMB=str(api / "content/reference-tomb-heirloom.yaml"),
               RD508_DATA=str(output / "tomb-data.json"))
    for repo, path in (("toolkit", toolkit), ("api", api)):
        pin = subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()
        print(f"{repo}: {pin}")
    jobs = (
        ("generate", "./dungeonspec", {
            str(encounter / "dungeonspec/rd508_generate_test.go"): str(source / "generate_test.go"),
        }),
        ("spatial", ".", {
            str(encounter / "rd508_spatial_test.go"): str(source / "spatial_test.go"),
            str(encounter / "rd508_walk_test.go"): str(source / "walk_test.go"),
            str(encounter / "rd508_knowledge_test.go"): str(source / "knowledge_test.go"),
        }),
    )
    for name, package, replacements in jobs:
        for target in replacements:
            if Path(target).exists():
                raise RuntimeError(f"refusing to overlay an existing provider file: {target}")
        overlay = output / f"{name}-overlay.json"
        overlay.write_text(json.dumps({"Replace": replacements}, indent=2) + "\n")
        command = ["go", "test", "-mod=readonly", f"-overlay={overlay}", "-race", "-count=1", "-v", package, "-run", "^TestRD508"]
        result = subprocess.run(command, cwd=encounter, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (output / f"{name}.log").write_text(result.stdout)
        print(result.stdout, end="")
        result.check_returncode()
    print(f"Local evidence: {output}")


if __name__ == "__main__":
    main()
