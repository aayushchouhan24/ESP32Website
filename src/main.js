/*
 * ESP32 C3 Admin Panel - Web Bluetooth Client
 * 
 * This file handles all Web Bluetooth communication with the ESP32 C3 device.
 * It manages connection, WiFi configuration, and file uploads via BLE.
 * 
 * Author: Generated for Aayush
 * Date: December 2025
 */

import './style.css';

// ==================== BLE Configuration ====================

const SERVICE_UUID = '12345678-1234-1234-1234-123456789abc';
const COMMAND_CHAR_UUID = '12345678-1234-1234-1234-123456789abd';
const STATUS_CHAR_UUID = '12345678-1234-1234-1234-123456789abe';

const MAX_CHUNK_SIZE = 100; // Small size to account for JSON overhead and stay under BLE limit

// ==================== Global State ====================

let bleDevice = null;
let bleServer = null;
let commandCharacteristic = null;
let statusCharacteristic = null;
let selectedFiles = [];
let esp32IpAddress = null;

// ==================== DOM Elements ====================

const connectBtn = document.getElementById('connectBtn');
const wifiSection = document.getElementById('wifiSection');
const wifiForm = document.getElementById('wifiForm');
const uploadSection = document.getElementById('uploadSection');
const fileInput = document.getElementById('fileInput');
const fileList = document.getElementById('fileList');
const uploadBtn = document.getElementById('uploadBtn');
const uploadProgress = document.getElementById('uploadProgress');
const finalizeSection = document.getElementById('finalizeSection');
const rebootBtn = document.getElementById('rebootBtn');
const logOutput = document.getElementById('logOutput');
const clearLogBtn = document.getElementById('clearLogBtn');
const statusIndicator = document.getElementById('statusIndicator');
const statusText = document.getElementById('statusText');

// ==================== Utility Functions ====================

function updateStatus(status, message) {
  statusIndicator.className = 'status-indicator';
  
  switch(status) {
    case 'connected':
      statusIndicator.classList.add('connected');
      break;
    case 'connecting':
      statusIndicator.classList.add('connecting');
      break;
    case 'error':
      statusIndicator.classList.add('error');
      break;
  }
  
  statusText.textContent = message;
}

function log(message, type = 'info') {
  const timestamp = new Date().toLocaleTimeString();
  const entry = document.createElement('div');
  entry.className = `log-entry ${type}`;
  entry.textContent = `[${timestamp}] ${message}`;
  logOutput.appendChild(entry);
  logOutput.scrollTop = logOutput.scrollHeight;
}

function formatFileSize(bytes) {
  if (bytes === 0) return '0 Bytes';
  const k = 1024;
  const sizes = ['Bytes', 'KB', 'MB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// ==================== BLE Functions ====================

async function connectToDevice() {
  try {
    log('Requesting Bluetooth device...', 'info');
    updateStatus('connecting', 'Connecting...');
    
    // Request device with specific service UUID
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ services: [SERVICE_UUID] }],
      optionalServices: [SERVICE_UUID]
    });
    
    log(`Found device: ${bleDevice.name}`, 'success');
    
    // Add disconnect listener
    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);
    
    // Connect to GATT server
    log('Connecting to GATT server...', 'info');
    bleServer = await bleDevice.gatt.connect();
    
    // Get service
    log('Getting configuration service...', 'info');
    const service = await bleServer.getPrimaryService(SERVICE_UUID);
    
    // Get characteristics
    log('Getting characteristics...', 'info');
    commandCharacteristic = await service.getCharacteristic(COMMAND_CHAR_UUID);
    statusCharacteristic = await service.getCharacteristic(STATUS_CHAR_UUID);
    
    // Start notifications for status updates
    await statusCharacteristic.startNotifications();
    statusCharacteristic.addEventListener('characteristicvaluechanged', handleStatusNotification);
    
    log('Connected successfully!', 'success');
    updateStatus('connected', `Connected to ${bleDevice.name}`);
    
    // Update UI
    connectBtn.textContent = 'Disconnect';
    connectBtn.classList.remove('btn-primary');
    connectBtn.classList.add('btn-secondary');
    wifiSection.style.display = 'block';
    uploadSection.style.display = 'block';
    
    // Request device status
    setTimeout(() => requestDeviceStatus(), 500);
    
  } catch (error) {
    log(`Connection failed: ${error.message}`, 'error');
    updateStatus('error', 'Connection failed');
    console.error('Connection error:', error);
  }
}

function onDisconnected() {
  log('Device disconnected', 'warning');
  updateStatus('error', 'Disconnected');
  
  bleDevice = null;
  bleServer = null;
  commandCharacteristic = null;
  statusCharacteristic = null;
  
  connectBtn.textContent = 'Connect via Bluetooth';
  connectBtn.classList.remove('btn-secondary');
  connectBtn.classList.add('btn-primary');
  wifiSection.style.display = 'none';
  uploadSection.style.display = 'none';
  finalizeSection.style.display = 'none';
}

function handleStatusNotification(event) {
  const value = event.target.value;
  const decoder = new TextDecoder('utf-8');
  const jsonString = decoder.decode(value);
  
  try {
    const status = JSON.parse(jsonString);
    log(`ESP32: ${status.message || JSON.stringify(status)}`, status.type || 'info');
    
    // Handle specific status types
    if (status.type === 'file_progress') {
      const percent = Math.round((status.bytes_received / status.total_bytes) * 100);
      document.getElementById('progressPercent').textContent = `${percent}%`;
      document.getElementById('progressFill').style.width = `${percent}%`;
    } else if (status.type === 'wifi_status' && status.ip) {
      esp32IpAddress = status.ip;
      log(`ESP32 IP Address: ${status.ip}`, 'success');
      showIpAddress(status.ip);
    } else if (status.type === 'device_status') {
      updateDeviceStatus(status);
    } else if (status.type === 'relay_status') {
      updateRelayStatus(status);
    }
  } catch (error) {
    log(`Status: ${jsonString}`, 'info');
  }
}

function updateDeviceStatus(status) {
  const deviceStatus = document.getElementById('deviceStatus');
  deviceStatus.style.display = 'block';
  
  // Update mode (now shows both)
  const modeEl = document.getElementById('deviceMode');
  if (status.webserver_active && status.ble_active) {
    modeEl.textContent = '🌐 Web + BLE';
    modeEl.className = 'status-value connected';
  } else if (status.webserver_active) {
    modeEl.textContent = '🌐 Web Server';
    modeEl.className = 'status-value connected';
  } else if (status.ble_active) {
    modeEl.textContent = '⚙️ Config Only';
    modeEl.className = 'status-value warning';
  }
  
  // Update WiFi status
  const wifiEl = document.getElementById('wifiStatus');
  if (status.wifi_connected) {
    wifiEl.textContent = `✓ ${status.ssid || 'Connected'}`;
    wifiEl.className = 'status-value connected';
  } else if (status.has_wifi_credentials) {
    wifiEl.textContent = `⚠ Saved (${status.saved_ssid})`;
    wifiEl.className = 'status-value warning';
  } else {
    wifiEl.textContent = '✗ Not Configured';
    wifiEl.className = 'status-value disconnected';
  }
  
  // Update website status
  const websiteEl = document.getElementById('websiteStatus');
  if (status.webserver_active && status.wifi_connected) {
    websiteEl.textContent = '✓ Hosted';
    websiteEl.className = 'status-value connected';
  } else {
    websiteEl.textContent = '✗ Not Hosted';
    websiteEl.className = 'status-value disconnected';
  }
  
  // Update IP address
  const ipItem = document.getElementById('ipAddressItem');
  const ipEl = document.getElementById('deviceIp');
  if (status.wifi_connected && status.ip) {
    ipItem.style.display = 'flex';
    ipEl.textContent = status.ip;
    ipEl.className = 'status-value';
    esp32IpAddress = status.ip;
    
    // Show IP address card with internet access info
    if (status.webserver_active) {
      showIpAddressWithInternetInfo(status.ip);
    }
  } else {
    ipItem.style.display = 'none';
  }
  
  // Show action hint if in config mode with WiFi credentials
  showActionHint(status);
  
  // Show relay section if web server is active
  const relaySection = document.getElementById('relaySection');
  if (status.webserver_active && status.wifi_connected) {
    relaySection.style.display = 'block';
    finalizeSection.style.display = 'block';
    
    // Update relay status if available
    if (status.relay_connected !== undefined) {
      updateRelayStatus({
        connected: status.relay_connected,
        public_url: status.public_url
      });
    }
  }
}

function showActionHint(status) {
  let hintSection = document.getElementById('actionHint');
  
  if (!status.webserver_active && status.has_wifi_credentials) {
    if (!hintSection) {
      hintSection = document.createElement('div');
      hintSection.id = 'actionHint';
      hintSection.className = 'card';
      hintSection.style.background = 'linear-gradient(135deg, #f59e0b 0%, #d97706 100%)';
      hintSection.style.color = 'white';
      hintSection.style.marginTop = '20px';
      
      const deviceStatus = document.getElementById('deviceStatus');
      deviceStatus.parentNode.insertBefore(hintSection, deviceStatus.nextSibling);
    }
    
    hintSection.innerHTML = `
      <h3 style="margin-top: 0; color: white;">⚡ Ready to Deploy</h3>
      <p style="font-size: 1rem; margin: 10px 0;">WiFi credentials are saved!</p>
      <p style="font-size: 0.95rem; margin: 10px 0;">To start hosting your website:</p>
      <ol style="font-size: 0.95rem; margin: 10px 0 10px 20px; line-height: 1.6;">
        <li>Upload your website files (if not done yet)</li>
        <li>Click <strong>"Start Web Server"</strong> button below</li>
        <li>Your site will be live immediately!</li>
      </ol>
      <p style="font-size: 0.85rem; margin-top: 15px; opacity: 0.9;">💡 BLE will stay active so you can upload more files or change settings anytime!</p>
    `;
  } else if (hintSection) {
    hintSection.remove();
  }
}

async function requestDeviceStatus() {
  await sendCommand({ type: 'get_status' });
}

async function sendCommand(command) {
  if (!commandCharacteristic) {
    log('Not connected to device', 'error');
    return false;
  }
  
  try {
    const encoder = new TextEncoder();
    const data = encoder.encode(JSON.stringify(command));
    await commandCharacteristic.writeValue(data);
    return true;
  } catch (error) {
    log(`Failed to send command: ${error.message}`, 'error');
    return false;
  }
}

// ==================== WiFi Configuration ====================

function loadSavedWiFiCredentials() {
  const savedSSID = localStorage.getItem('esp32_wifi_ssid');
  const savedPassword = localStorage.getItem('esp32_wifi_password');
  
  if (savedSSID) {
    document.getElementById('ssid').value = savedSSID;
  }
  if (savedPassword) {
    document.getElementById('password').value = savedPassword;
  }
}

function showIpAddressWithInternetInfo(ip) {
  // Add IP display section if it doesn't exist
  let ipSection = document.getElementById('ipSection');
  if (!ipSection) {
    ipSection = document.createElement('div');
    ipSection.id = 'ipSection';
    ipSection.className = 'card';
    ipSection.style.background = 'linear-gradient(135deg, #10b981 0%, #059669 100%)';
    ipSection.style.color = 'white';
    ipSection.innerHTML = `
      <h2 style="margin-top: 0; color: white;">✅ Website is Live!</h2>
      <p style="font-size: 1.125rem; margin: 10px 0;">Your ESP32 website is accessible at:</p>
      <div style="background: rgba(255,255,255,0.2); padding: 15px; border-radius: 8px; margin: 15px 0;">
        <code style="font-size: 1.25rem; color: white; word-break: break-all;">http://${ip}</code>
      </div>
      <a href="http://${ip}" target="_blank" class="btn" style="background: white; color: #059669; margin-top: 10px; display: inline-block; text-decoration: none;">Open Website →</a>
      
      <div style="margin-top: 20px; padding: 15px; background: rgba(255,255,255,0.15); border-radius: 8px; border-left: 4px solid white;">
        <h3 style="margin-top: 0; font-size: 1rem; color: white;">🌍 Access from Internet</h3>
        <p style="font-size: 0.9rem; margin: 8px 0; line-height: 1.5;">To access this from anywhere:</p>
        <ol style="font-size: 0.9rem; margin: 8px 0 8px 20px; line-height: 1.6;">
          <li>Log into your router admin panel</li>
          <li>Set up <strong>Port Forwarding</strong>:</li>
          <ul style="margin: 5px 0 5px 20px;">
            <li>External Port: 80 (or any port like 8080)</li>
            <li>Internal IP: ${ip}</li>
            <li>Internal Port: 80</li>
            <li>Protocol: TCP</li>
          </ul>
          <li>Find your public IP at <a href="https://whatismyip.com" target="_blank" style="color: white; text-decoration: underline;">whatismyip.com</a></li>
          <li>Access via: <code style="background: rgba(0,0,0,0.2); padding: 2px 6px; border-radius: 3px;">http://YOUR_PUBLIC_IP</code></li>
        </ol>
        <p style="font-size: 0.85rem; margin: 10px 0 0 0; opacity: 0.9;">⚠️ Note: Your ISP must provide a public IP (not CGNAT). Some ISPs block port 80, use alternate ports like 8080.</p>
      </div>
    `;
    
    // Insert before finalize section
    const finalizeSection = document.getElementById('finalizeSection');
    if (finalizeSection && finalizeSection.parentNode) {
      finalizeSection.parentNode.insertBefore(ipSection, finalizeSection);
    } else {
      document.querySelector('main').appendChild(ipSection);
    }
  } else {
    // Update existing IP display
    const codeElement = ipSection.querySelector('code');
    if (codeElement) {
      codeElement.textContent = `http://${ip}`;
    }
    const linkElement = ipSection.querySelector('a');
    if (linkElement) {
      linkElement.href = `http://${ip}`;
    }
  }
}

function showIpAddress(ip) {
  showIpAddressWithInternetInfo(ip);
}

async function handleWiFiSubmit(e) {
  e.preventDefault();
  
  const ssid = document.getElementById('ssid').value;
  const password = document.getElementById('password').value;
  
  if (!ssid || !password) {
    log('Please enter both SSID and password', 'error');
    return;
  }
  
  log(`Sending WiFi credentials (SSID: ${ssid})...`, 'info');
  
  const success = await sendCommand({
    type: 'wifi',
    ssid: ssid,
    pass: password
  });
  
  if (success) {
    // Save credentials to localStorage
    localStorage.setItem('esp32_wifi_ssid', ssid);
    localStorage.setItem('esp32_wifi_password', password);
    
    log('WiFi credentials saved on ESP32 and browser', 'success');
  }
}

// ==================== File Handling ====================

function handleFileSelection(e) {
  const files = Array.from(e.target.files);
  
  if (files.length === 0) return;
  
  selectedFiles = files;
  displayFileList();
  uploadBtn.style.display = 'block';
}

function displayFileList() {
  fileList.innerHTML = '';
  
  selectedFiles.forEach((file, index) => {
    const fileItem = document.createElement('div');
    fileItem.className = 'file-item';
    fileItem.innerHTML = `
      <div class="file-info">
        <div class="file-name">${file.name}</div>
        <div class="file-size">${formatFileSize(file.size)}</div>
      </div>
      <button class="file-remove" onclick="window.removeFile(${index})">Remove</button>
    `;
    fileList.appendChild(fileItem);
  });
}

window.removeFile = function(index) {
  selectedFiles.splice(index, 1);
  displayFileList();
  
  if (selectedFiles.length === 0) {
    uploadBtn.style.display = 'none';
  }
};

// ==================== Ngrok Relay ====================

function updateRelayStatus(status) {
  const relayStatus = document.getElementById('relayStatus');
  const relayStatusText = document.getElementById('relayStatusText');
  const publicUrlDisplay = document.getElementById('publicUrlDisplay');
  const publicUrlLink = document.getElementById('publicUrlLink');
  
  relayStatus.style.display = 'block';
  
  if (status.connected && status.public_url) {
    relayStatus.style.background = 'linear-gradient(135deg, #10b981 0%, #059669 100%)';
    relayStatus.style.color = 'white';
    relayStatusText.textContent = '✅ Connected to relay server';
    relayStatusText.style.color = 'white';
    
    publicUrlDisplay.style.display = 'block';
    publicUrlLink.href = status.public_url;
    publicUrlLink.textContent = status.public_url;
    publicUrlLink.style.color = 'white';
    
    log(`🌍 Public URL: ${status.public_url}`, 'success');
  } else {
    relayStatus.style.background = '#fee';
    relayStatus.style.color = '#c00';
    relayStatusText.textContent = '❌ Not connected to relay';
    publicUrlDisplay.style.display = 'none';
  }
}

async function handleRelaySubmit(e) {
  e.preventDefault();
  
  // Use current Vercel domain
  const currentDomain = window.location.hostname;
  const port = window.location.protocol === 'https:' ? 443 : 80;
  const path = '/api/relay';
  
  log(`Configuring relay: ${currentDomain}${path}`, 'info');
  
  const success = await sendCommand({
    type: 'relay_config',
    host: currentDomain,
    port: port,
    path: path
  });
  
  if (success) {
    log('Relay configuration sent. Connecting to relay...', 'success');
  }
}

// ==================== File Upload ====================

async function uploadFiles() {
  if (selectedFiles.length === 0) {
    log('No files selected', 'warning');
    return;
  }
  
  // Check for index.html
  const hasIndex = selectedFiles.some(f => f.name.toLowerCase() === 'index.html');
  if (!hasIndex) {
    log('Warning: No index.html file found!', 'warning');
    if (!confirm('No index.html found. The ESP32 needs index.html to serve a website. Continue anyway?')) {
      return;
    }
  }
  
  uploadBtn.disabled = true;
  uploadProgress.style.display = 'block';
  
  for (const file of selectedFiles) {
    try {
      await uploadSingleFile(file);
      await sleep(500); // Small delay between files
    } catch (error) {
      log(`Failed to upload ${file.name}: ${error.message}`, 'error');
      uploadBtn.disabled = false;
      return;
    }
  }
  
  log('All files uploaded successfully!', 'success');
  uploadBtn.disabled = false;
  uploadProgress.style.display = 'none';
  finalizeSection.style.display = 'block';
}

async function uploadSingleFile(file) {
  log(`Uploading ${file.name} (${formatFileSize(file.size)})...`, 'info');
  document.getElementById('currentFile').textContent = `Uploading: ${file.name}`;
  document.getElementById('progressPercent').textContent = '0%';
  document.getElementById('progressFill').style.width = '0%';
  
  // Send file metadata
  const metaSuccess = await sendCommand({
    type: 'file_meta',
    name: file.name,
    size: file.size
  });
  
  if (!metaSuccess) {
    throw new Error('Failed to send file metadata');
  }
  
  await sleep(200); // Wait for ESP32 to prepare
  
  // Read file as array buffer
  const arrayBuffer = await file.arrayBuffer();
  const uint8Array = new Uint8Array(arrayBuffer);
  
  // Send file in chunks
  let offset = 0;
  while (offset < uint8Array.length) {
    const chunkSize = Math.min(MAX_CHUNK_SIZE, uint8Array.length - offset);
    const chunk = uint8Array.slice(offset, offset + chunkSize);
    
    const chunkSuccess = await sendCommand({
      type: 'file_chunk',
      name: file.name,
      offset: offset,
      data: Array.from(chunk) // Convert Uint8Array to regular array
    });
    
    if (!chunkSuccess) {
      throw new Error(`Failed to send chunk at offset ${offset}`);
    }
    
    offset += chunkSize;
    
    // Update progress
    const percent = Math.round((offset / uint8Array.length) * 100);
    document.getElementById('progressPercent').textContent = `${percent}%`;
    document.getElementById('progressFill').style.width = `${percent}%`;
    
    // Small delay to prevent overwhelming the BLE connection
    await sleep(50);
  }
  
  log(`${file.name} uploaded successfully`, 'success');
}

// ==================== Finalize & Reboot ====================

async function rebootToWebServer() {
  log('Starting web server...', 'info');
  
  const success = await sendCommand({
    type: 'reboot',
    mode: 'web_server'
  });
  
  if (success) {
    log('Web server starting...', 'success');
    log('Your website will be live in a few seconds!', 'success');
    log('BLE will remain active for further configuration.', 'info');
    
    // Request status update after a delay
    setTimeout(() => {
      requestDeviceStatus();
    }, 5000);
  }
}

// ==================== Event Listeners ====================

connectBtn.addEventListener('click', () => {
  if (bleDevice && bleDevice.gatt.connected) {
    bleDevice.gatt.disconnect();
  } else {
    connectToDevice();
  }
});

wifiForm.addEventListener('submit', handleWiFiSubmit);
document.getElementById('relayForm').addEventListener('submit', handleRelaySubmit);
fileInput.addEventListener('change', handleFileSelection);
uploadBtn.addEventListener('click', uploadFiles);
rebootBtn.addEventListener('click', rebootToWebServer);

clearLogBtn.addEventListener('click', () => {
  logOutput.innerHTML = '';
  log('Log cleared', 'info');
});

document.getElementById('refreshStatusBtn').addEventListener('click', async () => {
  log('Requesting device status...', 'info');
  await requestDeviceStatus();
});

// Clear saved WiFi credentials
document.getElementById('clearWifiBtn').addEventListener('click', () => {
  if (confirm('Clear saved WiFi credentials from browser?')) {
    localStorage.removeItem('esp32_wifi_ssid');
    localStorage.removeItem('esp32_wifi_password');
    document.getElementById('ssid').value = '';
    document.getElementById('password').value = '';
    log('Saved WiFi credentials cleared', 'info');
  }
});

// Check for Web Bluetooth support
if (!navigator.bluetooth) {
  log('Web Bluetooth API is not available in this browser!', 'error');
  log('Please use Chrome, Edge, or Opera on desktop or Android.', 'error');
  connectBtn.disabled = true;
  updateStatus('error', 'Browser not supported');
} else {
  log('Admin panel ready. Click "Connect via Bluetooth" to begin.', 'success');
  // Load saved WiFi credentials
  loadSavedWiFiCredentials();
}
