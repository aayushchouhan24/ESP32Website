# ESP32 Web Server Admin Panel

A complete BLE-based configuration system for ESP32 C3 with web server deployment capabilities. Configure WiFi, upload website files, and deploy - all via Bluetooth. Optional internet access through integrated ngrok relay.

## ✨ Features

- **🔵 BLE Configuration** - Configure via Web Bluetooth (Chrome/Edge)
- **📁 File Upload** - Upload website files directly to ESP32 flash
- **🌐 Web Server** - ESP32 serves your website on local network
- **🌍 Internet Access** - Optional ngrok integration for public access
- **⚡ Dual Mode** - BLE and web server run simultaneously
- **💾 Auto-Save** - WiFi credentials persist across reboots

## 🚀 Quick Start

### 1. Hardware Setup

- ESP32 C3 Mini
- USB-C cable for programming

### 2. Upload Firmware

1. Install Arduino IDE
2. Install ESP32 board support
3. Install required libraries:
   - **WebSocketsClient** by Markus Sattler
   - WiFi, WebServer, BLEDevice, SPIFFS, Preferences, ArduinoJson (built-in)
4. Open `sketch/sketch.ino`
5. Select ESP32C3 Dev Module
6. Upload

### 3. Deploy Admin Panel

#### Option A: Vercel (Recommended)

```bash
npm i -g vercel
vercel
```

Add environment variable in Vercel dashboard:
- `NGROK_AUTH_TOKEN` (get from ngrok.com - free)

#### Option B: Local Development

```bash
pnpm install
pnpm dev
```

### 4. Configure ESP32

1. Open admin panel in Chrome/Edge
2. Click "Connect via Bluetooth"
3. Configure WiFi credentials
4. Upload website files (must include `index.html`)
5. Click "Start Web Server"
6. Optional: Click "Connect to Internet Relay" for public access

## 📁 Project Structure

```
├── api/
│   └── relay.js              # Vercel serverless ngrok relay
├── dist/                     # Built admin panel
├── docs/                     # Documentation
├── public/
│   └── demo-*                # Example website files
├── sketch/
│   └── sketch.ino            # ESP32 firmware
├── src/
│   ├── main.js               # Admin panel logic
│   ├── style.css             # Dark theme styles
│   └── counter.js            # Demo counter
├── index.html                # Admin panel HTML
├── package.json              # Dependencies
└── vercel.json               # Vercel configuration
```

## 📖 Documentation

- **[VERCEL_DEPLOY.md](docs/VERCEL_DEPLOY.md)** - Deploy to Vercel with ngrok
- **[QUICKSTART.md](docs/QUICKSTART.md)** - Quick setup guide
- **[TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)** - Common issues

## 🎨 UI Theme

Dark theme with neon accents:
- **Background**: Pure black (#0a0a0a)
- **Accent**: Neon yellow (#d0ff00)
- **Success**: Neon green (#00ff88)
- **Monospace**: Courier New for logs

## 🔧 Tech Stack

**ESP32 Firmware:**
- Arduino framework
- BLE (Web Bluetooth compatible)
- SPIFFS (1.5MB internal flash)
- WebSocket client for relay

**Admin Panel:**
- Vite (build tool)
- Vanilla JavaScript
- Web Bluetooth API
- Vercel (hosting)

**Relay Server:**
- Vercel Serverless Functions
- Ngrok tunneling
- WebSocket relay

## 📝 License

MIT License - feel free to use in your projects!

## 🤝 Contributing

Contributions welcome! Please open an issue or PR.

## ⚠️ Browser Compatibility

Web Bluetooth requires:
- Chrome 56+
- Edge 79+
- Opera 43+

**Not supported:** Firefox, Safari

## 🆘 Support

Having issues? Check [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) or open an issue on GitHub.

---

**Made with ❤️ for ESP32 enthusiasts**
