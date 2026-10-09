let isLogging = false;
let isCalibrating = false;
let ws = null;
let pollingInterval = null;
let wsConnected = false;
let wsRetryCount = 0;
const maxRetries = 3;
let lastDataMessageTime = 0;
const dataTimeout = 5000;
let pendingUpdates = null;

function startLogging() {
    fetch('/start', { method: 'GET' })
        .then(response => response.text())
        .then(data => {
            isLogging = true;
            updateStatus();
        })
        .catch(error => console.error(error));
}

function stopLogging() {
    fetch('/stop', { method: 'GET' })
        .then(response => response.text())
        .then(data => {
            isLogging = false;
            updateStatus();
        })
        .catch(error => console.error(error));
}

function downloadCSV() {
    window.location.href = '/download';
}

function calibrateIMU() {
    fetch('/calibrate')
        .then(response => response.text())
        .then(data => {
            document.getElementById('status').innerHTML = '<span class="status-bar calibrating"><span class="status-dot"></span>Calibrating</span>';
            document.getElementById('calibrateButton').disabled = true;
            isCalibrating = true;
            checkCalibrationStatus();
        })
        .catch(error => {
            document.getElementById('status').innerHTML = '<span class="status-bar ready"><span class="status-dot"></span>Error</span>';
            document.getElementById('calibrateButton').disabled = false;
            isCalibrating = false;
        });
}

function checkCalibrationStatus() {
    fetch('/calibration_status')
        .then(response => response.text())
        .then(status => {
            if (status === 'complete') {
                document.getElementById('status').innerHTML = '<span class="status-bar ready"><span class="status-dot"></span>Ready</span>';
                document.getElementById('calibrateButton').disabled = false;
                isCalibrating = false;
            } else if (status === 'in_progress') {
                setTimeout(checkCalibrationStatus, 1000);
            }
        });
}

function updateStatus() {
    if (isCalibrating) {
        document.getElementById('status').innerHTML = '<span class="status-bar calibrating"><span class="status-dot"></span>Calibrating</span>';
    } else if (isLogging) {
        document.getElementById('status').innerHTML = '<span class="status-bar logging"><span class="status-dot"></span>Logging</span>';
    } else {
        document.getElementById('status').innerHTML = '<span class="status-bar ready"><span class="status-dot"></span>Ready</span>';
    }
}

function showData(data) {
    document.getElementById('strain1').innerHTML = Number(data.strain1).toFixed(3);
    document.getElementById('strain2').innerHTML = Number(data.strain2).toFixed(3);
    document.getElementById('timestamp').innerHTML = data.timestamp;
    document.getElementById('qw').innerHTML = Number(data.qw).toFixed(3);
    document.getElementById('qx').innerHTML = Number(data.qx).toFixed(3);
    document.getElementById('qy').innerHTML = Number(data.qy).toFixed(3);
    document.getElementById('qz').innerHTML = Number(data.qz).toFixed(3);
    document.getElementById('accel_x').innerHTML = Number(data.accel_x).toFixed(3);
    document.getElementById('accel_y').innerHTML = Number(data.accel_y).toFixed(3);
    document.getElementById('accel_z').innerHTML = Number(data.accel_z).toFixed(3);
    document.getElementById('gyro_x').innerHTML = Number(data.gyro_x).toFixed(3);
    document.getElementById('gyro_y').innerHTML = Number(data.gyro_y).toFixed(3);
    document.getElementById('gyro_z').innerHTML = Number(data.gyro_z).toFixed(3);
}

function initWebSocket() {
    ws = new WebSocket('ws://' + window.location.hostname + '/ws');
    ws.onmessage = function(event) {
        const data = JSON.parse(event.data);
        if (data.type === 'keep-alive') return;
        if (isLogging) showData(data);
    };
}

function updateData() {
    fetch('/status')
        .then(response => response.text())
        .then(status => {
            isLogging = (status === 'true');
            updateStatus();
            if (isLogging) {
                fetch('/data').then(response => response.json()).then(showData);
            }
        });
}

function tareStrain() {
    fetch('/calibrate_strain_tare').then(() => checkStrainStatus());
}

function weightStrain() {
    fetch('/calibrate_strain_weight').then(() => checkStrainStatus());
}

function checkStrainStatus() {
    fetch('/calibrate_strain_status')
        .then(response => response.text())
        .then(status => {
            const statusElement = document.getElementById('status');
            const instructionElement = document.getElementById('instruction');
            if (status === 'tare') {
                statusElement.innerHTML = 'Status: Taring...';
                instructionElement.innerHTML = 'Unload both gauges and click Tare.';
                document.getElementById('tareButton').disabled = false;
            } else if (status === 'weight1') {
                statusElement.innerHTML = 'Status: Waiting for gauge 1...';
                instructionElement.innerHTML = 'Apply 50 N to gauge 1 only, then click Calibrate.';
                document.getElementById('tareButton').disabled = true;
                document.getElementById('weightButton').disabled = false;
            } else if (status === 'weight2') {
                statusElement.innerHTML = 'Status: Waiting for gauge 2...';
                instructionElement.innerHTML = 'Move the 50 N load to gauge 2 only, then click Calibrate.';
                document.getElementById('tareButton').disabled = true;
                document.getElementById('weightButton').disabled = false;
            } else if (status === 'complete') {
                statusElement.innerHTML = 'Status: Calibration complete';
                instructionElement.innerHTML = 'Both gauges are calibrated.';
                document.getElementById('tareButton').disabled = true;
                document.getElementById('weightButton').disabled = true;
            } else {
                statusElement.innerHTML = 'Status: Waiting';
                instructionElement.innerHTML = 'Unload both gauges and click Tare.';
                document.getElementById('tareButton').disabled = false;
                document.getElementById('weightButton').disabled = true;
            }
            if (status !== 'complete') setTimeout(checkStrainStatus, 1000);
        });
}

if (document.getElementById('dashboard')) {
    initWebSocket();
    setInterval(updateData, 500);
}
if (window.location.pathname === '/calibrate_strain') {
    checkStrainStatus();
}
