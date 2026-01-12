// Đây là file xử lý các file system opreations như kiểm tra tồn tại, tạo thư mục, lấy kích thước file, đổi tên và xóa file.
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

namespace FileUtils {
    // Kiểm tra file/folder tồn tại
    bool exists(const std::string& path);
    
    // Tạo thư mục (recursive)
    bool createDirectory(const std::string& path);
    
    // Lấy kích thước file (KB)
    double getFileSize(const std::string& path);
    
    // Đổi tên file
    bool renameFile(const std::string& oldPath, const std::string& newPath);
    
    // Xóa file
    bool deleteFile(const std::string& path);
}

#endif