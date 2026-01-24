# Medium Humanoid Character - Integration Guide

**For:** Frontend developers integrating the medium humanoid character into React + Three.js applications

**What you're getting:**
- Medium body OBJ files (12 parts: torso, head, arms, legs, feet, eyes)
- Pre-calculated position and rotation coordinates from Blender
- This integration guide

## Required OBJ Files

```
torso_medium.obj
head_human.obj
arm_upper_medium.obj (used for both arms)
forearm_medium.obj (used for both forearms)
leg_medium.obj (used for both legs)
foot_medium.obj (used for both feet)
eye_basic_oval.obj (used for both eyes)
```

## Complete Character Configuration

Copy this configuration directly into your code:

```javascript
const MEDIUM_HUMANOID_CONFIG = {
  // CRITICAL: Character group must be rotated to convert from Blender (Z-up) to Three.js (Y-up)
  characterGroupRotation: { x: -Math.PI / 2, y: 0, z: 0 }, // -90° on X axis

  parts: {
    torso: {
      file: 'torso_medium.obj',
      position: { x: 0.0, y: 0.0, z: 62.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    head: {
      file: 'head_human.obj',
      position: { x: 0.0, y: 0.0, z: 81.7782 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    armUpperRight: {
      file: 'arm_upper_medium.obj',
      position: { x: 0.0, y: 19.5383, z: 71.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    armUpperLeft: {
      file: 'arm_upper_medium.obj',
      position: { x: 0.0, y: -18.2636, z: 71.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    forearmRight: {
      file: 'forearm_medium.obj',
      position: { x: 0.0, y: 18.8854, z: 65.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    forearmLeft: {
      file: 'forearm_medium.obj',
      position: { x: 0.0, y: -18.885, z: 65.0 },
      rotation: { x: 1.5708, y: 0.0, z: 1.5708 }
    },
    legRight: {
      file: 'leg_medium.obj',
      position: { x: 0.0, y: 9.4368, z: 0.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    legLeft: {
      file: 'leg_medium.obj',
      position: { x: 0.0, y: -9.2074, z: 0.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    footRight: {
      file: 'foot_medium.obj',
      position: { x: 1.0, y: 9.5, z: 0.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    footLeft: {
      file: 'foot_medium.obj',
      position: { x: 1.0, y: -9.25, z: 0.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    eyeRight: {
      file: 'eye_basic_oval.obj',
      position: { x: -9.0, y: 6.9315, z: 95.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    },
    eyeLeft: {
      file: 'eye_basic_oval.obj',
      position: { x: -9.0, y: -6.0, z: 95.0 },
      rotation: { x: 1.5708, y: 0.0, z: 0.0 }
    }
  }
};
```

## Implementation

### Vanilla Three.js

```javascript
import * as THREE from 'three';
import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';

const loader = new OBJLoader();
const character = new THREE.Group();

// CRITICAL: Rotate character group to convert coordinate systems
character.rotation.x = -Math.PI / 2; // -90° to stand character upright

scene.add(character);

// Load all parts
Object.entries(MEDIUM_HUMANOID_CONFIG.parts).forEach(([partName, partConfig]) => {
  loader.load(partConfig.file, (obj) => {
    // CRITICAL: Set rotation order to ZYX (Blender XYZ → Three.js ZYX)
    obj.rotation.order = 'ZYX';

    // Apply rotation
    obj.rotation.set(
      partConfig.rotation.x,
      partConfig.rotation.y,
      partConfig.rotation.z
    );

    // Apply position
    obj.position.set(
      partConfig.position.x,
      partConfig.position.y,
      partConfig.position.z
    );

    character.add(obj);
  });
});
```

### React Three Fiber

```jsx
import { useLoader } from '@react-three/fiber';
import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';
import { useEffect, useRef } from 'react';

function MediumCharacter({ config = MEDIUM_HUMANOID_CONFIG }) {
  const characterRef = useRef();

  return (
    <group
      ref={characterRef}
      rotation={[config.characterGroupRotation.x, 0, 0]}
    >
      {Object.entries(config.parts).map(([partName, partConfig]) => (
        <CharacterPart
          key={partName}
          name={partName}
          config={partConfig}
        />
      ))}
    </group>
  );
}

function CharacterPart({ name, config }) {
  const obj = useLoader(OBJLoader, config.file);
  const partRef = useRef();

  useEffect(() => {
    if (partRef.current) {
      // CRITICAL: Set rotation order to ZYX (Blender XYZ → Three.js ZYX)
      partRef.current.rotation.order = 'ZYX';

      // Apply rotation
      partRef.current.rotation.set(
        config.rotation.x,
        config.rotation.y,
        config.rotation.z
      );

      // Apply position
      partRef.current.position.set(
        config.position.x,
        config.position.y,
        config.position.z
      );
    }
  }, [config]);

  return <primitive ref={partRef} object={obj.clone()} />;
}

// Usage
<Canvas>
  <MediumCharacter />
</Canvas>
```

## CRITICAL Implementation Requirements

### 1. Character Group Rotation (REQUIRED)

**YOU MUST rotate the character group -90° on X axis:**

```javascript
character.rotation.x = -Math.PI / 2;
```

**Why:** The coordinates are from Blender (Z-up coordinate system). Three.js uses Y-up. This rotation converts between them.

**Without this rotation:** Character will be lying on its side.

### 2. Rotation Order Conversion (CRITICAL - REQUIRED)

**KNOWN ISSUE:** Blender's 'XYZ' Euler rotation order ≠ Three.js's 'XYZ' order

**YOU MUST set rotation order to 'ZYX' when applying Blender rotations:**

```javascript
obj.rotation.order = 'ZYX'; // Blender XYZ → Three.js ZYX (REVERSED)
obj.rotation.set(x, y, z);  // Then apply the rotation values
```

**Why:** Matrix convention differences between Blender and Three.js cause the rotation order to be effectively reversed:
- Blender: Row-major matrices, different multiplication order
- Three.js: Column-major matrices, right multiplication
- Result: Blender 'XYZ' = Three.js 'ZYX'

**Without this fix:** Parts with multiple rotations will be perpendicular/wrong even if position is correct.

**Testing:** Left forearm (which has X: 90°, Z: 90° rotation) is the test case - should align with upper arm.

**References:**
- [Three.js Issue #10485](https://github.com/mrdoob/three.js/issues/10485)
- [Blender Euler Conventions](https://devtalk.blender.org/t/euler-angles-convention-in-blender-extrinsic-vs-intrinsic/28177)

### 3. Loading Order Doesn't Matter

Parts can load in any order - they all have absolute positions. No parent-child relationships needed.

### 4. All Rotations Are in Radians

```javascript
rotation: { x: 1.5708, y: 0.0, z: 0.0 }
// 1.5708 radians = 90 degrees
// Math.PI radians = 180 degrees
```

## Verification Checklist

After implementation, verify:

- [ ] Character is standing upright (not lying on side)
- [ ] Head is on top of torso
- [ ] Arms are at sides (both forearms aligned with upper arms)
- [ ] Legs are below torso
- [ ] Feet are at ground level
- [ ] Eyes are on face (not floating)
- [ ] All 12 parts are loaded and visible
- [ ] Rotation order set to 'ZYX' for all parts

## Debugging

### Character lying on side
**Problem:** Missing character group rotation
**Fix:** Add `character.rotation.x = -Math.PI / 2;`

### Parts perpendicular/wrong orientation
**Problem:** Missing rotation order setting or using wrong rotation order
**Fix:** Ensure `obj.rotation.order = 'ZYX';` is set before applying rotations

### Left forearm not aligned with upper arm
**Problem:** Incorrect rotation values or missing rotation order fix
**Fix:**
1. Verify rotation order is set to 'ZYX'
2. Confirm left forearm rotation is `{ x: 1.5708, y: 0.0, z: 1.5708 }`
3. Do NOT add manual mirroring - rotation data handles orientation

### Parts disconnected/floating
**Problem:** Positions not applied correctly
**Fix:** Verify position values match config exactly

### Parts invisible
**Problem:** OBJ files not loading or materials issue
**Fix:** Check file paths, add default materials to meshes

## Character Scale

All measurements are in Qubicle units (scale 1.0):

**Medium Humanoid:**
- Character height: ~137 units (feet at 0, eyes at 95, head top at ~137)
- Torso width: ~16 units
- Arm span: ~38 units (shoulder to shoulder)

If this is too large/small for your scene, scale the entire character group:

```javascript
character.scale.setScalar(0.1); // Scale down 10x if needed
```

## File Organization

Recommended structure:

```
/public/models/characters/
├── torso_medium.obj
├── head_human.obj
├── arm_upper_medium.obj
├── forearm_medium.obj
├── leg_medium.obj
├── foot_medium.obj
└── eye_basic_oval.obj
```

Update `objBasePath` in config to match your structure:

```javascript
const basePath = '/models/characters/';
const fullPath = basePath + partConfig.file;
```

## Performance Notes

- **12 separate OBJ files** - Consider combining for production
- **No LOD** - Full detail models, may need optimization
- **No textures** - Materials need to be applied in your code
- **No animations** - Static pose only

## Materials

The OBJ files have no materials/textures. You'll need to apply materials in your Three.js code:

```javascript
// Example: Apply simple colored materials
loader.load(partConfig.file, (obj) => {
  obj.traverse((child) => {
    if (child.isMesh) {
      child.material = new THREE.MeshStandardMaterial({
        color: 0x8B4513, // Brown for body parts
        roughness: 0.7,
        metalness: 0.1
      });
    }
  });
  // ... apply position/rotation
});
```

## Support

For questions about:
- **Coordinate data accuracy:** Contact the 3D modeling team
- **React integration:** Check React Three Fiber documentation
- **Three.js basics:** Check Three.js documentation

---

**Character Version:** Medium Humanoid v1.0
**Last Updated:** December 2024
**Coordinate System:** Blender Z-up → Three.js Y-up (via character group rotation)
