cmake_minimum_required(VERSION 3.16)
project(assistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QTAPPFW_HVAC REQUIRED qtappfw-hvac)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)
pkg_check_modules(QTAPPFW_APPLAUNCHER REQUIRED qtappfw-applauncher)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Concurrent Multimedia QuickControls2 ShaderTools DBus Network OpenGL)

set(LLAMA_CPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../llama.cpp")
set(LLAMA_INCLUDE_HINTS
    ${CMAKE_SYSROOT}/usr/include/llama
    ${CMAKE_SYSROOT}/usr/include
    ${LLAMA_CPP_SOURCE_DIR}/include
)
set(LLAMA_LIBRARY_HINTS
    ${CMAKE_SYSROOT}/usr/lib/llama
    ${CMAKE_SYSROOT}/usr/lib
    ${CMAKE_SYSROOT}/lib
)

find_path(LLAMA_INCLUDE_DIR
    NAMES llama.h
    HINTS ${LLAMA_INCLUDE_HINTS}
    NO_CMAKE_FIND_ROOT_PATH
)

find_library(LLAMA_LIBRARY
    NAMES llama
    HINTS ${LLAMA_LIBRARY_HINTS}
)

find_library(GGML_LIBRARY
    NAMES ggml
    HINTS ${LLAMA_LIBRARY_HINTS}
)

find_library(GGML_BASE_LIBRARY
    NAMES ggml-base
    HINTS ${LLAMA_LIBRARY_HINTS}
)

find_library(GGML_CPU_LIBRARY
    NAMES ggml-cpu
    HINTS ${LLAMA_LIBRARY_HINTS}
)

find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATHS ${CMAKE_SYSROOT}/usr/include
)

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS ${CMAKE_SYSROOT}/usr/lib
)

foreach(REQUIRED_DEP
        LLAMA_INCLUDE_DIR
        LLAMA_LIBRARY
        GGML_LIBRARY
        GGML_BASE_LIBRARY
        GGML_CPU_LIBRARY
        ONNXRUNTIME_INCLUDE_DIR
        ONNXRUNTIME_LIBRARY)
    if (NOT DEFINED ${REQUIRED_DEP} OR
        "${${REQUIRED_DEP}}" STREQUAL "" OR
        "${${REQUIRED_DEP}}" MATCHES "-NOTFOUND$")
        message(FATAL_ERROR "Missing required dependency: ${REQUIRED_DEP}")
    endif()
endforeach()

add_executable(assistant
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context/RuntimeContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context/RuntimeContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings/AssistantUiSettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings/AssistantUiSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vad/silero_vad_engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vad/silero_vad_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioInputService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioInputService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vad/CommandSegmenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vad/CommandSegmenter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asr/sherpaonnxengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asr/sherpaonnxengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/AssistantTtsSettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/AssistantTtsSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/PiperTTS.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/PiperTTS.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsWorker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsWorker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tts/VieNeuTtsManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wakeword/WakeWordTester.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wakeword/WakeWordTester.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asr/asrmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asr/asrmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/NluTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertMultiHeadClassifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertMultiHeadClassifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertSlotExtractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertSlotExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertExecutionPolicy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlu/PhoBertExecutionPolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llm/LlmQueryService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llm/LlmQueryService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/theme/ThemeSettingsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/theme/ThemeSettingsManager.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/HvacService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/HvacService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/MediaService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/MediaService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/NavigationService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/NavigationService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/PhoneService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/PhoneService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/VehicleService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/VehicleService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/phoneclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/phoneclient.cpp

)

qt6_add_shaders(assistant "assistant_shaders"
    PREFIX "/"
    FILES
        assistant.frag
)

target_include_directories(assistant PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/theme
    ${QTAPPFW_HVAC_INCLUDE_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
    ${QTAPPFW_APPLAUNCHER_INCLUDE_DIRS}
    ${LLAMA_INCLUDE_DIR}
    ${ONNXRUNTIME_INCLUDE_DIR}
)

target_compile_options(assistant PRIVATE
    ${QTAPPFW_HVAC_CFLAGS_OTHER}
    ${QTAPPFW_VEHICLE_SIGNALS_CFLAGS_OTHER}
    ${QTAPPFW_APPLAUNCHER_CFLAGS_OTHER}
)

target_link_directories(assistant PRIVATE
    ${QTAPPFW_HVAC_LIBRARY_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARY_DIRS}
    ${QTAPPFW_APPLAUNCHER_LIBRARY_DIRS}
)

find_library(SHERPA_ONNX_C_API_LIB sherpa-onnx-c-api REQUIRED)

target_link_libraries(assistant PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Concurrent
    Qt6::Network
    Qt6::OpenGL
    Qt6::DBus

    #${CMAKE_SYSROOT}/usr/lib/whisper/libwhisper.so

    ${LLAMA_LIBRARY}
    ${GGML_LIBRARY}
    ${GGML_BASE_LIBRARY}
    ${GGML_CPU_LIBRARY}

    wakeword
    ${ONNXRUNTIME_LIBRARY}
    asound
    rnnoise
    ${SHERPA_ONNX_C_API_LIB}

    ${QTAPPFW_HVAC_LIBRARIES}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
    ${QTAPPFW_APPLAUNCHER_LIBRARIES}
)

set_target_properties(assistant PROPERTIES
    BUILD_RPATH "/usr/lib/llama"
    INSTALL_RPATH "/usr/lib/llama"
)

install(TARGETS assistant
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
