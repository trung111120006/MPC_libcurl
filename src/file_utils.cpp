#include "file_utils.h"
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>

namespace FileUtils {
    bool exists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }
    
    bool createDirectory(const std::string& path) {
        if (exists(path)) return true;
        std::string cmd = "mkdir -p " + path;
        return system(cmd.c_str()) == 0;
    }
    
    double getFileSize(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            return st.st_size / 1024.0; // KB
        }
        return -1.0;
    }
    
    bool renameFile(const std::string& oldPath, const std::string& newPath) {
        return rename(oldPath.c_str(), newPath.c_str()) == 0;
    }
    
    bool deleteFile(const std::string& path) {
        return remove(path.c_str()) == 0;
    }
}