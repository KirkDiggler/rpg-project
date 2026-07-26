import assert from "node:assert/strict";
import test from "node:test";

import { CheckpointValidationError, renderCheckpoint } from "../src/checkpoint";

const validCheckpoint = {
  role: "PIH-2 implementation worker",
  completedWork: "Implemented the fixture-only dispatch contract.",
  verification: "npm test (fixture-only dispatch coverage)",
  blockers: "none",
  nextAction: "Kirk reviews the ready PR.",
  nextOwner: "Kirk",
  branch: "feat/141-pi-github-dispatch-contract",
  prUrl: "https://github.com/KirkDiggler/rpg-project/pull/142",
};

test("checkpoint template includes each durable handoff field and a role signature", () => {
  const body = renderCheckpoint(validCheckpoint);

  for (const expected of [
    "Completed work",
    "Verification",
    "Blockers",
    "Next action / owner",
    "Branch",
    "PR",
    "— PIH-2 implementation worker, on behalf of KirkDiggler",
  ]) {
    assert.match(body, new RegExp(expected.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")));
  }
  assert.doesNotMatch(body, /MERGE-READY/i);
});

test("checkpoint template refuses an incomplete or self-approving handoff", () => {
  assert.throws(
    () => renderCheckpoint({ ...validCheckpoint, verification: "" }),
    CheckpointValidationError,
  );
  assert.throws(
    () => renderCheckpoint({ ...validCheckpoint, nextAction: "Declare MERGE-READY" }),
    /MERGE-READY is reserved/,
  );
});
