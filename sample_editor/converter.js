// HTML skeleton
const appHTML = `
  <div id="app">
    <div id="settings">
      <label>
        Kit Name: <input type="text" id="kitName" placeholder="Enter filename" />
      </label>
      <label>
        Min Level (dB): <input type="range" id="minLevel" min="-80" max="0" value="-20" step="1" />
        <span id="minLevelValue" style="display:inline-block;width:50px;">-20</span>
      </label>
      <label>
        Sample Rate: <input type="number" id="sampleRate" min="4000" max="96000" value="16000" step="1000" />
      </label>
      <label>
        Fade In (ms): <input type="range" id="fadeIn" min="0" max="50" value="10" step="0.1" />
        <span id="fadeInValue" style="display:inline-block;width:50px;">10.0</span>
      </label>
      <label>
        Fade Out (ms): <input type="range" id="fadeOut" min="0" max="50" value="10" step="0.1" />
        <span id="fadeOutValue" style="display:inline-block;width:50px;">10.0</span>
      </label>
      <button id="clearState">Clear</button>
    </div>
    <div id="sampleGrid">
      ${Array.from({ length: 26 }, (_, i) => `<div class="sampleBox" data-key="${String.fromCharCode(97 + i)}">${String.fromCharCode(97 + i)}</div>`).join('')}
    </div>
    <div id="currentSample" class="sampleBox" style="margin-top: 20px;">[]</div>
    <div id="waveformDisplay" style="margin-top:20px;height:100px;border:1px solid #ccc;">Waveform display will appear here</div>
    <button id="downloadKit">Download Kit</button>
  </div>
`;

document.body.innerHTML = appHTML;

// CSS styling
document.head.insertAdjacentHTML('beforeend', `
<style>
  #app {
    font-family: Arial, sans-serif;
    padding: 20px;
  }
  #settings {
    margin-bottom: 20px;
  }
  #sampleGrid {
    display: grid;
    grid-template-columns: repeat(13, 1fr);
    gap: 10px;
  }
  #currentSample {
    width: 7%;
    height: 7%;
  }
  .sampleBox {
    width: 100%;
    aspect-ratio: 1 / 1;
    border: 2px dashed #ccc;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
  }
  .sampleBox.dragover {
    border-color: #007bff;
    background-color: rgba(0, 123, 255, 0.1);
  }
  input[type="range"] {
    width: 150px;
    margin: 0 10px;
  }
  #downloadKit {
    padding: 10px 20px;
    background-color: #007bff;
    color: white;
    border: none;
    border-radius: 5px;
    margin-top: 20px;
  }
  #downloadKit:hover {
    background-color: #0056b3;
  }
  #downloadKit:active {
    background-color: #004286;
  }
  #waveformDisplay {
    display: flex;
    justify-content: center;
    align-items: center;
    background: #f9f9f9;
  }
  body {
    max-width: 80%;
  }
</style>
`);

document.querySelectorAll('.sampleBox').forEach(square => {
    square.addEventListener('contextmenu', (event) => {
        event.preventDefault();
        clearSample(square);
    });
});

function clearSample(square) {
    const key = square.dataset.key; // Assuming each grid square has a data-key attribute
    // delete the sample from sampleMap
    delete sampleMap[key];
    saveState();        
    drawWaveform(null); // Clear the waveform display
    // restyle the sampleBox
    square.style.backgroundColor = '';    
    console.log(`Sample cleared for key: ${key}`);
}

function blobToBase64(blob) {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onloadend = () => resolve(reader.result.split(',')[1]); // Extract Base64 content
        reader.onerror = reject;
        reader.readAsDataURL(blob); // Reads the blob as a Data URL
    });
}

function base64ToBlob(base64, mimeType = 'application/octet-stream') {
    const byteCharacters = atob(base64);
    const byteNumbers = new Array(byteCharacters.length);
    for (let i = 0; i < byteCharacters.length; i++) {
        byteNumbers[i] = byteCharacters.charCodeAt(i);
    }
    const byteArray = new Uint8Array(byteNumbers);
    return new Blob([byteArray], { type: mimeType });
}
// JavaScript logic
const audioContext = new (window.AudioContext || window.webkitAudioContext)();
let sampleMap = JSON.parse(sessionStorage.getItem('sampleMap')) || {};
let currentSampleBuffer = null;
let currentSampleKey = null;

async function saveState() {
    const serializedMap = {};
    for (const [key, blob] of Object.entries(sampleMap)) {
        serializedMap[key] = await blobToBase64(blob);
    }
    sessionStorage.setItem('sampleMap', JSON.stringify(serializedMap));
    sessionStorage.setItem('settings', JSON.stringify({
        kitName: document.getElementById('kitName').value,
        minLevel: document.getElementById('minLevel').value,
        sampleRate: document.getElementById('sampleRate').value,
        fadeIn: document.getElementById('fadeIn').value,
        fadeOut: document.getElementById('fadeOut').value,
    }));
}

function restoreState() {
    const savedSettings = JSON.parse(sessionStorage.getItem('settings'));
    if (savedSettings) {
        document.getElementById('kitName').value = savedSettings.kitName;
        document.getElementById('minLevel').value = savedSettings.minLevel;
        document.getElementById('sampleRate').value = savedSettings.sampleRate;
        document.getElementById('fadeIn').value = savedSettings.fadeIn;
        document.getElementById('fadeOut').value = savedSettings.fadeOut;
        document.getElementById('minLevelValue').textContent = savedSettings.minLevel;
        document.getElementById('fadeInValue').textContent = savedSettings.fadeIn;
        document.getElementById('fadeOutValue').textContent = savedSettings.fadeOut;
    }
    const serializedMap = JSON.parse(sessionStorage.getItem('sampleMap'));
    if (serializedMap) {
        for (const [key, base64] of Object.entries(serializedMap)) {
            sampleMap[key] = base64ToBlob(base64, 'audio/wav');
            const box = document.querySelector(`.sampleBox[data-key="${key}"]`);
            if (box) box.style.backgroundColor = '#d4edda';
        }
    }
    updateCurrentSampleDisplay();
}

function clearState() {
    sessionStorage.clear();
    Object.keys(sampleMap).forEach((key) => {
        const box = document.querySelector(`.sampleBox[data-key="${key}"]`);
        if (box) {
            box.style.backgroundColor = '';
        }
    });
    sampleMap = {};
    currentSampleBuffer = null;
    currentSampleKey = null;
    updateCurrentSampleDisplay();
    document.getElementById('waveformDisplay').innerHTML = 'Waveform display will appear here';
}

function trimSilence(buffer, minLevel) {
    const rawData = buffer.getChannelData(0);
    let start = 0;
    let end = rawData.length;
    
    for (let i = 0; i < rawData.length; i++) {
        if (Math.abs(rawData[i]) > minLevel) {
            start = i;
            break;
        }
    }

    for (let i = rawData.length - 1; i >= 0; i--) {
        if (Math.abs(rawData[i]) > minLevel) {
            end = i + 1;
            break;
        }
    }

    const trimmedLength = (end - start);
    const trimmedBuffer = audioContext.createBuffer(1, trimmedLength, buffer.sampleRate);
    const newChannelData = trimmedBuffer.getChannelData(0);
    newChannelData.set(rawData.slice(start, end));
    console.log(`Trimmed buffer length: ${trimmedBuffer.length} samples`);
    return trimmedBuffer;
}

function applyFade(buffer, fadeInTime, fadeOutTime) {
    const channelData = buffer.getChannelData(0);
    const sampleRate = buffer.sampleRate;
    const fadeInSamples = Math.floor((fadeInTime / 1000) * sampleRate);
    const fadeOutSamples = Math.floor((fadeOutTime / 1000) * sampleRate);

    for (let i = 0; i < fadeInSamples; i++) {
        channelData[i] *= i / fadeInSamples;
    }

    for (let i = 0; i < fadeOutSamples; i++) {
        const idx = channelData.length - fadeOutSamples + i;
        channelData[idx] *= 1 - i / fadeOutSamples;
    }

    return buffer;
}

function drawWaveform(buffer) {
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    const container = document.getElementById('waveformDisplay');
    container.innerHTML = '';
    container.appendChild(canvas);

    const width = container.offsetWidth;
    const height = container.offsetHeight;
    canvas.width = width;
    canvas.height = height;

    if (!buffer) {
        ctx.clearRect(0, 0, width, height);
        return;
    }

    const channelData = buffer.getChannelData(0);
    const step = Math.ceil(channelData.length / width);
    const amp = height / 2;

    ctx.clearRect(0, 0, width, height);
    ctx.beginPath();
    ctx.moveTo(0, amp);
    for (let i = 0; i < width; i++) {
        const slice = channelData.slice(i * step, (i + 1) * step);
        const min = Math.min(...slice);
        const max = Math.max(...slice);
        ctx.lineTo(i, amp + min * amp);
        ctx.lineTo(i, amp + max * amp);
    }
    ctx.strokeStyle = '#007bff';
    ctx.stroke();
}
async function processWav(file, sampleRate, minLevel, fadeInTime, fadeOutTime, key) {

const arrayBuffer = await file.arrayBuffer();
const audioBuffer = await audioContext.decodeAudioData(arrayBuffer);

// Convert the sample rate
const offlineContext = new OfflineAudioContext(1, audioBuffer.length, sampleRate);
const bufferSource = offlineContext.createBufferSource();
bufferSource.buffer = audioBuffer;
bufferSource.connect(offlineContext.destination);
bufferSource.start();

const resampledBuffer = await offlineContext.startRendering();

// Trim the silence
const trimmedBuffer = trimSilence(resampledBuffer, minLevel);

// Apply fade in and fade out
const fadedBuffer = applyFade(trimmedBuffer, fadeInTime, fadeOutTime);

currentSampleBuffer = fadedBuffer;
currentSampleKey = key;
updateCurrentSampleDisplay();
drawWaveform(fadedBuffer);

// Create a new offline context for rendering the final buffer
const finalOfflineContext = new OfflineAudioContext(1, fadedBuffer.length, sampleRate);
const finalBufferSource = finalOfflineContext.createBufferSource();
finalBufferSource.buffer = fadedBuffer;
finalBufferSource.connect(finalOfflineContext.destination);
finalBufferSource.start();

const renderedBuffer = await finalOfflineContext.startRendering();

const wav = audioBufferToWav(renderedBuffer);
return new Blob([wav], { type: 'audio/wav' });
}
function updateCurrentSampleDisplay() {
    const currentSample = document.getElementById('currentSample');
    if (currentSampleKey) {
        currentSample.style.backgroundColor = '#d4edda';
        currentSample.textContent = `${currentSampleKey}`;
        // convert the blob in sampleMap[currentSampleKey] to buffer
        const reader = new FileReader();
        reader.onload = function () {
            const arrayBuffer = reader.result;
            audioContext.decodeAudioData(arrayBuffer).then((audioBuffer) => {
                currentSampleBuffer = audioBuffer;
                drawWaveform(audioBuffer);
            });

        }
       //reader.readAsArrayBuffer(sampleMap[currentSampleKey]);
    }
    else {
        currentSample.style.backgroundColor = '';
        currentSample.textContent = '[]';
    }

}

function audioBufferToWav(buffer) {
    const numOfChannels = buffer.numberOfChannels;
    const length = buffer.length * numOfChannels * 2 + 44;
    const output = new DataView(new ArrayBuffer(length));

    function writeString(view, offset, string) {
        for (let i = 0; i < string.length; i++) {
            view.setUint8(offset + i, string.charCodeAt(i));
        }
    }

    function write16(view, offset, value) {
        view.setUint16(offset, value, true);
    }

    function write32(view, offset, value) {
        view.setUint32(offset, value, true);
    }

    writeString(output, 0, 'RIFF');
    write32(output, 4, length - 8);
    writeString(output, 8, 'WAVE');
    writeString(output, 12, 'fmt ');
    write32(output, 16, 16);
    write16(output, 20, 1);
    write16(output, 22, numOfChannels);
    write32(output, 24, buffer.sampleRate);
    write32(output, 28, buffer.sampleRate * numOfChannels * 2);
    write16(output, 32, numOfChannels * 2);
    write16(output, 34, 16);
    writeString(output, 36, 'data');
    write32(output, 40, buffer.length * numOfChannels * 2);

    const channelData = buffer.getChannelData(0);
    const int16 = new Int16Array(channelData.length);
    for (let i = 0; i < channelData.length; i++) {
        int16[i] = Math.max(-1, Math.min(1, channelData[i])) * 0x7fff;
    }

    for (let i = 0; i < int16.length; i++) {
        output.setInt16(44 + i * 2, int16[i], true);
    }

    console.log('WAV file size:', length);
    return output.buffer;
}

function handleFileDrop(event) {
    event.preventDefault();
    const box = event.target.closest('.sampleBox');
    const key = box.dataset.key;
    const file = event.dataTransfer.files[0];

    if (file && file.type === 'audio/wav') {
        const sampleRate = parseInt(document.getElementById('sampleRate').value, 10);
        const minLevel = Math.pow(10, parseFloat(document.getElementById('minLevel').value) / 20);
        const fadeInTime = parseFloat(document.getElementById('fadeIn').value);
        const fadeOutTime = parseFloat(document.getElementById('fadeOut').value);

        processWav(file, sampleRate, minLevel, fadeInTime, fadeOutTime, key).then((processedFile) => {
            sampleMap[key] = processedFile;
            saveState();
            box.style.backgroundColor = '#d4edda';
            playSample(processedFile, key);
        });
    }
}

function playSample(blob, key) {
    const url = URL.createObjectURL(blob);
    const audio = new Audio(url);
    audio.play();

    audio.onplay = () => {
        if (key) {
            currentSampleKey = key;
            updateCurrentSampleDisplay();
        }
    };
}

document.querySelectorAll('.sampleBox').forEach((box) => {
    box.addEventListener('dragover', (event) => {
        event.preventDefault();
        box.classList.add('dragover');
    });
    box.addEventListener('dragleave', () => box.classList.remove('dragover'));
    box.addEventListener('drop', handleFileDrop);
    box.addEventListener('click', () => {
        const key = box.dataset.key;
        if (sampleMap[key]) {
            playSample(sampleMap[key], key);
        }
    });
});

document.getElementById('downloadKit').addEventListener('click', () => {
    const kitName = document.getElementById('kitName').value || 'sampleKit';
    const zip = new JSZip();
    const folder = zip.folder(`${kitName}.KIT`);

    Object.entries(sampleMap).forEach(([key, blob]) => {
        // log the size of the blob
        console.log(key, blob.size);
        folder.file(`${key}.wav`, blob);
    });

    zip.generateAsync({ type: 'blob' }).then((content) => {
        const a = document.createElement('a');
        a.href = URL.createObjectURL(content);
        a.download = `${kitName}.zip`;
        a.click();
    });
});

document.getElementById('minLevel').addEventListener('input', (event) => {
    document.getElementById('minLevelValue').textContent = event.target.value;
    saveState();
});

document.getElementById('fadeIn').addEventListener('input', (event) => {
    document.getElementById('fadeInValue').textContent = event.target.value;
    saveState();
});

document.getElementById('fadeOut').addEventListener('input', (event) => {
    document.getElementById('fadeOutValue').textContent = event.target.value;
    saveState();
});

document.getElementById('clearState').addEventListener('click', clearState);

// Restore state onload

restoreState();

