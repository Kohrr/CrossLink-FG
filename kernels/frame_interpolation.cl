// CROSSLINK-FG OpenCL Kernel for AMD GPU Frame Generation
// Performs Motion Estimation and Frame Interpolation between two captured frames
// Converts 60 FPS to 120 FPS via adaptive temporal blending

#pragma OPENCL EXTENSION cl_amd_fp64 : enable

// Frame Interpolation Kernel - Core Algorithm
__kernel void interpolate_frames(
    __global const uchar4* frame_current,   // Current frame (t0)
    __global const uchar4* frame_previous,  // Previous frame (t-1)
    __global uchar4* frame_output,          // Interpolated output (t0.5)
    const int width,
    const int height,
    const float blend_factor              // 0.0 = previous, 1.0 = current
) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    if (x >= width || y >= height) return;

    const int idx = y * width + x;

    // Load pixel data
    float4 current = convert_float4(frame_current[idx]) / 255.0f;
    float4 previous = convert_float4(frame_previous[idx]) / 255.0f;

    // Simple temporal interpolation (can be enhanced with motion vectors)
    float4 interpolated = mix(previous, current, blend_factor);

    // Add subtle motion blur for smoothness
    float motion_blur_strength = 0.1f;
    interpolated += (current - previous) * motion_blur_strength * (1.0f - blend_factor);

    // Output interpolated frame
    frame_output[idx] = convert_uchar4_sat(interpolated * 255.0f);
}

// Enhanced Motion Compensated Interpolation
__kernel void motion_compensated_interpolate(
    __global const uchar4* frame_current,
    __global const uchar4* frame_previous,
    __global float2* motion_vectors,       // Pre-computed motion field
    __global uchar4* frame_output,
    const int width,
    const int height,
    const float blend_factor
) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    if (x >= width || y >= height) return;

    const int idx = y * width + x;

    // Get motion vector for this pixel
    float2 motion = motion_vectors[idx] * (1.0f - blend_factor);

    // Sample from motion-compensated position
    int motion_x = clamp((int)(x + motion.x), 0, width - 1);
    int motion_y = clamp((int)(y + motion.y), 0, height - 1);
    const int motion_idx = motion_y * width + motion_x;

    // Load samples
    float4 current = convert_float4(frame_current[idx]) / 255.0f;
    float4 previous_warped = convert_float4(frame_previous[motion_idx]) / 255.0f;

    // Motion-compensated interpolation
    float4 interpolated = mix(previous_warped, current, blend_factor);

    // Apply edge-preserving smoothing
    float4 neighborhood_avg = (convert_float4(frame_current[idx]) +
                             convert_float4(frame_previous[idx])) / (2.0f * 255.0f);

    float preserve_edges = 0.7f;
    interpolated = mix(interpolated, neighborhood_avg, preserve_edges);

    // Output enhanced interpolated frame
    frame_output[idx] = convert_uchar4_sat(interpolated * 255.0f);
}

// Block Matching Motion Estimation (simplified optical flow)
__kernel void compute_motion_vectors(
    __global const uchar4* frame_current,
    __global const uchar4* frame_previous,
    __global float2* motion_vectors,
    const int width,
    const int height,
    const int block_size
) {
    const int block_x = get_global_id(0);
    const int block_y = get_global_id(1);

    const int x = block_x * block_size;
    const int y = block_y * block_size;

    if (x >= width || y >= height) return;

    // Search area around current block
    const int search_radius = 8;
    float best_sad = 1e10f;
    int best_dx = 0, best_dy = 0;

    // Calculate SAD (Sum of Absolute Differences) for motion estimation
    for (int dy = -search_radius; dy <= search_radius; dy++) {
        for (int dx = -search_radius; dx <= search_radius; dx++) {
            float sad = 0.0f;

            // Compare blocks
            for (int by = 0; by < block_size && y + by < height; by++) {
                for (int bx = 0; bx < block_size && x + bx < width; bx++) {
                    int cx = clamp(x + bx + dx, 0, width - 1);
                    int cy = clamp(y + by + dy, 0, height - 1);
                    int px = x + bx;
                    int py = y + by;

                    float4 curr_pix = convert_float4(frame_current[py * width + px]);
                    float4 prev_pix = convert_float4(frame_previous[cy * width + cx]);

                    // RGB SAD
                    sad += fabs(curr_pix.x - prev_pix.x) +
                           fabs(curr_pix.y - prev_pix.y) +
                           fabs(curr_pix.z - prev_pix.z);
                }
            }

            if (sad < best_sad) {
                best_sad = sad;
                best_dx = dx;
                best_dy = dy;
            }
        }
    }

    // Store motion vector for this block
    for (int by = 0; by < block_size && y + by < height; by++) {
        for (int bx = 0; bx < block_size && x + bx < width; bx++) {
            int idx = (y + by) * width + (x + bx);
            motion_vectors[idx] = (float2)(best_dx, best_dy);
        }
    }
}

// Advanced Frame Sharpening Kernel
__kernel void sharpen_interpolated_frames(
    __global uchar4* frame_input,
    __global uchar4* frame_output,
    const int width,
    const int height,
    const float sharpness_factor
) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    if (x >= width || y >= height || x == 0 || y == 0 ||
        x == width - 1 || y == height - 1) {
        if (x < width && y < height) {
            frame_output[y * width + x] = frame_input[y * width + x];
        }
        return;
    }

    const int idx = y * width + x;

    // Laplacian sharpening kernel
    float4 center = convert_float4(frame_input[idx]);
    float4 up = convert_float4(frame_input[(y-1) * width + x]);
    float4 down = convert_float4(frame_input[(y+1) * width + x]);
    float4 left = convert_float4(frame_input[y * width + (x-1)]);
    float4 right = convert_float4(frame_input[y * width + (x+1)]);

    // Apply Laplacian operator: center - (up + down + left + right) / 4
    float4 laplacian = center - (up + down + left + right) * 0.25f;

    // Sharpen: center + sharpness_factor * laplacian
    float4 sharpened = center + laplacian * sharpness_factor;

    // Clamp to valid range
    sharpened = clamp(sharpened, 0.0f, 255.0f);

    frame_output[idx] = convert_uchar4(sharpened);
}
