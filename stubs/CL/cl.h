// Stub header for OpenCL to avoid requiring full SDK installation
// Only minimal definitions for CrossLink-FG

#ifndef __OPENCL_CL_H
#define __OPENCL_CL_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Basic types
typedef int32_t         cl_int;
typedef uint32_t        cl_uint;
typedef uint64_t        cl_ulong;
typedef void*           cl_platform_id;
typedef void*           cl_device_id;
typedef void*           cl_context;
typedef void*           cl_command_queue;
typedef void*           cl_mem;
typedef void*           cl_program;
typedef void*           cl_kernel;

// Additional types for function prototypes
typedef uint32_t        cl_bitfield;  // For flags
typedef cl_bitfield     cl_device_type;
typedef cl_bitfield     cl_mem_flags;
typedef cl_bitfield     cl_command_queue_properties;
typedef unsigned int    cl_bool;
typedef cl_uint         cl_kernel_work_group_info;

// Event and callback types (simplified)
typedef void*           cl_event;
typedef void (*cl_event_callback)(cl_event, int, void*);

// Context properties
typedef intptr_t        cl_context_properties;

// Callback
#define CL_CALLBACK

// Error codes
#define CL_SUCCESS           0
#define CL_DEVICE_NOT_FOUND -33

// Platform info
#define CL_PLATFORM_NAME 0x0902
#define CL_PLATFORM_VERSION 0x0904

// Device type
#define CL_DEVICE_TYPE_GPU 0x1000
#define CL_DEVICE_TYPE_ALL 0xFFFFFFFF

// Device info
#define CL_DEVICE_NAME 0x102B
#define CL_DEVICE_TYPE 0x1000

// Memory flags
#define CL_MEM_READ_WRITE     0x04
#define CL_MEM_ALLOC_HOST_PTR 0x02

// Function pointer type definitions (for dynamic loading)
typedef cl_int (*PFN_clGetPlatformIDs)(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);
typedef cl_int (*PFN_clGetPlatformInfo)(cl_platform_id platform, cl_uint param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
typedef cl_int (*PFN_clGetDeviceIDs)(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);
typedef cl_int (*PFN_clGetDeviceInfo)(cl_device_id device, cl_uint param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
typedef cl_context (*PFN_clCreateContext)(const cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices, void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data), void *user_data, cl_int *errcode_ret);
typedef cl_int (*PFN_clReleaseContext)(cl_context context);
typedef cl_command_queue (*PFN_clCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int *errcode_ret);
typedef cl_int (*PFN_clReleaseCommandQueue)(cl_command_queue command_queue);
typedef cl_mem (*PFN_clCreateBuffer)(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret);
typedef cl_mem (*PFN_clCreateSharedBufferAMD)(cl_context context, cl_mem_flags flags, cl_uint *gl_object_type, cl_uint gl_object_name, int *errcode_ret);
typedef cl_int (*PFN_clEnqueueAcquireGLObjects)(cl_command_queue command_queue, cl_uint num_objects, const cl_mem *mem_objects, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
typedef cl_int (*PFN_clEnqueueReleaseGLObjects)(cl_command_queue command_queue, cl_uint num_objects, const cl_mem *mem_objects, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
typedef cl_program (*PFN_clCreateProgramWithSource)(cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret);
typedef cl_int (*PFN_clBuildProgram)(cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program, void *user_data), void *user_data);
typedef cl_int (*PFN_clReleaseProgram)(cl_program program);
typedef cl_kernel (*PFN_clCreateKernel)(cl_program program, const char *kernel_name, cl_int *errcode_ret);
typedef cl_int (*PFN_clReleaseKernel)(cl_kernel kernel);
typedef cl_int (*PFN_clSetKernelArg)(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);
typedef cl_int (*PFN_clEnqueueReadBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t size, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
typedef cl_int (*PFN_clEnqueueWriteBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
typedef cl_int (*PFN_clEnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
typedef cl_int (*PFN_clFlush)(cl_command_queue command_queue);
typedef cl_int (*PFN_clFinish)(cl_command_queue command_queue);
typedef cl_int (*PFN_clGetKernelWorkGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

// Initialization function to load the library
bool LoadOpenCL();

#ifdef __cplusplus
}
#endif

#endif  // __OPENCL_CL_H
