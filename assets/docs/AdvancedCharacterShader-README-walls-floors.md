# Advanced Character Shader

**Version:** 2.4
**For:** Three.js
**Purpose:** Complete character customization with color swapping, effects, and auras

## Features

1. **Color Swapping** - Replace marker colors with custom colors (skin, armor, trim, etc.)
2. **Emissive Glow** - Make regions glow (magic items, eyes, runes)
3. **Hit Flash** - White flash effect for damage feedback
4. **Transparency** - Fade characters in/out (invisibility, stealth)
5. **Auto-Shading** - HSL-based lighter/darker variants from picked colors
6. **Ghost Mode** - Ethereal transparency with Fresnel rim glow
7. **Fire Aura** - Animated flickering fire effect
8. **Selection Aura** - Subtle pulsing glow for selected characters

---

## Quick Start

```javascript
import * as THREE from 'three';
import {
    createAdvancedCharacterShader,
    startShaderAnimation,
    setSelected,
    setGhostMode,
    setFireMode,
    ColorPalettes
} from './AdvancedCharacterShader.js';

// Load your texture
const texture = new THREE.TextureLoader().load('character.png');

// Create shader with options
const shader = createAdvancedCharacterShader(texture, {
    skinColor: 0xD5A88C,
    primaryColor: 0x8B0000
});

// Apply to mesh
mesh.material = shader;

// Start animation loop (required for fire, selection, ghost effects)
const stopAnimation = startShaderAnimation(shader);
```

---

## Color Swapping

The shader detects marker colors in your texture and replaces them with your chosen colors.

### Marker Colors (paint these in your model)

| Marker | Hex | Replaces With |
|--------|-----|---------------|
| Pure White | `#FFFFFF` | `skinColor` |
| Magenta | `#F704FF` | `primaryColor` (emissive) |
| Yellow | `#E5FF02` | `secondaryColor` |
| Cyan | `#1EDFFF` | `tertiaryColor` |
| Green | `#2BFF06` | `detailColor` |
| Pure Red | `#FF0000` | `trimColor` (legacy) |
| Pure Blue | `#0000FF` | `teamColor` (legacy) |

### Usage

```javascript
const shader = createAdvancedCharacterShader(texture, {
    skinColor: 0xD5A88C,      // Replaces white regions
    primaryColor: 0x8B0000,   // Replaces magenta (glows)
    secondaryColor: 0xFFD700, // Replaces yellow
    tertiaryColor: 0x000000,  // Replaces cyan
    detailColor: 0xC0C0C0     // Replaces green
});

// Change at runtime
import { setCharacterColor } from './AdvancedCharacterShader.js';
setCharacterColor(shader, 'skin', 0xF1D4C0);
setCharacterColor(shader, 'primary', 0x0000FF);
```

### Preset Palettes

```javascript
ColorPalettes.SkinTones.pale      // 0xF1D4C0
ColorPalettes.SkinTones.light     // 0xE8C3A8
ColorPalettes.SkinTones.medium    // 0xD5A88C
ColorPalettes.SkinTones.tan       // 0xC68E6D
ColorPalettes.SkinTones.dark      // 0x9D6B4D
ColorPalettes.SkinTones.deep      // 0x704937

ColorPalettes.MetalColors.silver  // 0xC0C0C0
ColorPalettes.MetalColors.gold    // 0xFFD700
ColorPalettes.MetalColors.bronze  // 0xCD7F32

ColorPalettes.HairColors.black    // 0x1C1C1C
ColorPalettes.HairColors.blonde   // 0xE6C35C
ColorPalettes.HairColors.red      // 0xA0522D

ColorPalettes.EyeColors.brown     // 0x4A2511
ColorPalettes.EyeColors.blue      // 0x4A90E2
ColorPalettes.EyeColors.green     // 0x4CAF50
```

---

## Auto-Shading

Automatically generates lighter/darker color variants based on surface normals. Gives a hand-painted look without needing to bake lighting.

```javascript
const shader = createAdvancedCharacterShader(texture, {
    skinColor: 0xD5A88C,
    shadingVariance: 0.15  // ±15% lightness shift
});

// Or toggle at runtime
import { setShadingVariance } from './AdvancedCharacterShader.js';
setShadingVariance(shader, 0.0);   // Off (flat colors)
setShadingVariance(shader, 0.1);   // Subtle
setShadingVariance(shader, 0.15);  // Normal
setShadingVariance(shader, 0.25);  // Dramatic
```

---

## Ghost Mode

Ethereal transparency with glowing edges. Great for spirits, astral projection, invisibility.

```javascript
const shader = createAdvancedCharacterShader(texture, {
    ghostAmount: 1.0,
    ghostColor: 0x88CCFF,
    ghostRimPower: 2.0
});

// Toggle at runtime
import { setGhostMode, fadeToGhost } from './AdvancedCharacterShader.js';

setGhostMode(shader, 1.0);                    // Instant ghost
setGhostMode(shader, 1.0, 0x44FF88);          // Green ghost
fadeToGhost(shader, true, 1000);              // Animate to ghost (1 second)
fadeToGhost(shader, false, 500);              // Fade back to solid

// Preset ghost colors
ColorPalettes.GhostColors.classic   // 0x88CCFF - pale blue
ColorPalettes.GhostColors.spooky    // 0x44FF88 - eerie green
ColorPalettes.GhostColors.wraith    // 0xAA88FF - purple
ColorPalettes.GhostColors.banshee   // 0xFFFFFF - pale white
ColorPalettes.GhostColors.shadow    // 0x6688AA - dark blue-gray
ColorPalettes.GhostColors.void      // 0x220033 - dark void
```

### Parameters

| Uniform | Range | Description |
|---------|-------|-------------|
| `ghostAmount` | 0.0 - 1.0 | 0 = solid, 1 = full ghost |
| `ghostColor` | hex color | Tint color for the ghost effect |
| `ghostRimPower` | 1.0 - 4.0 | 1 = soft glow, 4 = sharp edge |

---

## Fire Aura

Animated magical fire effect with flickering rim glow. Requires animation loop.

```javascript
// IMPORTANT: Start animation loop first
const stopAnimation = startShaderAnimation(shader);

const shader = createAdvancedCharacterShader(texture, {
    fireAmount: 1.0,
    fireSpeed: 1.0,
    fireColorInner: 0xFF4400,
    fireColorOuter: 0xFFDD00
});

// Toggle at runtime
import { setFireMode } from './AdvancedCharacterShader.js';

setFireMode(shader, 1.0);  // Normal fire on
setFireMode(shader, 0.0);  // Fire off

// Different fire types
setFireMode(shader, 1.0, {
    innerColor: 0x0044FF,
    outerColor: 0x44DDFF
});  // Blue arcane fire

// Preset fire colors
ColorPalettes.FireColors.normal    // { inner: 0xFF4400, outer: 0xFFDD00 }
ColorPalettes.FireColors.arcane    // { inner: 0x0044FF, outer: 0x44DDFF } - blue magic
ColorPalettes.FireColors.fel       // { inner: 0x00FF44, outer: 0xAAFF00 } - green poison
ColorPalettes.FireColors.void      // { inner: 0x8800FF, outer: 0xFF44FF } - purple void
ColorPalettes.FireColors.holy      // { inner: 0xFFDD44, outer: 0xFFFFFF } - divine
ColorPalettes.FireColors.frost     // { inner: 0x0088FF, outer: 0xAAFFFF } - cold fire
ColorPalettes.FireColors.shadow    // { inner: 0x440066, outer: 0x8844AA } - dark
ColorPalettes.FireColors.infernal  // { inner: 0xFF0000, outer: 0xFF4400 } - hellfire
```

### Parameters

| Uniform | Range | Description |
|---------|-------|-------------|
| `fireAmount` | 0.0 - 1.0 | 0 = off, 1 = full blaze |
| `fireSpeed` | 0.5 - 3.0 | Animation speed multiplier |
| `fireColorInner` | hex color | Inner flame color (near surface) |
| `fireColorOuter` | hex color | Outer flame color (at edges) |

---

## Selection Aura

Subtle pulsing glow to indicate selected characters. Clean and uniform.

```javascript
// IMPORTANT: Start animation loop first
const stopAnimation = startShaderAnimation(shader);

// Toggle selection
import { setSelected } from './AdvancedCharacterShader.js';

setSelected(shader, true);   // Selected
setSelected(shader, false);  // Not selected

// With custom options
setSelected(shader, true, {
    color: 0xFFDD44,      // Gold glow
    intensity: 0.7,       // Brighter
    speed: 3.0            // Faster pulse
});

// Preset selection colors
ColorPalettes.SelectionColors.white   // 0xFFFFFF - default/neutral
ColorPalettes.SelectionColors.gold    // 0xFFDD44 - friendly/player
ColorPalettes.SelectionColors.blue    // 0x44AAFF - ally/NPC
ColorPalettes.SelectionColors.red     // 0xFF4444 - enemy/hostile
ColorPalettes.SelectionColors.green   // 0x44FF44 - targetable
ColorPalettes.SelectionColors.purple  // 0xAA44FF - special/quest
ColorPalettes.SelectionColors.cyan    // 0x44FFFF - hover highlight
```

### Parameters

| Uniform | Range | Description |
|---------|-------|-------------|
| `selected` | 0.0 or 1.0 | Selection state |
| `selectionColor` | hex color | Glow color |
| `selectionIntensity` | 0.1 - 1.0 | Glow brightness |
| `selectionSpeed` | 1.0 - 4.0 | Pulse speed |

---

## Hit Flash

Quick white flash for damage feedback.

```javascript
import { triggerHitFlash } from './AdvancedCharacterShader.js';

triggerHitFlash(shader);        // Default 300ms flash
triggerHitFlash(shader, 500);   // 500ms flash
```

---

## Transparency / Fade

```javascript
import { fadeCharacter, setInvisible } from './AdvancedCharacterShader.js';

fadeCharacter(shader, 0.5, 1000);  // Fade to 50% over 1 second
fadeCharacter(shader, 1.0, 500);   // Fade back to solid

setInvisible(shader, true);        // Quick fade to 30% (stealth)
setInvisible(shader, false);       // Fade back to solid
```

---

## Emissive Glow

Regions painted with the magenta marker (`#F704FF`) automatically glow.

```javascript
const shader = createAdvancedCharacterShader(texture, {
    primaryColor: 0x00FFFF,   // Cyan glowing eyes
    glowIntensity: 2.5        // Brightness multiplier
});

// Animated pulse
import { pulseGlow } from './AdvancedCharacterShader.js';
const stopPulse = pulseGlow(shader, 1.5, 3.0, 2.0);  // min, max, speed
// Later: stopPulse() to stop
```

---

## Animation Loop

Fire, selection, and any time-based effects require the animation loop:

```javascript
import { startShaderAnimation, updateShaderTime } from './AdvancedCharacterShader.js';

// Option 1: Auto-managed loop
const stopAnimation = startShaderAnimation(shader);
// Later: stopAnimation() to stop

// Option 2: Manual update in your own render loop
function animate() {
    const deltaTime = clock.getDelta();
    updateShaderTime(shader, deltaTime);
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
}
```

---

## All Uniforms Reference

| Uniform | Type | Default | Description |
|---------|------|---------|-------------|
| `characterTexture` | Texture | required | The character texture |
| `skinColor` | Color | 0xD5A88C | Replaces white marker |
| `primaryColor` | Color | 0x8B0000 | Replaces magenta (emissive) |
| `secondaryColor` | Color | 0xFFD700 | Replaces yellow |
| `tertiaryColor` | Color | 0x000000 | Replaces cyan |
| `detailColor` | Color | 0xC0C0C0 | Replaces green |
| `trimColor` | Color | 0x8B4513 | Replaces red (legacy) |
| `metalColor` | Color | 0xC0C0C0 | Legacy |
| `teamColor` | Color | 0x0000FF | Replaces blue (legacy) |
| `glowColor` | Color | 0x00FFFF | Legacy emissive |
| `accentColor` | Color | 0xFF00FF | Legacy |
| `glowIntensity` | Float | 1.0 | Emissive brightness |
| `opacity` | Float | 1.0 | Overall transparency |
| `flashAmount` | Float | 0.0 | Hit flash intensity |
| `shadingVariance` | Float | 0.0 | Auto-shading amount |
| `ghostAmount` | Float | 0.0 | Ghost effect intensity |
| `ghostColor` | Color | 0x88CCFF | Ghost tint |
| `ghostRimPower` | Float | 2.0 | Ghost rim sharpness |
| `time` | Float | 0.0 | Animation time (auto-updated) |
| `fireAmount` | Float | 0.0 | Fire aura intensity |
| `fireSpeed` | Float | 1.0 | Fire animation speed |
| `fireColorInner` | Color | 0xFF4400 | Inner fire color |
| `fireColorOuter` | Color | 0xFFDD00 | Outer fire color |
| `selected` | Float | 0.0 | Selection state |
| `selectionColor` | Color | 0xFFFFFF | Selection glow color |
| `selectionSpeed` | Float | 2.0 | Selection pulse speed |
| `selectionIntensity` | Float | 0.5 | Selection glow brightness |

---

## Helper Functions

| Function | Description |
|----------|-------------|
| `createAdvancedCharacterShader(texture, options)` | Create the shader |
| `setCharacterColor(shader, type, color)` | Change a color uniform |
| `setShadingVariance(shader, amount)` | Set auto-shading |
| `setGhostMode(shader, amount, color?)` | Set ghost effect |
| `fadeToGhost(shader, toGhost, duration?)` | Animate ghost transition |
| `setFireMode(shader, amount, options?)` | Set fire aura |
| `setSelected(shader, isSelected, options?)` | Set selection state |
| `triggerHitFlash(shader, duration?)` | Trigger hit flash |
| `fadeCharacter(shader, targetOpacity, duration?)` | Fade in/out |
| `setInvisible(shader, invisible)` | Quick stealth toggle |
| `pulseGlow(shader, min, max, speed)` | Animated glow pulse |
| `startShaderAnimation(shader)` | Start animation loop |
| `updateShaderTime(shader, deltaTime)` | Manual time update |

---

## Combining Effects

Effects can be combined:

```javascript
const shader = createAdvancedCharacterShader(texture, {
    skinColor: ColorPalettes.SkinTones.pale,
    primaryColor: ColorPalettes.EyeColors.violet,
    shadingVariance: 0.15,
    ghostAmount: 0.5,           // Semi-transparent ghost
    ghostColor: 0xAA88FF,
    selected: 1.0,              // With selection glow
    selectionColor: 0xFFDD44
});

startShaderAnimation(shader);
```

---

## License

Part of the D&D 5E Modular Character Assets project.
