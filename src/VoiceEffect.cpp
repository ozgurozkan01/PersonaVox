#include "VoiceEffect.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <fstream>

VoiceEffect::VoiceEffect(int sampleRate) : m_sampleRate(sampleRate) {
    m_soundTouch.setSampleRate(m_sampleRate);
    m_soundTouch.setChannels(1); 
    
    m_soundTouch.setSetting(SETTING_USE_QUICKSEEK, 1);
    m_soundTouch.setSetting(SETTING_USE_AA_FILTER, 0); 

    // Layer 3: 500ms buffer initialization
    m_glitchBuffer.assign(m_sampleRate / 2, 0.0f);

    // Forensic Capture: 1 second buffer (44100 samples)
    m_inputCaptureBuffer.reserve(m_sampleRate);
    m_outputCaptureBuffer.reserve(m_sampleRate);
}

void VoiceEffect::process(float* buffer, unsigned long frameCount) {
    if (!buffer || frameCount == 0 || m_glitchBuffer.empty()) return;

    // Capture INPUT before any effects
    float inEnt = calculateShannonEntropy(buffer, frameCount);
    inputEntropy.store(inEnt);
    
    // Smooth INPUT for UI (0.98 means ~50 buffers or 0.5s window)
    smoothedInputEntropy.store(smoothedInputEntropy.load() * 0.98f + inEnt * 0.02f);

    if (m_shouldCapture.load()) {
        for (unsigned long i = 0; i < frameCount && m_inputCaptureBuffer.size() < m_sampleRate; ++i) {
            m_inputCaptureBuffer.push_back(buffer[i]);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 1. ADIM: Kalınlaştırma (Pitch Shift)
    if (layer1Enabled.load()) {
        applyPitchShift(buffer, frameCount, layer1Intensity.load());
    }

    // 4. ADIM: Mechanic/Robotick (Ring Modulation)
    if (layer2Enabled.load()) {
        float rFreq = 50.0f + (layer2Intensity.load() * 4000.0f);
        // Formant Protection: Avoid 300Hz - 3000Hz if possible or shift intensity
        if (rFreq > 300.0f && rFreq < 3000.0f) {
            // Shift to higher frequency for better intelligibility
            rFreq += 2700.0f; 
        }
        currentRingFreqHz.store(rFreq);
        applyRingMod(buffer, frameCount, layer2Intensity.load(), rFreq);
    }

    // 3. ADIM: Dijital Glitch (Stutter)
    if (layer3Enabled.load()) {
        applyGlitch(buffer, frameCount, layer3Intensity.load());
    }

    // 4. ADIM: Güvenlik Sınırı (Gain Staging & Peak Limiting)
    float peak = 0.0f;
    for (unsigned long i = 0; i < frameCount; ++i) {
        // Gain reduction to prevent clipping when summing layers
        buffer[i] *= 0.7f; 
        
        // Safety limiter
        if (buffer[i] > 1.0f) buffer[i] = 1.0f;
        if (buffer[i] < -1.0f) buffer[i] = -1.0f;

        float absVal = std::abs(buffer[i]);
        if (absVal > peak) peak = absVal;
    }
    currentPeak.store(peak);

    // Calculate Output Entropy & Real-time Delta
    float outEnt = calculateShannonEntropy(buffer, frameCount);
    outputEntropy.store(outEnt);
    
    // Smooth OUTPUT for UI
    smoothedOutputEntropy.store(smoothedOutputEntropy.load() * 0.98f + outEnt * 0.02f);
    
    float delta = outEnt - inEnt;
    
    // SAFETY WATCHDOG: Check if masking is sufficient (Instantaneous)
    bool secure = (delta >= securityThreshold);
    isSecure.store(secure);

    // Emergency Mute: Prevent biometric leakage if security threshold is not met
    if (safetyMuteEnabled.load() && !secure) {
        std::memset(buffer, 0, frameCount * sizeof(float));
    }

    // VERY Heavy smoothing for session average (0.999 means stable long-term score)
    avgDeltaEntropy.store(avgDeltaEntropy.load() * 0.999f + delta * 0.001f);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> duration = end - start;
    lastProcessTimeMs.store(duration.count());

    // 5. ADIM: Low-Level Dithering & Intelligent Scrambling
    float dither = ditherIntensity.load();
    float sRatio = scrambleRatio.load();
    bool useEnv = envelopeSensingEnabled.load();

    for (unsigned long i = 0; i < frameCount; ++i) {
        // ENVELOPE TRACKING (Attack/Release approach)
        float absVal = std::fabs(buffer[i]);
        if (absVal > m_envelope) m_envelope = m_envelope * 0.9f + absVal * 0.1f; // Fast attack
        else m_envelope = m_envelope * 0.999f + absVal * 0.001f; // Slow release
        
        // Intelligent Scrambling: SIGNIFICANTLY more noise for real forensic masking
        float noiseMod = useEnv ? (1.0f - m_envelope) : 1.0f;
        // Increase noise intensity multiplier to 0.5f (was 0.05f)
        float noise = (fastRnd() - 0.5f) * (dither + sRatio * noiseMod * 0.5f);
        
        // LIMITER instead of heavy compression: Keep the signal shape
        float val = buffer[i] + noise;
        if (val > 0.98f) val = 0.98f;
        if (val < -0.98f) val = -0.98f;
        buffer[i] = val;
        
        if (std::fabs(buffer[i]) > peak) peak = std::fabs(buffer[i]);
    }
    currentPeak.store(peak);

    // Capture OUTPUT after all effects
    if (m_shouldCapture.load()) {
        for (unsigned long i = 0; i < frameCount && m_outputCaptureBuffer.size() < m_sampleRate; ++i) {
            m_outputCaptureBuffer.push_back(buffer[i]);
        }
        
        // If buffer is full, stop capture and save
        if (m_inputCaptureBuffer.size() >= m_sampleRate && m_outputCaptureBuffer.size() >= m_sampleRate) {
            m_shouldCapture.store(false);
            saveCaptureToDisk();
        }
    }
}

/**
 * @brief İSTASYON 1: Sesi Kalınlaştırma (Pitch Shift)
 */
void VoiceEffect::applyPitchShift(float* buffer, unsigned long frameCount, float intensity) {
    // intensity 0.0 ile 1.0 arası gelir.
    float basePitch = -1.0f - (intensity * 7.0f);
    
    // SMART MASKING: Micro-Pitch Jittering
    // Millisecond-scale instability to confuse forensic models
    float jitter = (fastRnd() - 0.5f) * jitterIntensity.load() * 0.2f; 
    float pitchAmount = basePitch + jitter;

    m_soundTouch.setPitchSemiTones(pitchAmount);

    // Telemetry: Hz hesabı (Yaklaşık)
    currentPitchHz.store(440.0f * std::pow(2.0f, pitchAmount / 12.0f));

    m_soundTouch.putSamples(buffer, frameCount);
    uint32_t received = m_soundTouch.receiveSamples(buffer, frameCount);
    if (received < frameCount) {
        std::memset(buffer + received, 0, (frameCount - received) * sizeof(float));
    }
}

/**
 * @brief İSTASYON 2: Mekanik Ses (Ring Modulation)
 */
void VoiceEffect::applyRingMod(float* buffer, unsigned long frameCount, float intensity, float freq) {
    // Her bir örnekte sinüs dalgasının ne kadar ilerleyeceğini hesapla
    float step = (2.0f * 3.14159265f * freq) / m_sampleRate;

    for (unsigned long i = 0; i < frameCount; ++i) {
        float lfo = std::sin(m_phase);
        
        // INTELLIGENT SCRAMBLING: Harmonic Noise Injection
        // Sinyale karakter katan ama reverse-engineering'i zorlaştıran gürültü
        float harmonicNoise = (fastRnd() - 0.5f) * intensity * 0.02f;
        
        buffer[i] = (buffer[i] * (1.0f - intensity)) + (buffer[i] * lfo * intensity) + harmonicNoise;
        
        m_phase += step;
        if (m_phase > 2.0f * 3.14159265f) m_phase -= 2.0f * 3.14159265f;
    }
    
    currentPhaseMonitor.store(m_phase / (2.0f * 3.14159265f));
}

/**
 * @brief İSTASYON 3: Digital Glitch (Stuttering)
 */
void VoiceEffect::applyGlitch(float* buffer, unsigned long frameCount, float intensity) {
    if (intensity <= 0.01f) return; // Çok düşükse hiç uğraşma
    
    // 1. Grain Boyutunu Küçült (0.005s ile 0.03s arası olsun)
    // Bu değerler sesin anlaşılır kalmasını sağlar ama ritmi bozar.
    unsigned int minGrain = static_cast<unsigned int>(m_sampleRate * 0.005f); // 5ms
    unsigned int maxGrain = static_cast<unsigned int>(m_sampleRate * 0.030f); // 30ms
    
    // intensity arttıkça grain süresini değiştir
    unsigned int grainSize = minGrain + static_cast<unsigned int>(intensity * (maxGrain - minGrain));

    for (unsigned long i = 0; i < frameCount; ++i) {
        // ... (Buffer'a kaydetme kısmı kalsın) ...
        m_glitchBuffer[m_glitchReadPos] = buffer[i];
        
        // 2. Glitch Tetikleme Sıklığını Ayarla
        // Intensity 1.0 ise sürekli glitch, intensity 0.1 ise çok seyrek
        if ( (rand() % 100) < (intensity * 20) ) { // Her 100 sample'da bir şans dene
            
            // 3. Stutter (Takılma) Efekti
            // buffer'ın o anki değerini, geçmişteki bir parça ile değiştir
            unsigned int readIndex = (m_glitchReadPos + m_glitchBuffer.size() - grainSize) % m_glitchBuffer.size();
            float stutterSample = m_glitchBuffer[readIndex];
            
            // Karıştırma (Wet/Dry)
            // Intensity %100 olsa bile orijinali tamamen yok etme, %30'unu tut.
            float mix = intensity * 0.5f; 
            buffer[i] = (buffer[i] * (1.0f - mix)) + (stutterSample * mix);
        }

        m_glitchReadPos = (m_glitchReadPos + 1) % m_glitchBuffer.size();
    }
}

void VoiceEffect::triggerCapture() {
    m_inputCaptureBuffer.clear();
    m_outputCaptureBuffer.clear();
    m_shouldCapture.store(true);
}

void VoiceEffect::saveCaptureToDisk() {
    std::ofstream inFile("/tmp/persona_input.raw", std::ios::binary);
    if (inFile) inFile.write(reinterpret_cast<const char*>(m_inputCaptureBuffer.data()), m_inputCaptureBuffer.size() * sizeof(float));
    
    std::ofstream outFile("/tmp/persona_output.raw", std::ios::binary);
    if (outFile) outFile.write(reinterpret_cast<const char*>(m_outputCaptureBuffer.data()), m_outputCaptureBuffer.size() * sizeof(float));
}

float VoiceEffect::fastRnd() {
    m_noiseSeed = (214013 * m_noiseSeed + 2531011);
    return static_cast<float>((m_noiseSeed >> 16) & 0x7FFF) / 32768.0f;
}

float VoiceEffect::calculateShannonEntropy(const float* buffer, unsigned long frameCount) {
    if (frameCount < 2) return 0.0f;

    const int numBins = 64; 
    int bins[numBins] = {0};

    // 1. DIFFERENTIAL ANALYSIS (Forensic Standard)
    // Instead of looking at raw amplitude, we look at the CHANGE between samples.
    // Clean speech has smooth changes; scrambled speech is chaotic.
    float maxDiff = 0.0001f;
    for (unsigned long i = 1; i < frameCount; ++i) {
        float diff = std::fabs(buffer[i] - buffer[i-1]); // 1st derivative
        if (diff > maxDiff) maxDiff = diff;
    }
    float invMax = 1.0f / maxDiff;

    // 2. Binning the changes
    for (unsigned long i = 1; i < frameCount; ++i) {
        float diff = (buffer[i] - buffer[i-1]); 
        // Normalize diff to -1.0..1.0 range
        float normDiff = diff * invMax;
        
        int binIdx = static_cast<int>((normDiff + 1.0f) * 0.5f * (numBins - 1));
        if (binIdx < 0) binIdx = 0;
        if (binIdx >= numBins) binIdx = numBins - 1;
        bins[binIdx]++;
    }

    // 2. Compute Entropy: -sum(p * log2(p))
    float entropy = 0.0f;
    float invTotal = 1.0f / static_cast<float>(frameCount);
    
    for (int i = 0; i < numBins; ++i) {
        if (bins[i] > 0) {
            float p = static_cast<float>(bins[i]) * invTotal;
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}
