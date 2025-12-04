#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// ==================== Configuration ====================

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define COMMAND_CHAR_UUID   "12345678-1234-1234-1234-123456789abd"
#define STATUS_CHAR_UUID    "12345678-1234-1234-1234-123456789abe"

// Device Configuration
#define DEVICE_NAME         "ESP32C3-Admin"
#define CONFIG_BOOT_BUTTON  9  // GPIO9 is BOOT button on ESP32-C3

// BLE Configuration
#define BLE_MTU_SIZE        512
#define MAX_CHUNK_SIZE      100  // Small size to account for JSON overhead and stay under BLE limit

// ==================== Global Variables ====================

bool bleEnabled = true;
bool webServerEnabled = false;
Preferences preferences;
WebServer server(80);

// BLE Variables
BLEServer* pServer = nullptr;
BLECharacteristic* pCommandCharacteristic = nullptr;
BLECharacteristic* pStatusCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// File Upload State
String currentFileName = "";
File currentFile;
size_t expectedFileSize = 0;
size_t receivedBytes = 0;

// ==================== Forward Declarations ====================

void sendStatus(const char* type, const char* message, JsonObject extraData = JsonObject());
void sendStatusWithData(const char* type, const char* message, const char* key, const char* value);
void handleBLECommand(const char* jsonCommand);
void handleWiFiConfig(JsonDocument& doc);
void handleFileMeta(JsonDocument& doc);
void handleFileChunk(JsonDocument& doc);
void handleReboot(JsonDocument& doc);
void handleListFiles();
String getContentType(String filename);

// ==================== BLE Callbacks ====================

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client connected");
    sendStatus("info", "BLE client connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client disconnected");
  }
};

class CommandCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String(pCharacteristic->getValue().c_str());
    if (value.length() > 0) {
      handleBLECommand(value.c_str());
    }
  }
};

// ==================== Utility Functions ====================

void sendStatus(const char* type, const char* message, JsonObject extraData) {
  if (!deviceConnected || pStatusCharacteristic == nullptr) return;
  
  StaticJsonDocument<512> doc;
  doc["type"] = type;
  doc["message"] = message;
  
  // Copy extra data if provided
  if (!extraData.isNull()) {
    for (JsonPair kv : extraData) {
      doc[kv.key()] = kv.value();
    }
  }
  
  String output;
  serializeJson(doc, output);
  pStatusCharacteristic->setValue(output.c_str());
  pStatusCharacteristic->notify();
  
  Serial.print("Status sent: ");
  Serial.println(output);
}

void sendStatusWithData(const char* type, const char* message, const char* key, const char* value) {
  if (!deviceConnected || pStatusCharacteristic == nullptr) return;
  
  StaticJsonDocument<512> doc;
  doc["type"] = type;
  doc["message"] = message;
  doc[key] = value;
  
  String output;
  serializeJson(doc, output);
  pStatusCharacteristic->setValue(output.c_str());
  pStatusCharacteristic->notify();
  
  Serial.print("Status sent: ");
  Serial.println(output);
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else if (filename.endsWith(".zip")) return "application/zip";
  return "text/plain";
}

// ==================== BLE Command Handlers ====================

void handleBLECommand(const char* jsonCommand) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, jsonCommand);
  
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    sendStatus("error", "Invalid JSON command");
    return;
  }
  
  const char* type = doc["type"];
  Serial.print("Received command: ");
  Serial.println(type);
  
  if (strcmp(type, "wifi") == 0) {
    handleWiFiConfig(doc);
  }
  else if (strcmp(type, "file_meta") == 0) {
    handleFileMeta(doc);
  }
  else if (strcmp(type, "file_chunk") == 0) {
    handleFileChunk(doc);
  }
  else if (strcmp(type, "reboot") == 0) {
    handleReboot(doc);
  }
  else if (strcmp(type, "list_files") == 0) {
    handleListFiles();
  }
  else if (strcmp(type, "get_status") == 0) {
    // Send current device status
    StaticJsonDocument<384> statusDoc;
    statusDoc["type"] = "device_status";
    statusDoc["ble_active"] = bleEnabled;
    statusDoc["webserver_active"] = webServerEnabled;
    statusDoc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    
    // Check if WiFi credentials are saved
    preferences.begin("wifi", true);
    String savedSSID = preferences.getString("ssid", "");
    String savedPassword = preferences.getString("password", "");
    preferences.end();
    
    statusDoc["has_wifi_credentials"] = (savedSSID.length() > 0 && savedPassword.length() > 0);
    if (savedSSID.length() > 0) {
      statusDoc["saved_ssid"] = savedSSID;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      statusDoc["ip"] = WiFi.localIP().toString();
      statusDoc["ssid"] = WiFi.SSID();
      statusDoc["rssi"] = WiFi.RSSI();
    }
    
    String output;
    serializeJson(statusDoc, output);
    if (deviceConnected && pStatusCharacteristic != nullptr) {
      pStatusCharacteristic->setValue(output.c_str());
      pStatusCharacteristic->notify();
    }
  }
  else {
    sendStatus("error", "Unknown command type");
  }
}

void handleWiFiConfig(JsonDocument& doc) {
  const char* ssid = doc["ssid"];
  const char* pass = doc["pass"];
  
  if (!ssid || !pass) {
    sendStatus("error", "Missing SSID or password");
    return;
  }
  
  // Save WiFi credentials to NVS
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", pass);
  preferences.end();
  
  Serial.println("WiFi credentials saved");
  sendStatus("success", "WiFi credentials saved");
  
  // Try to connect to WiFi immediately to get IP
  Serial.println("Attempting to connect to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.print("WiFi connected! IP: ");
    Serial.println(ip);
    
    // Send WiFi status with IP to BLE client
    StaticJsonDocument<256> statusDoc;
    statusDoc["type"] = "wifi_status";
    statusDoc["connected"] = true;
    statusDoc["ip"] = ip;
    statusDoc["ssid"] = ssid;
    statusDoc["rssi"] = WiFi.RSSI();
    
    String output;
    serializeJson(statusDoc, output);
    if (deviceConnected && pStatusCharacteristic != nullptr) {
      pStatusCharacteristic->setValue(output.c_str());
      pStatusCharacteristic->notify();
    }
    
    // Disconnect WiFi to stay in BLE mode
    WiFi.disconnect();
  } else {
    sendStatus("warning", "WiFi credentials saved but connection test failed. Will try again on reboot.");
  }
}

void handleFileMeta(JsonDocument& doc) {
  const char* filename = doc["name"];
  size_t filesize = doc["size"];
  
  if (!filename) {
    sendStatus("error", "Missing filename");
    return;
  }
  
  // Close any existing file
  if (currentFile) {
    currentFile.close();
  }
  
  // Ensure filename starts with /
  currentFileName = filename;
  if (!currentFileName.startsWith("/")) {
    currentFileName = "/" + currentFileName;
  }
  
  // Delete existing file if it exists
  if (SPIFFS.exists(currentFileName)) {
    SPIFFS.remove(currentFileName);
  }
  
  // Open new file for writing
  currentFile = SPIFFS.open(currentFileName, FILE_WRITE);
  if (!currentFile) {
    sendStatus("error", "Failed to create file");
    Serial.println("Failed to create file: " + currentFileName);
    return;
  }
  
  expectedFileSize = filesize;
  receivedBytes = 0;
  
  Serial.print("File meta received: ");
  Serial.print(currentFileName);
  Serial.print(" (");
  Serial.print(expectedFileSize);
  Serial.println(" bytes)");
  
  sendStatus("success", "Ready to receive file");
}

void handleFileChunk(JsonDocument& doc) {
  if (!currentFile) {
    sendStatus("error", "No active file transfer");
    return;
  }
  
  JsonArray dataArray = doc["data"];
  size_t offset = doc["offset"];
  
  if (dataArray.isNull()) {
    sendStatus("error", "Missing chunk data");
    return;
  }
  
  // Write chunk to file
  size_t chunkSize = dataArray.size();
  uint8_t* buffer = new uint8_t[chunkSize];
  
  for (size_t i = 0; i < chunkSize; i++) {
    buffer[i] = dataArray[i].as<uint8_t>();
  }
  
  size_t written = currentFile.write(buffer, chunkSize);
  delete[] buffer;
  
  if (written != chunkSize) {
    sendStatus("error", "File write failed");
    currentFile.close();
    return;
  }
  
  receivedBytes += written;
  
  Serial.print("Chunk received: ");
  Serial.print(receivedBytes);
  Serial.print(" / ");
  Serial.print(expectedFileSize);
  Serial.println(" bytes");
  
  // Send progress update
  StaticJsonDocument<256> statusDoc;
  statusDoc["type"] = "file_progress";
  statusDoc["name"] = currentFileName;
  statusDoc["bytes_received"] = receivedBytes;
  statusDoc["total_bytes"] = expectedFileSize;
  
  String output;
  serializeJson(statusDoc, output);
  pStatusCharacteristic->setValue(output.c_str());
  pStatusCharacteristic->notify();
  
  // Check if file is complete
  if (receivedBytes >= expectedFileSize) {
    currentFile.close();
    Serial.println("File transfer complete: " + currentFileName);
    sendStatus("success", "File saved successfully");
    currentFileName = "";
    receivedBytes = 0;
    expectedFileSize = 0;
  }
}

void handleReboot(JsonDocument& doc) {
  const char* mode = doc["mode"];
  
  if (strcmp(mode, "web_server") == 0) {
    // Start web server (no reboot needed)
    if (!webServerEnabled) {
      sendStatus("info", "Starting web server...");
      
      if (connectToWiFi()) {
        setupWebServer();
        webServerEnabled = true;
        sendStatus("success", "Web server started! Site is now live.");
        
        // Send updated status
        StaticJsonDocument<384> statusDoc;
        statusDoc["type"] = "device_status";
        statusDoc["ble_active"] = bleEnabled;
        statusDoc["webserver_active"] = webServerEnabled;
        statusDoc["wifi_connected"] = true;
        statusDoc["ip"] = WiFi.localIP().toString();
        statusDoc["ssid"] = WiFi.SSID();
        
        String output;
        serializeJson(statusDoc, output);
        if (deviceConnected && pStatusCharacteristic != nullptr) {
          pStatusCharacteristic->setValue(output.c_str());
          pStatusCharacteristic->notify();
        }
      } else {
        sendStatus("error", "Failed to connect to WiFi");
      }
    } else {
      sendStatus("info", "Web server already running");
    }
    return;
  }
  else if (strcmp(mode, "ble_config") == 0) {
    // Reset configuration and reboot
    preferences.begin("config", false);
    preferences.putBool("configured", false);
    preferences.end();
    
    sendStatus("success", "Rebooting to reset...");
    delay(1000);
    ESP.restart();
  }
}

void handleListFiles() {
  StaticJsonDocument<1024> doc;
  doc["type"] = "file_list";
  JsonArray files = doc.createNestedArray("files");
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    JsonObject fileObj = files.createNestedObject();
    fileObj["name"] = String(file.name());
    fileObj["size"] = file.size();
    file = root.openNextFile();
  }
  
  String output;
  serializeJson(doc, output);
  pStatusCharacteristic->setValue(output.c_str());
  pStatusCharacteristic->notify();
}

// ==================== BLE Setup ====================

void setupBLE() {
  Serial.println("Initializing BLE...");
  
  BLEDevice::init(DEVICE_NAME);
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create Command Characteristic (Write)
  pCommandCharacteristic = pService->createCharacteristic(
    COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCommandCharacteristic->setCallbacks(new CommandCallbacks());
  
  // Create Status Characteristic (Read + Notify)
  pStatusCharacteristic = pService->createCharacteristic(
    STATUS_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pStatusCharacteristic->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE advertising started");
  Serial.print("Device name: ");
  Serial.println(DEVICE_NAME);
}

// ==================== WiFi & Web Server ====================

bool connectToWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();
  
  if (ssid.length() == 0) {
    Serial.println("No WiFi credentials found");
    return false;
  }
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  
  Serial.println("WiFi connection failed");
  return false;
}

void handleFileRequest() {
  String path = server.uri();
  
  // Root path serves index.html
  if (path == "/") {
    path = "/index.html";
  }
  
  Serial.print("Request: ");
  Serial.println(path);
  
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    String contentType = getContentType(path);
    server.streamFile(file, contentType);
    file.close();
    Serial.println("File served successfully");
  } else {
    Serial.println("File not found");
    server.send(404, "text/plain", "File Not Found");
  }
}

void handleStatus() {
  StaticJsonDocument<256> doc;
  doc["device"] = DEVICE_NAME;
  doc["mode"] = "web_server";
  doc["ip"] = WiFi.localIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();
  
  // List files
  JsonArray files = doc.createNestedArray("files");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    files.add(String(file.name()));
    file = root.openNextFile();
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void setupWebServer() {
  // Handle all requests with file serving
  server.onNotFound(handleFileRequest);
  
  // Status endpoint
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started on port 80");
}

// ==================== Main Setup & Loop ====================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("ESP32 C3 - BLE Config & Web Server");
  Serial.println("=================================\n");
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  
  // List files in SPIFFS
  Serial.println("\nFiles in SPIFFS:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
  }
  Serial.println();
  
  // Check boot button for config mode reset
  pinMode(CONFIG_BOOT_BUTTON, INPUT_PULLUP);
  if (digitalRead(CONFIG_BOOT_BUTTON) == LOW) {
    Serial.println("Boot button pressed - forcing BLE config mode");
    preferences.begin("config", false);
    preferences.putBool("configured", false);
    preferences.end();
    delay(1000);
  }
  
  // Always start BLE for configuration
  Serial.println("Starting BLE for configuration...");
  setupBLE();
  bleEnabled = true;
  
  // Check if we should auto-start web server
  preferences.begin("config", true);
  bool configured = preferences.getBool("configured", false);
  preferences.end();
  
  if (configured) {
    Serial.println("Auto-starting web server...");
    if (connectToWiFi()) {
      setupWebServer();
      webServerEnabled = true;
      Serial.println("Web server started!");
    } else {
      Serial.println("WiFi connection failed - web server not started");
    }
  } else {
    Serial.println("Web server not configured - use BLE to configure");
  }
  
  Serial.println("\nSetup complete!");
}

void loop() {
  // Handle BLE if enabled
  if (bleEnabled) {
    if (!deviceConnected && oldDeviceConnected) {
      delay(500);
      pServer->startAdvertising();
      Serial.println("Restarted BLE advertising");
      oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
    }
  }
  
  // Handle web server if enabled
  if (webServerEnabled) {
    server.handleClient();
  }
  
  delay(10);
}
