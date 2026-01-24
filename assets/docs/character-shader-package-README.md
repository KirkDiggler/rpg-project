# Character Shader Package - React/Three.js Integration

Complete shader system for voxel character customization with runtime color swapping, emissive effects, and material management.

---

## 📦 Package Contents

```
character-shader-package/
├── README.md                           # This file
├── shader/
│   └── AdvancedCharacterShader.js      # Main shader module
├── materials/
│   └── medium/                         # Medium body textures
│       ├── torso_medium_fighter.png
│       ├── torso_medium_barbarian.png
│       ├── torso_medium_monk.png
│       ├── torso_medium_bard.png
│       ├── torso_medium_rogue.png
│       ├── arm_upper_medium_*.png      # (5 armor sets)
│       ├── forearm_medium_*.png        # (5 armor sets)
│       ├── leg_medium_*.png            # (5 armor sets)
│       ├── eye_color_swap.png          # Eye texture
│       └── foot_medium.png             # Foot texture
├── documentation/
│   └── AdvancedShaders-USAGE.md        # Complete usage guide
└── examples/
    └── ReactCharacterMaterial.jsx      # Example React component
```

**Total:** 22 texture files + shader + documentation

---

## 🚀 Quick Start (React + Three.js)

### 1. Install Dependencies

```bash
npm install three @react-three/fiber
```

### 2. Copy Files to Your Project

```bash
# Copy shader module
cp shader/AdvancedCharacterShader.js src/shaders/

# Copy textures
cp -r materials/medium public/textures/characters/medium/
```

### 3. Use in React Component

```jsx
import { useLoader } from '@react-three/fiber';
import { TextureLoader } from 'three';
import { createAdvancedCharacterShader } from './shaders/AdvancedCharacterShader';

function CharacterModel() {
  // Load armor texture
  const texture = useLoader(TextureLoader, '/textures/characters/medium/torso_medium_fighter.png');

  // Configure for pixel-perfect voxel rendering
  texture.magFilter = THREE.NearestFilter;
  texture.minFilter = THREE.NearestFilter;
  texture.generateMipmaps = false;

  // Create shader with color customization
  const material = createAdvancedCharacterShader(texture, {
    skinColor: '#D5A88C',      // Medium skin
    primaryColor: '#8B0000',   // Dark red (magenta marker)
    secondaryColor: '#FFD700', // Gold (yellow marker)
    tertiaryColor: '#000000',  // Black (cyan marker)
    detailColor: '#C0C0C0'     // Silver (green marker)
  });

  return (
    <mesh material={material}>
      {/* Your character geometry */}
    </mesh>
  );
}
```

---

## 🎨 Color Swapping System

### Qubicle Marker Colors

The shader replaces these exact hex colors at runtime:

| Marker Color | Qubicle Hex | Runtime Replacement | Use Case |
|--------------|-------------|---------------------|----------|
| White | #FFFFFF | Skin color | Exposed skin areas |
| Magenta | #F704FF | Primary color | Main armor color (can glow) |
| Yellow | #E5FF02 | Secondary color | Accent trim |
| Cyan | #1EDFFF | Tertiary color | Minor details |
| Green | #2BFF06 | Detail color | Fine decorative elements |

**Everything else renders as-is** (browns, grays, metallics painted in Qubicle).

### Runtime Color Updates

```jsx
import { setCharacterColor } from './shaders/AdvancedCharacterShader';

// Change primary color (updates all magenta markers)
setCharacterColor(material, 'primary', '#FF0000');

// Change skin tone
setCharacterColor(material, 'skin', '#704937');

// Change secondary accent
setCharacterColor(material, 'secondary', '#0000FF');
```

---

## 👁️ Special Parts: Eyes, Head, Feet

### Eyes (Iris + Sclera Control)

**Texture:** `eye_color_swap.png`

```jsx
const eyeTexture = useLoader(TextureLoader, '/textures/characters/medium/eye_color_swap.png');
eyeTexture.magFilter = THREE.NearestFilter;
eyeTexture.minFilter = THREE.NearestFilter;

const eyeMaterial = createAdvancedCharacterShader(eyeTexture, {
  skinColor: '#D5A88C',      // Not used for eyes
  primaryColor: '#4A2511',   // Magenta marker → IRIS color
  secondaryColor: '#FFFFFF', // Yellow marker → SCLERA (white) color
  tertiaryColor: '#000000',
  detailColor: '#C0C0C0'
});

// Change eye iris color
setCharacterColor(eyeMaterial, 'primary', '#0088FF'); // Blue eyes

// Change eye sclera/white color
setCharacterColor(eyeMaterial, 'secondary', '#FFFACD'); // Slight yellow tint
```

**Key Points:**
- Paint iris with **magenta marker** #F704FF in Qubicle
- Paint sclera/white with **yellow marker** #E5FF02 in Qubicle
- Primary color uniform controls iris
- Secondary color uniform controls sclera

### Head (Skin Tone Only)

**Texture:** Use solid white or no texture

```jsx
// Create 1x1 white texture for head
const canvas = document.createElement('canvas');
canvas.width = 1;
canvas.height = 1;
const ctx = canvas.getContext('2d');
ctx.fillStyle = '#FFFFFF';
ctx.fillRect(0, 0, 1, 1);

const whiteTexture = new THREE.CanvasTexture(canvas);
whiteTexture.magFilter = THREE.NearestFilter;
whiteTexture.minFilter = THREE.NearestFilter;

const headMaterial = createAdvancedCharacterShader(whiteTexture, {
  skinColor: '#D5A88C',      // Head uses ONLY skin color
  primaryColor: '#000000',   // Not used
  secondaryColor: '#000000', // Not used
  tertiaryColor: '#000000',
  detailColor: '#000000'
});

// Update head skin tone
headMaterial.uniforms.skinColor.value.set('#704937');
```

**Key Points:**
- Head has no texture patterns, just solid color
- Only `skinColor` uniform is used
- Entire head is one solid skin tone

### Feet (Boot/Shoe Color)

**Texture:** `foot_medium.png` or solid color

```jsx
const footMaterial = createAdvancedCharacterShader(whiteTexture, {
  skinColor: '#8B4513',      // BROWN BOOTS (uses skinColor slot!)
  primaryColor: '#000000',   // Not used
  secondaryColor: '#000000', // Not used
  tertiaryColor: '#000000',
  detailColor: '#000000'
});

// Change boot color
footMaterial.uniforms.skinColor.value.set('#000000'); // Black boots
```

**Key Points:**
- Feet use the `skinColor` uniform slot for boot/shoe color
- This is different from actual skin color
- Allows independent foot color control

---

## 🎭 Complete Body Part Shader Setup

```jsx
import React, { useRef, useMemo } from 'react';
import { useLoader } from '@react-three/fiber';
import { TextureLoader } from 'three';
import { createAdvancedCharacterShader, setCharacterColor } from './shaders/AdvancedCharacterShader';

function CharacterWithCustomization() {
  // Store shader references for runtime updates
  const shaders = useRef({
    torso: null,
    armUpper: null,
    forearm: null,
    leg: null,
    head: null,
    eyes: null,
    feet: null
  });

  // Load armor textures
  const armorTextures = {
    torso: useLoader(TextureLoader, '/textures/characters/medium/torso_medium_fighter.png'),
    armUpper: useLoader(TextureLoader, '/textures/characters/medium/arm_upper_medium_fighter.png'),
    forearm: useLoader(TextureLoader, '/textures/characters/medium/forearm_medium_fighter.png'),
    leg: useLoader(TextureLoader, '/textures/characters/medium/leg_medium_fighter.png'),
    eye: useLoader(TextureLoader, '/textures/characters/medium/eye_color_swap.png')
  };

  // Configure all textures for pixel-perfect rendering
  Object.values(armorTextures).forEach(texture => {
    texture.magFilter = THREE.NearestFilter;
    texture.minFilter = THREE.NearestFilter;
    texture.generateMipmaps = false;
    texture.anisotropy = 1;
  });

  // Create white texture for head and feet
  const whiteTexture = useMemo(() => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#FFFFFF';
    ctx.fillRect(0, 0, 1, 1);
    const tex = new THREE.CanvasTexture(canvas);
    tex.magFilter = THREE.NearestFilter;
    tex.minFilter = THREE.NearestFilter;
    return tex;
  }, []);

  // Color scheme
  const colors = {
    skin: '#D5A88C',
    primary: '#8B0000',
    secondary: '#FFD700',
    tertiary: '#000000',
    detail: '#C0C0C0',
    eyeIris: '#4A2511',
    eyeSclera: '#FFFFFF',
    boots: '#8B4513'
  };

  // Create materials
  const materials = useMemo(() => ({
    // Armor pieces (torso, arms, legs)
    torso: createAdvancedCharacterShader(armorTextures.torso, colors),
    armUpper: createAdvancedCharacterShader(armorTextures.armUpper, colors),
    forearm: createAdvancedCharacterShader(armorTextures.forearm, colors),
    leg: createAdvancedCharacterShader(armorTextures.leg, colors),

    // Eyes (special: primary=iris, secondary=sclera)
    eyes: createAdvancedCharacterShader(armorTextures.eye, {
      ...colors,
      primaryColor: colors.eyeIris,
      secondaryColor: colors.eyeSclera
    }),

    // Head (solid skin color)
    head: createAdvancedCharacterShader(whiteTexture, {
      skinColor: colors.skin,
      primaryColor: '#000000',
      secondaryColor: '#000000',
      tertiaryColor: '#000000',
      detailColor: '#000000'
    }),

    // Feet (uses skinColor slot for boot color)
    feet: createAdvancedCharacterShader(whiteTexture, {
      skinColor: colors.boots,
      primaryColor: '#000000',
      secondaryColor: '#000000',
      tertiaryColor: '#000000',
      detailColor: '#000000'
    })
  }), [armorTextures, whiteTexture]);

  // Store shader references
  shaders.current = materials;

  // Color update functions
  const updatePrimaryColor = (color) => {
    // Update all armor pieces (skip eyes, head, feet)
    ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
      setCharacterColor(materials[part], 'primary', color);
    });
  };

  const updateSkinColor = (color) => {
    // Update head
    materials.head.uniforms.skinColor.value.set(color);

    // Also update armor pieces (for exposed skin in textures)
    ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
      materials[part].uniforms.skinColor.value.set(color);
    });
  };

  const updateEyeIrisColor = (color) => {
    setCharacterColor(materials.eyes, 'primary', color);
  };

  const updateBootColor = (color) => {
    materials.feet.uniforms.skinColor.value.set(color);
  };

  return (
    <>
      {/* Torso */}
      <mesh material={materials.torso}>
        {/* torso geometry */}
      </mesh>

      {/* Arms */}
      <mesh material={materials.armUpper}>
        {/* upper arm geometry */}
      </mesh>
      <mesh material={materials.forearm}>
        {/* forearm geometry */}
      </mesh>

      {/* Legs */}
      <mesh material={materials.leg}>
        {/* leg geometry */}
      </mesh>

      {/* Feet */}
      <mesh material={materials.feet}>
        {/* foot geometry */}
      </mesh>

      {/* Head */}
      <mesh material={materials.head}>
        {/* head geometry */}
      </mesh>

      {/* Eyes */}
      <mesh material={materials.eyes}>
        {/* eye geometry */}
      </mesh>

      {/* UI for color picking */}
      <ColorPicker
        onPrimaryChange={updatePrimaryColor}
        onSkinChange={updateSkinColor}
        onEyeChange={updateEyeIrisColor}
        onBootChange={updateBootColor}
      />
    </>
  );
}
```

---

## 📋 Armor Set Mapping

All 5 armor sets included with 4 body parts each:

```javascript
const ARMOR_SETS = {
  fighter: {
    torso: 'torso_medium_fighter.png',
    armUpper: 'arm_upper_medium_fighter.png',
    forearm: 'forearm_medium_fighter.png',
    leg: 'leg_medium_fighter.png'
  },
  barbarian: {
    torso: 'torso_medium_barbarian.png',
    armUpper: 'arm_upper_medium_barbarian.png',
    forearm: 'forearm_medium_barbarian.png',
    leg: 'leg_medium_barbarian.png'
  },
  monk: {
    torso: 'torso_medium_monk.png',
    armUpper: 'arm_upper_medium_monk.png',
    forearm: 'forearm_medium_monk.png',
    leg: 'leg_medium_monk.png'
  },
  bard: {
    torso: 'torso_medium_bard.png',
    armUpper: 'arm_upper_medium_bard.png',
    forearm: 'forearm_medium_bard.png',
    leg: 'leg_medium_bard.png'
  },
  rogue: {
    torso: 'torso_medium_rogue.png',
    armUpper: 'arm_upper_medium_rogue.png',
    forearm: 'forearm_medium_rogue.png',
    leg: 'leg_medium_rogue.png'
  }
};

// Load armor set dynamically
function loadArmorSet(armorType, onLoad) {
  const set = ARMOR_SETS[armorType];
  // Load all 4 textures for this armor type
  // Apply to corresponding body parts
}
```

---

## 🎨 Color Picker Integration

```jsx
import { HexColorPicker } from 'react-colorful';

function CharacterCustomizer({ onColorChange }) {
  const [skinColor, setSkinColor] = useState('#D5A88C');
  const [primaryColor, setPrimaryColor] = useState('#8B0000');
  const [secondaryColor, setSecondaryColor] = useState('#FFD700');
  const [eyeIrisColor, setEyeIrisColor] = useState('#4A2511');
  const [bootColor, setBootColor] = useState('#8B4513');

  return (
    <div className="customizer-panel">
      <h3>Customize Character</h3>

      <div>
        <label>Skin Tone</label>
        <HexColorPicker color={skinColor} onChange={(c) => {
          setSkinColor(c);
          onColorChange('skin', c);
        }} />
      </div>

      <div>
        <label>Armor Primary Color (Magenta Marker)</label>
        <HexColorPicker color={primaryColor} onChange={(c) => {
          setPrimaryColor(c);
          onColorChange('primary', c);
        }} />
      </div>

      <div>
        <label>Armor Secondary Color (Yellow Marker)</label>
        <HexColorPicker color={secondaryColor} onChange={(c) => {
          setSecondaryColor(c);
          onColorChange('secondary', c);
        }} />
      </div>

      <div>
        <label>Eye Iris Color</label>
        <HexColorPicker color={eyeIrisColor} onChange={(c) => {
          setEyeIrisColor(c);
          onColorChange('eyeIris', c);
        }} />
      </div>

      <div>
        <label>Boot Color</label>
        <HexColorPicker color={bootColor} onChange={(c) => {
          setBootColor(c);
          onColorChange('boots', c);
        }} />
      </div>
    </div>
  );
}
```

---

## 🔧 Texture Configuration (CRITICAL)

**ALWAYS configure textures for pixel-perfect voxel rendering:**

```javascript
texture.magFilter = THREE.NearestFilter;   // No blur when zoomed in
texture.minFilter = THREE.NearestFilter;   // No blur when zoomed out
texture.generateMipmaps = false;           // No mipmap blur
texture.anisotropy = 1;                    // Minimum filtering
```

**Also disable antialiasing on renderer:**

```jsx
<Canvas gl={{ antialias: false }}>
  {/* Your scene */}
</Canvas>
```

---

## 📚 Additional Documentation

See `documentation/AdvancedShaders-USAGE.md` for:
- Complete shader API reference
- Emissive glow effects
- Hit flash animations
- Transparency/invisibility effects
- Troubleshooting guide
- Performance notes

---

## 🎯 Summary

**Special Part Rules:**
- **Eyes**: `primaryColor` = iris, `secondaryColor` = sclera
- **Head**: Uses `skinColor` only (solid color)
- **Feet**: Uses `skinColor` slot for boot color
- **Armor**: Uses all 4 color channels (primary, secondary, tertiary, detail)

**Color Marker Hierarchy:**
- Magenta #F704FF → Primary (most important, can glow)
- Yellow #E5FF02 → Secondary
- Cyan #1EDFFF → Tertiary
- Green #2BFF06 → Detail

**Armor Sets Included:**
- Fighter, Barbarian, Monk, Bard, Rogue (20 textures)
- Eye texture (1 texture)
- Foot texture (1 texture)
- **Total: 22 textures**

---

## 🐛 Troubleshooting

**Colors not swapping?**
- Verify Qubicle used EXACT hex values (#F704FF for magenta, etc.)
- Check texture filtering is set to NearestFilter
- Confirm shader threshold in `isColor()` function (0.02)

**Textures look blurry?**
```javascript
texture.magFilter = THREE.NearestFilter;
texture.minFilter = THREE.NearestFilter;
texture.generateMipmaps = false;
```

**Eyes not updating?**
- Remember: `primaryColor` = iris, `secondaryColor` = sclera
- Eyes use separate material from armor

---

**Package Version:** 1.0
**Date:** 2025-12-23
**Status:** Production Ready
**Three.js Version:** Compatible with 0.170.0+
