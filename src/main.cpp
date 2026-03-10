#include "AudioEngine.hpp"
#include "VoiceEffect.hpp"
#include "PersonaUI.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "[INFO] Ses motoru hazırlanıyor..." << std::endl;
    VoiceEffect* effect = new VoiceEffect(44100);
    AudioEngine engine(effect);

    if (!engine.startStream()) {
        std::cerr << "[ERROR] Ses motoru başlatılamadı!" << std::endl;
        delete effect;
        return -1;
    }
    std::cout << "[SUCCESS] Ses motoru aktif." << std::endl;

    std::cout << "[INFO] PersonaUI sistemi başlatılıyor..." << std::endl;
    PersonaUI ui("PersonaVox: Intelligence Terminal v2.0");
    
    if (!ui.init()) {
        std::cerr << "[ERROR] PersonaUI başlatılamadı!" << std::endl;
        engine.stopStream();
        delete effect;
        return -1;
    }
    std::cout << "[SUCCESS] PersonaUI hazır." << std::endl;

    bool done = false;
    while (!done) {
        ui.handleEvents(done);
        ui.startFrame();
        ui.render(effect, engine);
        ui.endFrame();
    }

    engine.stopStream();
    ui.cleanup();

    delete effect;
    return 0;
}
