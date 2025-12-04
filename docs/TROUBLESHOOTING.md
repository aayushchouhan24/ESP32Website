# 🔧 Troubleshooting Guide

Comprehensive solutions for common issues with ESP32 C3 BLE Configuration & Web Server system.

## Table of Contents
- [Firmware Upload Issues](#firmware-upload-issues)
- [BLE Connection Issues](#ble-connection-issues)
- [WiFi Connection Issues](#wifi-connection-issues)
- [File Upload Issues](#file-upload-issues)
- [Web Server Issues](#web-server-issues)
- [Admin Panel Issues](#admin-panel-issues)
- [General Debugging](#general-debugging)

---

## Firmware Upload Issues

### ❌ "Failed to connect to ESP32"

**Symptoms:** Arduino IDE can't connect to ESP32 for upload

**Solutions:**
1. **Hold BOOT button** while clicking Upload, release after upload starts
2. Check USB cable (must support data, not just charging)
3. Try different USB port
4. Install USB drivers:
   - Windows: [CP2102 driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - Mac/Linux: Usually built-in
5. Reduce upload speed: Tools → Upload Speed → 115200
6. Check port selection: Tools → Port → Select correct COM/tty port

### ❌ "Compilation error"

**Symptoms:** Code won't compile

**Solutions:**
1. Install ArduinoJson library: Sketch → Include Library → Manage Libraries
2. Update ESP32 board package: Tools → Board → Boards Manager → Update esp32
3. Check board settings:
   ```
   Board: ESP32C3 Dev Module
   USB CDC On Boot: Enabled
   Flash Size: 4MB
   Partition Scheme: Default 4MB with spiffs
   ```
4. Clear Arduino cache: Delete `C:\Users\[User]\AppData\Local\Temp\arduino_*`

### ❌ "Sketch too large"

**Symptoms:** Code doesn't fit in flash

**Solutions:**
1. Check partition scheme: Must be "Default 4MB with spiffs"
2. Remove debug code if added
3. Use ESP32-C3 with 4MB flash (not 2MB version)

---

## BLE Connection Issues

### ❌ "Cannot find ESP32C3-Admin"

**Symptoms:** Device doesn't appear in Bluetooth device list

**Solutions:**
1. **Check ESP32 is in BLE mode:**
   - Open Serial Monitor (115200 baud)
   - Should see: "Mode: BLE_CONFIG_MODE" and "BLE advertising started"
   
2. **If in wrong mode:**
   - Hold BOOT button while powering on
   - Or upload firmware again
   
3. **Check Bluetooth is enabled:**
   - Windows: Settings → Bluetooth & devices → On
   - Mac: System Preferences → Bluetooth → On
   - Android: Settings → Connections → Bluetooth → On
   
4. **Power cycle ESP32:**
   - Unplug and replug USB
   - Check serial monitor shows BLE advertising

5. **Check browser:**
   - Must use Chrome, Edge, or Opera
   - Firefox and Safari don't support Web Bluetooth
   - iOS doesn't support Web Bluetooth

### ❌ "Connection failed"

**Symptoms:** Device found but connection fails

**Solutions:**
1. **Move devices closer together** (within 1 meter)
2. **Reduce interference:**
   - Turn off other Bluetooth devices nearby
   - Move away from WiFi routers
   - Avoid microwave ovens
   
3. **Restart browser:**
   - Close all tabs
   - Clear cache
   - Reopen admin panel
   
4. **Reset Bluetooth:**
   - Turn Bluetooth off and on
   - On Windows, remove paired devices in Settings
   
5. **Check serial monitor:**
   - Look for connection errors
   - ESP32 might be crashing

### ❌ "Disconnects immediately"

**Symptoms:** Connects but disconnects right away

**Solutions:**
1. Check ESP32 power supply (needs stable 5V, 500mA+)
2. Use shorter/better USB cable
3. Check serial monitor for crash/restart messages
4. Increase BLE connection interval in code (advanced)
5. Update ESP32 board package to latest version

---

## WiFi Connection Issues

### ❌ "WiFi connection failed"

**Symptoms:** ESP32 can't connect to WiFi after reboot

**Solutions:**
1. **Check WiFi band:**
   - ESP32 C3 only supports 2.4 GHz
   - Check router is broadcasting 2.4 GHz
   - Many routers have separate 2.4 GHz and 5 GHz networks
   
2. **Verify credentials:**
   - SSID is case-sensitive
   - Password is case-sensitive
   - No leading/trailing spaces
   
3. **Check WiFi signal strength:**
   - Move ESP32 closer to router
   - Check for obstacles (walls, metal objects)
   
4. **Router settings:**
   - Ensure WPA2 is enabled (not WPA3 only)
   - Check MAC filtering isn't blocking ESP32
   - Verify DHCP is enabled
   
5. **Serial monitor debugging:**
   ```
   WiFi.status() values:
   0 = WL_IDLE_STATUS
   1 = WL_NO_SSID_AVAIL (SSID not found)
   3 = WL_CONNECTED
   4 = WL_CONNECT_FAILED (wrong password)
   6 = WL_DISCONNECTED
   ```

### ❌ "Connected but no IP address"

**Symptoms:** WiFi connects but DHCP fails

**Solutions:**
1. **Check DHCP server:**
   - Router DHCP must be enabled
   - Check DHCP pool isn't full
   
2. **Try static IP (advanced):**
   ```cpp
   IPAddress local_IP(192, 168, 1, 100);
   IPAddress gateway(192, 168, 1, 1);
   IPAddress subnet(255, 255, 255, 0);
   WiFi.config(local_IP, gateway, subnet);
   ```
   
3. **Power cycle router:**
   - Sometimes DHCP server needs reset

### ❌ "Weak signal / frequent disconnects"

**Symptoms:** WiFi works but unstable

**Solutions:**
1. Move ESP32 closer to router
2. Add external antenna (if board supports it)
3. Reduce WiFi power saving:
   ```cpp
   WiFi.setSleep(false);
   ```
4. Check for interference (other 2.4 GHz devices)

---

## File Upload Issues

### ❌ "Upload stuck at 0%"

**Symptoms:** Progress bar doesn't move

**Solutions:**
1. Check BLE connection is active (status indicator green)
2. Refresh admin panel and reconnect
3. Try uploading smaller file first
4. Check serial monitor for errors
5. Reduce file size if very large

### ❌ "Upload very slow"

**Symptoms:** Upload works but takes forever

**Solutions:**
- **This is normal!** BLE transfers are slow (2-5 KB/s)
- For a 100 KB file, expect 20-50 seconds
- For a 1 MB file, expect 3-8 minutes
- Keep devices close together
- Don't move devices during upload
- Don't navigate away from admin panel

### ❌ "File write failed"

**Symptoms:** Error message during upload

**Solutions:**
1. **Check SPIFFS space:**
   - Total space: 1.5 MB
   - Delete old files before uploading new ones
   
2. **Reformat SPIFFS:**
   - Upload firmware with "Erase All Flash" enabled
   - Tools → Erase Flash → All Flash Contents
   
3. **Check file permissions:**
   - File names must not start with special characters
   - Avoid very long file names (max 31 characters)
   
4. **Check serial monitor:**
   - Look for specific SPIFFS error messages

### ❌ "File upload interrupted"

**Symptoms:** Upload stops partway through

**Solutions:**
1. Don't close browser tab during upload
2. Keep screen on (mobile devices)
3. Ensure stable power to ESP32
4. Reduce chunk size in code if persistent:
   ```javascript
   const MAX_CHUNK_SIZE = 200; // Smaller chunks
   ```
5. Try uploading files one at a time

---

## Web Server Issues

### ❌ "Cannot access website"

**Symptoms:** Browser can't load http://[IP]

**Solutions:**
1. **Check ESP32 is in web server mode:**
   - Serial monitor should show: "Mode: WEB_SERVER_MODE"
   - Should see IP address printed
   
2. **Verify you're on same network:**
   - Computer/phone must be on same WiFi as ESP32
   - Can't access from different network
   
3. **Check IP address:**
   - Must use IP from serial monitor
   - Try http://192.168.1.xxx (not https://)
   
4. **Test with /status endpoint:**
   - Try http://[IP]/status
   - Should return JSON
   - If this works, problem is with index.html
   
5. **Check firewall:**
   - Windows Firewall might block connection
   - Try temporarily disabling firewall
   - Add exception for port 80

### ❌ "404 Not Found"

**Symptoms:** Web server works but files not found

**Solutions:**
1. **Verify files were uploaded:**
   - Check http://[IP]/status
   - Should list uploaded files
   
2. **Check file names:**
   - Must have index.html (not index.htm)
   - File names are case-sensitive
   - Check for typos in HTML links
   
3. **Check file paths in HTML:**
   ```html
   ✅ <link href="style.css">
   ✅ <script src="script.js">
   ❌ <link href="/style.css"> (leading slash can cause issues)
   ```

### ❌ "CSS/JS not loading"

**Symptoms:** HTML loads but no styling/scripts

**Solutions:**
1. Check browser console (F12) for 404 errors
2. Verify CSS/JS files were uploaded
3. Check file paths are relative (not absolute)
4. Try accessing files directly:
   - http://[IP]/style.css
   - http://[IP]/script.js

### ❌ "Website loads partially"

**Symptoms:** Some content missing or broken

**Solutions:**
1. Check all files were uploaded
2. Large images might not load (size limit)
3. Check browser console for errors
4. Verify file sizes in /status endpoint

---

## Admin Panel Issues

### ❌ "Web Bluetooth not available"

**Symptoms:** Error about Web Bluetooth API

**Solutions:**
1. **Check browser:**
   - ✅ Chrome 56+ on Windows/Mac/Linux/Android
   - ✅ Edge 79+ on Windows/Mac/Linux/Android
   - ✅ Opera 43+ on Windows/Mac/Linux/Android
   - ❌ Firefox (not supported)
   - ❌ Safari / iOS (not supported)
   
2. **Check HTTPS:**
   - Web Bluetooth requires HTTPS (except localhost)
   - Vercel automatically provides HTTPS
   - Local testing: use http://localhost:5173 (allowed)
   
3. **Enable Bluetooth:**
   - Chrome flags: chrome://flags/#enable-web-bluetooth
   - Should be enabled by default

### ❌ "Admin panel won't load"

**Symptoms:** Vercel URL doesn't work

**Solutions:**
1. Check Vercel deployment status
2. Verify build succeeded
3. Try force refresh: Ctrl+F5 (Windows) or Cmd+Shift+R (Mac)
4. Clear browser cache
5. Try different browser

### ❌ "Local dev server won't start"

**Symptoms:** `pnpm dev` fails

**Solutions:**
1. Install dependencies: `pnpm install`
2. Check Node.js version (need 14+)
3. Delete node_modules and reinstall
4. Check port 5173 isn't in use
5. Try `pnpm run dev` instead

---

## General Debugging

### Serial Monitor Tips

**Enable verbose output:**
```cpp
// Add to setup()
Serial.setDebugOutput(true);
esp_log_level_set("*", ESP_LOG_VERBOSE);
```

**Common messages:**
- "BLE advertising started" = BLE mode active
- "WiFi connected!" = Successfully connected
- "HTTP server started" = Web server running
- "Guru Meditation Error" = Crash (note the address)

### Browser Console Tips

**Open console:** Press F12

**Common checks:**
```javascript
// Check Web Bluetooth support
navigator.bluetooth

// Check if device is connected
// (Look for connection logs)

// View network requests
// (Network tab - see failed requests)
```

### Testing Flow

1. **Test firmware:**
   - Flash and check serial monitor
   - Verify BLE mode or web server mode
   - Check for error messages

2. **Test admin panel:**
   - Open in Chrome
   - Check console for errors
   - Verify Web Bluetooth available

3. **Test connection:**
   - Try connecting to ESP32
   - Watch both serial monitor and browser console
   - Look for BLE errors

4. **Test file upload:**
   - Start with small file (<10 KB)
   - Watch progress carefully
   - Check serial monitor for write errors

5. **Test web server:**
   - Check /status endpoint first
   - Then test index.html
   - Use browser DevTools Network tab

### Getting Help

**When asking for help, provide:**
1. ESP32 board model (ESP32-C3-DevKitM-1, etc.)
2. Arduino IDE version and ESP32 board package version
3. Full serial monitor output
4. Browser console errors (F12)
5. Steps to reproduce the issue
6. What you've already tried

### Advanced Debugging

**Enable ESP32 core debug:**
```
Tools → Core Debug Level → Verbose
```

**Monitor heap memory:**
```cpp
void loop() {
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  delay(1000);
}
```

**Check SPIFFS:**
```cpp
size_t totalBytes = SPIFFS.totalBytes();
size_t usedBytes = SPIFFS.usedBytes();
Serial.printf("SPIFFS: %d / %d bytes used\n", usedBytes, totalBytes);
```

---

## Still Having Issues?

1. **Read the documentation:**
   - README.md - Full documentation
   - QUICKSTART.md - Step-by-step guide
   - espCode/README.md - Firmware details

2. **Check examples:**
   - Demo website in public/ folder
   - Works out of the box

3. **Try from scratch:**
   - Erase all flash
   - Upload firmware fresh
   - Test with demo website

4. **Hardware check:**
   - Try different USB cable
   - Try different power source
   - Check for damaged board

5. **Software check:**
   - Update Arduino IDE
   - Update ESP32 board package
   - Update browser

---

**Last Resort:** If nothing works, try a different ESP32 board - rarely, boards can be defective.

**Success Indicator:** You should see "HTTP server started on port 80" in serial monitor and be able to access http://[IP]/status

Good luck! 🍀
