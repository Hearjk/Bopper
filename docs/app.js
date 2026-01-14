// Bopper Web App - Beat-Synced GIF Player

// State
let bpm = 120;
let isPlaying = false;
let speedDivisor = 1;
let animationId = null;
let lastBeatTime = 0;
let beatCount = 0;

// GIF frame control
let gifFrames = [];
let currentFrameIndex = 0;
let originalGifSrc = null;
let isAnimatedGif = false;
let generatedFrames = []; // For image-to-gif conversion

// Effects state
let pulseEnabled = false;
let shakeEnabled = false;
let currentFilter = 'none';
let isTheaterMode = false;

// Tap tempo state
let tapTimes = [];
const TAP_RESET_TIME = 2000;

// Microphone state
let micActive = false;
let audioContext = null;
let analyser = null;
let micStream = null;
let beatDetector = null;

// DOM Elements
const gifDisplay = document.getElementById('gifDisplay');
const gifCanvas = document.getElementById('gifCanvas');
const gifFrame = document.getElementById('gifFrame');
const placeholder = document.getElementById('placeholder');
const bpmValue = document.getElementById('bpmValue');
const bpmInput = document.getElementById('bpmInput');
const playBtn = document.getElementById('playBtn');
const playIcon = document.getElementById('playIcon');
const pauseIcon = document.getElementById('pauseIcon');
const pulseBtn = document.getElementById('pulseBtn');
const shakeBtn = document.getElementById('shakeBtn');
const micBtn = document.getElementById('micBtn');
const micStatus = document.getElementById('micStatus');
const theaterMode = document.getElementById('theaterMode');
const theaterGif = document.getElementById('theaterGif');
const theaterCanvas = document.getElementById('theaterCanvas');

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    updateBPMDisplay();

    // Setup canvas contexts
    if (gifCanvas) {
        gifCanvas.ctx = gifCanvas.getContext('2d');
    }
    if (theaterCanvas) {
        theaterCanvas.ctx = theaterCanvas.getContext('2d');
    }
});

// File Upload Handling
function triggerUpload() {
    if (!gifFrame.classList.contains('has-content')) {
        document.getElementById('gifInput').click();
    }
}

function handleDragOver(e) {
    e.preventDefault();
    e.stopPropagation();
    gifFrame.classList.add('drag-over');
}

function handleDrop(e) {
    e.preventDefault();
    e.stopPropagation();
    gifFrame.classList.remove('drag-over');

    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleFileUpload(files[0]);
    }
}

// Clear current GIF and allow new upload
function clearGIF(e) {
    e.stopPropagation(); // Prevent triggering upload

    // Stop playback
    if (isPlaying) {
        togglePlayback();
    }

    // Reset state
    generatedFrames = [];
    currentFrameIndex = 0;
    originalGifSrc = null;
    isAnimatedGif = false;

    // Clear displays
    gifDisplay.src = '';
    gifDisplay.classList.remove('active');
    gifCanvas.classList.remove('active');
    if (gifCanvas.ctx) {
        gifCanvas.ctx.clearRect(0, 0, gifCanvas.width, gifCanvas.height);
    }

    // Show placeholder
    placeholder.classList.remove('hidden');
    gifFrame.classList.remove('has-content');

    // Clear theater mode
    theaterGif.src = '';
    theaterCanvas.classList.remove('active');

    // Reset file input
    document.getElementById('gifInput').value = '';
}

// Remove drag-over when leaving
document.addEventListener('DOMContentLoaded', () => {
    gifFrame.addEventListener('dragleave', (e) => {
        e.preventDefault();
        gifFrame.classList.remove('drag-over');
    });
});

function handleFileUpload(file) {
    if (!file) return;

    const isGif = file.type === 'image/gif';
    const isImage = file.type.startsWith('image/');

    if (!isImage) {
        alert('Please select an image file');
        return;
    }

    if (isGif) {
        loadGIF(file);
    } else {
        // Convert static image to animated GIF
        convertImageToGif(file);
    }
}

// BPM Functions
function setBPM(value) {
    bpm = Math.max(20, Math.min(300, parseInt(value) || 120));
    bpmInput.value = bpm;
    updateBPMDisplay();
}

function adjustBPM(delta) {
    setBPM(bpm + delta);
}

function updateBPMDisplay() {
    bpmValue.textContent = bpm;
    if (isPlaying) {
        startAnimation();
    }
}

// Tap Tempo
function tapTempo() {
    const now = Date.now();

    if (tapTimes.length > 0 && now - tapTimes[tapTimes.length - 1] > TAP_RESET_TIME) {
        tapTimes = [];
    }

    tapTimes.push(now);

    if (tapTimes.length >= 2) {
        if (tapTimes.length > 8) {
            tapTimes.shift();
        }

        let totalInterval = 0;
        for (let i = 1; i < tapTimes.length; i++) {
            totalInterval += tapTimes[i] - tapTimes[i - 1];
        }
        const avgInterval = totalInterval / (tapTimes.length - 1);
        const detectedBPM = Math.round(60000 / avgInterval);
        setBPM(detectedBPM);
    }

    const tapBtn = document.getElementById('tapBtn');
    tapBtn.style.transform = 'scale(0.95)';
    setTimeout(() => tapBtn.style.transform = '', 100);
}

// Microphone BPM Detection
async function toggleMic() {
    if (micActive) {
        stopMic();
    } else {
        await startMic();
    }
}

async function startMic() {
    try {
        audioContext = new (window.AudioContext || window.webkitAudioContext)();
        micStream = await navigator.mediaDevices.getUserMedia({ audio: true });

        const source = audioContext.createMediaStreamSource(micStream);
        analyser = audioContext.createAnalyser();
        analyser.fftSize = 2048;
        analyser.smoothingTimeConstant = 0.8;
        source.connect(analyser);

        micActive = true;
        micBtn.classList.add('active');
        micStatus.textContent = 'Listening...';

        detectBeats();
    } catch (err) {
        console.error('Microphone access denied:', err);
        alert('Could not access microphone. Please allow microphone access and try again.');
    }
}

function stopMic() {
    if (micStream) {
        micStream.getTracks().forEach(track => track.stop());
    }
    if (audioContext) {
        audioContext.close();
    }
    if (beatDetector) {
        cancelAnimationFrame(beatDetector);
    }

    micActive = false;
    micBtn.classList.remove('active');
    micStatus.textContent = 'Start';
    audioContext = null;
    analyser = null;
    micStream = null;
}

function detectBeats() {
    if (!micActive || !analyser) return;

    const bufferLength = analyser.frequencyBinCount;
    const dataArray = new Uint8Array(bufferLength);

    let lastPeak = 0;
    let peakTimes = [];
    let energyHistory = [];
    const historySize = 43;

    function analyze() {
        if (!micActive) return;

        analyser.getByteFrequencyData(dataArray);

        let energy = 0;
        const bassEnd = Math.floor(bufferLength * 0.1);
        for (let i = 0; i < bassEnd; i++) {
            energy += dataArray[i];
        }
        energy /= bassEnd;

        energyHistory.push(energy);
        if (energyHistory.length > historySize) {
            energyHistory.shift();
        }

        const avgEnergy = energyHistory.reduce((a, b) => a + b, 0) / energyHistory.length;

        const now = Date.now();
        const threshold = avgEnergy * 1.3;
        const minBeatInterval = 200;

        if (energy > threshold && energy > 50 && now - lastPeak > minBeatInterval) {
            lastPeak = now;
            peakTimes.push(now);

            peakTimes = peakTimes.filter(t => now - t < 4000);

            if (peakTimes.length >= 4) {
                let intervals = [];
                for (let i = 1; i < peakTimes.length; i++) {
                    intervals.push(peakTimes[i] - peakTimes[i - 1]);
                }

                intervals.sort((a, b) => a - b);
                const median = intervals[Math.floor(intervals.length / 2)];
                intervals = intervals.filter(i => Math.abs(i - median) < median * 0.3);

                if (intervals.length >= 2) {
                    const avgInterval = intervals.reduce((a, b) => a + b, 0) / intervals.length;
                    let detectedBPM = Math.round(60000 / avgInterval);

                    while (detectedBPM > 180) detectedBPM /= 2;
                    while (detectedBPM < 60) detectedBPM *= 2;

                    const diff = Math.abs(detectedBPM - bpm);
                    if (diff > 5) {
                        setBPM(Math.round(bpm + (detectedBPM - bpm) * 0.3));
                    }
                }
            }

            gifFrame.style.boxShadow = '0 0 30px rgba(0, 255, 136, 0.5)';
            setTimeout(() => {
                gifFrame.style.boxShadow = '';
            }, 100);
        }

        beatDetector = requestAnimationFrame(analyze);
    }

    analyze();
}

// GIF Loading with frame extraction for BPM sync
function loadGIF(file) {
    const reader = new FileReader();
    reader.onload = async (e) => {
        originalGifSrc = e.target.result;

        // Try to extract frames from the GIF for BPM-synced playback
        try {
            const arrayBuffer = await file.arrayBuffer();
            const frames = await extractGifFrames(arrayBuffer);

            if (frames && frames.length > 1) {
                // Successfully extracted frames - use canvas playback
                generatedFrames = frames;
                isAnimatedGif = false; // Use our frame-based playback
                currentFrameIndex = 0;

                // Setup canvas with frame dimensions
                const firstFrame = frames[0];
                gifCanvas.width = firstFrame.width;
                gifCanvas.height = firstFrame.height;
                theaterCanvas.width = firstFrame.width;
                theaterCanvas.height = firstFrame.height;

                // Show canvas instead of img
                gifDisplay.classList.remove('active');
                gifCanvas.classList.add('active');
                placeholder.classList.add('hidden');
                gifFrame.classList.add('has-content');

                // Draw first frame
                drawFrame(0);
            } else {
                // Fallback to native GIF playback
                fallbackToNativeGif();
            }
        } catch (err) {
            console.warn('Could not extract GIF frames, using native playback:', err);
            fallbackToNativeGif();
        }

        if (!isPlaying) {
            togglePlayback();
        }
    };
    reader.readAsDataURL(file);

    function fallbackToNativeGif() {
        isAnimatedGif = true;
        generatedFrames = [];
        gifDisplay.src = originalGifSrc;
        gifDisplay.classList.add('active');
        gifCanvas.classList.remove('active');
        placeholder.classList.add('hidden');
        gifFrame.classList.add('has-content');
        theaterGif.src = originalGifSrc;
    }
}

// Extract frames from GIF binary data
async function extractGifFrames(arrayBuffer) {
    // Use ImageDecoder API if available (modern browsers)
    if ('ImageDecoder' in window) {
        return await extractFramesWithImageDecoder(arrayBuffer);
    }

    // Fallback: parse GIF manually
    return await parseGifFrames(arrayBuffer);
}

// Modern approach using ImageDecoder API
async function extractFramesWithImageDecoder(arrayBuffer) {
    const decoder = new ImageDecoder({
        data: arrayBuffer,
        type: 'image/gif'
    });

    await decoder.decode({ frameIndex: 0 }); // Ensure decoder is ready

    const track = decoder.tracks.selectedTrack;
    const frameCount = track.frameCount;
    const frames = [];

    // Create canvases for compositing frames
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    const compositeCanvas = document.createElement('canvas');
    const compositeCtx = compositeCanvas.getContext('2d');

    // Get first frame to determine dimensions
    const firstResult = await decoder.decode({ frameIndex: 0 });
    const firstFrame = firstResult.image;

    canvas.width = firstFrame.displayWidth;
    canvas.height = firstFrame.displayHeight;
    compositeCanvas.width = canvas.width;
    compositeCanvas.height = canvas.height;

    // Clear to transparent initially
    compositeCtx.clearRect(0, 0, canvas.width, canvas.height);

    firstFrame.close();

    // Decode all frames with proper compositing
    for (let i = 0; i < frameCount; i++) {
        const result = await decoder.decode({ frameIndex: i, completeFramesOnly: true });
        const frame = result.image;

        // Draw onto composite canvas (ImageDecoder handles disposal for us in complete mode)
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.drawImage(frame, 0, 0);

        // Copy to composite
        compositeCtx.drawImage(canvas, 0, 0);

        // Store the composited frame
        frames.push(compositeCtx.getImageData(0, 0, canvas.width, canvas.height));

        frame.close();
    }

    decoder.close();
    return frames;
}

// Fallback GIF parser for browsers without ImageDecoder
async function parseGifFrames(arrayBuffer) {
    const data = new Uint8Array(arrayBuffer);

    // Verify GIF signature
    const signature = String.fromCharCode(data[0], data[1], data[2]);
    if (signature !== 'GIF') {
        throw new Error('Not a valid GIF');
    }

    // Parse GIF using pure JavaScript
    const gif = parseGIF(data);
    const frames = decompressFrames(gif);

    if (frames.length === 0) {
        throw new Error('No frames found');
    }

    // Create canvas for compositing
    const canvas = document.createElement('canvas');
    canvas.width = gif.width;
    canvas.height = gif.height;
    const ctx = canvas.getContext('2d');

    const result = [];

    for (const frame of frames) {
        // Handle disposal method
        if (frame.disposalType === 2) {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
        }

        // Create ImageData for this frame
        const imageData = ctx.createImageData(frame.width, frame.height);
        imageData.data.set(frame.pixels);

        // Draw frame at its position
        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = frame.width;
        tempCanvas.height = frame.height;
        const tempCtx = tempCanvas.getContext('2d');
        tempCtx.putImageData(imageData, 0, 0);

        ctx.drawImage(tempCanvas, frame.left, frame.top);

        // Store composited frame
        result.push(ctx.getImageData(0, 0, canvas.width, canvas.height));
    }

    return result;
}

// GIF Parser - parses GIF structure
function parseGIF(data) {
    let pos = 6; // Skip signature

    const width = data[pos] | (data[pos + 1] << 8);
    const height = data[pos + 2] | (data[pos + 3] << 8);
    const packed = data[pos + 4];
    const globalColorTableFlag = (packed & 0x80) !== 0;
    const globalColorTableSize = 2 << (packed & 0x07);
    pos += 7;

    let globalColorTable = null;
    if (globalColorTableFlag) {
        globalColorTable = [];
        for (let i = 0; i < globalColorTableSize; i++) {
            globalColorTable.push([data[pos++], data[pos++], data[pos++]]);
        }
    }

    const frames = [];
    let gce = null;

    while (pos < data.length) {
        const blockType = data[pos++];

        if (blockType === 0x21) { // Extension
            const extType = data[pos++];
            if (extType === 0xF9) { // Graphics Control Extension
                pos++; // Block size
                const gcePacked = data[pos++];
                gce = {
                    disposalMethod: (gcePacked >> 2) & 0x07,
                    transparentColorFlag: (gcePacked & 0x01) !== 0,
                    delay: data[pos] | (data[pos + 1] << 8),
                    transparentColorIndex: data[pos + 2]
                };
                pos += 3;
                pos++; // Block terminator
            } else {
                // Skip other extensions
                while (data[pos] !== 0) {
                    pos += data[pos] + 1;
                }
                pos++; // Block terminator
            }
        } else if (blockType === 0x2C) { // Image Descriptor
            const frame = {
                left: data[pos] | (data[pos + 1] << 8),
                top: data[pos + 2] | (data[pos + 3] << 8),
                width: data[pos + 4] | (data[pos + 5] << 8),
                height: data[pos + 6] | (data[pos + 7] << 8),
                localColorTable: null,
                interlaced: false,
                gce: gce
            };
            pos += 8;

            const imgPacked = data[pos++];
            const localColorTableFlag = (imgPacked & 0x80) !== 0;
            frame.interlaced = (imgPacked & 0x40) !== 0;
            const localColorTableSize = 2 << (imgPacked & 0x07);

            if (localColorTableFlag) {
                frame.localColorTable = [];
                for (let i = 0; i < localColorTableSize; i++) {
                    frame.localColorTable.push([data[pos++], data[pos++], data[pos++]]);
                }
            }

            frame.colorTable = frame.localColorTable || globalColorTable;

            // LZW minimum code size
            frame.minCodeSize = data[pos++];

            // Collect image data sub-blocks
            frame.data = [];
            while (data[pos] !== 0) {
                const blockSize = data[pos++];
                for (let i = 0; i < blockSize; i++) {
                    frame.data.push(data[pos++]);
                }
            }
            pos++; // Block terminator

            frames.push(frame);
            gce = null;
        } else if (blockType === 0x3B) { // Trailer
            break;
        } else {
            // Unknown block, try to skip
            break;
        }
    }

    return { width, height, frames };
}

// LZW Decompression
function lzwDecode(minCodeSize, data) {
    const clearCode = 1 << minCodeSize;
    const eoiCode = clearCode + 1;
    let codeSize = minCodeSize + 1;
    let codeMask = (1 << codeSize) - 1;
    let nextCode = eoiCode + 1;

    // Initialize code table
    let codeTable = [];
    for (let i = 0; i < clearCode; i++) {
        codeTable[i] = [i];
    }

    const output = [];
    let bitPos = 0;
    let bytePos = 0;
    let prevCode = -1;

    function readCode() {
        let code = 0;
        let bitsRead = 0;
        while (bitsRead < codeSize) {
            if (bytePos >= data.length) return -1;
            const bitsAvailable = 8 - bitPos;
            const bitsToRead = Math.min(codeSize - bitsRead, bitsAvailable);
            const mask = (1 << bitsToRead) - 1;
            code |= ((data[bytePos] >> bitPos) & mask) << bitsRead;
            bitsRead += bitsToRead;
            bitPos += bitsToRead;
            if (bitPos >= 8) {
                bitPos = 0;
                bytePos++;
            }
        }
        return code;
    }

    while (true) {
        const code = readCode();
        if (code === -1 || code === eoiCode) break;

        if (code === clearCode) {
            codeSize = minCodeSize + 1;
            codeMask = (1 << codeSize) - 1;
            nextCode = eoiCode + 1;
            codeTable = [];
            for (let i = 0; i < clearCode; i++) {
                codeTable[i] = [i];
            }
            prevCode = -1;
            continue;
        }

        let entry;
        if (code < nextCode) {
            entry = codeTable[code];
        } else if (code === nextCode && prevCode !== -1) {
            entry = [...codeTable[prevCode], codeTable[prevCode][0]];
        } else {
            break; // Invalid code
        }

        output.push(...entry);

        if (prevCode !== -1 && nextCode < 4096) {
            codeTable[nextCode++] = [...codeTable[prevCode], entry[0]];
            if (nextCode > codeMask && codeSize < 12) {
                codeSize++;
                codeMask = (1 << codeSize) - 1;
            }
        }

        prevCode = code;
    }

    return output;
}

// Decompress all frames
function decompressFrames(gif) {
    const result = [];

    for (const frame of gif.frames) {
        const indices = lzwDecode(frame.minCodeSize, frame.data);
        const pixels = new Uint8ClampedArray(frame.width * frame.height * 4);

        const transparentIndex = frame.gce?.transparentColorFlag ? frame.gce.transparentColorIndex : -1;

        // Handle interlacing
        let rowMapping;
        if (frame.interlaced) {
            rowMapping = [];
            const passes = [[0, 8], [4, 8], [2, 4], [1, 2]];
            let row = 0;
            for (const [start, step] of passes) {
                for (let y = start; y < frame.height; y += step) {
                    rowMapping[row++] = y;
                }
            }
        }

        for (let i = 0; i < indices.length && i < frame.width * frame.height; i++) {
            const colorIndex = indices[i];

            let pixelIndex;
            if (frame.interlaced) {
                const srcRow = Math.floor(i / frame.width);
                const col = i % frame.width;
                const destRow = rowMapping[srcRow];
                pixelIndex = (destRow * frame.width + col) * 4;
            } else {
                pixelIndex = i * 4;
            }

            if (colorIndex === transparentIndex) {
                pixels[pixelIndex] = 0;
                pixels[pixelIndex + 1] = 0;
                pixels[pixelIndex + 2] = 0;
                pixels[pixelIndex + 3] = 0;
            } else if (frame.colorTable && frame.colorTable[colorIndex]) {
                const color = frame.colorTable[colorIndex];
                pixels[pixelIndex] = color[0];
                pixels[pixelIndex + 1] = color[1];
                pixels[pixelIndex + 2] = color[2];
                pixels[pixelIndex + 3] = 255;
            }
        }

        result.push({
            left: frame.left,
            top: frame.top,
            width: frame.width,
            height: frame.height,
            pixels: pixels,
            disposalType: frame.gce?.disposalMethod || 0
        });
    }

    return result;
}

// Convert static image to animated GIF frames
function convertImageToGif(file) {
    const reader = new FileReader();
    reader.onload = (e) => {
        const img = new Image();
        img.onload = () => {
            // Generate animation frames from static image
            generatedFrames = generateAnimationFrames(img);
            isAnimatedGif = false;
            currentFrameIndex = 0;

            // Setup canvas
            const size = Math.min(img.width, img.height, 400);
            gifCanvas.width = size;
            gifCanvas.height = size;
            theaterCanvas.width = size;
            theaterCanvas.height = size;

            // Show canvas instead of img
            gifDisplay.classList.remove('active');
            gifCanvas.classList.add('active');
            placeholder.classList.add('hidden');
            gifFrame.classList.add('has-content');

            // Draw first frame
            drawFrame(0);

            if (!isPlaying) {
                togglePlayback();
            }
        };
        img.src = e.target.result;
    };
    reader.readAsDataURL(file);
}

// Generate animation frames from a static image
function generateAnimationFrames(img) {
    const frames = [];
    const numFrames = 8;
    const size = Math.min(img.width, img.height, 400);

    // Create temporary canvas for frame generation
    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = size;
    tempCanvas.height = size;
    const ctx = tempCanvas.getContext('2d');

    for (let i = 0; i < numFrames; i++) {
        const progress = i / numFrames;
        const angle = progress * Math.PI * 2;

        ctx.clearRect(0, 0, size, size);
        ctx.save();

        // Center the transformation
        ctx.translate(size / 2, size / 2);

        // Animation effect: zoom pulse + slight rotation
        const zoomFactor = 1 + Math.sin(angle) * 0.08; // 8% zoom variation
        const rotationAngle = Math.sin(angle) * 0.05; // Slight wobble

        ctx.rotate(rotationAngle);
        ctx.scale(zoomFactor, zoomFactor);

        // Draw the image centered
        const drawSize = size * 0.9;
        ctx.drawImage(img, -drawSize / 2, -drawSize / 2, drawSize, drawSize);

        ctx.restore();

        // Store frame as ImageData
        frames.push(ctx.getImageData(0, 0, size, size));
    }

    return frames;
}

// Draw a specific frame
function drawFrame(frameIndex) {
    if (generatedFrames.length === 0) return;

    const frame = generatedFrames[frameIndex % generatedFrames.length];

    if (gifCanvas.ctx) {
        gifCanvas.ctx.putImageData(frame, 0, 0);
    }
    if (theaterCanvas.ctx && isTheaterMode) {
        theaterCanvas.ctx.putImageData(frame, 0, 0);
    }
}

// Preset GIFs - Same as DAW app
async function loadPresetGIF(preset) {
    const presets = {
        spongebob: 'spongebob.gif',
        band: 'danceband.gif',
        gandalf: 'gandalf.gif'
    };

    if (!presets[preset]) return;

    originalGifSrc = presets[preset];

    // Show loading state
    placeholder.classList.remove('hidden');
    placeholder.innerHTML = '<span>Loading...</span>';

    try {
        // Fetch the GIF and extract frames
        const response = await fetch(presets[preset]);
        const arrayBuffer = await response.arrayBuffer();
        const frames = await extractGifFrames(arrayBuffer);

        if (frames && frames.length > 1) {
            generatedFrames = frames;
            isAnimatedGif = false;
            currentFrameIndex = 0;

            const firstFrame = frames[0];
            gifCanvas.width = firstFrame.width;
            gifCanvas.height = firstFrame.height;
            theaterCanvas.width = firstFrame.width;
            theaterCanvas.height = firstFrame.height;

            gifDisplay.classList.remove('active');
            gifCanvas.classList.add('active');
            placeholder.classList.add('hidden');
            gifFrame.classList.add('has-content');

            drawFrame(0);
        } else {
            fallbackToNativePreset();
        }
    } catch (err) {
        console.warn('Could not extract preset GIF frames:', err);
        fallbackToNativePreset();
    }

    if (!isPlaying) {
        togglePlayback();
    }

    // Restore placeholder content
    placeholder.innerHTML = `
        <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
            <polyline points="17 8 12 3 7 8"/>
            <line x1="12" y1="3" x2="12" y2="15"/>
        </svg>
        <span>Drag or click here to upload GIF</span>
        <span class="placeholder-hint">or upload an image to animate</span>
    `;

    function fallbackToNativePreset() {
        isAnimatedGif = true;
        generatedFrames = [];
        gifDisplay.src = presets[preset];
        gifDisplay.classList.add('active');
        gifCanvas.classList.remove('active');
        placeholder.classList.add('hidden');
        gifFrame.classList.add('has-content');
        theaterGif.src = presets[preset];
    }
}

// Playback
function togglePlayback() {
    isPlaying = !isPlaying;

    if (isPlaying) {
        playIcon.style.display = 'none';
        pauseIcon.style.display = 'block';
        startAnimation();
    } else {
        playIcon.style.display = 'block';
        pauseIcon.style.display = 'none';
        stopAnimation();
    }
}

function startAnimation() {
    stopAnimation();

    const beatInterval = (60000 / bpm) * speedDivisor;
    lastBeatTime = Date.now();

    function beat() {
        const now = Date.now();
        const elapsed = now - lastBeatTime;

        // Advance frames for generated animations - complete full cycle each beat
        if (generatedFrames.length > 0) {
            // Calculate which frame we should be on based on progress through the beat
            const progress = (elapsed % beatInterval) / beatInterval;
            const frameIndex = Math.floor(progress * generatedFrames.length);

            if (frameIndex !== currentFrameIndex) {
                currentFrameIndex = frameIndex;
                drawFrame(currentFrameIndex);
            }
        }

        // Check for beat boundary for pulse effect
        if (elapsed >= beatInterval) {
            lastBeatTime = now;
            beatCount++;

            // Trigger pulse effect on beat
            if (pulseEnabled) {
                gifFrame.classList.remove('pulse');
                void gifFrame.offsetWidth;
                gifFrame.classList.add('pulse');

                if (isTheaterMode) {
                    const theaterContainer = document.querySelector('.theater-gif-container');
                    theaterContainer.style.transform = 'scale(1.03)';
                    setTimeout(() => {
                        theaterContainer.style.transform = 'scale(1)';
                    }, 100);
                }
            }
        }

        if (isPlaying) {
            animationId = requestAnimationFrame(beat);
        }
    }

    beat();
}

function stopAnimation() {
    if (animationId) {
        cancelAnimationFrame(animationId);
        animationId = null;
    }
}

function setSpeed(value) {
    speedDivisor = parseInt(value);
    if (isPlaying) {
        startAnimation();
    }
}

// Effects
function toggleEffect(effect) {
    if (effect === 'pulse') {
        pulseEnabled = !pulseEnabled;
        pulseBtn.classList.toggle('active', pulseEnabled);
        if (!pulseEnabled) {
            gifFrame.classList.remove('pulse');
        }
    } else if (effect === 'shake') {
        shakeEnabled = !shakeEnabled;
        shakeBtn.classList.toggle('active', shakeEnabled);
        gifFrame.classList.toggle('shake', shakeEnabled);
    }
}

function setFilter(filter) {
    gifFrame.classList.remove('filter-invert', 'filter-sepia', 'filter-cyber', 'filter-vapor', 'filter-matrix');

    currentFilter = filter;
    if (filter !== 'none') {
        gifFrame.classList.add(`filter-${filter}`);
    }
}

// Theater Mode
function enterTheaterMode() {
    isTheaterMode = true;
    theaterMode.classList.add('active');
    document.body.style.overflow = 'hidden';

    // Copy current state to theater
    if (generatedFrames.length > 0) {
        // Use canvas for BPM-synced playback (both extracted GIF frames and generated frames)
        theaterGif.classList.remove('active');
        theaterCanvas.classList.add('active');
        theaterCanvas.width = gifCanvas.width;
        theaterCanvas.height = gifCanvas.height;
        drawFrame(currentFrameIndex);
    } else if (isAnimatedGif && originalGifSrc) {
        // Fallback to native GIF
        theaterGif.src = originalGifSrc;
        theaterGif.classList.add('active');
        theaterCanvas.classList.remove('active');
    }

    // Apply current filter
    const theaterContainer = document.querySelector('.theater-gif-container');
    theaterContainer.className = 'theater-gif-container';
    if (currentFilter !== 'none') {
        theaterGif.style.filter = getFilterStyle(currentFilter);
        theaterCanvas.style.filter = getFilterStyle(currentFilter);
    } else {
        theaterGif.style.filter = '';
        theaterCanvas.style.filter = '';
    }
}

function exitTheaterMode() {
    isTheaterMode = false;
    theaterMode.classList.remove('active');
    document.body.style.overflow = '';
}

function getFilterStyle(filter) {
    switch(filter) {
        case 'invert': return 'invert(1)';
        case 'sepia': return 'sepia(1)';
        case 'cyber': return 'hue-rotate(180deg) saturate(1.5)';
        case 'vapor': return 'hue-rotate(270deg) saturate(1.3) brightness(1.1)';
        case 'matrix': return 'hue-rotate(90deg) saturate(2) brightness(0.8)';
        default: return '';
    }
}

// Keyboard shortcuts
document.addEventListener('keydown', (e) => {
    if (e.target.tagName === 'INPUT') return;

    switch(e.code) {
        case 'Space':
            e.preventDefault();
            togglePlayback();
            break;
        case 'KeyT':
            tapTempo();
            break;
        case 'KeyM':
            toggleMic();
            break;
        case 'KeyF':
            if (!isTheaterMode) {
                enterTheaterMode();
            }
            break;
        case 'Escape':
            if (isTheaterMode) {
                exitTheaterMode();
            }
            break;
        case 'ArrowUp':
            adjustBPM(1);
            break;
        case 'ArrowDown':
            adjustBPM(-1);
            break;
    }
});
