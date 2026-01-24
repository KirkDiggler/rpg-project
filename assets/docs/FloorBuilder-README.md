# Floor Builder

**Version:** 1.1
**For:** Three.js
**Purpose:** Hex grid floor generation with instanced meshes for performance

## Features

1. **Instanced Rendering** - Efficient rendering of large hex grids
2. **Per-Tile Color Variance** - Subtle random variation for natural look
3. **Elevation Support** - Height maps via function or lookup
4. **Custom Geometry** - Use procedural hexes or your own OBJ models
5. **Mixed Terrain** - Multiple colors/materials in single floor
6. **API Adapters** - Consume rpg-toolkit/rpg-api data directly

---

## Quick Start

```javascript
import * as THREE from 'three';
import { FloorBuilder, FloorColors } from './FloorBuilder.js';

const builder = new FloorBuilder({
    hexWidth: 48,
    hexHeight: 41.6  // ~hexWidth * 0.866
});

// Create a hex-shaped floor (radius 5 = 91 tiles)
const floor = builder.createProceduralFloor(5, {
    color: FloorColors.stone
});

scene.add(floor);
```

---

## Hex Grid Coordinates

Uses axial coordinates (q, r) with pointy-top orientation:

```
       (0,-1)  (1,-1)
    (-1,0)  (0,0)  (1,0)
       (-1,1)  (0,1)
```

---

## Creating Floors

### Procedural Hex Floor (No OBJ Needed)

```javascript
// Hex-shaped floor
const floor = builder.createProceduralFloor(radius, {
    color: FloorColors.stone,
    addVariation: true  // Subtle per-tile color variation
});

// Radius examples:
// 0 = 1 hex (center only)
// 1 = 7 hexes (center + 1 ring)
// 5 = 91 hexes
// 10 = 331 hexes
```

### Rectangular Floor

```javascript
const floor = builder.createRectangularFloor(10, 8, {
    color: FloorColors.woodMedium
});
```

### From Custom OBJ Geometry

```javascript
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';

const loader = new OBJLoader();
loader.load('hex_tile.obj', (obj) => {
    const hexGeometry = obj.children[0].geometry;

    const floor = builder.createFloorFromGeometry(hexGeometry, 5, {
        color: FloorColors.cobblestone
    });

    scene.add(floor);
});
```

---

## Elevation

### Using a Function

```javascript
const floor = builder.createProceduralFloor(5, {
    color: FloorColors.grass,
    elevation: (q, r) => {
        // Hills toward center
        const dist = Math.sqrt(q*q + r*r);
        return Math.max(0, 5 - dist);
    }
});
```

### Using a Lookup Object

```javascript
const floor = builder.createProceduralFloor(5, {
    color: FloorColors.stone,
    elevation: {
        "0,0": 2,    // Center raised
        "1,0": 1,
        "0,1": 1,
        "-1,1": 1
    }
});
```

### Full Elevation Map

```javascript
const elevationMap = [
    { q: 0, r: 0, y: 2, color: FloorColors.marble },
    { q: 1, r: 0, y: 1, color: FloorColors.stone },
    { q: 0, r: 1, y: 1, color: FloorColors.stone },
    { q: -1, r: 0, y: 0, color: FloorColors.grass },
    // ... more tiles
];

const floor = builder.createFloorWithElevation(hexGeometry, elevationMap);
```

---

## Mixed Terrain

Different colors per tile:

```javascript
const terrainMap = [
    { q: 0, r: 0, color: FloorColors.water },
    { q: 1, r: 0, color: FloorColors.sand },
    { q: 2, r: 0, color: FloorColors.grass },
    { q: 3, r: 0, color: FloorColors.grass },
    { q: 4, r: 0, color: FloorColors.dirt },
    // ... define all tiles
];

const floor = builder.createMixedTerrainFloor(terrainMap);
```

Tiles are automatically grouped by color for efficient instanced rendering.

---

## Color Presets

```javascript
import { FloorColors } from './FloorBuilder.js';

// Stone variants
FloorColors.stone         // 0x808080
FloorColors.stoneDark     // 0x606060
FloorColors.stoneLight    // 0xA0A0A0
FloorColors.cobblestone   // 0x707070

// Wood variants
FloorColors.woodLight     // 0xC4A484
FloorColors.woodMedium    // 0x8B7355
FloorColors.woodDark      // 0x5C4033

// Terrain
FloorColors.grass         // 0x4A7C3A
FloorColors.grassDark     // 0x3A5C2A
FloorColors.dirt          // 0x8B6914
FloorColors.sand          // 0xD4B896
FloorColors.snow          // 0xE8E8E8
FloorColors.water         // 0x4A90D9
FloorColors.waterDeep     // 0x2A5080

// Dungeon
FloorColors.dungeonFloor  // 0x4A4A4A
FloorColors.dungeonWet    // 0x3A4A5A
FloorColors.dungeonMoss   // 0x4A5A4A

// Special
FloorColors.lava          // 0xFF4500
FloorColors.ice           // 0xADD8E6
FloorColors.marble        // 0xF0F0F0
FloorColors.obsidian      // 0x1A1A2E
```

---

## Hex Highlighting

For hover/selection effects:

```javascript
// Highlight a specific hex
builder.highlightHex(floor, q, r, 0xFFFF00);  // Yellow highlight

// Get hex from mouse position
const raycaster = new THREE.Raycaster();
raycaster.setFromCamera(mouse, camera);
const hits = raycaster.intersectObject(floor);

if (hits.length > 0) {
    const point = hits[0].point;
    const hex = builder.worldToHex(point.x, point.z);
    builder.highlightHex(floor, hex.q, hex.r, 0x44FF44);
}
```

---

## Coordinate Conversion

```javascript
// Hex to world position
const worldPos = builder.hexToWorld(q, r);
// Returns { x, z }

// World to hex coordinates
const hex = builder.worldToHex(worldX, worldZ);
// Returns { q, r }
```

---

## API Integration

### From rpg-api Room Object

```javascript
import { createFloorFromAPI, createRoomFromAPI } from './FloorBuilder.js';

const apiRoom = {
    width: 10,
    height: 8,
    walls: [/* wall data */]
};

// Floor only
const floor = createFloorFromAPI(apiRoom, {
    hexSize: 1,
    color: FloorColors.dungeonFloor,
    markWallHexes: true  // Darken hexes where walls are
});

// Complete room (floor + walls handled separately)
const room = createRoomFromAPI(apiRoom, {
    hexSize: 1,
    floorColor: FloorColors.stone,
    colorVariance: 0.05
});

scene.add(floor);
```

### Cube Coordinate Helpers

```javascript
import { cubeToWorld, cubeToAxial } from './FloorBuilder.js';

// Cube {x, y, z} -> World {x, z}
const worldPos = cubeToWorld({ x: 1, y: -1, z: 0 }, hexSize);

// Cube {x, y, z} -> Axial {q, r}
const axial = cubeToAxial({ x: 1, y: -1, z: 0 });
// Returns { q: 1, r: 0 }
```

---

## Convenience Functions

Quick creation without instantiating FloorBuilder:

```javascript
import { createHexFloor, createRectFloor } from './FloorBuilder.js';

// Quick hex floor
const floor = createHexFloor(5, {
    color: FloorColors.stone,
    hexWidth: 48
});

// Quick rectangular floor
const floor = createRectFloor(10, 8, {
    color: FloorColors.wood,
    hexWidth: 48
});
```

---

## Hex Math Exports

For game logic integration:

```javascript
import {
    axialToWorld,     // (q, r, hexWidth, hexHeight) => {x, z}
    worldToAxial,     // (x, z, hexWidth, hexHeight) => {q, r}
    getHexesInRadius, // (radius) => [{q, r}, ...]
    getHexesInRect    // (width, height) => [{q, r}, ...]
} from './FloorBuilder.js';

// Get all hexes in radius
const hexes = getHexesInRadius(3);
// Returns 37 hexes

// Get world position
const pos = axialToWorld(2, -1, 48, 41.6);
```

---

## Constructor Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `hexWidth` | number | 48 | Hex width (point to point) |
| `hexHeight` | number | hexWidth * 0.866 | Hex height (flat to flat) |
| `tileThickness` | number | 2 | Thickness of floor tiles |
| `shadingVariance` | number | 0.12 | Auto-shading amount |
| `colorVariance` | number | 0.05 | Per-tile color variation (5%) |

---

## Methods Reference

| Method | Description |
|--------|-------------|
| `createProceduralFloor(radius, options?)` | Hex-shaped floor |
| `createRectangularFloor(width, height, options?)` | Rectangular floor |
| `createFloorFromGeometry(geometry, radius, options?)` | Floor with custom OBJ |
| `createFloorWithElevation(geometry, elevationMap, options?)` | Per-tile elevation/color |
| `createMixedTerrainFloor(terrainMap, options?)` | Multi-color terrain |
| `highlightHex(floor, q, r, color)` | Highlight specific hex |
| `worldToHex(x, z)` | World coords to hex |
| `hexToWorld(q, r)` | Hex to world coords |
| `startAnimations()` | Start shader animations |
| `dispose()` | Clean up materials |

---

## Performance Notes

- Uses `THREE.InstancedMesh` for efficient rendering
- Tiles grouped by color for minimal draw calls
- Material caching prevents duplicate shader creation
- Typical performance:
  - 100 hexes: instant
  - 1,000 hexes: instant
  - 10,000 hexes: ~50ms initial, 60fps render

---

## Coordinate System

```
Axial coordinates (q, r):
  - Used internally for hex positioning
  - Pointy-top orientation

Cube coordinates (x, y, z):
  - Used by rpg-api
  - x + y + z = 0 constraint
  - Conversion: q = x, r = z

World coordinates (Three.js):
  - X/Z plane, Y is up
  - Conversion from axial:
    world.x = hexWidth * (sqrt(3) * q + sqrt(3)/2 * r)
    world.z = hexHeight * (3/2 * r)
```

---

## License

Part of the D&D 5E Modular Character Assets project.
