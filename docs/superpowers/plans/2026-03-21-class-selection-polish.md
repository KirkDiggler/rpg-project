# Class Selection Polish — Concepts Prototype Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a `/concepts` view in rpg-dnd5e-web with a richly detailed class selection prototype using hardcoded Monk data, validating the UX before touching backend systems.

**Architecture:** Add a `'concepts'` value to the existing `AppView` union type in `App.tsx`. The concepts view lives under `src/concepts/` with its own sub-navigation. The class selection concept uses hardcoded enriched data to prototype contextual guidance at each decision point (class overview, ability scores, saving throws, proficiencies, equipment).

**Tech Stack:** React 19, TypeScript, Tailwind CSS + CSS variables (via `var(--*)` for theming), Framer Motion, Lucide icons

**Spec:** `rpg-project/docs/superpowers/specs/2026-03-21-class-selection-polish-design.md`

**Working directory:** `/home/kirk/personal/rpg-dnd5e-web`

---

## File Structure

| File | Action | Responsibility |
|------|--------|----------------|
| `src/App.tsx` | Modify | Add `'concepts'` to `AppView`, add nav link on debug panel, render `ConceptsView` |
| `src/concepts/ConceptsView.tsx` | Create | Container with sub-nav for concept pages |
| `src/concepts/class-selection/data.ts` | Create | `EnrichedClassInfo` interface + hardcoded Monk data |
| `src/concepts/class-selection/ClassSelectionConcept.tsx` | Create | Main concept page — assembles all sections |
| `src/concepts/class-selection/ClassOverview.tsx` | Create | Rich description + class identity |
| `src/concepts/class-selection/AbilityScoreGuidance.tsx` | Create | Priority-ordered ability recommendations with explanations |
| `src/concepts/class-selection/SavingThrowContext.tsx` | Create | Saving throws with gameplay context |
| `src/concepts/class-selection/ProficiencyDetails.tsx` | Create | Expanded proficiencies with class-specific notes |
| `src/concepts/class-selection/EquipmentGuidance.tsx` | Create | Equipment options with stats and contextual tips |

---

## Task 0: Project setup — issue and branch

- [ ] **Step 1: Create a GitHub issue for this work**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && gh issue create --title "Add concepts prototype for enriched class selection" --body "Build a /concepts view with hardcoded Monk data to prototype richer class selection UX. See rpg-project spec: docs/superpowers/specs/2026-03-21-class-selection-polish-design.md" --label "type: enhancement"`

- [ ] **Step 2: Create a feature branch from fresh main**

```bash
cd /home/kirk/personal/rpg-dnd5e-web
git checkout main
git pull
git checkout -b feat/concepts-class-selection
```

- [ ] **Step 3: Create a separate issue for the "Primary: 2" bug**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && gh issue create --title "Fix class selection modal showing enum integer instead of ability name" --body "The ClassSelectionModal shows 'Primary: 2' instead of 'Primary: Dexterity'. The primary_ability enum value is displayed as its integer instead of the ability name." --label "type: bug"`

---

## Task 1: Create enriched data types and hardcoded Monk data

**Files:**
- Create: `src/concepts/class-selection/data.ts`

- [ ] **Step 1: Create the EnrichedClassInfo interface and Monk data**

```typescript
// src/concepts/class-selection/data.ts

export interface AbilityGuidance {
  ability: string;
  priority: 'primary' | 'secondary' | 'tertiary';
  explanation: string;
}

export interface EquipmentOption {
  itemName: string;
  damage: string;
  damageType: string;
  properties: string[];
  tip: string;
}

export interface ProficiencyDetails {
  weapons: string[];
  weaponNotes: string;
  armor: string[];
  armorNotes: string;
  tools: string[];
  toolNotes: string;
}

export interface EnrichedClassInfo {
  name: string;
  emoji: string;
  hitDie: number;
  primaryAbility: string;
  savingThrows: string[];
  description: string;
  abilityGuidance: AbilityGuidance[];
  savingThrowContext: string;
  proficiencyDetails: ProficiencyDetails;
  equipmentGuidance: EquipmentOption[];
  skillChoices: {
    count: number;
    options: string[];
    tips: string;
  };
}

export const MONK_DATA: EnrichedClassInfo = {
  name: 'Monk',
  emoji: '👊',
  hitDie: 8,
  primaryAbility: 'Dexterity',
  savingThrows: ['Strength', 'Dexterity'],
  description:
    'Masters of martial arts who channel ki — an innate magical energy — through disciplined combat. Monks are fast, mobile strikers who dart in and out of melee using unarmed strikes and monk weapons. Unlike Fighters who rely on heavy armor and martial weapons, Monks use Dexterity and Wisdom to power both their offense and defense.',
  abilityGuidance: [
    {
      ability: 'Dexterity',
      priority: 'primary',
      explanation:
        'Your attack and damage modifier for monk weapons and unarmed strikes. Also determines your AC through Unarmored Defense (10 + DEX + WIS) and is one of your saving throw proficiencies.',
    },
    {
      ability: 'Wisdom',
      priority: 'secondary',
      explanation:
        'Boosts your AC via Unarmored Defense, sets the save DC for ki abilities like Stunning Strike, and improves Perception — the most-rolled skill in the game.',
    },
    {
      ability: 'Constitution',
      priority: 'tertiary',
      explanation:
        'Hit points matter for a melee class with only a d8 hit die. You will be in the thick of combat without heavy armor.',
    },
  ],
  savingThrowContext:
    'Strength & Dexterity — DEX saves are one of the most common in the game, protecting against fireballs, dragon breath, and area effects. STR saves protect against being knocked prone, grappled, or pushed. At level 14, Diamond Soul gives you proficiency in ALL saving throws.',
  proficiencyDetails: {
    weapons: ['Simple weapons', 'Shortswords'],
    weaponNotes:
      'Any simple melee weapon without the Heavy or Special property counts as a monk weapon. You can use DEX instead of STR for attack and damage rolls with monk weapons. Your Martial Arts damage die starts at d4 and replaces the weapon\'s die when it\'s higher — eventually reaching d10 at level 17.',
    armor: [],
    armorNotes:
      'Monks use Unarmored Defense: AC = 10 + DEX modifier + WIS modifier. Wearing any armor disables Unarmored Defense, Martial Arts, and Unarmored Movement. With 16 DEX and 16 WIS, your AC is 16 — equivalent to chain mail without the stealth penalty.',
    tools: [],
    toolNotes: 'No tool proficiencies. Choose an artisan tool or musical instrument from your background.',
  },
  equipmentGuidance: [
    {
      itemName: 'Shortsword',
      damage: '1d6',
      damageType: 'slashing',
      properties: ['Finesse', 'Light'],
      tip: 'Best monk starting weapon — uses DEX for attacks, counts as a monk weapon, and the Light property lets you dual-wield (though your Martial Arts bonus attack is usually better).',
    },
    {
      itemName: 'Handaxe',
      damage: '1d6',
      damageType: 'slashing',
      properties: ['Light', 'Thrown (20/60)'],
      tip: 'Equal damage to shortsword with a ranged option. Good backup for when you can\'t close the distance. Counts as a monk weapon.',
    },
    {
      itemName: 'Quarterstaff',
      damage: '1d6 (1d8 versatile)',
      damageType: 'bludgeoning',
      properties: ['Versatile'],
      tip: 'Can be wielded two-handed for 1d8 damage before your Martial Arts die surpasses it. Counts as a monk weapon.',
    },
    {
      itemName: 'Spear',
      damage: '1d6 (1d8 versatile)',
      damageType: 'piercing',
      properties: ['Thrown (20/60)', 'Versatile'],
      tip: 'Versatile like the quarterstaff but can also be thrown. Solid all-around pick.',
    },
  ],
  skillChoices: {
    count: 2,
    options: [
      'Acrobatics',
      'Athletics',
      'History',
      'Insight',
      'Religion',
      'Stealth',
    ],
    tips: 'Acrobatics and Stealth complement your mobile combat style. Acrobatics lets you escape grapples using DEX (your best stat), and Stealth pairs with your lack of armor penalties. Insight (WIS-based) is also strong since WIS is your secondary ability.',
  },
};
```

- [ ] **Step 2: Verify the file compiles**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npx tsc --noEmit 2>&1 | head -20`

Expected: No new errors introduced

- [ ] **Step 3: Commit**

```bash
git add src/concepts/class-selection/data.ts
git commit -m "feat: add enriched class data types and hardcoded Monk data for concepts prototype"
```

---

## Task 2: Create the ConceptsView container

**Files:**
- Create: `src/concepts/ConceptsView.tsx`

- [ ] **Step 1: Create ConceptsView with sub-navigation**

```typescript
// src/concepts/ConceptsView.tsx
import { useState } from 'react';
import { motion } from 'framer-motion';
import { ClassSelectionConcept } from './class-selection/ClassSelectionConcept';

type ConceptPage = 'class-selection';

const CONCEPT_PAGES: { id: ConceptPage; label: string }[] = [
  { id: 'class-selection', label: 'Class Selection' },
];

interface ConceptsViewProps {
  onBack: () => void;
}

export function ConceptsView({ onBack }: ConceptsViewProps) {
  const [activePage, setActivePage] = useState<ConceptPage>('class-selection');

  return (
    <div className="max-w-7xl mx-auto">
      {/* Header */}
      <div className="flex items-center justify-between mb-6">
        <div className="flex items-center gap-4">
          <button
            onClick={onBack}
            className="px-3 py-1.5 rounded text-sm"
            style={{
              backgroundColor: 'var(--bg-secondary)',
              color: 'var(--text-primary)',
              border: '1px solid var(--border-primary)',
            }}
          >
            Back
          </button>
          <h1
            className="text-3xl font-bold"
            style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
          >
            Concepts Lab
          </h1>
        </div>
      </div>

      {/* Sub-navigation */}
      <div className="flex gap-2 mb-6">
        {CONCEPT_PAGES.map((page) => (
          <button
            key={page.id}
            onClick={() => setActivePage(page.id)}
            className="px-4 py-2 rounded text-sm font-medium transition-colors"
            style={{
              backgroundColor:
                activePage === page.id
                  ? 'var(--accent-primary)'
                  : 'var(--bg-secondary)',
              color: 'var(--text-primary)',
              border:
                activePage === page.id
                  ? '1px solid var(--accent-primary)'
                  : '1px solid var(--border-primary)',
            }}
          >
            {page.label}
          </button>
        ))}
      </div>

      {/* Active concept page */}
      <motion.div
        key={activePage}
        initial={{ opacity: 0, y: 10 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.2 }}
      >
        {activePage === 'class-selection' && <ClassSelectionConcept />}
      </motion.div>
    </div>
  );
}
```

- [ ] **Step 2: Create a placeholder ClassSelectionConcept so this compiles**

```typescript
// src/concepts/class-selection/ClassSelectionConcept.tsx (placeholder)
export function ClassSelectionConcept() {
  return (
    <div style={{ color: 'var(--text-primary)' }}>
      <p>Class selection concept — coming soon</p>
    </div>
  );
}
```

- [ ] **Step 3: Verify compilation**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npx tsc --noEmit 2>&1 | head -20`

- [ ] **Step 4: Commit**

```bash
git add src/concepts/ConceptsView.tsx src/concepts/class-selection/ClassSelectionConcept.tsx
git commit -m "feat: add ConceptsView container with sub-navigation"
```

---

## Task 3: Wire concepts view into App.tsx

**Files:**
- Modify: `src/App.tsx`

- [ ] **Step 1: Add 'concepts' to AppView type**

In `src/App.tsx`, change line 14:

```typescript
// Before:
type AppView = 'home' | 'character-creation' | 'character-sheet' | 'lobby';

// After:
type AppView = 'home' | 'character-creation' | 'character-sheet' | 'lobby' | 'concepts';
```

- [ ] **Step 2: Import ConceptsView and add rendering**

Add import at the top of `src/App.tsx`:
```typescript
import { ConceptsView } from './concepts/ConceptsView';
```

Add a handler function inside `AppContent`:
```typescript
const handleOpenConcepts = () => {
  setCurrentView('concepts');
};
```

Add the concepts view rendering in the conditional chain. In `App.tsx` around line 177-193, find this exact code:

```typescript
        ) : currentView === 'lobby' ? (
          <LobbyView characterId={lobbyCharacterId} onBack={handleBackToHome} />
        ) : currentView === 'home' ? (
```

Replace it with:

```typescript
        ) : currentView === 'lobby' ? (
          <LobbyView characterId={lobbyCharacterId} onBack={handleBackToHome} />
        ) : currentView === 'concepts' ? (
          <ConceptsView onBack={handleBackToHome} />
        ) : currentView === 'home' ? (
```

- [ ] **Step 3: Add concepts link to the debug panel toggle area**

In `App.tsx` around line 212-220, find this exact code:

```typescript
        {/* Debug panel toggle button */}
        <div className="fixed bottom-4 right-4 z-50">
          <button
            onClick={() => setShowDebugPanel(!showDebugPanel)}
            className="bg-gray-800 hover:bg-gray-700 text-white p-2 rounded-full shadow-lg transition-all"
            title={showDebugPanel ? 'Hide Debug Panel' : 'Show Debug Panel'}
          >
            {showDebugPanel ? '🔧✕' : '🔧'}
```

Replace the entire `<div className="fixed bottom-4 right-4 z-50">` block (through its closing `</div>`) with:

```typescript
        {/* Dev tools buttons */}
        <div className="fixed bottom-4 right-4 z-50 flex gap-2">
          <button
            onClick={handleOpenConcepts}
            className="bg-gray-800 hover:bg-gray-700 text-white p-2 rounded-full shadow-lg transition-all"
            title="Open Concepts Lab"
          >
            🧪
          </button>
          <button
            onClick={() => setShowDebugPanel(!showDebugPanel)}
            className="bg-gray-800 hover:bg-gray-700 text-white p-2 rounded-full shadow-lg transition-all"
            title={showDebugPanel ? 'Hide Debug Panel' : 'Show Debug Panel'}
          >
            {showDebugPanel ? '🔧✕' : '🔧'}
          </button>
        </div>
```

- [ ] **Step 4: Verify the app builds and the concepts view renders**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npm run build 2>&1 | tail -10`

- [ ] **Step 5: Commit**

```bash
git add src/App.tsx
git commit -m "feat: wire concepts view into App navigation with dev button"
```

---

## Task 4: Build ClassOverview component

**Files:**
- Create: `src/concepts/class-selection/ClassOverview.tsx`

- [ ] **Step 1: Create the ClassOverview component**

```typescript
// src/concepts/class-selection/ClassOverview.tsx
import type { EnrichedClassInfo } from './data';

interface ClassOverviewProps {
  classInfo: EnrichedClassInfo;
}

export function ClassOverview({ classInfo }: ClassOverviewProps) {
  return (
    <div
      className="rounded-lg p-6"
      style={{
        backgroundColor: 'var(--bg-secondary)',
        border: '1px solid var(--border-primary)',
      }}
    >
      {/* Class identity */}
      <div className="flex items-center gap-4 mb-4">
        <span className="text-4xl">{classInfo.emoji}</span>
        <div>
          <h2
            className="text-2xl font-bold"
            style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
          >
            {classInfo.name}
          </h2>
          <div className="flex gap-4 mt-1 text-sm" style={{ color: 'var(--text-muted)' }}>
            <span>Hit Die: d{classInfo.hitDie}</span>
            <span>Primary: {classInfo.primaryAbility}</span>
          </div>
        </div>
      </div>

      {/* Rich description */}
      <p
        className="text-base leading-relaxed"
        style={{ color: 'var(--text-primary)', fontFamily: 'Crimson Text, serif' }}
      >
        {classInfo.description}
      </p>
    </div>
  );
}
```

- [ ] **Step 2: Commit**

```bash
git add src/concepts/class-selection/ClassOverview.tsx
git commit -m "feat: add ClassOverview component with rich description"
```

---

## Task 5: Build AbilityScoreGuidance component

**Files:**
- Create: `src/concepts/class-selection/AbilityScoreGuidance.tsx`

- [ ] **Step 1: Create the AbilityScoreGuidance component**

```typescript
// src/concepts/class-selection/AbilityScoreGuidance.tsx
import type { AbilityGuidance } from './data';

interface AbilityScoreGuidanceProps {
  guidance: AbilityGuidance[];
}

const PRIORITY_STYLES: Record<
  AbilityGuidance['priority'],
  { label: string; color: string }
> = {
  primary: { label: 'PRIMARY', color: 'var(--legendary)' },
  secondary: { label: 'SECONDARY', color: 'var(--rare)' },
  tertiary: { label: 'IMPORTANT', color: 'var(--uncommon)' },
};

export function AbilityScoreGuidance({ guidance }: AbilityScoreGuidanceProps) {
  return (
    <div
      className="rounded-lg p-5"
      style={{
        backgroundColor: 'var(--bg-secondary)',
        border: '1px solid var(--border-primary)',
      }}
    >
      <h3
        className="text-lg font-bold mb-4"
        style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
      >
        Ability Score Priority
      </h3>
      <div className="space-y-3">
        {guidance.map((g) => {
          const style = PRIORITY_STYLES[g.priority];
          return (
            <div
              key={g.ability}
              className="flex gap-3 items-start"
            >
              <span
                className="text-xs font-bold px-2 py-0.5 rounded shrink-0 mt-0.5"
                style={{
                  color: style.color,
                  border: `1px solid ${style.color}`,
                }}
              >
                {style.label}
              </span>
              <div>
                <span className="font-semibold" style={{ color: 'var(--text-primary)' }}>
                  {g.ability}
                </span>
                <span className="mx-1.5" style={{ color: 'var(--text-muted)' }}>—</span>
                <span style={{ color: 'var(--text-muted)' }}>{g.explanation}</span>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Commit**

```bash
git add src/concepts/class-selection/AbilityScoreGuidance.tsx
git commit -m "feat: add AbilityScoreGuidance component with priority badges"
```

---

## Task 6: Build SavingThrowContext component

**Files:**
- Create: `src/concepts/class-selection/SavingThrowContext.tsx`

- [ ] **Step 1: Create the SavingThrowContext component**

```typescript
// src/concepts/class-selection/SavingThrowContext.tsx

interface SavingThrowContextProps {
  savingThrows: string[];
  context: string;
}

export function SavingThrowContext({ savingThrows, context }: SavingThrowContextProps) {
  return (
    <div
      className="rounded-lg p-5"
      style={{
        backgroundColor: 'var(--bg-secondary)',
        border: '1px solid var(--border-primary)',
      }}
    >
      <h3
        className="text-lg font-bold mb-3"
        style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
      >
        Saving Throws
      </h3>
      <div className="flex gap-2 mb-3">
        {savingThrows.map((st) => (
          <span
            key={st}
            className="text-sm font-semibold px-3 py-1 rounded"
            style={{
              backgroundColor: 'var(--accent-primary)',
              color: 'var(--text-primary)',
            }}
          >
            {st}
          </span>
        ))}
      </div>
      <p
        className="text-sm leading-relaxed"
        style={{ color: 'var(--text-muted)', fontFamily: 'Crimson Text, serif' }}
      >
        {context}
      </p>
    </div>
  );
}
```

- [ ] **Step 2: Commit**

```bash
git add src/concepts/class-selection/SavingThrowContext.tsx
git commit -m "feat: add SavingThrowContext component with gameplay explanation"
```

---

## Task 7: Build ProficiencyDetails component

**Files:**
- Create: `src/concepts/class-selection/ProficiencyDetails.tsx`

- [ ] **Step 1: Create the ProficiencyDetails component**

```typescript
// src/concepts/class-selection/ProficiencyDetails.tsx
import type { ProficiencyDetails as ProficiencyDetailsType } from './data';

interface ProficiencyDetailsProps {
  details: ProficiencyDetailsType;
  skillChoices: {
    count: number;
    options: string[];
    tips: string;
  };
}

interface ProficiencySectionProps {
  label: string;
  items: string[];
  notes: string;
  emptyText?: string;
}

function ProficiencySection({ label, items, notes, emptyText }: ProficiencySectionProps) {
  return (
    <div className="mb-4 last:mb-0">
      <h4
        className="text-sm font-bold uppercase tracking-wide mb-1.5"
        style={{ color: 'var(--accent-primary)' }}
      >
        {label}
      </h4>
      {items.length > 0 ? (
        <div className="flex flex-wrap gap-2 mb-2">
          {items.map((item) => (
            <span
              key={item}
              className="text-sm px-2 py-0.5 rounded"
              style={{
                backgroundColor: 'var(--card-bg)',
                color: 'var(--text-primary)',
                border: '1px solid var(--border-primary)',
              }}
            >
              {item}
            </span>
          ))}
        </div>
      ) : (
        <p className="text-sm mb-2" style={{ color: 'var(--text-muted)' }}>
          {emptyText || 'None'}
        </p>
      )}
      {notes && (
        <p
          className="text-sm leading-relaxed"
          style={{ color: 'var(--text-muted)', fontFamily: 'Crimson Text, serif' }}
        >
          {notes}
        </p>
      )}
    </div>
  );
}

export function ProficiencyDetails({ details, skillChoices }: ProficiencyDetailsProps) {
  return (
    <div
      className="rounded-lg p-5"
      style={{
        backgroundColor: 'var(--bg-secondary)',
        border: '1px solid var(--border-primary)',
      }}
    >
      <h3
        className="text-lg font-bold mb-4"
        style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
      >
        Proficiencies
      </h3>

      <ProficiencySection
        label="Weapons"
        items={details.weapons}
        notes={details.weaponNotes}
      />
      <ProficiencySection
        label="Armor"
        items={details.armor}
        notes={details.armorNotes}
        emptyText="None — see Unarmored Defense"
      />
      <ProficiencySection
        label="Tools"
        items={details.tools}
        notes={details.toolNotes}
        emptyText="None"
      />

      {/* Skill choices */}
      <div className="mt-4 pt-4" style={{ borderTop: '1px solid var(--border-primary)' }}>
        <h4
          className="text-sm font-bold uppercase tracking-wide mb-1.5"
          style={{ color: 'var(--accent-primary)' }}
        >
          Skills (Choose {skillChoices.count})
        </h4>
        <div className="flex flex-wrap gap-2 mb-2">
          {skillChoices.options.map((skill) => (
            <span
              key={skill}
              className="text-sm px-2 py-0.5 rounded"
              style={{
                backgroundColor: 'var(--card-bg)',
                color: 'var(--text-primary)',
                border: '1px solid var(--border-primary)',
              }}
            >
              {skill}
            </span>
          ))}
        </div>
        <p
          className="text-sm leading-relaxed"
          style={{ color: 'var(--text-muted)', fontFamily: 'Crimson Text, serif' }}
        >
          {skillChoices.tips}
        </p>
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Commit**

```bash
git add src/concepts/class-selection/ProficiencyDetails.tsx
git commit -m "feat: add ProficiencyDetails component with weapon/armor/skill notes"
```

---

## Task 8: Build EquipmentGuidance component

**Files:**
- Create: `src/concepts/class-selection/EquipmentGuidance.tsx`

- [ ] **Step 1: Create the EquipmentGuidance component**

```typescript
// src/concepts/class-selection/EquipmentGuidance.tsx
import type { EquipmentOption } from './data';

interface EquipmentGuidanceProps {
  equipment: EquipmentOption[];
}

export function EquipmentGuidance({ equipment }: EquipmentGuidanceProps) {
  return (
    <div
      className="rounded-lg p-5"
      style={{
        backgroundColor: 'var(--bg-secondary)',
        border: '1px solid var(--border-primary)',
      }}
    >
      <h3
        className="text-lg font-bold mb-4"
        style={{ fontFamily: 'Cinzel, serif', color: 'var(--text-primary)' }}
      >
        Starting Equipment Options
      </h3>

      <div className="space-y-3">
        {equipment.map((item) => (
          <div
            key={item.itemName}
            className="rounded p-4"
            style={{
              backgroundColor: 'var(--card-bg)',
              border: '1px solid var(--border-primary)',
            }}
          >
            <div className="flex items-start justify-between mb-2">
              <h4
                className="font-semibold"
                style={{ color: 'var(--text-primary)' }}
              >
                {item.itemName}
              </h4>
              <div className="flex gap-2 text-xs">
                <span
                  className="px-2 py-0.5 rounded"
                  style={{
                    backgroundColor: 'var(--bg-secondary)',
                    color: 'var(--accent-primary)',
                    border: '1px solid var(--accent-primary)',
                  }}
                >
                  {item.damage} {item.damageType}
                </span>
              </div>
            </div>

            {/* Properties */}
            {item.properties.length > 0 && (
              <div className="flex gap-1.5 mb-2">
                {item.properties.map((prop) => (
                  <span
                    key={prop}
                    className="text-xs px-1.5 py-0.5 rounded"
                    style={{
                      backgroundColor: 'var(--bg-primary)',
                      color: 'var(--text-muted)',
                    }}
                  >
                    {prop}
                  </span>
                ))}
              </div>
            )}

            {/* Tip */}
            <p
              className="text-sm leading-relaxed"
              style={{ color: 'var(--text-muted)', fontFamily: 'Crimson Text, serif' }}
            >
              {item.tip}
            </p>
          </div>
        ))}
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Commit**

```bash
git add src/concepts/class-selection/EquipmentGuidance.tsx
git commit -m "feat: add EquipmentGuidance component with weapon stats and tips"
```

---

## Task 9: Assemble ClassSelectionConcept page

**Files:**
- Modify: `src/concepts/class-selection/ClassSelectionConcept.tsx`

- [ ] **Step 1: Replace placeholder with full assembled page**

```typescript
// src/concepts/class-selection/ClassSelectionConcept.tsx
import { MONK_DATA } from './data';
import { ClassOverview } from './ClassOverview';
import { AbilityScoreGuidance } from './AbilityScoreGuidance';
import { SavingThrowContext } from './SavingThrowContext';
import { ProficiencyDetails } from './ProficiencyDetails';
import { EquipmentGuidance } from './EquipmentGuidance';

export function ClassSelectionConcept() {
  const classInfo = MONK_DATA;

  return (
    <div className="space-y-6">
      {/* Page header */}
      <div>
        <p className="text-sm" style={{ color: 'var(--text-muted)' }}>
          Concept prototype — hardcoded Monk data. Validates what info the toolkit
          needs to produce for an informed class selection experience.
        </p>
      </div>

      {/* Class overview — the "sell" */}
      <ClassOverview classInfo={classInfo} />

      {/* Two-column grid for mechanical details */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <AbilityScoreGuidance guidance={classInfo.abilityGuidance} />
        <SavingThrowContext
          savingThrows={classInfo.savingThrows}
          context={classInfo.savingThrowContext}
        />
      </div>

      {/* Proficiencies — full width */}
      <ProficiencyDetails
        details={classInfo.proficiencyDetails}
        skillChoices={classInfo.skillChoices}
      />

      {/* Equipment guidance */}
      <EquipmentGuidance equipment={classInfo.equipmentGuidance} />
    </div>
  );
}
```

- [ ] **Step 2: Verify the full app builds**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npm run build 2>&1 | tail -10`

Expected: Build succeeds

- [ ] **Step 3: Commit**

```bash
git add src/concepts/class-selection/ClassSelectionConcept.tsx
git commit -m "feat: assemble ClassSelectionConcept page with all sections"
```

---

## Task 10: Visual verification and polish

**Files:**
- Possibly modify any of the concept components based on visual review

- [ ] **Step 1: Start dev server and verify the concepts page renders**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npm run dev`

Open the app in browser. Click the beaker (🧪) button in the bottom-right. Verify:
- Concepts Lab page loads with "Class Selection" tab active
- Back button returns to home
- Monk overview shows emoji, name, hit die, primary ability, and full description
- Ability score guidance shows 3 priorities with colored badges
- Saving throws show STR and DEX badges with context paragraph
- Proficiencies show weapons, armor (empty with note), tools, and skills with tips
- Equipment shows 4 weapon cards with damage, properties, and tips

- [ ] **Step 2: Run ci-check before any push**

Run: `cd /home/kirk/personal/rpg-dnd5e-web && npm run ci-check`

Expected: All checks pass (format, lint, typecheck, build)

- [ ] **Step 3: Fix any issues found during visual review or ci-check**

Address formatting, lint, or visual issues. Re-run `npm run ci-check` after fixes.

- [ ] **Step 4: Final commit if any polish was needed**

```bash
git add -A
git commit -m "fix: address polish and CI issues in concepts prototype"
```
