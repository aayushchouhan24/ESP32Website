# 🌍 ESP32 Internet Access via Ngrok

## What's New

Your ESP32 can now be accessed from **anywhere on the internet** without port forwarding or router configuration!

## How It Works

```
Internet User → Ngrok URL → Relay Server → ESP32 (via WebSocket)
```

1. **Relay Server** runs on free cloud hosting (Render/Railway)
2. **Ngrok** creates public tunnel to relay server
3. **ESP32** connects OUT to relay (works through any firewall)
4. **Relay** forwards HTTP requests to ESP32 via WebSocket
5. **ESP32** sends responses back through relay

## Setup (5 minutes)

### 1. Deploy Relay Server

```bash
cd relay-server
npm install
```

See `relay-server/DEPLOY.md` for detailed instructions to deploy to Render (free).

### 2. Get Ngrok Token

- Sign up at https://ngrok.com (free)
- Get auth token from dashboard
- Add as environment variable: `NGROK_AUTH_TOKEN=your_token`

### 3. Upload Firmware

- Install WebSocketsClient library in Arduino IDE:
  - Tools → Manage Libraries → Search "WebSocketsClient" by Markus Sattler
- Upload `sketch/sketch.ino` to ESP32

### 4. Configure via Admin Panel

1. Connect via BLE
2. Configure WiFi
3. Upload website files
4. Click "Start Web Server"
5. In "Internet Access" section:
   - Enter relay server URL
   - Click "Connect to Relay"
6. You'll receive a public ngrok URL!

## Features

✅ **No port forwarding** - Works through any NAT/firewall
✅ **No static IP needed** - ESP32 connects outbound
✅ **Automatic reconnection** - Handles disconnects gracefully
✅ **Free hosting** - Render/Railway/Fly.io free tiers
✅ **BLE still works** - Update files anytime via Bluetooth
✅ **Dual-mode** - Local AND internet access simultaneously

## URLs You'll Get

- **Local**: `http://192.168.x.x` (same WiFi)
- **Internet**: `https://abc123.ngrok-free.app` (anywhere)

## Cost Breakdown

| Service | Free Tier | Paid Plan |
|---------|-----------|-----------|
| Ngrok | Random URL, changes on restart | $8/mo - Static domain |
| Render | Spins down after 15min idle | $7/mo - Always on |
| **Total** | **$0** | **$15/mo** for production use |

## Libraries Required

Add to Arduino IDE (Sketch → Include Library → Manage Libraries):
- **WebSocketsClient** by Markus Sattler (v2.3.6+)

Existing libraries (already used):
- WiFi, WebServer, BLEDevice, SPIFFS, Preferences, ArduinoJson

## Files Added

```
relay-server/
├── package.json       - Dependencies
├── server.js          - Relay server code
├── README.md          - Documentation
├── DEPLOY.md          - Deployment guide
└── .env.example       - Environment template
```

## Limitations

- Free ngrok URL changes on relay server restart
- Render free tier spins down after 15 min inactivity
- First request after spin-down takes ~30 seconds
- For production, upgrade to paid plans for stability

## Troubleshooting

**ESP32 won't connect to relay:**
- Check relay server is running
- Verify WebSocket URL is correct
- Check ESP32 serial monitor for errors

**Ngrok not working:**
- Verify `NGROK_AUTH_TOKEN` environment variable is set
- Check relay server logs
- Free ngrok accounts have rate limits

**Website not loading:**
- Ensure web server is started (check device status)
- Verify files are uploaded to ESP32
- Try local IP first to confirm web server works

## Next Steps

1. Deploy relay server to Render (free)
2. Upload firmware with WebSocket support
3. Configure relay in admin panel
4. Share your ngrok URL with the world! 🚀
