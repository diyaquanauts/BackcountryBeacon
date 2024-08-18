const turf = require('@turf/turf');
const tilebelt = require('@mapbox/tilebelt');
const fs = require('fs');
const path = require('path');

// Load state boundary data from GeoJSON
let statesData;
try {
    const data = fs.readFileSync('us-state-boundaries.json', 'utf8');
    statesData = JSON.parse(data);
} catch (err) {
    console.error('Error loading state boundaries:', err);
    process.exit(1);
}

// Base directory for tile cache
const cacheBaseDir = 'tile-poly-cache-14';

// Create cache directory if it doesn't exist
if (!fs.existsSync(cacheBaseDir)) {
    fs.mkdirSync(cacheBaseDir, { recursive: true });
}

// Define zoom levels to process
const manualCheckMaxZoom = 9;
const maxZoom = 14; // Maximum zoom level to generate child tiles

// Array of state names to process
const stateNames = statesData.map(state => state.name); // Extracting all state names from the GeoJSON data

// Function to get GeoJSON for a state by name
function getGeoJSONForState(stateName) {
    const state = statesData.find(s => s.name.toLowerCase() === stateName.toLowerCase());
    const geojson = state ? state.st_asgeojson : null;
    return geojson;
}

// Function to calculate tiles for a polygon at a specific zoom level
function calculateTilesForZoom(polygon, zoomLevel) {
    const bbox = turf.bbox(polygon);
    const topLeftTile = tilebelt.pointToTile(bbox[0], bbox[3], zoomLevel);
    const bottomRightTile = tilebelt.pointToTile(bbox[2], bbox[1], zoomLevel);

    const tiles = [];

    for (let x = topLeftTile[0]; x <= bottomRightTile[0]; x++) {
        for (let y = topLeftTile[1]; y <= bottomRightTile[1]; y++) {
            const tilePolygon = turf.polygon([tilebelt.tileToGeoJSON([x, y, zoomLevel]).coordinates[0]]);
            if (turf.intersect(tilePolygon, polygon)) {
                tiles.push([x, y, zoomLevel]);
            }
        }
    }

    return tiles;
}

// Function to generate child tiles from a base tile at a lower zoom level
function generateChildTiles(x, y, baseZoom, maxZoom) {
    const tiles = [];
    for (let z = baseZoom + 1; z <= maxZoom; z++) {
        const scale = Math.pow(2, z - baseZoom);
        const minX = x * scale;
        const minY = y * scale;
        const maxX = (x + 1) * scale - 1;
        const maxY = (y + 1) * scale - 1;

        for (let tx = minX; tx <= maxX; tx++) {
            for (let ty = minY; ty <= maxY; ty++) {
                tiles.push([tx, ty, z]);
            }
        }
    }
    return tiles;
}

// Function to store tile cache in JSON format with x values on the same line
function storeTileCache(stateName, tileData) {
    const cache = {};

    tileData.forEach(([x, y, z]) => {
        if (!cache[z]) {
            cache[z] = {};
        }
        if (!cache[z][y]) {
            cache[z][y] = [];
        }
        cache[z][y].push(x);
    });

    const cacheFilePath = path.join(cacheBaseDir, `${stateName}-tiles.json`);

    // Use JSON.stringify to create a properly formatted JSON string
    const formattedCache = JSON.stringify(cache, null, 2); // Pretty print with 2-space indentation

    try {
        fs.writeFileSync(cacheFilePath, formattedCache);
        console.log(`Tile cache stored for ${stateName} at ${cacheFilePath}`);
    } catch (err) {
        console.error(`Error saving tile cache for ${stateName}:`, err);
    }
}

// Main function to process a state and generate its tile cache
function processState(stateName) {
    const stateGeoJSON = getGeoJSONForState(stateName);
    const simplifiedPolygon = turf.simplify(stateGeoJSON, { tolerance: 0.01, highQuality: true });
    if (!stateGeoJSON) {
        console.error(`State "${stateName}" not found in the dataset.`);
        return;
    }

    console.log(`Processing state: ${stateName}`);

    const tiles = [];

    // Manually check tiles for zoom levels 0 to manualCheckMaxZoom (0 to 10)
    for (let z = 0; z <= manualCheckMaxZoom; z++) {
        console.log(`Calculating tiles for zoom level ${z}`);
        const baseTiles = calculateTilesForZoom(simplifiedPolygon, z);
        tiles.push(...baseTiles);
    }

    // Generate child tiles for zoom levels 11 to maxZoom
    console.log(`Generating child tiles for zoom levels ${manualCheckMaxZoom + 1} to ${maxZoom}`);
    tiles.forEach(([x, y, z]) => {
        if (z === manualCheckMaxZoom) {
            const childTiles = generateChildTiles(x, y, z, maxZoom);
            tiles.push(...childTiles);
        }
    });

    storeTileCache(stateName, tiles);
}

// Process all states
stateNames.forEach(stateName => {
    processState(stateName);
});
