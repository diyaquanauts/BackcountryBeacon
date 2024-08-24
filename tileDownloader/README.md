## High-Level Overview

The code in this folder is responsible for:

-   **Tile Calculation:** Determining which map tiles are required for specific geographic regions and zoom levels. This is done by analyzing state boundaries and generating a list of necessary tiles based on user-defined zoom levels.
    
-   **Tile Downloading:** Fetching the calculated tiles from an online map server. The tiles are downloaded in parallel to optimize the process, with progress tracking to ensure efficient and reliable downloading.
    
-   **Tile Organization and Caching:** Organizing downloaded tiles into a structured directory format that the Backcountry Beacon can serve. This includes caching tile data in JSON files, which are used to manage and optimize the download process.
    
-   **Command-Line Interface:** Providing a user-friendly command-line interface (CLI) that allows users to specify which states and zoom levels to download, manage output directories, and estimate storage requirements before proceeding with the download.
    

These tools work together to automate the process of preparing map data for the Backcountry Beacon, enabling the device to deliver reliable offline navigation capabilities.


## Visualize Hawaii’s Tile Cache
![Calculating State Tiles](https://i.ibb.co/sQ1Sr69/Screenshot-2024-08-24-at-4-39-02-PM.png)
To see a visualization of the tiles and tile cache specifically for Hawaii, visit the [Cache Explorer](https://cache-explorer.netlify.app/). This tool helps you understand the geographic areas and zoom levels covered by your downloaded tiles for Hawaii.

## Using the Tile Downloader

The `tile-downloader` script provides a command-line interface (CLI) that allows you to download map tiles for specific states and zoom levels, organizing them for use on the Backcountry Beacon device.

#### Step-by-Step Guide:

1.  **Prerequisites:**
    
    -   Ensure you have [Node.js](https://nodejs.org/) installed on your system. The script uses Node.js to manage dependencies and run the downloading process.
2.  **Clone the Repository:**
    
    -   If you haven’t already, clone the Backcountry Beacon repository to your local machine:
        
 ```
git clone https://github.com/diyaquanauts/BackcountryBeacon.git
cd BackcountryBeacon/tileDownloader
 ```
    
**Install Dependencies:**

-   Navigate to the `tileDownloader` directory and install the required Node.js packages:
 `npm install `

**Run the Tile Downloader:**

-   Use the `tile-downloader.js` script to specify which states and zoom levels you want to download. You can also specify the output directory for the tiles and the directory for cached data.
-   Example command:
 ```
node tile-downloader.js -s "California:12,Nevada:14" -o ./tiles -c ./tile-poly-cache
 ```
 -  **Options:**
        -   `-s, --states <states>`: Comma-separated list of states with their max zoom levels. For example, `"California:12,Nevada:14"`.
        -   `-o, --output <directory>`: Directory where tiles will be saved. Default is `./tiles`.
        -   `-c, --cache <directory>`: Directory where cached data is stored. Default is `./tile-poly-cache`.

### How the Tile Downloader Works

The `tile-downloader.js` script is designed to efficiently download and organize map tiles for offline use. Here’s a breakdown of how it works:

1.  **Tile Calculation:**
    
    -   The script first determines which tiles are needed based on the geographic regions (states) and zoom levels specified by the user.
    -   It uses precomputed tile data stored in JSON format (`tile-poly-cache`) to quickly identify the required tiles.
2.  **Concurrency and Progress Tracking:**
    
    -   The script supports concurrent downloading of tiles, which speeds up the process. By default, it downloads up to 240 tiles simultaneously.
    -   A progress bar provides real-time feedback on the status of the download.
3.  **Tile Downloading:**
    
    -   For each required tile, the script constructs the appropriate URL to fetch the tile image from the map server.
    -   It organizes the downloaded tiles into a directory structure (`/z/x/y.png`), which matches the format expected by the Backcountry Beacon’s map viewer.

## Understanding the Tile Cache

The tile caching system in the `tileDownloader` script is designed to optimize the process of downloading and organizing map tiles. By using precomputed tile data stored in JSON format, the script reduces the need for real-time calculations and ensures that only the necessary tiles are downloaded.

#### How the Cache Works

1.  **Cache Directory Structure:**
    
    -   The cached data is stored in the `tile-poly-cache` directory, with each state having its own JSON file (e.g., `California-tiles.json`).
    -   Each JSON file contains precomputed information about the tile coordinates (x, y, z) that are required for that state at various zoom levels.
    
#### Example JSON Structure:

    {
     "12": {
       "456": [789, 790, 791],
       "457": [789, 790]
     },
     "13": {
       "912": [345, 346],
       "913": [345]
     }
    }
-   -   **Explanation:**
        -   In this structure, the keys represent the zoom levels (e.g., `"12"`), and each zoom level contains an object where the keys represent the x-coordinates of the tiles.
        -   The values are arrays of y-coordinates, indicating which tiles are required at that specific x-coordinate and zoom level.
