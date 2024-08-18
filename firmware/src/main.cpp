#include <WiFi.h>
#include <SD_MMC.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "FunctionTimer.h"
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include "pin_config.h"
#include <Middlewares.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <qrcode.h>
#include <routes.h>

// Access point credentials
const char* ssid = "topo-map";
const char* password = "mapables"; // Set to "" for no password

TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);
MiddlewareHandler app(server);
TinyGPSPlus gps;

volatile bool buttonPressed = false;
bool displayNetwork = true;
bool screenOn = true;
long lastButtonPress = 0;
long lastGpsUpdate = 0;

void IRAM_ATTR handleButtonPress() {
    buttonPressed = true;  // Set a flag indicating the button was pressed
}

// Function to turn off the display
void turnOffDisplay() {
    digitalWrite(TFT_LEDA_PIN, 1);  // Turn off the display backlight
    screenOn = false;
}

void turnOnDisplay() {
    digitalWrite(TFT_LEDA_PIN, 0);  // Turn on the display backlight
    screenOn = true;
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 44, 43); // Initialize Serial2 for GPS on pins 44 (RX) and 43 (TX)
    
    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(BTN_PIN, handleButtonPress, FALLING);

    pinMode(TFT_LEDA_PIN, OUTPUT);
    tft.init();
    tft.setRotation(4);
    tft.fillScreen(TFT_BLACK);
    digitalWrite(TFT_LEDA_PIN, 0);
    tft.setTextFont(1);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);

    tft.println();
    tft.println("  YOU");
    tft.println("  ARE");
    tft.println("  OFF");
    tft.println(" GRID.");
    

    SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
    if (!SD_MMC.begin("/sdcard", false, false, 40000000, 16)) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.println("SD card initialized.");

    // Set up access point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Add default headers for CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // Add middlewares
    app.use(logRequest);
    app.use(monitorResources);

    
    setupRoutes(app, server, gps);

    server.onNotFound(notFound);

    server.begin();
}

void readGPS() {
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
    if(gps.location.isUpdated()){
        lastGpsUpdate = millis();
    }
}

void checkForStaleConnections(){
    app.checkForStaleConnections();
}

void logHeap(){
    if(displayNetwork){
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.setTextSize(2);
        tft.setCursor(0,0);
        tft.println();
        tft.println("  YOU");
        tft.println("  ARE");
        if(gps.location.isValid()){
            tft.setTextColor(TFT_GREEN);
            tft.println("  OFF");
            tft.setTextColor(TFT_RED);
        }else{
            tft.println("  OFF");
        }
        tft.println(" GRID.");
        tft.println();

        // Create the Wi-Fi QR code content
        char wifiConfig[128];
        snprintf(wifiConfig, sizeof(wifiConfig), "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);

        // Generate QR code
        QRCode qrcode;
        uint8_t qrcodeData[qrcode_getBufferSize(3)];
        qrcode_initText(&qrcode, qrcodeData, 3, 0, wifiConfig);
        // Calculate position to center QR code at the bottom
        int qrSize = qrcode.size * 2; // Scale the QR code size by 2
        int qrX = (tft.width() - qrSize) / 2;
        int qrY = tft.height() - qrSize - 10; // 10 pixels padding from bottom

        // Draw the QR code on the display
        for (int y = 0; y < qrcode.size; y++) {
            for (int x = 0; x < qrcode.size; x++) {
                int pixelColor = qrcode_getModule(&qrcode, x, y) ? TFT_BLACK : TFT_WHITE;
                tft.fillRect(qrX + x * 2, qrY + y * 2, 2, 2, pixelColor); // Scale the QR code
            }
        }
    }else{
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0);
         // Generate QR code for the SoftAP IP address
        String ipAddress = WiFi.softAPIP().toString();
        char qrUrl[64];
        snprintf(qrUrl, sizeof(qrUrl), "http://%s/", ipAddress.c_str());

        QRCode qrcode;
        uint8_t qrcodeData[qrcode_getBufferSize(3)];
        qrcode_initText(&qrcode, qrcodeData, 3, 0, qrUrl);

        // Calculate position to center QR code at the top
        int qrSize = qrcode.size * 2; // Scale the QR code size by 2
        int qrX = (tft.width() - qrSize) / 2;
        int qrY = 10; // 10 pixels padding from top

        // Draw the QR code on the display
        for (int y = 0; y < qrcode.size; y++) {
            for (int x = 0; x < qrcode.size; x++) {
                int pixelColor = qrcode_getModule(&qrcode, x, y) ? TFT_BLACK : TFT_WHITE;
                tft.fillRect(qrX + x * 2, qrY + y * 2, 2, 2, pixelColor); // Scale the QR code
            }
        }

        // Draw the "YOU ARE HERE" text below the QR code
        tft.setCursor(0, qrY + qrSize + 10);
        tft.setTextSize(2);
        tft.setTextColor(TFT_RED);
        tft.println();
        tft.println("  YOU");
        tft.println("  ARE");
        tft.println(" HERE.");
    }
}

void buttonHandler(){
    if(buttonPressed){
        lastButtonPress = millis();
        buttonPressed=false;
        if(!screenOn){
            turnOnDisplay();
            return;
        }
        displayNetwork = !displayNetwork;
        logHeap();
    }
}
void checkIfScreenInactive(){
    if(!screenOn){
        return;
    }
    if(millis() - lastButtonPress > 30000){
        turnOffDisplay();
    }
}

FunctionTimer screnMonitor(checkIfScreenInactive, 10000);
FunctionTimer buttonMonitor(buttonHandler,100);
FunctionTimer heapMonitor(logHeap, 10000);
FunctionTimer connectionMonitor(&checkForStaleConnections, 1000);
FunctionTimer gpsReader(readGPS, 1000); // Read GPS data every second

void loop() {
    connectionMonitor.service();
    heapMonitor.service();
    gpsReader.service();
    buttonMonitor.service();
    screnMonitor.service();
    vTaskDelay(1); // Yield to the scheduler
}
