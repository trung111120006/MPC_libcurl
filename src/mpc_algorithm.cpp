#include "mpc_algorithm.h"
#include "config.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace MPC {
    void generateCombinations(int P, std::string combo, 
                             std::vector<std::string>& combinations) {
        if (combo.length() == P) {
            combinations.push_back(combo);
            return;
        }
        for (char i = '0'; i <= '2'; i++) {
            combo.push_back(i);
            generateCombinations(P, combo, combinations);
            combo.pop_back();
        }
    }
    
    double estimateBandwidth(const std::vector<double>& pastBandwidth,
                            const std::vector<double>& pastErrors) {
        // Harmonic mean của 5 samples gần nhất
        int useSample = std::min((int)pastBandwidth.size(), 5);
        double bandwidthSum = 0.0;
        
        for (int i = pastBandwidth.size() - useSample; i < pastBandwidth.size(); i++) {
            bandwidthSum += 1.0 / std::max(pastBandwidth[i], 1e-6);
        }
        double harmonicBandwidth = useSample / bandwidthSum;
        
        // Max error từ 5 samples gần nhất
        int errorSample = std::min((int)pastErrors.size(), 5);
        double maxError = 0.0;
        if (errorSample > 0) {
            auto start = pastErrors.end() - errorSample;
            maxError = *std::max_element(start, pastErrors.end());
        }
        
        // Conservative estimate
        double futureBandwidth = harmonicBandwidth / (1.0 + maxError);
        return std::max(futureBandwidth, 1e-6);
    }
    
    int selectNextQuality(std::vector<double>& pastBandwidth,
                         std::vector<double>& pastBandwidthEsts,
                         std::vector<double>& pastErrors,
                         const std::vector<double>& allFutureChunksSizes,
                         int P,
                         double bufferSize,
                         int videoChunkRemain,
                         int lastQuality) {
        // Tính prediction error
        double currError = 0.0;
        if (!pastBandwidthEsts.empty() && !pastBandwidth.empty()) {
            currError = std::abs(pastBandwidthEsts.back() - pastBandwidth.back()) /
                       std::max(pastBandwidth.back(), 1e-6);
        }
        pastErrors.push_back(currError);
        
        // Estimate future bandwidth
        double futureBandwidth = estimateBandwidth(pastBandwidth, pastErrors);
        pastBandwidthEsts.push_back(futureBandwidth);
        
        std::cout << "[MPC Debug] Future bandwidth estimate: " 
                  << futureBandwidth << " kbps" << std::endl;
        std::cout << "[MPC Debug] Current buffer: " 
                  << bufferSize << " seconds" << std::endl;
        
        // Generate tất cả combinations
        std::vector<std::string> chunkComboOptions;
        generateCombinations(P, "", chunkComboOptions);
        
        double maxReward = -1e18;
        std::string bestCombo = "";
        double startBuffer = bufferSize;
        
        // Evaluate từng combination
        for (const std::string& chunkCombo : chunkComboOptions) {
            double currRebufferTime = 0.0;
            double currBuffer = startBuffer;
            double bitrateSum = 0.0;
            double smoothnessDif = 0.0;
            int tmpLastQuality = lastQuality;
            
            for (size_t chunkIndex = 0; chunkIndex < chunkCombo.length(); chunkIndex++) {
                int chunkQuality = chunkCombo[chunkIndex] - '0';
                int idx = (videoChunkRemain + chunkIndex) * 3 + chunkQuality;
                
                if (idx >= allFutureChunksSizes.size()) continue;
                
                double downloadTime = allFutureChunksSizes[idx] / futureBandwidth;
                
                // Update buffer
                if (currBuffer < downloadTime) {
                    currRebufferTime += downloadTime - currBuffer;
                    currBuffer = 0.0;
                } else {
                    currBuffer -= downloadTime;
                }
                
                currBuffer += 1.0; // Chunk duration
                bitrateSum += (chunkQuality + 1) * 5.0; // Quality bitrate
                smoothnessDif += std::abs(chunkQuality - tmpLastQuality) * 5.0;
                
                tmpLastQuality = chunkQuality;
            }
            
            // Reward function
            double reward = bitrateSum / 1000.0 
                          - Config::REBUFFER_PENALTY * currRebufferTime 
                          - smoothnessDif / 1000.0;
            
            if (reward > maxReward) {
                maxReward = reward;
                bestCombo = chunkCombo;
            }
        }
        
        std::cout << "[MPC Debug] Best combo: " << bestCombo 
                  << " (reward: " << maxReward << ")" << std::endl;
        
        int bitRate = lastQuality;
        if (!bestCombo.empty()) {
            bitRate = bestCombo[0] - '0';
        }
        return bitRate;
    }
}