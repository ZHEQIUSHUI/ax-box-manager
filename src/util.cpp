#include "util.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

namespace axbox {

// ---------------------------------------------------------------- 日志

namespace {
std::mutex g_log_mutex;
FILE* g_log_file = nullptr;

void log_write(const char* level, const std::string& msg) {
    char ts[32];
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);
    std::lock_guard<std::mutex> lock(g_log_mutex);
    fprintf(stderr, "%s [%s] %s\n", ts, level, msg.c_str());
    if (g_log_file) {
        fprintf(g_log_file, "%s [%s] %s\n", ts, level, msg.c_str());
        fflush(g_log_file);
    }
}
}  // namespace

void log_set_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_file) fclose(g_log_file);
    g_log_file = fopen(path.c_str(), "a");
}

void log_info(const std::string& msg) { log_write("INFO", msg); }
void log_warn(const std::string& msg) { log_write("WARN", msg); }
void log_error(const std::string& msg) { log_write("ERROR", msg); }

// ---------------------------------------------------------------- 文件

std::optional<std::string> read_file(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return std::nullopt;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

bool write_file_atomic(const std::string& path, const std::string& content) {
    const std::string tmp = path + ".tmp";
    {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) return false;
        ofs << content;
        if (!ofs) return false;
    }
    return rename(tmp.c_str(), path.c_str()) == 0;
}

bool path_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool is_dir(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool make_dirs(const std::string& path) {
    if (path.empty()) return false;
    std::string cur;
    for (size_t i = 0; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            if (!cur.empty() && !is_dir(cur)) {
                if (mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) return false;
            }
        }
        if (i < path.size()) cur += path[i];
    }
    return is_dir(path);
}

bool remove_tree(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return errno == ENOENT;
    if (S_ISDIR(st.st_mode)) {
        for (const auto& name : list_dir(path)) {
            if (!remove_tree(join_path(path, name))) return false;
        }
        return rmdir(path.c_str()) == 0;
    }
    return unlink(path.c_str()) == 0;
}

std::vector<std::string> list_dir(const std::string& path) {
    std::vector<std::string> out;
    DIR* d = opendir(path.c_str());
    if (!d) return out;
    while (auto* e = readdir(d)) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        out.emplace_back(e->d_name);
    }
    closedir(d);
    return out;
}

uint64_t dir_size(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return 0;
    if (S_ISLNK(st.st_mode)) {
        // 指向文件的软链接按目标大小计（模型目录可能用软链接引用共享存储上的权重），不跟随目录链接
        struct stat target;
        return stat(path.c_str(), &target) == 0 && S_ISREG(target.st_mode) ? static_cast<uint64_t>(target.st_size) : 0;
    }
    if (!S_ISDIR(st.st_mode)) return S_ISREG(st.st_mode) ? static_cast<uint64_t>(st.st_size) : 0;
    uint64_t total = 0;
    for (const auto& name : list_dir(path)) total += dir_size(join_path(path, name));
    return total;
}

std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    out.push_back(cur);
    return out;
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string tail_file(const std::string& path, size_t max_lines, size_t max_bytes) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return "";
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    long start = size > static_cast<long>(max_bytes) ? size - static_cast<long>(max_bytes) : 0;
    fseek(f, start, SEEK_SET);
    std::string data(static_cast<size_t>(size - start), '\0');
    size_t n = fread(&data[0], 1, data.size(), f);
    fclose(f);
    data.resize(n);
    if (start > 0) {
        auto nl = data.find('\n');
        if (nl != std::string::npos) data.erase(0, nl + 1);
    }
    size_t lines = 0;
    size_t pos = data.size();
    if (pos > 0 && data[pos - 1] == '\n') --pos;
    while (pos > 0) {
        if (data[pos - 1] == '\n' && ++lines >= max_lines) break;
        --pos;
    }
    return data.substr(pos);
}

bool is_safe_name(const std::string& name) {
    if (name.empty() || name.size() > 128 || name[0] == '.') return false;
    for (char c : name) {
        if (!(isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

// ---------------------------------------------------------------- 其他

std::string random_hex(size_t bytes) {
    std::string raw(bytes, '\0');
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    size_t got = 0;
    if (fd >= 0) {
        while (got < bytes) {
            ssize_t n = read(fd, &raw[got], bytes - got);
            if (n <= 0) break;
            got += static_cast<size_t>(n);
        }
        close(fd);
    }
    if (got < bytes) abort();  // 拿不到随机数时不能生成会话令牌
    static const char* hex = "0123456789abcdef";
    std::string out;
    for (unsigned char c : raw) {
        out += hex[c >> 4];
        out += hex[c & 15];
    }
    return out;
}

int64_t now_unix() { return static_cast<int64_t>(time(nullptr)); }

int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

// ---------------------------------------------------------------- sha256

namespace {

struct Sha256 {
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint8_t buf[64];
    size_t buf_len = 0;
    uint64_t total = 0;

    static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

    void block(const uint8_t* p) {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (uint32_t(p[i * 4]) << 24) | (uint32_t(p[i * 4 + 1]) << 16) | (uint32_t(p[i * 4 + 2]) << 8) |
                   uint32_t(p[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    void update(const uint8_t* p, size_t n) {
        total += n;
        if (buf_len) {
            size_t take = std::min(n, 64 - buf_len);
            memcpy(buf + buf_len, p, take);
            buf_len += take;
            p += take;
            n -= take;
            if (buf_len == 64) {
                block(buf);
                buf_len = 0;
            }
        }
        while (n >= 64) {
            block(p);
            p += 64;
            n -= 64;
        }
        if (n) {
            memcpy(buf, p, n);
            buf_len = n;
        }
    }

    std::string hex() {
        uint64_t bits = total * 8;
        uint8_t pad = 0x80;
        update(&pad, 1);
        uint8_t zero = 0;
        while (buf_len != 56) update(&zero, 1);
        uint8_t len[8];
        for (int i = 0; i < 8; ++i) len[i] = uint8_t(bits >> (56 - 8 * i));
        update(len, 8);
        static const char* digits = "0123456789abcdef";
        std::string out;
        for (uint32_t v : h) {
            for (int i = 28; i >= 0; i -= 4) out += digits[(v >> i) & 15];
        }
        return out;
    }
};

}  // namespace

std::string sha256_file(const std::string& path, uint64_t* bytes_done) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return "";
    Sha256 s;
    std::vector<uint8_t> buf(1 << 20);
    size_t n;
    while ((n = fread(buf.data(), 1, buf.size(), f)) > 0) {
        s.update(buf.data(), n);
        if (bytes_done) *bytes_done += n;
    }
    bool ok = !ferror(f);
    fclose(f);
    return ok ? s.hex() : "";
}

std::string sha256_hex(const std::string& data) {
    Sha256 s;
    s.update(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return s.hex();
}

// ---------------------------------------------------------------- 子进程

std::optional<std::string> run_capture(const std::vector<std::string>& argv, int timeout_ms, size_t max_bytes,
                                       int* exit_code) {
    if (argv.empty()) return std::nullopt;
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) != 0) return std::nullopt;
    // fork 之后子进程里不能分配内存（多线程程序），参数提前准备好
    std::vector<char*> args;
    for (const auto& a : argv) args.push_back(const_cast<char*>(a.c_str()));
    args.push_back(nullptr);
    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return std::nullopt;
    }
    if (pid == 0) {
        dup2(fds[1], 1);
        dup2(fds[1], 2);
        int devnull = open("/dev/null", O_RDONLY);
        if (devnull >= 0) dup2(devnull, 0);
        execvp(args[0], args.data());
        _exit(127);
    }
    close(fds[1]);
    std::string out;
    const int64_t deadline = now_ms() + timeout_ms;
    bool timed_out = false;
    for (;;) {
        int64_t left = deadline - now_ms();
        if (left <= 0) {
            timed_out = true;
            break;
        }
        struct pollfd pfd = {fds[0], POLLIN, 0};
        int r = poll(&pfd, 1, static_cast<int>(left));
        if (r <= 0) {
            if (r == 0) timed_out = true;
            if (r < 0 && errno == EINTR) continue;
            break;
        }
        char buf[4096];
        ssize_t n = read(fds[0], buf, sizeof(buf));
        if (n <= 0) break;
        if (out.size() < max_bytes) out.append(buf, std::min(static_cast<size_t>(n), max_bytes - out.size()));
    }
    close(fds[0]);
    if (timed_out) kill(pid, SIGKILL);
    int status = 0;
    waitpid(pid, &status, 0);
    if (exit_code) *exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    if (timed_out) return std::nullopt;
    return out;
}

}  // namespace axbox
