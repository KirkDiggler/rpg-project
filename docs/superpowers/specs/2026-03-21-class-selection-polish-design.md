# Class Selection Polish — Concepts Prototype

**Date:** 2026-03-21
**Status:** Draft
**Scope:** Build a concepts prototype for an enriched class selection experience

## Problem

The current class selection modal shows mechanical data (hit die, saving throws, proficiencies) but lacks the contextual guidance that helps players make informed decisions. Descriptions are single sentences. Ability score priorities aren't explained. Equipment choices show names without stats or recommendations. A new player choosing Monk has no idea whether to prioritize STR or DEX, or why a shortsword is better than a handaxe for them.

## Goals

1. Prototype a richer class selection UX using hardcoded Monk data
2. Surface contextual guidance at each decision point (not just top-level descriptions)
3. Validate the UX before touching toolkit, protos, or the real modal
4. Establish the pattern for race and background polish later

## Non-Goals

- No toolkit changes
- No proto field additions
- No API changes
- No modifications to the real ClassSelectionModal
- No race or background work yet

## Approach: Top-Down Concepts Page

Build a `/concepts` view in the app with a `ClassSelectionConcept` component. This uses hardcoded data to mock the ideal experience. Once validated, we work backwards to make the toolkit produce this data.

### Why concepts page, not the real modal?

- Iterate freely without disrupting the working creation flow
- Experiment with layout/content without worrying about data plumbing
- The app is pre-alpha — we can tear down and rebuild once we know what works

## Design

### Navigation

The app uses view-based navigation (`AppView` type in `App.tsx`), not a router. Add a `'concepts'` view accessible via a dev-only link (e.g., on the debug panel or a keyboard shortcut). The concepts view has its own sub-navigation for different concept pages.

### Class Selection Concept — Content Sections

#### 1. Class Overview (replaces single-sentence description)

2-3 sentences covering:
- What fantasy this class fulfills
- Core playstyle (melee/ranged, mobile/tanky, etc.)
- What makes it distinct from similar classes

**Monk example:**
> "Masters of martial arts who channel ki — an innate magical energy — through disciplined combat. Monks are fast, mobile strikers who dart in and out of melee using unarmed strikes and monk weapons. Unlike Fighters who rely on heavy armor and martial weapons, Monks use Dexterity and Wisdom to power both their offense and defense."

#### 2. Ability Score Guidance

Shown prominently near class details AND repeated contextually when assigning ability scores.

**Format:** Primary ability with explanation, then secondary, then dump stats.

**Monk example:**
- **DEX (Primary):** Your attack modifier, AC (via Unarmored Defense), and key saving throw
- **WIS (Secondary):** Boosts AC (Unarmored Defense), ki save DC, and Perception
- **CON:** Hit points matter for a melee class with d8 hit die

#### 3. Saving Throws with Context

Not just "Strength, Dexterity" but what that means in play.

**Monk example:**
> "**Strength & Dexterity** — You're hard to knock down, grapple, or hit with traps. DEX saves are one of the most common in the game, protecting against fireballs, dragon breath, and similar area effects."

#### 4. Proficiency Details (Expanded)

Replace "Simple Weapons, Specific Weapons" with actual weapon lists and class-relevant callouts.

**Monk example:**
- **Weapons:** Simple weapons and shortswords
- **Monk weapon note:** "Any simple melee weapon without the Heavy or Special property counts as a monk weapon. You can use DEX instead of STR for attacks, and the damage die scales with your Martial Arts feature (starts at d4, eventually d10)."
- **Armor:** None — "Monks use Unarmored Defense: AC = 10 + DEX + WIS. Wearing armor disables this and several monk features."
- **Tools:** None

#### 5. Equipment Choices with Guidance

When selecting starting equipment, show:
- Weapon stats (damage die, properties like Finesse/Light)
- A contextual tip explaining why one option suits the class

**Monk equipment example:**
| Option | Damage | Properties | Tip |
|--------|--------|------------|-----|
| Shortsword | 1d6 slashing | Finesse, Light | Best monk starting weapon — uses DEX, counts as monk weapon |
| Any simple weapon | Varies | — | All simple melee weapons (non-Heavy) work as monk weapons |

#### 6. Bug Fix: "Primary: 2" (Separate Issue)

The current modal displays the enum integer for `primary_ability` instead of the ability name. This is a real bug in the existing ClassSelectionModal. Per the "one issue per PR" rule, this should be tracked and fixed as a separate issue — not part of this concepts prototype work. Noted here for awareness.

### Layout

The concept page should feel like a polished version of the existing modal, not a completely different paradigm. Keep the class icon carousel at the top, but below it present the enriched content in a clean, scannable layout:

- Class overview at top (the "sell")
- Two-column grid for mechanical details (ability guidance + saving throws, proficiencies + skills)
- Equipment section with table layout for weapon comparisons
- Subclass section at bottom (if applicable)

### Hardcoded Data Structure

Define a TypeScript interface for the enriched class data that represents what we'd eventually want the toolkit to produce:

```typescript
interface EnrichedClassInfo {
  // Existing fields
  name: string;
  hitDie: number;
  primaryAbility: string;
  savingThrows: string[];

  // Enriched fields
  description: string;           // 2-3 sentences
  abilityGuidance: {
    ability: string;
    priority: 'primary' | 'secondary' | 'tertiary';
    explanation: string;
  }[];
  savingThrowContext: string;    // Why these saves matter
  proficiencyDetails: {
    weapons: string[];
    weaponNotes: string;         // Class-specific weapon guidance
    armor: string[];
    armorNotes: string;          // Why armor works this way for this class
    tools: string[];
  };
  equipmentGuidance: {
    itemName: string;
    damage: string;
    properties: string[];
    tip: string;
  }[];
}
```

This interface is speculative — the point is to see what content we need, then decide how to structure it in the toolkit.

## Files to Create/Modify

| File | Action | Purpose |
|------|--------|---------|
| `src/App.tsx` | Modify | Add `'concepts'` to AppView, add navigation |
| `src/concepts/ConceptsView.tsx` | Create | Container with sub-navigation for concept pages |
| `src/concepts/class-selection/ClassSelectionConcept.tsx` | Create | Main concept component |
| `src/concepts/class-selection/data.ts` | Create | Hardcoded enriched Monk data |
| `src/concepts/class-selection/components/` | Create | Sub-components (overview, ability guidance, proficiencies, equipment) |

## Success Criteria

- A developer or player can open the concepts view and immediately understand what makes Monk tick
- Ability score assignment feels guided, not guesswork
- Equipment choices show enough info to pick intelligently
- The content and layout inform what fields the toolkit needs to produce
- The "Primary: 2" bug is identified and fixed in the real modal

## Open Questions

1. Should the concepts page be stripped from production builds, or is it useful as a reference?
2. Do we want to prototype the ability score assignment step too (with guidance repeated there), or just the class modal for now?
3. Should we include a subclass selection prototype (e.g., Way of the Open Hand) in this pass?
