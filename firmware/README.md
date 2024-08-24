# Backcountry Beacon Firmware Overview

The Backcountry Beacon firmware transforms the LilyGo S3 USB dongle into a portable, offline mapping and file server. Developed using PlatformIO, this firmware integrates essential hardware components and software functionalities to support outdoor adventurers.

### Hardware Overview

The firmware is designed for the LilyGo S3 USB dongle, which includes:

-   **ESP32 Microcontroller** for handling Wi-Fi connectivity and processing tasks.
-   **TFT Display** for displaying basic status information.
-   **GPS Module** to track and provide real-time location data.
-   **SD Card Storage** for storing and serving offline maps and files.

### Webpage Loading

-   **Embedded HTML as `.h` Files:** The webpages, such as the map viewer and file browser, are stored as `.h` files within the firmware. This approach allows the entire web interface to be embedded directly in the device’s memory. As a result, there’s no need to modify the file system on the device after flashing, making development and updates simpler and more robust.
-   **Single Flash Process:** Since the web files are compiled into the firmware, the entire system is flashed to the device in one step, ensuring that everything is self-contained and ready to use without additional setup.

### Middleware System

The firmware uses a middleware system built on the async web server, providing an "express-like" experience for handling HTTP requests:

-   **Async Web Server Foundation:** The middleware is layered on top of the `ESPAsyncWebServer`, allowing non-blocking, asynchronous handling of requests.
-   **"Express-like" Design:** The middleware system is designed to be modular and easy to extend, similar to Express.js in the Node.js ecosystem. This structure makes it straightforward to add, remove, or modify how requests are processed before reaching their final handlers.

### FunctionTimer Utility

The `FunctionTimer` is a simple task manager that schedules and executes periodic tasks without blocking the main loop
