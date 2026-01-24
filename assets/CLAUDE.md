# Assets

Deliverables from teammate. These are the source assets that get integrated into rpg-dnd5e-web.

## Structure

```
assets/
├── models/          OBJ + MTL files (Qubicle-exported)
│   ├── body/        Medium humanoid body parts (torso, arms, legs, feet, eyes)
│   ├── heads/       Race variants (human, elf, dwarf, halfling, goblin)
│   ├── weapons/     All weapon types (swords, axes, daggers, club, glaive)
│   ├── equipment/   Shields (kite, round, tower)
│   ├── hair/        Hairstyles (afro, buzz, cornrows, mohawk, short)
│   └── fhair/       Facial hair (chin, goatee, mustache)
├── textures/        PNG textures with marker colors for shader swapping
│   ├── medium/      Per-class body textures (5 classes × 5 parts)
│   ├── base/        Bare skin fallback textures
│   ├── weapons/     Weapon textures
│   ├── equipment/   Shield textures
│   ├── hair/        Hair textures
│   ├── fhair/       Facial hair textures
│   └── heads/       Head textures per race
├── coords/          JSON position/rotation data for attachments (weapons, hair)
├── shaders/         JS shader implementations
│   ├── AdvancedCharacterShader.js   Marker color detection + replacement
│   ├── OutlineShader.js             Cel-shading outline effect
│   ├── FloorBuilder.js              Hex floor tile generation
│   └── WallBuilder.js               Wall segment generation
└── docs/            Teammate's integration guides and proposals
```

## How Assets Map to rpg-dnd5e-web

| Here | Web Location | Notes |
|------|--------------|-------|
| `models/body/` | `public/models/characters/` | Direct copy |
| `models/heads/` | `public/models/characters/` | Merged with body |
| `textures/medium/` | `public/models/characters/textures/medium/` | Direct copy |
| `textures/base/` | `public/models/characters/textures/base/` | Fallbacks |
| `coords/` | Consumed by `src/config/characterModels.ts` | Position data |
| `shaders/` | Consumed by `src/shaders/*.ts` | Converted to TypeScript |

## Texture Marker Colors

Textures use specific colors that the shader detects and replaces at runtime:
- `#FFFFFF` (White) → Skin color
- `#F704FF` (Magenta) → Primary armor color
- `#E5FF02` (Yellow) → Secondary accent
- `#1EDFFF` (Cyan) → Tertiary details
- `#2BFF06` (Green) → Fine decorative elements

## Naming Conventions

- Models: `{part}_{size}.obj` or `{category}_{type}.obj`
- Textures: `{bodypart}_{class}.png` or `{category}_{type}.png`
- Coords: `{attachment}_coordinates.json` (bilateral: `{name}_left_coordinates.json`)

## New Asset Integration

When teammate delivers new assets:
1. Extract and organize into the structure above
2. Check `docs/` for integration guides
3. Copy to rpg-dnd5e-web `public/` matching the mapping table
4. Update `src/config/characterModels.ts` for new model parts
5. Update `src/config/characterTextures.ts` KNOWN_TEXTURES for new textures
