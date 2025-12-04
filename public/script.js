// Update time every second
function updateTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString();
  document.getElementById('time').textContent = `Time: ${timeString}`;
}

updateTime();
setInterval(updateTime, 1000);

// Log to console
console.log('ESP32 Website Loaded!');
console.log('This page is served directly from ESP32 flash storage');
