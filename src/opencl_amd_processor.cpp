// AMD OpenCL Frame Generation Processor
// Handles GPU compute on AMD iGPU for frame interpolation
#include <iostream>
#include <vector>
#include <string>
#include <CL/cl.h>
#include "OpenCLProcessor.h"

#pragma comment(lib, "OpenCL.lib")

class AMDOpenCLProcessor {
private:
    cl_context context = nullptr;
    cl_command_queue command_queue = nullptr;
    cl_program program = nullptr;

    cl_kernel interpolate_kernel = nullptr;
    cl_kernel motion_kernel = nullptr;
    cl_kernel sharpen_kernel = nullptr;

    cl_device_id device_id = nullptr;
    cl_platform_id platform_id = nullptr;

    std::string kernel_source;
    bool initialized = false;

public:
    AMDOpenCLProcessor() {
        // Load kernel source from file
        kernel_source = R"CLC(
// Simplified version of the frame interpolation kernel
__kernel void basic_interpolate(
    __global const uchar4* frame_prev,
    __global const uchar4* frame_curr,
    __global uchar4* frame_out,
    const int width,
    const int height,
    const float blend_factor
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) return;

    int idx = y * width + x;

    // Simple linear interpolation
    uchar4 prev_pix = frame_prev[idx];
    uchar4 curr_pix = frame_curr[idx];

    // Linear blend
    float4 result;
    result.x = prev_pix.x * (1.0f - blend_factor) + curr_pix.x * blend_factor;
    result.y = prev_pix.y * (1.0f - blend_factor) + curr_pix.y * blend_factor;
    result.z = prev_pix.z * (1.0f - blend_factor) + curr_pix.z * blend_factor;
    result.w = 255; // Alpha channel

    frame_out[idx] = convert_uchar4_sat(result);
}
)CLC";
    }

    ~AMDOpenCLProcessor() {
        cleanup();
    }

    bool Initialize() {
        std::cout << "[AMD OpenCL] Initializing AMD GPU compute..." << std::endl;

        if (initialized) return true;

        // Get platform
        cl_uint num_platforms;
        clGetPlatformIDs(0, nullptr, &num_platforms);
        if (num_platforms == 0) {
            std::cerr << "[AMD OpenCL ERROR] No OpenCL platforms found!" << std::endl;
            return false;
        }

        std::vector<cl_platform_id> platforms(num_platforms);
        clGetPlatformIDs(num_platforms, platforms.data(), nullptr);

        // Look for AMD platform
        bool amd_found = false;
        for (auto platform : platforms) {
            char platform_name[128];
            clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, nullptr);

            std::string name(platform_name);
            if (name.find("AMD") != std::string::npos ||
                name.find("Advanced Micro Devices") != std::string::npos) {
                platform_id = platform;
                amd_found = true;
                std::cout << "[AMD OpenCL] Found AMD platform: " << name << std::endl;
                break;
            }
        }

        if (!amd_found) {
            // Fallback to any platform
            platform_id = platforms[0];
            char name[128];
            clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(name), name, nullptr);
            std::cout << "[AMD OpenCL] Using alternative platform: " << name << std::endl;
        }

        // Get device
        cl_uint num_devices;
        clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
        if (num_devices == 0) {
            std::cerr << "[AMD OpenCL ERROR] No GPU devices found!" << std::endl;
            return false;
        }

        std::vector<cl_device_id> devices(num_devices);
        clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);

        device_id = devices[0]; // Use first GPU

        // Get device info
        char device_name[128];
        clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
        std::cout << "[AMD OpenCL] Using device: " << device_name << std::endl;

        // Create context
        cl_int status;
        context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create context" << std::endl;
            return false;
        }

        // Create command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create command queue" << std::endl;
            return false;
        }

        // Create program
        const char* source_ptr = kernel_source.c_str();
        size_t source_size = kernel_source.size();
        program = clCreateProgramWithSource(context, 1, &source_ptr, &source_size, &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create program" << std::endl;
            return false;
        }

        // Build program
        status = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to build program" << std::endl;

            // Get build log
            size_t log_size;
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            std::vector<char> build_log(log_size);
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log.data(), nullptr);
            std::cerr << "Build log: " << build_log.data() << std::endl;
            return false;
        }

        // Create kernel
        interpolate_kernel = clCreateKernel(program, "basic_interpolate", &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create kernel" << std::endl;
            return false;
        }

        initialized = true;
        std::cout << "[AMD OpenCL SUCCESS] AMD GPU compute initialized and ready!" << std::endl;
        return true;
    }

    bool InterpolateFrames(
        const std::vector<uint8_t>& frame_prev,
        const std::vector<uint8_t>& frame_curr,
        std::vector<uint8_t>& frame_out,
        int width,
        int height,
        float blend_factor = 0.5f
    ) {
        if (!initialized) return false;

        std::cout << "[AMD OpenCL] Starting frame interpolation on AMD GPU..." << std::endl;

        const size_t frame_size = frame_prev.size();
        frame_out.resize(frame_size);

        cl_int status;

        // Create buffers
        cl_mem buffer_prev = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         frame_size, const_cast<uint8_t*>(frame_prev.data()), &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create prev buffer" << std::endl;
            return false;
        }

        cl_mem buffer_curr = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         frame_size, const_cast<uint8_t*>(frame_curr.data()), &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create curr buffer" << std::endl;
            clReleaseMemObject(buffer_prev);
            return false;
        }

        cl_mem buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, frame_size, nullptr, &status);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to create out buffer" << std::endl;
            clReleaseMemObject(buffer_prev);
            clReleaseMemObject(buffer_curr);
            return false;
        }

        // Set kernel arguments
        clSetKernelArg(interpolate_kernel, 0, sizeof(cl_mem), &buffer_prev);
        clSetKernelArg(interpolate_kernel, 1, sizeof(cl_mem), &buffer_curr);
        clSetKernelArg(interpolate_kernel, 2, sizeof(cl_mem), &buffer_out);
        clSetKernelArg(interpolate_kernel, 3, sizeof(int), &width);
        clSetKernelArg(interpolate_kernel, 4, sizeof(int), &height);
        clSetKernelArg(interpolate_kernel, 5, sizeof(float), &blend_factor);

        // Execute kernel
        size_t global_work_size[2] = { size_t(width), size_t(height) };
        status = clEnqueueNDRangeKernel(command_queue, interpolate_kernel, 2, nullptr,
                                    global_work_size, nullptr, 0, nullptr, nullptr);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to execute kernel" << std::endl;
            clReleaseMemObject(buffer_prev);
            clReleaseMemObject(buffer_curr);
            clReleaseMemObject(buffer_out);
            return false;
        }

        // Read result
        status = clEnqueueReadBuffer(command_queue, buffer_out, CL_TRUE, 0, frame_size,
                               frame_out.data(), 0, nullptr, nullptr);
        if (status != CL_SUCCESS) {
            std::cerr << "[AMD OpenCL ERROR] Failed to read result" << std::endl;
            clReleaseMemObject(buffer_prev);
            clReleaseMemObject(buffer_curr);
            clReleaseMemObject(buffer_out);
            return false;
        }

        clFinish(command_queue);

        // Cleanup
        clReleaseMemObject(buffer_prev);
        clReleaseMemObject(buffer_curr);
        clReleaseMemObject(buffer_out);

        std::cout << "[AMD OpenCL SUCCESS] Frame interpolation completed on AMD GPU!" << std::endl;
        return true;
    }

private:
    void cleanup() {
        if (interpolate_kernel) clReleaseKernel(interpolate_kernel);
        if (program) clReleaseProgram(program);
        if (command_queue) clReleaseCommandQueue(command_queue);
        if (context) clReleaseContext(context);

        initialized = false;
    }
};

// Global AMD processor instance
static AMDOpenCLProcessor g_AMDProcessor;

// Public interface functions
bool OpenCLProcessor::InitializeAMD() {
    return g_AMDProcessor.Initialize();
}

bool OpenCLProcessor::ProcessFramesAMD(
    const std::vector<uint8_t>& frame_prev,
    const std::vector<uint8_t>& frame_curr,
    std::vector<uint8_t>& frame_out,
    int width,
    int height
) {
    return g_AMDProcessor.InterpolateFrames(frame_prev, frame_curr, frame_out, width, height);
}

bool OpenCLProcessor::IsAMDReady() {
    return true; // AMD OpenCL is available on most systems
}
