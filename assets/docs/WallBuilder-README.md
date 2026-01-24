# Wall Builder

**Version:** 1.1
**For:** Three.js
**Purpose:** Procedural wall and pillar generation with voxel-style shading

## Features

1. **Percentage-Based Thickness** - Walls scale proportionally to hex size
2. **Auto-Shading** - HSL-based light/dark variance via AdvancedCharacterShader
3. **Material Caching** - Efficient reuse of shader materials
4. **Selection Effects** - Highlight walls/pillars with animated glow
5. **API Adapters** - Consume rpg-toolkit/rpg-api data directly

---

## Quick Start

```javascript
import * as THREE from 'three';
import { WallBuilder, WallColors } from './WallBuilder.js';

// Create builder with your hex dimensions
const builder = new WallBuilder({
    hexWidth: 48,              // Your hex tile width
    wallThicknessRatio: 0.10   // 10% of hex width = 4.8 units
});

// Create a pillar
const pillar = builder.createPillar(
    new THREE.Vector3(0, 0, 0),  // Base position
    10                            // Height
);

// Create a wall between two points
const wall = builder.createWallBetween(
    new THREE.Vector3(0, 0, 0),   // Start anchor
    new THREE.Vector3(20, 0, 0),  // End anchor
    10                             // Height
);

scene.add(pillar, wall);
```

---

## Wall Thickness

Thickness is calculated as a percentage of hex width by default.

```javascript
// Percentage-based (recommended)
const builder = new WallBuilder({
    hexWidth: 48,              // Your hex tile width
    wallThicknessRatio: 0.10   // 10% = 4.8 units thick
});

// Fixed thickness override
const builder = new WallBuilder({
    thickness: 4   // Always 4 units, ignores hex width
});

// Check calculated thickness
console.log(builder.getThickness());  // 4.8

// Update hex width later
builder.setHexWidth(64);  // Recalculates thickness to 6.4
```

---

## Creating Walls

### Single Pillar

```javascript
const pillar = builder.createPillar(
    new THREE.Vector3(x, y, z),  // Base position (floor level)
    height,                       // Pillar height
    { color: WallColors.stoneDark }  // Optional overrides
);
```

### Wall Between Two Points

```javascript
const wall = builder.createWallBetween(
    new THREE.Vector3(0, 0, 0),    // First anchor
    new THREE.Vector3(10, 0, 5),   // Second anchor
    8,                              // Height
    { color: WallColors.brick }
);
```

The wall automatically:
- Calculates length from anchors
- Positions at midpoint
- Rotates to align with direction

### Complete Wall Section (Pillars + Wall)

```javascript
const section = builder.createWallSection(
    new THREE.Vector3(0, 0, 0),
    new THREE.Vector3(20, 0, 0),
    10,
    {
        pillarThickness: 6,  // Larger pillars at ends
        wallThickness: 4,
        color: WallColors.dungeonGray
    }
);
```

### Hex Edge Walls

Build walls along specific edges of a hex:

```javascript
// Edge indices (pointy-top orientation):
//     1
//   2   0
//   3   5
//     4

const hexWalls = builder.createHexWalls(
    new THREE.Vector3(0, 0, 0),  // Hex center
    24,                           // Hex radius (center to vertex)
    10,                           // Wall height
    [0, 1, 2]                     // Which edges to build
);
```

---

## Color Presets

```javascript
import { WallColors } from './WallBuilder.js';

// Stone variants
WallColors.stoneLight    // 0x9C9C9C
WallColors.stoneMedium   // 0x707070
WallColors.stoneDark     // 0x505050

// Brick variants
WallColors.brickRed      // 0x8B4513
WallColors.brickBrown    // 0x6B3A2E
WallColors.brickTan      // 0xA67B5B

// Wood variants
WallColors.woodLight     // 0xC4A484
WallColors.woodMedium    // 0x8B6914
WallColors.woodDark      // 0x5C4033

// Dungeon variants
WallColors.dungeonGray   // 0x4A4A4A
WallColors.dungeonMoss   // 0x4A5A4A
WallColors.dungeonWet    // 0x3A4A5A

// Special
WallColors.marble        // 0xE8E8E8
WallColors.obsidian      // 0x1A1A2E
WallColors.sandstone     // 0xD4B896
WallColors.ice           // 0xADD8E6
```

---

## Selection / Highlight

```javascript
// Enable shader animations (required for selection glow)
const stopAnimations = builder.startAnimations();

// Select a wall
builder.setSelected(wall, true);
builder.setSelected(wall, true, 0xFF4444);  // Red highlight

// Deselect
builder.setSelected(wall, false);

// Works on groups too (wall sections)
builder.setSelected(wallSection, true);
```

---

## Pillar with Custom Base

Use your own OBJ model for the pillar base:

```javascript
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';

const loader = new OBJLoader();
loader.load('pillar_base.obj', (obj) => {
    const baseGeometry = obj.children[0].geometry;

    const pillar = builder.createPillarWithBase(
        obj,           // Base model
        position,      // Base position
        20,            // Total height (including base)
        4,             // Base model height
        { color: WallColors.stoneMedium }
    );

    scene.add(pillar);
});
```

---

## API Integration

### From rpg-api Wall Array

```javascript
import { createWallsFromAPI } from './WallBuilder.js';

// API wall format: { start: {x,y,z}, end: {x,y,z}, material?: string }
const apiWalls = [
    { start: {x: 0, y: 0, z: 0}, end: {x: 1, y: -1, z: 0}, material: 'stone' },
    { start: {x: 1, y: -1, z: 0}, end: {x: 1, y: 0, z: -1}, material: 'stone' }
];

const wallGroup = createWallsFromAPI(apiWalls, {
    hexSize: 1,           // Hex size for coordinate conversion
    wallHeight: 3,        // Wall height in world units
    floorY: 0,            // Floor Y position
    thicknessRatio: 0.10, // Wall thickness ratio
    color: WallColors.stoneMedium  // Default color
});

scene.add(wallGroup);
```

### From rpg-api Room Object

```javascript
import { createRoomWallsFromAPI } from './WallBuilder.js';

const apiRoom = {
    width: 10,
    height: 8,
    walls: [
        { start: {x: 0, y: 0, z: 0}, end: {x: 5, y: -5, z: 0} },
        // ... more walls
    ]
};

const walls = createRoomWallsFromAPI(apiRoom, {
    hexSize: 1,
    wallHeight: 3
});

scene.add(walls);
```

### Material Mapping

API materials automatically map to colors:

| API Material | Wall Color |
|--------------|------------|
| `stone` | stoneMedium |
| `wood` | woodMedium |
| `metal` | stoneDark |
| `dungeon` | dungeonGray |
| `brick` | brickRed |
| `marble` | marble |
| `obsidian` | obsidian |
| `ice` | ice |
| `sandstone` | sandstone |

---

## Convenience Functions

Quick creation without instantiating WallBuilder:

```javascript
import { createPillar, createWall, createWallSection } from './WallBuilder.js';

// Quick pillar
const pillar = createPillar(
    new THREE.Vector3(0, 0, 0),
    10,
    { color: WallColors.stone, hexWidth: 48 }
);

// Quick wall
const wall = createWall(
    new THREE.Vector3(0, 0, 0),
    new THREE.Vector3(20, 0, 0),
    10,
    { color: WallColors.brick }
);

// Quick wall section
const section = createWallSection(
    anchorA, anchorB, height,
    { color: WallColors.dungeonGray }
);
```

---

## Constructor Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `hexWidth` | number | 48 | Hex tile width (for thickness calc) |
| `wallThicknessRatio` | number | 0.10 | Thickness as % of hex width |
| `thickness` | number | - | Fixed thickness override |
| `color` | hex | 0x707070 | Default wall color |
| `shadingVariance` | number | 0.15 | Auto-shading amount |
| `flatShading` | boolean | true | Use flat shading |

---

## Methods Reference

| Method | Description |
|--------|-------------|
| `createPillar(position, height, options?)` | Single pillar at position |
| `createWallBetween(anchorA, anchorB, height, options?)` | Wall between two points |
| `createWallSection(anchorA, anchorB, height, options?)` | Two pillars + wall |
| `createHexWalls(center, radius, height, edges, options?)` | Walls on hex edges |
| `createPillarWithBase(baseModel, position, totalHeight, baseHeight, options?)` | Pillar with OBJ base |
| `setSelected(mesh, isSelected, color?)` | Set selection highlight |
| `getThickness()` | Get calculated thickness |
| `setHexWidth(width)` | Update hex width |
| `startAnimations()` | Start shader animations |
| `updateTime(deltaTime)` | Manual time update |
| `dispose()` | Clean up materials |

---

## Coordinate System

The API adapters convert cube coordinates to Three.js world space:

```
Cube coordinates: {x, y, z} where x + y + z = 0
World coordinates: Three.js XZ plane (Y is up)

Conversion:
  world.x = hexSize * sqrt(3) * (cube.x + cube.z / 2)
  world.z = hexSize * (3/2) * cube.z
```

---

## License

Part of the D&D 5E Modular Character Assets project.
