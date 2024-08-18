#ifndef ROUTES_H
#define ROUTES_H

#include <ESPAsyncWebServer.h>
#include <SD_MMC.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include "index.h"
#include "files.h"

extern long lastGpsUpdate;

void notFound(AsyncWebServerRequest *request) {
    request->redirect("/");
    Serial.println("Request not found: " + request->url() + " - redirected to /");
}

String formatFileSize(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < (1024 * 1024)) {
        return String(bytes / 1024.0, 2) + " KB";
    } else if (bytes < (1024 * 1024 * 1024)) {
        return String(bytes / (1024.0 * 1024.0), 2) + " MB";
    } else {
        return String(bytes / (1024.0 * 1024.0 * 1024.0), 2) + " GB";
    }
}

void setupRoutes(MiddlewareHandler& app, AsyncWebServer& server, TinyGPSPlus& gps) {
    // Add the root route
    app.addRoute("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // Add the location route
    app.addRoute("/location", HTTP_GET, [&](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(500);
        doc["last_update"] = (int) (millis() - lastGpsUpdate) / 1000;

        doc["latitude"] = gps.location.isValid() ? gps.location.lat() : 0.0;
        doc["longitude"] = gps.location.isValid() ? gps.location.lng() : 0.0;
        doc["altitude"] = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
        doc["satellites"] = gps.satellites.isValid() ? gps.satellites.value() : 0;
        doc["hdop"] = gps.hdop.isValid() ? gps.hdop.value() : 0.0;
        doc["time"] = gps.time.isValid() ? gps.time.value() : 0;

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
        Serial.println("Sending GPS data: " + jsonResponse);
    });

    // Add the diagnostics route
    app.addRoute("/diagnostics", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(200);
        doc["free_heap"] = ESP.getFreeHeap();
        doc["uptime_seconds"] = millis() / 1000;

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
        Serial.println("Sending Diagnostics data: " + jsonResponse);
    });

   app.addRoute("/files", HTTP_GET, [](AsyncWebServerRequest *request){
        String fileListHtml = "";

        File root = SD_MMC.open("/serve");
        if (!root) {
            fileListHtml += "<div class=\"file\">Failed to open directory</div>";
        } else {
            File file = root.openNextFile();
            while (file) {
                String fileName = file.name();
                if (!file.isDirectory() && fileName.charAt(0) != '.') {
                    fileListHtml += "<div class=\"file\">";
                    fileListHtml += "<a href=\"/download?file=" + String(file.name()) + "\">";
                    fileListHtml += String(file.name());
                    fileListHtml += " (" + formatFileSize(file.size()) + ")";                    fileListHtml += "</a></div>";
                }
                file = root.openNextFile();
            }
        }

        // Replace the placeholder with the actual file list HTML
        String htmlResponse = files_html;
        htmlResponse.replace("<!-- FILES_PLACEHOLDER -->", fileListHtml);

        // Send the response
        request->send(200, "text/html", htmlResponse);
    });


    app.addRoute("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String fileName = request->getParam("file")->value();
            String path = "/serve/" + fileName;
            File file = SD_MMC.open(path, FILE_READ);

            if (!file) {
                request->send(404, "text/plain", "File Not Found");
                return;
            }

            // Set the Content-Disposition header to specify the file name
            AsyncWebServerResponse *response = request->beginChunkedResponse("application/octet-stream", [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
                size_t bytesRead = file.read(buffer, maxLen);
                vTaskDelay(1 / portTICK_PERIOD_MS); // Yield to the scheduler
                if (bytesRead > 0) {
                    return bytesRead;
                } else {
                    file.close();
                    return 0; // End of file
                }
            });

            response->addHeader("Content-Disposition", "attachment; filename=" + fileName);
            request->send(response);
        } else {
            request->send(400, "text/plain", "File parameter missing");
        }
    });
    // Add the tiles route
    app.addRoute("/tiles/*", HTTP_GET, [](AsyncWebServerRequest *request) {
        String path = "/" + request->url(); // Ensure the path starts with a leading '/'

        File file = SD_MMC.open(path, FILE_READ);

        if (!file) {
            request->send(404, "text/plain", "File Not Found");
            return;
        }

        request->sendChunked("application/octet-stream", [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
            size_t bytesRead = file.read(buffer, maxLen);
            vTaskDelay(1 / portTICK_PERIOD_MS); // Yield to the scheduler
            if (bytesRead > 0) {
                return bytesRead;
            } else {
                file.close();
                return 0; // End of file
            }
        });
    });

    // Add the ping route
    app.addRoute("/ping", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "pong");
    });

    // Serve static files
    app.serveStatic("/leaflet.js", SD_MMC, "/leaflet.js");
    app.serveStatic("/leaflet.css", SD_MMC, "/leaflet.css");
    app.serveStatic("/error.png", SD_MMC, "/error.png");
    app.serveStatic("/favicon.ico", SD_MMC, "/favicon.ico");

    // Handle connectivity routes
    app.addRoute("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    app.addRoute("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    app.addRoute("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    app.addRoute("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    app.addRoute("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200); });
    app.addRoute("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });

    server.onNotFound(notFound);  // Handle 404 errors with middleware
}

#endif // ROUTES_H
