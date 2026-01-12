#include "curl_helper.h"
#include <iostream>
#include <chrono>
#include <cstdio>

namespace CurlHelper {
    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* buffer = (std::string*)userp;
        buffer->append((char*)contents, totalSize);
        return totalSize;
    }
    
    size_t writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        FILE* fp = (FILE*)userp;
        return fwrite(contents, size, nmemb, fp);
    }
    
    CURL* initCurl() {
        return curl_easy_init();
    }
    
    void setupCurlOptions(CURL* curl, const std::string& url) {
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.91.0");
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    }
    
    std::string getFileExtension(CURL* curl, const std::string& url) {
        size_t lastDot = url.find_last_of(".");
        size_t lastSlash = url.find_last_of("/");
        
        if (lastDot != std::string::npos && lastDot > lastSlash) {
            std::string ext = url.substr(lastDot);
            size_t queryPos = ext.find("?");
            if (queryPos != std::string::npos) {
                ext = ext.substr(0, queryPos);
            }
            return ext;
        }
        
        char* contentType = nullptr;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
        
        if (contentType) {
            std::string ct(contentType);
            if (ct.find("text/plain") != std::string::npos) return ".txt";
            if (ct.find("application/octet-stream") != std::string::npos) return ".bin";
            if (ct.find("video/mp4") != std::string::npos) return ".mp4";
        }
        
        return ".dat";
    }
    
    double measureBandwidth(CURL* curl, const std::string& url, double& chunkSize) {
        std::string buffer;
        setupCurlOptions(curl, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        
        auto start = std::chrono::high_resolution_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end = std::chrono::high_resolution_clock::now();
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return -1.0;
        }
        
        double downloadTime = std::chrono::duration<double>(end - start).count();
        chunkSize = buffer.size() / 1000.0; // KB
        double bandwidth = (chunkSize * 8) / downloadTime; // kbps
        
        return bandwidth;
    }
    
    bool downloadFile(CURL* curl, const std::string& url, 
                      const std::string& savePath, std::string& extension) {
        FILE* fp = fopen(savePath.c_str(), "wb");
        if (!fp) {
            std::cerr << "Cannot open file for writing: " << savePath << std::endl;
            return false;
        }
        
        setupCurlOptions(curl, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        
        CURLcode res = curl_easy_perform(curl);
        fclose(fp);
        
        if (res != CURLE_OK) {
            std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;
            remove(savePath.c_str());
            return false;
        }
        
        extension = getFileExtension(curl, url);
        std::cout << "✓ File saved to: " << savePath << std::endl;
        std::cout << "  Detected format: " << extension << std::endl;
        return true;
    }
}