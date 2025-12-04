# Documentation

## Getting Started

- **[Quick Start Guide](guides/QUICKSTART.md)** - Get up and running in 5 minutes
- **[Website Guide](guides/WEBSITE_GUIDE.md)** - Creating and uploading your website
- **[Vercel Deployment](guides/VERCEL_DEPLOY.md)** - Deploying the admin panel
- **[Troubleshooting](TROUBLESHOOTING.md)** - Common issues and solutions

## Advanced

- **[Internet Access (Ngrok)](guides/NGROK_SETUP.md)** - Making your ESP32 accessible from anywhere

## Technical Details

### System Architecture

The system operates in **dual-mode**:

1. **BLE Mode** (Always Active)
   - Advertises as "ESP32C3-Admin"
   - Accepts configuration commands
   - File upload via chunked transfer (100 bytes/chunk)
   - JSON-based command protocol

2. **Web Server Mode** (After Configuration)
   - Connects to configured WiFi
   - Serves files from SPIFFS (1.5MB)
   - HTTP server on port 80
   - Both modes run simultaneously

### File Structure

```
ESP32Website/
├── sketch/
│   └── sketch.ino          # ESP32 firmware
├── src/
│   ├── main.js             # Admin panel logic
│   └── style.css           # Dark theme UI
├── public/
│   └── demo-*              # Example website
├── docs/
│   ├── guides/             # Step-by-step guides
│   └── TROUBLESHOOTING.md  # Common issues
└── dist/                   # Built admin panel (deploy this)
```

### BLE Protocol

Commands are sent as JSON:

```json
// WiFi Configuration
{"type": "wifi", "ssid": "NetworkName", "pass": "password"}

// File Upload (1. Meta)
{"type": "file_meta", "name": "/index.html", "size": 1024}

// File Upload (2. Chunks)
{"type": "file_chunk", "data": "base64..."}

// Start Web Server
{"type": "reboot", "mode": "web_server"}

// Get Status
{"type": "get_status"}
```

### Storage Limits

- **SPIFFS**: 1.5MB total
- **Max file size**: Limited by available SPIFFS space
- **BLE chunk size**: 100 bytes (accounts for JSON overhead)
- **Recommended**: Minify HTML/CSS/JS before upload

## Support

For issues, check [Troubleshooting](TROUBLESHOOTING.md) first.
