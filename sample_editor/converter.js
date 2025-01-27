// --------------------------------------------------------------------------------
// HTML skeleton
// --------------------------------------------------------------------------------
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

    <!-- The A-Z grid -->
    <div id="sampleGrid">
      ${Array.from({ length: 26 }, (_, i) => `<div class="sampleBox" data-key="${String.fromCharCode(97 + i)}">${String.fromCharCode(97 + i)}</div>`).join('')}
    </div>

    <div id="currentSample" class="sampleBox" style="margin-top: 20px;">[]</div>
    <div id="waveformDisplay" style="margin-top:20px;height:100px;border:1px solid #ccc;">
      Waveform display will appear here
    </div>

    <!-- The new 'Split Multi' button/area -->
    <button id="splitMultiBtn" style="margin-top:20px;">Split Multi</button>
    <div
      id="splitDropZone"
      style="
        margin: 10px 0;
        padding: 20px;
        border: 2px dashed #999;
        text-align: center;
        cursor: pointer;
        background: #f1f1f1;
      "
    >
      Drag & Drop WAV here to split
    </div>

    <div id="splitResults" style="display: flex; flex-wrap: wrap; gap: 10px; margin-bottom:20px;"></div>

    <button id="downloadKit">Download Kit</button>
  </div>
`;

document.body.innerHTML = appHTML;

// --------------------------------------------------------------------------------
// CSS styling
// --------------------------------------------------------------------------------
document.head.insertAdjacentHTML('beforeend', `
<style>
  #app {
    font-family: Arial, sans-serif;
    padding: 20px;
    max-width: 80%;
    margin: 0 auto;
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
  #splitResults .splitSampleBox {
    width: 60px;
    height: 60px;
    border: 2px dashed #ccc;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: grab;
    background-color: #eee;
    user-select: none;
  }
  #splitResults .splitSampleBox.dragging {
    opacity: 0.5;
  }
</style>
`);

// --------------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------------
const audioContext = new (window.AudioContext || window.webkitAudioContext)();
let sampleMap = JSON.parse(sessionStorage.getItem('sampleMap')) || {};
let currentSampleBuffer = null;
let currentSampleKey = null;

/**
 * We'll keep a global array/object for split results.
 * Key: index of the split chunk
 * Value: { blob, buffer }
 */
let splitResultsMap = {};

// --------------------------------------------------------------------------------
// Helper functions (File <-> Base64, etc.)
// --------------------------------------------------------------------------------
function blobToBase64(blob) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onloadend = () => resolve(reader.result.split(",")[1]); // Extract Base64 content
    reader.onerror = reject;
    reader.readAsDataURL(blob); // Reads the blob as a Data URL
  });
}

function base64ToBlob(base64, mimeType = "application/octet-stream") {
  const byteCharacters = atob(base64);
  const byteNumbers = new Array(byteCharacters.length);
  for (let i = 0; i < byteCharacters.length; i++) {
    byteNumbers[i] = byteCharacters.charCodeAt(i);
  }
  const byteArray = new Uint8Array(byteNumbers);
  return new Blob([byteArray], { type: mimeType });
}

// --------------------------------------------------------------------------------
// State Management
// --------------------------------------------------------------------------------
async function saveState() {
  const serializedMap = {};
  for (const [key, blob] of Object.entries(sampleMap)) {
    serializedMap[key] = await blobToBase64(blob);
  }
  sessionStorage.setItem("sampleMap", JSON.stringify(serializedMap));
  sessionStorage.setItem(
    "settings",
    JSON.stringify({
      kitName: document.getElementById("kitName").value,
      minLevel: document.getElementById("minLevel").value,
      sampleRate: document.getElementById("sampleRate").value,
      fadeIn: document.getElementById("fadeIn").value,
      fadeOut: document.getElementById("fadeOut").value,
    })
  );
}

function restoreState() {
  const savedSettings = JSON.parse(sessionStorage.getItem("settings"));
  if (savedSettings) {
    document.getElementById("kitName").value = savedSettings.kitName;
    document.getElementById("minLevel").value = savedSettings.minLevel;
    document.getElementById("sampleRate").value = savedSettings.sampleRate;
    document.getElementById("fadeIn").value = savedSettings.fadeIn;
    document.getElementById("fadeOut").value = savedSettings.fadeOut;
    document.getElementById("minLevelValue").textContent = savedSettings.minLevel;
    document.getElementById("fadeInValue").textContent = savedSettings.fadeIn;
    document.getElementById("fadeOutValue").textContent = savedSettings.fadeOut;
  }
  const serializedMap = JSON.parse(sessionStorage.getItem("sampleMap"));
  if (serializedMap) {
    for (const [key, base64] of Object.entries(serializedMap)) {
      sampleMap[key] = base64ToBlob(base64, "audio/wav");
      const box = document.querySelector(`.sampleBox[data-key="${key}"]`);
      if (box) box.style.backgroundColor = "#d4edda";
    }
  }
  updateCurrentSampleDisplay();
}

function clearState() {
  sessionStorage.clear();
  Object.keys(sampleMap).forEach((key) => {
    const box = document.querySelector(`.sampleBox[data-key="${key}"]`);
    if (box) {
      box.style.backgroundColor = "";
    }
  });
  sampleMap = {};
  currentSampleBuffer = null;
  currentSampleKey = null;
  updateCurrentSampleDisplay();
  document.getElementById("waveformDisplay").innerHTML =
    "Waveform display will appear here";

  // Clear any existing splits
  splitResultsMap = {};
  document.getElementById("splitResults").innerHTML = "";
}

// --------------------------------------------------------------------------------
// Audio Processing
// --------------------------------------------------------------------------------
function trimSilence(buffer, minLevel) {
  const rawData = buffer.getChannelData(0);
  let start = 0;
  let end = rawData.length;

  // find start
  for (let i = 0; i < rawData.length; i++) {
    if (Math.abs(rawData[i]) > minLevel) {
      start = i;
      break;
    }
  }
  // find end
  for (let i = rawData.length - 1; i >= 0; i--) {
    if (Math.abs(rawData[i]) > minLevel) {
      end = i + 1;
      break;
    }
  }

  const trimmedLength = end - start;
  const trimmedBuffer = audioContext.createBuffer(
    1,
    trimmedLength,
    buffer.sampleRate
  );
  const newChannelData = trimmedBuffer.getChannelData(0);
  newChannelData.set(rawData.slice(start, end));
  return trimmedBuffer;
}

function applyFade(buffer, fadeInTime, fadeOutTime) {
  const channelData = buffer.getChannelData(0);
  const sampleRate = buffer.sampleRate;
  const fadeInSamples = Math.floor((fadeInTime / 1000) * sampleRate);
  const fadeOutSamples = Math.floor((fadeOutTime / 1000) * sampleRate);

  // fade in
  for (let i = 0; i < fadeInSamples && i < channelData.length; i++) {
    channelData[i] *= i / fadeInSamples;
  }
  // fade out
  for (let i = 0; i < fadeOutSamples && i < channelData.length; i++) {
    const idx = channelData.length - fadeOutSamples + i;
    if (idx < 0) continue;
    channelData[idx] *= 1 - i / fadeOutSamples;
  }
  return buffer;
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

  // RIFF reference
  writeString(output, 0, "RIFF");
  write32(output, 4, length - 8);
  writeString(output, 8, "WAVE");
  writeString(output, 12, "fmt ");
  write32(output, 16, 16);
  write16(output, 20, 1);
  write16(output, 22, numOfChannels);
  write32(output, 24, buffer.sampleRate);
  write32(output, 28, buffer.sampleRate * numOfChannels * 2);
  write16(output, 32, numOfChannels * 2);
  write16(output, 34, 16);
  writeString(output, 36, "data");
  write32(output, 40, buffer.length * numOfChannels * 2);

  // Write interleaved data
  const channelData = buffer.getChannelData(0);
  const int16 = new Int16Array(channelData.length);
  for (let i = 0; i < channelData.length; i++) {
    int16[i] = Math.max(-1, Math.min(1, channelData[i])) * 0x7fff;
  }

  for (let i = 0; i < int16.length; i++) {
    output.setInt16(44 + i * 2, int16[i], true);
  }

  return output.buffer;
}

/**
 * Resample a file, trim, fade, and return a single processed Blob.
 */
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

  // For drawing in the waveform display
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
  return new Blob([wav], { type: "audio/wav" });
}

// --------------------------------------------------------------------------------
// Waveform Drawing
// --------------------------------------------------------------------------------
function drawWaveform(buffer) {
  const canvas = document.createElement("canvas");
  const ctx = canvas.getContext("2d");
  const container = document.getElementById("waveformDisplay");
  container.innerHTML = "";
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
  ctx.strokeStyle = "#007bff";
  ctx.stroke();
}

/**
 * Called whenever the 'currentSampleKey' changes or we want to re-draw the "current sample" box.
 */
function updateCurrentSampleDisplay() {
  const currentSample = document.getElementById("currentSample");
  if (currentSampleKey) {
    currentSample.style.backgroundColor = "#d4edda";
    currentSample.textContent = `${currentSampleKey}`;
  } else {
    currentSample.style.backgroundColor = "";
    currentSample.textContent = "[]";
  }
}

// --------------------------------------------------------------------------------
// A-Z Grid Interactions
// --------------------------------------------------------------------------------
function handleFileDrop(event) {
  event.preventDefault();
  const box = event.target.closest(".sampleBox");
  if (!box) return;

  const key = box.dataset.key;

  // If there's a real File (drag-drop from system):
  const file = event.dataTransfer.files[0];
  if (file && file.type === "audio/wav") {
    const sampleRate = parseInt(document.getElementById("sampleRate").value, 10);
    const minLevel = Math.pow(10, parseFloat(document.getElementById("minLevel").value) / 20);
    const fadeInTime = parseFloat(document.getElementById("fadeIn").value);
    const fadeOutTime = parseFloat(document.getElementById("fadeOut").value);

    processWav(file, sampleRate, minLevel, fadeInTime, fadeOutTime, key).then(
      (processedFile) => {
        sampleMap[key] = processedFile;
        saveState();
        box.style.backgroundColor = "#d4edda";
        playSample(processedFile, key);
      }
    );
    return;
  }

  // If there's no file but we have data from a "split sample" drag:
  // (we set 'text/split-index' in dragstart below)
  const splitIndex = event.dataTransfer.getData("text/split-index");
  if (splitIndex) {
    const seg = splitResultsMap[splitIndex];
    if (!seg || !seg.blob) return;

    // same logic as above, but we already have a processed blob
    sampleMap[key] = seg.blob;
    box.style.backgroundColor = "#d4edda";
    currentSampleKey = key;
    updateCurrentSampleDisplay();
    drawWaveform(seg.buffer);
    playSample(seg.blob, key);
    saveState();
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

function clearSample(square) {
  const key = square.dataset.key; // each grid square has data-key
  delete sampleMap[key];
  saveState();
  drawWaveform(null);
  square.style.backgroundColor = "";
  console.log(`Sample cleared for key: ${key}`);
}

// --------------------------------------------------------------------------------
// Attach event listeners to A-Z boxes
// --------------------------------------------------------------------------------
document.querySelectorAll(".sampleBox").forEach((box) => {
  box.addEventListener("dragover", (event) => {
    event.preventDefault();
    box.classList.add("dragover");
  });
  box.addEventListener("dragleave", () => box.classList.remove("dragover"));
  box.addEventListener("drop", (event) => {
    box.classList.remove("dragover");
    handleFileDrop(event);
  });
  box.addEventListener("click", () => {
    const key = box.dataset.key;
    if (sampleMap[key]) {
      playSample(sampleMap[key], key);
    }
  });
  // Right-click to clear sample
  box.addEventListener("contextmenu", (event) => {
    event.preventDefault();
    clearSample(box);
  });
});

// --------------------------------------------------------------------------------
// "Download Kit" logic
// --------------------------------------------------------------------------------
document.getElementById("downloadKit").addEventListener("click", () => {
  const kitName = document.getElementById("kitName").value || "sampleKit";
  const zip = new JSZip();
  const folder = zip.folder(`base-${kitName}.kit`);

  Object.entries(sampleMap).forEach(([key, blob]) => {
    folder.file(`${key}.wav`, blob);
  });

  zip.generateAsync({ type: "blob" }).then((content) => {
    const a = document.createElement("a");
    a.href = URL.createObjectURL(content);
    a.download = `${kitName}.zip`;
    a.click();
  });
});

// --------------------------------------------------------------------------------
// "Clear" button
// --------------------------------------------------------------------------------
document.getElementById("clearState").addEventListener("click", clearState);

// --------------------------------------------------------------------------------
// Setting sliders
// --------------------------------------------------------------------------------
document.getElementById("minLevel").addEventListener("input", (event) => {
  document.getElementById("minLevelValue").textContent = event.target.value;
  saveState();
});
document.getElementById("fadeIn").addEventListener("input", (event) => {
  document.getElementById("fadeInValue").textContent = event.target.value;
  saveState();
});
document.getElementById("fadeOut").addEventListener("input", (event) => {
  document.getElementById("fadeOutValue").textContent = event.target.value;
  saveState();
});

// --------------------------------------------------------------------------------
// "Split Multi" logic
// --------------------------------------------------------------------------------

/**
 * Splits an AudioBuffer into segments by threshold. 
 * Returns an array of AudioBuffers (already trimmed, no fade).
 */
function splitAudioByThreshold(buffer, minLevelDB) {
  const channelData = buffer.getChannelData(0);
  const sampleRate = buffer.sampleRate;
  const minLevel = Math.pow(10, minLevelDB / 20);

  const frames = channelData.length;
  const segments = [];

  let i = 0;
  while (i < frames) {
    // find start
    while (i < frames && Math.abs(channelData[i]) < minLevel) {
      i++;
    }
    if (i >= frames) break;
    const start = i;

    // find end
    while (i < frames && Math.abs(channelData[i]) >= minLevel) {
      i++;
    }
    const end = i;

    // create buffer
    const length = end - start;
    if (length > 0) {
      const segBuffer = audioContext.createBuffer(1, length, sampleRate);
      segBuffer.copyToChannel(channelData.slice(start, end), 0, 0);
      segments.push(segBuffer);
    }
  }
  return segments;
}

/**
 * Given an AudioBuffer, fadeInTime, fadeOutTime, returns a new AudioBuffer with fade applied.
 */
function fadeBuffer(original, fadeInTime, fadeOutTime) {
  const sampleRate = original.sampleRate;
  const newBuffer = audioContext.createBuffer(1, original.length, sampleRate);
  const data = newBuffer.getChannelData(0);
  const sourceData = original.getChannelData(0);
  data.set(sourceData);

  const fadeInSamples = Math.floor((fadeInTime / 1000) * sampleRate);
  const fadeOutSamples = Math.floor((fadeOutTime / 1000) * sampleRate);

  // fade in
  for (let i = 0; i < fadeInSamples && i < data.length; i++) {
    data[i] *= i / fadeInSamples;
  }
  // fade out
  for (let i = 0; i < fadeOutSamples && i < data.length; i++) {
    const idx = data.length - fadeOutSamples + i;
    if (idx >= 0 && idx < data.length) {
      data[idx] *= 1 - i / fadeOutSamples;
    }
  }
  return newBuffer;
}

/**
 * Orchestrate the splitting:
 *  1. decode
 *  2. resample
 *  3. find segments
 *  4. disregard any segment shorter than fadeIn+fadeOut
 *  5. create Blob for each
 */
async function splitWavFile(file, sampleRate, minLevelDB, fadeInMs, fadeOutMs) {
  const arrayBuffer = await file.arrayBuffer();
  const audioBuffer = await audioContext.decodeAudioData(arrayBuffer);

  // Resample
  const offlineContext = new OfflineAudioContext(1, audioBuffer.length, sampleRate);
  const bufferSource = offlineContext.createBufferSource();
  bufferSource.buffer = audioBuffer;
  bufferSource.connect(offlineContext.destination);
  bufferSource.start();
  const resampledBuffer = await offlineContext.startRendering();

  // Split
  const segments = splitAudioByThreshold(resampledBuffer, minLevelDB);

  // The min acceptable length is fadeIn + fadeOut in samples
  const minSamples = Math.floor((fadeInMs + fadeOutMs) / 1000 * sampleRate);

  // Process each segment
  const results = [];
  for (const seg of segments) {
    if (seg.length < minSamples) {
      // skip
      continue;
    }
    // Trim internally again, just in case
    const trimmed = trimSilence(seg, Math.pow(10, minLevelDB / 20));
    if (trimmed.length < minSamples) {
      // skip
      continue;
    }
    const faded = fadeBuffer(trimmed, fadeInMs, fadeOutMs);

    // final offline render (to generate consistent WAV data)
    const finalOffline = new OfflineAudioContext(1, faded.length, sampleRate);
    const bs = finalOffline.createBufferSource();
    bs.buffer = faded;
    bs.connect(finalOffline.destination);
    bs.start();
    const rendered = await finalOffline.startRendering();

    // create wav blob
    const wav = audioBufferToWav(rendered);
    const blob = new Blob([wav], { type: "audio/wav" });

    results.push({ blob, buffer: rendered });
  }

  return results;
}

// "Split Multi" drop logic
const splitDropZone = document.getElementById("splitDropZone");
splitDropZone.addEventListener("dragover", (e) => {
  e.preventDefault();
  splitDropZone.style.backgroundColor = "#d4edda";
});
splitDropZone.addEventListener("dragleave", () => {
  splitDropZone.style.backgroundColor = "#f1f1f1";
});
splitDropZone.addEventListener("drop", async (e) => {
  e.preventDefault();
  splitDropZone.style.backgroundColor = "#f1f1f1";

  const file = e.dataTransfer.files[0];
  if (file && file.type === "audio/wav") {
    const sampleRate = parseInt(document.getElementById("sampleRate").value, 10);
    const minLevelDB = parseFloat(document.getElementById("minLevel").value);
    const fadeInMs = parseFloat(document.getElementById("fadeIn").value);
    const fadeOutMs = parseFloat(document.getElementById("fadeOut").value);

    const splitted = await splitWavFile(file, sampleRate, minLevelDB, fadeInMs, fadeOutMs);

    // Clear old results
    splitResultsMap = {};
    document.getElementById("splitResults").innerHTML = "";

    // Build new results
    splitted.forEach((obj, idx) => {
      splitResultsMap[idx] = obj;
      addSplitResultBox(idx, obj);
    });
  }
});

/**
 * Creates a small draggable box in #splitResults that, when clicked, plays the audio.
 * Also wires up dragstart so that it can be dropped onto the A-Z grid.
 */
function addSplitResultBox(index, { blob, buffer }) {
  const container = document.getElementById("splitResults");
  const div = document.createElement("div");
  div.className = "splitSampleBox";
  div.textContent = index;
  div.draggable = true; // important

  div.addEventListener("click", () => {
    playSample(blob, null);
  });

  div.addEventListener("dragstart", (e) => {
    // We'll store the index in the DataTransfer
    e.dataTransfer.setData("text/split-index", String(index));
    // Set an effect
    e.dataTransfer.effectAllowed = "copyMove";
    // Add a 'dragging' class if you like
    div.classList.add("dragging");
  });
  div.addEventListener("dragend", () => {
    div.classList.remove("dragging");
  });

  container.appendChild(div);
}

// --------------------------------------------------------------------------------
// Final init calls
// --------------------------------------------------------------------------------
restoreState();
