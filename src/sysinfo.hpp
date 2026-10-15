#pragma once

#include "config.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>

namespace axbox {

struct CmmInfo {
    uint64_t total_kb = 0;
    uint64_t used_kb = 0;
    uint64_t free_kb = 0;
    bool valid = false;
};

// 读取 /proc/ax_proc/mem_cmm_info，非 AX 设备返回 valid=false
CmmInfo read_cmm();

struct MemInfo {
    uint64_t total_kb = 0;
    uint64_t available_kb = 0;
};
MemInfo read_meminfo();

/**
 * 设备信息与实时状态
 *
 * 后台线程每隔 interval 采样一次（CPU、内存、CMM、NPU、温度、存储），保留最近 history_len 个点。
 */
class SysInfo {
public:
    SysInfo(const Config& config, int interval_ms = 2000, size_t history_len = 300);
    ~SysInfo();

    void start();
    void stop();

    nlohmann::json device() const;                   // 基本不变的设备信息
    nlohmann::json current() const;                  // 最近一次采样
    nlohmann::json history(int64_t since_ms) const;  // since_ms 之后的采样（unix 毫秒）

    // NPU 利用率统计：临时开启，到时间或没人查看时自动关闭；只关闭自己打开的
    nlohmann::json npu_top() const;
    std::string set_npu_top(bool enabled);
    void touch();  // 有人在查看状态

private:
    nlohmann::json sample();
    void loop();

    const Config& config_;
    int interval_ms_;
    size_t history_len_;

    mutable std::mutex mutex_;
    std::deque<nlohmann::json> history_;
    nlohmann::json device_;

    uint64_t prev_total_ = 0, prev_idle_ = 0;
    void npu_top_check();
    bool npu_top_by_us_ = false;
    int64_t npu_top_until_ms_ = 0;
    std::atomic<int64_t> last_view_ms_{0};

    std::mutex stop_mutex_;
    std::condition_variable stop_cv_;
    bool stop_ = false;
    std::thread thread_;
};

}  // namespace axbox
