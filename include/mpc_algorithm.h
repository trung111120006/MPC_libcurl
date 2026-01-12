// đây là file xử lý các hàm thuật toán MPC để chọn quality level tiếp theo dựa trên băng thông đã đo và kích thước chunk video trong tương lai.
#ifndef MPC_ALGORITHM_H
#define MPC_ALGORITHM_H

#include <vector>
#include <string>

namespace MPC {
    // Generate tất cả combinations của quality levels
    void generateCombinations(int P, std::string combo, 
                             std::vector<std::string>& combinations);
    
    // Tính next quality level dựa trên MPC
    int selectNextQuality(std::vector<double>& pastBandwidth,
                         std::vector<double>& pastBandwidthEsts,
                         std::vector<double>& pastErrors,
                         const std::vector<double>& allFutureChunksSizes,
                         int P,
                         double bufferSize,
                         int videoChunkRemain,
                         int lastQuality);
    
    // Estimate future bandwidth
    double estimateBandwidth(const std::vector<double>& pastBandwidth,
                            const std::vector<double>& pastErrors);
}

#endif