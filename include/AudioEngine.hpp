#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <portaudio.h>
#include <vector>
#include <mutex>
#include "IEffect.hpp"

class AudioEngine {
public:
    AudioEngine(IEffect* effect);
    ~AudioEngine();

    bool startStream();
    void stopStream();

    std::vector<float> getLastAudioBuffer();

private:
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData);

    PaStream* m_stream{nullptr};
    bool m_isInitialized{false};
    IEffect* m_currentEffect{nullptr};

    std::vector<float> m_lastBuffer;
    std::mutex m_bufferMutex;
};

#endif
