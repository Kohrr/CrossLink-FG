// Stub header for Nvidia Optical Flow to avoid requiring full SDK installation
// Only minimal definitions for CrossLink-FG

#ifndef NV_OPTICAL_FLOW_H_
#define NV_OPTICAL_FLOW_H_

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for basic types
typedef struct NvOFGPUVAAPIInteropHelper NvOFGPUVAAPIInteropHelper;
typedef struct NvOFObj NvOFHandle;

// Structs
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t format;  // NV_OF_BUFFER_FORMAT_
} NvOFBufferFormat;

typedef struct {
    NvOFBufferFormat inputBufferFormat;
    NvOFBufferFormat outputBufferFormat;
    uint32_t width;
    uint32_t height;
    uint32_t preset;  // NV_OF_MODE_
} NvOFInitParams;

typedef struct {
    uint32_t performanceState;  // NV_OF_PERF_
} NvOFExecuteParams;

// Error codes
#define NV_OF_SUCCESS              0
#define NV_OF_ERR_INVALID_INPUT   -1
#define NV_OF_ERR_DEVICE_NOT_SUPPORTED -2

// Buffer formats
#define NV_OF_BUFFER_FORMAT_ABGR8 0
#define NV_OF_BUFFER_FORMAT_NV12 1

// Presets
#define NV_OF_MODE_OPTICALFLOW_ESTIMATION 0

// Performance levels
#define NV_OF_PERF_LEVEL_SLOW 0
#define NV_OF_PERF_LEVEL_MEDIUM 1
#define NV_OF_PERF_LEVEL_FAST 2

// Simplified buffer handle (placeholder)
typedef void* NvOFBufferHandle;

// Function prototypes (stubs - will be loaded dynamically if possible)
typedef int32_t (*PFNNVOF_CREATE_INSTANCE)(uint32_t apiVer, NvOFHandle **hOf);
typedef int32_t (*PFNNVOF_INIT)(NvOFHandle hOf, NvOFInitParams *initParams);
typedef int32_t (*PFNNVOF_EXECUTE)(NvOFHandle hOf, NvOFBufferHandle hInput, NvOFBufferHandle hReference, NvOFBufferHandle hOutput, NvOFExecuteParams *executeParams, void *pFencePoint);
typedef int32_t (*PFNNVOF_DESTROY_INSTANCE)(NvOFHandle hOf);

// Function pointer type definitions (for dynamic loading)
typedef int32_t (*PFNNVOF_CREATE_INSTANCE)(uint32_t apiVer, NvOFHandle **hOf);
typedef int32_t (*PFNNVOF_INIT)(NvOFHandle hOf, NvOFInitParams *initParams);
typedef int32_t (*PFNNVOF_EXECUTE)(NvOFHandle hOf, NvOFBufferHandle hInput, NvOFBufferHandle hReference, NvOFBufferHandle hOutput, NvOFExecuteParams *executeParams, void *pFencePoint);
typedef int32_t (*PFNNVOF_DESTROY_INSTANCE)(NvOFHandle hOf);

// Initialization function to load the library
bool LoadNvidiaOpticalFlow();

#ifdef __cplusplus
}
#endif

#endif  // NV_OPTICAL_FLOW_H_
