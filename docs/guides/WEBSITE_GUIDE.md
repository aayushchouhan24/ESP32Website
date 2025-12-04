# Creating Websites for ESP32

This guide shows you how to create and prepare websites for deployment to your ESP32 C3.

## 📐 Size Constraints

- **Maximum total size**: 1.5 MB (SPIFFS partition)
- **Recommended**: Keep under 1 MB for safety
- **File count**: No hard limit, but fewer is better

## ✅ Best Practices

### 1. Structure
```
my-website/
├── index.html          (Required - entry point)
├── style.css           (Optional - styles)
├── script.js           (Optional - interactivity)
└── assets/
    ├── logo.png        (Optional - images)
    └── icon.svg        (Optional - icons)
```

### 2. HTML Guidelines

**✅ DO:**
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My ESP32 Site</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <h1>Hello from ESP32!</h1>
    <script src="script.js"></script>
</body>
</html>
```

**❌ AVOID:**
- External CDN links (no internet on ESP32)
- Large embedded images (use separate files)
- Server-side code (PHP, Python, etc.)

### 3. CSS Guidelines

**✅ DO:**
- Use internal stylesheet or separate .css file
- Minify for smaller size
- Use CSS variables for theming
- Keep it simple and efficient

**❌ AVOID:**
- External font CDNs (Google Fonts, etc.)
- Massive CSS frameworks (Bootstrap, Tailwind)
- Unused CSS rules

### 4. JavaScript Guidelines

**✅ DO:**
```javascript
// Simple, efficient code
document.getElementById('btn').addEventListener('click', () => {
    console.log('Button clicked!');
});

// Fetch from ESP32
fetch('/status')
    .then(r => r.json())
    .then(data => console.log(data));
```

**❌ AVOID:**
- Large frameworks (React, Vue, Angular)
- External library CDNs (jQuery, etc.)
- WebSocket connections (not implemented)

### 5. Images

**✅ DO:**
- Use SVG for icons (small file size)
- Compress PNG/JPG images
- Use appropriate dimensions (no 4K images!)
- Consider base64 encoding for tiny images

**❌ AVOID:**
- Uncompressed images
- Multiple large images
- Animated GIFs (large file size)

## 🎨 Example: Simple Dashboard

```html
<!-- index.html -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .card {
            background: white;
            padding: 40px;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
        }
        h1 {
            color: #667eea;
            margin-bottom: 20px;
        }
        .status {
            display: flex;
            justify-content: space-between;
            padding: 15px;
            background: #f7fafc;
            border-radius: 10px;
            margin: 10px 0;
        }
        .value {
            font-weight: bold;
            color: #667eea;
        }
        button {
            width: 100%;
            padding: 15px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            margin-top: 20px;
        }
        button:hover {
            background: #764ba2;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1>📊 ESP32 Dashboard</h1>
        <div class="status">
            <span>Device:</span>
            <span class="value" id="device">ESP32-C3</span>
        </div>
        <div class="status">
            <span>IP Address:</span>
            <span class="value" id="ip">Loading...</span>
        </div>
        <div class="status">
            <span>WiFi Network:</span>
            <span class="value" id="ssid">Loading...</span>
        </div>
        <div class="status">
            <span>Signal Strength:</span>
            <span class="value" id="rssi">Loading...</span>
        </div>
        <button onclick="refreshData()">Refresh Data</button>
    </div>

    <script>
        async function refreshData() {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                
                document.getElementById('device').textContent = data.device || 'ESP32-C3';
                document.getElementById('ip').textContent = data.ip || 'N/A';
                document.getElementById('ssid').textContent = data.ssid || 'N/A';
                document.getElementById('rssi').textContent = data.rssi ? `${data.rssi} dBm` : 'N/A';
            } catch (error) {
                console.error('Failed to fetch status:', error);
            }
        }
        
        // Auto-refresh on load
        refreshData();
        
        // Auto-refresh every 5 seconds
        setInterval(refreshData, 5000);
    </script>
</body>
</html>
```

**File size**: ~2.5 KB  
**Features**: Auto-refreshing dashboard, responsive design, no external dependencies

## 🔧 Example: Interactive Control Panel

```html
<!-- index.html -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Control</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <h1>🎛️ Control Panel</h1>
        
        <div class="control">
            <label>LED Status</label>
            <button id="ledToggle" class="btn">Turn On</button>
        </div>
        
        <div class="control">
            <label>Brightness</label>
            <input type="range" id="brightness" min="0" max="255" value="128">
            <span id="brightnessValue">128</span>
        </div>
        
        <div class="control">
            <label>Color</label>
            <input type="color" id="color" value="#667eea">
        </div>
        
        <div class="log" id="log"></div>
    </div>
    
    <script src="script.js"></script>
</body>
</html>
```

```css
/* style.css */
* { margin: 0; padding: 0; box-sizing: border-box; }

body {
    font-family: Arial, sans-serif;
    background: #1a202c;
    color: white;
    padding: 20px;
}

.container {
    max-width: 600px;
    margin: 0 auto;
}

h1 {
    text-align: center;
    margin-bottom: 30px;
    color: #667eea;
}

.control {
    background: #2d3748;
    padding: 20px;
    border-radius: 10px;
    margin-bottom: 15px;
}

label {
    display: block;
    margin-bottom: 10px;
    font-weight: bold;
}

.btn {
    width: 100%;
    padding: 12px;
    background: #667eea;
    color: white;
    border: none;
    border-radius: 8px;
    font-size: 16px;
    cursor: pointer;
}

.btn:hover {
    background: #764ba2;
}

input[type="range"] {
    width: 80%;
    margin-right: 10px;
}

input[type="color"] {
    width: 100%;
    height: 50px;
    border: none;
    border-radius: 8px;
    cursor: pointer;
}

.log {
    background: #000;
    padding: 15px;
    border-radius: 8px;
    margin-top: 20px;
    font-family: monospace;
    max-height: 200px;
    overflow-y: auto;
}
```

```javascript
/* script.js */
let ledState = false;

const ledToggle = document.getElementById('ledToggle');
const brightness = document.getElementById('brightness');
const brightnessValue = document.getElementById('brightnessValue');
const color = document.getElementById('color');
const log = document.getElementById('log');

function logMessage(msg) {
    const entry = document.createElement('div');
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
    log.appendChild(entry);
    log.scrollTop = log.scrollHeight;
}

ledToggle.addEventListener('click', () => {
    ledState = !ledState;
    ledToggle.textContent = ledState ? 'Turn Off' : 'Turn On';
    ledToggle.style.background = ledState ? '#48bb78' : '#667eea';
    logMessage(`LED ${ledState ? 'ON' : 'OFF'}`);
    
    // Here you would send command to ESP32
    // Example: fetch('/api/led', { method: 'POST', body: JSON.stringify({ state: ledState }) })
});

brightness.addEventListener('input', (e) => {
    brightnessValue.textContent = e.target.value;
});

brightness.addEventListener('change', (e) => {
    logMessage(`Brightness: ${e.target.value}`);
    // Send to ESP32
});

color.addEventListener('change', (e) => {
    logMessage(`Color: ${e.target.value}`);
    // Send to ESP32
});

logMessage('Control panel loaded');
```

**Total size**: ~4 KB  
**Features**: Interactive controls, real-time updates, clean UI

## 🌐 Testing Your Website

### 1. Test Locally
```bash
# Simple HTTP server
python -m http.server 8000

# Or use VS Code Live Server extension
```

### 2. Check File Sizes
```bash
# Linux/Mac
du -sh *

# Windows PowerShell
Get-ChildItem | Select-Object Name, @{Name="Size (KB)";Expression={[math]::Round($_.Length/1KB,2)}}
```

### 3. Optimize
- Minify HTML/CSS/JS
- Compress images
- Remove unused code
- Combine files where possible

## 📦 Preparing for Upload

### Checklist
- [ ] index.html exists
- [ ] Total size under 1.5 MB
- [ ] All links are relative
- [ ] No external CDN dependencies
- [ ] Images compressed
- [ ] Tested locally
- [ ] Files named correctly (case-sensitive)

### File Naming
- ✅ `index.html`, `style.css`, `script.js`
- ✅ `logo.png`, `icon.svg`
- ❌ `Index.HTML`, `Style.CSS` (ESP32 is case-sensitive)
- ❌ `my file.html` (avoid spaces)

## 🚀 Upload to ESP32

1. Open admin panel
2. Connect to ESP32
3. Select all website files
4. Click "Upload Files to ESP32"
5. Wait for completion
6. Deploy & Reboot

## 🔍 Debugging

### Common Issues

**Website doesn't load**
- Check if index.html was uploaded
- Verify files are in root (not in subfolder)
- Check serial monitor for errors

**CSS/JS not loading**
- Verify file paths in HTML
- Ensure files were uploaded
- Check browser console (F12)

**Images not showing**
- Check file size (too large?)
- Verify image path
- Use `/status` to list uploaded files

### Testing Endpoint
```javascript
// Check what files are on ESP32
fetch('/status')
    .then(r => r.json())
    .then(data => console.log('Files:', data.files));
```

## 💡 Pro Tips

1. **Start Simple** - Get basic HTML working first
2. **Test Everything** - Test locally before uploading
3. **Monitor Serial** - Watch for ESP32 errors
4. **Use /status** - Check uploaded files
5. **Keep Backups** - Save your website files locally
6. **Iterate Fast** - Small changes, quick uploads

## 📚 Resources

- [HTML Basics](https://developer.mozilla.org/en-US/docs/Learn/HTML)
- [CSS Guide](https://developer.mozilla.org/en-US/docs/Learn/CSS)
- [JavaScript Intro](https://developer.mozilla.org/en-US/docs/Learn/JavaScript)
- [Image Optimization](https://tinypng.com/)
- [CSS Minifier](https://cssminifier.com/)
- [JS Minifier](https://javascript-minifier.com/)

---

**Happy coding!** 🎉
