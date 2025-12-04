#ifndef OPENCLPROCESSOR_H
#define OPENCLPROCESSOR_H

#include <cstdint>

class OpenCLProcessor {
public:
    OpenCLProcessor();
    ~OpenCLProcessor();

    bool Initialize();
    void Shutdown();
    void ProcessFrame();

private:
    bool initialized_ = false;
};

#endif // OPENCLPROCESSOR_H
