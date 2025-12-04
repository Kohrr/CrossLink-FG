// NeuralInterpolator - ncnn + RIFE Model
// Real-Time Intermediate Flow Estimation for frame generation

#pragma once
#include "ncnn/net.h"
#include <string>
#include <vector>

class NeuralInterpolator {
public:
    NeuralInterpolator();
    ~NeuralInterpolator();

    bool Initialize(const std::string& modelPath = "");
    void Shutdown();

    // Frame interpolation
    bool GenerateInterpolation(const unsigned char* frameA, const unsigned char* frameB,
                              unsigned char* output, int width, int height);

    // Model loading
    bool LoadRIFEModel();
    bool IsInitialized() const { return m_initialized; }

private:
    ncnn::Net m_net;
    bool m_initialized = false;

    // RIFE specific
    int m_inputWidth = 1920;
    int m_inputHeight = 1080;
    int m_channels = 3;

    // Model paths
    std::string m_modelParamPath;
    std::string m_modelBinPath;

    // Preprocessing
    void PreprocessFrame(const unsigned char* input, ncnn::Mat& output, int w, int h);
    void PostprocessFrame(const ncnn::Mat& input, unsigned char* output, int w, int h);

    // AI Pipeline
    bool RunInference(const ncnn::Mat& frameA, const ncnn::Mat& frameB, ncnn::Mat& output);

    // Memory management
    std::vector<ncnn::Mat> m_inputMats;
    std::vector<ncnn::Mat> m_outputMats;
};
