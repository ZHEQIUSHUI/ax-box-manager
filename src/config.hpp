#pragma once

#include "utils/json.hpp"

#include <map>
#include <string>
#include <vector>

namespace axbox {

/**
 * 管理服务的配置（/etc/axbox/config.json），全部字段都有默认值。
 */
struct Config {
    std::string listen_host = "0.0.0.0";
    int listen_port = 8080;

    std::string models_dir = "/data/models";      // 本地模型仓库
    std::string data_dir = "/var/lib/axbox";      // 设置、日志、运行状态
    std::string web_dir;                          // 非空时从该目录提供网页（开发用），否则用内置网页

    // 统一网关
    std::string gateway_bin = "/opt/axbox/bin/openai_api_gateway";
    std::string gateway_host = "0.0.0.0";
    int gateway_port = 8000;

    // 推理运行时：model.json 里 runtime.name -> 可执行文件
    std::map<std::string, std::string> runtimes = {{"axllm", "/opt/axbox/bin/axllm"}};
    // 所有模型进程都带的环境变量（如 LD_LIBRARY_PATH）
    std::map<std::string, std::string> env;

    int model_port_base = 18001;
    int model_port_count = 1000;
    // 模型进程（如 axllm serve）监听 0.0.0.0，局域网能绕过网关和 API Key 直接访问。
    // 开启后用 iptables 拒绝非本机访问模型端口段（需要 iptables；运行时支持只监听本机后可关闭）
    bool isolate_model_ports = true;
    int start_timeout_s = 600;      // 等待模型就绪的最长时间（从慢存储加载大模型可能要几分钟）
    int stop_timeout_s = 60;        // SIGINT 后等待退出的时间，超时再 SIGTERM / SIGKILL

    // 登录
    std::string auth_mode = "system";             // system：系统账号；none：不校验（仅开发调试）
    std::vector<std::string> allowed_users = {"root"};
    std::vector<std::string> allowed_groups;      // 组内成员也允许登录
    int session_ttl_s = 12 * 3600;

    bool npu_top = true;            // 启动时打开 /proc/ax_proc/npu/enable 以读取 NPU 利用率

    static Config load(const std::string& path, std::string* error);
    nlohmann::json to_json() const;

    std::string settings_path() const;
    std::string logs_dir() const;
    std::string run_dir() const;
    std::string incoming_dir() const;
};

}  // namespace axbox
