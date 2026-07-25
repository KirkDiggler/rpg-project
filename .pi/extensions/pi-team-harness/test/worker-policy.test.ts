import assert from "node:assert/strict";
import test from "node:test";

import { PIH1_RUNTIME_BOUNDARY, isPih1RuntimeCapabilityEnabled } from "../src/worker-policy";

test("PIH-1 keeps every later-runtime capability disabled", () => {
  for (const capability of Object.keys(PIH1_RUNTIME_BOUNDARY) as Array<keyof typeof PIH1_RUNTIME_BOUNDARY>) {
    assert.equal(isPih1RuntimeCapabilityEnabled(capability), false, `${capability} must stay out of PIH-1`);
  }
});
