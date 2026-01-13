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
    reader.onload = (e) => {
        originalGifSrc = e.target.result;
        isAnimatedGif = true;
        generatedFrames = [];

        // Show the GIF directly (browser handles animation)
        gifDisplay.src = originalGifSrc;
        gifDisplay.classList.add('active');
        gifCanvas.classList.remove('active');
        placeholder.classList.add('hidden');
        gifFrame.classList.add('has-content');

        // Update theater mode
        theaterGif.src = originalGifSrc;

        if (!isPlaying) {
            togglePlayback();
        }
    };
    reader.readAsDataURL(file);
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
function loadPresetGIF(preset) {
    const presets = {
        spongebob: 'https://media.giphy.com/media/nDSlfqf0gn5g4/giphy.gif',
        band: 'https://media.giphy.com/media/l46CyJmS9KUbokzsI/giphy.gif',
        gandalf: 'https://media.giphy.com/media/TcdpZwYDPlWXC/giphy.gif'
    };

    if (presets[preset]) {
        originalGifSrc = presets[preset];
        isAnimatedGif = true;
        generatedFrames = [];

        gifDisplay.src = presets[preset];
        gifDisplay.classList.add('active');
        gifCanvas.classList.remove('active');
        placeholder.classList.add('hidden');
        gifFrame.classList.add('has-content');

        theaterGif.src = presets[preset];

        if (!isPlaying) {
            togglePlayback();
        }
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

        if (now - lastBeatTime >= beatInterval) {
            lastBeatTime = now;
            beatCount++;

            // Advance frame for generated animations
            if (generatedFrames.length > 0) {
                currentFrameIndex = (currentFrameIndex + 1) % generatedFrames.length;
                drawFrame(currentFrameIndex);
            }

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
    if (isAnimatedGif && originalGifSrc) {
        theaterGif.src = originalGifSrc;
        theaterGif.classList.add('active');
        theaterCanvas.classList.remove('active');
    } else if (generatedFrames.length > 0) {
        theaterGif.classList.remove('active');
        theaterCanvas.classList.add('active');
        theaterCanvas.width = gifCanvas.width;
        theaterCanvas.height = gifCanvas.height;
        drawFrame(currentFrameIndex);
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
