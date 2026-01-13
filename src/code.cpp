#include "config.h"
#include "file_utils.h"
#include "curl_helper.h"
#include "decoder.h"
#include "curl/curl.h"
#include "mpc_algorithm.h"
#include "streaming_stats.h"
#include "mpd_parser.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <bits/stdc++.h>

int main() {
    // ===== Init =====
    Config::init();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = CurlHelper::initCurl();

    if (!curl) {
        std::cerr << "Failed to init CURL\n";
        return 1;
    }

    FileUtils::createDirectory(Config::DOWNLOAD_DIR);
    FileUtils::createDirectory(Config::DECODED_OUTPUT_DIR);

    // ===== 1. Parse MPD =====
    MPD mpd = MPDParser::parse(Config::MPD_URL, Config::SERVER_BASE_URL);
    if (mpd.segments.empty()) {
        std::cerr << "No segments in MPD\n";
        return 1;
    }

    double bufferSize = Config::INITIAL_BUFFER_SIZE;

    std::cout << "===== MPD-based Streaming =====\n";
    std::cout << "Starting streaming...\n\n";

    // ===== 2. Streaming loop =====
    for (size_t i = 0; i < mpd.segments.size(); i++) {
        const Segment& seg = mpd.segments[i];

        StreamingStats::printSegmentHeader(i + 1, mpd.segments.size());

        std::stringstream ss;
        ss << Config::DOWNLOAD_DIR << "/segment_"
           << std::setw(4) << std::setfill('0') << seg.number
           << ".bin";

        std::string savePath = ss.str();

        double chunkSizeKb;
        double bandwidth = CurlHelper::measureBandwidth(
            curl, seg.url, chunkSizeKb
        );

        if (bandwidth < 0) {
            std::cerr << "Download failed: " << seg.url << "\n";
            continue;
        }

        std::string extension;
        CurlHelper::downloadFile(curl, seg.url, savePath, extension);

        if (Config::DECODE_REALTIME) {
            Decoder::decodeSegmentAsync(
                savePath,
                seg.number,
                "POINTCLOUD"
            );
        }

        double downloadTime = chunkSizeKb * 8 / bandwidth;
        bufferSize = std::max(0.0, bufferSize - downloadTime)
                   + seg.duration;

        StreamingStats::printDownloadStats(
            bandwidth, chunkSizeKb, downloadTime, bufferSize
        );

        std::cout << "\n";
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    std::cout << "All segments saved to: "
              << Config::DOWNLOAD_DIR << "\n";

    return 0;
}
