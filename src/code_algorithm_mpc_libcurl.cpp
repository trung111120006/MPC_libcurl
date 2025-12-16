#include <bits/stdc++.h>
#include <curl/curl.h>
using namespace std;

// Callback để ghi dữ liệu download vào buffer
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    string *buffer = (string *)userp;
    buffer->append((char *)contents, totalSize);
    return totalSize;
}

// Hàm đo tốc độ download thực tế
double measure_download_speed(CURL *curl, const string &url, double &chunk_size)
{
    string buffer;
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.91.0");
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(
        curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

    auto start = chrono::high_resolution_clock::now();
    CURLcode res = curl_easy_perform(curl);
    auto end = chrono::high_resolution_clock::now();

    if (res != CURLE_OK)
    {
        cerr << "curl_easy_perform() failed: " << curl_easy_getinfo << endl;
        return -1.0;
    }

    double download_time = chrono::duration<double>(end - start).count();
    chunk_size = buffer.size() / 1000.0;                 // KB
    double bandwidth = (chunk_size * 8) / download_time; // kbps

    return bandwidth;
}

// Hàm tạo tất cả tổ hợp chunk
void Combine_Chunk(int P, string combo, vector<string> &CHUNK_COMBO_OPTIONS)
{
    if (combo.length() == P)
    {
        CHUNK_COMBO_OPTIONS.push_back(combo);
        return;
    }
    for (char i = '0'; i <= '2'; i++)
    {
        combo.push_back(i);
        Combine_Chunk(P, combo, CHUNK_COMBO_OPTIONS);
        combo.pop_back();
    }
}

double mpc(vector<double> &past_bandwidth,
           vector<double> &past_bandwidth_ests,
           vector<double> &past_errors,
           vector<double> &all_future_chunks_size,
           int P,
           double buffer_size,
           int video_chunk_remain,
           int last_quality)
{
    // Tính error hiện tại
    double curr_error = 0.0;
    if (!past_bandwidth_ests.empty() && !past_bandwidth.empty())
    {
        curr_error = abs(past_bandwidth_ests.back() - past_bandwidth.back()) /
                     max(past_bandwidth.back(), 1e-6);
    }
    past_errors.push_back(curr_error);

    // Harmonic mean của 5 mẫu băng thông gần nhất
    int use_sample = min((int)past_bandwidth.size(), 5);
    double bandwidth_sum = 0.0;
    for (int i = past_bandwidth.size() - use_sample; i < past_bandwidth.size(); i++)
    {
        bandwidth_sum += 1.0 / max(past_bandwidth[i], 1e-6);
    }
    double harmonic_bandwidth = use_sample / bandwidth_sum;

    // Lấy max error từ 5 lần dự đoán gần nhất
    int error_sample = min((int)past_errors.size(), 5);
    double max_error = 0.0;
    if (error_sample > 0)
    {
        auto start = past_errors.end() - error_sample;
        max_error = *max_element(start, past_errors.end());
    }

    // Dự đoán băng thông tương lai (conservative estimate)
    double future_bandwidth = harmonic_bandwidth / (1.0 + max_error);
    future_bandwidth = max(future_bandwidth, 1e-6);
    past_bandwidth_ests.push_back(future_bandwidth);

    // Tạo tất cả combo
    vector<string> CHUNK_COMBO_OPTIONS;
    Combine_Chunk(P, "", CHUNK_COMBO_OPTIONS);

    double max_reward = -1e18;
    string best_combo = "";
    double start_buffer = buffer_size;

    // Thử tất cả combo để tìm reward tối ưu
    for (const string &chunk_combo : CHUNK_COMBO_OPTIONS)
    {
        double curr_rebuffer_time = 0.0;
        double curr_buffer = start_buffer;
        double bitrate_sum = 0.0;
        double smoothness_dif = 0.0;
        int tmp_last_quality = last_quality;

        for (int chunk_index = 0; chunk_index < chunk_combo.length(); chunk_index++)
        {
            int chunk_quality = chunk_combo[chunk_index] - '0';
            int idx = (video_chunk_remain + chunk_index) * 3 + chunk_quality;

            if (idx >= all_future_chunks_size.size())
                continue;

            double download_time = all_future_chunks_size[idx] / future_bandwidth;

            if (curr_buffer < download_time)
            {
                curr_rebuffer_time += download_time - curr_buffer;
                curr_buffer = 0.0;
            }
            else
            {
                curr_buffer -= download_time;
            }

            curr_buffer += 1.0;                       // chunk duration = 1 giây
            bitrate_sum += (chunk_quality + 1) * 5.0; // 5, 10, 15 Mbps
            smoothness_dif += abs(chunk_quality - tmp_last_quality) * 5.0;

            tmp_last_quality = chunk_quality;
        }

        // Hàm reward: bitrate cao, rebuffer thấp, smooth transitions
        double reward = bitrate_sum / 1000.0 - 4.3 * curr_rebuffer_time - smoothness_dif / 1000.0;

        if (reward > max_reward)
        {
            max_reward = reward;
            best_combo = chunk_combo;
        }
    }

    int bit_rate = last_quality;
    if (!best_combo.empty())
    {
        bit_rate = best_combo[0] - '0';
    }
    return bit_rate;
}

int main()
{
    // Khởi tạo CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        cerr << "Failed to initialize CURL" << endl;
        return 1;
    }

    // Cấu hình streaming
    string base_url = "https://192.168.101.17:8443/trim_and_convert.sh";
    vector<string> quality_suffix = {"_low.mp4", "_medium.mp4", "_high.mp4"};

    // State variables
    vector<double> past_bandwidth = {1000.0}; // kbps, khởi tạo
    vector<double> past_bandwidth_ests;
    vector<double> past_errors;
    vector<double> all_future_chunks_size(100 * 3, 1000.0); // giả định 100 chunks

    int P = 5;                // lookahead horizon
    double buffer_size = 3.0; // giây
    int last_quality = 1;     // bắt đầu từ medium quality
    int video_chunk_remain = 0;
    int total_segments = 20; // tổng số segment cần tải

    cout << "===== MPC Adaptive Bitrate Streaming =====" << endl;
    cout << "Lookahead: " << P << " chunks" << endl;
    cout << "Initial buffer: " << buffer_size << " seconds" << endl
         << endl;

    for (int seg = 0; seg < total_segments; seg++)
    {
        cout << "===== Segment " << seg + 1 << " =====" << endl;

        // Gọi MPC để quyết định quality cho chunk tiếp theo
        int next_quality = (int)mpc(past_bandwidth, past_bandwidth_ests, past_errors,
                                    all_future_chunks_size, P, buffer_size,
                                    video_chunk_remain, last_quality);

        cout << "MPC Decision: Quality " << next_quality
             << " (" << (next_quality + 1) * 5 << " Mbps)" << endl;

        // Tạo URL cho chunk với quality đã chọn
        // string chunk_url = base_url + to_string(seg) + quality_suffix[next_quality];
        // cout << "Downloading: " << chunk_url << endl;
        cout << "Downloading: " << base_url << endl;
        // Download chunk và đo băng thông thực tế
        double chunk_size_kb;
        double measured_bandwidth = measure_download_speed(curl, base_url, chunk_size_kb);

        if (measured_bandwidth < 0)
        {
            cout << "Download failed, using previous bandwidth estimate" << endl;
            measured_bandwidth = past_bandwidth.back();
        }
        else
        {
            cout << "Measured bandwidth: " << measured_bandwidth << " kbps" << endl;
            cout << "Chunk size: " << chunk_size_kb << " KB" << endl;
            past_bandwidth.push_back(measured_bandwidth);
        }

        // Cập nhật buffer (giả định chunk duration
        double download_time = chunk_size_kb * 8 / measured_bandwidth;
        buffer_size = max(0.0, buffer_size - download_time) + 1.0;
        cout << "Buffer level: " << buffer_size << " seconds" << endl;

        // In thông tin dự đoán
        if (!past_bandwidth_ests.empty())
        {
            cout << "Bandwidth estimate: " << past_bandwidth_ests.back() << " kbps" << endl;
        }
        if (!past_errors.empty())
        {
            cout << "Prediction error: " << past_errors.back() * 100 << "%" << endl;
        }

        // Cập nhật state
        video_chunk_remain++;
        last_quality = next_quality;

        cout << endl;
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    cout << "===== Streaming Statistics =====" << endl;
    cout << "Average bandwidth: "
         << accumulate(past_bandwidth.begin(), past_bandwidth.end(), 0.0) / past_bandwidth.size()
         << " kbps" << endl;
    cout << "Average prediction error: "
         << accumulate(past_errors.begin(), past_errors.end(), 0.0) / past_errors.size() * 100
         << "%" << endl;

    return 0;
}