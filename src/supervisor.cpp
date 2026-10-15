#include "supervisor.hpp"

#include "sysinfo.hpp"
#include "util.hpp"
#include "utils/httplib.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstring>

extern char** environ;

namespace axbox {

namespace {

constexpr int kCrashWindowS = 10 * 60;
constexpr int kMaxCrashes = 3;
constexpr int kRestartDelayMs = 3000;
constexpr uint64_t kLogRotateBytes = 20ull << 20;

uint64_t read_proc_start(pid_t pid) {
    auto s = read_file("/proc/" + std::to_string(pid) + "/stat");
    if (!s) return 0;
    // 进程名可能含空格和括号，从最后一个 ')' 之后开始数：state 是第 3 个字段，starttime 是第 22 个
    auto rp = s->rfind(')');
    if (rp == std::string::npos) return 0;
    auto fields = split(trim(s->substr(rp + 1)), ' ');
    if (fields.size() < 20) return 0;
    if (fields[0] == "Z" || fields[0] == "X") return 0;  // 僵尸进程视为已退出
    return std::strtoull(fields[19].c_str(), nullptr, 10);
}

bool port_free(int port) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return false;
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bool ok = bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    close(fd);
    return ok;
}

std::string describe_exit(int status) {
    if (WIFEXITED(status)) return "exit code " + std::to_string(WEXITSTATUS(status));
    if (WIFSIGNALED(status)) return std::string("signal ") + strsignal(WTERMSIG(status));
    return "exited";
}

void rotate_log(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && static_cast<uint64_t>(st.st_size) > kLogRotateBytes) {
        rename(path.c_str(), (path + ".1").c_str());
    }
}

std::string substitute(std::string s, const std::map<std::string, std::string>& vars) {
    for (const auto& kv : vars) {
        const std::string key = "{" + kv.first + "}";
        for (size_t pos = 0; (pos = s.find(key, pos)) != std::string::npos; pos += kv.second.size()) {
            s.replace(pos, key.size(), kv.second);
        }
    }
    return s;
}

}  // namespace

const char* Supervisor::to_string(State s) {
    switch (s) {
        case State::Stopped: return "stopped";
        case State::Starting: return "starting";
        case State::Running: return "running";
        case State::Stopping: return "stopping";
        case State::Error: return "error";
    }
    return "unknown";
}

Supervisor::Supervisor(const Config& config, ModelRepo& repo) : config_(config), repo_(repo) {}

Supervisor::~Supervisor() { shutdown(); }

// ---------------------------------------------------------------- 进程工具

pid_t Supervisor::spawn(const std::vector<std::string>& argv, const std::map<std::string, std::string>& env,
                        const std::string& cwd, const std::string& log_path) {
    // fork 后子进程里只做异步信号安全的操作，所有参数提前准备好
    std::map<std::string, std::string> merged;
    for (char** e = environ; *e; ++e) {
        std::string kv = *e;
        auto eq = kv.find('=');
        if (eq != std::string::npos) merged[kv.substr(0, eq)] = kv.substr(eq + 1);
    }
    for (const auto& kv : env) merged[kv.first] = kv.second;
    std::vector<std::string> env_strs;
    for (const auto& kv : merged) env_strs.push_back(kv.first + "=" + kv.second);
    std::vector<char*> envp, args;
    for (auto& s : env_strs) envp.push_back(&s[0]);
    envp.push_back(nullptr);
    for (const auto& a : argv) args.push_back(const_cast<char*>(a.c_str()));
    args.push_back(nullptr);

    rotate_log(log_path);
    int log_fd = open(log_path.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (log_fd < 0) return -1;
    int null_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    struct rlimit rl;
    int max_fd = getrlimit(RLIMIT_NOFILE, &rl) == 0 ? static_cast<int>(std::min<rlim_t>(rl.rlim_cur, 65536)) : 1024;

    pid_t pid = fork();
    if (pid == 0) {
        setsid();  // 独立进程组：管理服务退出或重启时模型进程不受影响
        dup2(null_fd, 0);
        dup2(log_fd, 1);
        dup2(log_fd, 2);
        // 关闭继承的其他描述符（尤其是管理服务的监听 socket）
#ifdef SYS_close_range
        if (syscall(SYS_close_range, 3, ~0u, 0) != 0)
#endif
            for (int fd = 3; fd < max_fd; ++fd) close(fd);
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, nullptr);
        signal(SIGPIPE, SIG_DFL);
        if (!cwd.empty() && chdir(cwd.c_str()) != 0) _exit(126);
        execve(args[0], args.data(), envp.data());
        _exit(127);
    }
    close(log_fd);
    if (null_fd >= 0) close(null_fd);
    return pid;
}

bool Supervisor::alive(pid_t pid, uint64_t start_time, bool own_child, int* exit_status) {
    if (pid <= 0) return false;
    if (own_child) {
        int status = 0;
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) {
            if (exit_status) *exit_status = status;
            return false;
        }
        if (r == 0) return true;
    }
    uint64_t st = read_proc_start(pid);
    return st != 0 && (start_time == 0 || st == start_time);
}

bool Supervisor::terminate(pid_t pid, uint64_t start_time, bool own_child, int stop_timeout_s,
                           const std::string& what) {
    struct Step {
        int sig;
        int wait_s;
    };
    const Step steps[] = {{SIGINT, stop_timeout_s}, {SIGTERM, 10}, {SIGKILL, 5}};
    for (const auto& step : steps) {
        if (!alive(pid, start_time, own_child, nullptr)) return true;
        if (step.sig == SIGKILL) {
            log_warn(what + " did not exit after SIGINT/SIGTERM, sending SIGKILL (the NPU may need a reboot)");
        }
        kill(pid, step.sig);
        const int64_t deadline = now_ms() + step.wait_s * 1000;
        while (now_ms() < deadline) {
            if (!alive(pid, start_time, own_child, nullptr)) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    return !alive(pid, start_time, own_child, nullptr);
}

// ---------------------------------------------------------------- 设置

void Supervisor::load_settings() {
    auto text = read_file(config_.settings_path());
    if (!text) return;
    try {
        auto j = nlohmann::json::parse(*text);
        api_key_enabled_ = j.value("api_key_enabled", false);
        api_key_ = j.value("api_key", "");
        if (j.contains("autostart")) {
            for (const auto& n : j["autostart"]) autostart_.insert(n.get<std::string>());
        }
    } catch (const std::exception& e) {
        log_error("invalid settings file: " + std::string(e.what()));
    }
}

void Supervisor::save_settings() {
    // 调用方持有 mutex_
    nlohmann::json j = {{"api_key_enabled", api_key_enabled_},
                        {"api_key", api_key_},
                        {"autostart", std::vector<std::string>(autostart_.begin(), autostart_.end())}};
    const std::string path = config_.settings_path();
    if (!write_file_atomic(path, j.dump(2))) log_error("failed to save " + path);
    chmod(path.c_str(), 0600);  // 含 API Key
}

nlohmann::json Supervisor::settings() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {{"api_key_enabled", api_key_enabled_},
            {"api_key", api_key_},
            {"autostart", std::vector<std::string>(autostart_.begin(), autostart_.end())}};
}

std::string Supervisor::api_key() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return api_key_enabled_ ? api_key_ : "";
}

std::string Supervisor::set_autostart(const std::string& name, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (enabled) {
        autostart_.insert(name);
    } else {
        autostart_.erase(name);
    }
    save_settings();
    return "";
}

std::string Supervisor::set_api_key(bool enabled, const std::string& key) {
    if (enabled && key.size() < 8) return "API key must be at least 8 characters";
    for (char c : key) {
        if (c <= ' ' || c > '~') return "API key may only contain printable ASCII characters without spaces";
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        api_key_enabled_ = enabled;
        if (!key.empty()) api_key_ = key;
        save_settings();
    }
    // 网关的 API Key 是启动参数，修改后重启网关并重新注册上游
    spawn_op([this] {
        std::lock_guard<std::mutex> lock(gateway_mutex_);
        stop_gateway();
        start_gateway();
        register_all_running();
    });
    return "";
}

std::string Supervisor::log_path(const std::string& target) const {
    if (target == "gateway" || target == "manager") return join_path(config_.logs_dir(), target + ".log");
    if (!is_safe_name(target)) return "";
    return join_path(config_.logs_dir(), "model-" + target + ".log");
}

// ---------------------------------------------------------------- 网关

void Supervisor::start_gateway() {
    std::vector<std::string> argv = {config_.gateway_bin,
                                     "--host", config_.gateway_host,
                                     "--port", std::to_string(config_.gateway_port),
                                     "--health-interval", "5000"};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (api_key_enabled_ && !api_key_.empty()) {
            argv.push_back("--api-key");
            argv.push_back(api_key_);
        }
    }
    pid_t pid = spawn(argv, {}, "", log_path("gateway"));
    if (pid <= 0) {
        log_error("failed to start gateway: " + config_.gateway_bin);
        return;
    }
    // 等待网关可以访问
    httplib::Client cli("127.0.0.1", config_.gateway_port);
    cli.set_connection_timeout(std::chrono::milliseconds(500));
    for (int i = 0; i < 50; ++i) {
        if (auto res = cli.Get("/admin/upstreams"); res && res->status == 200) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::lock_guard<std::mutex> lock(mutex_);
    gateway_pid_ = pid;
    gateway_start_ = read_proc_start(pid);
    gateway_started_at_ = now_unix();
    write_file_atomic(join_path(config_.run_dir(), "gateway.json"),
                      nlohmann::json{{"pid", pid}, {"proc_start", gateway_start_}}.dump());
    log_info("gateway started, pid " + std::to_string(pid) + ", port " + std::to_string(config_.gateway_port));
}

void Supervisor::stop_gateway() {
    pid_t pid;
    uint64_t start;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pid = gateway_pid_;
        start = gateway_start_;
        gateway_pid_ = 0;
    }
    if (pid > 0) terminate(pid, start, true, 10, "gateway");
}

bool Supervisor::register_upstream(const Proc& p) {
    nlohmann::json body = {{"model", p.name},
                           {"base_url", "http://127.0.0.1:" + std::to_string(p.port)},
                           {"health_path", p.spec.health_path}};
    if (!p.served_model.empty() && p.served_model != p.name) body["upstream_model"] = p.served_model;
    nlohmann::json extra = {{"owned_by", "local"}};
    if (p.spec.max_context > 0) extra["max_context"] = p.spec.max_context;
    if (!p.spec.type.empty()) extra["type"] = p.spec.type;
    if (!p.spec.version.empty()) extra["version"] = p.spec.version;
    body["extra"] = extra;
    httplib::Client cli("127.0.0.1", config_.gateway_port);
    cli.set_connection_timeout(std::chrono::seconds(2));
    auto res = cli.Post("/admin/upstreams", body.dump(), "application/json");
    if (!res || res->status != 200) {
        log_warn("register " + p.name + " to gateway failed: " +
                 (res ? std::to_string(res->status) + " " + res->body : httplib::to_string(res.error())));
        return false;
    }
    return true;
}

void Supervisor::unregister_upstream(const std::string& name) {
    httplib::Client cli("127.0.0.1", config_.gateway_port);
    cli.set_connection_timeout(std::chrono::seconds(2));
    cli.Delete("/admin/upstreams/" + httplib::encode_uri_component(name));
}

void Supervisor::register_all_running() {
    std::vector<Proc> running;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& kv : procs_) {
            if (kv.second->state == State::Running) running.push_back(*kv.second);
        }
    }
    for (const auto& p : running) register_upstream(p);
}

nlohmann::json Supervisor::gateway_status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {{"running", gateway_pid_ > 0},
            {"pid", gateway_pid_},
            {"port", config_.gateway_port},
            {"started_at", gateway_started_at_},
            {"restarts", gateway_restarts_},
            {"api_key_enabled", api_key_enabled_}};
}

nlohmann::json Supervisor::gateway_upstreams() const {
    httplib::Client cli("127.0.0.1", config_.gateway_port);
    cli.set_connection_timeout(std::chrono::seconds(1));
    cli.set_read_timeout(std::chrono::seconds(2));
    auto res = cli.Get("/admin/upstreams");
    nlohmann::json out = nlohmann::json::object();
    if (!res || res->status != 200) return out;
    auto j = nlohmann::json::parse(res->body, nullptr, false);
    if (!j.is_object() || !j.contains("data")) return out;
    for (const auto& u : j["data"]) out[u.value("model", "")] = u;
    return out;
}

// ---------------------------------------------------------------- 模型

void Supervisor::save_run_state(const Proc& p) {
    nlohmann::json j = {{"name", p.name}, {"pid", p.pid},          {"proc_start", p.proc_start},
                        {"port", p.port}, {"started_at", p.started_at}, {"ready_at", p.ready_at},
                        {"served_model", p.served_model}, {"dir", p.spec.dir}};
    write_file_atomic(join_path(config_.run_dir(), p.name + ".json"), j.dump());
}

void Supervisor::remove_run_state(const std::string& name) {
    unlink(join_path(config_.run_dir(), name + ".json").c_str());
}

int Supervisor::allocate_port(const std::string& name) {
    std::set<int> used;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& kv : procs_) {
            if (kv.first != name && kv.second->port > 0 && kv.second->state != State::Stopped) {
                used.insert(kv.second->port);
            }
        }
    }
    for (int port = config_.model_port_base; port < config_.model_port_base + config_.model_port_count; ++port) {
        if (port == config_.gateway_port || port == config_.listen_port || used.count(port)) continue;
        if (port_free(port)) return port;
    }
    return 0;
}

void Supervisor::spawn_op(std::function<void()> fn) {
    // 操作线程分离运行：模型加载可能持续数分钟，不能阻塞监控线程
    std::thread([fn = std::move(fn)] { fn(); }).detach();
}

std::string Supervisor::start_model(const std::string& name) {
    auto spec = repo_.find(name);
    if (!spec) return "model not found or model.json invalid: " + name;
    if (config_.runtimes.find(spec->runtime) == config_.runtimes.end()) {
        return "runtime \"" + spec->runtime + "\" is not installed on this device";
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& p = procs_[name];
        if (!p) {
            p = std::make_shared<Proc>();
            p->name = name;
        }
        if (p->busy || p->state == State::Running || p->state == State::Starting) {
            return "model is already " + std::string(to_string(p->state));
        }
        p->spec = *spec;
        p->busy = true;
        p->state = State::Starting;
        p->message = "queued";
        p->crashes.clear();
    }
    spawn_op([this, name] { run_start(name); });
    return "";
}

void Supervisor::run_start(const std::string& name) {
    std::unique_lock<std::mutex> start_lock(start_mutex_);
    std::shared_ptr<Proc> p;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p = procs_[name];
        if (p->state != State::Starting) {  // 排队期间被取消
            p->busy = false;
            return;
        }
        p->message = "checking memory";
    }
    auto fail = [&](const std::string& msg) {
        log_error("start " + name + " failed: " + msg);
        std::lock_guard<std::mutex> lock(mutex_);
        p->state = State::Error;
        p->message = msg;
        p->pid = 0;
        p->busy = false;
    };

    const ModelSpec spec = p->spec;
    // 1. 内存检查
    auto mem = read_meminfo();
    auto cmm = read_cmm();
    if (spec.mem_ram_mb > 0 && mem.available_kb / 1024 < static_cast<uint64_t>(spec.mem_ram_mb)) {
        return fail("not enough memory: need " + std::to_string(spec.mem_ram_mb) + " MB, available " +
                    std::to_string(mem.available_kb / 1024) + " MB. Stop other models first.");
    }
    if (spec.mem_cmm_mb > 0 && cmm.valid && cmm.free_kb / 1024 < static_cast<uint64_t>(spec.mem_cmm_mb)) {
        return fail("not enough NPU memory (CMM): need " + std::to_string(spec.mem_cmm_mb) + " MB, available " +
                    std::to_string(cmm.free_kb / 1024) + " MB. Stop other models first.");
    }

    // 2. 端口
    int port = allocate_port(name);
    if (port == 0) return fail("no free port");

    // 3. 启动进程
    const std::string bin = config_.runtimes.at(spec.runtime);
    if (!path_exists(bin)) return fail("runtime binary not found: " + bin);
    std::map<std::string, std::string> vars = {
        {"model_dir", spec.dir}, {"port", std::to_string(port)}, {"name", spec.name}};
    std::vector<std::string> argv = {bin};
    for (const auto& a : spec.args) argv.push_back(substitute(a, vars));
    std::map<std::string, std::string> env = config_.env;
    for (const auto& kv : spec.env) env[kv.first] = substitute(kv.second, vars);

    const std::string log = log_path(name);
    {
        FILE* f = fopen(log.c_str(), "a");
        if (f) {
            std::string cmd;
            for (const auto& a : argv) cmd += a + " ";
            fprintf(f, "\n===== [axbox] starting %s on port %d: %s\n", name.c_str(), port, cmd.c_str());
            fclose(f);
        }
    }
    pid_t pid = spawn(argv, env, spec.dir, log);
    if (pid <= 0) return fail("failed to start process");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p->pid = pid;
        p->proc_start = read_proc_start(pid);
        p->own_child = true;
        p->port = port;
        p->started_at = now_unix();
        p->ready_at = 0;
        p->message = "loading model";
    }
    log_info("starting " + name + ", pid " + std::to_string(pid) + ", port " + std::to_string(port));

    // 4. 等待就绪
    const int timeout_s = spec.start_timeout_s > 0 ? spec.start_timeout_s : config_.start_timeout_s;
    const int64_t deadline = now_ms() + int64_t(timeout_s) * 1000;
    httplib::Client cli("127.0.0.1", port);
    cli.set_connection_timeout(std::chrono::seconds(1));
    cli.set_read_timeout(std::chrono::seconds(5));
    std::string served = spec.served_model;
    bool ready = false;
    while (now_ms() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (p->state != State::Starting) break;  // 启动过程中被要求停止
        }
        int status = 0;
        if (!alive(pid, p->proc_start, true, &status)) {
            return fail("process exited during startup (" + describe_exit(status) + "), see the log for details");
        }
        if (spec.health_path.empty()) {
            ready = true;
            break;
        }
        auto res = cli.Get(spec.health_path);
        if (res && res->status == 200) {
            if (served.empty()) {
                auto j = nlohmann::json::parse(res->body, nullptr, false);
                if (j.is_object() && j.contains("data") && j["data"].is_array() && !j["data"].empty()) {
                    served = j["data"][0].value("id", "");
                }
            }
            ready = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (p->state != State::Starting) {
            // 被 stop 打断：run_stop 会负责结束进程
            p->busy = false;
            return;
        }
    }
    if (!ready) {
        terminate(pid, p->proc_start, true, config_.stop_timeout_s, name);
        return fail("model did not become ready within " + std::to_string(timeout_s) + " s");
    }

    // 5. 注册到网关
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p->served_model = served;
        p->ready_at = now_unix();
        p->state = State::Running;
        p->message = "";
        p->busy = false;
    }
    save_run_state(*p);
    register_upstream(*p);
    log_info(name + " is ready (" + std::to_string(now_unix() - p->started_at) + " s)");
}

std::string Supervisor::stop_model(const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = procs_.find(name);
        if (it == procs_.end() || it->second->state == State::Stopped) return "model is not running";
        auto& p = it->second;
        if (p->state == State::Stopping) return "model is already stopping";
        if (p->state == State::Error && p->pid == 0) {
            p->state = State::Stopped;
            p->message = "";
            return "";
        }
        p->state = State::Stopping;
        p->message = "stopping";
    }
    spawn_op([this, name] { run_stop(name, false); });
    return "";
}

std::string Supervisor::restart_model(const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = procs_.find(name);
        if (it == procs_.end() || it->second->state == State::Stopped ||
            (it->second->state == State::Error && it->second->pid == 0)) {
            // 没在运行，直接启动
        } else {
            if (it->second->state == State::Stopping) return "model is stopping";
            it->second->state = State::Stopping;
            it->second->message = "restarting";
            spawn_op([this, name] { run_stop(name, true); });
            return "";
        }
    }
    return start_model(name);
}

void Supervisor::run_stop(const std::string& name, bool then_start) {
    std::shared_ptr<Proc> p;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p = procs_[name];
    }
    unregister_upstream(name);
    // 等待启动线程退出等待循环（它看到 state 变化后会立即返回）
    for (int i = 0; i < 100; ++i) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!p->busy) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    pid_t pid;
    uint64_t start;
    bool own;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pid = p->pid;
        start = p->proc_start;
        own = p->own_child;
    }
    bool ok = pid <= 0 || terminate(pid, start, own, config_.stop_timeout_s, name);
    remove_run_state(name);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p->pid = 0;
        p->ready_at = 0;
        p->state = ok ? State::Stopped : State::Error;
        p->message = ok ? "" : "process did not exit";
    }
    log_info("stopped " + name);
    if (then_start && ok) start_model(name);
}

void Supervisor::handle_crash(const std::string& name, int status) {
    std::shared_ptr<Proc> p;
    bool retry = false;
    std::string reason = status >= 0 ? describe_exit(status) : "process disappeared";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        p = procs_[name];
        const int64_t now = now_unix();
        p->crashes.push_back(now);
        while (!p->crashes.empty() && now - p->crashes.front() > kCrashWindowS) p->crashes.pop_front();
        retry = p->crashes.size() < static_cast<size_t>(kMaxCrashes);
        p->pid = 0;
        p->ready_at = 0;
        p->state = State::Error;
        p->message = "process exited unexpectedly (" + reason + ")" +
                     (retry ? ", restarting" : ", restarted too many times, giving up");
        if (retry) p->busy = true;  // 防止重启前被重复操作
    }
    log_error(name + " crashed: " + reason);
    unregister_upstream(name);
    remove_run_state(name);
    if (!retry) return;
    spawn_op([this, name, p] {
        std::this_thread::sleep_for(std::chrono::milliseconds(kRestartDelayMs));
        auto crashes = p->crashes;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            p->busy = false;
            if (p->state != State::Error) return;  // 期间被用户操作过
        }
        if (start_model(name).empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            p->crashes = crashes;  // start_model 会清空崩溃计数，这里保留
        }
    });
}

void Supervisor::adopt_running() {
    for (const auto& file : list_dir(config_.run_dir())) {
        const std::string path = join_path(config_.run_dir(), file);
        if (!ends_with(file, ".json")) continue;
        auto text = read_file(path);
        auto j = text ? nlohmann::json::parse(*text, nullptr, false) : nlohmann::json();
        if (!j.is_object()) {
            unlink(path.c_str());
            continue;
        }
        pid_t pid = j.value("pid", 0);
        uint64_t start = j.value("proc_start", uint64_t(0));
        if (file == "gateway.json") {
            // 旧网关：结束掉，用当前设置重新启动
            if (pid > 0 && alive(pid, start, false, nullptr)) terminate(pid, start, false, 10, "old gateway");
            unlink(path.c_str());
            continue;
        }
        const std::string name = j.value("name", "");
        auto spec = repo_.find(name);
        if (!spec || pid <= 0 || !alive(pid, start, false, nullptr)) {
            unlink(path.c_str());
            continue;
        }
        auto p = std::make_shared<Proc>();
        p->name = name;
        p->spec = *spec;
        p->state = State::Running;
        p->pid = pid;
        p->proc_start = start;
        p->own_child = false;
        p->port = j.value("port", 0);
        p->started_at = j.value("started_at", int64_t(0));
        p->ready_at = j.value("ready_at", int64_t(0));
        p->served_model = j.value("served_model", "");
        std::lock_guard<std::mutex> lock(mutex_);
        procs_[name] = p;
        log_info("adopted running model " + name + ", pid " + std::to_string(pid));
    }
}

void Supervisor::isolate_model_ports() {
    const std::string range = std::to_string(config_.model_port_base) + ":" +
                              std::to_string(config_.model_port_base + config_.model_port_count - 1);
    const std::vector<std::string> rule = {"INPUT", "-p", "tcp", "--dport", range, "!", "-i", "lo",
                                           "-j", "DROP"};  // 部分 BSP 内核没有 REJECT 模块
    // 模型服务（httplib）只监听 IPv4，处理 iptables 即可
    for (const char* bin : {"iptables"}) {
        std::vector<std::string> check = {bin, "-w", "-C"};
        check.insert(check.end(), rule.begin(), rule.end());
        int code = -1;
        if (!run_capture(check, 5000, 4096, &code)) {
            log_warn(std::string(bin) + " not available: model ports " + range + " stay reachable from the network");
            continue;
        }
        if (code == 0) continue;  // 规则已存在
        std::vector<std::string> insert = {bin, "-w", "-I"};
        insert.insert(insert.end(), rule.begin(), rule.end());
        auto out = run_capture(insert, 5000, 4096, &code);
        if (out && code == 0) {
            log_info(std::string(bin) + ": blocked non-local access to model ports " + range);
        } else {
            log_warn(std::string(bin) + " failed to block model ports " + range + ": " + (out ? trim(*out) : ""));
        }
    }
}

void Supervisor::start() {
    make_dirs(config_.run_dir());
    make_dirs(config_.logs_dir());
    if (config_.isolate_model_ports) isolate_model_ports();
    load_settings();
    adopt_running();
    {
        std::lock_guard<std::mutex> lock(gateway_mutex_);
        start_gateway();
        register_all_running();
    }
    std::set<std::string> autostart;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        autostart = autostart_;
    }
    for (const auto& name : autostart) {
        if (!is_active(name)) {
            auto err = start_model(name);
            if (!err.empty()) log_warn("autostart " + name + ": " + err);
        }
    }
    monitor_ = std::thread([this] { monitor_loop(); });
}

void Supervisor::monitor_loop() {
    while (!stop_) {
        // 网关
        bool gateway_dead = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            int status = 0;
            if (gateway_pid_ > 0 && !alive(gateway_pid_, gateway_start_, true, &status)) {
                log_error("gateway exited (" + describe_exit(status) + "), restarting");
                gateway_pid_ = 0;
                ++gateway_restarts_;
                gateway_dead = true;
            }
        }
        if (gateway_dead) {
            std::lock_guard<std::mutex> lock(gateway_mutex_);
            start_gateway();
            register_all_running();
        }

        // 模型进程
        std::vector<std::pair<std::string, int>> crashed;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& kv : procs_) {
                auto& p = kv.second;
                if (p->state != State::Running || p->pid <= 0) continue;
                int status = -1;
                if (!alive(p->pid, p->proc_start, p->own_child, &status)) {
                    crashed.emplace_back(kv.first, status);
                }
            }
        }
        for (const auto& c : crashed) handle_crash(c.first, c.second);

        std::unique_lock<std::mutex> lock(stop_mutex_);
        stop_cv_.wait_for(lock, std::chrono::seconds(1), [this] { return stop_.load(); });
    }
}

void Supervisor::shutdown() {
    stop_ = true;
    stop_cv_.notify_all();
    if (monitor_.joinable()) monitor_.join();
}

// ---------------------------------------------------------------- 状态

bool Supervisor::is_active(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = procs_.find(name);
    if (it == procs_.end()) return false;
    auto s = it->second->state;
    return s == State::Starting || s == State::Running || s == State::Stopping || it->second->busy ||
           it->second->pid > 0;
}

nlohmann::json Supervisor::proc_json(const Proc& p) const {
    nlohmann::json j = {{"state", to_string(p.state)},
                        {"message", p.message},
                        {"pid", p.pid},
                        {"port", p.port},
                        {"started_at", p.started_at},
                        {"ready_at", p.ready_at},
                        {"served_model", p.served_model},
                        {"crashes", p.crashes.size()}};
    if (p.pid > 0) {
        if (auto s = read_file("/proc/" + std::to_string(p.pid) + "/status")) {
            for (const auto& line : split(*s, '\n')) {
                if (starts_with(line, "VmRSS:")) j["rss_kb"] = std::strtoull(line.c_str() + 6, nullptr, 10);
            }
        }
    }
    return j;
}

nlohmann::json Supervisor::model_status(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json j;
    auto it = procs_.find(name);
    if (it == procs_.end()) {
        j = {{"state", "stopped"}, {"message", ""}};
    } else {
        j = proc_json(*it->second);
    }
    j["autostart"] = autostart_.count(name) > 0;
    return j;
}

}  // namespace axbox
