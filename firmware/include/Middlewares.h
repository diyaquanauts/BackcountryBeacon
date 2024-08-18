#include <MiddlewareSystem.h>

CREATE_MIDDLEWARE(logRequest, {
    Serial.println();
    size_t freeHeap = ESP.getFreeHeap();
    Serial.printf("Free Heap: %u bytes.\n", freeHeap);
    Serial.printf("Request: %s %s\n", request->methodToString(), request->url().c_str());
    Serial.printf("Version: HTTP/%d.%d\n", request->version() / 10, request->version() % 10);
    Serial.printf("Host: %s\n", request->host().c_str());
    Serial.printf("Content-Type: %s\n", request->contentType().c_str());
    Serial.printf("Content-Length: %u\n", request->contentLength());
    Serial.printf("Multipart: %s\n", request->multipart() ? "true" : "false");

    int headers = request->headers();
    for (int i = 0; i < headers; i++) {
        AsyncWebHeader* h = request->getHeader(i);
        Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isFile()) {
            Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        } else if (p->isPost()) {
            Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        } else {
            Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }
    Serial.println();

    next();
});

CREATE_MIDDLEWARE(monitorResources, {
    size_t freeHeap = ESP.getFreeHeap();
    const size_t criticalHeapThreshold = 170000; 
    if (freeHeap < criticalHeapThreshold) {
        Serial.printf("Free Heap: %u bytes. Sending 503\n", freeHeap);
        request->send(503, "text/plain", "Service Unavailable: Low Memory, Please Try Again Later");
        return;
    }

    next();
});


