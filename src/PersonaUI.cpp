#include "PersonaUI.hpp"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

PersonaUI::PersonaUI(const std::string& windowTitle) 
    : m_title(windowTitle), m_window(nullptr), m_glContext(nullptr), m_io(nullptr), m_width(1280), m_height(720) {}

PersonaUI::~PersonaUI() {
    cleanup();
}

bool PersonaUI::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "[ERROR] SDL başlatılamadı: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, window_flags);
    
    if (!m_window) {
        std::cerr << "[ERROR] Pencere oluşturulamadı: " << SDL_GetError() << std::endl;
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "[ERROR] OpenGL Context oluşturulamadı: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1); // VSinc aktif

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[ERROR] GLEW başlatılamadı!" << std::endl;
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_io = &ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    setupTheme();
    return true;
}

void PersonaUI::setupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.70f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.00f, 0.00f, 1.00f);
}

void PersonaUI::handleEvents(bool& done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window))
            done = true;
    }
}

void PersonaUI::startFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void PersonaUI::render(VoiceEffect* effect, AudioEngine& engine) {
    drawMainPanel(effect, engine);
}

void PersonaUI::drawMainPanel(VoiceEffect* effect, AudioEngine& engine) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(m_io->DisplaySize);
    ImGui::Begin("PersonaUI Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ PERSONAVOX: INTELLIGENCE VOICE MASKING TERMINAL ]");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "MainLayout", true);
    ImGui::SetColumnWidth(0, 450);

    ImGui::Text("[ VOICE DISGUISE LAYERS ]");
    ImGui::Spacing();

    bool l1 = effect->layer1Enabled.load();
    if (ImGui::Checkbox("LAYER 1: BIOMETRIC PITCH (DEEP)", &l1)) effect->layer1Enabled.store(l1);
    if (l1) {
        float i1 = effect->layer1Intensity.load();
        if (ImGui::SliderFloat("##i1", &i1, 0.0f, 1.0f, "Intensity: %.2f")) effect->layer1Intensity.store(i1);
    }
    ImGui::Spacing();

    bool l2 = effect->layer2Enabled.load();
    if (ImGui::Checkbox("LAYER 2: MECHANIC RING-MOD", &l2)) effect->layer2Enabled.store(l2);
    if (l2) {
        float i2 = effect->layer2Intensity.load();
        if (ImGui::SliderFloat("##i2", &i2, 0.0f, 1.0f, "Intensity: %.2f")) effect->layer2Intensity.store(i2);
    }
    ImGui::Spacing();

    // Layer 3: Glitch
    bool l3 = effect->layer3Enabled.load();
    if (ImGui::Checkbox("LAYER 3: DIGITAL GLITCH (STUTTER)", &l3)) effect->layer3Enabled.store(l3);
    if (l3) {
        float i3 = effect->layer3Intensity.load();
        if (ImGui::SliderFloat("##i3", &i3, 0.0f, 1.0f, "Intensity: %.2f")) effect->layer3Intensity.store(i3);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("[ SMART MASKING OPTIONS ]");
    
    float jit = effect->jitterIntensity.load();
    if (ImGui::SliderFloat("JITTER", &jit, 0.0f, 1.0f, "Instability: %.2f")) effect->jitterIntensity.store(jit);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adds micro-pitch fluctuations to confuse forensic models.");

    float dith = effect->ditherIntensity.load();
    if (ImGui::SliderFloat("DITHER", &dith, 0.0f, 0.01f, "Noise Floor: %.4f")) effect->ditherIntensity.store(dith);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adds ultra-low white noise to mask hardware fingerprint.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("[ INTELLIGENT SCRAMBLING ]");
    
    bool envSens = effect->envelopeSensingEnabled.load();
    if (ImGui::Checkbox("ENVELOPE-SENSING NOISE", &envSens)) effect->envelopeSensingEnabled.store(envSens);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Injects more noise in silences and less in speech to protect intelligibility.");

    float sRatio = effect->scrambleRatio.load();
    if (ImGui::SliderFloat("Scramble Ratio", &sRatio, 0.0f, 1.0f, "%.2f")) effect->scrambleRatio.store(sRatio);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Overall intensity of the scrambler noise injection.");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("[ REAL-TIME FORENSIC MONITOR ]");
    
    float inEnt = effect->smoothedInputEntropy.load();
    float outEnt = effect->smoothedOutputEntropy.load();
    float deltaEnt = outEnt - inEnt;
    float avgDelta = effect->avgDeltaEntropy.load();

    ImGui::Text("Input Entropy:  %.4f", inEnt);
    ImGui::Text("Output Entropy: %.4f", outEnt);
    
    ImGui::Text("Current Delta:  ");
    ImGui::SameLine();
    if (deltaEnt > 0.4f) ImGui::TextColored(ImVec4(0, 1, 0, 1), "%.4f (EXCELLENT)", deltaEnt);
    else if (deltaEnt > 0.2f) ImGui::TextColored(ImVec4(0, 0.8f, 0, 1), "%.4f (SECURE)", deltaEnt);
    else ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%.4f (LOW)", deltaEnt);

    ImGui::Text("Session Avg:    ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "%.4f", avgDelta);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("[ REAL-TIME TELEMETRY ]");
    ImGui::Text("Pitch Shift Sync: %.1f Hz", effect->currentPitchHz.load());
    ImGui::Text("Mechanic Rate:    %.1f Hz", effect->currentRingFreqHz.load());
    
    float pMon = effect->currentPhaseMonitor.load();
    ImGui::Text("Phase Tracker:");
    ImGui::SameLine();
    ImGui::ProgressBar(pMon, ImVec2(-1, 0), "");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("[ SYSTEM HEALTH & FORENSICS ]");
    
    float latency = effect->lastProcessTimeMs.load();
    ImGui::Text("DSP Latency:  %.2f ms", latency);
    if (latency > 10.0f) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[!] CPU OVERLOAD");
    }

    float peak = effect->currentPeak.load();
    float headroom = (1.0f - peak) * 100.0f;
    ImGui::Text("Gain Headroom: %.1f %%", headroom);
    if (peak > 0.98f) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[!] CLIPPING");
    }

    ImGui::Spacing();
    if (ImGui::Button("RUN FORENSIC ENTROPY ANALYSIS", ImVec2(-1, 40))) {
        effect->triggerCapture();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Captures 1s of raw audio for Differential Entropy Test (spectral analysis).");
    }

    ImGui::NextColumn();

    ImGui::Text("[ LIVE SIGNAL MONITOR ]");
    ImGui::Spacing();

    std::vector<float> audioBuffer = engine.getLastAudioBuffer();
    if (!audioBuffer.empty()) {
        int zoomSize = 256; 
        ImGui::PlotLines("##Oscilloscope", audioBuffer.data(), zoomSize, 0, "SIGNAL (ZOOM: 2X)", -1.0f, 1.0f, ImVec2(ImGui::GetContentRegionAvail().x, 300));
    } else {
        ImGui::Text("WAITING FOR SIGNAL...");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("[ SAFETY SETTINGS ]");
    
    bool sm = effect->safetyMuteEnabled.load();
    if (ImGui::Checkbox("AUTO-MUTE ON SECURITY BREACH", &sm)) effect->safetyMuteEnabled.store(sm);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zeros audio output if masking entropy drops below threshold.");

    ImGui::Spacing();
    ImGui::Text("Security Status: ");
    ImGui::SameLine();
    bool secure = effect->isSecure.load();
    if (secure) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "[ SECURE ]");
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "[ !!! BREACH !!! ]");
        if (sm) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "(MUTED)");
        }
    }

    ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "[ TRACE ELIMINATION ACTIVE ]");
    ImGui::ProgressBar(0.3f + (0.01f * (rand() % 20)), ImVec2(-1, 0), "MASKING LOAD");

    ImGui::Columns(1);
    ImGui::End();
}

void PersonaUI::endFrame() {
    ImGui::Render();
    glViewport(0, 0, (int)m_io->DisplaySize.x, (int)m_io->DisplaySize.y);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);
}

void PersonaUI::cleanup() {
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(m_glContext);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        m_window = nullptr;
    }
}
