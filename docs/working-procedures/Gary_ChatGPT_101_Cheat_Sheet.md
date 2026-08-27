# 🎓 BILLY + GARY — CHATGPT 101 CHEAT SHEET

> A compact reference for working with Gary during real project work.

## Core Mental Model

**Gary is a collaborator, not an oracle.**

```text
BILLY + GARY
     │
     ▼
  EVIDENCE
     │
     ▼
  REASONING
     │
     ▼
  DECISION
     │
     ▼
   ACTION
     │
     ▼
   VERIFY
```

Neither person's assumption automatically wins. **The evidence does.**

---

## 1 — Understand What AI Is

Gary is an AI model generating responses from context, learned patterns, available tools, and reasoning.

**Remember:** Confident language ≠ guaranteed truth.

> Ask: **“How do we know?”**

---

## 2 — Prompting & Context

Give Gary the **goal**, relevant context, evidence, and important constraints.

You don't need a magical prompt.

**Good pattern:**

> “Here's what I'm trying to accomplish, here's what I know, here's what I'm unsure about, and here's what I want you to do.”

---

## 3 — Context vs. Memory vs. Project Knowledge

```text
RIGHT NOW
→ Conversation context

ABOUT ME / HOW I WORK
→ Memory

HOW GARY SHOULD WORK IN THIS PROJECT
→ Project instructions

HOW THE SOFTWARE ACTUALLY WORKS
→ Code / specifications / project artifacts
```

Don't make memory carry information that belongs in the project itself.

---

## 4 — Authoritative Sources / Source of Truth

**Authoritative = the source we trust when information conflicts.**

```text
Software behavior → Current code/tests
API contract      → Current specification
Library behavior  → Version-specific official docs
Project history   → Git history
```

Always ask:

> **“Which source wins if these disagree?”**

---

## 5 — Iterative Prompting

You don't need to solve everything in one giant prompt.

```text
Ask
 ↓
Learn
 ↓
Refine understanding
 ↓
Ask better question
 ↓
Repeat
```

Conversation itself is part of the reasoning process.

---

## 6 — Debugging: Trace, Don't Guess

Start with:

```text
EXPECTED
   │
ACTUAL
   │
EVIDENCE
   │
TRACE VALUE / REQUEST
   │
FIRST DIVERGENCE
   │
CAUSE
```

A suspicion is a **hypothesis**, not evidence.

**Useful prompt:**

> “I suspect X, but that's unverified. Trace the relevant path and identify the first divergence. Don't modify anything yet.”

---

## 7 — Research

Research belongs inside the workflow when **external or current information is needed to make the project decision**.

```text
Project question
      ↓
Knowledge gap
      ↓
Research authoritative source
      ↓
Bring evidence back
      ↓
Apply it to project
      ↓
Decision
```

Research isn't the goal. **Resolving the project uncertainty is.**

---

## 8 — Trace & Compare

When one thing works and another doesn't:

```text
WORKING PATH ─────┐
                  ├──► FIRST DIFFERENCE
FAILING PATH ─────┘
```

Compare equivalent paths rather than staring only at the broken result.

---

## 9 — Long-Running Projects

Current conversation context is temporary.

Important project truth should live somewhere durable and appropriate.

Keep:

- code current,
- specs current,
- implementation plans current,
- comments useful,
- completed TODOs removed.

---

## 10 — Persistent Rules & Memory

Don't create giant permanent instruction sets.

Save things that are **durable and repeatedly useful**. Temporary project facts should remain temporary.

Remember:

> **Persistent information can become stale too.**

---

## 11 — Tools

Gary can sometimes do more than answer questions. Depending on available tools, tasks may involve:

```text
Search
Read files
Inspect code
Run tests
Use connected services
Create artifacts
Automate future work
```

But:

> **Never assume an action happened merely because Gary described it.**

Look for actual tool or action evidence.

---

## 12 — Match Reasoning Effort to Risk

### Light

Simple explanation, terminology, or an obvious low-risk question.

### Medium

Bug investigation, comparison, or an implementation decision with contained impact.

### Heavy

Architecture, migrations, broad project changes, or consequential decisions.

> Don't turn every question into research.  
> Don't turn major decisions into guesses.

---

## 13 — Coding Workflow

```text
UNDERSTAND
    ↓
INVESTIGATE
    ↓
RESEARCH (if needed)
    ↓
PLAN
    ↓
🛑 DECIDE
    ↓
IMPLEMENT
    ↓
TEST
    ↓
VERIFY
    ↓
UPDATE SOURCE OF TRUTH
```

Not every task needs every step.

---

## 14 — Failure Modes & Trust

### Anchoring

> “I think it's the database.”

Don't accidentally turn that into:

> “It's the database.”

### Unsupported Conclusions

> “Fixed.”

Ask:

> “What evidence demonstrates that?”

### Stale Information

Version, code, documentation, and project state matter.

### Agreement Bias

Gary agreeing with you does not make you correct.

---

## 15 — Agents

Give an agent:

```text
GOAL
CONTEXT
AVAILABLE EVIDENCE / TOOLS
AUTHORITY
BOUNDARIES
STOP CONDITION
ESCALATION CONDITION
SUCCESS CRITERIA
```

But don't prescribe every harmless intermediate step.

### Key Principle

> **Be precise about the destination without unnecessarily dictating every turn along the road.**

Useful context ≠ micromanagement.

---

## 16 — Monitoring & Automation

### Automation

> “Do this on a schedule.”

### Monitoring

> “Keep checking until this condition matters.”

### Agentic Monitoring

> “Check, evaluate what you find, and involve me only when appropriate.”

Most important rule:

```text
DETECT
   ↓
EVALUATE
   ↓
DECIDE
   ↓
ACT
```

These **do not need the same authority level**.

For consequential project work:

```text
Detect      → automated
Evaluate    → Gary/agent
Recommend   → Gary/agent
DECIDE      → BILLY
Act         → approved
Verify      → evidence
```

---

## 17 — The Gary Playbook

Your compact operating procedure:

```text
1. Define the goal.

2. Give real evidence when available.

3. Admit what you don't know.

4. Separate observations from suspicions.

5. Establish the authoritative source.

6. Investigate before changing when warranted.

7. Research external unknowns when needed.

8. Match effort to consequence.

9. Let Gary choose low-risk intermediate steps.

10. Keep consequential decisions controlled.

11. Stop if new evidence changes the plan.

12. Verify before declaring success.

13. Update affected sources of truth.

14. Review scope before shipping.

15. PR / handoff when collaborating.
```

---

# 🐛 The Debugging Card

Use this during D&D project debugging:

```text
╔════════════════════════════════════╗
║           BUG DISCOVERED           ║
╠════════════════════════════════════╣
║                                    ║
║  What did I EXPECT?                ║
║                                    ║
║  What ACTUALLY happened?           ║
║                                    ║
║  What EVIDENCE do I have?          ║
║                                    ║
║  What do I merely SUSPECT?         ║
║                                    ║
║              ↓                     ║
║                                    ║
║  TRACE THE ACTUAL PATH             ║
║              ↓                     ║
║                                    ║
║  FIND FIRST DIVERGENCE             ║
║              ↓                     ║
║                                    ║
║  EXPLAIN CAUSE WITH EVIDENCE       ║
║              ↓                     ║
║                                    ║
║  COMPARE SOLUTIONS                 ║
║              ↓                     ║
║                                    ║
║       🛑 DECISION POINT            ║
║              ↓                     ║
║                                    ║
║  IMPLEMENT → TEST → VERIFY         ║
║              ↓                     ║
║                                    ║
║  UPDATE DOCS / SOURCE OF TRUTH     ║
║              ↓                     ║
║                                    ║
║  REVIEW DIFF → PR                  ║
║                                    ║
╚════════════════════════════════════╝
```

---

# 🤖 The Agent Card

Before giving an agent significant independence:

```text
╔════════════════════════════════════╗
║           AGENT CONTRACT           ║
╠════════════════════════════════════╣
║                                    ║
║ GOAL                               ║
║ What should it accomplish?         ║
║                                    ║
║ CONTEXT                            ║
║ What useful information do I know? ║
║                                    ║
║ TOOLS                              ║
║ What can it inspect/use?           ║
║                                    ║
║ AUTHORITY                          ║
║ What may it change?                ║
║                                    ║
║ BOUNDARIES                         ║
║ What may it NOT change?            ║
║                                    ║
║ STOP                               ║
║ When should it stop?               ║
║                                    ║
║ ESCALATE                           ║
║ When should it return to me?       ║
║                                    ║
║ VERIFY                             ║
║ What demonstrates success?         ║
║                                    ║
╚════════════════════════════════════╝
```

---

# 🛞 BUMPER

When Gary says:

# **BUMPER**

it means:

> **We're potentially drifting outside one of our learned procedures.**

It does **not** automatically mean:

> “Billy is wrong.”

It means:

```text
STOP FOR A SECOND
       ↓
Something procedural may
have been overlooked
       ↓
BILLY examines it first
       ↓
BILLY chooses the next move
       ↓
Gary continues
```

Common BUMPER triggers:

```text
Suspicion treated as fact
Investigation skipped
Authority/source unclear
Current/version-specific info missing
Scope silently expanding
Decision checkpoint skipped
Agent authority too broad
Stopping condition missing
Success claimed without evidence
Documentation/source-of-truth forgotten
```

**BUMPER is the guardrail, not the answer.**

---

# ⚽ The Wingmen Rule

```text
                  ⚽
               THE GOAL
                  ▲
                  │
           ┌──────┴──────┐
           │             │
         BILLY          GARY
           │             │
        Intent        Reasoning
       Judgment      Investigation
      Project info      Tools
       Decisions     Explanation
           │             │
           └──────┬──────┘
                  │
               EVIDENCE
```

**Billy doesn't work for Gary.**

**Gary doesn't work instead of Billy.**

We're teammates working toward the same goal.

If you know something I don't—**tell me.**

If I see evidence against your hypothesis—**I should tell you.**

If you don't understand—**say so.**

If I don't have enough evidence—**I should say so.**

If either of us is wrong—**correct course.**

### The goal isn't to win the argument.

# **Put the ball in the net. ⚽**

---

*Billy + Gary — wingmen guided by evidence, good judgment, and verified results.*
