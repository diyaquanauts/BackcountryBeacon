#ifndef MIDDLEWARESYSTEM_H
#define MIDDLEWARESYSTEM_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <functional>
#include <vector>
#include <map>

using MiddlewareFunction = std::function<void(AsyncWebServerRequest*, std::function<void()>)>;

class MiddlewareHandler {
public:
    MiddlewareHandler(AsyncWebServer& server) : server(server) {}

    void use(MiddlewareFunction middleware) {
        middlewares.push_back(middleware);
    }

    void handle(AsyncWebServerRequest* request, std::function<void()> finalHandler) {
        std::function<void(int)> next;
        next = [this, request, finalHandler, &next](int index) {
            if (index < middlewares.size()) {
                middlewares[index](request, [index, &next]() {
                    next(index + 1);
                });
            } else {
                finalHandler();
            }
        };
        next(0);
    }

    void addRoute(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler) {
        server.on(uri, method, [this, handler](AsyncWebServerRequest* request) {
            this->handle(request, [request, handler]() {
                handler(request);
            });
        });
    }

    void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control = nullptr, const char* last_modified = nullptr) {
        auto handler = &server.serveStatic(uri, fs, path);
        if (cache_control) {
            handler->setCacheControl(cache_control);
        }
        if (last_modified) {
            handler->setLastModified(last_modified);
        }
        server.on(uri, HTTP_GET, [this, handler](AsyncWebServerRequest* request) {
            this->handle(request, [handler, request]() {
                handler->handleRequest(request);
            });
        });
    }

    void checkForStaleConnections() {
        uint32_t currentTime = millis();
        for (auto it = lastActiveTimes.begin(); it != lastActiveTimes.end();) {
            if (currentTime - it->second > connectionTimeout) {
                it->first->client()->close();
                it = lastActiveTimes.erase(it); // Erase and move to the next
            } else {
                ++it;
            }
        }
    }

    void updateLastActiveTime(AsyncWebServerRequest* request) {
        lastActiveTimes[request] = millis();
    }

private:
    AsyncWebServer& server;
    std::vector<MiddlewareFunction> middlewares;
    std::map<AsyncWebServerRequest*, uint32_t> lastActiveTimes;
    const uint32_t connectionTimeout = 60000; // 60 seconds timeout
};

#define CREATE_MIDDLEWARE(name, code) \
MiddlewareFunction name = [](AsyncWebServerRequest* request, std::function<void()> next) { code; }

#endif // MIDDLEWARESYSTEM_H
