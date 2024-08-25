
The `include` folder contains custom header files that define various components and functionalities used throughout the Backcountry Beacon firmware. These files provide the foundational elements for handling tasks such as timing functions, managing middleware, and defining routes.

## File Overview

### 1. `FunctionTimer.h`

The `FunctionTimer.h` file defines a simple task manager that allows functions to be executed at regular intervals without blocking the main loop.

#### Code Example:

    #include "FunctionTimer.h"
    
    void updateDisplay() {
        // Code to update the TFT display with new information
    }
    
    FunctionTimer displayUpdater(updateDisplay, 10000); // Run every 10 seconds
    
    void loop() {
        displayUpdater.service(); // Check if it's time to run the function
    }
#### Explanation:

-   **Purpose:** `FunctionTimer` is designed to manage periodic tasks efficiently, ensuring that various functions can be executed on a schedule without interfering with the main loop’s responsiveness.
-   **Usage:** The class is instantiated with a function pointer and an update interval. The `service()` method is called in the main loop to check if the function should be executed based on the elapsed time.

### 2. `MiddlewareSystem.h`

The `MiddlewareSystem.h` file defines a middleware handler system built on top of the `ESPAsyncWebServer`. This system is designed to be "express-like," allowing for easy chaining and management of middleware functions.

#### Code Example:

    #include "MiddlewareSystem.h"
    
    MiddlewareHandler app(server);
    
    // Custom middleware function to log requests
    CREATE_MIDDLEWARE(logRequest, {
        Serial.printf("Request URL: %s\n", request->url().c_str());
        next(); // Pass control to the next middleware or handler
    });
    
    void setup() {
        // Initialize the server
        app.use(logRequest); // Add middleware to the chain
        app.addRoute("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", "Hello, world!");
        });
        server.begin();
    }
#### Explanation:

-   **Purpose:** The middleware system allows for modular and flexible handling of HTTP requests. Middleware functions can be chained together, each performing specific tasks like logging, resource monitoring, or custom request processing.
-   **Usage:** Middlewares are created using the `CREATE_MIDDLEWARE` macro and can be added to the middleware chain using the `use()` method. The `addRoute()` method allows the integration of routes that are processed through the middleware chain before handling the request.
### 3. `routes.h`

The `routes.h` file defines the routes for the web server, linking specific URLs to their corresponding handlers. This file is central to how the firmware serves content and responds to user interactions.

#### Code Example:

    #include "routes.h"
    
    void setupRoutes(MiddlewareHandler &app, AsyncWebServer &server, TinyGPSPlus &gps) {
        app.addRoute("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html); // Serve the main page
        });
    
        app.addRoute("/location", HTTP_GET, [&gps](AsyncWebServerRequest *request){
            String locationData = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
            request->send(200, "application/json", "{\"location\": \"" + locationData + "\"}");
        });
    
        // Additional routes can be added here
    }

#### Explanation:

-   **Purpose:** `routes.h` centralizes the definition of all web routes used by the server. This makes it easy to manage and update the URLs and their corresponding handlers.
-   **Usage:** The `setupRoutes()` function is called in the `setup()` function of the main firmware file. It registers routes with the server, associating them with specific handlers that serve HTML, JSON, or other content types.
