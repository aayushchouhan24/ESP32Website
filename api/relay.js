// Vercel Serverless Function for ESP32 Relay
import ngrok from '@ngrok/ngrok';
import WebSocket from 'ws';

let esp32Socket = null;
let activeConnections = new Map();
let ngrokListener = null;
let publicUrl = null;
let wss = null;

// Initialize ngrok tunnel
async function initNgrok() {
  if (ngrokListener) return publicUrl;
  
  try {
    ngrokListener = await ngrok.forward({
      addr: `https://${process.env.VERCEL_URL}`,
      authtoken: process.env.NGROK_AUTH_TOKEN,
    });
    
    publicUrl = ngrokListener.url();
    return publicUrl;
  } catch (error) {
    console.error('Ngrok error:', error);
    return null;
  }
}

export const config = {
  runtime: 'nodejs',
};

export default async function handler(req, res) {
  const upgradeHeader = req.headers['upgrade'];
  
  // WebSocket upgrade for ESP32
  if (upgradeHeader === 'websocket') {
    if (!wss) {
      wss = new WebSocket.Server({ noServer: true });
      
      wss.on('connection', async (socket) => {
        esp32Socket = socket;
        
        // Initialize ngrok and send URL to ESP32
        const url = await initNgrok();
        if (url) {
          socket.send(JSON.stringify({
            type: 'registered',
            success: true,
            publicUrl: url
          }));
        }
        
        socket.on('message', (data) => {
          try {
            const message = JSON.parse(data.toString());
            
            if (message.type === 'http_response') {
              const { requestId, statusCode, headers, body } = message;
              const connection = activeConnections.get(requestId);
              
              if (connection) {
                connection.resolve({
                  statusCode: statusCode || 200,
                  headers: headers || {},
                  body: body || ''
                });
                activeConnections.delete(requestId);
              }
            }
          } catch (error) {
            console.error('Error handling ESP32 message:', error);
          }
        });
        
        socket.on('close', () => {
          esp32Socket = null;
        });
      });
    }
    
    wss.handleUpgrade(req, req.socket, Buffer.alloc(0), (ws) => {
      wss.emit('connection', ws, req);
    });
    
    return;
  }
  
  // HTTP requests - forward to ESP32
  if (!esp32Socket || esp32Socket.readyState !== 1) {
    res.status(503).setHeader('Content-Type', 'text/html').send(`
      <!DOCTYPE html>
      <html>
        <head><title>ESP32 Offline</title></head>
        <body style="font-family: Arial; text-align: center; padding: 50px;">
          <h1>🔌 ESP32 Device Offline</h1>
          <p>The ESP32 device is not connected to the relay server.</p>
          <p>Please ensure your ESP32 is powered on and connected to WiFi.</p>
        </body>
      </html>
    `);
    return;
  }
  
  // Create request ID
  const requestId = Date.now() + Math.random();
  
  // Forward request to ESP32
  const requestData = {
    type: 'http_request',
    requestId,
    method: req.method,
    path: req.url,
    headers: req.headers
  };
  
  esp32Socket.send(JSON.stringify(requestData));
  
  // Wait for response from ESP32
  try {
    const response = await new Promise((resolve, reject) => {
      activeConnections.set(requestId, { resolve, reject });
      
      // Timeout after 30 seconds
      setTimeout(() => {
        activeConnections.delete(requestId);
        reject(new Error('Timeout'));
      }, 30000);
    });
    
    res.status(response.statusCode).set(response.headers).send(response.body);
  } catch (error) {
    res.status(504).send('Gateway timeout - ESP32 did not respond');
  }
}
