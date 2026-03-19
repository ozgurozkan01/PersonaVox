# PersonaVox Development Diary

## Day 1: Architectural Foundation & Build System Initialization
**Date:** 2026-03-19

### 1. Objective
Our primary objective for Day 1 was to establish a robust, deterministic, and future-proof build environment for the PersonaVox Anti-Forensic DSP Engine. Before delving into complex signal processing mathematics (such as Formant Shifting or Phase Scrambling), it was imperative to construct a scalable C++ project infrastructure capable of accommodating our stringent real-time audio constraints and forthcoming cryptographic requirements.

### 2. The Departure from Projucer
We deliberately eschewed JUCE's native Projucer application in favor of modern **CMake**. While Projucer provides a highly visual and rapid setup for conventional audio plugins, it imposes severe limitations on a military-grade architectural scope like PersonaVox. Our rationale for enforcing CMake encompasses the following strategic advantages:
- **Cryptographic Integration (Phase 4):** PersonaVox will eventually mandate the static linking of robust cryptographic libraries (e.g., OpenSSL or libsodium) to facilitate AES-256-GCM End-to-End Encryption (E2EE) and Point-to-Point (P2P) networking. Managing complex external dependency trees in Projucer is notoriously cumbersome. Conversely, CMake accomplishes this seamlessly via `find_package()` and target linking directives.
- **Reproducible Builds & CI/CD:** In the realm of adversarial machine learning and anti-forensics, code compilation must be strictly deterministic. CMake ensures that our compilation pipeline remains highly reproducible across disparate Linux environments and is fundamentally compatible with automated Continuous Integration/Continuous Deployment (CI/CD) pipelines.

### 3. CMake Configuration Breakdown
We engineered a sophisticated `CMakeLists.txt` file tailored specifically for our engine:
- **`FetchContent` Paradigm:** Instead of polluting the host operating system with global JUCE installations, we implemented automated Git fetching for the JUCE framework (`v8.0.0`). This guarantees that any developer cloning the repository automatically obtains the precise framework version, securing a decoupled and globally consistent build environment.
- **`juce_add_plugin` Definition:** We targeted the output as simultaneously a "Standalone" application and a "VST3" plugin. Crucially, we initialized a **headless** DSP engine (explicitly omitting `PluginEditor.h/cpp`). By stripping away the Graphical User Interface (GUI), we eliminate rendering overhead, thereby dedicating 100% of the CPU's processing bandwidth to the real-time audio thread (`processBlock`).
- **Target Sources and the Eradication of `main.cpp`:** We deleted the conventional C++ `main.cpp` artifact. Within the JUCE ecosystem, operating-system-level constraints (like audio driver instantiation, ALSA wrappers, and window management) are abstracted away by JUCE's internal macros. Consequently, our entry point for signal manipulation was re-routed directly to `PluginProcessor.h` and `PluginProcessor.cpp`. This purges unnecessary boilerplate and grants us unadulterated, low-latency access to the audio hardware callbacks.

### 4. Conclusion & Next Steps
With the CMake configuration successfully generating the Makefile hierarchy and compiling without fatal linker errors, the infrastructural bedrock is now fully operational. 

Moving forward into **Day 2**, we will commence Phase 1 (Data Acquisition). This will entail the rigorous implementation of lock-free `CircularBuffer` mechanisms and `juce::AudioFormatWriter::ThreadedWriter` instances, enabling us to concurrently capture and stream both the pristine and obfuscated audio data directly to the disk without provoking latency spikes or dropouts on the high-priority audio thread.
