cmake_minimum_required(VERSION 3.16)
project(CrossLinkFG VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CRITICAL: FORCE x64 BUILD - Prevent 32-bit builds that cause injection failures
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ERROR: You are trying to build 32-bit. This project REQUIRES 64-bit (x64)! Use: cmake -Ax64 ..")
endif()

# STATIC RUNTIME LINKING - DLL works standalone without VS Redistributables
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enable FetchContent
include(FetchContent)

# Fetch MinHook
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.3
)
FetchContent_MakeAvailable(minhook)

# Build MinHook static library
add_library(minhook STATIC
    ${minhook_SOURCE_DIR}/src/buffer.c
    ${minhook_SOURCE_DIR}/src/hook.c
    ${minhook_SOURCE_DIR}/src/trampoline.c
    ${minhook_SOURCE_DIR}/src/HDE/hde32.c
    ${minhook_SOURCE_DIR}/src/HDE/hde64.c
)
target_include_directories(minhook PUBLIC ${minhook_SOURCE_DIR}/include)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.6
)
FetchContent_MakeAvailable(imgui)

# --- FIX: ImGui als STATISCHE Library bauen ---
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PRIVATE
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_IMPL_API=extern\ "C"
)

# Source files - PROXY MODE: Build version.dll as a proxy/wrapper (EXCLUDE IMGUI FOR NOW)
set(SOURCES
    src/dllmain.cpp
    src/proxy.cpp
    src/hooks_clean.cpp  # CLEAN HOOKS WITHOUT IMGUI DEPENDENCY
    # REMOVED: src/imgui_manager.cpp - causes compilation issues
    src/frame_capture.cpp
    src/frame_processor.cpp
    src/opencl_amd_processor.cpp
    src/FrameGenManager.cpp
    src/DX12Renderer.cpp
    src/OpenCLProcessor.cpp
    src/SharedMemoryBridge.cpp
    src/NvidiaOpticalFlow.cpp
)

# Include directories
include_directories(include)
include_directories(stubs)
include_directories(${imgui_SOURCE_DIR})
include_directories(${imgui_SOURCE_DIR}/backends)

# Add shared library (DLL) - PROXY MODE: Build as version.dll
add_library(CrossLinkFG SHARED ${SOURCES})
set_target_properties(CrossLinkFG PROPERTIES OUTPUT_NAME "version")

# Add Simple Overlay DLL - BASIC WINDOW WITH INFO DISPLAY
set(OVERLAY_SOURCES
    src/simple_overlay.cpp
)

add_library(CrossLinkOverlay SHARED ${OVERLAY_SOURCES})
set_target_properties(CrossLinkOverlay PROPERTIES OUTPUT_NAME "overlay")
target_compile_definitions(CrossLinkOverlay PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)

# Link overlay DLL properly
target_link_libraries(CrossLinkOverlay
    imgui
    dxgi.lib
)

# Add executable for testing (optional)
add_executable(Simulator src/main.cpp src/Simulator.cpp src/DX12Renderer.cpp src/OpenCLProcessor.cpp src/SharedMemoryBridge.cpp src/NvidiaOpticalFlow.cpp)

# Compiler definitions for DLL
target_compile_definitions(CrossLinkFG PRIVATE
    CROSSLINKFG_DLL_EXPORT
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

# Compiler definitions for exe
target_compile_definitions(Simulator PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

# Include dirs for both
target_link_libraries(CrossLinkFG
    minhook
)

target_include_directories(CrossLinkFG PRIVATE
    $<BUILD_INTERFACE:${minhook_SOURCE_DIR}/include/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends/>
)

target_include_directories(CrossLinkOverlay PRIVATE
    $<BUILD_INTERFACE:${minhook_SOURCE_DIR}/include/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends/>
)

target_include_directories(Simulator PRIVATE
    $<BUILD_INTERFACE:${minhook_SOURCE_DIR}/include/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends/>
)

# Custom target for building in Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(CrossLinkFG PRIVATE /O2 /fp:fast)
    target_compile_options(Simulator PRIVATE /O2 /fp:fast)
endif()

# Copy any required assets (none yet)
