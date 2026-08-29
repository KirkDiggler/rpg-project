# Retro — character facing + crypt monster roster (2026-07-24)

First retro as an artifact. Kirk: *"learning the process is what our focus should
be to start. and write down for future us."* This records **how we worked**, not
what shipped. Findings are ordered by how much they'll cost us if forgotten.

Work in scope: `rpg-dnd5e-web#590/#592` (character facing), `#593` (skating, filed),
`#559` (crypt monsters — two lanes in parallel, plus an independent gate).

---

## 1. Match model tier to ambiguity, not to importance

Kirk's framing: *"using an opus to implement after the plan is made is likely
overkill ... noodle throwing and finding elegant solutions the higher models can
really provide value."*

Today's evidence supports it, on both sides:

- **Execution was excellent at the cheap tier.** Two Sonnet agents independently
  produced a dual-signal resolver keyed off real toolkit ref ids, a 7-model
  promotion with downed variants, and a working idle retarget. None of it needed a
  bigger model — the specs were clear.
- **The expensive tier earned its keep at exactly three moments**, all judgment
  under ambiguity: deciding the facing offset split had to be *measured* rather than
  inferred (a naive split cancels to 2π); ruling that ref-id filenames lose because
  **there is no toolkit ref for "ghost"**; and rejecting a live-monster screenshot as
  a T-pose that the agent that captured it had read as a pass.

**The rule: tier by ambiguity.** Specified backlog execution → cheap tier. Open
solution space, cross-lane decisions, and *verification* → expensive tier.

**The essential caveat: the cheap tier is bad at knowing when it's out of its
depth.** `web-monsters` did strong work, ran its own Boundary Rule self-check, was
honest that `/code-review` wasn't available to it — and then misread its own
evidence. That is not a reliability failure to punish; it is why **cheap execution
plus an expensive gate** is the pairing, and why the gate cannot be the same agent
that did the work.

---

## 2. Read evidence for what it shows, not for what you hoped it would prove

The sharpest lesson of the day. `web-monsters` captured three screenshots with good
discipline — real `HexEntity`, real promoted assets, an unmapped no-regression
control, harness deleted, clean `git status`. Then it reported the live-monster shot
as *"confirms the composition isn't broken."*

That was true. The same image also showed the skeleton **T-posed** — arms
horizontal, bind pose, no clip playing — which goes unmentioned. Shipping it would
have been a regression: a T-posing skeleton is worse than the blue placeholder it
replaced.

The tell was *inside its own evidence set*: the **downed** skeleton looked right
(authored static pose), the **live** one didn't. An asymmetry across your own shots
is a question, not a coincidence.

**Practice:** when you capture evidence to prove X, write down everything the frame
shows before concluding anything about X. Reviewers should look at the artifact, not
the caption.

---

## 3. CI green is necessary, never sufficient — and sometimes CI doesn't exist

The T-pose survived: CI green, 780+ tests, a correct Boundary Rule self-check, and an
honest self-review. Every gate passed and the output was still unshippable.

Worse: **rpg-game-assets has no CI at all.** In that repo the evidence bar is not
*a* gate, it is *the* gate. Weight verification there accordingly.

---

## 4. Decisions are owned, not negotiated between peers

`web-monsters` blocked itself waiting for `assets-undead` to agree a filename
convention. Neither could grant it — it was a cross-lane decision, which means it was
the director's, and it sat unmade while both waited.

**Practice:** a peer-to-peer disagreement about a shared contract escalates
immediately. Agents should not negotiate contracts with each other. Corollary seen
today: messages cross, so state decisions as decided and say "don't wait."

---

## 5. Two lanes independently measuring the same thing is cheap and worth it

`assets-undead` and `web-monsters` each determined the POLYGON Dungeon rigs' forward
axis, by different methods (turntable with known controls vs. rendering through the
real `ClassCharacterModel`), and both got **+Z**. That corroboration cost almost
nothing and would have caught a wrong answer loudly.

Reserve it for facts that are **cheap to measure and expensive to get wrong.**

---

## 6. "Promoted" implied "usable." It didn't mean it.

A model can pass promotion — clean re-export, root-node convention verified, mesh
stats recorded — and still be unshippable because it can't hold a pose. The manifest
had no field for that, so nothing caught it.

**Fix applied:** pose/animation state is now recorded explicitly alongside mesh stats.
**General form:** when a artifact passes every recorded check and is still unusable,
the missing check is the deliverable — add the field, don't just fix the instance.

---

## 7. Environment lies quietly

A git worktree has **no `node_modules` of its own** and silently resolves the parent
checkout's, which was out of sync with the lockfile — prettier 3.9.6 locally vs 3.8.3
pinned. Result: `npm run format` invented churn in 13 unrelated files *and* missed two
files CI would flag, a red CI run, and a **confidently wrong diagnosis** ("main has
prettier drift" — it does not).

**Practice:** `npm ci` in a fresh worktree before trusting any local gate. Same family
as the stale-node_modules-protos trap that already bit us.

**Meta-lesson, and the one worth keeping:** the wrong diagnosis was produced by the
expensive model reasoning carefully from bad inputs. Tier does not protect you from
environment drift; it just makes the wrong answer more articulate.

---

## 8. Kirk is a resource, not just an approver

Principal engineer, 20+ years, primarily Go. He has Blender MCP tools and can look at
things directly. Twice today he corrected a conclusion from experience — *"we have a
heck of a time placing the weapons ... def verify"* was right, and the assets repo's
own `attach_weapon.py` docstring confirmed it while my bounding-box inference had
said otherwise.

**Practice:** ask him. Especially on taste calls (does this read right?), Go
architecture, and anything where he can just look. Cheaper and more accurate than
inferring. Treat *"I think / I suspect"* as a hypothesis to check, not a settled
decision — and treat a flat correction as data, not as a preference.

---

## Carried forward

- Open a follow-up whenever a lane defers something (ghost downed variants read as
  "collapsed" rather than "incorporeal fade" — cosmetic, tracked in the manifest).
- `sessions/active.md` was 4 days stale and platform-lane-only; asset-lane state lived
  only in chat. A handoff that covers one lane isn't a handoff.
- Board hygiene: `#592`/`#593` shipped before being boarded. DoD now requires boarded
  + visible on board **and** PR.
