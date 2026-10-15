/**
 * axbox-manager：大模型一体机的设备与模型管理服务
 *
 * - 网页管理后台（内置静态页面）+ /api/* 管理接口
 * - 管理统一网关（openai_api_gateway）和各模型推理进程
 */

#include "assets.hpp"
#include "auth.hpp"
#include "config.hpp"
#include "jobs.hpp"
#include "model_repo.hpp"
#include "supervisor.hpp"
#include "sysinfo.hpp"
#include "util.hpp"
#include "version.hpp"

#include "openai_api/upstream.hpp"
#include "utils/httplib.h"

#include <signal.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>

using namespace axbox;
using nlohmann::json;

namespace {

httplib::Server* g_server = nullptr;

void on_signal(int) {
    if (g_server) g_server->stop();
}

const char* kCookie = "axbox_sid";

void send_json(httplib::Response& res, const json& j, int status = 200) {
    res.status = status;
    res.set_content(j.dump(), "application/json; charset=utf-8");
}

void send_error(httplib::Response& res, int status, const std::string& message, const std::string& code = "") {
    send_json(res, {{"error", {{"message", message}, {"code", code}}}}, status);
}

std::string cookie_value(const httplib::Request& req, const std::string& name) {
    const std::string header = req.get_header_value("Cookie");
    for (auto& part : split(header, ';')) {
        part = trim(part);
        if (starts_with(part, name + "=")) return part.substr(name.size() + 1);
    }
    return "";
}

json parse_body(const httplib::Request& req) {
    auto j = json::parse(req.body, nullptr, false);
    return j.is_discarded() ? json::object() : j;
}

std::string mime_for(const std::string& path) {
    if (ends_with(path, ".html")) return "text/html; charset=utf-8";
    if (ends_with(path, ".js")) return "application/javascript; charset=utf-8";
    if (ends_with(path, ".css")) return "text/css; charset=utf-8";
    if (ends_with(path, ".svg")) return "image/svg+xml";
    if (ends_with(path, ".json")) return "application/json";
    if (ends_with(path, ".woff2")) return "font/woff2";
    return "application/octet-stream";
}

}  // namespace

int main(int argc, char** argv) {
    std::string config_path = "/etc/axbox/config.json";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-c" || a == "--config") && i + 1 < argc) {
            config_path = argv[++i];
        } else if (a == "-v" || a == "--version") {
            std::cout << "axbox-manager " << kVersion << std::endl;
            return 0;
        } else if (a == "-h" || a == "--help") {
            std::cout << "Usage: " << argv[0] << " [-c /etc/axbox/config.json]" << std::endl;
            return 0;
        } else {
            std::cerr << "Unknown argument: " << a << std::endl;
            return 1;
        }
    }

    std::string error;
    Config config = Config::load(path_exists(config_path) ? config_path : "", &error);
    if (!error.empty()) {
        std::cerr << error << std::endl;
        return 1;
    }
    if (!make_dirs(config.data_dir) || !make_dirs(config.logs_dir()) || !make_dirs(config.models_dir)) {
        std::cerr << "cannot create data_dir / models_dir" << std::endl;
        return 1;
    }
    log_set_file(join_path(config.logs_dir(), "manager.log"));
    log_info(std::string("axbox-manager ") + kVersion + " starting, models_dir=" + config.models_dir);
    if (config.auth_mode == "none") log_warn("auth_mode is \"none\": anyone can manage this device");

    signal(SIGPIPE, SIG_IGN);

    Auth auth(config);
    auth.load_sessions();
    ModelRepo repo(config);
    repo.scan();
    SysInfo sysinfo(config);
    sysinfo.start();
    Supervisor supervisor(config, repo);
    Jobs jobs(config, repo, supervisor);
    jobs.cleanup_incoming();
    supervisor.start();

    // 网页上的对话测试经过网关转发，和业务调用走同一条路径
    openai_api::UpstreamManager playground;
    std::mutex playground_mutex;

    httplib::Server server;
    g_server = &server;
    server.new_task_queue = [] { return new httplib::ThreadPool(16); };
    server.set_payload_max_length(64ull << 30);

    // ---------------------------------------------------------------- 鉴权
    server.set_pre_routing_handler([&](const httplib::Request& req, httplib::Response& res) {
        const bool is_api = starts_with(req.path, "/api/");
        const bool is_v1 = starts_with(req.path, "/v1/");
        if (!is_api && !is_v1) return httplib::Server::HandlerResponse::Unhandled;
        if (req.path == "/api/login" || req.path == "/api/info") return httplib::Server::HandlerResponse::Unhandled;
        // 修改类请求必须带自定义头：跨站页面无法在不经过 CORS 预检的情况下加这个头
        if (req.method != "GET" && req.get_header_value("X-Axbox") != "1") {
            send_error(res, 403, "missing X-Axbox header");
            return httplib::Server::HandlerResponse::Handled;
        }
        if (auth.enabled() && !auth.check(cookie_value(req, kCookie))) {
            send_error(res, 401, "login required", "unauthorized");
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    server.Get("/api/info", [&](const httplib::Request& req, httplib::Response& res) {
        auto user = auth.check(cookie_value(req, kCookie));
        send_json(res, {{"version", kVersion},
                        {"auth", auth.enabled()},
                        {"user", user ? json(*user) : json(nullptr)},
                        {"hostname", sysinfo.device().value("hostname", "")}});
    });

    server.Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.get_header_value("X-Axbox") != "1") return send_error(res, 403, "missing X-Axbox header");
        auto j = parse_body(req);
        const std::string user = j.value("user", "");
        const std::string password = j.value("password", "");
        std::string token;
        int retry_after = 0;
        switch (auth.login(user, password, req.remote_addr, &token, &retry_after)) {
            case Auth::Result::Ok:
                res.set_header("Set-Cookie", std::string(kCookie) + "=" + token +
                                                 "; Path=/; HttpOnly; SameSite=Strict; Max-Age=" +
                                                 std::to_string(config.session_ttl_s));
                return send_json(res, {{"user", user}});
            case Auth::Result::Locked:
                res.set_header("Retry-After", std::to_string(retry_after));
                return send_json(res, {{"error", {{"message", "too many failed attempts"},
                                                  {"code", "locked"},
                                                  {"retry_after", retry_after}}}},
                                 429);
            case Auth::Result::Unavailable:
                return send_error(res, 500, "password check is unavailable on this device, see manager log",
                                  "unavailable");
            default:
                return send_error(res, 401, "wrong user name or password", "bad_credentials");
        }
    });

    server.Post("/api/logout", [&](const httplib::Request& req, httplib::Response& res) {
        auth.logout(cookie_value(req, kCookie));
        res.set_header("Set-Cookie", std::string(kCookie) + "=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");
        send_json(res, json::object());
    });

    // ---------------------------------------------------------------- 设备
    server.Get("/api/device", [&](const httplib::Request&, httplib::Response& res) {
        json d = sysinfo.device();
        d["gateway_port"] = config.gateway_port;
        d["models_dir"] = config.models_dir;
        send_json(res, d);
    });

    server.Get("/api/status", [&](const httplib::Request& req, httplib::Response& res) {
        int64_t since = 0;
        if (req.has_param("since")) since = std::strtoll(req.get_param_value("since").c_str(), nullptr, 10);
        sysinfo.touch();
        send_json(res, {{"now", sysinfo.current()},
                        {"history", sysinfo.history(since)},
                        {"gateway", supervisor.gateway_status()},
                        {"npu_top", sysinfo.npu_top()}});
    });

    server.Put("/api/npu-top", [&](const httplib::Request& req, httplib::Response& res) {
        auto err = sysinfo.set_npu_top(parse_body(req).value("enabled", false));
        if (!err.empty()) return send_error(res, 409, err);
        send_json(res, sysinfo.npu_top());
    });

    // ---------------------------------------------------------------- 模型
    server.Get("/api/models", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("rescan")) repo.scan();
        auto stats = supervisor.gateway_upstreams();
        json out = json::array();
        for (const auto& m : repo.list()) {
            json j = m.to_json();
            j["status"] = supervisor.model_status(m.name);
            if (stats.contains(m.name)) {
                const auto& u = stats[m.name];
                j["gateway"] = {{"health", u.value("health", "")},
                                {"requests", u.value("requests", 0)},
                                {"errors", u.value("errors", 0)},
                                {"active", u.value("active", 0)},
                                {"last_error", u.value("last_error", "")},
                                {"last_call", u.contains("last_call") ? u["last_call"] : json(nullptr)},
                                {"model_info", u.contains("model_info") ? u["model_info"] : json(nullptr)}};
            }
            out.push_back(j);
        }
        send_json(res, {{"data", out}});
    });

    auto model_action = [&](const std::string& action) {
        return [&, action](const httplib::Request& req, httplib::Response& res) {
            const std::string name = req.path_params.at("name");
            std::string err;
            if (action == "start") err = supervisor.start_model(name);
            if (action == "stop") err = supervisor.stop_model(name);
            if (action == "restart") err = supervisor.restart_model(name);
            if (!err.empty()) return send_error(res, 409, err);
            send_json(res, supervisor.model_status(name));
        };
    };
    server.Post("/api/models/:name/start", model_action("start"));
    server.Post("/api/models/:name/stop", model_action("stop"));
    server.Post("/api/models/:name/restart", model_action("restart"));

    server.Put("/api/models/:name/autostart", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string name = req.path_params.at("name");
        if (!repo.find(name)) return send_error(res, 404, "model not found");
        supervisor.set_autostart(name, parse_body(req).value("enabled", false));
        send_json(res, supervisor.model_status(name));
    });

    server.Post("/api/models/:name/verify", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string id = jobs.start_verify(req.path_params.at("name"));
        if (id.empty()) return send_error(res, 404, "model not found");
        send_json(res, jobs.get(id));
    });

    server.Delete("/api/models/:name", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string name = req.path_params.at("name");
        std::string dir;
        for (const auto& m : repo.list()) {
            if (m.name == name) dir = m.dir;
        }
        if (dir.empty()) return send_error(res, 404, "model not found");
        if (supervisor.is_active(name)) return send_error(res, 409, "stop the model before deleting it");
        // 先改名再删除：删到一半出错时也不会留下半个模型
        const std::string trash = join_path(config.models_dir, ".trash-" + random_hex(4));
        if (rename(dir.c_str(), trash.c_str()) != 0) {
            return send_error(res, 500, std::string("cannot delete: ") + strerror(errno));
        }
        remove_tree(trash);
        supervisor.set_autostart(name, false);
        repo.scan();
        log_info("deleted model " + name + " (" + dir + ")");
        send_json(res, json::object());
    });

    // 上传模型包：请求体就是压缩包本身（PUT /api/upload?filename=xxx.tar.gz），流式写盘
    server.Put("/api/upload", [&](const httplib::Request& req, httplib::Response& res,
                                  const httplib::ContentReader& content_reader) {
        const std::string filename = req.get_param_value("filename");
        uint64_t length = req.get_header_value_u64("Content-Length");
        struct statvfs vfs;
        make_dirs(config.incoming_dir());
        if (length > 0 && statvfs(config.incoming_dir().c_str(), &vfs) == 0) {
            const uint64_t free_bytes = uint64_t(vfs.f_bavail) * vfs.f_frsize;
            // 解压时压缩包和解压后的文件同时存在，按 2 倍估算，另外保留 64 MB 给系统
            const uint64_t need = length * 2 + (64ull << 20);
            if (free_bytes < need) {
                return send_error(res, 507,
                                  "not enough storage: a " + std::to_string((length + (1 << 20) - 1) >> 20) +
                                      " MB package needs " + std::to_string(need >> 20) + " MB free, " +
                                      std::to_string(free_bytes >> 20) + " MB available",
                                  "insufficient_storage");
            }
        }
        std::string archive, err;
        const std::string id = jobs.create_import(filename, &archive, &err);
        if (id.empty()) return send_error(res, 400, err);
        FILE* f = fopen(archive.c_str(), "wb");
        if (!f) {
            jobs.fail(id, "cannot write " + archive);
            return send_error(res, 500, "cannot write upload file");
        }
        bool write_ok = true;
        content_reader([&](const char* data, size_t len) {
            if (fwrite(data, 1, len, f) != len) write_ok = false;
            return write_ok;
        });
        write_ok = (fclose(f) == 0) && write_ok;
        if (!write_ok) {
            jobs.fail(id, "write failed (disk full?)");
            return send_error(res, 500, "write failed (disk full?)");
        }
        jobs.start_import(id);
        send_json(res, jobs.get(id));
    });

    server.Get("/api/jobs", [&](const httplib::Request&, httplib::Response& res) {
        send_json(res, {{"data", jobs.list()}});
    });
    server.Get("/api/jobs/:id", [&](const httplib::Request& req, httplib::Response& res) {
        auto j = jobs.get(req.path_params.at("id"));
        if (j.is_null()) return send_error(res, 404, "job not found");
        send_json(res, j);
    });
    server.Post("/api/jobs/:id/confirm", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string id = req.path_params.at("id");
        auto err = jobs.confirm_import(id, parse_body(req).value("overwrite", false));
        if (!err.empty()) return send_error(res, 409, err);
        send_json(res, jobs.get(id));
    });

    // ---------------------------------------------------------------- 日志
    server.Get("/api/logs/:target", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string path = supervisor.log_path(req.path_params.at("target"));
        if (path.empty()) return send_error(res, 400, "invalid log target");
        if (req.has_param("download")) {
            if (!path_exists(path)) return send_error(res, 404, "log not found");
            auto size = dir_size(path);
            res.set_header("Content-Disposition",
                           "attachment; filename=\"" + req.path_params.at("target") + ".log\"");
            res.set_content_provider(size, "text/plain; charset=utf-8",
                                     [path](size_t offset, size_t length, httplib::DataSink& sink) {
                                         FILE* f = fopen(path.c_str(), "rb");
                                         if (!f) return false;
                                         fseek(f, static_cast<long>(offset), SEEK_SET);
                                         std::vector<char> buf(std::min<size_t>(length, 1 << 20));
                                         size_t n = fread(buf.data(), 1, buf.size(), f);
                                         fclose(f);
                                         if (n == 0) return false;
                                         sink.write(buf.data(), n);
                                         return true;
                                     });
            return;
        }
        size_t lines = 300;
        if (req.has_param("lines")) lines = std::min<size_t>(5000, std::stoul(req.get_param_value("lines")));
        res.set_content(tail_file(path, lines), "text/plain; charset=utf-8");
    });

    // ---------------------------------------------------------------- 设置
    server.Get("/api/settings", [&](const httplib::Request&, httplib::Response& res) {
        json s = supervisor.settings();
        s["gateway_port"] = config.gateway_port;
        send_json(res, s);
    });

    server.Put("/api/settings/api-key", [&](const httplib::Request& req, httplib::Response& res) {
        auto j = parse_body(req);
        const bool enabled = j.value("enabled", false);
        std::string key = j.value("key", "");
        if (j.value("generate", false)) key = "sk-" + random_hex(24);
        if (enabled && key.empty()) key = supervisor.settings().value("api_key", "");
        if (enabled && key.empty()) key = "sk-" + random_hex(24);
        auto err = supervisor.set_api_key(enabled, key);
        if (!err.empty()) return send_error(res, 400, err);
        json s = supervisor.settings();
        s["gateway_port"] = config.gateway_port;
        send_json(res, s);
    });

    server.Post("/api/system/reboot", [&](const httplib::Request& req, httplib::Response& res) {
        log_warn("reboot requested by " + auth.check(cookie_value(req, kCookie)).value_or("?"));
        std::thread([&] {
            // 先正常停止所有模型，避免强杀 NPU 进程
            for (const auto& m : repo.list()) {
                if (supervisor.is_active(m.name)) supervisor.stop_model(m.name);
            }
            for (int i = 0; i < config.stop_timeout_s + 20; ++i) {
                bool any = false;
                for (const auto& m : repo.list()) any = any || supervisor.is_active(m.name);
                if (!any) break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            sync();
            if (system("systemctl reboot") != 0 && system("reboot") != 0) log_error("reboot failed");
        }).detach();
        send_json(res, json::object());
    });

    // ---------------------------------------------------------------- 对话测试
    // 浏览器带登录 cookie 调 /v1/*，管理服务转发到网关（带上网关的 API Key）
    auto playground_proxy = [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        const std::string model = body.is_object() ? body.value("model", "") : "";
        if (model.empty()) return send_error(res, 400, "model is required");
        {
            std::lock_guard<std::mutex> lock(playground_mutex);
            openai_api::UpstreamOptions opt;
            opt.base_url = "http://127.0.0.1:" + std::to_string(config.gateway_port);
            opt.api_key = supervisor.api_key();
            opt.health_path = "";
            playground.add(model, opt);
        }
        if (!playground.tryProxy(req, res)) send_error(res, 404, "model not found");
    };
    server.Post("/v1/chat/completions", playground_proxy);
    server.Post("/v1/completions", playground_proxy);
    server.Post("/v1/embeddings", playground_proxy);

    // ---------------------------------------------------------------- 网页
    auto serve_asset = [&](const httplib::Request& req, httplib::Response& res) {
        std::string path = req.path;
        if (path == "/" || path.empty()) path = "/index.html";
        if (path.find("..") != std::string::npos) return send_error(res, 400, "bad path");
        if (!config.web_dir.empty()) {
            if (auto data = read_file(config.web_dir + path)) {
                res.set_header("Cache-Control", "no-cache");
                return res.set_content(*data, mime_for(path));
            }
        } else if (auto a = find_asset(path.substr(1))) {
            res.set_header("Cache-Control", "no-cache");
            return res.set_content(reinterpret_cast<const char*>(a->data), a->size, mime_for(path));
        }
        // 单页应用：其他路径都返回首页
        if (path.find('.') == std::string::npos) {
            if (!config.web_dir.empty()) {
                if (auto data = read_file(config.web_dir + "/index.html")) return res.set_content(*data, mime_for(".html"));
            } else if (auto a = find_asset("index.html")) {
                return res.set_content(reinterpret_cast<const char*>(a->data), a->size, mime_for(".html"));
            }
        }
        res.status = 404;
    };
    server.Get(R"(/(?!api/|v1/).*)", serve_asset);

    server.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
        std::string msg = "internal error";
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            msg = e.what();
        } catch (...) {
        }
        send_error(res, 500, msg);
    });

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    log_info("web console on http://" + config.listen_host + ":" + std::to_string(config.listen_port) +
             ", gateway on port " + std::to_string(config.gateway_port));
    if (!server.listen(config.listen_host, config.listen_port)) {
        log_error("cannot listen on " + config.listen_host + ":" + std::to_string(config.listen_port));
        supervisor.shutdown();
        sysinfo.stop();
        return 1;
    }
    log_info("shutting down (model processes keep running)");
    supervisor.shutdown();
    sysinfo.stop();
    return 0;
}
