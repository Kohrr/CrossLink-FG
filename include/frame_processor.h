#ifndef FRAME_PROCESSOR_H
#define FRAME_PROCESSOR_H

#include "frame_capture.h"
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

class FrameProcessor {
public:
    static void Start();
    static void Stop();
    static void QueueFrame(const CapturedFrame& frame);
    static bool IsRunning() { return s_running; }

private:
    static void ProcessingThread();
    static void ProcessFrame(const CapturedFrame& frame);

    static std::atomic<bool> s_running;
    static std::thread s_processingThread;
    static std::queue<CapturedFrame> s_frameQueue;
    static std::mutex s_queueMutex;
};

#endif // FRAME_PROCESSOR_H
