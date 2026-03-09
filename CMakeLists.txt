cmake_minimum_required(VERSION 3.15)

project(PersonaVox CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Derlenecek tüm C++ dosyalarımızı buraya ekliyoruz.
add_executable(PersonaVox 
    src/main.cpp
    src/AudioEngine.cpp
    src/VoiceEffect.cpp
    
    # ImGui Core (Çekirdek Dosyalar)
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    
    # ImGui SDL2 ve OpenGL Entegrasyon Dosyaları
    vendor/imgui/backends/imgui_impl_sdl2.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

# Projemizin tüm "başlık" dosyalarını (*.hpp) arayacağı klasörü tanımlıyoruz.
# BU KOMUT add_executable'dan SONRA GELMELİDİR!
find_package(PkgConfig REQUIRED)
pkg_check_modules(SOUNDTOUCH REQUIRED soundtouch)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(GLEW REQUIRED glew)

target_include_directories(PersonaVox PRIVATE include)
target_include_directories(PersonaVox PRIVATE vendor/imgui)
target_include_directories(PersonaVox PRIVATE vendor/imgui/backends)
target_include_directories(PersonaVox PRIVATE ${SOUNDTOUCH_INCLUDE_DIRS})
target_include_directories(PersonaVox PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(PersonaVox PRIVATE ${GLEW_INCLUDE_DIRS})

# PortAudio, SoundTouch, SDL2 ve OpenGL (GLEW) kütüphanelerini projeye bağlıyoruz
target_link_libraries(PersonaVox PRIVATE portaudio)
target_link_libraries(PersonaVox PRIVATE ${SOUNDTOUCH_LIBRARIES})
target_link_libraries(PersonaVox PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(PersonaVox PRIVATE ${GLEW_LIBRARIES} GL)
