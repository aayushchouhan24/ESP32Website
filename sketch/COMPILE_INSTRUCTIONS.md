# ESP32-C3 Compilation Instructions

## Important: Change Partition Scheme

The sketch requires more than the default 1.3MB program space. To compile successfully:

### In Arduino IDE:

1. **Select Board**: Tools > Board > ESP32 Arduino > ESP32C3 Dev Module
2. **Change Partition Scheme**: Tools > Partition Scheme > **"Huge APP (3MB No OTA/1MB SPIFFS)"**
   - This gives you ~3MB for program code and 1MB for SPIFFS storage
3. **Set Flash Size**: Tools > Flash Size > 4MB (or higher if your board supports it)
4. **Optimization**: Tools > Core Debug Level > None (reduces binary size)

### Alternative Partition Schemes (if Huge APP is not available):

- **"Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"** - Good balance
- **"No OTA (2MB APP/2MB SPIFFS)"** - If you don't need OTA updates

### After changing partition scheme:

The sketch should compile to approximately 1.2-1.4MB, which will fit in the larger APP partition.

## Optimization Applied:

- All Serial.print statements removed
- Variable names minimized
- DynamicJsonDocument used instead of Static for memory efficiency
- Code formatted compactly
- All comments removed

## Features Retained:

✅ BLE configuration
✅ WiFi connectivity
✅ File upload via BLE
✅ Web server
✅ WebSocket relay connection to Vercel
✅ Remote access via ngrok tunnel

## Current Code Size:

- Source: ~7KB
- Compiled (with Huge APP partition): Should fit in 3MB partition
