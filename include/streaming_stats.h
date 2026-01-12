// đây là file xử lý các hàm in thống kê streaming như in thống kê cuối session, in header segment, in quyết định MPC và in thống kê download.
#ifndef STREAMING_STATS_H
#define STREAMING_STATS_H

#include <vector>
#include <string>

namespace StreamingStats {
    // Print statistics cuối session
    void printFinalStats(const std::vector<double>& pastBandwidth,
                        const std::vector<double>& pastErrors,
                        const std::vector<int>& qualityHistory);
    
    // Print segment info
    void printSegmentHeader(int segNum, int totalSegments);
    
    // Print MPC decision
    void printMPCDecision(int quality, const std::string& qualityName);
    
    // Print download stats
    void printDownloadStats(double bandwidth, double chunkSize, 
                           double downloadTime, double bufferLevel);
}

#endif