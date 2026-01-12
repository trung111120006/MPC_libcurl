#include "streaming_stats.h"
#include "config.h"
#include <iostream>
#include <numeric>
#include <map>
#include <iomanip>

namespace StreamingStats {
    void printSegmentHeader(int segNum, int totalSegments) {
        std::cout << "===== Segment " << segNum << "/" << totalSegments 
                  << " =====" << std::endl;
    }
    
    void printMPCDecision(int quality, const std::string& qualityName) {
        std::cout << "MPC Decision: Quality " << quality
                  << " (" << qualityName << " - " 
                  << (quality + 1) * 5 << " Mbps)" << std::endl;
    }
    
    void printDownloadStats(double bandwidth, double chunkSize, 
                           double downloadTime, double bufferLevel) {
        std::cout << "Measured bandwidth: " << bandwidth << " kbps" << std::endl;
        std::cout << "Chunk size: " << chunkSize << " KB" << std::endl;
        std::cout << "Download time: " << downloadTime << " seconds" << std::endl;
        std::cout << "Buffer level: " << bufferLevel << " seconds" << std::endl;
    }
    
    void printFinalStats(const std::vector<double>& pastBandwidth,
                        const std::vector<double>& pastErrors,
                        const std::vector<int>& qualityHistory) {
        std::cout << "\n===== Streaming Statistics =====" << std::endl;
        
        if (!pastBandwidth.empty()) {
            double avgBandwidth = std::accumulate(pastBandwidth.begin(), 
                                                 pastBandwidth.end(), 0.0) 
                                / pastBandwidth.size();
            std::cout << "Average bandwidth: " << avgBandwidth << " kbps" << std::endl;
        }
        
        if (!pastErrors.empty()) {
            double avgError = std::accumulate(pastErrors.begin(), 
                                             pastErrors.end(), 0.0) 
                            / pastErrors.size() * 100;
            std::cout << "Average prediction error: " << avgError << "%" << std::endl;
        }
        
        std::cout << "\nQuality distribution:" << std::endl;
        std::map<int, int> qualityCount;
        for (int q : qualityHistory) {
            qualityCount[q]++;
        }
        
        for (auto& pair : qualityCount) {
            std::cout << "  " << Config::QUALITY_NAMES[pair.first] << ": " 
                      << pair.second << " segments (" 
                      << std::fixed << std::setprecision(1)
                      << (pair.second * 100.0 / qualityHistory.size()) 
                      << "%)" << std::endl;
        }
    }
}