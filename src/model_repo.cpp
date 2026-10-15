#include "model_repo.hpp"

#include "util.hpp"

#include <sys/stat.h>

#include <algorithm>

namespace axbox {

nlohmann::json ModelSpec::to_json() const {
    return {
        {"name", name},
        {"dir", dir},
        {"version", version},
        {"type", type},
        {"description", description},
        {"max_context", max_context},
        {"mem_required", {{"cmm_mb", mem_cmm_mb}, {"ram_mb", mem_ram_mb}}},
        {"runtime", {{"name", runtime}, {"min_version", runtime_min_version}}},
        {"files", files.size()},
        {"size_bytes", size_bytes},
        {"error", error},
    };
}

ModelSpec load_model_spec(const std::string& dir, bool compute_size) {
    ModelSpec m;
    m.dir = dir;
    auto slash = dir.find_last_of('/');
    m.name = slash == std::string::npos ? dir : dir.substr(slash + 1);
    if (compute_size) m.size_bytes = dir_size(dir);

    auto text = read_file(join_path(dir, "model.json"));
    if (!text) {
        m.error = "model.json not found";
        return m;
    }
    try {
        auto j = nlohmann::json::parse(*text);
        m.name = j.value("name", m.name);
        m.version = j.value("version", "");
        m.type = j.value("type", "chat");
        m.description = j.value("description", "");
        m.max_context = j.value("max_context", 0);
        if (j.contains("mem_required") && j["mem_required"].is_object()) {
            m.mem_cmm_mb = j["mem_required"].value("cmm_mb", 0);
            m.mem_ram_mb = j["mem_required"].value("ram_mb", 0);
        }
        if (j.contains("runtime")) {
            if (j["runtime"].is_string()) {
                m.runtime = j["runtime"].get<std::string>();
            } else if (j["runtime"].is_object()) {
                m.runtime = j["runtime"].value("name", "");
                m.runtime_min_version = j["runtime"].value("min_version", "");
            }
        }
        if (j.contains("launch") && j["launch"].is_object()) {
            const auto& l = j["launch"];
            if (l.contains("args")) m.args = l["args"].get<std::vector<std::string>>();
            if (l.contains("env")) m.env = l["env"].get<std::map<std::string, std::string>>();
            m.health_path = l.value("health_path", m.health_path);
            m.served_model = l.value("served_model", "");
            m.start_timeout_s = l.value("start_timeout_s", 0);
        }
        if (j.contains("files") && j["files"].is_object()) {
            m.files = j["files"].get<std::map<std::string, std::string>>();
        }
    } catch (const std::exception& e) {
        m.error = std::string("invalid model.json: ") + e.what();
        return m;
    }

    if (!is_safe_name(m.name)) {
        m.error = "invalid model name \"" + m.name + "\" (allowed: letters, digits, . _ -)";
    } else if (m.runtime.empty()) {
        m.error = "model.json: runtime is required";
    } else if (m.args.empty()) {
        if (m.runtime == "axllm") {
            m.args = {"serve", "{model_dir}", "--port", "{port}"};
        } else {
            m.error = "model.json: launch.args is required";
        }
    }
    for (const auto& kv : m.files) {
        if (kv.first.find("..") != std::string::npos || starts_with(kv.first, "/")) {
            m.error = "model.json: invalid file path " + kv.first;
            break;
        }
    }
    return m;
}

VerifyResult verify_model_files(const ModelSpec& spec, const std::function<void(uint64_t, uint64_t)>& progress) {
    VerifyResult r;
    uint64_t total = 0;
    for (const auto& kv : spec.files) {
        struct stat st;
        if (stat(join_path(spec.dir, kv.first).c_str(), &st) == 0) total += static_cast<uint64_t>(st.st_size);
    }
    uint64_t done = 0;
    for (const auto& kv : spec.files) {
        const std::string path = join_path(spec.dir, kv.first);
        if (!path_exists(path)) {
            r.missing.push_back(kv.first);
            continue;
        }
        std::string hash = sha256_file(path, &done);
        if (progress) progress(done, total);
        ++r.checked;
        std::string expected = kv.second;
        std::transform(expected.begin(), expected.end(), expected.begin(), ::tolower);
        if (hash.empty() || hash != expected) r.mismatch.push_back(kv.first);
    }
    r.ok = r.missing.empty() && r.mismatch.empty();
    return r;
}

ModelRepo::ModelRepo(const Config& config) : config_(config) {}

void ModelRepo::scan() {
    std::vector<ModelSpec> found;
    for (const auto& entry : list_dir(config_.models_dir)) {
        if (entry.empty() || entry[0] == '.') continue;
        const std::string dir = join_path(config_.models_dir, entry);
        if (!is_dir(dir)) continue;
        found.push_back(load_model_spec(dir));
    }
    // 同名模型：保留第一个，其余标记错误
    std::map<std::string, int> seen;
    for (auto& m : found) {
        if (m.error.empty() && seen[m.name]++ > 0) m.error = "duplicate model name \"" + m.name + "\"";
    }
    std::sort(found.begin(), found.end(), [](const ModelSpec& a, const ModelSpec& b) { return a.name < b.name; });
    std::lock_guard<std::mutex> lock(mutex_);
    models_ = std::move(found);
}

std::vector<ModelSpec> ModelRepo::list() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return models_;
}

std::optional<ModelSpec> ModelRepo::find(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& m : models_) {
        if (m.name == name && m.error.empty()) return m;
    }
    return std::nullopt;
}

}  // namespace axbox
