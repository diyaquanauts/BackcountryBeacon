#!/usr/bin/env node

const async = require('async');
const https = require('https');
const fs = require('fs');
const path = require('path');
const { Command } = require('commander');
const cliProgress = require('cli-progress');
const colors = require('ansi-colors');

// Create new progress bar
const progressBar = new cliProgress.SingleBar({
    format: 'CLI Progress |' + colors.cyan('{bar}') + '| {percentage}% || {value}/{total} Chunks',
    barCompleteChar: '\u2588',
    barIncompleteChar: '\u2591',
    hideCursor: true
});

// Base URL for tile server and base directory for saving tiles
const baseUrl = 'https://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/tile';

// Adjustable parameters for download concurrency
const concurrentTileDownloadLimit = 240; // Number of concurrent tasks for tile download

// Function to parse the cached tile data in the nested JSON format
function parseTilesFromCache(cacheData, maxZoom, processTile) {
    for (const z in cacheData) {
        if (cacheData.hasOwnProperty(z) && z <= maxZoom) {
            const xTiles = cacheData[z];
            for (const x in xTiles) {
                if (xTiles.hasOwnProperty(x)) {
                    const yTiles = xTiles[x];
                    yTiles.forEach(y => {
                        processTile([parseInt(x), parseInt(y), parseInt(z)]);
                    });
                }
            }
        }
    }
}

// Function to estimate the number of tiles and total size
function estimateTilesAndSize(states, cacheBaseDir) {
    let totalTiles = 0;
    let totalSizeMB = 0;

    states.forEach(({ stateName, maxZoom }) => {
        const filePath = path.join(cacheBaseDir, `${stateName}-tiles.json`);

        if (fs.existsSync(filePath)) {
            const cacheData = JSON.parse(fs.readFileSync(filePath, 'utf8'));
            parseTilesFromCache(cacheData, maxZoom, () => {
                totalTiles++;
                totalSizeMB += 20 / 1024; // Approx 20KB per tile
            });
        } else {
            console.error(`No cached tile data found for ${stateName}`);
        }
    });

    return { totalTiles, totalSizeMB };
}

// Function to download a tile
function downloadTile(z, x, y, outputDir) {
    return new Promise((resolve, reject) => {
        const dir = path.join(outputDir, z.toString(), x.toString());
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

// Function to start downloading tiles for all states
function startDownloadingTilesForStates(states, outputDir, cacheBaseDir, callback) {
    let totalTiles = 0;
    let downloadedTiles = 0;
    const tileQueue = [];

    states.forEach(({ stateName, maxZoom }) => {
        const filePath = path.join(cacheBaseDir, `${stateName}-tiles.json`);

        if (fs.existsSync(filePath)) {
            const cacheData = JSON.parse(fs.readFileSync(filePath, 'utf8'));
            parseTilesFromCache(cacheData, maxZoom, (tile) => {
                tileQueue.push(tile);
                totalTiles++;
            });
        }
    });

    // Initialize the progress bar
    progressBar.start(totalTiles, 0);

    async.mapLimit(tileQueue, concurrentTileDownloadLimit, (tile, cb) => {
        const [x, y, z] = tile;
        downloadTile(z, x, y, outputDir).then(() => {
            downloadedTiles++;
            progressBar.update(downloadedTiles);
            cb(null);
        }).catch(cb);
    }, (err) => {
        progressBar.stop();
        if (err) {
            console.error('Error downloading tiles:', err);
            return callback(err);
        } else {
            console.log('All tiles downloaded successfully.');
            return callback();
        }
    });
}

// Command-line interface setup
const program = new Command();

program
    .name('tile-downloader')
    .description('Download map tiles for multiple states')
    .option('-s, --states <states>', 'Comma-separated list of states with their max zoom levels (e.g. California:12,Nevada:14)', (value) => value.split(','))
    .option('-o, --output <directory>', 'Directory where tiles will be saved', './tiles')
    .option('-c, --cache <directory>', 'Directory where cached data is stored', './tile-poly-cache')
    .parse(process.argv);

const options = program.opts();

if (!options.states) {
    program.outputHelp();
    process.exit(1);
} else {
    const states = options.states.map(stateArg => {
        const [stateName, zoom] = stateArg.split(':');
        const maxZoom = parseInt(zoom);

        if (isNaN(maxZoom) || maxZoom < 0 || maxZoom > 16) {
            console.error(`Invalid zoom level for ${stateName}. Please provide a zoom level between 0 and 16.`);
            process.exit(1);
        }

        return { stateName, maxZoom };
    });

    const { totalTiles, totalSizeMB } = estimateTilesAndSize(states, options.cache);

    console.log(`Total tiles to be downloaded: ${totalTiles}`);
    console.log(`Estimated size: ${totalSizeMB.toFixed(2)} MB`);
    console.log(`Output directory: ${options.output}`);

    // Ask user for confirmation before proceeding
    const readline = require('readline').createInterface({
        input: process.stdin,
        output: process.stdout,
    });

    readline.question('Do you want to proceed? (yes/no): ', (answer) => {
        readline.close();
        if (answer.toLowerCase() === 'yes') {
            progressBar.start(totalTiles, 0, {
                speed: "N/A"
            });
            startDownloadingTilesForStates(states, options.output, options.cache, (err) => {
                if (!err) {
                    console.log('Download completed successfully.');
                }
            });
        } else {
            console.log('Download aborted by user.');
        }
    });
}
