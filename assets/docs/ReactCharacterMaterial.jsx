import React, { useRef, useMemo, useState } from 'react';
import { useLoader } from '@react-three/fiber';
import { TextureLoader } from 'three';
import * as THREE from 'three';
import { createAdvancedCharacterShader, setCharacterColor } from '../shader/AdvancedCharacterShader';

/**
 * Complete React component demonstrating character shader usage
 * with all special part handling (eyes, head, feet)
 */
export function CharacterWithShaders({ armorType = 'fighter' }) {
  // Store material references for runtime updates
  const materialsRef = useRef({});

  // Color state
  const [colors, setColors] = useState({
    skin: '#D5A88C',
    primary: '#8B0000',
    secondary: '#FFD700',
    tertiary: '#000000',
    detail: '#C0C0C0',
    eyeIris: '#4A2511',
    eyeSclera: '#FFFFFF',
    boots: '#8B4513'
  });

  // Load armor textures
  const armorTextures = {
    torso: useLoader(TextureLoader, `/textures/characters/medium/torso_medium_${armorType}.png`),
    armUpper: useLoader(TextureLoader, `/textures/characters/medium/arm_upper_medium_${armorType}.png`),
    forearm: useLoader(TextureLoader, `/textures/characters/medium/forearm_medium_${armorType}.png`),
    leg: useLoader(TextureLoader, `/textures/characters/medium/leg_medium_${armorType}.png`),
    eye: useLoader(TextureLoader, '/textures/characters/medium/eye_color_swap.png')
  };

  // Configure all textures for pixel-perfect voxel rendering
  Object.values(armorTextures).forEach(texture => {
    texture.magFilter = THREE.NearestFilter;
    texture.minFilter = THREE.NearestFilter;
    texture.generateMipmaps = false;
    texture.anisotropy = 1;
  });

  // Create 1x1 white texture for head and feet
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

  // Create materials
  const materials = useMemo(() => {
    const mats = {
      // Armor pieces - use full color scheme
      torso: createAdvancedCharacterShader(armorTextures.torso, {
        skinColor: colors.skin,
        primaryColor: colors.primary,
        secondaryColor: colors.secondary,
        tertiaryColor: colors.tertiary,
        detailColor: colors.detail
      }),
      armUpper: createAdvancedCharacterShader(armorTextures.armUpper, {
        skinColor: colors.skin,
        primaryColor: colors.primary,
        secondaryColor: colors.secondary,
        tertiaryColor: colors.tertiary,
        detailColor: colors.detail
      }),
      forearm: createAdvancedCharacterShader(armorTextures.forearm, {
        skinColor: colors.skin,
        primaryColor: colors.primary,
        secondaryColor: colors.secondary,
        tertiaryColor: colors.tertiary,
        detailColor: colors.detail
      }),
      leg: createAdvancedCharacterShader(armorTextures.leg, {
        skinColor: colors.skin,
        primaryColor: colors.primary,
        secondaryColor: colors.secondary,
        tertiaryColor: colors.tertiary,
        detailColor: colors.detail
      }),

      // Eyes - SPECIAL: primary = iris, secondary = sclera
      eyes: createAdvancedCharacterShader(armorTextures.eye, {
        skinColor: colors.skin,
        primaryColor: colors.eyeIris,    // Magenta marker → iris
        secondaryColor: colors.eyeSclera, // Yellow marker → sclera/white
        tertiaryColor: colors.tertiary,
        detailColor: colors.detail
      }),

      // Head - SPECIAL: only uses skinColor
      head: createAdvancedCharacterShader(whiteTexture, {
        skinColor: colors.skin,
        primaryColor: '#000000',
        secondaryColor: '#000000',
        tertiaryColor: '#000000',
        detailColor: '#000000'
      }),

      // Feet - SPECIAL: uses skinColor slot for boot color
      feet: createAdvancedCharacterShader(whiteTexture, {
        skinColor: colors.boots,  // Boot color uses skinColor slot!
        primaryColor: '#000000',
        secondaryColor: '#000000',
        tertiaryColor: '#000000',
        detailColor: '#000000'
      })
    };

    materialsRef.current = mats;
    return mats;
  }, [armorTextures, whiteTexture, colors]);

  // Color update functions
  const updateColor = (colorType, newColor) => {
    setColors(prev => ({ ...prev, [colorType]: newColor }));

    switch (colorType) {
      case 'skin':
        // Update head
        materialsRef.current.head.uniforms.skinColor.value.set(newColor);
        // Update armor pieces (for exposed skin areas)
        ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
          materialsRef.current[part].uniforms.skinColor.value.set(newColor);
        });
        break;

      case 'primary':
        // Update all armor pieces
        ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
          setCharacterColor(materialsRef.current[part], 'primary', newColor);
        });
        break;

      case 'secondary':
        ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
          setCharacterColor(materialsRef.current[part], 'secondary', newColor);
        });
        break;

      case 'tertiary':
        ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
          setCharacterColor(materialsRef.current[part], 'tertiary', newColor);
        });
        break;

      case 'detail':
        ['torso', 'armUpper', 'forearm', 'leg'].forEach(part => {
          setCharacterColor(materialsRef.current[part], 'detail', newColor);
        });
        break;

      case 'eyeIris':
        setCharacterColor(materialsRef.current.eyes, 'primary', newColor);
        break;

      case 'eyeSclera':
        setCharacterColor(materialsRef.current.eyes, 'secondary', newColor);
        break;

      case 'boots':
        materialsRef.current.feet.uniforms.skinColor.value.set(newColor);
        break;
    }
  };

  return {
    materials,
    updateColor,
    currentColors: colors
  };
}

/**
 * Example usage in React Three Fiber
 */
export function CharacterScene() {
  const { materials, updateColor, currentColors } = CharacterWithShaders({ armorType: 'fighter' });

  return (
    <>
      {/* Torso */}
      <mesh position={[0, 0, 0]} material={materials.torso}>
        {/* Your torso geometry here */}
        <boxGeometry args={[1, 2, 1]} />
      </mesh>

      {/* Arms */}
      <mesh position={[-1.5, 0.5, 0]} material={materials.armUpper}>
        <boxGeometry args={[0.5, 1, 0.5]} />
      </mesh>
      <mesh position={[1.5, 0.5, 0]} material={materials.armUpper}>
        <boxGeometry args={[0.5, 1, 0.5]} />
      </mesh>

      {/* Forearms */}
      <mesh position={[-1.5, -0.5, 0]} material={materials.forearm}>
        <boxGeometry args={[0.5, 1, 0.5]} />
      </mesh>
      <mesh position={[1.5, -0.5, 0]} material={materials.forearm}>
        <boxGeometry args={[0.5, 1, 0.5]} />
      </mesh>

      {/* Legs */}
      <mesh position={[-0.5, -2, 0]} material={materials.leg}>
        <boxGeometry args={[0.6, 2, 0.6]} />
      </mesh>
      <mesh position={[0.5, -2, 0]} material={materials.leg}>
        <boxGeometry args={[0.6, 2, 0.6]} />
      </mesh>

      {/* Feet */}
      <mesh position={[-0.5, -3.5, 0.2]} material={materials.feet}>
        <boxGeometry args={[0.6, 0.4, 0.8]} />
      </mesh>
      <mesh position={[0.5, -3.5, 0.2]} material={materials.feet}>
        <boxGeometry args={[0.6, 0.4, 0.8]} />
      </mesh>

      {/* Head */}
      <mesh position={[0, 1.5, 0]} material={materials.head}>
        <boxGeometry args={[1, 1, 1]} />
      </mesh>

      {/* Eyes */}
      <mesh position={[-0.2, 1.6, 0.5]} material={materials.eyes}>
        <sphereGeometry args={[0.1, 8, 8]} />
      </mesh>
      <mesh position={[0.2, 1.6, 0.5]} material={materials.eyes}>
        <sphereGeometry args={[0.1, 8, 8]} />
      </mesh>

      {/* Color picker UI component */}
      <ColorPickerUI
        colors={currentColors}
        onColorChange={updateColor}
      />
    </>
  );
}

/**
 * Simple color picker UI (placeholder - use your preferred color picker library)
 */
function ColorPickerUI({ colors, onColorChange }) {
  return (
    <div className="color-picker-panel">
      <h3>Customize Character</h3>

      <div>
        <label>Skin Tone</label>
        <input
          type="color"
          value={colors.skin}
          onChange={(e) => onColorChange('skin', e.target.value)}
        />
      </div>

      <div>
        <label>Armor Primary (Magenta Marker)</label>
        <input
          type="color"
          value={colors.primary}
          onChange={(e) => onColorChange('primary', e.target.value)}
        />
      </div>

      <div>
        <label>Armor Secondary (Yellow Marker)</label>
        <input
          type="color"
          value={colors.secondary}
          onChange={(e) => onColorChange('secondary', e.target.value)}
        />
      </div>

      <div>
        <label>Armor Tertiary (Cyan Marker)</label>
        <input
          type="color"
          value={colors.tertiary}
          onChange={(e) => onColorChange('tertiary', e.target.value)}
        />
      </div>

      <div>
        <label>Armor Detail (Green Marker)</label>
        <input
          type="color"
          value={colors.detail}
          onChange={(e) => onColorChange('detail', e.target.value)}
        />
      </div>

      <div>
        <label>Eye Iris</label>
        <input
          type="color"
          value={colors.eyeIris}
          onChange={(e) => onColorChange('eyeIris', e.target.value)}
        />
      </div>

      <div>
        <label>Eye White/Sclera</label>
        <input
          type="color"
          value={colors.eyeSclera}
          onChange={(e) => onColorChange('eyeSclera', e.target.value)}
        />
      </div>

      <div>
        <label>Boot Color</label>
        <input
          type="color"
          value={colors.boots}
          onChange={(e) => onColorChange('boots', e.target.value)}
        />
      </div>
    </div>
  );
}
