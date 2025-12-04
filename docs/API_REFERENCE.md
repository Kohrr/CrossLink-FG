# CrossLink-FG API Reference

**Version:** 1.0.0  
**Last Updated:** December 4, 2025  
**Language:** C++17/C++20

## Table of Contents

1. [Core API](#core-api)
2. [Frame Generation API](#frame-generation-api)
3. [Configuration API](#configuration-api)
4. [Statistics API](#statistics-api)
5. [Debug API](#debug-api)
6. [Game Integration API](#game-integration-api)
7. [Data Structures](#data-structures)
8. [Error Codes](#error-codes)

## Core API

### CrossLinkFG_Initialize

Initializes the CrossLink-FG framework.

```cpp
extern "C" {
    /**
     * @brief Initialize CrossLink-FG framework
     * @param config Configuration structure
     * @return true if initialization successful, false otherwise
     * @throws None
     */
    CROSSLINK_FG_API bool CrossLinkFG_Initialize(
        const CrossLinkFGConfig* config
    );
}
```

**Parameters:**
- `config` - Pointer to configuration structure

**Returns:** `bool` - True if successful, false otherwise

**Example:**
```cpp
CrossLinkFGConfig config = {
    .version = CROSSLINK_FG_VERSION,
    .width = 1920,
    .height = 1080,
    .targetFPS = 60,
    .quality = FRAME_GEN_QUALITY_HIGH,
    .enableNvidiaFlow = true,
    .nvidiaDeviceId = 0,
    .amdDeviceId = 1,
    .memoryPoolSize = 2048
};

if (!CrossLinkFG_Initialize(&config)) {
    fprintf(stderr, "Failed to initialize CrossLink-FG\n");
    return false;
}
```

### CrossLinkFG_Shutdown

Shuts down and cleans up CrossLink-FG resources.

```cpp
/**
 * @brief Shutdown CrossLink-FG framework
 * @return void
 * @throws None
 */
CROSSLINK_FG_API void CrossLinkFG_Shutdown();
```

**Example:**
```cpp
// Clean shutdown
CrossLinkFG_Shutdown();
```

### CrossLinkFG_IsInitialized

Checks if CrossLink-FG is properly initialized.

```cpp
/**
 * @brief Check if CrossLink-FG is initialized
 * @return true if initialized, false otherwise
 */
CROSSLINK_FG_API bool CrossLinkFG_IsInitialized();
```

## Frame Generation API

### CrossLinkFG_ProcessFrame

Processes a single frame for interpolation.

```cpp
/**
 * @brief Process frame for interpolation
 * @param input Input frame data
 * @param output Generated intermediate frame
 * @return true if processing successful, false otherwise
 * @throws std::invalid_argument if input/output is nullptr
 */
CROSSLINK_FG_API bool CrossLinkFG_ProcessFrame(
    const FrameData* input,
    FrameData* output
);
```

**Parameters:**
- `input` - Pointer to input frame data
- `output` - Pointer to output frame data (will be filled)

**Returns:** `bool` - True if successful, false otherwise

**Example:**
```cpp
FrameData input = CreateTestFrame();
FrameData output = {0};

if (CrossLinkFG_ProcessFrame(&input, &output)) {
    printf("Frame generated successfully\n");
    // Use output.colorData for the interpolated frame
    free(output.colorData);
}
```

### CrossLinkFG_SetQuality

Sets frame generation quality level.

```cpp
/**
 * @brief Set frame generation quality
 * @param quality Quality level
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetQuality(
    FrameGenQuality quality
);
```

**Example:**
```cpp
// Set ultra quality
CrossLinkFG_SetQuality(FRAME_GEN_QUALITY_ULTRA);

// Set high quality
CrossLinkFG_SetQuality(FRAME_GEN_QUALITY_HIGH);

// Set medium quality
CrossLinkFG_SetQuality(FRAME_GEN_QUALITY_MEDIUM);

// Set low quality
CrossLinkFG_SetQuality(FRAME_GEN_QUALITY_LOW);
```

### CrossLinkFG_GetQuality

Gets current frame generation quality level.

```cpp
/**
 * @brief Get current frame generation quality
 * @return Current quality level
 */
CROSSLINK_FG_API FrameGenQuality CrossLinkFG_GetQuality();
```

## Configuration API

### CrossLinkFG_SetTargetFPS

Sets target frame rate.

```cpp
/**
 * @brief Set target frame rate
 * @param fps Target FPS (30-240)
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetTargetFPS(uint32_t fps);
```

**Parameters:**
- `fps` - Target frame rate (30-240 FPS)

**Example:**
```cpp
// Target 60 FPS
CrossLinkFG_SetTargetFPS(60);

// Target 144 FPS for competitive gaming
CrossLinkFG_SetTargetFPS(144);

// Target 30 FPS for low-end systems
CrossLinkFG_SetTargetFPS(30);
```

### CrossLinkFG_EnableNvidiaOpticalFlow

Enables or disables NVIDIA Optical Flow hardware acceleration.

```cpp
/**
 * @brief Enable/disable NVIDIA Optical Flow
 * @param enable True to enable, false to disable
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_EnableNvidiaOpticalFlow(bool enable);
```

**Example:**
```cpp
// Enable hardware optical flow
CrossLinkFG_EnableNvidiaOpticalFlow(true);

// Disable hardware optical flow (fallback to software)
CrossLinkFG_EnableNvidiaOpticalFlow(false);
```

### CrossLinkFG_SetDeviceIDs

Sets GPU device IDs for processing.

```cpp
/**
 * @brief Set GPU device IDs
 * @param nvidiaDeviceId NVIDIA GPU device ID
 * @param amdDeviceId AMD GPU device ID
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetDeviceIDs(
    uint32_t nvidiaDeviceId,
    uint32_t amdDeviceId
);
```

**Example:**
```cpp
// Use first NVIDIA and first AMD GPU
CrossLinkFG_SetDeviceIDs(0, 0);

// Use second NVIDIA and first AMD GPU
CrossLinkFG_SetDeviceIDs(1, 0);
```

### CrossLinkFG_SetMemoryPoolSize

Sets GPU memory pool size.

```cpp
/**
 * @brief Set GPU memory pool size
 * @param sizeMB Memory pool size in megabytes
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetMemoryPoolSize(uint32_t sizeMB);
```

**Parameters:**
- `sizeMB` - Memory pool size in MB (512-8192)

**Example:**
```cpp
// Set 2GB memory pool
CrossLinkFG_SetMemoryPoolSize(2048);

// Set 1GB memory pool
CrossLinkFG_SetMemoryPoolSize(1024);
```

## Statistics API

### CrossLinkFG_GetStatistics

Gets current performance statistics.

```cpp
/**
 * @brief Get performance statistics
 * @return Statistics structure
 */
CROSSLINK_FG_API CrossLinkFGStats CrossLinkFG_GetStatistics();
```

**Returns:** `CrossLinkFGStats` - Current statistics

**Example:**
```cpp
CrossLinkFGStats stats = CrossLinkFG_GetStatistics();

printf("FPS: %.2f\n", stats.currentFPS);
printf("Generated Frames: %u\n", stats.totalGeneratedFrames);
printf("Average Frame Time: %.2f ms\n", stats.averageFrameTime);
printf("GPU Utilization: NVIDIA %.1f%%, AMD %.1f%%\n", 
       stats.nvidiaUtilization * 100.0f,
       stats.amdUtilization * 100.0f);
```

### CrossLinkFG_ResetStatistics

Resets statistics counters.

```cpp
/**
 * @brief Reset performance statistics
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_ResetStatistics();
```

**Example:**
```cpp
// Reset statistics before new benchmark
CrossLinkFG_ResetStatistics();

// Run workload...
// Get statistics
```

## Debug API

### CrossLinkFG_SetLogLevel

Sets logging level.

```cpp
/**
 * @brief Set logging level
 * @param level Log level
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetLogLevel(LogLevel level);
```

**Example:**
```cpp
// Enable debug logging
CrossLinkFG_SetLogLevel(LOG_LEVEL_DEBUG);

// Set to info level
CrossLinkFG_SetLogLevel(LOG_LEVEL_INFO);

// Set to warnings only
CrossLinkFG_SetLogLevel(LOG_LEVEL_WARNING);

// Errors only
CrossLinkFG_SetLogLevel(LOG_LEVEL_ERROR);
```

### CrossLinkFG_EnableDebugOverlay

Enables or disables debug overlay.

```cpp
/**
 * @brief Enable/disable debug overlay
 * @param enable True to enable, false to disable
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_EnableDebugOverlay(bool enable);
```

**Example:**
```cpp
// Enable debug overlay
CrossLinkFG_EnableDebugOverlay(true);

// Disable debug overlay
CrossLinkFG_EnableDebugOverlay(false);
```

### CrossLinkFG_DebugCommand

Execute debug command.

```cpp
/**
 * @brief Execute debug command
 * @param command Command string
 * @return Command result string
 * @throws std::invalid_argument if command is nullptr or empty
 */
CROSSLINK_FG_API const char* CrossLinkFG_DebugCommand(
    const char* command
);
```

**Parameters:**
- `command` - Debug command string

**Returns:** `const char*` - Result string

**Example:**
```cpp
// Get system info
const char* result = CrossLinkFG_DebugCommand("info system");
printf("System info: %s\n", result);

// Show memory usage
result = CrossLinkFG_DebugCommand("debug memory");
printf("Memory usage: %s\n", result);

// Toggle motion vectors
result = CrossLinkFG_DebugCommand("debug motion on");
```

**Available Commands:**
- `info system` - System information
- `info gpus` - GPU information
- `info memory` - Memory usage
- `debug overlay` - Toggle overlay
- `debug motion` - Show motion vectors
- `debug timing` - Show timing info
- `profile start` - Start profiling
- `profile stop` - Stop profiling
- `logs open` - Open log file

## Game Integration API

### CrossLinkFG_RegisterGameHandler

Registers custom game handler.

```cpp
/**
 * @brief Register custom game handler
 * @param gameName Game name
 * @param handler Pointer to game handler
 * @return true if registration successful, false otherwise
 */
CROSSLINK_FG_API bool CrossLinkFG_RegisterGameHandler(
    const char* gameName,
    GameHandler* handler
);
```

**Parameters:**
- `gameName` - Game identifier string
- `handler` - Pointer to game handler implementation

**Example:**
```cpp
class CustomGameHandler : public GameHandler {
public:
    bool IsCompatible(const char* exeName) override {
        return strstr(exeName, "MyGame.exe") != nullptr;
    }
    
    bool Initialize() override {
        // Custom initialization
        return true;
    }
    
    void ProcessFrame(const FrameData& frame) override {
        // Custom frame processing
    }
};

CustomGameHandler handler;
CrossLinkFG_RegisterGameHandler("MyGame", &handler);
```

### CrossLinkFG_UnregisterGameHandler

Unregisters game handler.

```cpp
/**
 * @brief Unregister game handler
 * @param gameName Game name
 * @return true if unregistration successful, false otherwise
 */
CROSSLINK_FG_API bool CrossLinkFG_UnregisterGameHandler(
    const char* gameName
);
```

## Data Structures

### CrossLinkFGConfig

Configuration structure for initialization.

```cpp
typedef struct CrossLinkFGConfig {
    uint32_t version;              /**< Version (must be CROSSLINK_FG_VERSION) */
    uint32_t width;                /**< Frame width */
    uint32_t height;               /**< Frame height */
    uint32_t targetFPS;            /**< Target frame rate */
    FrameGenQuality quality;       /**< Quality level */
    bool enableNvidiaFlow;         /**< Enable NVIDIA Optical Flow */
    uint32_t nvidiaDeviceId;       /**< NVIDIA GPU device ID */
    uint32_t amdDeviceId;          /**< AMD GPU device ID */
    uint32_t memoryPoolSize;       /**< Memory pool size in MB */
    const char* logPath;           /**< Optional log file path */
    const char* configPath;        /**< Optional config file path */
    void* userData;                /**< User data pointer */
} CrossLinkFGConfig;
```

**Fields:**
- `version` - Must be set to `CROSSLINK_FG_VERSION`
- `width` - Frame width in pixels
- `height` - Frame height in pixels  
- `targetFPS` - Target frame rate (30-240)
- `quality` - Frame generation quality
- `enableNvidiaFlow` - Enable hardware optical flow
- `nvidiaDeviceId` - NVIDIA GPU device ID (0-based)
- `amdDeviceId` - AMD GPU device ID (0-based)
- `memoryPoolSize` - GPU memory pool size in MB
- `logPath` - Optional log file path (can be NULL)
- `configPath` - Optional config file path (can be NULL)
- `userData` - User data pointer (optional)

### FrameData

Frame data structure.

```cpp
typedef struct FrameData {
    uint8_t* colorData;            /**< RGBA color data */
    uint8_t* depthData;            /**< Depth information (optional) */
    uint32_t width;                /**< Frame width */
    uint32_t height;               /**< Frame height */
    uint32_t stride;               /**< Bytes per row */
    float timestamp;               /**< Presentation timestamp */
    uint32_t frameId;              /**< Unique frame identifier */
    FrameFormat format;            /**< Pixel format */
    void* userData;                /**< User data pointer */
} FrameData;
```

**Fields:**
- `colorData` - Pointer to RGBA color data
- `depthData` - Optional depth data pointer
- `width` - Frame width in pixels
- `height` - Frame height in pixels
- `stride` - Bytes per row
- `timestamp` - Frame timestamp in seconds
- `frameId` - Unique frame identifier
- `format` - Pixel format (RGBA8, BGRA8, etc.)
- `userData` - User data pointer

### CrossLinkFGStats

Performance statistics structure.

```cpp
typedef struct CrossLinkFGStats {
    float currentFPS;              /**< Current FPS */
    float averageFrameTime;        /**< Average frame generation time (ms) */
    uint32_t totalFrames;          /**< Total frames processed */
    uint32_t totalGeneratedFrames; /**< Total generated frames */
    float nvidiaUtilization;       /**< NVIDIA GPU utilization (0.0-1.0) */
    float amdUtilization;          /**< AMD GPU utilization (0.0-1.0) */
    uint32_t memoryUsageMB;        /**< Memory usage in MB */
    uint32_t errorCount;           /**< Error count */
    uint32_t warningCount;         /**< Warning count */
} CrossLinkFGStats;
```

### Enumerations

#### FrameGenQuality

Frame generation quality levels.

```cpp
typedef enum FrameGenQuality {
    FRAME_GEN_QUALITY_ULTRA = 0,   /**< Ultra quality */
    FRAME_GEN_QUALITY_HIGH,        /**< High quality */
    FRAME_GEN_QUALITY_MEDIUM,      /**< Medium quality */
    FRAME_GEN_QUALITY_LOW,         /**< Low quality */
    FRAME_GEN_QUALITY_CUSTOM       /**< Custom quality */
} FrameGenQuality;
```

#### LogLevel

Logging levels.

```cpp
typedef enum LogLevel {
    LOG_LEVEL_DEBUG = 0,           /**< Debug messages */
    LOG_LEVEL_INFO,                /**< Information messages */
    LOG_LEVEL_WARNING,             /**< Warning messages */
    LOG_LEVEL_ERROR                /**< Error messages */
} LogLevel;
```

#### FrameFormat

Pixel formats.

```cpp
typedef enum FrameFormat {
    FRAME_FORMAT_RGBA8 = 0,        /**< RGBA 8-bit per channel */
    FRAME_FORMAT_BGRA8,            /**< BGRA 8-bit per channel */
    FRAME_FORMAT_RGB8,             /**< RGB 8-bit per channel */
    FRAME_FORMAT_BGR8,             /**< BGR 8-bit per channel */
    FRAME_FORMAT_RGBA16,           /**< RGBA 16-bit per channel */
    FRAME_FORMAT_RGBA32F           /**< RGBA 32-bit float per channel */
} FrameFormat;
```

## Error Codes

### Return Values

Most API functions return `bool` indicating success (`true`) or failure (`false`). Use `CrossLinkFG_GetLastError()` to get detailed error information.

### CrossLinkFG_GetLastError

Gets the last error that occurred.

```cpp
/**
 * @brief Get last error information
 * @return Error code
 */
CROSSLINK_FG_API CrossLinkFGError CrossLinkFG_GetLastError();
```

### Error Codes

```cpp
typedef enum CrossLinkFGError {
    CROSSLINK_FG_SUCCESS = 0,              /**< No error */
    CROSSLINK_FG_ERROR_GENERAL,            /**< General error */
    CROSSLINK_FG_ERROR_INVALID_CONFIG,     /**< Invalid configuration */
    CROSSLINK_FG_ERROR_GPU_NOT_FOUND,      /**< GPU not found */
    CROSSLINK_FG_ERROR_GPU_UNSUPPORTED,    /**< GPU unsupported */
    CROSSLINK_FG_ERROR_DIRECTX12_NOT_FOUND,/**< DirectX 12 not found */
    CROSSLINK_FG_ERROR_INSUFFICIENT_MEMORY,/**< Insufficient memory */
    CROSSLINK_FG_ERROR_OPENCL_FAILED,      /**< OpenCL initialization failed */
    CROSSLINK_FG_ERROR_OPTICAL_FLOW_FAILED,/**< Optical flow failed */
    CROSSLINK_FG_ERROR_FRAME_PROCESSING,   /**< Frame processing failed */
    CROSSLINK_FG_ERROR_INVALID_PARAMETER,  /**< Invalid parameter */
    CROSSLINK_FG_ERROR_NOT_INITIALIZED,    /**< Not initialized */
    CROSSLINK_FG_ERROR_ALREADY_INITIALIZED,/**< Already initialized */
    CROSSLINK_FG_ERROR_PERMISSION_DENIED,  /**< Permission denied */
    CROSSLINK_FG_ERROR_TIMEOUT             /**< Operation timeout */
} CrossLinkFGError;
```

### Error Helper Functions

```cpp
/**
 * @brief Get error description
 * @param error Error code
 * @return Error description string
 */
CROSSLINK_FG_API const char* CrossLinkFG_ErrorString(
    CrossLinkFGError error
);

/**
 * @brief Check if error indicates temporary failure
 * @param error Error code
 * @return true if error is temporary
 */
CROSSLINK_FG_API bool CrossLinkFG_IsTemporaryError(
    CrossLinkFGError error
);

/**
 * @brief Check if error indicates fatal failure
 * @param error Error code
 * @return true if error is fatal
 */
CROSSLINK_FG_API bool CrossLinkFG_IsFatalError(
    CrossLinkFGError error
);
```

**Example:**
```cpp
CrossLinkFGError error = CrossLinkFG_GetLastError();
const char* errorStr = CrossLinkFG_ErrorString(error);

if (CrossLinkFG_IsFatalError(error)) {
    fprintf(stderr, "Fatal error: %s\n", errorStr);
    // Application should exit
} else if (CrossLinkFG_IsTemporaryError(error)) {
    fprintf(stderr, "Temporary error: %s\n", errorStr);
    // Retry operation
} else {
    fprintf(stderr, "Error: %s\n", errorStr);
    // Handle error
}
```

## Constants

### Version Information

```cpp
#define CROSSLINK_FG_VERSION_MAJOR    1
#define CROSSLINK_FG_VERSION_MINOR    0
#define CROSSLINK_FG_VERSION_PATCH    0
#define CROSSLINK_FG_VERSION          ((CROSSLINK_FG_VERSION_MAJOR << 16) | \
                                       (CROSSLINK_FG_VERSION_MINOR << 8) | \
                                       (CROSSLINK_FG_VERSION_PATCH))

#define CROSSLINK_FG_VERSION_STRING   "1.0.0"
```

### Limits

```cpp
#define CROSSLINK_FG_MIN_WIDTH        640
#define CROSSLINK_FG_MIN_HEIGHT       480
#define CROSSLINK_FG_MAX_WIDTH        7680
#define CROSSLINK_FG_MAX_HEIGHT       4320
#define CROSSLINK_FG_MIN_FPS          30
#define CROSSLINK_FG_MAX_FPS          240
#define CROSSLINK_FG_MIN_MEMORY_POOL  512      // MB
#define CROSSLINK_FG_MAX_MEMORY_POOL  8192     // MB
```

## Callback API

### Progress Callback

```cpp
typedef void (*CrossLinkFG_ProgressCallback)(
    uint32_t current,
    uint32_t total,
    void* userData
);

/**
 * @brief Set progress callback
 * @param callback Progress callback function
 * @param userData User data pointer
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetProgressCallback(
    CrossLinkFG_ProgressCallback callback,
    void* userData
);
```

**Example:**
```cpp
void ProgressCallback(uint32_t current, uint32_t total, void* userData) {
    printf("Progress: %u/%u (%.1f%%)\n", 
           current, total, 
           (float)current / total * 100.0f);
}

CrossLinkFG_SetProgressCallback(ProgressCallback, nullptr);
```

### Log Callback

```cpp
typedef void (*CrossLinkFG_LogCallback)(
    LogLevel level,
    const char* message,
    void* userData
);

/**
 * @brief Set log callback
 * @param callback Log callback function
 * @param userData User data pointer
 * @return void
 */
CROSSLINK_FG_API void CrossLinkFG_SetLogCallback(
    CrossLinkFG_LogCallback callback,
    void* userData
);
```

**Example:**
```cpp
void LogCallback(LogLevel level, const char* message, void* userData) {
    const char* levelStr[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    printf("[%s] %s\n", levelStr[level], message);
}

CrossLinkFG_SetLogCallback(LogCallback, nullptr);
```

---

For more examples and usage patterns, see the [User Guide](USER_GUIDE.md) and [Developer Guide](DEVELOPER_GUIDE.md).
