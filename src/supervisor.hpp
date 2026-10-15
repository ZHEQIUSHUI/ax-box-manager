#pragma once

#include "config.hpp"
#include "model_repo.hpp"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace axbox {

/**
 * 进程管理：统一网关 + 每个模型一个推理进程
 *
 * - 模型进程用 setsid 启动，管理服务重启后按 run/<name>.json 里记录的 pid 重新接管，不影响正在运行的模型。
 * - 启动：内存检查 → 分配端口 → 启动进程 → 等待健康检查 → 注册到网关。同一时间只启动一个模型。
 * - 停止：先从网关注销，再 SIGINT，超时后 SIGTERM，最后才 SIGKILL（NPU 进程被强杀可能导致 NPU 卡死）。
 * - 进程意外退出：从网关注销并自动重启；10 分钟内崩溃 3 次则停止重启，标记为异常。
 */
class Supervisor {
public:
    Supervisor(const Config& config, ModelRepo& repo);
    ~Supervisor();

    void start();     // 接管已有进程、启动网关、启动开机自启的模型
    void shutdown();  // 停止后台线程（不停止模型进程）

    // 以下操作是异步的：返回空字符串表示已受理，否则为错误信息
    std::string start_model(const std::string& name);
    std::string stop_model(const std::string& name);
    std::string restart_model(const std::string& name);

    bool is_active(const std::string& name) const;  // 启动中 / 运行中 / 停止中
    nlohmann::json model_status(const std::string& name) const;
    nlohmann::json gateway_status() const;
    nlohmann::json gateway_upstreams() const;   // 网关上的调用统计（GET /admin/upstreams）

    // 设置
    nlohmann::json settings() const;  // 不含 api_key 明文以外的敏感字段
    std::string set_autostart(const std::string& name, bool enabled);
    std::string set_api_key(bool enabled, const std::string& key);  // 会重启网关
    std::string api_key() const;      // 未启用时为空

    std::string log_path(const std::string& target) const;  // gateway / manager / 模型名

private:
    enum class State { Stopped, Starting, Running, Stopping, Error };
    static const char* to_string(State s);

    struct Proc {
        std::string name;
        ModelSpec spec;
        State state = State::Stopped;
        std::string message;      // 当前进度或错误原因
        pid_t pid = 0;
        uint64_t proc_start = 0;  // /proc/<pid>/stat 里的启动时间，防止 pid 复用
        bool own_child = false;   // 本进程 fork 的（可 waitpid），否则是接管的
        int port = 0;
        int64_t started_at = 0;
        int64_t ready_at = 0;
        std::string served_model;
        std::deque<int64_t> crashes;
        bool busy = false;        // 正在执行启动 / 停止
    };

    // 进程
    pid_t spawn(const std::vector<std::string>& argv, const std::map<std::string, std::string>& env,
                const std::string& cwd, const std::string& log_path);
    bool alive(pid_t pid, uint64_t start_time, bool own_child, int* exit_status);
    bool terminate(pid_t pid, uint64_t start_time, bool own_child, int stop_timeout_s, const std::string& what);

    // 网关
    void start_gateway();
    void stop_gateway();
    bool register_upstream(const Proc& p);
    void unregister_upstream(const std::string& name);
    void register_all_running();

    // 模型
    void run_start(const std::string& name);
    void run_stop(const std::string& name, bool then_start);
    void handle_crash(const std::string& name, int status);
    int allocate_port(const std::string& name);
    void save_run_state(const Proc& p);
    void remove_run_state(const std::string& name);
    void adopt_running();
    void isolate_model_ports();
    void monitor_loop();
    void spawn_op(std::function<void()> fn);

    void load_settings();
    void save_settings();

    nlohmann::json proc_json(const Proc& p) const;

    const Config& config_;
    ModelRepo& repo_;

    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<Proc>> procs_;
    std::set<std::string> autostart_;
    bool api_key_enabled_ = false;
    std::string api_key_;

    // 网关进程
    pid_t gateway_pid_ = 0;
    uint64_t gateway_start_ = 0;
    int64_t gateway_started_at_ = 0;
    int gateway_restarts_ = 0;
    std::mutex gateway_mutex_;  // 串行化网关重启

    std::mutex start_mutex_;    // 同一时间只启动一个模型

    std::atomic<bool> stop_{false};
    std::mutex stop_mutex_;
    std::condition_variable stop_cv_;
    std::thread monitor_;
};

}  // namespace axbox
