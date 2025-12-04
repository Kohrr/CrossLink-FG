# CrossLink-FG Developer Guide

**Version:** 1.0.0  
**Last Updated:** December 4, 2025  
**Target Audience:** Software Developers, Graphics Engineers, Researchers

## Table of Contents

1. [Getting Started](#getting-started)
2. [Architecture Overview](#architecture-overview)
3. [Build System](#build-system)
4. [Core Components](#core-components)
5. [API Reference](#api-reference)
6. [Extending CrossLink-FG](#extending-crosslink-fg)
7. [Contributing Guidelines](#contributing-guidelines)
8. [Testing Framework](#testing-framework)
9. [Debugging](#debugging)
10. [Performance Profiling](#performance-profiling)

## Getting Started

### Development Environment Setup

1. **Clone Repository**
   ```bash
   git clone https://github.com/crosslink-fg/crosslink-fg.git
   cd crosslink-fg
   ```

2. **Install Dependencies**
   ```bash
   # Install WiX Toolset for installer building
   # Download from: https://github.com/wixtoolset/wix3/releases
   
   # Install Visual Studio 2022 with Desktop Development workload
   # Required components:
   # - MSVC v143 compiler toolset (x64/x86)
   # - Windows 10/11 SDK (latest version)
   # - CMake tools for Visual Studio
   ```

3. **Build Project**
   ```bash
   # Using Visual Studio
   # Open CrossLink-FG.sln in Visual Studio 2022
   # Select Release configuration
   # Build → Build Solution (Ctrl+Shift+B)
   
   # Using command line
   cmake -B build -S .
   cmake --build build --config Release
   ```

### Project Structure

```
CrossLink-FG/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # Project overview
├── include/                    # Public headers
│   ├── imgui_manager.h         # ImGui overlay system
│   ├── FrameGenManager.h       # Main frame generation manager
│   ├── NvidiaOpticalFlow.h     # NVIDIA Optical Flow API
│   ├── OpenCLProcessor.h       # OpenCL processing interface
│   └── ...
├── src/                        # Implementation files
│   ├── imgui_manager.cpp       # ImGui implementation
│   ├── FrameGenManager.cpp     # Core frame generation logic
│   ├── NvidiaOpticalFlow.cpp   # NVIDIA API implementation
│   ├── OpenCLProcessor.cpp     # OpenCL processor
│   ├── dx12_hooks.cpp          # DirectX 12 hooking
│   └── ...
├── kernels/                    # OpenCL kernels
│   ├── frame_interpolation.cl  # Frame interpolation kernel
│   ├── motion_compensation.cl  # Motion compensation
│   └── temporal_filtering.cl   # Temporal filtering
├── installer/                  # WiX installer files
│   ├── CrossLink-FG.wxs       # Main installer definition
│   ├── Product.wxs            # Product configuration
│   └── build_installer.bat    # Build script
├── docs/                       # Documentation
│   ├── INSTALLATION.md         # Installation guide
│   ├── USER_GUIDE.md           # User documentation
│   ├── DEVELOPER_GUIDE.md      # This file
│   └── ...
└── tests/                      # Unit tests
    ├── test_frame_gen.cpp      # Frame generation tests
    ├── test_opencl.cpp         # OpenCL tests
    └── ...
```

## Architecture Overview

### High-Level Architecture

CrossLink-FG follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────┐
│                 Application Layer            │
├─────────────────────────────────────────────┤
│  Game Proxy Layer  │  ImGui Overlay Layer   │
├─────────────────────────────────────────────┤
│              CrossLink-FG Core              │
│  ┌─────────────┐ ┌─────────────┐ ┌────────┐ │
│  │   Frame     │ │   Nvidia    │ │ OpenCL │ │
│  │   Capture   │ │  Optical    │ │Process │ │
│  │             │ │   Flow      │ │        │ │
│  └─────────────┘ └─────────────┘ └────────┘ │
├─────────────────────────────────────────────┤
│                Hardware Layer               │
│      NVIDIA GPU      │      AMD GPU         │
└─────────────────────────────────────────────┘
```

### Component Interaction

1. **Game Proxy Layer**
   - Intercepts DirectX 12 API calls
   - Forwards calls to original game implementation
   - Captures frame data for processing

2. **Frame Capture**
   - Extracts color and depth information
   - Synchronizes frame data across GPUs
   - Manages frame buffers and timestamps

3. **NVIDIA Optical Flow**
   - Hardware-accelerated flow estimation
   - Motion vector calculation
   - Scene analysis and preprocessing

4. **OpenCL Processing**
   - Frame interpolation using AMD GPU
   - Temporal filtering and smoothing
   - Quality enhancement algorithms

5. **ImGui Overlay**
   - Real-time performance monitoring
   - User interface for configuration
   - Debug information display

## Build System

### CMake Configuration

The project uses CMake for cross-platform build configuration:

```cmake
# CMakeLists.txt (excerpt)
cmake_minimum_required(VERSION 3.16)
project(CrossLink-FG VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCL REQUIRED)
find_package(DirectX REQUIRED)

# Enable static linking for ImGui
set(IMGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(IMGUI_BUILD_IMGUI_DEMO ON CACHE BOOL "" FORCE)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(kernels)
```

### Build Targets

| Target | Description | Dependencies |
|--------|-------------|--------------|
| `CrossLink-FG` | Main application | All core components |
| `version.dll` | Game proxy DLL | CrossLink-FG |
| `imgui` | ImGui static library | CrossLink-FG |
| `tests` | Unit test suite | Google Test, All targets |

### Build Configurations

#### Debug
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

#### Release
```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

#### RelWithDebInfo
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build
cmake --build build
```

### Installer Building

```bash
# Build installer package
cd installer
build_installer.bat

# Output: CrossLink-FG_Setup.msi
```

## Core Components

### FrameGenManager

Central orchestrator for frame generation operations:

```cpp
// include/FrameGenManager.h
class FrameGenManager {
public:
    // Initialization and shutdown
    bool Initialize();
    void Shutdown();
    
    // Configuration
    void SetQuality(FrameGenQuality quality);
    void SetTargetFPS(int fps);
    void EnableNvidiaOpticalFlow(bool enable);
    
    // Processing
    bool ProcessFrame(const FrameData& frame);
    FrameData* GetGeneratedFrame();
    
    // Statistics
    FrameGenStats GetStatistics() const;
    
private:
    std::unique_ptr<NvidiaOpticalFlow> m_opticalFlow;
    std::unique_ptr<OpenCLProcessor> m_openclProcessor;
    std::unique_ptr<FrameCapture> m_frameCapture;
};
```

### NvidiaOpticalFlow

Hardware-accelerated optical flow using NVIDIA SDK:

```cpp
// include/NvidiaOpticalFlow.h
class NvidiaOpticalFlow {
public:
    bool Initialize(uint32_t width, uint32_t height);
    bool AnalyzeFrames(const FrameData& prev, const FrameData& curr);
    MotionVectors GetMotionVectors() const;
    
    // Hardware capabilities
    bool SupportsHardwareFlow() const;
    uint32_t GetMaxResolution() const;
    
private:
    NV_OF_STATUS InitializeHardware();
    void ProcessFlowVectorField();
    
    NV_OF_HANDLE m_handle;
    MotionVectors m_motionVectors;
};
```

### OpenCLProcessor

Cross-GPU frame generation using OpenCL:

```cpp
// include/OpenCLProcessor.h
class OpenCLProcessor {
public:
    bool Initialize(uint32_t deviceId);
    bool CompileKernels(const std::string& kernelDir);
    bool GenerateFrame(const FrameData& input, 
                      const MotionVectors& motion,
                      FrameData& output);
    
    // Memory management
    bool AllocateBuffers(size_t width, size_t height);
    void FreeBuffers();
    
private:
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    std::vector<cl::Kernel> m_kernels;
    
    cl::Buffer m_inputBuffer;
    cl::Buffer m_motionBuffer;
    cl::Buffer m_outputBuffer;
};
```

### DirectX 12 Hooking

System for intercepting DirectX 12 API calls:

```cpp
// src/dx12_hooks.cpp
class DX12HookManager {
public:
    bool InstallHooks();
    void RemoveHooks();
    
    // Hooked interfaces
    static STDMETHODCALLTYPE ID3D12CommandQueue_ExecuteCommandLists_Hook(
        ID3D12CommandQueue* pThis, 
        UINT NumCommandLists, 
        ID3D12CommandList* const* ppCommandLists);
    
private:
    static bool m_hooksInstalled;
    static std::vector<uint8_t> m_originalBytes;
};
```

## API Reference

### Public API Functions

```cpp
// Main API entry point
extern "C" {
    // Initialize CrossLink-FG
    CROSSLINK_FG_API bool CrossLinkFG_Initialize(
        const CrossLinkFGConfig* config);
    
    // Shutdown and cleanup
    CROSSLINK_FG_API void CrossLinkFG_Shutdown();
    
    // Frame processing
    CROSSLINK_FG_API bool CrossLinkFG_ProcessFrame(
        const FrameData* input, 
        FrameData* output);
    
    // Configuration
    CROSSLINK_FG_API void CrossLinkFG_SetQuality(
        FrameGenQuality quality);
    
    // Statistics
    CROSSLINK_FG_API CrossLinkFGStats CrossLinkFG_GetStatistics();
}
```

### Configuration Structure

```cpp
// Configuration passed to Initialize()
struct CrossLinkFGConfig {
    uint32_t version;           // Must be CROSSLINK_FG_VERSION
    uint32_t width;             // Frame width
    uint32_t height;            // Frame height
    uint32_t targetFPS;         // Desired frame rate
    FrameGenQuality quality;    // Quality setting
    bool enableNvidiaFlow;      // Use hardware optical flow
    uint32_t nvidiaDeviceId;    // NVIDIA GPU device ID
    uint32_t amdDeviceId;       // AMD GPU device ID
    uint32_t memoryPoolSize;    // Memory pool size in MB
    const char* logPath;        // Optional log file path
};
```

### Frame Data Structure

```cpp
// Input/output frame data
struct FrameData {
    uint8_t* colorData;         // RGBA color data
    uint8_t* depthData;         // Depth information
    uint32_t width;             // Frame width
    uint32_t height;            // Frame height
    uint32_t stride;            // Bytes per row
    float timestamp;            // Presentation timestamp
    uint32_t frameId;           // Unique frame identifier
    FrameFormat format;         // Pixel format
};
```

## Extending CrossLink-FG

### Adding New Games

1. **Create Game Handler**
   ```cpp
   // src/game_handlers/borderlands3_handler.cpp
   class Borderlands3Handler : public GameHandler {
   public:
       bool IsCompatible(const std::string& exeName) override;
       bool Initialize() override;
       void ProcessFrame(const FrameData& frame) override;
   };
   ```

2. **Register Handler**
   ```cpp
   // src/game_manager.cpp
   void GameManager::RegisterHandlers() {
       RegisterHandler(std::make_unique<Borderlands3Handler>());
       RegisterHandler(std::make_unique<Borderlands4Handler>());
   }
   ```

3. **Add Proxy Configuration**
   ```xml
   <!-- installer/games.xml -->
   <Game Id="Borderlands3" Name="Borderlands 3">
       <Proxy Executable="Borderlands3.exe" />
       <Config Directory="Steam/steamapps/common/Borderlands 3" />
   </Game>
   ```

### Custom Frame Generation Algorithms

1. **Implement Kernel**
   ```c
   // kernels/custom_interpolation.cl
   __kernel void customFrameGeneration(
       __global const uchar4* prevFrame,
       __global const uchar4* currFrame,
       __global const float2* motionVectors,
       __global uchar4* outputFrame,
       const uint width,
       const uint height) {
       
       uint x = get_global_id(0);
       uint y = get_global_id(1);
       
       // Custom interpolation logic
       // ...
   }
   ```

2. **Create Processor**
   ```cpp
   // src/custom_processor.cpp
   class CustomProcessor : public FrameProcessor {
   public:
       bool Initialize() override;
       bool ProcessFrame(const FrameData& input, 
                        FrameData& output) override;
   };
   ```

3. **Register Processor**
   ```cpp
   // src/processor_manager.cpp
   void ProcessorManager::RegisterProcessors() {
       RegisterProcessor("custom", std::make_unique<CustomProcessor>());
   }
   ```

### Adding New Quality Presets

1. **Define Preset**
   ```cpp
   // src/quality_presets.cpp
   static const QualityPreset g_customPreset = {
       .name = "My Custom Preset",
       .interpolationStrength = 0.75f,
       .temporalFiltering = 0.5f,
       .motionSmoothing = 0.3f,
       .qualitySettings = QualitySettings{
           .sharpening = 1.2f,
           .contrast = 1.0f,
           .saturation = 1.0f
       }
   };
   ```

2. **Register Preset**
   ```cpp
   // During initialization
   QualityManager::RegisterPreset(g_customPreset);
   ```

## Contributing Guidelines

### Code Style

- **C++ Standard**: Use C++20 features where appropriate
- **Naming Conventions**:
  - Classes: PascalCase (`FrameGenManager`)
  - Functions: camelCase (`processFrame`)
  - Variables: snake_case (`frame_data`)
  - Constants: UPPER_SNAKE_CASE (`MAX_FRAME_RATE`)
- **Formatting**: Use clang-format with provided configuration
- **Documentation**: Doxygen-style comments for public APIs

### Code Examples

```cpp
/**
 * @brief Processes a single frame for interpolation
 * @param input Current frame data
 * @param output Generated intermediate frame
 * @param motionVectors Motion estimation data
 * @return true if successful, false otherwise
 * @throws std::invalid_argument if input is nullptr
 */
bool FrameGenerator::ProcessFrame(
    const FrameData& input,
    FrameData& output,
    const MotionVectors& motionVectors) {
    
    // Input validation
    if (!input.colorData || !output.colorData) {
        throw std::invalid_argument("Invalid frame data");
    }
    
    // Processing logic
    return m_openclProcessor->GenerateFrame(input, motionVectors, output);
}
```

### Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

#### PR Checklist

- [ ] Code follows project style guidelines
- [ ] Self-review of code completed
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] All tests pass locally
- [ ] No memory leaks detected
- [ ] Performance impact analyzed

### Issue Reporting

When reporting bugs, include:

1. **System Information**:
   - Operating system version
   - Graphics card models
   - Driver versions
   - CrossLink-FG version

2. **Reproduction Steps**:
   - Exact steps to reproduce
   - Expected behavior
   - Actual behavior
   - Game version and settings

3. **Log Files**:
   - Runtime log output
   - Error messages
   - System event logs

## Testing Framework

### Unit Tests

Located in the `tests/` directory using Google Test:

```cpp
// tests/test_frame_gen.cpp
class FrameGenTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_config = CreateTestConfig();
        CrossLinkFG_Initialize(&m_config);
    }
    
    void TearDown() override {
        CrossLinkFG_Shutdown();
    }
    
    CrossLinkFGConfig m_config;
};

TEST_F(FrameGenTest, BasicFrameProcessing) {
    FrameData input = CreateTestFrame();
    FrameData output = CreateEmptyFrame();
    
    bool result = CrossLinkFG_ProcessFrame(&input, &output);
    
    EXPECT_TRUE(result);
    EXPECT_NE(output.colorData, nullptr);
}
```

### Integration Tests

```cpp
// tests/integration/test_game_integration.cpp
class GameIntegrationTest : public ::testing::Test {
public:
    void SetUpGame(const std::string& gameName) {
        m_gameHandler = GameManager::CreateHandler(gameName);
        EXPECT_TRUE(m_gameHandler->Initialize());
    }
    
protected:
    std::unique_ptr<GameHandler> m_gameHandler;
};

TEST_F(GameIntegrationTest, Borderlands3FrameCapture) {
    SetUpGame("Borderlands3");
    
    // Simulate game frame rendering
    FrameData testFrame = CreateTestFrame();
    m_gameHandler->ProcessFrame(testFrame);
    
    // Verify frame was captured correctly
    EXPECT_EQ(testFrame.width, 1920);
    EXPECT_EQ(testFrame.height, 1080);
}
```

### Performance Tests

```cpp
// tests/performance/test_benchmarks.cpp
class PerformanceTest : public ::testing::Test {
protected:
    void SetUpPerformanceTest() {
        m_timer = std::make_unique<PerformanceTimer>();
        m_timer->Start();
    }
    
    void MeasureFrameTime() {
        m_frameTimes.push_back(m_timer->ElapsedMilliseconds());
    }
    
    double GetAverageFrameTime() const {
        return std::accumulate(m_frameTimes.begin(), 
                              m_frameTimes.end(), 0.0) / m_frameTimes.size();
    }
    
    std::vector<double> m_frameTimes;
    std::unique_ptr<PerformanceTimer> m_timer;
};

TEST_F(PerformanceTest, FrameGenerationPerformance) {
    SetUpPerformanceTest();
    
    const int NUM_FRAMES = 1000;
    for (int i = 0; i < NUM_FRAMES; ++i) {
        FrameData frame = CreateTestFrame();
        CrossLinkFG_ProcessFrame(&frame, &frame);
        MeasureFrameTime();
    }
    
    double avgFrameTime = GetAverageFrameTime();
    double targetFPS = 144.0;
    double maxFrameTime = 1000.0 / targetFPS; // ~6.94ms
    
    EXPECT_LT(avgFrameTime, maxFrameTime);
}
```

### Running Tests

```bash
# Build tests
cmake -DBUILD_TESTS=ON -B build
cmake --build build

# Run all tests
cd build
ctest --output-on-failure

# Run specific test suite
ctest -R FrameGen
ctest -R OpenCL
ctest -R Integration
```

## Debugging

### Debug Configuration

Build with debug symbols and logging:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_VERBOSE_LOGGING=ON -B build
cmake --build build
```

### Logging System

```cpp
// src/logging.cpp
enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

void CrossLinkFG_Log(LogLevel level, const char* format, ...);

// Usage examples
CrossLinkFG_Log(LOG_INFO, "Initializing Frame Generation");
CrossLinkFG_Log(LOG_WARNING, "GPU memory low: %u MB", memoryMB);
CrossLinkFG_Log(LOG_ERROR, "Failed to initialize OpenCL context");
```

### Debug Visualization

Use the built-in debugging tools:

```cpp
// Debug overlay toggles
CrossLinkFG_DebugOverlay(true);     // Show debug overlay
CrossLinkFG_DebugWireframe(false);  // Disable wireframe mode
CrossLinkFG_DebugMotionVectors(true); // Show motion vectors
```

### Common Debugging Techniques

1. **Frame Inspector**:
   - Visualize captured frames
   - Analyze motion vectors
   - Check pixel quality

2. **Performance Profiler**:
   - Identify bottlenecks
   - Measure kernel execution times
   - Monitor memory usage

3. **Memory Debugging**:
   - Detect leaks using AddressSanitizer
   - Check for buffer overruns
   - Validate memory alignments

### Debug Commands

| Command | Description |
|---------|-------------|
| `debug overlay` | Toggle debug overlay |
| `debug motion` | Show motion vector field |
| `debug timing` | Display frame timing info |
| `debug memory` | Show memory usage |
| `debug logs` | Open log file |

## Performance Profiling

### Built-in Profiler

```cpp
// Performance monitoring
struct PerformanceMetrics {
    double frameGenerationTime;    // ms per frame
    double opticalFlowTime;        // NVIDIA hardware time
    double openclProcessingTime;   // AMD processing time
    double memoryTransferTime;     // GPU->GPU transfer
    double totalFrameTime;         // End-to-end time
};

PerformanceMetrics metrics = CrossLinkFG_GetMetrics();
printf("Frame Gen: %.2f ms\n", metrics.frameGenerationTime);
```

### External Profiling Tools

#### NVIDIA Nsight
```bash
# Start Nsight Graphics
nsight_graphics

# Profile CrossLink-FG
# 1. Launch Nsight Graphics
# 2. Connect to CrossLink-FG process
# 3. Start capture
# 4. Run frame generation workload
# 5. Analyze performance data
```

#### AMD Radeon Profiler
```bash
# Launch Radeon Profiler
radeon_profiler

# Profile OpenCL kernels
# 1. Select CrossLink-FG process
# 2. Start kernel analysis
# 3. Monitor GPU utilization
# 4. Analyze memory bandwidth
```

#### Intel VTune
```bash
# Profile CPU performance
vtune -collect hotspots ./CrossLink-FG.exe

# Analyze results
vtune -report summary
```

### Optimization Guidelines

1. **Kernel Optimization**:
   - Minimize memory transactions
   - Use local memory effectively
   - Optimize thread utilization

2. **Pipeline Optimization**:
   - Stream processing over batch
   - Minimize synchronization points
   - Use asynchronous operations

3. **Memory Optimization**:
   - Optimize buffer allocations
   - Use pinned memory for transfers
   - Minimize GPU memory fragmentation

---

For more technical details, see the [API Reference](API_REFERENCE.md) and [Troubleshooting Guide](TROUBLESHOOTING.md).
