#include "sysinfo.hpp"

#include "util.hpp"
#include "version.hpp"

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <chrono>
#include <algorithm>
#include <cstring>
#include <regex>
#include <sstream>

namespace axbox {

namespace {

uint64_t parse_kb_field(const std::string& text, const std::string& key) {
    // 例：total size=4718592KB(4608MB),used=274852KB(...)
    auto pos = text.find(key + "=");
    if (pos == std::string::npos) return 0;
    return std::strtoull(text.c_str() + pos + key.size() + 1, nullptr, 10);
}

std::string first_line(const std::string& s) {
    auto nl = s.find('\n');
    return trim(nl == std::string::npos ? s : s.substr(0, nl));
}

std::string read_trimmed(const std::string& path) {
    auto s = read_file(path);
    if (!s) return "";
    // device-tree 里的字符串以 \0 结尾
    std::string v = *s;
    v.erase(std::find(v.begin(), v.end(), '\0'), v.end());
    return trim(v);
}

std::string os_pretty_name() {
    auto s = read_file("/etc/os-release");
    if (!s) return "";
    for (const auto& line : split(*s, '\n')) {
        if (starts_with(line, "PRETTY_NAME=")) {
            std::string v = line.substr(12);
            if (v.size() >= 2 && v.front() == '"' && v.back() == '"') v = v.substr(1, v.size() - 2);
            return v;
        }
    }
    return "";
}

nlohmann::json network_interfaces() {
    nlohmann::json out = nlohmann::json::array();
    struct ifaddrs* ifa = nullptr;
    if (getifaddrs(&ifa) != 0) return out;
    std::map<std::string, nlohmann::json> by_name;
    for (auto* p = ifa; p; p = p->ifa_next) {
        if (!p->ifa_addr || (p->ifa_flags & IFF_LOOPBACK)) continue;
        std::string name = p->ifa_name;
        if (starts_with(name, "docker") || starts_with(name, "veth") || starts_with(name, "br-")) continue;
        auto& j = by_name[name];
        if (j.is_null()) {
            j = {{"name", name},
                 {"mac", read_trimmed("/sys/class/net/" + name + "/address")},
                 {"up", (p->ifa_flags & IFF_UP) != 0},
                 {"ipv4", nlohmann::json::array()}};
        }
        if (p->ifa_addr->sa_family == AF_INET) {
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(p->ifa_addr)->sin_addr, buf, sizeof(buf));
            j["ipv4"].push_back(buf);
        }
    }
    freeifaddrs(ifa);
    for (auto& kv : by_name) {
        if (!kv.second["ipv4"].empty()) out.push_back(kv.second);
    }
    return out;
}

std::string runtime_version(const std::string& bin) {
    if (!path_exists(bin)) return "";
    int code = -1;
    auto out = run_capture({bin, "--version"}, 3000, 4096, &code);
    if (!out || code != 0) return "";
    // 优先取带数字的版本行；axllm 输出的是 "git commit : xxx" 和 "build time : xxx"
    std::string commit, built;
    for (const auto& raw : split(*out, '\n')) {
        const std::string line = trim(raw);
        auto colon = line.find(':');
        const std::string key = colon == std::string::npos ? line : trim(line.substr(0, colon));
        const std::string val = colon == std::string::npos ? "" : trim(line.substr(colon + 1));
        if (key == "git commit") commit = val;
        else if (key == "build time") built = val;
        else if (key == "version" && !val.empty()) return val;
    }
    if (!commit.empty()) return commit + (built.empty() ? "" : " (" + built + ")");
    return first_line(*out);
}

// NPU 利用率：/proc/ax_proc/npu/top（需要先往 enable 写 1）里每个核一行百分比，取平均
std::optional<double> read_npu_util() {
    auto s = read_file("/proc/ax_proc/npu/top");
    if (!s || s->find("not enabled") != std::string::npos) return std::nullopt;
    if (s->find("is empty") != std::string::npos) return 0.0;
    static const std::regex pct(R"(([0-9]+(?:\.[0-9]+)?)\s*%)");
    double sum = 0;
    int n = 0;
    for (std::sregex_iterator it(s->begin(), s->end(), pct), end; it != end; ++it) {
        sum += std::stod((*it)[1].str());
        ++n;
    }
    if (n == 0) return 0.0;
    return sum / n;
}

nlohmann::json read_temperatures() {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& name : list_dir("/sys/class/thermal")) {
        if (!starts_with(name, "thermal_zone")) continue;
        const std::string dir = "/sys/class/thermal/" + name;
        auto temp = read_trimmed(dir + "/temp");
        if (temp.empty()) continue;
        out.push_back({{"name", read_trimmed(dir + "/type")}, {"celsius", std::stod(temp) / 1000.0}});
    }
    return out;
}

std::optional<int> read_fan_rpm() {
    for (const auto& hw : list_dir("/sys/class/hwmon")) {
        auto rpm = read_trimmed("/sys/class/hwmon/" + hw + "/fan1_input");
        if (!rpm.empty()) return std::stoi(rpm);
    }
    return std::nullopt;
}

const char* kNpuEnable = "/proc/ax_proc/npu/enable";

bool write_npu_enable(bool on) {
    FILE* f = fopen(kNpuEnable, "w");
    if (!f) return false;
    bool ok = fputs(on ? "1\n" : "0\n", f) >= 0;  // 驱动要求带换行
    ok = (fclose(f) == 0) && ok;
    return ok;
}

}  // namespace

CmmInfo read_cmm() {
    CmmInfo info;
    auto s = read_file("/proc/ax_proc/mem_cmm_info");
    if (!s) return info;
    // 汇总行在 ---CMM_USE_INFO: 之后
    auto pos = s->find("CMM_USE_INFO");
    std::string summary = pos == std::string::npos ? *s : s->substr(pos);
    info.total_kb = parse_kb_field(summary, "total size");
    info.used_kb = parse_kb_field(summary, "used");
    info.free_kb = parse_kb_field(summary, "remain");
    info.valid = info.total_kb > 0;
    return info;
}

MemInfo read_meminfo() {
    MemInfo m;
    auto s = read_file("/proc/meminfo");
    if (!s) return m;
    for (const auto& line : split(*s, '\n')) {
        if (starts_with(line, "MemTotal:")) m.total_kb = std::strtoull(line.c_str() + 9, nullptr, 10);
        if (starts_with(line, "MemAvailable:")) m.available_kb = std::strtoull(line.c_str() + 13, nullptr, 10);
    }
    return m;
}

SysInfo::SysInfo(const Config& config, int interval_ms, size_t history_len)
    : config_(config), interval_ms_(interval_ms), history_len_(history_len) {}

SysInfo::~SysInfo() { stop(); }

void SysInfo::start() {
    // 设备信息
    nlohmann::json d;
    char host[256] = {0};
    gethostname(host, sizeof(host) - 1);
    d["hostname"] = host;
    d["board"] = read_trimmed("/proc/ax_proc/board_id");
    if (d["board"].get<std::string>().empty()) d["board"] = read_trimmed("/proc/device-tree/model");
    d["chip"] = read_trimmed("/proc/ax_proc/chip_type");
    {
        // ax_uid: 0x6013101941419107
        std::string uid = read_trimmed("/proc/ax_proc/uid");
        auto colon = uid.find(':');
        d["serial"] = colon == std::string::npos ? uid : trim(uid.substr(colon + 1));
    }
    d["os"] = os_pretty_name();
    {
        auto v = read_file("/proc/sys/kernel/osrelease");
        d["kernel"] = v ? trim(*v) : "";
    }
    d["cpu_cores"] = sysconf(_SC_NPROCESSORS_ONLN);
    d["sdk_version"] = read_trimmed("/proc/ax_proc/version");
    {
        std::string npu = read_trimmed("/proc/ax_proc/npu/version");
        static const std::regex ver(R"(ax_npu\s+(V[0-9.]+))");
        std::smatch m;
        d["npu_version"] = std::regex_search(npu, m, ver) ? m[1].str() : "";
    }
    d["platform_version"] = kVersion;
    nlohmann::json runtimes = nlohmann::json::object();
    for (const auto& kv : config_.runtimes) {
        runtimes[kv.first] = {{"path", kv.second},
                              {"installed", path_exists(kv.second)},
                              {"version", runtime_version(kv.second)}};
    }
    d["runtimes"] = runtimes;
    auto mem = read_meminfo();
    d["mem_total_kb"] = mem.total_kb;
    auto cmm = read_cmm();
    d["cmm_total_kb"] = cmm.valid ? nlohmann::json(cmm.total_kb) : nlohmann::json(nullptr);
    d["npu_util_available"] = path_exists(kNpuEnable);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        device_ = d;
    }

    sample();  // 初始化 CPU 计数
    stop_ = false;
    thread_ = std::thread([this] { loop(); });
}

void SysInfo::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (npu_top_by_us_) {
            write_npu_enable(false);
            npu_top_by_us_ = false;
        }
    }
    {
        std::lock_guard<std::mutex> lock(stop_mutex_);
        stop_ = true;
    }
    stop_cv_.notify_all();
    if (thread_.joinable()) thread_.join();
}

nlohmann::json SysInfo::sample() {
    nlohmann::json s;
    s["t"] = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                      .count());
    // CPU
    if (auto stat = read_file("/proc/stat")) {
        std::istringstream ss(first_line(*stat));
        std::string cpu;
        uint64_t v, total = 0, idle = 0;
        ss >> cpu;
        for (int i = 0; ss >> v; ++i) {
            total += v;
            if (i == 3 || i == 4) idle += v;  // idle + iowait
        }
        if (prev_total_ && total > prev_total_) {
            s["cpu"] = 100.0 * (1.0 - double(idle - prev_idle_) / double(total - prev_total_));
        }
        prev_total_ = total;
        prev_idle_ = idle;
    }
    auto mem = read_meminfo();
    s["mem_total_kb"] = mem.total_kb;
    s["mem_used_kb"] = mem.total_kb - mem.available_kb;
    auto cmm = read_cmm();
    if (cmm.valid) {
        s["cmm_total_kb"] = cmm.total_kb;
        s["cmm_used_kb"] = cmm.used_kb;
    }
    if (read_trimmed(kNpuEnable) == "1") {
        if (auto u = read_npu_util()) s["npu"] = *u;
    }
    auto temps = read_temperatures();
    if (!temps.empty()) {
        double max_t = 0;
        for (const auto& t : temps) max_t = std::max(max_t, t["celsius"].get<double>());
        s["temp"] = max_t;
        s["temps"] = temps;
    }
    if (auto rpm = read_fan_rpm()) s["fan_rpm"] = *rpm;
    struct statvfs vfs;
    if (statvfs(config_.models_dir.c_str(), &vfs) == 0) {
        s["disk_total_kb"] = uint64_t(vfs.f_blocks) * vfs.f_frsize / 1024;
        s["disk_free_kb"] = uint64_t(vfs.f_bavail) * vfs.f_frsize / 1024;
    }
    struct sysinfo si;
    if (::sysinfo(&si) == 0) {
        s["uptime_s"] = si.uptime;
        s["load1"] = si.loads[0] / 65536.0;
    }
    return s;
}

nlohmann::json SysInfo::npu_top() const {
    std::lock_guard<std::mutex> lock(mutex_);
    const bool available = path_exists(kNpuEnable);
    const bool enabled = available && read_trimmed(kNpuEnable) == "1";
    nlohmann::json j = {{"available", available}, {"enabled", enabled}, {"by_us", npu_top_by_us_ && enabled}};
    if (npu_top_by_us_ && enabled) {
        j["auto_off_in_s"] = std::max<int64_t>(0, (npu_top_until_ms_ - now_ms()) / 1000);
    }
    return j;
}

std::string SysInfo::set_npu_top(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!path_exists(kNpuEnable)) return "NPU statistics are not available on this device";
    const bool on = read_trimmed(kNpuEnable) == "1";
    if (enabled) {
        if (!on) {
            if (!write_npu_enable(true)) return std::string("cannot enable NPU statistics: ") + strerror(errno);
            npu_top_by_us_ = true;
            log_info("NPU statistics enabled from the web console");
        }
        // 已经由别人打开时不接管，不会自动关掉
        if (npu_top_by_us_) npu_top_until_ms_ = now_ms() + int64_t(config_.npu_top_minutes) * 60 * 1000;
        last_view_ms_ = now_ms();
    } else if (on) {
        if (!write_npu_enable(false)) return std::string("cannot disable NPU statistics: ") + strerror(errno);
        npu_top_by_us_ = false;
        log_info("NPU statistics disabled from the web console");
    }
    return "";
}

void SysInfo::touch() { last_view_ms_ = now_ms(); }

void SysInfo::npu_top_check() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!npu_top_by_us_) return;
    if (read_trimmed(kNpuEnable) != "1") {  // 被别人关掉了
        npu_top_by_us_ = false;
        return;
    }
    const int64_t now = now_ms();
    const bool expired = now >= npu_top_until_ms_;
    const bool unwatched = now - last_view_ms_ > 60 * 1000;
    if (expired || unwatched) {
        write_npu_enable(false);
        npu_top_by_us_ = false;
        log_info(std::string("NPU statistics turned off automatically (") + (expired ? "time limit" : "nobody watching") + ")");
    }
}

void SysInfo::loop() {
    for (;;) {
        npu_top_check();
        auto s = sample();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            history_.push_back(std::move(s));
            while (history_.size() > history_len_) history_.pop_front();
        }
        std::unique_lock<std::mutex> lock(stop_mutex_);
        if (stop_cv_.wait_for(lock, std::chrono::milliseconds(interval_ms_), [this] { return stop_; })) return;
    }
}

nlohmann::json SysInfo::device() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json d = device_;
    d["network"] = network_interfaces();
    return d;
}

nlohmann::json SysInfo::current() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return history_.empty() ? nlohmann::json::object() : history_.back();
}

nlohmann::json SysInfo::history(int64_t since_ms) const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json out = nlohmann::json::array();
    for (const auto& s : history_) {
        if (s["t"].get<int64_t>() > since_ms) {
            nlohmann::json p = s;
            p.erase("temps");
            out.push_back(std::move(p));
        }
    }
    return out;
}

}  // namespace axbox
