#include "config.h"
#include "file_utils.h"
#include "curl_helper.h"
#include "decoder.h"
#include "mpc_algorithm.h"
#include "streaming_stats.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

int main() {
    // Initialize
    Config::init();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = CurlHelper::initCurl();
    
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return 1;
    }
    
    // Tạo thư mục
    FileUtils::createDirectory(Config::DOWNLOAD_DIR);
    FileUtils::createDirectory(Config::DECODED_OUTPUT_DIR);
    
    // State variables
    std::vector<double> pastBandwidth;
    std::vector<double> pastBandwidthEsts;
    std::vector<double> pastErrors;
    std::vector<int> qualityHistory;
    
    // Initialize chunk sizes
    std::vector<double> allFutureChunksSizes;
    for (int i = 0; i < 100; i++) {
        for (double size : Config::CHUNK_SIZES) {
            allFutureChunksSizes.push_back(size);
        }
    }
    
    double bufferSize = Config::INITIAL_BUFFER_SIZE;
    int lastQuality = 0;
    int videoChunkRemain = 0;
    
    // Print initial info
    std::cout << "===== MPC Adaptive Bitrate Streaming =====" << std::endl;
    std::cout << "Download directory: " << Config::DOWNLOAD_DIR << std::endl;
    std::cout << "Decoded output directory: " << Config::DECODED_OUTPUT_DIR << std::endl;
    std::cout << "Lookahead: " << Config::LOOKAHEAD_P << " chunks" << std::endl;
    std::cout << "Initial buffer: " << bufferSize << " seconds" << std::endl;
    std::cout << "Decode mode: " << (Config::DECODE_REALTIME ? "Real-time" : "Manual") << std::endl;
    std::cout << "Rebuffer penalty: " << Config::REBUFFER_PENALTY << std::endl << std::endl;
    
    // Main streaming loop
    for (int seg = 0; seg < Config::TOTAL_SEGMENTS; seg++) {
        StreamingStats::printSegmentHeader(seg + 1, Config::TOTAL_SEGMENTS);
        
        // Select next quality
        int nextQuality;
        if (pastBandwidth.empty()) {
            nextQuality = 0; // Start with LOW
            std::cout << "First chunk: using LOW quality" << std::endl;
        } else {
            nextQuality = MPC::selectNextQuality(
                pastBandwidth, pastBandwidthEsts, pastErrors,
                allFutureChunksSizes, Config::LOOKAHEAD_P,
                bufferSize, videoChunkRemain, lastQuality
            );
            StreamingStats::printMPCDecision(nextQuality, 
                                            Config::QUALITY_NAMES[nextQuality]);
        }
        
        qualityHistory.push_back(nextQuality);
        
        std::cout << "Downloading: " << Config::BASE_URL << std::endl;
        
        // Measure bandwidth
        double chunkSizeKb;
        double measuredBandwidth = CurlHelper::measureBandwidth(
            curl, Config::BASE_URL, chunkSizeKb
        );
        
        if (measuredBandwidth < 0) {
            std::cout << "Download failed" << std::endl;
            measuredBandwidth = !pastBandwidth.empty() ? 
                              pastBandwidth.back() : 1000.0;
        }
        
        pastBandwidth.push_back(measuredBandwidth);
        
        // Download and save file
        std::stringstream ss;
        ss << Config::DOWNLOAD_DIR << "/segment_" 
           << std::setfill('0') << std::setw(3) << (seg + 1)
           << "_" << Config::QUALITY_NAMES[nextQuality];
        std::string savePathBase = ss.str();
        
        std::cout << "Saving file..." << std::endl;
        std::string actualExtension;
        std::string tempSavePath = savePathBase + ".tmp";
        
        if (CurlHelper::downloadFile(curl, Config::BASE_URL, 
                                     tempSavePath, actualExtension)) {
            std::string finalSavePath = savePathBase + actualExtension;
            FileUtils::renameFile(tempSavePath, finalSavePath);
            
            double fileSize = FileUtils::getFileSize(finalSavePath);
            if (fileSize > 0) {
                std::cout << "File size: " << fileSize << " KB" << std::endl;
            }
            
            // Decode segment
            if (Config::DECODE_REALTIME) {
                Decoder::decodeSegmentAsync(finalSavePath, seg + 1, 
                                           Config::QUALITY_NAMES[nextQuality]);
            }
        }
        
        // Update buffer
        double downloadTime = chunkSizeKb * 8 / measuredBandwidth;
        bufferSize = std::max(0.0, bufferSize - downloadTime) + 1.0;
        
        StreamingStats::printDownloadStats(measuredBandwidth, chunkSizeKb,
                                          downloadTime, bufferSize);
        
        videoChunkRemain++;
        lastQuality = nextQuality;
        std::cout << std::endl;
    }
    
    // Cleanup
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    // Print final statistics
    StreamingStats::printFinalStats(pastBandwidth, pastErrors, qualityHistory);
    
    std::cout << "\nAll files saved to: " << Config::DOWNLOAD_DIR << std::endl;
    if (Config::DECODE_REALTIME) {
        std::cout << " Decode processes running" << std::endl;
        std::cout << "  Check: " << Config::DECODED_OUTPUT_DIR << std::endl;
    }
    
    return 0;
}