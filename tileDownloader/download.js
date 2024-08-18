const async = require('async');
const https = require('https');
const fs = require('fs');
const path = require('path');

// Base URL for tile server and base directory for saving tiles
const baseUrl = 'https://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/tile';
const baseDir = './allTiles'; // Set to your external SSD path
const cacheBaseDir = 'tile-poly-cache'; // Directory where cached data is stored

// Adjustable parameters for download concurrency
const concurrentTileDownloadLimit = 240; // Number of concurrent tasks for tile download

// Function to parse the cached tile data in the nested JSON format
function parseTilesFromCache(cacheData) {
    const tiles = [];

    // Iterate over the zoom levels
    for (const z in cacheData) {
        if (cacheData.hasOwnProperty(z)) {
            const xTiles = cacheData[z];

            // Iterate over the x coordinates
            for (const x in xTiles) {
                if (xTiles.hasOwnProperty(x)) {
                    const yTiles = xTiles[x];

                    // Iterate over the y coordinates
                    yTiles.forEach(y => {
                        tiles.push([parseInt(x), parseInt(y), parseInt(z)]);
                    });
                }
            }
        }
    }

    return tiles;
}

// Function to download a tile
function downloadTile(z, x, y, stateName) {
    return new Promise((resolve, reject) => {
        const stateDir = path.join(baseDir, stateName);
        const dir = path.join(stateDir, z.toString(), x.toString());
        const filePath = path.join(dir, `${y}.png`);
        if (!fs.existsSync(filePath)) {
            const url = `${baseUrl}/${z}/${y}/${x}`;

            if (!fs.existsSync(dir)) {
                fs.mkdirSync(dir, { recursive: true });
            }

            const file = fs.createWriteStream(filePath);
            https.get(url, function(response) {
                response.pipe(file);
                file.on('finish', function() {
                    file.close();
                    resolve();
                });
            }).on('error', function(err) {
                fs.unlink(filePath, function(unlinkErr) {
                    if (unlinkErr) {
                        console.error(`Error removing incomplete file: ${unlinkErr.message}`);
                    }
                });
                console.error(`Error downloading tile ${z}/${x}/${y}: ${err.message}`);
                reject(err);
            });
        } else {
            resolve();
        }
    });
}

// Function to start downloading tiles for a specific state
function startDownloadingTilesForState(stateName, tiles, callback) {
    let downloadedTiles = 0;
    const totalTiles = tiles.length;

    async.mapLimit(tiles, concurrentTileDownloadLimit, (tile, cb) => {
        const [x, y, z] = tile;
        downloadTile(z, x, y, stateName).then(() => {
            downloadedTiles++;
            if (downloadedTiles % concurrentTileDownloadLimit === 0) {
                console.log(`Downloaded tile ${downloadedTiles}/${totalTiles} for ${stateName}`);
            }
            cb(null);
        }).catch(cb);
    }, (err) => {
        console.log(`Downloaded tile ${downloadedTiles}/${totalTiles}  for ${stateName}`);
        if (err) {
            console.error(`Error downloading tiles for ${stateName}:`, err);
            return callback(err);
        } else {
            console.log(`All tiles downloaded for ${stateName}`);
            return callback();
        }
    });
}

// Function to process all states sequentially
function processAllStates() {
    const stateFiles = fs.readdirSync(cacheBaseDir);

    async.eachSeries(stateFiles, (file, cb) => {
        const stateName = file.replace('-tiles.json', '');
        const filePath = path.join(cacheBaseDir, file);

        try {
            const cacheData = JSON.parse(fs.readFileSync(filePath, 'utf8'));
            const tiles = parseTilesFromCache(cacheData);
            console.log(`Starting download for ${stateName} with ${tiles.length} tiles`);

            startDownloadingTilesForState(stateName, tiles, cb);
        } catch (err) {
            console.error(`Error parsing JSON for ${stateName}:, err.message`);
            cb(err);
        }
    }, (err) => {
        if (err) {
            console.error('Error processing states:', err);
        } else {
            console.log('All states processed successfully.');
        }
    });
}

// Start processing all states
processAllStates();
