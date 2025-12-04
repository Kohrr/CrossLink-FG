#ifndef NVIDIAOPTICALFLOW_H
#define NVIDIAOPTICALFLOW_H

#include <cstdint>

class NvidiaOpticalFlow {
public:
    NvidiaOpticalFlow();
    ~NvidiaOpticalFlow();

    bool Initialize();
    void Shutdown();

private:
    bool initialized_ = false;
};

#endif // NVIDIAOPTICALFLOW_H
