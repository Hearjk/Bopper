// Bopper Web App - Beat-Synced GIF Player

// State
let bpm = 120;
let isPlaying = false;
let speedDivisor = 1;
let gifFrames = [];
let currentFrame = 0;
let animationId = null;
let lastBeatTime = 0;

// Effects state
let pulseEnabled = false;
let shakeEnabled = false;
let currentFilter = 'none';

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

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    updateBPMDisplay();
});

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
    // Update animation speed if playing
    if (isPlaying) {
        startAnimation();
    }
}

// Tap Tempo
function tapTempo() {
    const now = Date.now();

    // Reset if too much time has passed
    if (tapTimes.length > 0 && now - tapTimes[tapTimes.length - 1] > TAP_RESET_TIME) {
        tapTimes = [];
    }

    tapTimes.push(now);

    // Need at least 2 taps to calculate BPM
    if (tapTimes.length >= 2) {
        // Keep only last 8 taps for accuracy
        if (tapTimes.length > 8) {
            tapTimes.shift();
        }

        // Calculate average interval
        let totalInterval = 0;
        for (let i = 1; i < tapTimes.length; i++) {
            totalInterval += tapTimes[i] - tapTimes[i - 1];
        }
        const avgInterval = totalInterval / (tapTimes.length - 1);

        // Convert to BPM
        const detectedBPM = Math.round(60000 / avgInterval);
        setBPM(detectedBPM);
    }

    // Visual feedback
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

        // Start beat detection
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
    const historySize = 43; // ~1 second at 60fps

    function analyze() {
        if (!micActive) return;

        analyser.getByteFrequencyData(dataArray);

        // Calculate energy in bass frequencies (where beats usually are)
        let energy = 0;
        const bassEnd = Math.floor(bufferLength * 0.1); // Focus on low frequencies
        for (let i = 0; i < bassEnd; i++) {
            energy += dataArray[i];
        }
        energy /= bassEnd;

        // Keep energy history
        energyHistory.push(energy);
        if (energyHistory.length > historySize) {
            energyHistory.shift();
        }

        // Calculate average energy
        const avgEnergy = energyHistory.reduce((a, b) => a + b, 0) / energyHistory.length;

        // Detect beat if current energy is significantly above average
        const now = Date.now();
        const threshold = avgEnergy * 1.3;
        const minBeatInterval = 200; // Minimum 300 BPM

        if (energy > threshold && energy > 50 && now - lastPeak > minBeatInterval) {
            lastPeak = now;
            peakTimes.push(now);

            // Keep only recent peaks
            peakTimes = peakTimes.filter(t => now - t < 4000);

            // Calculate BPM from peaks
            if (peakTimes.length >= 4) {
                let intervals = [];
                for (let i = 1; i < peakTimes.length; i++) {
                    intervals.push(peakTimes[i] - peakTimes[i - 1]);
                }

                // Filter outliers
                intervals.sort((a, b) => a - b);
                const median = intervals[Math.floor(intervals.length / 2)];
                intervals = intervals.filter(i => Math.abs(i - median) < median * 0.3);

                if (intervals.length >= 2) {
                    const avgInterval = intervals.reduce((a, b) => a + b, 0) / intervals.length;
                    let detectedBPM = Math.round(60000 / avgInterval);

                    // Normalize to reasonable BPM range
                    while (detectedBPM > 180) detectedBPM /= 2;
                    while (detectedBPM < 60) detectedBPM *= 2;

                    // Smooth BPM changes
                    const diff = Math.abs(detectedBPM - bpm);
                    if (diff > 5) {
                        setBPM(Math.round(bpm + (detectedBPM - bpm) * 0.3));
                    }
                }
            }

            // Visual feedback on beat
            gifFrame.style.boxShadow = '0 0 30px rgba(0, 255, 136, 0.5)';
            setTimeout(() => {
                gifFrame.style.boxShadow = '';
            }, 100);
        }

        beatDetector = requestAnimationFrame(analyze);
    }

    analyze();
}

// GIF Loading
function loadGIF(file) {
    if (!file || !file.type.includes('gif')) {
        alert('Please select a GIF file');
        return;
    }

    const reader = new FileReader();
    reader.onload = (e) => {
        gifDisplay.src = e.target.result;
        gifDisplay.classList.add('active');
        placeholder.classList.add('hidden');

        // Auto-start playback
        if (!isPlaying) {
            togglePlayback();
        }
    };
    reader.readAsDataURL(file);
}

function loadPresetGIF(preset) {
    // Using placeholder GIFs from giphy
    const presets = {
        dance: 'https://media.giphy.com/media/l0MYt5jPR6QX5pnqM/giphy.gif',
        cat: 'https://media.giphy.com/media/JIX9t2j0ZTN9S/giphy.gif',
        nyan: 'https://media.giphy.com/media/sIIhZliB2McAo/giphy.gif'
    };

    if (presets[preset]) {
        gifDisplay.src = presets[preset];
        gifDisplay.classList.add('active');
        placeholder.classList.add('hidden');

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

    // Calculate beat interval
    const beatInterval = (60000 / bpm) * speedDivisor;

    function beat() {
        const now = Date.now();

        if (now - lastBeatTime >= beatInterval) {
            lastBeatTime = now;

            // Trigger pulse effect on beat
            if (pulseEnabled) {
                gifFrame.classList.remove('pulse');
                void gifFrame.offsetWidth; // Trigger reflow
                gifFrame.classList.add('pulse');
            }
        }

        if (isPlaying) {
            animationId = requestAnimationFrame(beat);
        }
    }

    lastBeatTime = Date.now();
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
    // Remove all filter classes
    gifFrame.classList.remove('filter-invert', 'filter-sepia', 'filter-cyber', 'filter-vapor', 'filter-matrix');

    currentFilter = filter;
    if (filter !== 'none') {
        gifFrame.classList.add(`filter-${filter}`);
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
        case 'ArrowUp':
            adjustBPM(1);
            break;
        case 'ArrowDown':
            adjustBPM(-1);
            break;
    }
});
