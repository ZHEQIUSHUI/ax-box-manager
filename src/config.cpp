#include "config.hpp"

#include "util.hpp"

namespace axbox {

namespace {

template <typename T>
void get_if(const nlohmann::json& j, const char* key, T& out) {
    if (j.contains(key) && !j[key].is_null()) out = j[key].get<T>();
}

}  // namespace

Config Config::load(const std::string& path, std::string* error) {
    Config c;
    if (path.empty()) return c;
    auto text = read_file(path);
    if (!text) {
        if (error) *error = "cannot read config file: " + path;
        return c;
    }
    try {
        auto j = nlohmann::json::parse(*text);
        get_if(j, "listen_host", c.listen_host);
        get_if(j, "listen_port", c.listen_port);
        get_if(j, "models_dir", c.models_dir);
        get_if(j, "data_dir", c.data_dir);
        get_if(j, "web_dir", c.web_dir);
        get_if(j, "gateway_bin", c.gateway_bin);
        get_if(j, "gateway_host", c.gateway_host);
        get_if(j, "gateway_port", c.gateway_port);
        get_if(j, "runtimes", c.runtimes);
        get_if(j, "env", c.env);
        get_if(j, "model_port_base", c.model_port_base);
        get_if(j, "model_port_count", c.model_port_count);
        get_if(j, "isolate_model_ports", c.isolate_model_ports);
        get_if(j, "start_timeout_s", c.start_timeout_s);
        get_if(j, "stop_timeout_s", c.stop_timeout_s);
        get_if(j, "auth_mode", c.auth_mode);
        get_if(j, "allowed_users", c.allowed_users);
        get_if(j, "allowed_groups", c.allowed_groups);
        get_if(j, "session_ttl_s", c.session_ttl_s);
        get_if(j, "npu_top", c.npu_top);
    } catch (const std::exception& e) {
        if (error) *error = "invalid config file " + path + ": " + e.what();
    }
    return c;
}

nlohmann::json Config::to_json() const {
    return {
        {"listen_host", listen_host},       {"listen_port", listen_port},
        {"models_dir", models_dir},         {"data_dir", data_dir},
        {"web_dir", web_dir},               {"gateway_bin", gateway_bin},
        {"gateway_host", gateway_host},     {"gateway_port", gateway_port},
        {"runtimes", runtimes},             {"env", env},
        {"model_port_base", model_port_base}, {"model_port_count", model_port_count},
        {"isolate_model_ports", isolate_model_ports}, {"start_timeout_s", start_timeout_s},
        {"stop_timeout_s", stop_timeout_s}, {"auth_mode", auth_mode},
        {"allowed_users", allowed_users},   {"allowed_groups", allowed_groups},
        {"session_ttl_s", session_ttl_s},   {"npu_top", npu_top},
    };
}

std::string Config::settings_path() const { return join_path(data_dir, "settings.json"); }
std::string Config::logs_dir() const { return join_path(data_dir, "logs"); }
std::string Config::run_dir() const { return join_path(data_dir, "run"); }
std::string Config::incoming_dir() const { return join_path(models_dir, ".incoming"); }

}  // namespace axbox
