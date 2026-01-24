/**
 * Wall Builder Utility
 * Version: 1.1
 * Date: 2026-01-19
 *
 * Generates procedural walls and pillars using the AdvancedCharacterShader
 * for consistent voxel-style shading with auto light/dark variance.
 *
 * WALL THICKNESS:
 *   Thickness is calculated as a percentage of hex width.
 *   Default: 10% of hex width (recommended starting point)
 *
 *   Example: hexWidth=48, ratio=0.10 → thickness=4.8 units
 *
 *   You can override with a fixed value if preferred.
 *
 * Usage:
 *   import { WallBuilder, WallColors } from './WallBuilder.js';
 *
 *   // Percentage-based (recommended)
 *   const builder = new WallBuilder({
 *       hexWidth: 48,              // Your hex tile width
 *       wallThicknessRatio: 0.10   // 10% = 4.8 units thick
 *   });
 *
 *   // Or fixed thickness
 *   const builder = new WallBuilder({
 *       thickness: 4   // Always 4 units, ignores hex width
 *   });
 *
 *   const pillar = builder.createPillar(position, height);
 *   const wall = builder.createWallBetween(anchorA, anchorB, height);
 *   scene.add(pillar, wall);
 */

import * as THREE from 'three';
import { createAdvancedCharacterShader, startShaderAnimation } from './AdvancedCharacterShader.js';

// ============================================================================
// COLOR PRESETS
// ============================================================================

export const WallColors = {
    // Stone variants
    stoneLight: 0x9C9C9C,
    stoneMedium: 0x707070,
    stoneDark: 0x505050,

    // Brick variants
    brickRed: 0x8B4513,
    brickBrown: 0x6B3A2E,
    brickTan: 0xA67B5B,

    // Wood variants
    woodLight: 0xC4A484,
    woodMedium: 0x8B6914,
    woodDark: 0x5C4033,

    // Dungeon variants
    dungeonGray: 0x4A4A4A,
    dungeonMoss: 0x4A5A4A,
    dungeonWet: 0x3A4A5A,

    // Special
    marble: 0xE8E8E8,
    obsidian: 0x1A1A2E,
    sandstone: 0xD4B896,
    ice: 0xADD8E6
};

// ============================================================================
// TEXTURE HELPERS
// ============================================================================

/**
 * Create a solid color texture (1x1 pixel)
 * Used to feed the shader when we just want a solid color
 */
function createSolidColorTexture(color) {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;

    const ctx = canvas.getContext('2d');
    const c = new THREE.Color(color);
    ctx.fillStyle = `rgb(${Math.floor(c.r * 255)}, ${Math.floor(c.g * 255)}, ${Math.floor(c.b * 255)})`;
    ctx.fillRect(0, 0, 1, 1);

    const texture = new THREE.CanvasTexture(canvas);
    texture.magFilter = THREE.NearestFilter;
    texture.minFilter = THREE.NearestFilter;

    return texture;
}

// ============================================================================
// WALL BUILDER CLASS
// ============================================================================

export class WallBuilder {
    /**
     * @param {Object} options
     * @param {number} options.hexWidth - Width of your hex tiles (required for percentage-based thickness)
     * @param {number} options.wallThicknessRatio - Wall thickness as percentage of hex width (default: 0.10 = 10%)
     * @param {number} options.thickness - Fixed thickness override (ignores ratio if set)
     * @param {number} options.color - Default wall color
     * @param {number} options.shadingVariance - Auto-shading amount (default: 0.15)
     */
    constructor(options = {}) {
        this.hexWidth = options.hexWidth || 48;  // Default hex width
        this.wallThicknessRatio = options.wallThicknessRatio ?? 0.10;  // 10% recommended default

        // Calculate thickness: fixed override OR percentage of hex width
        this.defaultThickness = options.thickness || (this.hexWidth * this.wallThicknessRatio);

        this.defaultColor = options.color || WallColors.stoneMedium;
        this.shadingVariance = options.shadingVariance ?? 0.15;
        this.flatShading = options.flatShading ?? true;

        // Cache materials by color
        this.materialCache = new Map();

        // Track shaders that need animation updates
        this.animatedShaders = [];
    }

    /**
     * Get the calculated wall thickness
     */
    getThickness() {
        return this.defaultThickness;
    }

    /**
     * Update hex width and recalculate thickness
     */
    setHexWidth(width) {
        this.hexWidth = width;
        this.defaultThickness = this.hexWidth * this.wallThicknessRatio;
    }

    /**
     * Get or create a material for the given color
     */
    getMaterial(color) {
        const colorHex = typeof color === 'number' ? color : color.getHex();

        if (this.materialCache.has(colorHex)) {
            return this.materialCache.get(colorHex);
        }

        const texture = createSolidColorTexture(colorHex);
        const material = createAdvancedCharacterShader(texture, {
            skinColor: colorHex,        // Won't be used (no white in texture)
            primaryColor: colorHex,     // Fallback
            shadingVariance: this.shadingVariance,
            selected: 0.0
        });

        this.materialCache.set(colorHex, material);
        this.animatedShaders.push(material);

        return material;
    }

    /**
     * Create a pillar at the given position
     * @param {Vector3} position - Base position of the pillar
     * @param {number} height - Pillar height
     * @param {Object} options - Optional overrides
     */
    createPillar(position, height, options = {}) {
        const thickness = options.thickness || this.defaultThickness;
        const color = options.color || this.defaultColor;

        const geometry = new THREE.BoxGeometry(thickness, height, thickness);
        const material = this.getMaterial(color);
        const mesh = new THREE.Mesh(geometry, material);

        mesh.position.set(
            position.x,
            position.y + height / 2,
            position.z
        );

        mesh.userData.type = 'pillar';
        mesh.userData.basePosition = position.clone();
        mesh.userData.height = height;

        return mesh;
    }

    /**
     * Create a wall between two anchor positions
     * @param {Vector3} anchorA - First anchor (floor level)
     * @param {Vector3} anchorB - Second anchor (floor level)
     * @param {number} height - Wall height
     * @param {Object} options - Optional overrides
     */
    createWallBetween(anchorA, anchorB, height, options = {}) {
        const thickness = options.thickness || this.defaultThickness;
        const color = options.color || this.defaultColor;

        // Calculate horizontal distance and direction
        const dx = anchorB.x - anchorA.x;
        const dz = anchorB.z - anchorA.z;
        const length = Math.sqrt(dx * dx + dz * dz);

        // Create wall geometry
        const geometry = new THREE.BoxGeometry(length, height, thickness);
        const material = this.getMaterial(color);
        const mesh = new THREE.Mesh(geometry, material);

        // Position at midpoint, raised by half height
        mesh.position.set(
            (anchorA.x + anchorB.x) / 2,
            anchorA.y + height / 2,
            (anchorA.z + anchorB.z) / 2
        );

        // Rotate to align with anchor direction
        const angle = Math.atan2(dz, dx);
        mesh.rotation.y = -angle;

        mesh.userData.type = 'wall';
        mesh.userData.anchorA = anchorA.clone();
        mesh.userData.anchorB = anchorB.clone();
        mesh.userData.height = height;

        return mesh;
    }

    /**
     * Create a pillar with base OBJ and extended geometry
     * @param {THREE.Object3D} baseModel - The pillar base OBJ
     * @param {Vector3} position - Base position
     * @param {number} totalHeight - Total pillar height (including base)
     * @param {number} baseHeight - Height of the base model
     * @param {Object} options - Optional overrides
     */
    createPillarWithBase(baseModel, position, totalHeight, baseHeight, options = {}) {
        const group = new THREE.Group();

        // Clone and position the base
        const base = baseModel.clone();
        base.position.copy(position);
        group.add(base);

        // Create the extended pillar portion above the base
        const extensionHeight = totalHeight - baseHeight;
        if (extensionHeight > 0) {
            const extension = this.createPillar(
                new THREE.Vector3(position.x, position.y + baseHeight, position.z),
                extensionHeight,
                options
            );
            // Adjust position since createPillar already offsets by half height
            extension.position.y = position.y + baseHeight + extensionHeight / 2;
            group.add(extension);
        }

        group.userData.type = 'pillarWithBase';
        group.userData.basePosition = position.clone();
        group.userData.totalHeight = totalHeight;

        return group;
    }

    /**
     * Create a complete wall section with two pillars
     * @param {Vector3} anchorA - First pillar position
     * @param {Vector3} anchorB - Second pillar position
     * @param {number} height - Wall and pillar height
     * @param {Object} options - Optional overrides
     */
    createWallSection(anchorA, anchorB, height, options = {}) {
        const group = new THREE.Group();

        const pillarThickness = options.pillarThickness || this.defaultThickness * 1.5;
        const wallThickness = options.wallThickness || this.defaultThickness;

        // Create pillars
        const pillarA = this.createPillar(anchorA, height, {
            ...options,
            thickness: pillarThickness
        });
        const pillarB = this.createPillar(anchorB, height, {
            ...options,
            thickness: pillarThickness
        });

        // Create wall between them
        const wall = this.createWallBetween(anchorA, anchorB, height, {
            ...options,
            thickness: wallThickness
        });

        group.add(pillarA, pillarB, wall);

        group.userData.type = 'wallSection';
        group.userData.anchorA = anchorA.clone();
        group.userData.anchorB = anchorB.clone();
        group.userData.height = height;

        return group;
    }

    /**
     * Create walls along hex grid edges
     * @param {Vector3} centerPos - Hex center position
     * @param {number} hexRadius - Distance from center to vertex
     * @param {number} height - Wall height
     * @param {Array<number>} edges - Which edges to build (0-5)
     * @param {Object} options - Optional overrides
     */
    createHexWalls(centerPos, hexRadius, height, edges = [0, 1, 2, 3, 4, 5], options = {}) {
        const group = new THREE.Group();

        // Hex vertex angles (pointy-top orientation)
        const vertexAngles = [
            Math.PI / 6,      // 0: top-right
            Math.PI / 2,      // 1: top
            5 * Math.PI / 6,  // 2: top-left
            7 * Math.PI / 6,  // 3: bottom-left
            3 * Math.PI / 2,  // 4: bottom
            11 * Math.PI / 6  // 5: bottom-right
        ];

        // Calculate vertex positions
        const vertices = vertexAngles.map(angle => new THREE.Vector3(
            centerPos.x + Math.cos(angle) * hexRadius,
            centerPos.y,
            centerPos.z + Math.sin(angle) * hexRadius
        ));

        // Build requested edges
        edges.forEach(edgeIndex => {
            const v1 = vertices[edgeIndex];
            const v2 = vertices[(edgeIndex + 1) % 6];
            const wall = this.createWallBetween(v1, v2, height, options);
            group.add(wall);
        });

        group.userData.type = 'hexWalls';
        group.userData.center = centerPos.clone();
        group.userData.edges = edges;

        return group;
    }

    /**
     * Start animation loop for all shaders (needed for effects)
     * Call this if you're using selection, fire, or other animated effects
     */
    startAnimations() {
        const stopFunctions = this.animatedShaders.map(shader =>
            startShaderAnimation(shader)
        );

        return () => stopFunctions.forEach(stop => stop());
    }

    /**
     * Update shader time manually (alternative to startAnimations)
     * Call this in your render loop
     */
    updateTime(deltaTime) {
        this.animatedShaders.forEach(shader => {
            shader.uniforms.time.value += deltaTime;
        });
    }

    /**
     * Set selection state on a wall/pillar
     */
    setSelected(mesh, isSelected, color = 0xFFFFFF) {
        if (mesh.material && mesh.material.uniforms) {
            mesh.material.uniforms.selected.value = isSelected ? 1.0 : 0.0;
            mesh.material.uniforms.selectionColor.value.set(color);
        }

        // Handle groups
        if (mesh.children) {
            mesh.children.forEach(child => this.setSelected(child, isSelected, color));
        }
    }

    /**
     * Clear material cache (call when changing scenes)
     */
    dispose() {
        this.materialCache.forEach(material => {
            material.dispose();
        });
        this.materialCache.clear();
        this.animatedShaders = [];
    }
}

// ============================================================================
// API ADAPTER - Consume rpg-toolkit/rpg-api data format
// ============================================================================

/**
 * Convert cube coordinates to world coordinates
 * Cube coords: {x, y, z} where x + y + z = 0
 * World coords: {x, z} on the Three.js XZ plane
 *
 * @param {Object} cube - Cube coordinate {x, y, z}
 * @param {number} hexSize - Size of hex (width)
 * @returns {Object} World position {x, z}
 */
export function cubeToWorld(cube, hexSize) {
    const SQRT_3 = Math.sqrt(3);
    return {
        x: hexSize * SQRT_3 * (cube.x + cube.z / 2),
        z: hexSize * (3 / 2) * cube.z
    };
}

/**
 * Map API material names to our color presets
 */
const MATERIAL_TO_COLOR = {
    'stone': WallColors.stoneMedium,
    'wood': WallColors.woodMedium,
    'metal': WallColors.stoneDark,
    'dungeon': WallColors.dungeonGray,
    'brick': WallColors.brickRed,
    'marble': WallColors.marble,
    'obsidian': WallColors.obsidian,
    'ice': WallColors.ice,
    'sandstone': WallColors.sandstone,
};

/**
 * Create walls from rpg-api Wall array
 *
 * API Wall format:
 *   { start: {x, y, z}, end: {x, y, z}, material?: string, type?: number }
 *
 * @param {Array} apiWalls - Array of walls from API
 * @param {Object} options
 * @param {number} options.hexSize - Hex size for coordinate conversion
 * @param {number} options.wallHeight - Wall height in world units
 * @param {number} options.floorY - Y position of floor (default 0)
 * @returns {THREE.Group} Group containing all wall meshes
 */
export function createWallsFromAPI(apiWalls, options = {}) {
    const hexSize = options.hexSize || 1;
    const wallHeight = options.wallHeight || 3;
    const floorY = options.floorY || 0;
    const defaultColor = options.color || WallColors.stoneMedium;

    const builder = new WallBuilder({
        hexWidth: hexSize,
        wallThicknessRatio: options.thicknessRatio || 0.10,
        color: defaultColor
    });

    const wallGroup = new THREE.Group();

    apiWalls.forEach((apiWall, index) => {
        // Convert cube coords to world coords
        const startWorld = cubeToWorld(apiWall.start, hexSize);
        const endWorld = cubeToWorld(apiWall.end, hexSize);

        // Create Three.js vectors
        const anchorA = new THREE.Vector3(startWorld.x, floorY, startWorld.z);
        const anchorB = new THREE.Vector3(endWorld.x, floorY, endWorld.z);

        // Determine color from material
        const color = apiWall.material
            ? (MATERIAL_TO_COLOR[apiWall.material] || defaultColor)
            : defaultColor;

        // Create wall
        const wall = builder.createWallBetween(anchorA, anchorB, wallHeight, { color });

        // Store API reference for later (destruction, etc.)
        wall.userData.apiWall = apiWall;
        wall.userData.wallIndex = index;

        wallGroup.add(wall);
    });

    wallGroup.userData.type = 'apiWallGroup';
    wallGroup.userData.wallCount = apiWalls.length;

    return wallGroup;
}

/**
 * Create a complete room (walls + optional pillars at corners) from API data
 *
 * @param {Object} apiRoom - Room data from API { width, height, walls[] }
 * @param {Object} options
 */
export function createRoomWallsFromAPI(apiRoom, options = {}) {
    const walls = apiRoom.walls || [];
    return createWallsFromAPI(walls, {
        ...options,
        hexSize: options.hexSize || 1,
    });
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * Quick pillar creation without instantiating WallBuilder
 * @param {Vector3} position
 * @param {number} height
 * @param {Object} options - { color, hexWidth, wallThicknessRatio, thickness }
 */
export function createPillar(position, height, options = {}) {
    const builder = new WallBuilder({
        color: options.color || WallColors.stoneMedium,
        hexWidth: options.hexWidth,
        wallThicknessRatio: options.wallThicknessRatio,
        thickness: options.thickness
    });
    return builder.createPillar(position, height);
}

/**
 * Quick wall creation without instantiating WallBuilder
 * @param {Vector3} anchorA
 * @param {Vector3} anchorB
 * @param {number} height
 * @param {Object} options - { color, hexWidth, wallThicknessRatio, thickness }
 */
export function createWall(anchorA, anchorB, height, options = {}) {
    const builder = new WallBuilder({
        color: options.color || WallColors.stoneMedium,
        hexWidth: options.hexWidth,
        wallThicknessRatio: options.wallThicknessRatio,
        thickness: options.thickness
    });
    return builder.createWallBetween(anchorA, anchorB, height);
}

/**
 * Quick wall section (two pillars + wall between)
 * @param {Vector3} anchorA
 * @param {Vector3} anchorB
 * @param {number} height
 * @param {Object} options - { color, hexWidth, wallThicknessRatio, thickness }
 */
export function createWallSection(anchorA, anchorB, height, options = {}) {
    const builder = new WallBuilder({
        color: options.color || WallColors.stoneMedium,
        hexWidth: options.hexWidth,
        wallThicknessRatio: options.wallThicknessRatio,
        thickness: options.thickness
    });
    return builder.createWallSection(anchorA, anchorB, height);
}
