#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const { Command } = require('commander');

// Function to parse the cached tile data in the nested JSON format
function parseTilesFromCache(cacheData) {
    const tilesByZoom = {};

    for (const z in cacheData) {
        if (cacheData.hasOwnProperty(z)) {
            const zoomLevel = parseInt(z);
            const xTiles = cacheData[z];
            if (!tilesByZoom[zoomLevel]) {
                tilesByZoom[zoomLevel] = 0;
            }

            for (const x in xTiles) {
                if (xTiles.hasOwnProperty(x)) {
                    const yTiles = xTiles[x];
                    tilesByZoom[zoomLevel] += yTiles.length;
                }
            }
        }
    }

    return tilesByZoom;
}

// Function to generate the Markdown table
function generateMarkdownTable(states, cacheBaseDir, outputFile) {
    let markdownContent = `# Tile Summary for US States\n\n`;
    markdownContent += `| State | Zoom Level | Number of Tiles | Estimated Size (MB) |\n`;
    markdownContent += `|-------|------------|-----------------|---------------------|\n`;

    states.forEach((stateName) => {
        const filePath = path.join(cacheBaseDir, `${stateName}-tiles.json`);
        if (fs.existsSync(filePath)) {
            const cacheData = JSON.parse(fs.readFileSync(filePath, 'utf8'));
            const tilesByZoom = parseTilesFromCache(cacheData);

            for (const zoom in tilesByZoom) {
                if (zoom <= 6) continue; // Skip zoom levels 1 through 6
                const numTiles = tilesByZoom[zoom];
                const estimatedSizeMB = (numTiles * 20) / 1024; // Approx 20KB per tile
                markdownContent += `| ${stateName} | ${zoom} | ${numTiles} | ${estimatedSizeMB.toFixed(2)} |\n`;
            }
        } else {
            console.error(`No cached tile data found for ${stateName}`);
        }
    });

    fs.writeFileSync(outputFile, markdownContent);
    console.log(`Markdown file generated: ${outputFile}`);
}

// Command-line interface setup
const program = new Command();

program
    .name('us-tile-summary')
    .description('Generate a markdown table summarizing the number of tiles and their estimated size for each state and zoom level')
    .option('-c, --cache <directory>', 'Directory where cached data is stored', './tile-poly-cache')
    .option('-o, --output <file>', 'Output Markdown file', 'us_tile_summary.md')
    .parse(process.argv);

const options = program.opts();

const cacheBaseDir = options.cache;
const outputFile = options.output;

// Read the list of state JSON files from the cache directory
const stateFiles = fs.readdirSync(cacheBaseDir).filter(file => file.endsWith('-tiles.json'));

// Extract state names from file names
const states = stateFiles.map(file => file.replace('-tiles.json', ''));

// Generate the Markdown table
generateMarkdownTable(states, cacheBaseDir, outputFile);
