# Character Material Guide

**How to apply materials to 10-part box characters**

## Material Assignment by Armor Type

### Cloth Armor
```
Head       → skin
Torso      → armor (cloth)
Upper arms → skin (exposed shoulders)
Forearms   → skin (bare hands)
Legs       → armor (cloth pants)
Feet       → armor (cloth boots)
```

### Leather Armor
```
Head       → skin
Torso      → armor (leather)
Upper arms → skin (exposed shoulders)
Forearms   → skin (bare hands)
Legs       → armor (leather pants)
Feet       → armor (leather boots)
```

### Chain Mail
```
Head       → skin
Torso      → armor (chain)
Upper arms → armor (chain pauldrons)
Forearms   → skin (no gauntlets, exposed forearms)
Legs       → armor (chain leggings)
Feet       → armor (boots)
```

### Plate Armor
```
Head       → skin
Torso      → armor (plate)
Upper arms → armor (plate pauldrons)
Forearms   → armor (plate gauntlets, full coverage)
Legs       → armor (plate greaves)
Feet       → armor (plate boots)
```

## Implementation Code

```javascript
function refreshMaterials() {
    const skin = MaterialLibrary.getSkin(character.skinTone);
    const armor = MaterialLibrary.getArmor(character.armorType);

    // Head: always skin
    applyMaterialToMesh(character.head, skin);

    // Torso: always armor
    applyMaterialToMesh(character.torso, armor);

    // Upper arms: cloth/leather = skin, chain/plate = armor
    if (character.armorType === 'cloth' || character.armorType === 'leather') {
        applyMaterialToMesh(character.leftUpperArm, skin);
        applyMaterialToMesh(character.rightUpperArm, skin);
    } else {
        applyMaterialToMesh(character.leftUpperArm, armor);
        applyMaterialToMesh(character.rightUpperArm, armor);
    }

    // Forearms: plate only = armor, everything else = skin
    if (character.armorType === 'plate') {
        applyMaterialToMesh(character.leftForearmHand, armor);
        applyMaterialToMesh(character.rightForearmHand, armor);
    } else {
        applyMaterialToMesh(character.leftForearmHand, skin);
        applyMaterialToMesh(character.rightForearmHand, skin);
    }

    // Legs: always armor
    applyMaterialToMesh(character.leftLeg, armor);
    applyMaterialToMesh(character.rightLeg, armor);

    // Feet: always armor
    applyMaterialToMesh(character.leftFoot, armor);
    applyMaterialToMesh(character.rightFoot, armor);
}
```

## Key Principle

**The lighter the armor, the more skin shows**

- Cloth/Leather: Arms fully exposed
- Chain: Shoulders covered, forearms exposed
- Plate: Full arm coverage

---

## NEW APPROACH: Pure White Skin Marker

**Status:** Replaces whole-mesh material assignment for armor variants with skin/armor regions

### Convention: Pure White (#FFFFFF) = Skin Placeholder

When creating armor material variants in Qubicle:

**✅ DO:**
- Paint exposed skin regions with **pure white (#FFFFFF / RGB 255,255,255)**
- Paint armor/clothing regions with actual armor colors

**❌ DON'T:**
- Use pure white for anything else in character models
- Use pure white is ONLY for skin region markers

### Why Pure White?

1. **Visual Clarity**: Bright white is extremely visible in Qubicle - easy to see which areas are "skin zones"
2. **Art Style Violation**: Pure white should never appear in final renders (use off-white like #F5F5F5 for highlights)
3. **Built-in Debug Indicator**: If you see pure white in the viewer, material replacement failed - something's broken

### How It Works

**In Qubicle:**
```
1. Open torso_medium.qb
2. Paint armor with brown leather colors
3. Paint exposed skin (shoulders, neck, etc.) with pure white #FFFFFF
4. Export → torso_medium_leather.obj + torso_medium_leather.mtl + torso_medium_leather.png
5. Repeat for other materials (cloth, chainmail, plate)
```

**At Runtime (Three.js):**
```javascript
// After loading OBJ
obj.traverse((child) => {
    if (child.isMesh) {
        // Check if material is pure white (skin marker)
        const color = child.material.color;
        if (color.r === 1 && color.g === 1 && color.b === 1) {
            // Replace with actual skin color
            child.material = MaterialLibrary.getSkin(character.skinTone);
        }
        // All other colors stay as exported (armor keeps its colors)
    }
});
```

### Benefits

✅ **No Combinatorial Explosion**: One export per armor type (not per skin tone)
- Export: `torso_medium_leather.obj` (has white skin regions)
- Runtime: Swap white regions with fair/tan/brown/dark skin

✅ **Independent Swapping**: Change armor and skin color separately
- 4 armor types × 4 skin tones = 8 materials to manage (not 16 files)

✅ **Visual Workflow**: Artist can see exactly what's skin vs armor in Qubicle

✅ **Flexible**: Easy to add new skin tones without re-exporting all armor variants

### Example: Torso with Leather Armor

**Qubicle painting:**
- Leather vest/straps: Browns (#8B4513, #654321)
- Exposed shoulders/neck: Pure white (#FFFFFF)

**Export produces:**
- `torso_medium_leather.obj` (geometry)
- `torso_medium_leather.mtl` (material references)
- `torso_medium_leather.png` (texture with white skin areas)

**Runtime:**
- Load torso_medium_leather.obj
- Detect pure white regions
- Replace with selected skin tone (pale/light/medium/tan/dark/deep)
- Result: Leather armor with any skin tone, from one file!
