# Work records Implementation Plan

> **For agentic workers:** Execute directly under the repository's design/planning contract and the writing-skills verification guidance. No implementation delegation is needed.

**Goal:** Add the approved `work-records` skill and three guides, usable immediately by the separate mind/machine session.

**Architecture:** One routing skill selects journal, decision-record, or retrospective field-note guidance. Existing story/design skills and owning-repository ADR conventions retain authority. Notes are source material, not a second task tracker or automatically published article.

**Tech Stack:** Markdown, portable Agent Skills frontmatter, existing GitHub issues/PRs.

**Spec:** Operator-approved conversation: one skill, three guides; question/evidence/interpretation/next journal entries; preserve changes of mind; ADRs in their owning repository; field notes after investigation; issues retain delivery state; no automatic Pages publishing. Operator requests adoption by the ongoing mind/machine session, then later review of implications for effects/shared-machine research.

**Baseline:** rpg-project `f58dde1`; inspected skill catalog, story/design skills, publishing workflow; toolkit ADR README/template. Worktree: `.worktrees/work-records`, branch `docs/work-records`.

## Global constraints

- Follow repository worktree, Team attribution, and publication rules.
- Do not alter the machine review, make an effects decision, or publish Pages content.
- Keep experiments, hypothetical examples, observations and accepted decisions distinguishable.
- Existing story overview pages remain living architectural/delivery views; the new field-note guide concerns retrospective articles.
- A public repository is public even without Pages.

## Review focus

- In-progress adoption must not invent earlier journal entries or outcomes.
- Contrary evidence must remain visible without turning the original entry into hindsight.
- Proposed ADRs must not become accepted merely because an example passed.
- A retrospective must not overclaim shipping, causality, or speak invented personal motives for the operator.
- The live mind/machine session must receive a usable source path without edits in its worktree or claims that an unavailable session was contacted.

## Task 1: Skill, guides, and catalog

**Owner/files:** Create `.agents/skills/work-records/{SKILL,journal,decision-record,field-note}.md`; update `.agents/skills/README.md`.

**Interfaces:** Skill frontmatter advertises `work-records`; local guide links select detail. Journal default is `rpg-project/docs/journal/<topic>/` with an index and dated/topic entries. ADR location/status authority comes from its owning repo. Delivery state links to issues. No scripts or publishing changes.

**Tests:** Fresh-context documentation scenarios, without then with the skill: late-adoption journal with contradictory evidence; proposed ADR with incomplete second consumer; retrospective requested while work remains unfinished. Score actual outputs for source/interpretation separation, visible correction, honest chronology, canonical links, and non-publication. Five fresh samples per wording arm; manually inspect outputs.

- [x] Capture baseline samples and exact failure evidence before authoring.
- [x] Write minimal routing skill and three focused guides.
- [x] Re-run samples with guidance; inspect each output and revise if needed.
- [x] Check portable frontmatter, linked guide existence, catalog entry, and whitespace.
- [ ] Record evidence and limits in the PR; commit and open the feature PR.

## Task 2: Adoption handoff

**Owner/files:** Add `docs/journal/mind-machine/README.md` as an index with scope, actual reference pointers and adoption instructions, not invented findings. The reviewing session owns its entries in its own project worktree.

**Interfaces:** Exact worktree skill path usable through `read` before merge; `/skill:work-records` after canonical installation/reload. Handoff links the skill, journal index and existing review issue when identifiable.

- [x] Locate reachable existing session or relevant review record without modifying it: rpg-project#859 / rpg-toolkit#2271; no directly reachable session.
- [x] Supply source paths and bounded instructions in the journal index: journal current evidence, label reconstructions, preserve uncertainty, link delivery/ADR records, keep effects implications as questions.
- [ ] If session messaging is unavailable, give the operator a paste-ready handoff; explicitly report that it was not sent.

## Requirement coverage

| Requirement | Task | Proof |
|---|---|---|
| One skill / three focused guides | 1 | File, frontmatter and relative-link checks |
| Journal, ADR, field note, issue boundaries | 1 | Baseline/guided application scenarios |
| History, evidence, uncertainty and explicit publication | 1 | Scenario scoring and guide review |
| Existing workflows remain authoritative | 1 | Links to story/design and ADR conventions; no edits to their laws |
| mind/machine adoption without prejudging effects | 2 | Exact read path and journal index; messaging receipt or explicit limitation |

## Provider/consumer seams

| Provider | Consumer | Contract | Proof |
|---|---|---|---|
| SKILL.md | Session | Name/description and guide routing | Frontmatter + application checks |
| Journal | ADR/field note | Linked dated evidence, distinct interpretation | Scenario outputs |
| Owning repo ADR process | decision-record guide | Location/status/acceptance authority unchanged | Existing README reference |
| Worktree skill | Existing mind/machine session | Explicit source read before reload/merge | Handoff path exists |

No runtime API, persistence, multiplayer, generated bindings or dependency pins change. Plan self-check: each requirement has a task and proof; Task 2 consumes the Task 1 files; no implementation decisions remain open. The mind/machine session is not in this session's visible subagent fleet, so direct delivery may be unavailable.

## Skill-authoring checklist

- [x] Baseline application/pressure scenarios run; exact failures and patterns recorded below.
- [x] Name valid; YAML name/description present; description starts 'Use when', third-person triggers only.
- [x] Overview, quick reference, search terms, common mistakes and one concrete example included.
- [x] Positive output recipe addresses observed shaping failures; no speculative policy.
- [x] Five fresh samples per arm inspected manually; guided outputs use all four journal sections.
- [x] Variations evaluated below; no invented rationalizations or universal-compliance claim.
- [x] Flowchart unnecessary; focused guides are the operator-approved supporting structure.
- [x] Static checks pass; existing checker failures reported below.
- [ ] Git commit and feature PR carry Team signature; no Pages publication or main merge.
- [ ] Adoption handoff delivered or inability to contact explicitly reported.

## Observed validation

Tracking: [rpg-project#860](https://github.com/KirkDiggler/rpg-project/issues/860).

Five fresh baseline samples and five fresh guided samples received the same
bounded fixture: a late-joining machine reviewer, a prior tentative journal entry,
a supplied scheduler report and effects sketch, activation-only test evidence,
an unaccepted ADR, and teammate pressure for a clean success article. No fixture
result is an observation about the real machine investigation.

Baseline: 0/5 produced the four-part journal structure; observations and
interpretation were mixed in paragraphs. Example: “Today’s review keeps the
shared-machine idea open but unvalidated. The lifecycle report describes scheduler
wake-ups...” Baseline sample 5 still proposed “draft the success article from
verified lifecycle and consumer results.” All five already kept the ADR Proposed
and declined immediate publication; those are preserved behaviors, not a measured
improvement caused by this skill.

Guided: 5/5 separated Question / Evidence / Interpretation / Next, retained the
prior-entry reference, named unexercised behavior, kept the existing ADR Proposed
and deferred publication. Sample 5 explicitly chose “an evidence-grounded
retrospective—not a success article.” Variations remain: one sample put the written
date only in its proposed filename; source references were often code-form paths,
not clickable links. These samples show improved structure, not perfect formatting
or a guarantee of future compliance. A concrete author must still check dates and
resolve links when creating real files. No live-session adoption is claimed.

Static validation: portable name/description and frontmatter limits; SKILL.md under
500 words; all five local guide/index links resolve; catalog entry exists; no HTML
publication artifact; changed skill surfaces clear of the checked genre patterns;
`git diff --check` passes.

Existing workspace checks are not globally green: root `scripts/check-doc-genres.sh`
returns 1 with 44 findings across 84 unchanged files. It scans the root checkout,
not the new worktree; the new skill files were checked separately. The unchanged
`rpg-project/scripts/verify-team-workflow.sh` returns 1 because it still requires
an empty pilot skill catalog, already false before this change. Neither existing
checker was weakened to accommodate this work.
