# ESP32 Web Server via Bluetooth

Complete system for configuring ESP32 C3 via Bluetooth and hosting websites from internal flash storage.

## Overview

**Dual-Mode Operation**: BLE configuration and web server run simultaneously - no reboot needed!

### Features

- 🔧 **BLE Configuration** - Configure WiFi and upload files via Web Bluetooth
- 🌐 **Web Server** - Host websites from 1.5MB internal flash storage  
- ⚡ **Live Updates** - Upload new files while website is running
- 📱 **Remote Admin** - Manage from any Web Bluetooth-enabled browser
- 🔒 **Persistent Storage** - WiFi credentials saved to NVS (encrypted)

## Quick Start

### 1. Upload Firmware

```bash
# Open sketch/sketch.ino in Arduino IDE
# Select Board: ESP32C3 Dev Module
# Upload to ESP32
```

### 2. Deploy Admin Panel

```bash
pnpm install
pnpm build
# Deploy dist/ folder to Vercel
```

### 3. Configure ESP32

1. Open admin panel in Chrome/Edge/Opera
2. Click "Connect via Bluetooth"
3. Enter WiFi credentials
4. Upload website files
5. Click "Start Web Server"

**Requirements**: Web Bluetooth (Chrome, Edge, Opera) • ESP32 C3 Mini • Arduino IDE

## Documentation

- 📖 [Full Documentation](docs/README.md)
- 🚀 [Quick Start Guide](docs/guides/QUICKSTART.md)
- 🌐 [Website Guide](docs/guides/WEBSITE_GUIDE.md)
- 🔧 [Troubleshooting](docs/TROUBLESHOOTING.md)

## Features

### Admin Panel
- Dark theme UI
- Real-time device status
- WiFi configuration
- File upload with progress
- One-click deployment

### ESP32 Firmware
- Dual-mode operation (BLE + Web Server)
- Persistent WiFi credentials
- 1.5MB SPIFFS storage
- Auto-reconnect
- OTA-ready architecture

## Project Structure

```
ESP32Website/
├── sketch/sketch.ino    # ESP32 firmware
├── src/                 # Admin panel source
├── dist/                # Built admin panel
├── docs/                # Documentation
└── public/              # Demo website
```

## Tech Stack

**ESP32**: Arduino, BLE, SPIFFS, Preferences (NVS)  
**Admin**: Vite, Web Bluetooth API, Vanilla JS  
**Hosting**: Vercel (admin panel)

## License

MIT

---

**Made with ESP32 C3 Mini** • [Documentation](docs/) • [Troubleshooting](docs/TROUBLESHOOTING.md)
- ✅ Chrome/Edge/Opera on Windows/Mac/Linux
- ✅ Chrome on Android
- ❌ Firefox, Safari, iOS browsers (not supported)
- ⚠️ Requires HTTPS (Vercel provides this automatically)

### ESP32 Firmware

#### Prerequisites

- Arduino IDE 2.x or Platform.IO
- ESP32 board support installed
- Required libraries:
  - ESP32 board package (by Espressif)
  - ArduinoJson (by Benoit Blanchon)

#### Installation

1. **Install Arduino IDE** (if not already installed)
   - Download from: https://www.arduino.cc/en/software

2. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

3. **Install Required Libraries**
   - Go to Sketch → Include Library → Manage Libraries
   - Search and install: "ArduinoJson" (version 6.x or later)

4. **Configure Board Settings**
   - Tools → Board → ESP32 Arduino → ESP32C3 Dev Module
   - Tools → USB CDC On Boot → Enabled
   - Tools → Flash Size → 4MB
   - Tools → Partition Scheme → Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
   - Tools → Upload Speed → 921600

5. **Upload Firmware**
   - Open `espCode/sketch.ino` in Arduino IDE
   - Connect ESP32 C3 via USB
   - Click Upload button
   - Wait for "Done uploading" message

6. **Monitor Serial Output**
   - Open Tools → Serial Monitor
   - Set baud rate to 115200
   - You should see boot messages and current mode

## 📖 Usage Guide

### Step 1: Prepare Your Website

Create your website files locally:
- `index.html` (required)
- `style.css` (optional)
- `script.js` (optional)
- Images, fonts, etc. (optional)

**File Size Limits**: Keep total files under 1.5MB (SPIFFS partition size)

### Step 2: Power On ESP32

1. Connect ESP32 C3 to power via USB
2. On first boot, it will start in **BLE Config Mode**
3. Check serial monitor - it should show:
   ```
   Mode: BLE_CONFIG_MODE
   BLE advertising started
   Device name: ESP32C3-Admin
   ```

### Step 3: Connect via Admin Panel

1. Open the Vercel admin panel in Chrome/Edge
2. Click **"Connect via Bluetooth"**
3. Select "ESP32C3-Admin" from the list
4. Wait for connection confirmation

### Step 4: Configure WiFi

1. Enter your WiFi SSID (network name)
2. Enter your WiFi password
3. Click **"Save WiFi Credentials"**
4. Wait for confirmation

### Step 5: Upload Website Files

1. Click **"Choose Files"** or drag and drop
2. Select your website files (must include index.html)
3. Review the file list
4. Click **"Upload Files to ESP32"**
5. Wait for upload progress to complete

### Step 6: Deploy & Reboot

1. Click **"Deploy & Reboot"**
2. Confirm the action
3. ESP32 will:
   - Disconnect from Bluetooth
   - Connect to your WiFi network
   - Start serving your website

### Step 7: Access Your Website

1. Check serial monitor for IP address:
   ```
   WiFi connected!
   IP address: 192.168.1.150
   HTTP server started on port 80
   ```

2. Open browser and navigate to the IP address
3. Your website should load!

## 🔧 Advanced Features

### Resetting to Config Mode

If you need to reconfigure the device:

**Method 1: Hold BOOT button**
- Press and hold the BOOT button during power-on
- Device will reset to BLE config mode

**Method 2: Send BLE command**
- Connect via admin panel
- Send reboot command with mode "ble_config"

### Checking Status

Access the status endpoint:
```
http://[ESP32-IP]/status
```

Returns JSON with:
- Device name
- Current mode
- IP address
- WiFi SSID and signal strength
- List of stored files

### Serial Commands

Monitor the serial output at 115200 baud for:
- Boot mode selection
- BLE connection status
- WiFi connection progress
- File upload progress
- HTTP server requests

## 🛠️ Troubleshooting

### Connection Issues

**"Bluetooth device not found"**
- Ensure ESP32 is powered on
- Check serial monitor shows "BLE advertising started"
- Make sure you're using a compatible browser
- Try refreshing the page and connecting again

**"Connection failed"**
- ESP32 might be in web server mode already
- Reset ESP32 while holding BOOT button
- Check Bluetooth is enabled on your device

### WiFi Issues

**"WiFi connection failed"**
- Double-check SSID and password
- Ensure WiFi network is 2.4GHz (ESP32 C3 doesn't support 5GHz)
- Check WiFi signal strength near ESP32
- Try resetting and reconfiguring

### Upload Issues

**"File upload failed"**
- Ensure total file size < 1.5MB
- Try uploading files one at a time
- Check serial monitor for specific error messages
- Reduce chunk size if experiencing timeouts

**"File write failed"**
- SPIFFS might be full
- Reformat SPIFFS (reset device)
- Reduce number/size of files

### Web Server Issues

**"Cannot access website"**
- Verify ESP32 is connected to WiFi (check serial monitor)
- Ensure you're on the same network as ESP32
- Check firewall settings
- Try accessing `/status` endpoint first

**"404 Not Found"**
- Ensure index.html was uploaded
- Check file names are correct (case-sensitive)
- Access `/status` to see list of stored files

## 📁 Project Structure

```
ESP32Website/
├── espCode/
│   └── sketch.ino          # ESP32 firmware (Arduino)
├── src/
│   ├── main.js             # Web Bluetooth client logic
│   └── style.css           # Admin panel styles
├── index.html              # Admin panel UI
├── package.json            # Node.js dependencies
└── README.md              # This file
```

## 🔌 BLE Protocol Reference

### Service UUID
```
12345678-1234-1234-1234-123456789abc
```

### Characteristics

**Command Characteristic** (Write)
```
UUID: 12345678-1234-1234-1234-123456789abd
```

**Status Characteristic** (Read + Notify)
```
UUID: 12345678-1234-1234-1234-123456789abe
```

### Command Format (JSON)

**WiFi Configuration:**
```json
{
  "type": "wifi",
  "ssid": "NetworkName",
  "pass": "password123"
}
```

**File Metadata:**
```json
{
  "type": "file_meta",
  "name": "index.html",
  "size": 10240
}
```

**File Chunk:**
```json
{
  "type": "file_chunk",
  "name": "index.html",
  "offset": 0,
  "data": [72, 84, 77, 76, ...]
}
```

**Reboot:**
```json
{
  "type": "reboot",
  "mode": "web_server"
}
```

### Status Format (JSON)

```json
{
  "type": "success|error|info|file_progress",
  "message": "Description of status"
}
```

## 🔐 Security Notes

- WiFi credentials are stored in ESP32's NVS (encrypted storage)
- BLE communication is not encrypted by default
- Consider implementing BLE pairing for production use
- Website is accessible to anyone on the same network
- No authentication on web server by default

## 📝 License

This project is provided as-is for educational and development purposes.

## 🤝 Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.

## 📧 Support

For issues and questions:
- Check the troubleshooting section above
- Review serial monitor output
- Check browser console for JavaScript errors
- Ensure all prerequisites are met

---

**Built with ❤️ for ESP32 C3 Mini | December 2025**
