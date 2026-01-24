/**
 * Floor Builder Utility
 * Version: 1.1
 * Date: 2026-01-19
 *
 * Creates hex-tiled floors using instanced meshes for performance.
 * Supports per-tile color variation and integrates with AdvancedCharacterShader.
 *
 * HEX GRID COORDINATE SYSTEM:
 *   Uses axial coordinates (q, r) for hex positioning.
 *   Pointy-top orientation by default.
 *
 *   Visual layout:
 *        (0,-1)  (1,-1)
 *     (-1,0)  (0,0)  (1,0)
 *        (-1,1)  (0,1)
 *
 * Usage:
 *   import { FloorBuilder, FloorColors } from './FloorBuilder.js';
 *
 *   const builder = new FloorBuilder({ hexWidth: 48, hexHeight: 32 });
 *
 *   // Option 1: Procedural hex (no OBJ needed)
 *   const floor = builder.createProceduralFloor(radius, { color: FloorColors.stone });
 *
 *   // Option 2: Using your hex OBJ
 *   const floor = builder.createFloorFromOBJ(hexModel, radius, options);
 *
 *   scene.add(floor);
 */

import * as THREE from 'three';
import { createAdvancedCharacterShader, startShaderAnimation } from './AdvancedCharacterShader.js';

// ============================================================================
// COLOR PRESETS
// ============================================================================

export const FloorColors = {
    // Stone variants
    stone: 0x808080,
    stoneDark: 0x606060,
    stoneLight: 0xA0A0A0,
    cobblestone: 0x707070,

    // Wood variants
    woodLight: 0xC4A484,
    woodMedium: 0x8B7355,
    woodDark: 0x5C4033,

    // Terrain
    grass: 0x4A7C3A,
    grassDark: 0x3A5C2A,
    dirt: 0x8B6914,
    sand: 0xD4B896,
    snow: 0xE8E8E8,
    water: 0x4A90D9,
    waterDeep: 0x2A5080,

    // Dungeon
    dungeonFloor: 0x4A4A4A,
    dungeonWet: 0x3A4A5A,
    dungeonMoss: 0x4A5A4A,

    // Special
    lava: 0xFF4500,
    ice: 0xADD8E6,
    marble: 0xF0F0F0,
    obsidian: 0x1A1A2E
};

// ============================================================================
// HEX GRID MATH
// ============================================================================

/**
 * Convert axial hex coordinates (q, r) to world position (x, z)
 * Pointy-top orientation
 */
function axialToWorld(q, r, hexWidth, hexHeight) {
    const x = hexWidth * (Math.sqrt(3) * q + Math.sqrt(3) / 2 * r);
    const z = hexHeight * (3 / 2 * r);
    return { x, z };
}

/**
 * Convert world position to axial coordinates (approximate, for picking)
 */
function worldToAxial(x, z, hexWidth, hexHeight) {
    const q = (Math.sqrt(3) / 3 * x - 1 / 3 * z) / hexWidth;
    const r = (2 / 3 * z) / hexHeight;
    return { q: Math.round(q), r: Math.round(r) };
}

/**
 * Get all hex coordinates within a radius (hexagonal shape)
 */
function getHexesInRadius(radius) {
    const hexes = [];
    for (let q = -radius; q <= radius; q++) {
        const r1 = Math.max(-radius, -q - radius);
        const r2 = Math.min(radius, -q + radius);
        for (let r = r1; r <= r2; r++) {
            hexes.push({ q, r });
        }
    }
    return hexes;
}

/**
 * Get all hex coordinates in a rectangular area
 */
function getHexesInRect(width, height) {
    const hexes = [];
    for (let r = 0; r < height; r++) {
        const rOffset = Math.floor(r / 2);
        for (let q = -rOffset; q < width - rOffset; q++) {
            hexes.push({ q, r });
        }
    }
    return hexes;
}

// ============================================================================
// TEXTURE HELPERS
// ============================================================================

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
// PROCEDURAL HEX GEOMETRY
// ============================================================================

/**
 * Create a flat hexagon geometry (pointy-top)
 */
function createHexGeometry(width, height, thickness = 1) {
    const shape = new THREE.Shape();

    // Pointy-top hex vertices
    const angles = [
        Math.PI / 6,      // 30°
        Math.PI / 2,      // 90°
        5 * Math.PI / 6,  // 150°
        7 * Math.PI / 6,  // 210°
        3 * Math.PI / 2,  // 270°
        11 * Math.PI / 6  // 330°
    ];

    const radiusX = width / 2;
    const radiusZ = height / 2;

    // Start at first vertex
    shape.moveTo(
        Math.cos(angles[0]) * radiusX,
        Math.sin(angles[0]) * radiusZ
    );

    // Draw to remaining vertices
    for (let i = 1; i < 6; i++) {
        shape.lineTo(
            Math.cos(angles[i]) * radiusX,
            Math.sin(angles[i]) * radiusZ
        );
    }
    shape.closePath();

    // Extrude to give thickness
    const extrudeSettings = {
        depth: thickness,
        bevelEnabled: false
    };

    const geometry = new THREE.ExtrudeGeometry(shape, extrudeSettings);

    // Rotate so it lies flat (Y-up)
    geometry.rotateX(-Math.PI / 2);

    // Center vertically
    geometry.translate(0, thickness / 2, 0);

    return geometry;
}

// ============================================================================
// FLOOR BUILDER CLASS
// ============================================================================

export class FloorBuilder {
    /**
     * @param {Object} options
     * @param {number} options.hexWidth - Width of hex (point to point)
     * @param {number} options.hexHeight - Height of hex (flat to flat), defaults to hexWidth * 0.866
     * @param {number} options.tileThickness - Thickness of floor tiles
     * @param {number} options.shadingVariance - Auto-shading amount (default: 0.12)
     * @param {number} options.colorVariance - Per-tile color variation (default: 0.05)
     */
    constructor(options = {}) {
        this.hexWidth = options.hexWidth || 48;
        this.hexHeight = options.hexHeight || this.hexWidth * 0.866;
        this.tileThickness = options.tileThickness || 2;
        this.shadingVariance = options.shadingVariance ?? 0.12;
        this.colorVariance = options.colorVariance ?? 0.05;  // 5% random color variation

        // Material cache
        this.materialCache = new Map();
        this.animatedShaders = [];
    }

    /**
     * Get or create material for color
     */
    getMaterial(color) {
        const colorHex = typeof color === 'number' ? color : color.getHex();

        if (this.materialCache.has(colorHex)) {
            return this.materialCache.get(colorHex);
        }

        const texture = createSolidColorTexture(colorHex);
        const material = createAdvancedCharacterShader(texture, {
            skinColor: colorHex,
            shadingVariance: this.shadingVariance,
            selected: 0.0
        });

        this.materialCache.set(colorHex, material);
        this.animatedShaders.push(material);

        return material;
    }

    /**
     * Create a floor using procedural hex geometry (no OBJ needed)
     * Uses InstancedMesh for performance
     *
     * @param {number} radius - Hex radius (0 = just center, 1 = center + ring, etc.)
     * @param {Object} options
     * @param {number} options.color - Base floor color
     * @param {boolean} options.addVariation - Add per-tile color variation
     * @param {Function|Object} options.elevation - Elevation function(q,r) or object {"q,r": y}
     * @param {Function|Object} options.scale - Scale function(q,r) or object {"q,r": y or {x,y,z}}
     */
    createProceduralFloor(radius, options = {}) {
        const color = options.color || FloorColors.stone;
        const addVariation = options.addVariation ?? true;
        const elevation = options.elevation || null;
        const scale = options.scale || null;

        const hexes = getHexesInRadius(radius);
        const hexGeometry = createHexGeometry(this.hexWidth, this.hexHeight, this.tileThickness);
        const material = this.getMaterial(color);

        const instancedMesh = new THREE.InstancedMesh(hexGeometry, material, hexes.length);

        // Set up instance colors if variation enabled
        if (addVariation) {
            const colors = new Float32Array(hexes.length * 3);
            const baseColor = new THREE.Color(color);

            for (let i = 0; i < hexes.length; i++) {
                // Random variation
                const variation = 1 + (Math.random() - 0.5) * 2 * this.colorVariance;
                colors[i * 3] = baseColor.r * variation;
                colors[i * 3 + 1] = baseColor.g * variation;
                colors[i * 3 + 2] = baseColor.b * variation;
            }

            instancedMesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
        }

        // Position each hex
        const matrix = new THREE.Matrix4();
        const position = new THREE.Vector3();
        const quaternion = new THREE.Quaternion();
        const scaleVec = new THREE.Vector3(1, 1, 1);

        hexes.forEach((hex, index) => {
            const pos = axialToWorld(hex.q, hex.r, this.hexWidth, this.hexHeight);

            // Get elevation
            let y = 0;
            if (elevation) {
                if (typeof elevation === 'function') {
                    y = elevation(hex.q, hex.r) || 0;
                } else if (typeof elevation === 'object') {
                    y = elevation[`${hex.q},${hex.r}`] || 0;
                }
            }

            // Get scale
            scaleVec.set(1, 1, 1);
            if (scale) {
                let s;
                if (typeof scale === 'function') {
                    s = scale(hex.q, hex.r);
                } else if (typeof scale === 'object') {
                    s = scale[`${hex.q},${hex.r}`];
                }
                if (s) {
                    if (typeof s === 'number') {
                        scaleVec.set(1, s, 1);  // Just Y scale
                    } else {
                        scaleVec.set(s.x ?? 1, s.y ?? 1, s.z ?? 1);
                    }
                }
            }

            position.set(pos.x, y, pos.z);
            matrix.compose(position, quaternion, scaleVec);
            instancedMesh.setMatrixAt(index, matrix);
        });

        instancedMesh.instanceMatrix.needsUpdate = true;

        instancedMesh.userData.type = 'hexFloor';
        instancedMesh.userData.hexes = hexes;
        instancedMesh.userData.radius = radius;

        return instancedMesh;
    }

    /**
     * Create a floor using your custom hex OBJ model
     *
     * @param {THREE.BufferGeometry} hexGeometry - Geometry from your hex OBJ
     * @param {number} radius - Hex radius
     * @param {Object} options
     * @param {Function|Map} options.elevation - Elevation function(q,r) or Map of "q,r" -> y
     * @param {Function|Map} options.scale - Scale function(q,r) or Map of "q,r" -> {x,y,z}
     */
    createFloorFromGeometry(hexGeometry, radius, options = {}) {
        const color = options.color || FloorColors.stone;
        const addVariation = options.addVariation ?? true;
        const elevation = options.elevation || null;
        const scale = options.scale || null;

        const hexes = getHexesInRadius(radius);
        const material = this.getMaterial(color);

        const instancedMesh = new THREE.InstancedMesh(hexGeometry, material, hexes.length);

        if (addVariation) {
            const colors = new Float32Array(hexes.length * 3);
            const baseColor = new THREE.Color(color);

            for (let i = 0; i < hexes.length; i++) {
                const variation = 1 + (Math.random() - 0.5) * 2 * this.colorVariance;
                colors[i * 3] = baseColor.r * variation;
                colors[i * 3 + 1] = baseColor.g * variation;
                colors[i * 3 + 2] = baseColor.b * variation;
            }

            instancedMesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
        }

        const matrix = new THREE.Matrix4();
        const position = new THREE.Vector3();
        const quaternion = new THREE.Quaternion();
        const scaleVec = new THREE.Vector3(1, 1, 1);

        hexes.forEach((hex, index) => {
            const pos = axialToWorld(hex.q, hex.r, this.hexWidth, this.hexHeight);

            // Get elevation
            let y = 0;
            if (elevation) {
                if (typeof elevation === 'function') {
                    y = elevation(hex.q, hex.r) || 0;
                } else if (elevation instanceof Map) {
                    y = elevation.get(`${hex.q},${hex.r}`) || 0;
                } else if (typeof elevation === 'object') {
                    y = elevation[`${hex.q},${hex.r}`] || 0;
                }
            }

            // Get scale
            scaleVec.set(1, 1, 1);
            if (scale) {
                let s;
                if (typeof scale === 'function') {
                    s = scale(hex.q, hex.r);
                } else if (scale instanceof Map) {
                    s = scale.get(`${hex.q},${hex.r}`);
                } else if (typeof scale === 'object') {
                    s = scale[`${hex.q},${hex.r}`];
                }
                if (s) {
                    if (typeof s === 'number') {
                        scaleVec.set(1, s, 1);  // Just Y scale
                    } else {
                        scaleVec.set(s.x ?? 1, s.y ?? 1, s.z ?? 1);
                    }
                }
            }

            position.set(pos.x, y, pos.z);
            matrix.compose(position, quaternion, scaleVec);
            instancedMesh.setMatrixAt(index, matrix);
        });

        instancedMesh.instanceMatrix.needsUpdate = true;

        instancedMesh.userData.type = 'hexFloor';
        instancedMesh.userData.hexes = hexes;

        return instancedMesh;
    }

    /**
     * Create a floor with explicit elevation map
     *
     * @param {THREE.BufferGeometry} hexGeometry - Geometry from your hex OBJ
     * @param {Array} elevationMap - Array of { q, r, y, scale?, color? }
     * @param {Object} options
     */
    createFloorWithElevation(hexGeometry, elevationMap, options = {}) {
        const defaultColor = options.color || FloorColors.stone;
        const addVariation = options.addVariation ?? true;

        // Build lookup maps
        const elevationLookup = {};
        const scaleLookup = {};
        const colorLookup = {};

        elevationMap.forEach(tile => {
            const key = `${tile.q},${tile.r}`;
            if (tile.y !== undefined) elevationLookup[key] = tile.y;
            if (tile.scale !== undefined) scaleLookup[key] = tile.scale;
            if (tile.color !== undefined) colorLookup[key] = tile.color;
        });

        // Get all hex positions from the map
        const hexes = elevationMap.map(t => ({ q: t.q, r: t.r }));

        // Group by color for efficient rendering
        const colorGroups = new Map();
        hexes.forEach(hex => {
            const key = `${hex.q},${hex.r}`;
            const color = colorLookup[key] || defaultColor;
            if (!colorGroups.has(color)) {
                colorGroups.set(color, []);
            }
            colorGroups.get(color).push(hex);
        });

        const floorGroup = new THREE.Group();

        colorGroups.forEach((groupHexes, colorHex) => {
            const material = this.getMaterial(colorHex);
            const instancedMesh = new THREE.InstancedMesh(hexGeometry, material, groupHexes.length);

            if (addVariation) {
                const colors = new Float32Array(groupHexes.length * 3);
                const baseColor = new THREE.Color(colorHex);

                for (let i = 0; i < groupHexes.length; i++) {
                    const variation = 1 + (Math.random() - 0.5) * 2 * this.colorVariance;
                    colors[i * 3] = baseColor.r * variation;
                    colors[i * 3 + 1] = baseColor.g * variation;
                    colors[i * 3 + 2] = baseColor.b * variation;
                }

                instancedMesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
            }

            const matrix = new THREE.Matrix4();
            const position = new THREE.Vector3();
            const quaternion = new THREE.Quaternion();
            const scaleVec = new THREE.Vector3(1, 1, 1);

            groupHexes.forEach((hex, index) => {
                const pos = axialToWorld(hex.q, hex.r, this.hexWidth, this.hexHeight);
                const key = `${hex.q},${hex.r}`;

                const y = elevationLookup[key] || 0;
                const s = scaleLookup[key];

                scaleVec.set(1, 1, 1);
                if (s) {
                    if (typeof s === 'number') {
                        scaleVec.set(1, s, 1);
                    } else {
                        scaleVec.set(s.x ?? 1, s.y ?? 1, s.z ?? 1);
                    }
                }

                position.set(pos.x, y, pos.z);
                matrix.compose(position, quaternion, scaleVec);
                instancedMesh.setMatrixAt(index, matrix);
            });

            instancedMesh.instanceMatrix.needsUpdate = true;
            instancedMesh.userData.hexes = groupHexes;
            floorGroup.add(instancedMesh);
        });

        floorGroup.userData.type = 'elevatedHexFloor';
        floorGroup.userData.elevationMap = elevationMap;

        return floorGroup;
    }

    /**
     * Create a rectangular floor area
     *
     * @param {number} width - Width in hexes
     * @param {number} height - Height in hexes
     * @param {Object} options
     */
    createRectangularFloor(width, height, options = {}) {
        const color = options.color || FloorColors.stone;
        const addVariation = options.addVariation ?? true;

        const hexes = getHexesInRect(width, height);
        const hexGeometry = createHexGeometry(this.hexWidth, this.hexHeight, this.tileThickness);
        const material = this.getMaterial(color);

        const instancedMesh = new THREE.InstancedMesh(hexGeometry, material, hexes.length);

        if (addVariation) {
            const colors = new Float32Array(hexes.length * 3);
            const baseColor = new THREE.Color(color);

            for (let i = 0; i < hexes.length; i++) {
                const variation = 1 + (Math.random() - 0.5) * 2 * this.colorVariance;
                colors[i * 3] = baseColor.r * variation;
                colors[i * 3 + 1] = baseColor.g * variation;
                colors[i * 3 + 2] = baseColor.b * variation;
            }

            instancedMesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
        }

        const matrix = new THREE.Matrix4();
        hexes.forEach((hex, index) => {
            const pos = axialToWorld(hex.q, hex.r, this.hexWidth, this.hexHeight);
            matrix.setPosition(pos.x, 0, pos.z);
            instancedMesh.setMatrixAt(index, matrix);
        });

        instancedMesh.instanceMatrix.needsUpdate = true;

        instancedMesh.userData.type = 'hexFloor';
        instancedMesh.userData.hexes = hexes;

        return instancedMesh;
    }

    /**
     * Create a floor with mixed terrain (multiple colors)
     *
     * @param {Array} terrainMap - Array of { q, r, color } objects
     * @param {Object} options
     */
    createMixedTerrainFloor(terrainMap, options = {}) {
        const addVariation = options.addVariation ?? true;

        // Group hexes by color for efficient instancing
        const colorGroups = new Map();

        terrainMap.forEach(tile => {
            const colorHex = tile.color || FloorColors.stone;
            if (!colorGroups.has(colorHex)) {
                colorGroups.set(colorHex, []);
            }
            colorGroups.get(colorHex).push({ q: tile.q, r: tile.r });
        });

        const floorGroup = new THREE.Group();
        const hexGeometry = createHexGeometry(this.hexWidth, this.hexHeight, this.tileThickness);

        colorGroups.forEach((hexes, colorHex) => {
            const material = this.getMaterial(colorHex);
            const instancedMesh = new THREE.InstancedMesh(hexGeometry, material, hexes.length);

            if (addVariation) {
                const colors = new Float32Array(hexes.length * 3);
                const baseColor = new THREE.Color(colorHex);

                for (let i = 0; i < hexes.length; i++) {
                    const variation = 1 + (Math.random() - 0.5) * 2 * this.colorVariance;
                    colors[i * 3] = baseColor.r * variation;
                    colors[i * 3 + 1] = baseColor.g * variation;
                    colors[i * 3 + 2] = baseColor.b * variation;
                }

                instancedMesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
            }

            const matrix = new THREE.Matrix4();
            hexes.forEach((hex, index) => {
                const pos = axialToWorld(hex.q, hex.r, this.hexWidth, this.hexHeight);
                matrix.setPosition(pos.x, 0, pos.z);
                instancedMesh.setMatrixAt(index, matrix);
            });

            instancedMesh.instanceMatrix.needsUpdate = true;
            floorGroup.add(instancedMesh);
        });

        floorGroup.userData.type = 'mixedTerrainFloor';
        floorGroup.userData.terrainMap = terrainMap;

        return floorGroup;
    }

    /**
     * Highlight a specific hex tile (for hover/selection)
     *
     * @param {THREE.InstancedMesh} floor - The floor mesh
     * @param {number} q - Hex Q coordinate
     * @param {number} r - Hex R coordinate
     * @param {number} color - Highlight color
     */
    highlightHex(floor, q, r, color = 0xFFFF00) {
        const hexes = floor.userData.hexes;
        const index = hexes.findIndex(h => h.q === q && h.r === r);

        if (index === -1) return;

        if (!floor.instanceColor) {
            // Initialize instance colors if not present
            const colors = new Float32Array(hexes.length * 3);
            for (let i = 0; i < hexes.length; i++) {
                colors[i * 3] = 1;
                colors[i * 3 + 1] = 1;
                colors[i * 3 + 2] = 1;
            }
            floor.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
        }

        const c = new THREE.Color(color);
        floor.instanceColor.setXYZ(index, c.r, c.g, c.b);
        floor.instanceColor.needsUpdate = true;
    }

    /**
     * Get hex coordinates from world position (for mouse picking)
     */
    worldToHex(x, z) {
        return worldToAxial(x, z, this.hexWidth, this.hexHeight);
    }

    /**
     * Get world position from hex coordinates
     */
    hexToWorld(q, r) {
        return axialToWorld(q, r, this.hexWidth, this.hexHeight);
    }

    /**
     * Start shader animations
     */
    startAnimations() {
        const stopFunctions = this.animatedShaders.map(shader =>
            startShaderAnimation(shader)
        );
        return () => stopFunctions.forEach(stop => stop());
    }

    /**
     * Clean up
     */
    dispose() {
        this.materialCache.forEach(material => material.dispose());
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
 */
export function cubeToWorld(cube, hexSize) {
    const SQRT_3 = Math.sqrt(3);
    return {
        x: hexSize * SQRT_3 * (cube.x + cube.z / 2),
        z: hexSize * (3 / 2) * cube.z
    };
}

/**
 * Convert cube coordinates to our axial format
 * Cube {x, y, z} -> Axial {q, r}
 * Note: cube.y is derived (y = -x - z), so we use x and z
 */
export function cubeToAxial(cube) {
    return { q: cube.x, r: cube.z };
}

/**
 * Generate all hex positions in a rectangular room
 * Uses offset coordinates internally, converts to cube for output
 *
 * @param {number} width - Room width in hexes
 * @param {number} height - Room height in hexes
 * @returns {Array} Array of cube coordinates {x, y, z}
 */
function generateRoomHexes(width, height) {
    const hexes = [];

    for (let row = 0; row < height; row++) {
        const rowOffset = Math.floor(row / 2);
        for (let col = 0; col < width; col++) {
            // Offset to cube conversion (odd-r offset)
            const x = col - rowOffset;
            const z = row;
            const y = -x - z;
            hexes.push({ x, y, z });
        }
    }

    return hexes;
}

/**
 * Create floor from rpg-api Room data
 *
 * API Room format:
 *   { width: number, height: number, walls?: Wall[], entities?: {} }
 *
 * @param {Object} apiRoom - Room data from API
 * @param {Object} options
 * @param {number} options.hexSize - Hex size for coordinate conversion
 * @param {THREE.BufferGeometry} options.hexGeometry - Optional custom hex geometry
 * @param {number} options.color - Base floor color
 * @param {boolean} options.markWallHexes - Darken hexes where walls are
 * @returns {THREE.Object3D} Floor mesh or group
 */
export function createFloorFromAPI(apiRoom, options = {}) {
    const hexSize = options.hexSize || 1;
    const baseColor = options.color || FloorColors.stone;
    const markWallHexes = options.markWallHexes ?? true;

    // Calculate hex dimensions from hexSize
    const hexWidth = hexSize * Math.sqrt(3);
    const hexHeight = hexSize * 2;

    const builder = new FloorBuilder({
        hexWidth: hexWidth,
        hexHeight: hexHeight * 0.75,  // Hex row spacing
        tileThickness: options.thickness || 2,
        colorVariance: options.colorVariance ?? 0.05
    });

    // Generate all hex positions for the room
    const roomHexes = generateRoomHexes(apiRoom.width, apiRoom.height);

    // Build wall lookup for marking wall hexes
    const wallHexSet = new Set();
    if (markWallHexes && apiRoom.walls) {
        apiRoom.walls.forEach(wall => {
            // Get all hexes along wall path
            const hexLine = getHexLineCube(wall.start, wall.end);
            hexLine.forEach(hex => {
                wallHexSet.add(`${hex.x},${hex.z}`);
            });
        });
    }

    // Build elevation/color map
    const terrainMap = roomHexes.map(cube => {
        const axial = cubeToAxial(cube);
        const isWallHex = wallHexSet.has(`${cube.x},${cube.z}`);

        return {
            q: axial.q,
            r: axial.r,
            y: 0,
            color: isWallHex ? FloorColors.stoneDark : baseColor
        };
    });

    // Use custom geometry if provided, otherwise procedural
    if (options.hexGeometry) {
        return builder.createFloorWithElevation(options.hexGeometry, terrainMap, {
            addVariation: true
        });
    } else {
        return builder.createMixedTerrainFloor(terrainMap, {
            addVariation: true
        });
    }
}

/**
 * Get all hexes along a line (cube coordinates)
 * Used for wall path calculation
 */
function getHexLineCube(start, end) {
    const N = Math.max(
        Math.abs(end.x - start.x),
        Math.abs(end.y - start.y),
        Math.abs(end.z - start.z)
    );

    const results = [];

    for (let i = 0; i <= N; i++) {
        const t = N === 0 ? 0 : i / N;
        const x = Math.round(start.x + (end.x - start.x) * t);
        const z = Math.round(start.z + (end.z - start.z) * t);
        const y = -x - z;
        results.push({ x, y, z });
    }

    return results;
}

/**
 * Create complete room visualization from API data
 * Returns both floor and walls as a group
 *
 * @param {Object} apiRoom - Room from API
 * @param {Object} options
 */
export function createRoomFromAPI(apiRoom, options = {}) {
    const roomGroup = new THREE.Group();

    // Create floor
    const floor = createFloorFromAPI(apiRoom, {
        hexSize: options.hexSize,
        color: options.floorColor || FloorColors.stone,
        thickness: options.floorThickness,
        colorVariance: options.colorVariance,
        markWallHexes: options.markWallHexes,
        hexGeometry: options.hexGeometry
    });
    floor.name = 'floor';
    roomGroup.add(floor);

    // Import WallBuilder functions if walls exist
    if (apiRoom.walls && apiRoom.walls.length > 0) {
        // Note: Consumer needs to also import from WallBuilder.js
        // and call createWallsFromAPI separately, or we do it here
        // For now, just return the floor - walls handled separately
    }

    roomGroup.userData.type = 'apiRoom';
    roomGroup.userData.width = apiRoom.width;
    roomGroup.userData.height = apiRoom.height;

    return roomGroup;
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * Quick hex floor creation
 */
export function createHexFloor(radius, options = {}) {
    const builder = new FloorBuilder({
        hexWidth: options.hexWidth,
        hexHeight: options.hexHeight,
        tileThickness: options.thickness
    });
    return builder.createProceduralFloor(radius, options);
}

/**
 * Quick rectangular floor
 */
export function createRectFloor(width, height, options = {}) {
    const builder = new FloorBuilder({
        hexWidth: options.hexWidth,
        hexHeight: options.hexHeight,
        tileThickness: options.thickness
    });
    return builder.createRectangularFloor(width, height, options);
}

// ============================================================================
// EXPORTS FOR HEX MATH (useful for game logic)
// ============================================================================

export { axialToWorld, worldToAxial, getHexesInRadius, getHexesInRect };
