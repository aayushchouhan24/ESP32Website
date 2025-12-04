#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define COMMAND_CHAR_UUID "12345678-1234-1234-1234-123456789abd"
#define STATUS_CHAR_UUID "12345678-1234-1234-1234-123456789abe"
#define DEVICE_NAME "ESP32C3-Admin"
#define CONFIG_BOOT_BUTTON 9
#define BLE_MTU_SIZE 512
#define MAX_CHUNK_SIZE 100

bool bleEnabled=true,webServerEnabled=false,deviceConnected=false,oldDeviceConnected=false;
Preferences preferences;
WebServer server(80);
BLEServer* pServer=nullptr;
BLECharacteristic *pCommandCharacteristic=nullptr,*pStatusCharacteristic=nullptr;
String currentFileName;
File currentFile;
size_t expectedFileSize=0,receivedBytes=0;

void sendStatus(const char*,const char*,JsonObject=JsonObject());
void handleBLECommand(const char*);
void handleWiFiConfig(JsonDocument&);
void handleFileMeta(JsonDocument&);
void handleFileChunk(JsonDocument&);
void handleReboot(JsonDocument&);
void handleListFiles();
String getContentType(String);

class ServerCallbacks:public BLEServerCallbacks{
  void onConnect(BLEServer*p){deviceConnected=true;sendStatus("info","Connected");}
  void onDisconnect(BLEServer*p){deviceConnected=false;}
};
class CommandCallbacks:public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic*c){
    String v=String(c->getValue().c_str());
    if(v.length()>0)handleBLECommand(v.c_str());
  }
};

// ==================== Utility Functions ====================

void sendStatus(const char*t,const char*m,JsonObject e){
  if(!deviceConnected||!pStatusCharacteristic)return;
  StaticJsonDocument<512>d;
  d["type"]=t;d["message"]=m;
  if(!e.isNull())for(JsonPair kv:e)d[kv.key()]=kv.value();
  String o;serializeJson(d,o);
  pStatusCharacteristic->setValue(o.c_str());
  pStatusCharacteristic->notify();
}

String getContentType(String f){
  if(f.endsWith(".html"))return "text/html";
  if(f.endsWith(".css"))return "text/css";
  if(f.endsWith(".js"))return "application/javascript";
  if(f.endsWith(".json"))return "application/json";
  if(f.endsWith(".png"))return "image/png";
  if(f.endsWith(".jpg")||f.endsWith(".jpeg"))return "image/jpeg";
  if(f.endsWith(".gif"))return "image/gif";
  if(f.endsWith(".svg"))return "image/svg+xml";
  if(f.endsWith(".ico"))return "image/x-icon";
  return "text/plain";
}
void handleBLECommand(const char* c){
  StaticJsonDocument<1024>doc;
  if(deserializeJson(doc,c)){sendStatus("error","Invalid JSON");return;}
  const char*t=doc["type"];
  if(!strcmp(t,"wifi"))handleWiFiConfig(doc);
  else if(!strcmp(t,"file_meta"))handleFileMeta(doc);
  else if(!strcmp(t,"file_chunk"))handleFileChunk(doc);
  else if(!strcmp(t,"reboot"))handleReboot(doc);
  else if(!strcmp(t,"list_files"))handleListFiles();
  else if(!strcmp(t,"get_status")){
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

void handleWiFiConfig(JsonDocument&d){
  const char*s=d["ssid"],*p=d["pass"];
  if(!s||!p){sendStatus("error","Missing SSID/pass");return;}
  preferences.begin("wifi",0);
  preferences.putString("ssid",s);
  preferences.putString("password",p);
  preferences.end();
  sendStatus("success","WiFi saved");
  WiFi.mode(WIFI_STA);
  WiFi.begin(s,p);
  
  for(int i=0;WiFi.status()!=WL_CONNECTED&&i<20;i++)delay(500);
  if(WiFi.status()==WL_CONNECTED){
    StaticJsonDocument<256>st;
    st["type"]="wifi_status";
    st["connected"]=1;
    st["ip"]=WiFi.localIP().toString();
    st["ssid"]=s;
    st["rssi"]=WiFi.RSSI();
    String o;serializeJson(st,o);
    if(deviceConnected&&pStatusCharacteristic){
      pStatusCharacteristic->setValue(o.c_str());
      pStatusCharacteristic->notify();
    }
    WiFi.disconnect();
  }else sendStatus("warning","Connection test failed");
}

void handleFileMeta(JsonDocument&d){
  const char*fn=d["name"];
  if(!fn){sendStatus("error","Missing filename");return;}
  if(currentFile)currentFile.close();
  currentFileName=fn;
  if(!currentFileName.startsWith("/"))currentFileName="/"+currentFileName;
  if(SPIFFS.exists(currentFileName))SPIFFS.remove(currentFileName);
  currentFile=SPIFFS.open(currentFileName,FILE_WRITE);
  if(!currentFile){sendStatus("error","Failed to create file");return;}
  expectedFileSize=d["size"];
  receivedBytes=0;
  sendStatus("success","Ready");
}

void handleFileChunk(JsonDocument&d){
  if(!currentFile){sendStatus("error","No active transfer");return;}
  JsonArray a=d["data"];
  if(a.isNull()){sendStatus("error","Missing data");return;}
  size_t sz=a.size();
  uint8_t*b=new uint8_t[sz];
  for(size_t i=0;i<sz;i++)b[i]=a[i];
  size_t w=currentFile.write(b,sz);
  delete[]b;
  if(w!=sz){sendStatus("error","Write failed");currentFile.close();return;}
  receivedBytes+=w;
  StaticJsonDocument<256>s;
  s["type"]="file_progress";
  s["name"]=currentFileName;
  s["bytes_received"]=receivedBytes;
  s["total_bytes"]=expectedFileSize;
  String o;serializeJson(s,o);
  pStatusCharacteristic->setValue(o.c_str());
  pStatusCharacteristic->notify();
  if(receivedBytes>=expectedFileSize){
    currentFile.close();
    sendStatus("success","File saved");
    currentFileName="";receivedBytes=0;expectedFileSize=0;
  }
}

void handleReboot(JsonDocument&d){
  const char*m=d["mode"];
  if(!strcmp(m,"web_server")){
    if(!webServerEnabled){
      sendStatus("info","Starting server...");
      if(connectToWiFi()){
        setupWebServer();
        webServerEnabled=1;
        sendStatus("success","Server started");
        StaticJsonDocument<384>s;
        s["type"]="device_status";
        s["ble_active"]=bleEnabled;
        s["webserver_active"]=1;
        s["wifi_connected"]=1;
        s["ip"]=WiFi.localIP().toString();
        s["ssid"]=WiFi.SSID();
        String o;serializeJson(s,o);
        if(deviceConnected&&pStatusCharacteristic){
          pStatusCharacteristic->setValue(o.c_str());
          pStatusCharacteristic->notify();
        }
      }else sendStatus("error","WiFi failed");
    }else sendStatus("info","Already running");
    return;
  }
  else if (strcmp(m, "ble_config") == 0) {
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

void setupBLE(){
  BLEDevice::init(DEVICE_NAME);
  pServer=BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService*s=pServer->createService(SERVICE_UUID);
  pCommandCharacteristic=s->createCharacteristic(COMMAND_CHAR_UUID,BLECharacteristic::PROPERTY_WRITE);
  pCommandCharacteristic->setCallbacks(new CommandCallbacks());
  pStatusCharacteristic=s->createCharacteristic(STATUS_CHAR_UUID,BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
  pStatusCharacteristic->addDescriptor(new BLE2902());
  s->start();
  BLEAdvertising*a=BLEDevice::getAdvertising();
  a->addServiceUUID(SERVICE_UUID);
  a->setScanResponse(1);
  a->setMinPreferred(0x06);
  a->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}
bool connectToWiFi(){
  preferences.begin("wifi",1);
  String s=preferences.getString("ssid",""),p=preferences.getString("password","");
  preferences.end();
  if(!s.length())return 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(s.c_str(),p.c_str());
  for(int i=0;WiFi.status()!=WL_CONNECTED&&i<30;i++)delay(500);
  return WiFi.status()==WL_CONNECTED;
}

void handleFileRequest(){
  String p=server.uri();
  if(p=="/")p="/index.html";
  if(SPIFFS.exists(p)){
    File f=SPIFFS.open(p,"r");
    server.streamFile(f,getContentType(p));
    f.close();
  }else server.send(404,"text/plain","Not Found");
}
void handleStatus(){
  StaticJsonDocument<256>d;
  d["device"]=DEVICE_NAME;
  d["mode"]="web_server";
  d["ip"]=WiFi.localIP().toString();
  d["ssid"]=WiFi.SSID();
  d["rssi"]=WiFi.RSSI();
  JsonArray f=d.createNestedArray("files");
  File r=SPIFFS.open("/"),file=r.openNextFile();
  while(file){f.add(String(file.name()));file=r.openNextFile();}
  String o;serializeJson(d,o);
  server.send(200,"application/json",o);
}
void setupWebServer(){
  server.onNotFound(handleFileRequest);
  server.on("/status",handleStatus);
  server.begin();
}
void setup(){
  if(!SPIFFS.begin(1))return;
  pinMode(CONFIG_BOOT_BUTTON,INPUT_PULLUP);
  if(digitalRead(CONFIG_BOOT_BUTTON)==LOW){
    preferences.begin("config",0);
    preferences.putBool("configured",0);
    preferences.end();
    delay(1000);
  }
  setupBLE();
  bleEnabled=1;
  preferences.begin("config",1);
  bool c=preferences.getBool("configured",0);
  preferences.end();
  if(c&&connectToWiFi()){
    setupWebServer();
    webServerEnabled=1;
  }
}

void loop(){
  if(bleEnabled){
    if(!deviceConnected&&oldDeviceConnected){
      delay(500);
      pServer->startAdvertising();
      oldDeviceConnected=deviceConnected;
    }
    if(deviceConnected&&!oldDeviceConnected)oldDeviceConnected=deviceConnected;
  }
  if(webServerEnabled)server.handleClient();
  delay(10);
}
