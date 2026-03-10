#ifndef PERSONA_UI_HPP
#define PERSONA_UI_HPP

#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include "VoiceEffect.hpp"
#include "AudioEngine.hpp"
#include <vector>
#include <string>

class PersonaUI {
public:
    PersonaUI(const std::string& windowTitle = "PersonaVox: Intelligence Control Panel");
    ~PersonaUI();

    bool init();
    void handleEvents(bool& done);
    void startFrame();
    void render(VoiceEffect* effect, AudioEngine& engine);
    void endFrame();
    void cleanup();

private:
    void setupTheme();
    void drawMainPanel(VoiceEffect* effect, AudioEngine& engine);

    std::string m_title;
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    ImGuiIO* m_io;

    int m_width;
    int m_height;
};

#endif 
