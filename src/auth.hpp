#pragma once

#include "config.hpp"

#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace axbox {

/**
 * 登录与会话
 *
 * 用设备的系统账号登录：读 /etc/shadow，用系统 libcrypt 的 crypt() 校验（支持 yescrypt / sha512 等系统使用的算法）。
 * 不保存、不记录明文密码；不提供改密码功能。
 */
class Auth {
public:
    explicit Auth(const Config& config);

    enum class Result { Ok, BadCredentials, NotAllowed, Locked, Unavailable };

    // 校验用户名密码，成功时返回会话令牌
    Result login(const std::string& user, const std::string& password, const std::string& client_ip,
                 std::string* token, int* retry_after_s);
    void logout(const std::string& token);

    // 令牌有效时返回用户名
    std::optional<std::string> check(const std::string& token);

    bool enabled() const { return config_.auth_mode != "none"; }

private:
    bool verify_password(const std::string& user, const std::string& password, std::string* error);
    bool user_allowed(const std::string& user) const;

    const Config& config_;

    struct Session {
        std::string user;
        int64_t expires_at;
    };
    struct Failures {
        int count = 0;
        int64_t first_at = 0;
        int64_t locked_until = 0;
    };

    std::mutex mutex_;
    std::map<std::string, Session> sessions_;
    std::map<std::string, Failures> failures_;  // key: user@ip
    std::mutex crypt_mutex_;
};

}  // namespace axbox
