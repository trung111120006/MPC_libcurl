// đây là file xử lý các hàm giải mã segment video từ file nhị phân sang định dạng video chuẩn.
#ifndef DECODER_H
#define DECODER_H

#include <string>

namespace Decoder {
    // Decode segment async (background hoặc foreground)
    bool decodeSegmentAsync(const std::string& binFilePath, 
                           int segmentNum, 
                           const std::string& qualityName);
    
    // Kiểm tra decode script có tồn tại không
    bool checkDecodeScript();
}

#endif