/**
 * Outline Shader Module
 * Version: 1.0
 * Date: 2025-12-18
 *
 * Creates cel-shaded outlines around characters (toon/anime style).
 * Makes voxel characters pop against backgrounds and improves readability.
 *
 * Technique: Two-pass rendering
 * 1. First pass: Render slightly expanded backfaces with outline color
 * 2. Second pass: Render normal frontfaces with character material
 * Result: Black (or colored) outline around character
 *
 * Popular in: Borderlands, Breath of the Wild, anime games
 */

import * as THREE from 'three';

/**
 * Creates an outline material (for first pass rendering)
 *
 * @param {Object} options - Configuration options
 * @param {number|THREE.Color} options.outlineColor - Outline color (default: black)
 * @param {number} options.outlineThickness - Thickness in world units (default: 0.03)
 * @returns {THREE.ShaderMaterial}
 */
export function createOutlineMaterial(options = {}) {
    const defaults = {
        outlineColor: 0x000000,
        outlineThickness: 0.03
    };

    const config = { ...defaults, ...options };

    return new THREE.ShaderMaterial({
        uniforms: {
            outlineColor: { value: new THREE.Color(config.outlineColor) },
            outlineThickness: { value: config.outlineThickness }
        },

        vertexShader: `
            uniform float outlineThickness;

            void main() {
                // Expand vertices along their normals to create outline
                vec3 expandedPosition = position + normal * outlineThickness;

                gl_Position = projectionMatrix * modelViewMatrix * vec4(expandedPosition, 1.0);
            }
        `,

        fragmentShader: `
            uniform vec3 outlineColor;

            void main() {
                gl_FragColor = vec4(outlineColor, 1.0);
            }
        `,

        side: THREE.BackSide,  // CRITICAL: Render only backfaces
        depthWrite: true,
        depthTest: true
    });
}

/**
 * Helper class to manage outline rendering for a character
 *
 * Usage:
 * const outlineManager = new OutlineManager(character, {
 *     outlineColor: 0x000000,
 *     outlineThickness: 0.05
 * });
 * scene.add(outlineManager.outlineGroup);
 */
export class OutlineManager {
    constructor(character, options = {}) {
        this.character = character;
        this.outlineMaterial = createOutlineMaterial(options);
        this.outlineGroup = new THREE.Group();

        this.createOutlineMeshes();
    }

    createOutlineMeshes() {
        // Clone character meshes for outline pass
        this.character.traverse((child) => {
            if (child.isMesh) {
                const outlineMesh = new THREE.Mesh(
                    child.geometry,
                    this.outlineMaterial
                );

                // Match transform
                outlineMesh.position.copy(child.position);
                outlineMesh.rotation.copy(child.rotation);
                outlineMesh.scale.copy(child.scale);

                this.outlineGroup.add(outlineMesh);
            }
        });

        // Position outline group same as character
        this.outlineGroup.position.copy(this.character.position);
        this.outlineGroup.rotation.copy(this.character.rotation);
        this.outlineGroup.scale.copy(this.character.scale);
    }

    setOutlineColor(color) {
        this.outlineMaterial.uniforms.outlineColor.value.set(color);
    }

    setOutlineThickness(thickness) {
        this.outlineMaterial.uniforms.outlineThickness.value = thickness;
    }

    setVisible(visible) {
        this.outlineGroup.visible = visible;
    }

    dispose() {
        this.outlineMaterial.dispose();
        this.outlineGroup.traverse((child) => {
            if (child.geometry) child.geometry.dispose();
        });
    }
}

/**
 * Simple helper: Add outline to character in one call
 *
 * @param {THREE.Object3D} character - The character mesh/group
 * @param {THREE.Scene} scene - The scene to add outline to
 * @param {Object} options - Outline options
 * @returns {OutlineManager}
 *
 * @example
 * const outline = addOutline(character, scene, {
 *     outlineColor: 0x000000,
 *     outlineThickness: 0.05
 * });
 *
 * // Later: Change outline color
 * outline.setOutlineColor(0xFF0000); // Red outline
 *
 * // Disable outline
 * outline.setVisible(false);
 */
export function addOutline(character, scene, options = {}) {
    const manager = new OutlineManager(character, options);
    scene.add(manager.outlineGroup);
    return manager;
}

/**
 * Preset outline styles
 */
export const OutlinePresets = {
    // Classic black outline (most common)
    classic: {
        outlineColor: 0x000000,
        outlineThickness: 0.04
    },

    // Thick cartoon outline
    cartoon: {
        outlineColor: 0x000000,
        outlineThickness: 0.08
    },

    // Thin subtle outline
    subtle: {
        outlineColor: 0x000000,
        outlineThickness: 0.02
    },

    // Colored outline (team indicator)
    teamRed: {
        outlineColor: 0xFF0000,
        outlineThickness: 0.05
    },

    teamBlue: {
        outlineColor: 0x0000FF,
        outlineThickness: 0.05
    },

    // Glowing outline (magical effect)
    glow: {
        outlineColor: 0x00FFFF,
        outlineThickness: 0.06
    },

    // Dark outline (for bright backgrounds)
    dark: {
        outlineColor: 0x1a1a1a,
        outlineThickness: 0.04
    }
};

/**
 * Example: Character with dynamic outline
 */
export function exampleOutlineUsage(character, scene) {
    // Add classic black outline
    const outline = addOutline(character, scene, OutlinePresets.classic);

    // Example: Change outline based on character state

    // When selected
    function onSelect() {
        outline.setOutlineColor(0xFFFF00); // Yellow
        outline.setOutlineThickness(0.06);
    }

    // When on red team
    function onJoinRedTeam() {
        outline.setOutlineColor(0xFF0000); // Red
    }

    // When taking damage
    function onDamage() {
        outline.setOutlineColor(0xFF0000); // Flash red
        setTimeout(() => {
            outline.setOutlineColor(0x000000); // Back to black
        }, 200);
    }

    // Disable outline
    function hideOutline() {
        outline.setVisible(false);
    }

    return outline;
}
