/**
 * Advanced Character Shader Module
 * Version: 2.4
 * Date: 2026-01-19
 *
 * Combines multiple shader effects for complete character customization:
 * 1. Color Swapping (skin, trim, team colors, etc.)
 * 2. Emissive Glow (magic items, runes, glowing eyes)
 * 3. Hit Flash (damage feedback)
 * 4. Transparency (invisibility, stealth, fade in/out)
 * 5. Team Colors (faction/guild identification)
 * 6. Outline Effect (see OutlineShader.js)
 * 7. Auto-Shading (HSL-based lighter/darker variants from picked colors)
 * 8. Ghost Mode (Fresnel rim glow, desaturation, ethereal transparency)
 * 9. Fire Aura (Animated rim glow, flickering, ember colors)
 * 10. Selection Aura (Subtle pulsing rim glow for selected characters)
 *
 * Marker Color Convention:
 * - Pure White   #FFFFFF - Skin/primary swappable
 * - Pure Red     #FF0000 - Trim/accent swappable
 * - Pure Green   #00FF00 - Metal/detail swappable
 * - Pure Blue    #0000FF - Team/faction swappable
 * - Pure Cyan    #00FFFF - Emissive/glow regions
 * - Pure Magenta #FF00FF - Secondary accent swappable
 */

import * as THREE from 'three';

/**
 * Creates an advanced character shader with multiple effects
 *
 * @param {THREE.Texture} texture - The character texture (with marker colors)
 * @param {Object} options - Configuration options
 * @param {number|THREE.Color} options.skinColor - Primary color (replaces white)
 * @param {number|THREE.Color} options.trimColor - Trim color (replaces red)
 * @param {number|THREE.Color} options.metalColor - Metal color (replaces green)
 * @param {number|THREE.Color} options.teamColor - Team color (replaces blue)
 * @param {number|THREE.Color} options.glowColor - Emissive color (replaces cyan)
 * @param {number|THREE.Color} options.accentColor - Accent color (replaces magenta)
 * @param {number} options.glowIntensity - Glow brightness multiplier (default: 2.0)
 * @param {number} options.opacity - Overall transparency (0.0-1.0, default: 1.0)
 * @param {number} options.flashAmount - Hit flash intensity (0.0-1.0, default: 0.0)
 * @param {number} options.shadingVariance - Auto-shading intensity (0.0=off, 0.15=±15% lightness, default: 0.0)
 * @param {number} options.ghostAmount - Ghost effect intensity (0.0=solid, 1.0=full ghost, default: 0.0)
 * @param {number|THREE.Color} options.ghostColor - Ghost tint color (default: pale cyan 0x88CCFF)
 * @param {number} options.ghostRimPower - Ghost rim glow sharpness (1.0=soft, 4.0=sharp, default: 2.0)
 * @param {number} options.fireAmount - Fire aura intensity (0.0=off, 1.0=full blaze, default: 0.0)
 * @param {number} options.fireSpeed - Fire animation speed (default: 1.0)
 * @param {number|THREE.Color} options.fireColorInner - Inner fire color (default: 0xFF4400 orange-red)
 * @param {number|THREE.Color} options.fireColorOuter - Outer fire color (default: 0xFFDD00 yellow)
 * @param {number} options.selected - Selection aura (0.0=not selected, 1.0=selected, default: 0.0)
 * @param {number|THREE.Color} options.selectionColor - Selection aura color (default: 0xFFFFFF white)
 * @param {number} options.selectionSpeed - Selection pulse speed (default: 2.0)
 * @param {number} options.selectionIntensity - Selection glow brightness (default: 0.5)
 * @returns {THREE.ShaderMaterial}
 *
 * @example
 * const shader = createAdvancedCharacterShader(texture, {
 *     skinColor: 0xD5A88C,
 *     trimColor: 0x8B0000,
 *     teamColor: 0x0000FF,
 *     glowColor: 0x00FFFF,
 *     glowIntensity: 3.0
 * });
 */
export function createAdvancedCharacterShader(texture, options = {}) {
    const defaults = {
        skinColor: 0xD5A88C,      // Medium skin (white marker)
        primaryColor: 0x8B0000,   // Dark red - Primary (replaces magenta)
        secondaryColor: 0xFFD700, // Gold - Secondary (replaces yellow)
        tertiaryColor: 0x000000,  // Black - Tertiary (replaces cyan)
        detailColor: 0xC0C0C0,    // Silver - Detail (replaces green)
        trimColor: 0x8B4513,      // Brown trim (red marker - legacy)
        metalColor: 0xC0C0C0,     // Silver metal (green marker - legacy)
        teamColor: 0x0000FF,      // Blue team (blue marker - legacy)
        glowColor: 0x00FFFF,      // Cyan glow (cyan marker - legacy)
        accentColor: 0xFF00FF,    // Magenta accent (magenta marker - legacy)
        glowIntensity: 1.0,       // Default 1.0 (no boost), increase to make emissive regions glow
        opacity: 1.0,
        flashAmount: 0.0,
        shadingVariance: 0.0,     // Default 0.0 (off), set to 0.15 for ±15% lightness variance
        ghostAmount: 0.0,         // Default 0.0 (solid), set to 1.0 for full ghost
        ghostColor: 0x88CCFF,     // Pale cyan ghost tint
        ghostRimPower: 2.0,       // Rim glow sharpness (1.0=soft, 4.0=sharp)
        fireAmount: 0.0,          // Default 0.0 (off), set to 1.0 for full blaze
        fireSpeed: 1.0,           // Animation speed multiplier
        fireColorInner: 0xFF4400, // Orange-red inner fire
        fireColorOuter: 0xFFDD00, // Yellow outer fire
        selected: 0.0,            // Default 0.0 (not selected)
        selectionColor: 0xFFFFFF, // White selection glow
        selectionSpeed: 2.0,      // Gentle pulse speed
        selectionIntensity: 0.5   // Subtle glow brightness
    };

    const config = { ...defaults, ...options };

    return new THREE.ShaderMaterial({
        uniforms: {
            // Texture
            characterTexture: { value: texture },

            // Swappable colors (marker replacement) - NEW SYSTEM
            skinColor: { value: new THREE.Color(config.skinColor) },
            primaryColor: { value: new THREE.Color(config.primaryColor) },
            secondaryColor: { value: new THREE.Color(config.secondaryColor) },
            tertiaryColor: { value: new THREE.Color(config.tertiaryColor) },
            detailColor: { value: new THREE.Color(config.detailColor) },

            // Legacy uniforms (kept for backwards compatibility)
            trimColor: { value: new THREE.Color(config.trimColor) },
            metalColor: { value: new THREE.Color(config.metalColor) },
            teamColor: { value: new THREE.Color(config.teamColor) },
            glowColor: { value: new THREE.Color(config.glowColor) },
            accentColor: { value: new THREE.Color(config.accentColor) },

            // Effect parameters
            glowIntensity: { value: config.glowIntensity },
            opacity: { value: config.opacity },
            flashAmount: { value: config.flashAmount },
            shadingVariance: { value: config.shadingVariance },

            // Ghost effect
            ghostAmount: { value: config.ghostAmount },
            ghostColor: { value: new THREE.Color(config.ghostColor) },
            ghostRimPower: { value: config.ghostRimPower },

            // Fire aura effect
            time: { value: 0.0 },
            fireAmount: { value: config.fireAmount },
            fireSpeed: { value: config.fireSpeed },
            fireColorInner: { value: new THREE.Color(config.fireColorInner) },
            fireColorOuter: { value: new THREE.Color(config.fireColorOuter) },

            // Selection aura
            selected: { value: config.selected },
            selectionColor: { value: new THREE.Color(config.selectionColor) },
            selectionSpeed: { value: config.selectionSpeed },
            selectionIntensity: { value: config.selectionIntensity }
        },

        vertexShader: `
            // Output to fragment shader
            varying vec2 vUv;
            varying vec3 vNormal;
            varying vec3 vViewPosition;

            void main() {
                // Pass UV coordinates for texture sampling
                vUv = uv;

                // Pass normals for lighting
                vNormal = normalize(normalMatrix * normal);

                // Calculate view position for advanced lighting
                vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
                vViewPosition = -mvPosition.xyz;

                // Standard vertex transformation
                gl_Position = projectionMatrix * mvPosition;
            }
        `,

        fragmentShader: `
            // Inputs from vertex shader
            varying vec2 vUv;
            varying vec3 vNormal;
            varying vec3 vViewPosition;

            // Uniforms
            uniform sampler2D characterTexture;

            // Swappable colors - NEW SYSTEM
            uniform vec3 skinColor;
            uniform vec3 primaryColor;
            uniform vec3 secondaryColor;
            uniform vec3 tertiaryColor;
            uniform vec3 detailColor;

            // Legacy uniforms (backwards compatibility)
            uniform vec3 trimColor;
            uniform vec3 metalColor;
            uniform vec3 teamColor;
            uniform vec3 glowColor;
            uniform vec3 accentColor;

            // Effects
            uniform float glowIntensity;
            uniform float opacity;
            uniform float flashAmount;
            uniform float shadingVariance;

            // Ghost effect
            uniform float ghostAmount;
            uniform vec3 ghostColor;
            uniform float ghostRimPower;

            // Fire aura effect
            uniform float time;
            uniform float fireAmount;
            uniform float fireSpeed;
            uniform vec3 fireColorInner;
            uniform vec3 fireColorOuter;

            // Selection aura
            uniform float selected;
            uniform vec3 selectionColor;
            uniform float selectionSpeed;
            uniform float selectionIntensity;

            // === HSL CONVERSION FUNCTIONS ===
            // RGB to HSL conversion
            vec3 rgbToHsl(vec3 color) {
                float maxC = max(max(color.r, color.g), color.b);
                float minC = min(min(color.r, color.g), color.b);
                float l = (maxC + minC) / 2.0;

                if (maxC == minC) {
                    return vec3(0.0, 0.0, l); // achromatic
                }

                float d = maxC - minC;
                float s = l > 0.5 ? d / (2.0 - maxC - minC) : d / (maxC + minC);

                float h;
                if (maxC == color.r) {
                    h = (color.g - color.b) / d + (color.g < color.b ? 6.0 : 0.0);
                } else if (maxC == color.g) {
                    h = (color.b - color.r) / d + 2.0;
                } else {
                    h = (color.r - color.g) / d + 4.0;
                }
                h /= 6.0;

                return vec3(h, s, l);
            }

            // Helper for HSL to RGB
            float hueToRgb(float p, float q, float t) {
                if (t < 0.0) t += 1.0;
                if (t > 1.0) t -= 1.0;
                if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
                if (t < 1.0/2.0) return q;
                if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
                return p;
            }

            // HSL to RGB conversion
            vec3 hslToRgb(vec3 hsl) {
                float h = hsl.x;
                float s = hsl.y;
                float l = hsl.z;

                if (s == 0.0) {
                    return vec3(l); // achromatic
                }

                float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
                float p = 2.0 * l - q;

                float r = hueToRgb(p, q, h + 1.0/3.0);
                float g = hueToRgb(p, q, h);
                float b = hueToRgb(p, q, h - 1.0/3.0);

                return vec3(r, g, b);
            }

            // Apply shading variance: shift lightness based on normal direction
            vec3 applyAutoShading(vec3 color, float shadeFactor, float variance) {
                if (variance <= 0.0) return color;

                vec3 hsl = rgbToHsl(color);
                // shadeFactor: 0.0 = full shadow, 1.0 = full highlight
                // Shift lightness by variance amount (e.g., ±0.15)
                float lightnessShift = (shadeFactor - 0.5) * 2.0 * variance;
                hsl.z = clamp(hsl.z + lightnessShift, 0.0, 1.0);
                return hslToRgb(hsl);
            }

            // Marker color detection helper
            bool isColor(vec4 texColor, float r, float g, float b) {
                // Tight threshold since we're using actual Qubicle export colors
                float threshold = 0.02;
                return abs(texColor.r - r) < threshold &&
                       abs(texColor.g - g) < threshold &&
                       abs(texColor.b - b) < threshold;
            }

            void main() {
                // Sample the character texture
                vec4 texColor = texture2D(characterTexture, vUv);

                vec3 finalColor;
                bool isEmissive = false;

                // Check marker colors and replace
                // Using ACTUAL Qubicle export colors (not pure colors)
                // Hierarchy: Magenta > Yellow > Cyan > Green

                // White (#FFFFFF) - Skin (pure white)
                if (isColor(texColor, 1.0, 1.0, 1.0)) {
                    finalColor = skinColor;
                }
                // Magenta #F704FF - Primary (most important swappable region)
                else if (isColor(texColor, 0.969, 0.016, 1.0)) {
                    finalColor = primaryColor;
                    isEmissive = true;  // Enable glow for primary regions (e.g., iris)
                }
                // Yellow #E5FF02 - Secondary (accent/detail regions)
                else if (isColor(texColor, 0.898, 1.0, 0.008)) {
                    finalColor = secondaryColor;
                }
                // Cyan #1EDFFF - Tertiary (smaller details/trim)
                else if (isColor(texColor, 0.118, 0.875, 1.0)) {
                    finalColor = tertiaryColor;
                }
                // Green #2BFF06 - Detail/fourth level
                else if (isColor(texColor, 0.169, 1.0, 0.024)) {
                    finalColor = detailColor;
                }
                // Legacy markers (backwards compatibility) - using pure colors
                // Pure red (#FF0000) - Legacy trim color
                else if (isColor(texColor, 1.0, 0.0, 0.0)) {
                    finalColor = trimColor;
                }
                // Pure blue (#0000FF) - Legacy team color
                else if (isColor(texColor, 0.0, 0.0, 1.0)) {
                    finalColor = teamColor;
                }
                // Keep original texture color
                else {
                    finalColor = texColor.rgb;
                }

                // === LIGHTING ===
                // Subtle lighting to show 3D form (nose, etc.) without harsh shadows
                vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5)); // From upper-front-right
                float diffuse = dot(normalize(vNormal), lightDir);

                // shadeFactor: 0.0 (facing away) to 1.0 (facing light)
                float shadeFactor = diffuse * 0.5 + 0.5;

                if (isEmissive) {
                    // Emissive regions glow (boost brightness, no lighting)
                    finalColor *= glowIntensity;
                } else if (shadingVariance > 0.0) {
                    // === AUTO-SHADING MODE ===
                    // Use HSL lightness shift for color-preserving shading
                    finalColor = applyAutoShading(finalColor, shadeFactor, shadingVariance);
                } else {
                    // === LEGACY MODE (shadingVariance = 0) ===
                    // Gentle lighting: 0.7 base + 0.3 directional (never goes below 70% brightness)
                    float lightFactor = 0.7 + 0.3 * max(diffuse, 0.0);
                    finalColor *= lightFactor;
                }

                // === GHOST EFFECT ===
                float finalOpacity = opacity;
                if (ghostAmount > 0.0) {
                    // Fresnel rim calculation - edges facing away from camera glow brighter
                    vec3 viewDir = normalize(vViewPosition);
                    float fresnel = 1.0 - abs(dot(normalize(vNormal), viewDir));
                    fresnel = pow(fresnel, ghostRimPower);

                    // Desaturate the base color
                    float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
                    vec3 desaturated = vec3(luminance);

                    // Blend toward ghost color based on ghostAmount
                    vec3 ghostBase = mix(finalColor, desaturated, ghostAmount * 0.7);
                    ghostBase = mix(ghostBase, ghostColor, ghostAmount * 0.5);

                    // Add rim glow
                    vec3 rimGlow = ghostColor * fresnel * ghostAmount * 1.5;
                    finalColor = ghostBase + rimGlow;

                    // Reduce base opacity, but keep rim more visible
                    float baseOpacity = mix(1.0, 0.3, ghostAmount);
                    float rimOpacity = fresnel * ghostAmount * 0.5;
                    finalOpacity = opacity * (baseOpacity + rimOpacity);
                }

                // === FIRE AURA EFFECT ===
                if (fireAmount > 0.0) {
                    // Fresnel rim for fire glow
                    vec3 viewDir = normalize(vViewPosition);
                    float rim = 1.0 - abs(dot(normalize(vNormal), viewDir));
                    rim = pow(rim, 1.5);

                    // Animated time value
                    float t = time * fireSpeed;

                    // Multi-frequency flicker for organic fire look
                    float flicker = 0.0;
                    flicker += sin(t * 10.0) * 0.15;
                    flicker += sin(t * 23.0 + 1.0) * 0.1;
                    flicker += sin(t * 37.0 + 2.0) * 0.08;
                    flicker += sin(t * 53.0) * 0.05;
                    flicker = flicker + 0.7; // Base intensity

                    // Vertical variation - flames rise up (use world Y via normal)
                    float rise = sin(t * 5.0 + vNormal.y * 3.0) * 0.2 + 0.8;

                    // Combine rim with flicker and rise
                    float fireIntensity = rim * flicker * rise * fireAmount;

                    // Blend between inner (orange-red) and outer (yellow) fire colors
                    vec3 fireColor = mix(fireColorInner, fireColorOuter, rim);

                    // Add fire glow
                    finalColor += fireColor * fireIntensity * 2.0;

                    // Warm up the base color slightly
                    finalColor = mix(finalColor, finalColor * vec3(1.2, 0.9, 0.7), fireAmount * 0.3);

                    // Clamp to prevent over-bright
                    finalColor = min(finalColor, vec3(1.5));
                }

                // === SELECTION AURA ===
                if (selected > 0.0) {
                    // Fresnel rim glow
                    vec3 viewDir = normalize(vViewPosition);
                    float rim = 1.0 - abs(dot(normalize(vNormal), viewDir));
                    rim = pow(rim, 2.0);

                    // Gentle sine pulse - smooth and subtle
                    float pulse = sin(time * selectionSpeed) * 0.3 + 0.7;

                    // Apply selection glow
                    float glowStrength = rim * pulse * selected * selectionIntensity;
                    finalColor += selectionColor * glowStrength;
                }

                // === HIT FLASH EFFECT ===
                // Mix toward white based on flash amount
                finalColor = mix(finalColor, vec3(1.0), flashAmount);

                // === OUTPUT ===
                gl_FragColor = vec4(finalColor, finalOpacity);
            }
        `,

        // Material properties
        transparent: true,        // Enable transparency
        side: THREE.DoubleSide,   // Render both sides
        depthWrite: true,
        depthTest: true
    });
}

/**
 * Pre-defined color palettes
 */
export const ColorPalettes = {
    // Skin tones
    SkinTones: {
        pale: 0xF1D4C0,
        light: 0xE8C3A8,
        medium: 0xD5A88C,
        tan: 0xC68E6D,
        dark: 0x9D6B4D,
        deep: 0x704937
    },

    // Trim/leather colors
    TrimColors: {
        brown: 0x8B4513,
        black: 0x1C1C1C,
        darkRed: 0x8B0000,
        darkGreen: 0x006400,
        darkBlue: 0x00008B,
        purple: 0x800080,
        orange: 0xFF8C00
    },

    // Metal colors
    MetalColors: {
        silver: 0xC0C0C0,
        gold: 0xFFD700,
        bronze: 0xCD7F32,
        copper: 0xB87333,
        iron: 0x808080,
        steel: 0xB0C4DE
    },

    // Team/faction colors
    TeamColors: {
        red: 0xFF0000,
        blue: 0x0000FF,
        green: 0x00FF00,
        yellow: 0xFFFF00,
        purple: 0x800080,
        orange: 0xFF8C00,
        cyan: 0x00FFFF,
        white: 0xFFFFFF
    },

    // Emissive/glow colors
    GlowColors: {
        cyan: 0x00FFFF,
        magenta: 0xFF00FF,
        yellow: 0xFFFF00,
        green: 0x00FF00,
        blue: 0x0088FF,
        red: 0xFF0000,
        white: 0xFFFFFF
    },

    // Eye colors
    EyeColors: {
        brown: 0x4A2511,
        blue: 0x4A90E2,
        green: 0x4CAF50,
        hazel: 0x8B7355,
        gray: 0x708090,
        amber: 0xFFBF00,
        violet: 0x8A2BE2
    },

    // Hair colors
    HairColors: {
        black: 0x1C1C1C,
        brown: 0x4A2511,
        blonde: 0xE6C35C,
        red: 0xA0522D,
        auburn: 0x8B4513,
        gray: 0x808080,
        white: 0xE0E0E0,
        platinum: 0xE5E4E2
    },

    // Ghost tint colors
    GhostColors: {
        classic: 0x88CCFF,      // Pale blue (default)
        spooky: 0x44FF88,       // Eerie green
        wraith: 0xAA88FF,       // Purple wraith
        banshee: 0xFFFFFF,      // Pale white
        shadow: 0x6688AA,       // Dark blue-gray
        fire: 0xFF8844,         // Fiery orange (for fire elementals/spirits)
        void: 0x220033          // Dark void purple
    },

    // Fire aura color presets (inner, outer pairs)
    FireColors: {
        // Standard fire
        normal: { inner: 0xFF4400, outer: 0xFFDD00 },
        // Blue magic fire
        arcane: { inner: 0x0044FF, outer: 0x44DDFF },
        // Green fel/poison fire
        fel: { inner: 0x00FF44, outer: 0xAAFF00 },
        // Purple void fire
        void: { inner: 0x8800FF, outer: 0xFF44FF },
        // Holy/divine fire
        holy: { inner: 0xFFDD44, outer: 0xFFFFFF },
        // Ice/frost (cold fire)
        frost: { inner: 0x0088FF, outer: 0xAAFFFF },
        // Shadow/dark fire
        shadow: { inner: 0x440066, outer: 0x8844AA },
        // Infernal/hellfire
        infernal: { inner: 0xFF0000, outer: 0xFF4400 }
    },

    // Selection aura colors
    SelectionColors: {
        white: 0xFFFFFF,        // Default/neutral
        gold: 0xFFDD44,         // Friendly/player
        blue: 0x44AAFF,         // Ally/NPC
        red: 0xFF4444,          // Enemy/hostile
        green: 0x44FF44,        // Targetable/interactive
        purple: 0xAA44FF,       // Special/quest
        cyan: 0x44FFFF          // Highlighted/hover
    }
};

/**
 * Helper: Update a specific color uniform
 */
export function setCharacterColor(shader, colorType, color) {
    const uniformName = `${colorType}Color`;

    if (!shader.uniforms[uniformName]) {
        console.warn(`Unknown color type: ${colorType}`);
        return;
    }

    if (color instanceof THREE.Color) {
        shader.uniforms[uniformName].value.copy(color);
    } else {
        shader.uniforms[uniformName].value.set(color);
    }
}

/**
 * Helper: Trigger hit flash effect
 */
export function triggerHitFlash(shader, duration = 300) {
    // Set flash to full
    shader.uniforms.flashAmount.value = 1.0;

    // Fade out over duration
    const startTime = Date.now();

    function animate() {
        const elapsed = Date.now() - startTime;
        const progress = Math.min(elapsed / duration, 1.0);

        // Ease out
        shader.uniforms.flashAmount.value = 1.0 - progress;

        if (progress < 1.0) {
            requestAnimationFrame(animate);
        }
    }

    animate();
}

/**
 * Helper: Fade character in/out
 */
export function fadeCharacter(shader, targetOpacity, duration = 1000) {
    const startOpacity = shader.uniforms.opacity.value;
    const startTime = Date.now();

    function animate() {
        const elapsed = Date.now() - startTime;
        const progress = Math.min(elapsed / duration, 1.0);

        // Ease in-out
        const eased = progress < 0.5
            ? 2 * progress * progress
            : 1 - Math.pow(-2 * progress + 2, 2) / 2;

        shader.uniforms.opacity.value = startOpacity + (targetOpacity - startOpacity) * eased;

        if (progress < 1.0) {
            requestAnimationFrame(animate);
        }
    }

    animate();
}

/**
 * Helper: Pulse glow effect
 */
export function pulseGlow(shader, minIntensity = 1.5, maxIntensity = 3.0, speed = 2.0) {
    const startTime = Date.now();

    function animate() {
        const time = (Date.now() - startTime) / 1000;
        const intensity = minIntensity + (maxIntensity - minIntensity) * (Math.sin(time * speed) * 0.5 + 0.5);
        shader.uniforms.glowIntensity.value = intensity;
        requestAnimationFrame(animate);
    }

    animate();

    // Return stop function
    return () => {
        shader.uniforms.glowIntensity.value = 2.0; // Reset to default
    };
}

/**
 * Helper: Make character invisible (stealth)
 */
export function setInvisible(shader, invisible = true) {
    fadeCharacter(shader, invisible ? 0.3 : 1.0, 500);
}

/**
 * Helper: Set auto-shading variance
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {number} variance - Lightness variance (0.0=off, 0.1=subtle, 0.15=normal, 0.25=dramatic)
 */
export function setShadingVariance(shader, variance) {
    shader.uniforms.shadingVariance.value = Math.max(0.0, Math.min(0.5, variance));
}

/**
 * Helper: Set ghost effect
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {number} amount - Ghost intensity (0.0=solid, 1.0=full ghost)
 * @param {number|THREE.Color} [color] - Optional ghost tint color
 */
export function setGhostMode(shader, amount, color) {
    shader.uniforms.ghostAmount.value = Math.max(0.0, Math.min(1.0, amount));
    if (color !== undefined) {
        if (color instanceof THREE.Color) {
            shader.uniforms.ghostColor.value.copy(color);
        } else {
            shader.uniforms.ghostColor.value.set(color);
        }
    }
}

/**
 * Helper: Animate ghost fade in/out
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {boolean} toGhost - True to fade to ghost, false to fade to solid
 * @param {number} duration - Animation duration in ms (default: 1000)
 */
export function fadeToGhost(shader, toGhost = true, duration = 1000) {
    const startAmount = shader.uniforms.ghostAmount.value;
    const targetAmount = toGhost ? 1.0 : 0.0;
    const startTime = Date.now();

    function animate() {
        const elapsed = Date.now() - startTime;
        const progress = Math.min(elapsed / duration, 1.0);

        // Ease in-out
        const eased = progress < 0.5
            ? 2 * progress * progress
            : 1 - Math.pow(-2 * progress + 2, 2) / 2;

        shader.uniforms.ghostAmount.value = startAmount + (targetAmount - startAmount) * eased;

        if (progress < 1.0) {
            requestAnimationFrame(animate);
        }
    }

    animate();
}

/**
 * Helper: Update shader time (call this in your render loop for animated effects)
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {number} deltaTime - Time since last frame in seconds
 */
export function updateShaderTime(shader, deltaTime) {
    shader.uniforms.time.value += deltaTime;
}

/**
 * Helper: Set fire aura effect
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {number} amount - Fire intensity (0.0=off, 1.0=full blaze)
 * @param {Object} [options] - Optional fire settings
 * @param {number} [options.speed] - Animation speed multiplier
 * @param {number|THREE.Color} [options.innerColor] - Inner fire color
 * @param {number|THREE.Color} [options.outerColor] - Outer fire color
 */
export function setFireMode(shader, amount, options = {}) {
    shader.uniforms.fireAmount.value = Math.max(0.0, Math.min(1.0, amount));

    if (options.speed !== undefined) {
        shader.uniforms.fireSpeed.value = options.speed;
    }
    if (options.innerColor !== undefined) {
        if (options.innerColor instanceof THREE.Color) {
            shader.uniforms.fireColorInner.value.copy(options.innerColor);
        } else {
            shader.uniforms.fireColorInner.value.set(options.innerColor);
        }
    }
    if (options.outerColor !== undefined) {
        if (options.outerColor instanceof THREE.Color) {
            shader.uniforms.fireColorOuter.value.copy(options.outerColor);
        } else {
            shader.uniforms.fireColorOuter.value.set(options.outerColor);
        }
    }
}

/**
 * Helper: Set selection state
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @param {boolean} isSelected - Whether the character is selected
 * @param {Object} [options] - Optional selection settings
 * @param {number|THREE.Color} [options.color] - Selection glow color
 * @param {number} [options.intensity] - Glow brightness (default 0.5)
 * @param {number} [options.speed] - Pulse speed (default 2.0)
 */
export function setSelected(shader, isSelected, options = {}) {
    shader.uniforms.selected.value = isSelected ? 1.0 : 0.0;

    if (options.color !== undefined) {
        if (options.color instanceof THREE.Color) {
            shader.uniforms.selectionColor.value.copy(options.color);
        } else {
            shader.uniforms.selectionColor.value.set(options.color);
        }
    }
    if (options.intensity !== undefined) {
        shader.uniforms.selectionIntensity.value = options.intensity;
    }
    if (options.speed !== undefined) {
        shader.uniforms.selectionSpeed.value = options.speed;
    }
}

/**
 * Helper: Start automatic time updates for animated effects
 * Returns a stop function to cancel the animation loop
 * @param {THREE.ShaderMaterial} shader - The character shader
 * @returns {Function} Stop function
 */
export function startShaderAnimation(shader) {
    let lastTime = Date.now();
    let animationId = null;

    function animate() {
        const now = Date.now();
        const deltaTime = (now - lastTime) / 1000;
        lastTime = now;

        shader.uniforms.time.value += deltaTime;
        animationId = requestAnimationFrame(animate);
    }

    animate();

    // Return stop function
    return () => {
        if (animationId !== null) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }
    };
}

/**
 * Example: Full character setup with all effects
 */
export function exampleCharacterSetup(texture) {
    const shader = createAdvancedCharacterShader(texture, {
        skinColor: ColorPalettes.SkinTones.medium,
        trimColor: ColorPalettes.TrimColors.darkRed,
        metalColor: ColorPalettes.MetalColors.silver,
        teamColor: ColorPalettes.TeamColors.blue,
        glowColor: ColorPalettes.GlowColors.cyan,
        glowIntensity: 2.5
    });

    // Example usage:
    // triggerHitFlash(shader);                    // Flash when damaged
    // setCharacterColor(shader, 'team', 0xFF0000); // Change to red team
    // pulseGlow(shader);                          // Pulse magical glow
    // setInvisible(shader, true);                 // Go invisible

    return shader;
}
