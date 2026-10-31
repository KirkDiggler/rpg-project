#!/usr/bin/env python3
"""Check the recorded walk, not a substitute for running the browser journey."""
import gzip
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
THOUGHTFUL = "dnd5e:activities:thoughtful"
RELAXED = "dnd5e:activities:relaxed"
WALK = "dnd5e:activities:walk"


def rows(name):
    with gzip.open(ROOT / name, "rt") as stream:
        samples = json.load(stream)
    return [dict(entity, at=sample["at"]) for sample in samples
            for entity in sample["entities"] if entity["id"] == "walker-b"]


def counted(data, count):
    selected = [row for row in data if row.get("cueId")
                and row.get("activity") == THOUGHTFUL and row.get("phase") == "sustain"
                and any(action["clip"] == "Idle_Thoughtful"
                        and action["repetitions"] == count
                        for action in row.get("actions", []))]
    assert selected, f"missing native {count}-cycle sustain"
    return selected


def full_three(data, final=None):
    if final:
        data = data + [dict(entity, at=final["at"]) for entity in final["entities"]
                       if entity["id"] == "walker-b"]
    selected = counted(data, 3)
    assert all("/03-b.glb" in row["url"] for row in selected)
    loops = {action["loop"] for row in selected for action in row["actions"]
             if action["clip"] == "Idle_Thoughtful" and action["repetitions"] == 3}
    # Three 0.181.2 keeps _loopCount=-1 until the first Repeat wrap;
    # it persists 1 and 2 at the next two boundaries, then finishes at three.
    assert {-1, 1, 2} <= loops, loops
    next_cue = min((row["at"] for row in data if row["at"] > selected[-1]["at"]
                    and row.get("cueId") and row["cueId"] != selected[0]["cueId"]), default=float("inf"))
    assert any(selected[-1]["at"] < row["at"] < next_cue and row.get("activity") == RELAXED
               and row.get("phase") == "sustain" and not row.get("cueId") for row in data)


full_three(rows("studio-samples.json.gz"), json.loads((ROOT / "studio.json").read_text())["returned"])
for name in ("fighter", "barbarian"):
    data = rows(f"{name}.json.gz")
    full_three(data)
    long = counted(data, 20)
    after = [row for row in data if row["at"] >= long[0]["at"]]
    assert any(row.get("phase") == "exit" and row.get("requested") == WALK
               and not row.get("cueId") for row in after)
    walking = next(index for index, row in enumerate(after) if row.get("activity") == WALK)
    assert not any(row.get("activity") == THOUGHTFUL for row in after[walking:])
    assert after[-1].get("activity") == RELAXED and not after[-1].get("cueId")
    loops = [action["loop"] for row in long for action in row["actions"]
             if action["clip"] == "Idle_Thoughtful" and action["repetitions"] == 20]
    assert max(loops) < 19, "the long cue was not interrupted early"
    for before, now in zip(after, after[1:]):
        moved = ((now["x"] - before["x"]) ** 2 + (now["z"] - before["z"]) ** 2) ** .5
        if moved > .001:
            assert WALK in (before.get("activity"), now.get("activity"))
    print(f"{name}: three cycles, cancelled long cue, walking-only translation, no revival")

for name in ("catchup.json.gz", "fresh-launch.json.gz"):
    data = rows(name)
    assert data and any(row.get("phase") == "sustain" for row in data)
    assert not any(row.get("cueId") or row.get("activity") == THOUGHTFUL for row in data)
    if name == "fresh-launch.json.gz":
        assert all(row.get("seq") is None and not row.get("held") for row in data)

for name in ("studio.json", "two-client.json", "lifecycle.json"):
    report = json.loads((ROOT / name).read_text())
    assert report["errors"] == [], (name, report["errors"])
print("Recorded evidence is consistent; this does not claim a new live QA run.")
