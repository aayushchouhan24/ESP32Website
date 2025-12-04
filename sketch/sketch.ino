#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

#define SVC_UUID "12345678-1234-1234-1234-123456789abc"
#define CMD_UUID "12345678-1234-1234-1234-123456789abd"
#define STAT_UUID "12345678-1234-1234-1234-123456789abe"
#define DEV_NAME "ESP32C3"
#define BTN 9

bool ble=1,web=0,conn=0,old=0,relay=0;
Preferences p;
WebServer s(80);
WebSocketsClient w;
BLEServer* srv=0;
BLECharacteristic *cmd=0,*st=0;
String url,host="esp-web-deoloy.vercel.app",path="/api/relay",fn;
int port=443;
File fl;
size_t sz=0,rx=0;

void ss(const char*t,const char*m){
  if(!conn||!st)return;
  DynamicJsonDocument d(200);
  d["type"]=t;d["message"]=m;
  String o;serializeJson(d,o);
  st->setValue(o.c_str());st->notify();
}

String ct(String f){
  if(f.endsWith(".html"))return"text/html";
  if(f.endsWith(".css"))return"text/css";
  if(f.endsWith(".js"))return"application/javascript";
  if(f.endsWith(".json"))return"application/json";
  if(f.endsWith(".png"))return"image/png";
  if(f.endsWith(".jpg")||f.endsWith(".jpeg"))return"image/jpeg";
  if(f.endsWith(".gif"))return"image/gif";
  if(f.endsWith(".svg"))return"image/svg+xml";
  if(f.endsWith(".ico"))return"image/x-icon";
  return"text/plain";
}

void cr();
void we(WStype_t,uint8_t*,size_t);

void hc(const char*c){
  DynamicJsonDocument d(800);
  if(deserializeJson(d,c))return;
  const char*t=d["type"];
  
  if(!strcmp(t,"wifi")){
    const char*ss=d["ssid"],*pw=d["pass"];
    if(!ss||!pw)return;
    p.begin("wifi",0);p.putString("ssid",ss);p.putString("password",pw);p.end();
    WiFi.mode(WIFI_STA);WiFi.begin(ss,pw);
    int a=0;while(WiFi.status()!=WL_CONNECTED&&a++<20)delay(500);
    if(WiFi.status()==WL_CONNECTED){
      DynamicJsonDocument sd(200);
      sd["type"]="wifi_status";sd["connected"]=1;sd["ip"]=WiFi.localIP().toString();
      sd["ssid"]=ss;sd["rssi"]=WiFi.RSSI();
      String o;serializeJson(sd,o);
      if(conn&&st){st->setValue(o.c_str());st->notify();}
      WiFi.disconnect();
    }
  }
  else if(!strcmp(t,"file_meta")){
    const char*n=d["name"];if(!n)return;
    if(fl)fl.close();
    fn=n;if(!fn.startsWith("/"))fn="/"+fn;
    if(SPIFFS.exists(fn))SPIFFS.remove(fn);
    fl=SPIFFS.open(fn,FILE_WRITE);if(!fl)return;
    sz=d["size"];rx=0;ss("success","Ready");
  }
  else if(!strcmp(t,"file_chunk")){
    if(!fl)return;
    JsonArray da=d["data"];if(da.isNull())return;
    size_t cs=da.size();
    uint8_t*b=new uint8_t[cs];
    for(size_t i=0;i<cs;i++)b[i]=da[i].as<uint8_t>();
    size_t wr=fl.write(b,cs);delete[]b;
    if(wr!=cs){fl.close();return;}
    rx+=wr;
    DynamicJsonDocument sd(200);
    sd["type"]="file_progress";sd["name"]=fn;
    sd["bytes_received"]=rx;sd["total_bytes"]=sz;
    String o;serializeJson(sd,o);
    st->setValue(o.c_str());st->notify();
    if(rx>=sz){fl.close();ss("success","Saved");fn="";rx=0;sz=0;}
  }
  else if(!strcmp(t,"reboot")){
    const char*m=d["mode"];
    if(!strcmp(m,"web_server")){
      if(!web){
        p.begin("wifi",1);
        String ss=p.getString("ssid",""),pw=p.getString("password","");
        p.end();
        if(ss.length()>0){
          WiFi.mode(WIFI_STA);WiFi.begin(ss.c_str(),pw.c_str());
          int a=0;while(WiFi.status()!=WL_CONNECTED&&a++<30)delay(500);
          if(WiFi.status()==WL_CONNECTED){
            s.onNotFound([](){
              String pt=s.uri();if(pt=="/")pt="/index.html";
              if(SPIFFS.exists(pt)){
                File f=SPIFFS.open(pt,"r");
                s.streamFile(f,ct(pt));f.close();
              }else s.send(404,"text/plain","Not Found");
            });
            s.begin();web=1;
            DynamicJsonDocument sd(200);
            sd["type"]="device_status";sd["webserver_active"]=1;
            sd["wifi_connected"]=1;sd["ip"]=WiFi.localIP().toString();
            String o;serializeJson(sd,o);
            if(conn&&st){st->setValue(o.c_str());st->notify();}
          }
        }
      }
    }
  }
  else if(!strcmp(t,"list_files")){
    DynamicJsonDocument ld(800);
    ld["type"]="file_list";
    JsonArray fa=ld.createNestedArray("files");
    File r=SPIFFS.open("/"),f=r.openNextFile();
    while(f){
      JsonObject fo=fa.createNestedObject();
      fo["name"]=String(f.name());fo["size"]=f.size();
      f=r.openNextFile();
    }
    String o;serializeJson(ld,o);
    st->setValue(o.c_str());st->notify();
  }
  else if(!strcmp(t,"relay_config")){
    const char*h=d["host"];
    if(!h||!strlen(h))return;
    int pt=d["port"]|443;
    const char*ph=d["path"]|"/api/relay";
    p.begin("relay",0);p.putString("host",h);
    p.putInt("port",pt);p.putString("path",ph);p.end();
    host=h;port=pt;path=ph;cr();
  }
  else if(!strcmp(t,"get_status")){
    DynamicJsonDocument sd(350);
    sd["type"]="device_status";sd["ble_active"]=ble;
    sd["webserver_active"]=web;
    sd["wifi_connected"]=WiFi.status()==WL_CONNECTED;
    p.begin("wifi",1);
    String ss=p.getString("ssid","");p.end();
    sd["has_wifi_credentials"]=ss.length()>0;
    if(ss.length()>0)sd["saved_ssid"]=ss;
    if(WiFi.status()==WL_CONNECTED){
      sd["ip"]=WiFi.localIP().toString();
      sd["ssid"]=WiFi.SSID();sd["rssi"]=WiFi.RSSI();
    }
    sd["relay_connected"]=relay;
    if(url.length()>0)sd["public_url"]=url;
    String o;serializeJson(sd,o);
    if(conn&&st){st->setValue(o.c_str());st->notify();}
  }
}

class SC:public BLEServerCallbacks{
  void onConnect(BLEServer*){conn=1;}
  void onDisconnect(BLEServer*){conn=0;}
};

class CC:public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic*p){
    String v=String(p->getValue().c_str());
    if(v.length()>0)hc(v.c_str());
  }
};

void cr(){
  if(WiFi.status()!=WL_CONNECTED||host.length()==0)return;
  w.beginSSL(host,port,path);w.onEvent(we);w.setReconnectInterval(5000);
}

void we(WStype_t tp,uint8_t*pl,size_t ln){
  if(tp==WStype_DISCONNECTED){relay=0;url="";}
  else if(tp==WStype_CONNECTED){
    relay=1;
    DynamicJsonDocument d(150);
    d["type"]="register";d["deviceId"]="ESP32-"+WiFi.macAddress();
    String o;serializeJson(d,o);w.sendTXT(o);
  }
  else if(tp==WStype_TEXT){
    DynamicJsonDocument d(1500);
    if(deserializeJson(d,pl,ln))return;
    const char*mt=d["type"];
    if(!strcmp(mt,"registered")){
      url=d["publicUrl"].as<String>();
      if(conn&&st){
        DynamicJsonDocument sd(200);
        sd["type"]="relay_status";sd["connected"]=1;sd["public_url"]=url;
        String o;serializeJson(sd,o);
        st->setValue(o.c_str());st->notify();
      }
    }
    else if(!strcmp(mt,"http_request")){
      int rid=d["requestId"];
      String pt=d["path"].as<String>(),body;
      int code=200;String ctp="text/html";
      if(pt=="/")pt="/index.html";
      if(SPIFFS.exists(pt)){
        File f=SPIFFS.open(pt,"r");
        if(f){body=f.readString();ctp=ct(pt);f.close();}
        else{code=500;body="Error";}
      }else{code=404;body="Not found";}
      DynamicJsonDocument rd(3500);
      rd["type"]="http_response";rd["requestId"]=rid;
      rd["statusCode"]=code;rd["headers"]["Content-Type"]=ctp;
      rd["body"]=body;
      String o;serializeJson(rd,o);w.sendTXT(o);
    }
  }
}

void setup(){
  if(!SPIFFS.begin(1))return;
  pinMode(BTN,INPUT_PULLUP);
  BLEDevice::init(DEV_NAME);
  srv=BLEDevice::createServer();srv->setCallbacks(new SC());
  BLEService*svc=srv->createService(SVC_UUID);
  cmd=svc->createCharacteristic(CMD_UUID,BLECharacteristic::PROPERTY_WRITE);
  cmd->setCallbacks(new CC());
  st=svc->createCharacteristic(STAT_UUID,BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
  st->addDescriptor(new BLE2902());
  svc->start();
  BLEAdvertising*adv=BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);adv->setScanResponse(1);
  BLEDevice::startAdvertising();
  p.begin("wifi",1);
  String ss=p.getString("ssid",""),pw=p.getString("password","");
  p.end();
  if(ss.length()>0){
    WiFi.mode(WIFI_STA);WiFi.begin(ss.c_str(),pw.c_str());
    int a=0;while(WiFi.status()!=WL_CONNECTED&&a++<30)delay(500);
    if(WiFi.status()==WL_CONNECTED){
      s.onNotFound([](){
        String pt=s.uri();if(pt=="/")pt="/index.html";
        if(SPIFFS.exists(pt)){
          File f=SPIFFS.open(pt,"r");
          s.streamFile(f,ct(pt));f.close();
        }else s.send(404,"text/plain","Not Found");
      });
      s.begin();web=1;
      p.begin("relay",1);
      host=p.getString("host","esp-web-deoloy.vercel.app");
      port=p.getInt("port",443);
      path=p.getString("path","/api/relay");
      p.end();
      if(host.length()>0)cr();
    }
  }
}

void loop(){
  if(ble){
    if(!conn&&old){delay(500);srv->startAdvertising();old=conn;}
    if(conn&&!old)old=conn;
  }
  if(web)s.handleClient();
  if(host.length()>0&&WiFi.status()==WL_CONNECTED)w.loop();
  delay(10);
}
