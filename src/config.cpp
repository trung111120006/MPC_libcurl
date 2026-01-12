#include "config.h"

namespace Config {
    const std::string DOWNLOAD_DIR = "/home/backne/Trung/code/code_mpc/MPC_libcurl/download";
    const std::string DECODED_OUTPUT_DIR = "/home/backne/Trung/code/code_mpc/MPC_libcurl/decoded_output";
    const std::string DECODE_SCRIPT_PATH = "/home/backne/Trung/code/code_mpc/MPC_libcurl/auto_decode.sh";
    
    const bool DECODE_REALTIME = true;
    const bool DECODE_BACKGROUND = true;
    
    const std::string BASE_URL = "https://100.66.2.42:8443/longdress_test2.bin";
    const std::vector<std::string> QUALITY_SUFFIX = {"_low", "_medium", "_high"};
    const std::vector<std::string> QUALITY_NAMES = {"LOW", "MEDIUM", "HIGH"};
    
    const int LOOKAHEAD_P = 5;
    const double INITIAL_BUFFER_SIZE = 1.0;
    const double REBUFFER_PENALTY = 10.0;
    const int TOTAL_SEGMENTS = 10;
    
    const std::vector<double> CHUNK_SIZES = {625.0, 1250.0, 1875.0};
    
    void init() {
        // Initialization logic nếu cần
    }
}