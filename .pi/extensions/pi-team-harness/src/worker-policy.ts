export const PIH1_RUNTIME_BOUNDARY = {
  workerProcess: false,
  daemon: false,
  externalDispatch: false,
  githubMutation: false,
  tui: false,
  mcp: false,
  automaticApproval: false,
  durableOrchestrationStore: false,
} as const;

export type Pih1RuntimeCapability = keyof typeof PIH1_RUNTIME_BOUNDARY;

export function isPih1RuntimeCapabilityEnabled(capability: Pih1RuntimeCapability): boolean {
  return PIH1_RUNTIME_BOUNDARY[capability];
}
