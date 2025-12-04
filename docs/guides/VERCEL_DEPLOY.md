# 🚀 Deploy to Vercel with Built-in Ngrok Relay

## Everything in One Place!

The relay server is now **built into** the admin panel. One deployment = Admin Panel + Ngrok Relay!

## Quick Deploy (2 minutes)

### 1. Get Ngrok Auth Token

1. Go to https://ngrok.com (sign up free)
2. Go to https://dashboard.ngrok.com/get-started/your-authtoken
3. Copy your auth token

### 2. Deploy to Vercel

#### Option A: Deploy Button (Easiest)

[![Deploy with Vercel](https://vercel.com/button)](https://vercel.com/new/clone?repository-url=YOUR_GITHUB_REPO_URL)

#### Option B: Manual Deploy

```bash
# Install Vercel CLI
npm i -g vercel

# Deploy
cd D:\Projects\ESP32Website
vercel
```

### 3. Add Environment Variable

In Vercel dashboard:
1. Go to your project → Settings → Environment Variables
2. Add variable:
   - **Name**: `NGROK_AUTH_TOKEN`
   - **Value**: Your ngrok token from step 1
3. Click "Save"
4. Redeploy (Deployments → ... → Redeploy)

### 4. Use It!

1. Open your Vercel URL: `https://your-project.vercel.app`
2. Connect to ESP32 via BLE
3. Configure WiFi and start web server
4. Click "Connect to Internet Relay"
5. Get your public ngrok URL!

## What You Get

- ✅ Admin Panel at: `https://your-project.vercel.app`
- ✅ Relay API at: `https://your-project.vercel.app/api/relay`
- ✅ Ngrok tunnel created automatically
- ✅ ESP32 accessible worldwide!

## Files Structure

```
.
├── api/
│   └── relay.js          # Vercel serverless relay function
├── dist/                 # Built admin panel (auto-generated)
├── src/                  # Admin panel source
├── sketch/               # ESP32 firmware
├── vercel.json          # Vercel configuration
└── package.json         # Includes @ngrok/ngrok
```

## Cost

- **Vercel**: Free (hobby plan)
- **Ngrok**: Free (random URL)
- **Total**: $0

Upgrade options:
- Ngrok Pro ($8/mo): Static domain
- Vercel Pro ($20/mo): Better limits

## How It Works

```
User → Vercel (Admin Panel + Relay API)
            ↓
        Ngrok Tunnel
            ↓
        ESP32 (WebSocket)
```

1. You deploy once to Vercel
2. Admin panel and relay both deployed
3. ESP32 connects to Vercel's `/api/relay`
4. Vercel creates ngrok tunnel
5. Public access through ngrok URL

## Troubleshooting

**"NGROK_AUTH_TOKEN not found"**
- Add environment variable in Vercel dashboard
- Redeploy after adding

**ESP32 won't connect**
- Check WebSocket URL in ESP32 serial monitor
- Should connect to: `wss://your-project.vercel.app/api/relay`
- Verify ESP32 has internet access

**Ngrok URL not working**
- Check Vercel function logs
- Verify ngrok token is valid
- Free ngrok has rate limits (60 requests/min)

## Next Steps

1. Deploy to Vercel
2. Add `NGROK_AUTH_TOKEN` environment variable
3. Upload firmware to ESP32
4. Open admin panel
5. Connect and enjoy! 🎉
