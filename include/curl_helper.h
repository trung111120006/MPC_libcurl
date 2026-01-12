//đây là file sử lý các hàm hỗ trợ CURL như callback, khởi tạo CURL, đo bandwidth, download file, detect extension và setup options.
#ifndef CURL_HELPER_H
#define CURL_HELPER_H

#include <curl/curl.h>
#include <string>

namespace CurlHelper {
    // Callback functions
    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    size_t writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    // Khởi tạo CURL với options chuẩn
    CURL* initCurl();
    
    // Đo bandwidth và download
    double measureBandwidth(CURL* curl, const std::string& url, double& chunkSize);
    
    // Download và lưu file
    bool downloadFile(CURL* curl, const std::string& url, 
                      const std::string& savePath, std::string& extension);
    
    // Detect file extension từ URL hoặc Content-Type
    std::string getFileExtension(CURL* curl, const std::string& url);
    
    // Setup CURL options
    void setupCurlOptions(CURL* curl, const std::string& url);
}

#endif