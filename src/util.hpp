#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace axbox {

// ---- 日志 ----
// 同时写 stderr 和日志文件（如果设置了）
void log_set_file(const std::string& path);
void log_info(const std::string& msg);
void log_warn(const std::string& msg);
void log_error(const std::string& msg);

// ---- 文件和字符串 ----
std::optional<std::string> read_file(const std::string& path);
bool write_file_atomic(const std::string& path, const std::string& content);
bool path_exists(const std::string& path);
bool is_dir(const std::string& path);
bool make_dirs(const std::string& path);
bool remove_tree(const std::string& path);
std::vector<std::string> list_dir(const std::string& path);  // 不含 . 和 ..
uint64_t dir_size(const std::string& path);
std::string join_path(const std::string& a, const std::string& b);
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char sep);
bool starts_with(const std::string& s, const std::string& prefix);
bool ends_with(const std::string& s, const std::string& suffix);

// 读取文件最后 max_lines 行
std::string tail_file(const std::string& path, size_t max_lines, size_t max_bytes = 1 << 20);

// 模型名、文件名等只允许 [A-Za-z0-9._-]，且不能以 . 开头
bool is_safe_name(const std::string& name);

// ---- 其他 ----
std::string random_hex(size_t bytes);
int64_t now_unix();
int64_t now_ms();  // 单调时钟，毫秒

// 返回文件的 sha256（十六进制），读失败返回空
std::string sha256_file(const std::string& path, uint64_t* bytes_done = nullptr);
std::string sha256_hex(const std::string& data);

// 运行命令并取输出（最多 max_bytes 字节），失败返回 nullopt
std::optional<std::string> run_capture(const std::vector<std::string>& argv, int timeout_ms = 5000,
                                       size_t max_bytes = 64 * 1024, int* exit_code = nullptr);

}  // namespace axbox
