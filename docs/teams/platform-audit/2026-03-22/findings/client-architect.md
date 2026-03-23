# Client Architect Audit — rpg-dnd5e-web

**Date**: 2026-03-22
**Domain**: rpg-dnd5e-web (React game client / Discord Activity)
**Auditor**: Client Architect specialist

---

## Summary

The client architect report confirmed the web client is largely **boundary-compliant** — it renders proto data and sends intent, without interpreting game rules. The key issues are downstream consequences of API/toolkit gaps.

## Key Findings

### Proto Consumption
- Client consumes proto-generated TypeScript types
- Falls back to hardcoded ability list when API doesn't provide availability data
- This is the API's fault, not the client's — web is doing the best it can with incomplete data

### Boundary Compliance — OK
- `CombatAbilitiesPanel.tsx` renders buttons based on proto `AvailableAbility` list
- No game logic (checks, validations, rule interpretations) found in client
- Client is correctly render-only

### Component Health
- Class audit found 1,342 lines of untested combat/feature React code
- State management needs assessment (mix of Redux, context, local state patterns)

### Data Gaps
- Client WANTS to show class-specific abilities but API sends same list for all classes
- No death save UI possible (no proto support)
- No rest UI possible (no proto support)
- Equipment stats not flowing during character creation

### Discord Activity Constraints
- Runs as iframe within Discord
- Authentication flows through Discord SDK
- Disconnect/reconnect handling needs improvement

### Tech Debt
- TODO comments and workarounds present
- Hardcoded fallback lists (compensating for API gaps)
- Some complex components that could be split

---

## Cross-Cutting Notes

The client architect's findings reinforce the synthesis: the web client's issues are almost entirely caused by upstream gaps (API not exposing TurnManager, protos missing death save/rest messages). Fix the API boundary violations and proto contracts, and most client issues resolve automatically.
