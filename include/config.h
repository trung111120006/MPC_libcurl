// Đây là file quản lý của cấu hình của dự án 
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

namespace Config {
    // Directories
    extern const std::string DOWNLOAD_DIR;
    extern const std::string DECODED_OUTPUT_DIR;
    extern const std::string DECODE_SCRIPT_PATH;
    
    // Decode options
    extern const bool DECODE_REALTIME;
    extern const bool DECODE_BACKGROUND;
    
    // -===== Server / MPD url ======
    extern const std::string SERVER_BASE_URL;
    extern const std::string MPD_URL;
    
    // MPC parameters
    extern const int LOOKAHEAD_P;
    extern const double INITIAL_BUFFER_SIZE;
    extern const double REBUFFER_PENALTY;
    
    // Quality names
    extern const std::vector<std::string> QUALITY_NAMES;
        
    void init();
}

#endif