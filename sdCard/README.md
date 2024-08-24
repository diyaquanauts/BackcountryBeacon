
### SD Card Organization for Backcountry Beacon

To ensure the Backcountry Beacon functions smoothly, the SD card needs to be organized with specific directories and files.

#### Recommended Structure:

-   **Root Directory (`/`):**
    
    -   This is where key assets required for the Backcountry Beacon’s web interface should be placed.
    -   **Files included:**
        -   **`leaflet.css`**: Stylesheet for the map viewer.
        -   **`leaflet.js`**: JavaScript library for interactive maps.
        -   **`error.png`**: Image displayed when a map tile fails to load.
        -   **`favicon.ico`**: Icon displayed in the browser tab.
-   **Tiles Directory (`/tiles`):**
    
    -   This directory contains all the map tiles that the Backcountry Beacon will serve. The tiles are organized by zoom level (`z`), x-coordinate (`x`), and y-coordinate (`y`).
    -   **Example path:** `/tiles/14/2653/6335.png` represents a tile at zoom level 14, x-coordinate 2653, and y-coordinate 6335.
-   **Serve Directory (`/serve`):**
    
    -   This directory contains custom resources that you want to access while on the trail, such as PDFs, documents, or audio files.

#### Example SD Card Structure:

    /
    ├── leaflet.css
    ├── leaflet.js
    ├── error.png
    ├── favicon.ico
    ├── tiles
    │   ├── 14
    │   │   ├── 2653
    │   │   │   └── 6335.png
    │   │   └── ...
    │   └── ...
    └── serve
        ├── knot_guide.pdf
        ├── plant_id.json
        └── nature_audiobook.mp3
### Loading Files onto the SD Card

1.  **Prepare Map Tiles:**
    
    -   Use the `tile-downloader` script to fetch and organize your map tiles into the `/tiles` directory as described above.
2.  **Add Custom Files and Assets:**
    
    -   Place `leaflet.css`, `leaflet.js`, `error.png`, and `favicon.ico` in the root directory.
    -   Place your additional resources into the `/serve` directory.
3.  **Insert SD Card:**
    
    -   Once the files are loaded, insert the SD card into the Backcountry Beacon, and it will automatically begin serving the content.

