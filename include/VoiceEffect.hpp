#ifndef VOICE_EFFECT_HPP
#define VOICE_EFFECT_HPP

#include "IEffect.hpp"
#include <SoundTouch.h>
#include <atomic>
#include <vector>

class VoiceEffect : public IEffect {
public:
    VoiceEffect(int sampleRate = 44100);
    virtual ~VoiceEffect() = default;

    std::atomic<bool> layer1Enabled{false};
    std::atomic<bool> layer2Enabled{false};
    std::atomic<bool> layer3Enabled{false};

    std::atomic<float> layer1Intensity{1.0f};
    std::atomic<float> layer2Intensity{0.35f};
    std::atomic<float> layer3Intensity{0.25f};

    std::atomic<float> jitterIntensity{0.1f};
    std::atomic<float> ditherIntensity{0.001f};

    std::atomic<float> currentPitchHz{0.0f};
    std::atomic<float> currentRingFreqHz{0.0f};
    std::atomic<float> currentPhaseMonitor{0.0f};
    std::atomic<float> lastProcessTimeMs{0.0f};
    std::atomic<float> currentPeak{0.0f};
    
    // Real-time Forensic Metrics
    std::atomic<float> inputEntropy{0.0f};
    std::atomic<float> outputEntropy{0.0f};
    std::atomic<float> avgDeltaEntropy{0.0f};

    // Safety Interlock & Alerting
    std::atomic<bool> isSecure{true};
    std::atomic<bool> safetyMuteEnabled{false};
    std::atomic<bool> envelopeSensingEnabled{true};
    std::atomic<float> scrambleRatio{0.2f};
    
    float securityThreshold{0.15f};

    // Smoothed UI Metrics (for readability)
    std::atomic<float> smoothedInputEntropy{0.0f};
    std::atomic<float> smoothedOutputEntropy{0.0f};

    void process(float* buffer, unsigned long frameCount) override;
    void triggerCapture();

private:
    int m_sampleRate;

    soundtouch::SoundTouch m_soundTouch;

    float m_phase{0.0f};

    // 3. Digital Glitch (Digital Stutter)
    std::vector<float> m_glitchBuffer;
    unsigned int m_glitchReadPos{0};
    unsigned int m_glitchIteration{0};

    void applyPitchShift(float* buffer, unsigned long frameCount, float intensity);
    void applyRingMod(float* buffer, unsigned long frameCount, float intensity, float freq);
    void applyGlitch(float* buffer, unsigned long frameCount, float intensity);

    float calculateShannonEntropy(const float* buffer, unsigned long frameCount);

    // Smart Masking State
    unsigned int m_noiseSeed{12345};
    float m_envelope{0.0f};
    float fastRnd();

    // Forensic Capture state
    std::atomic<bool> m_shouldCapture{false};
    unsigned long m_captureCount{0};
    std::vector<float> m_inputCaptureBuffer;
    std::vector<float> m_outputCaptureBuffer;

    void saveCaptureToDisk();
};

#endif 
