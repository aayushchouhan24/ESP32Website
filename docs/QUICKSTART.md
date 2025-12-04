# 🚀 Quick Start Guide

Get your ESP32 C3 serving a website in 10 minutes!

## 📋 What You'll Need

- [ ] ESP32 C3 Mini development board
- [ ] USB cable (for programming and power)
- [ ] Computer with Arduino IDE installed
- [ ] Chrome, Edge, or Opera browser
- [ ] WiFi network (2.4GHz)

## 🔧 Step 1: Set Up Arduino IDE (5 minutes)

### Install Arduino IDE
Download from: https://www.arduino.cc/en/software

### Add ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search "esp32" and install **"esp32 by Espressif Systems"**

### Install ArduinoJson Library
1. Go to **Sketch → Include Library → Manage Libraries**
2. Search "ArduinoJson"
3. Install **"ArduinoJson by Benoit Blanchon"**

## 📡 Step 2: Flash ESP32 Firmware (2 minutes)

1. **Connect ESP32** to computer via USB

2. **Open the sketch**
   - Open `espCode/sketch.ino` in Arduino IDE

3. **Configure board settings**
   - Tools → Board → ESP32 Arduino → **ESP32C3 Dev Module**
   - Tools → USB CDC On Boot → **Enabled**
   - Tools → Flash Size → **4MB**
   - Tools → Partition Scheme → **Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)**
   - Tools → Port → Select your ESP32's port

4. **Upload**
   - Click the Upload button (→)
   - Wait for "Done uploading"

5. **Verify**
   - Open Serial Monitor (Tools → Serial Monitor)
   - Set baud rate to **115200**
   - You should see:
     ```
     Mode: BLE_CONFIG_MODE
     BLE advertising started
     Device name: ESP32C3-Admin
     ```

## 🌐 Step 3: Deploy Admin Panel (2 minutes)

### Option A: Test Locally (Fastest)
```bash
cd ESP32Website
pnpm install
pnpm dev
```
Open http://localhost:5173 in Chrome/Edge

### Option B: Deploy to Vercel (Recommended)
```bash
npm install -g vercel
vercel login
vercel
```
Use the provided URL

## 🎯 Step 4: Configure ESP32 (1 minute)

1. **Open admin panel** in Chrome, Edge, or Opera

2. **Connect to ESP32**
   - Click "Connect via Bluetooth"
   - Select "ESP32C3-Admin" from the list
   - Wait for "Connected" status

3. **Enter WiFi credentials**
   - SSID: Your network name
   - Password: Your WiFi password
   - Click "Save WiFi Credentials"

4. **Upload demo website**
   - Click "Choose Files"
   - Navigate to `public/` folder
   - Select these files:
     - demo-index.html (rename to index.html when uploading, or upload as-is and rename later)
     - demo-style.css (rename to style.css)
     - demo-script.js (rename to script.js)
   - Click "Upload Files to ESP32"
   - Wait for 100% completion

5. **Deploy**
   - Click "Deploy & Reboot"
   - Confirm the action

## ✅ Step 5: Access Your Website!

1. **Check Serial Monitor** for IP address:
   ```
   WiFi connected!
   IP address: 192.168.1.150
   ```

2. **Open browser** and navigate to the IP address
   ```
   http://192.168.1.150
   ```

3. **🎉 Success!** You're now viewing a website served from ESP32!

## 💡 Tips & Tricks

### Can't Find ESP32 in Bluetooth List?
- Check serial monitor shows "BLE advertising started"
- Power cycle the ESP32
- Make sure Bluetooth is enabled on your device

### WiFi Connection Failed?
- Ensure you're using 2.4GHz network (ESP32 doesn't support 5GHz)
- Double-check SSID and password
- Move ESP32 closer to router

### Upload Stuck or Slow?
- BLE transfers are slow (~2-5 KB/s), be patient
- Keep devices close together
- Don't navigate away from admin panel during upload

### Want to Upload Your Own Website?
- Prepare files: index.html (required), CSS, JS, images
- Keep total size under 1.5MB
- Use relative paths in HTML (e.g., `src="style.css"` not `src="/style.css"`)
- Test locally first

### Need to Reconfigure?
Hold BOOT button while powering on ESP32 to reset to BLE config mode

## 🎓 Next Steps

### Customize Your Website
1. Create your own HTML, CSS, JS files
2. Upload via admin panel
3. See changes immediately!

### Advanced Features
- Check `/status` endpoint for device info
- Monitor serial output for debugging
- Add more complex web applications
- Implement REST API endpoints

### Share Your Project
- Note down the IP address
- Anyone on same network can access it
- For internet access, set up port forwarding on your router

## 📚 Need Help?

- **Full Documentation**: See `README.md`
- **Firmware Details**: See `espCode/README.md`
- **Deployment Guide**: See `DEPLOYMENT.md`
- **Serial Monitor**: Check for error messages
- **Browser Console**: Press F12 to see logs

## ⚡ Common Issues & Solutions

| Problem | Solution |
|---------|----------|
| "Upload failed" | Hold BOOT button during upload |
| "Bluetooth not found" | Use Chrome/Edge, enable Bluetooth |
| "Connection timeout" | Keep ESP32 and device close |
| "File too large" | Reduce file size or compress images |
| "404 on website" | Ensure index.html was uploaded |
| "Can't connect to WiFi" | Check 2.4GHz network, verify password |

## 🎉 That's It!

You now have a fully functional ESP32 web server that can be configured and updated over Bluetooth!

**Total Time**: ~10 minutes  
**Difficulty**: Beginner-friendly  
**Cost**: Free (using free Vercel tier)

---

**Questions?** Check the full README.md for detailed troubleshooting and advanced usage.
