// Vercel Serverless Function for ESP32 Relay
import ngrok from '@ngrok/ngrok';

let esp32Socket = null;
let activeConnections = new Map();
let ngrokListener = null;
let publicUrl = null;

// Initialize ngrok tunnel
async function initNgrok() {
  if (ngrokListener) return publicUrl;
  
  try {
    ngrokListener = await ngrok.forward({
      addr: `https://${process.env.VERCEL_URL}`,
      authtoken: process.env.NGROK_AUTH_TOKEN,
    });
    
    publicUrl = ngrokListener.url();
    console.log('Ngrok tunnel created:', publicUrl);
    return publicUrl;
  } catch (error) {
    console.error('Ngrok error:', error);
    return null;
  }
}

export const config = {
  runtime: 'edge',
};

export default async function handler(req) {
  const url = new URL(req.url);
  
  // WebSocket upgrade for ESP32
  if (req.headers.get('upgrade') === 'websocket') {
    const { socket, response } = Deno.upgradeWebSocket(req);
    
    socket.addEventListener('open', async () => {
      console.log('ESP32 connected');
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
    });
    
    socket.addEventListener('message', (event) => {
      try {
        const message = JSON.parse(event.data);
        
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
    
    socket.addEventListener('close', () => {
      console.log('ESP32 disconnected');
      esp32Socket = null;
    });
    
    return response;
  }
  
  // HTTP requests - forward to ESP32
  if (!esp32Socket || esp32Socket.readyState !== 1) {
    return new Response(`
      <!DOCTYPE html>
      <html>
        <head><title>ESP32 Offline</title></head>
        <body style="font-family: Arial; text-align: center; padding: 50px;">
          <h1>🔌 ESP32 Device Offline</h1>
          <p>The ESP32 device is not connected to the relay server.</p>
          <p>Please ensure your ESP32 is powered on and connected to WiFi.</p>
        </body>
      </html>
    `, {
      status: 503,
      headers: { 'Content-Type': 'text/html' }
    });
  }
  
  // Create request ID
  const requestId = Date.now() + Math.random();
  
  // Forward request to ESP32
  const requestData = {
    type: 'http_request',
    requestId,
    method: req.method,
    path: url.pathname + url.search,
    headers: Object.fromEntries(req.headers.entries())
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
    
    return new Response(response.body, {
      status: response.statusCode,
      headers: response.headers
    });
  } catch (error) {
    return new Response('Gateway timeout - ESP32 did not respond', {
      status: 504
    });
  }
}
