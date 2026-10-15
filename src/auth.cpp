#include "auth.hpp"

#include "util.hpp"

#include <dlfcn.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <vector>

namespace axbox {

namespace {

constexpr int kMaxFailures = 5;            // 连续失败次数
constexpr int kFailureWindowS = 10 * 60;   // 统计窗口
constexpr int kLockS = 5 * 60;             // 锁定时长

using crypt_fn = char* (*)(const char*, const char*);

crypt_fn load_crypt() {
    // 按运行时加载，交叉编译时不需要目标机的 libcrypt 开发包
    static crypt_fn fn = [] () -> crypt_fn {
        for (const char* lib : {"libcrypt.so.1", "libcrypt.so.2", "libcrypt.so", "libc.so.6", "libc.so"}) {
            void* h = dlopen(lib, RTLD_NOW);
            if (!h) continue;
            if (auto f = reinterpret_cast<crypt_fn>(dlsym(h, "crypt"))) return f;
        }
        return nullptr;
    }();
    return fn;
}

// 常数时间比较，避免按比较耗时猜测哈希
bool equal_ct(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

}  // namespace

Auth::Auth(const Config& config) : config_(config) {}

static std::string sessions_path(const Config& c) { return join_path(c.data_dir, "sessions.json"); }

void Auth::load_sessions() {
    auto text = read_file(sessions_path(config_));
    if (!text) return;
    auto j = nlohmann::json::parse(*text, nullptr, false);
    if (!j.is_object()) return;
    const int64_t now = now_unix();
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = j.begin(); it != j.end(); ++it) {
        const auto& v = it.value();
        if (!v.is_object()) continue;
        Session s{v.value("user", ""), v.value("expires_at", int64_t(0))};
        if (s.expires_at > now && !s.user.empty()) sessions_[it.key()] = s;
    }
}

void Auth::save_sessions() {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& kv : sessions_) j[kv.first] = {{"user", kv.second.user}, {"expires_at", kv.second.expires_at}};
    const std::string path = sessions_path(config_);
    if (write_file_atomic(path, j.dump())) chmod(path.c_str(), 0600);
}

bool Auth::user_allowed(const std::string& user) const {
    if (std::find(config_.allowed_users.begin(), config_.allowed_users.end(), user) != config_.allowed_users.end()) {
        return true;
    }
    if (config_.allowed_groups.empty()) return false;
    struct passwd* pw = getpwnam(user.c_str());
    if (!pw) return false;
    int ngroups = 64;
    std::vector<gid_t> groups(ngroups);
    if (getgrouplist(user.c_str(), pw->pw_gid, groups.data(), &ngroups) < 0) {
        groups.resize(ngroups);
        getgrouplist(user.c_str(), pw->pw_gid, groups.data(), &ngroups);
    }
    groups.resize(ngroups);
    for (gid_t g : groups) {
        struct group* gr = getgrgid(g);
        if (!gr) continue;
        if (std::find(config_.allowed_groups.begin(), config_.allowed_groups.end(), gr->gr_name) !=
            config_.allowed_groups.end()) {
            return true;
        }
    }
    return false;
}

bool Auth::verify_password(const std::string& user, const std::string& password, std::string* error) {
    auto shadow = read_file("/etc/shadow");
    if (!shadow) {
        *error = "cannot read /etc/shadow (the manager must run as root)";
        return false;
    }
    std::string hash;
    bool found = false;
    for (const auto& line : split(*shadow, '\n')) {
        auto fields = split(line, ':');
        if (fields.size() >= 2 && fields[0] == user) {
            hash = fields[1];
            found = true;
            break;
        }
    }
    if (!found || hash.empty() || hash[0] == '!' || hash[0] == '*') return false;  // 无此用户、无密码或已锁定
    auto fn = load_crypt();
    if (!fn) {
        *error = "crypt() not available on this system";
        return false;
    }
    std::lock_guard<std::mutex> lock(crypt_mutex_);  // crypt() 不可重入
    const char* out = fn(password.c_str(), hash.c_str());
    return out && equal_ct(out, hash);
}

Auth::Result Auth::login(const std::string& user, const std::string& password, const std::string& client_ip,
                         std::string* token, int* retry_after_s) {
    const int64_t now = now_unix();
    const std::string key = user + "@" + client_ip;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& f = failures_[key];
        if (f.locked_until > now) {
            *retry_after_s = static_cast<int>(f.locked_until - now);
            return Result::Locked;
        }
    }

    bool ok = false;
    if (!enabled()) {
        ok = true;
    } else {
        std::string error;
        ok = user_allowed(user) && verify_password(user, password, &error);
        if (!error.empty()) {
            log_error("login: " + error);
            return Result::Unavailable;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (!ok) {
        auto& f = failures_[key];
        if (now - f.first_at > kFailureWindowS) {
            f.count = 0;
            f.first_at = now;
        }
        if (++f.count >= kMaxFailures) {
            f.locked_until = now + kLockS;
            f.count = 0;
            log_warn("login: too many failures for " + key + ", locked for " + std::to_string(kLockS) + "s");
        }
        return Result::BadCredentials;
    }
    failures_.erase(key);
    // 清理过期会话
    for (auto it = sessions_.begin(); it != sessions_.end();) {
        it = it->second.expires_at <= now ? sessions_.erase(it) : std::next(it);
    }
    *token = random_hex(32);
    sessions_[sha256_hex(*token)] = Session{user, now + config_.session_ttl_s};
    save_sessions();
    log_info("login: " + user + " from " + client_ip);
    return Result::Ok;
}

void Auth::logout(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessions_.erase(sha256_hex(token))) save_sessions();
}

std::optional<std::string> Auth::check(const std::string& token) {
    if (token.empty()) return std::nullopt;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sha256_hex(token));
    if (it == sessions_.end()) return std::nullopt;
    if (it->second.expires_at <= now_unix()) {
        sessions_.erase(it);
        save_sessions();
        return std::nullopt;
    }
    return it->second.user;
}

}  // namespace axbox
