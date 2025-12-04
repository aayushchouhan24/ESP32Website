# ESP32 C3 Firmware - BLE Config & Web Server

## 📖 Overview

This Arduino sketch implements a dual-mode system for the ESP32 C3 Mini:

1. **BLE Configuration Mode** - Accepts configuration via Web Bluetooth
2. **Web Server Mode** - Serves uploaded website from SPIFFS

## 🔧 Hardware Requirements

- ESP32-C3-Mini-1 module
- USB cable for programming and power
- Computer with Arduino IDE

## 📚 Required Libraries

Install via Arduino Library Manager:

- **ESP32 Board Package** (by Espressif Systems)
  - Version: 2.0.11 or later
  - Includes: WiFi, WebServer, BLE, SPIFFS, Preferences

- **ArduinoJson** (by Benoit Blanchon)
  - Version: 6.21.0 or later
  - Used for: JSON command parsing

## ⚙️ Arduino IDE Configuration

### Board Settings

```
Board: "ESP32C3 Dev Module"
USB CDC On Boot: "Enabled"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
Upload Speed: "921600"
```

### Port Selection

- Windows: Usually `COM3`, `COM4`, etc.
- Mac/Linux: Usually `/dev/ttyUSB0` or `/dev/ttyACM0`

## 🚀 Upload Instructions

1. Connect ESP32 C3 to computer via USB
2. Open `sketch.ino` in Arduino IDE
3. Select correct board and port (Tools menu)
4. Click Upload button (→)
5. Wait for "Done uploading" message
6. Open Serial Monitor (115200 baud)

## 📡 Operating Modes

### BLE Configuration Mode

**Activated when:**
- First boot (no configuration saved)
- BOOT button held during power-on
- Reboot command with mode "ble_config"

**Behavior:**
- Initializes SPIFFS filesystem
- Starts BLE advertising as "ESP32C3-Admin"
- Accepts commands via BLE characteristics
- Saves WiFi credentials to NVS
- Receives and stores website files
- Waits for reboot command

**BLE Service:**
```cpp
Service UUID:  12345678-1234-1234-1234-123456789abc
Command UUID:  12345678-1234-1234-1234-123456789abd  (Write)
Status UUID:   12345678-1234-1234-1234-123456789abe  (Read + Notify)
```

### Web Server Mode

**Activated when:**
- Configuration flag is set in NVS
- Successfully connects to WiFi

**Behavior:**
- Reads WiFi credentials from NVS
- Connects to WiFi network (30 second timeout)
- Starts HTTP server on port 80
- Serves files from SPIFFS
- Maps "/" to "/index.html"
- Provides "/status" endpoint

## 📋 Supported Commands

All commands use JSON format via BLE Command Characteristic.

### 1. WiFi Configuration

```json
{
  "type": "wifi",
  "ssid": "YourNetworkName",
  "pass": "YourPassword"
}
```

**Response:**
```json
{
  "type": "success",
  "message": "WiFi credentials saved"
}
```

### 2. File Metadata

Must be sent before file chunks.

```json
{
  "type": "file_meta",
  "name": "index.html",
  "size": 10240
}
```

**Response:**
```json
{
  "type": "success",
  "message": "Ready to receive file"
}
```

### 3. File Chunk

Send file data in chunks (max 400 bytes per chunk).

```json
{
  "type": "file_chunk",
  "name": "index.html",
  "offset": 0,
  "data": [72, 84, 77, 76, ...]
}
```

**Response:**
```json
{
  "type": "file_progress",
  "name": "index.html",
  "bytes_received": 400,
  "total_bytes": 10240
}
```

### 4. Reboot

```json
{
  "type": "reboot",
  "mode": "web_server"
}
```

**Response:**
```json
{
  "type": "success",
  "message": "Rebooting to web server mode..."
}
```

### 5. List Files

```json
{
  "type": "list_files"
}
```

**Response:**
```json
{
  "type": "file_list",
  "files": [
    {"name": "/index.html", "size": 1024},
    {"name": "/style.css", "size": 512}
  ]
}
```

## 🌐 Web Server Endpoints

### Root Path
```
GET /
```
Serves `/index.html` from SPIFFS

### Static Files
```
GET /style.css
GET /script.js
GET /image.png
```
Serves files from SPIFFS with appropriate Content-Type

### Status Endpoint
```
GET /status
```
Returns JSON:
```json
{
  "device": "ESP32C3-Admin",
  "mode": "web_server",
  "ip": "192.168.1.150",
  "ssid": "YourNetwork",
  "rssi": -45,
  "files": ["/index.html", "/style.css"]
}
```

## 💾 Storage

### SPIFFS Partition
- Size: 1.5 MB (1,572,864 bytes)
- Format: SPIFFS (SPI Flash File System)
- Auto-formats on first boot if needed
- Stores uploaded website files

### NVS (Non-Volatile Storage)
- Namespace "wifi": Stores SSID and password
- Namespace "config": Stores configuration flag
- Survives reboots and power cycles

## 🔍 Serial Monitor Output

### Boot Messages
```
=================================
ESP32 C3 - BLE Config & Web Server
=================================

SPIFFS mounted successfully
Files in SPIFFS:
  /index.html (1024 bytes)
  /style.css (512 bytes)

Mode: WEB_SERVER_MODE
Connecting to WiFi: YourNetwork
.....
WiFi connected!
IP address: 192.168.1.150
HTTP server started on port 80
Setup complete!
```

### BLE Mode Messages
```
Mode: BLE_CONFIG_MODE
Initializing BLE...
BLE advertising started
Device name: ESP32C3-Admin

BLE Client connected
Received command: wifi
WiFi credentials saved
Received command: file_meta
File meta received: /index.html (1024 bytes)
Chunk received: 400 / 1024 bytes
Chunk received: 800 / 1024 bytes
Chunk received: 1024 / 1024 bytes
File transfer complete: /index.html
```

## 🐛 Debugging Tips

### Enable Verbose Output

Add to `setup()`:
```cpp
Serial.setDebugOutput(true);
esp_log_level_set("*", ESP_LOG_VERBOSE);
```

### Check SPIFFS Status

Add to `loop()`:
```cpp
size_t totalBytes = SPIFFS.totalBytes();
size_t usedBytes = SPIFFS.usedBytes();
Serial.printf("SPIFFS: %d / %d bytes used\n", usedBytes, totalBytes);
```

### Monitor BLE Events

Uncomment debug lines in BLE callbacks for detailed connection info.

### Check WiFi Signal

In Web Server mode:
```cpp
int32_t rssi = WiFi.RSSI();
Serial.printf("WiFi Signal: %d dBm\n", rssi);
```

## ⚡ Performance Notes

- **BLE MTU**: 512 bytes (negotiated)
- **Chunk Size**: 400 bytes (safe for BLE)
- **Upload Speed**: ~2-5 KB/s via BLE
- **Web Server**: Can handle 4-8 concurrent connections
- **Flash Write Speed**: ~100 KB/s

## 🔒 Security Considerations

### Current Implementation
- ✅ WiFi credentials stored in encrypted NVS
- ✅ BLE advertising is discoverable
- ⚠️ No BLE pairing/bonding
- ⚠️ No BLE encryption
- ⚠️ No web server authentication

### Production Recommendations
1. Implement BLE pairing with passkey
2. Add web server authentication
3. Use HTTPS (requires certificate)
4. Implement OTA updates
5. Add firewall rules

## 📝 Customization

### Change BLE Device Name
```cpp
#define DEVICE_NAME "MyESP32Device"
```

### Change UUIDs
```cpp
#define SERVICE_UUID        "your-uuid-here"
#define COMMAND_CHAR_UUID   "your-uuid-here"
#define STATUS_CHAR_UUID    "your-uuid-here"
```

### Change Web Server Port
```cpp
WebServer server(8080);  // Instead of 80
```

### Modify Chunk Size
```cpp
#define MAX_CHUNK_SIZE 200  // Smaller chunks for reliability
```

## 🔄 Firmware Updates

### Over USB
1. Connect via USB
2. Upload new sketch via Arduino IDE
3. SPIFFS files are preserved

### OTA (Future Enhancement)
Not currently implemented. Could be added using ArduinoOTA library.

## 📊 Memory Usage

Typical compilation results:
```
Sketch uses: ~800KB of program space
Global variables use: ~40KB of RAM
SPIFFS partition: 1.5MB for files
Free heap after boot: ~300KB
```

## 🛠️ Troubleshooting

### Upload Fails
- Hold BOOT button during upload
- Try lower upload speed (115200)
- Check USB cable quality
- Install USB drivers (CP2102/CH340)

### BLE Not Starting
- Check "Partition Scheme" includes Bluetooth
- Ensure ESP32 board package is up to date
- Try power cycle after upload

### SPIFFS Mount Failed
- Reupload with "Erase All Flash Before Sketch Upload" enabled
- Check partition scheme includes SPIFFS
- Verify flash size setting matches hardware

### WiFi Won't Connect
- Check 2.4GHz network (ESP32 C3 doesn't support 5GHz)
- Verify credentials are correct
- Check WiFi signal strength
- Try static IP if DHCP fails

## 📚 Further Reading

- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [Arduino-ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [Web Bluetooth API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API)
- [ArduinoJson Documentation](https://arduinojson.org/)

---

**Version**: 1.0  
**Last Updated**: December 2025  
**Tested On**: ESP32-C3-Mini-1  
**Arduino Core**: 2.0.11+
