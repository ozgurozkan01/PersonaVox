#include "AudioEngine.hpp"
#include <iostream>
#include <algorithm>

/**
 * @brief Constructor: PortAudio'yu hazır hale getirir ve efekti kaydeder.
 * 
 * @param effect: Kullanılacak olan ses efekti nesnesi.
 */
AudioEngine::AudioEngine(IEffect* effect) : m_currentEffect(effect) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio başlatılamadı: " << Pa_GetErrorText(err) << std::endl;
        return;
    }
    m_isInitialized = true;
}

AudioEngine::~AudioEngine() {
    stopStream();
    Pa_Terminate();
}

bool AudioEngine::startStream() {
    if (!m_isInitialized) return false;
    
    PaError err = Pa_OpenDefaultStream(&m_stream, 
                                       1,          // Input (Mic)
                                       1,          // Output (Speaker)
                                       paFloat32, 
                                       44100, 
                                       512, 
                                       paCallback, 
                                       this);

    if (err != paNoError) return false;

    err = Pa_StartStream(m_stream);
    return err == paNoError;
}

void AudioEngine::stopStream() {
    if (m_stream) {
        Pa_StopStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
    }
}

int AudioEngine::paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData) {
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    if (!out) return paContinue;
    if (!engine) return paContinue;

    // 1. Veriyi kopyala (Mikrofon varsa kopyala, yoksa sessizlik ver)
    if (in) {
        std::copy(in, in + framesPerBuffer, out);
    } else {
        std::fill(out, out + framesPerBuffer, 0.0f);
    }

    // 2. Efekti uygula (Constructor'da aldığımız efekt)
    if (engine->m_currentEffect) {
        engine->m_currentEffect->process(out, framesPerBuffer);
    }

    // 3. UI için kilitlenmeden veriyi paylaşmayı dene
    if (engine->m_bufferMutex.try_lock()) {
        engine->m_lastBuffer.assign(out, out + framesPerBuffer);
        engine->m_bufferMutex.unlock();
    }

    return paContinue;
}

std::vector<float> AudioEngine::getLastAudioBuffer() {
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    return m_lastBuffer;
}
