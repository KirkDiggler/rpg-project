# Generated SDK Release Implementation

**Issue:** `rpg-api-protos#261`  
**Parent discovery:** `rpg-project#340`  
**Status:** Implementation correction after internal review

## Why this implementation document exists

The initial #261 plan was to add `gen/go/vX.Y.Z` beside the existing root tag. Implementation review proved that this is not enough:

1. The `generated` branch is force-pushed before local module/tag validation and before tag publication, so a failure can advance generated output without publishing a release.
2. Concurrent or retried main-branch workflows can allocate the same or an unnecessary next version.
3. GitHub release and npm steps are guarded by `startsWith(github.ref, 'refs/tags/v')`, but the workflow only runs for pushes to `main`; those steps are unreachable.
4. Internal review proved the dormant npm step must not simply be enabled: the packed artifact declares nonexistent JS/declaration entrypoints and its documented import paths do not match the packed layout. npm packaging is now separately shaped by rpg-api-protos#263.
5. Native GitHub Actions concurrency is not FIFO, so serialization alone cannot prevent an older delayed main workflow from releasing after a newer source and rewinding `generated`.

These findings meaningfully change release implementation, though they do not change the quantity contract or player-facing Slice 1 behavior.

## Corrected release transaction

One `publish-packages` job owns one serialized release transaction for each main commit.

### 1. Serialize release jobs

Add job-level GitHub Actions concurrency:

```yaml
concurrency:
  group: rpg-api-protos-generated-release
  cancel-in-progress: false
```

PR validation jobs remain parallel; only the main-branch publication job is serialized. Because GitHub does not guarantee FIFO ordering, a job must also fetch and verify immediately before planning/publication that its `SOURCE_SHA` is still `origin/main`. A superseded source exits successfully as coalesced work without publishing. The release contract is latest-main, not one release per intermediate commit.

### 2. Generate and validate locally before remote mutation

The job still generates Go/TypeScript and mocks and constructs a local `generated` commit. Before any push it must:

- confirm `gen/go/go.mod` declares `github.com/KirkDiggler/rpg-api-protos/gen/go`;
- derive the module prefix `gen/go` from that path rather than hardcoding a second answer;
- select only strict final root tags matching `^v[0-9]+\.[0-9]+\.[0-9]+$` for the release clock;
- compute root `vX.Y.Z` and module `gen/go/vX.Y.Z` names;
- create both as annotated local tags on the exact generated commit;
- verify both peel to that commit.

A validation failure performs no remote write.

### 3. Make reruns idempotent

Before allocating a new version, inspect every strict root release tag whose generated commit records the triggering source SHA.

If any complete root + `gen/go/<root-tag>` pair exists for that source and both peel to the same generated commit, reuse that release identity even when newer releases exist. A reuse run performs no branch/tag push; it retries only missing external GitHub release publication.

If any source-associated release identity is partial or inconsistent, fail closed. Do not silently allocate another version or rewrite a tag.

Record the source SHA in the generated commit message or another deterministic commit relationship that the rerun check can verify without external mutable state. Add an A-release → B-release → retry-A test proving A is reused without rewinding `generated`.

### 4. Publish branch and tags atomically

After validation, perform one atomic push containing:

```text
+<generated-commit>:refs/heads/generated
refs/tags/vX.Y.Z
refs/tags/gen/go/vX.Y.Z
```

GitHub supports atomic pushes. The explicit `+` applies only to the generated branch; tag refs are create-only and never force-updated.

If any ref is rejected, none of the three remote refs advances.

### 5. Make the GitHub release reachable and keep npm fail-closed

The job is running under `refs/heads/main`, so GitHub release publication must not depend on a tag-triggered `github.ref`.

- Create or update the GitHub release with explicit `tag_name: ${{ env.NEW_TAG }}`.
- The module-qualified Go tag does not create a second GitHub release.
- Same-source reuse may retry creation/update of the root GitHub release without pushing refs again.
- Remove or explicitly disable the unreachable npm publish step and correct docs to say npm is not currently a supported publication channel.
- rpg-api-protos#263 owns npm package layout, exports, compile output, versioning, clean-install import proof, and idempotent registry publication. No npm publish becomes reachable before that contract lands.

## Code shape

Keep mutation orchestration in the workflow, but move deterministic tag/version/source-SHA planning into a small repository script if doing so is necessary to test it without copying shell text into a test. The script must not push, publish, or rewrite refs.

A project-specific test may initialize a temporary repository and bare remote to prove:

- `gen/go/v9.9.9` does not advance the root version clock;
- non-final root tags do not advance it;
- both new tags are annotated and peel to the generated commit;
- rerunning for the same source SHA reuses the existing pair, including after a newer source release, without pushing refs;
- a superseded source SHA is coalesced before release planning and cannot rewind `generated`;
- a partial pair fails closed;
- atomic publication leaves all refs unchanged when one ref is rejected;
- historical refs are unchanged.

Do not add tests that merely grep workflow text.

## Documentation corrections

Update the existing architecture/status/how-to documents to state:

- historical releases through `v0.1.147` require exact generated-commit pseudo-versions for Go;
- releases after #261 expose `gen/go/vX.Y.Z`;
- one root tag remains the GitHub release identity;
- GitHub release publication runs from the main-branch transaction, not a nonexistent tag-triggered workflow;
- npm publication is unsupported until rpg-api-protos#263 proves a usable package contract.

## Verification

Before PR readiness:

1. Parse workflow YAML.
2. Run `bash -n` on all shell blocks after replacing GitHub expressions with safe placeholders.
3. Run the temporary-repository release transaction test.
4. Run `git diff --check` and source-scope verification.
5. Public GLM inline review and implementer thread responses.

After human merge, the main workflow itself is the final acceptance:

- `generated` advances;
- root and `gen/go/` tags both exist and peel to the same generated commit;
- `GOPROXY=direct go list -m github.com/KirkDiggler/rpg-api-protos/gen/go@vX.Y.Z` resolves;
- GitHub release exists for root tag;
- no npm publication step ran.

`rpg-project#340` remains open until this post-merge release evidence is recorded and the separately attached npm design issue rpg-api-protos#263 is dispositioned.
