const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Map Tiles</title>
    <link rel="stylesheet" href="leaflet.css">
    <style>
        #map {
            height: 100vh;
        }
        .emoji-icon {
            font-size: 30px; /* Adjust the size of the emoji */
            line-height: 1; /* Ensure the emoji is centered */
        }
        .info {
            background: rgb(0, 0, 0);
            padding: 15px;
            z-index: 1000;
            color: white;
            display: flex;
            flex-direction: column;
        }
        .info p {
            margin: 5px 0; /* Adjust vertical space between paragraphs */
            line-height: 1; /* Adjust line spacing within each paragraph */
            padding: 0; /* Remove any default padding */
            font-family: "Apple SD Gothic Neo";
            font-size: small;
        }
        .info.error {
            background: rgb(251, 145, 59);
        }
        /* Mobile styling */
        @media (max-width: 768px) {
            .info {
                position: absolute;
                bottom: 0;
                left: 0;
                width: 100%;
                flex-direction: row;
                justify-content: space-around;
                box-sizing: border-box;
            }

            #map {
                height: calc(100vh - 50px); /* Adjust map height to account for status bar */
            }
        }

        /* Desktop styling */
        @media (min-width: 769px) {
            .info {
                position: absolute;
                bottom: 10px;
                left: 10px;
                flex-direction: column;
            }
        }

        /* Center button */
        #centerButton {
            position: absolute;
            bottom: 70px;
            right: 30px;
            z-index: 1001;
            background: #007bff;
            color: white;
            border: none;
            padding: 10px;
            cursor: pointer;
            font-size: 14px;
            border-radius: 5px;
            display: none; /* Initially hidden */
        }
        #filesButton {
            background-color: transparent; /* Match background with status */
            color: white; /* Match text color */
            border: none; /* Remove border */
            cursor: pointer; /* Pointer cursor to indicate it's clickable */
            font-family: "Apple SD Gothic Neo"; /* Match font family */
            font-size: small; /* Match font size */
            text-align: left; /* Align text to the left */
            padding: 0; /* Match padding with status */
            margin: 5px 0; /* Match margin with status */
            line-height: 1; /* Match line height with status */
            text-decoration: underline; /* Subtle underline to indicate it's clickable */
        }
        #filesButton:hover {
            background-color: #0056b3;
        }
    </style>
</head>
<body>
<div id="map"></div>
<button id="centerButton">🧭 Center</button>
<div class="info" id="infoBox">
    <p id="altitude">🏔️: N/A</p>
    <p id="lastUpdate">⏱️: N/A</p>
    <p id="satellites">🛰️: N/A</p>
    <p><button id="filesButton" onclick="location.href='/files'">🗂️ Files</button></p>
</div>
<script src="leaflet.js"></script>
<script>
    const host = window.location.hostname;
    const tileQueue = [];
    let isProcessingQueue = false;
    let map, userMarker;
    let currentLat, currentLng;
    let mapCentered = true;

    function extractCoordsFromUrl(url) {
        const regex = /\/(\d+)\/(\d+)\/(\d+)\.png$/;
        const matches = url.match(regex);
        if (matches) {
            const [_, z, x, y] = matches.map(Number); // Convert to numbers
            return { z, x, y };
        }
        return null;
    }

    function loadTile(url, tile, done) {
        console.log(`Loading tile: ${url}`);

        tile.onload = function() {
            console.log(`Tile loaded successfully: ${url}`);
            done(null, tile);
            setTimeout(processQueue, 100); // Wait before processing the next tile
        };

        tile.onerror = function() {
            console.error(`Tile failed to load: ${url}`);
            done('error', tile);
            setTimeout(processQueue, 100); // Wait before processing the next tile
        };

        tile.src = url; // Start loading the image
    }

    function processQueue() {
        if (isProcessingQueue || tileQueue.length === 0) return;

        isProcessingQueue = true;

        const { url, tile, done } = tileQueue.shift(); // Get the next item in the queue
        loadTile(url, tile, function(error, tile) {
            isProcessingQueue = false; // Allow the next item to be processed
            done(error, tile);
        });
    }

    L.TileLayer.Queued = L.TileLayer.extend({
        createTile: function(coords, done) {
            const url = this.getTileUrl(coords);
            const tile = document.createElement('img');
            console.log(`Queuing tile: ${url}`);
            tileQueue.push({ url, tile, done }); // Add the tile load task to the queue
            if (!isProcessingQueue) processQueue(); // Start processing if not already doing so
            return tile;
        }
    });

    L.tileLayer.queued = function(url, options) {
        return new L.TileLayer.Queued(url, options);
    };

    function updateMapLocation(lat, lng) {
        currentLat = lat;
        currentLng = lng;

        if (!map) {
            map = L.map('map').setView([lat, lng], 13);
            L.tileLayer.queued(`http://${host}/tiles/{z}/{x}/{y}.png`, {
                maxZoom: 16,
                tileSize: 256,
                errorTileUrl: 'error.png'
            }).addTo(map);

            const emojiIcon = L.divIcon({
                className: 'emoji-icon', // Custom class for styling if needed
                html: '📍',
                iconAnchor: [15, 30] // Adjust anchor to position the icon correctly
            });

            // Add the custom marker to the map
            userMarker = L.marker([lat, lng], { icon: emojiIcon }).addTo(map);

            // Detect when the user pans the map
            map.on('move', () => {
                if (!mapCentered) {
                    document.getElementById('centerButton').style.display = 'block';
                }
            });

            map.on('moveend', () => {
                const center = map.getCenter();
                if (Math.abs(center.lat - currentLat) > 0.001 || Math.abs(center.lng - currentLng) > 0.001) {
                    mapCentered = false;
                    document.getElementById('centerButton').style.display = 'block';
                }
            });
        } else {
            if (mapCentered) {
                map.setView([lat, lng]);
                userMarker.setLatLng([lat, lng]);
            } else {
                userMarker.setLatLng([lat, lng]);
            }
        }
    }

    function centerMapOnUser() {
        if (map && currentLat !== undefined && currentLng !== undefined) {
            map.setView([currentLat, currentLng], map.getZoom());
            document.getElementById('centerButton').style.display = 'none';
            mapCentered = true;
        }
    }

    document.getElementById('centerButton').addEventListener('click', centerMapOnUser);

    function getAccuracyFromHdop(hdop) {
        const uere = 8; // Assume a typical UERE value in meters
        return hdop * uere;
    }

    function updateInfoBox(location, diagnostics) {
        const accuracy = getAccuracyFromHdop(location.hdop);
        document.getElementById('altitude').textContent = `🏔️: ${location.altitude.toFixed(2)} m`;
        document.getElementById('lastUpdate').textContent = `⏱️: ${location.last_update} sec`;
        document.getElementById('satellites').textContent = `🛰️: ${location.satellites}`;
    }

    async function fetchDataAndUpdate() {
        try {
            const pingResponse = await fetch(`http://${host}/ping`);
            if (!pingResponse.ok) {
                infoBox.classList.add('error');
            }else{
                infoBox.classList.remove('error');
            }
            const locationResponse = await fetch(`http://${host}/location`);
            const diagnosticsResponse = await fetch(`http://${host}/diagnostics`);
            if (!locationResponse.ok || !diagnosticsResponse.ok) throw new Error(`HTTP error!`);

            const location = await locationResponse.json();
            const diagnostics = await diagnosticsResponse.json();

            const userLat = location.latitude;
            const userLng = location.longitude;

            updateMapLocation(userLat, userLng);
            updateInfoBox(location, diagnostics);
        } catch (error) {
            console.error('Error fetching data:', error);
        }
    }

    document.addEventListener('DOMContentLoaded', () => {
        fetchDataAndUpdate();
        setInterval(fetchDataAndUpdate, 2500); // Poll data every 5 seconds
    });
</script>
</body>
</html>


)rawliteral";
