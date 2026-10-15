#include "jobs.hpp"

#include "util.hpp"

#include <unistd.h>

#include <thread>

namespace axbox {

namespace {

constexpr size_t kMaxJobs = 50;

// 在解压目录里找 model.json：根目录，或唯一的子目录
std::string find_model_dir(const std::string& root) {
    if (path_exists(join_path(root, "model.json"))) return root;
    std::vector<std::string> dirs;
    for (const auto& name : list_dir(root)) {
        if (is_dir(join_path(root, name))) dirs.push_back(name);
    }
    if (dirs.size() == 1) {
        const std::string sub = join_path(root, dirs[0]);
        if (path_exists(join_path(sub, "model.json"))) return sub;
    }
    return "";
}

}  // namespace

Jobs::Jobs(const Config& config, ModelRepo& repo, Supervisor& supervisor)
    : config_(config), repo_(repo), supervisor_(supervisor) {}

void Jobs::cleanup_incoming() {
    remove_tree(config_.incoming_dir());
}

nlohmann::json Jobs::to_json(const Job& j) const {
    return {{"id", j.id},           {"kind", j.kind},         {"state", j.state},
            {"stage", j.stage},     {"progress", j.progress}, {"message", j.message},
            {"model", j.model},     {"created_at", j.created_at}, {"result", j.result}};
}

nlohmann::json Jobs::get(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) return nullptr;
    return to_json(*it->second);
}

nlohmann::json Jobs::list() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json out = nlohmann::json::array();
    for (const auto& kv : jobs_) out.push_back(to_json(*kv.second));
    return out;
}

void Jobs::set(const std::shared_ptr<Job>& job, const std::string& state, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    job->state = state;
    job->message = message;
    if (state == "failed") log_warn(job->kind + " job " + job->id + " failed: " + message);
}

std::string Jobs::create_import(const std::string& filename, std::string* archive_path, std::string* error) {
    std::string ext;
    for (const char* e : {".tar.gz", ".tgz", ".tar", ".zip"}) {
        if (ends_with(filename, e)) {
            ext = e;
            break;
        }
    }
    if (ext.empty()) {
        *error = "unsupported package format (use .tar, .tar.gz, .tgz or .zip)";
        return "";
    }
    auto job = std::make_shared<Job>();
    job->id = random_hex(8);
    job->kind = "import";
    job->state = "running";
    job->stage = "receiving";
    job->created_at = now_unix();
    job->work_dir = join_path(config_.incoming_dir(), job->id);
    job->archive = join_path(job->work_dir, "package" + ext);
    if (!make_dirs(job->work_dir)) {
        *error = "cannot create " + job->work_dir;
        return "";
    }
    *archive_path = job->archive;
    std::lock_guard<std::mutex> lock(mutex_);
    // 只保留最近的任务记录
    while (jobs_.size() >= kMaxJobs) {
        auto oldest = jobs_.begin();
        for (auto it = jobs_.begin(); it != jobs_.end(); ++it) {
            if (it->second->created_at < oldest->second->created_at) oldest = it;
        }
        if (oldest->second->state == "running") break;
        jobs_.erase(oldest);
    }
    jobs_[job->id] = job;
    return job->id;
}

void Jobs::fail(const std::string& id, const std::string& message) {
    std::shared_ptr<Job> job;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }
    set(job, "failed", message);
    if (!job->work_dir.empty()) remove_tree(job->work_dir);
}

void Jobs::start_import(const std::string& id) {
    std::shared_ptr<Job> job;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job = jobs_.at(id);
    }
    std::thread([this, job] { run_import(job); }).detach();
}

void Jobs::run_import(std::shared_ptr<Job> job) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->stage = "extracting";
        job->progress = -1;
    }
    const std::string out_dir = join_path(job->work_dir, "content");
    make_dirs(out_dir);
    std::vector<std::string> cmd;
    if (ends_with(job->archive, ".zip")) {
        cmd = {"unzip", "-q", job->archive, "-d", out_dir};
    } else {
        // GNU tar 和 busybox tar 都能自动识别 gzip
        cmd = {"tar", "-xf", job->archive, "-C", out_dir};
    }
    int code = -1;
    auto out = run_capture(cmd, 60 * 60 * 1000, 8192, &code);
    unlink(job->archive.c_str());  // 解压后立即删除压缩包，释放空间
    if (!out || code != 0) {
        set(job, "failed", "failed to extract package: " + (out ? trim(*out) : std::string("timeout")));
        remove_tree(job->work_dir);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->stage = "checking";
    }
    const std::string dir = find_model_dir(out_dir);
    if (dir.empty()) {
        set(job, "failed", "model.json not found in the package");
        remove_tree(job->work_dir);
        return;
    }
    ModelSpec spec = load_model_spec(dir, false);
    if (!spec.error.empty()) {
        set(job, "failed", spec.error);
        remove_tree(job->work_dir);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->model = spec.name;
        job->model_dir = dir;
    }
    if (config_.runtimes.find(spec.runtime) == config_.runtimes.end()) {
        set(job, "failed", "runtime \"" + spec.runtime + "\" is not installed on this device");
        remove_tree(job->work_dir);
        return;
    }

    if (!spec.files.empty()) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            job->stage = "verifying";
            job->progress = 0;
        }
        auto r = verify_model_files(spec, [&](uint64_t done, uint64_t total) {
            std::lock_guard<std::mutex> lock(mutex_);
            job->progress = total ? double(done) / double(total) : 1.0;
        });
        if (!r.ok) {
            std::string msg = "file check failed:";
            for (const auto& f : r.missing) msg += " missing " + f + ";";
            for (const auto& f : r.mismatch) msg += " sha256 mismatch " + f + ";";
            set(job, "failed", msg);
            remove_tree(job->work_dir);
            return;
        }
    }

    // 同名模型
    for (const auto& m : repo_.list()) {
        if (m.name == spec.name) {
            std::lock_guard<std::mutex> lock(mutex_);
            job->state = "conflict";
            job->message = "model \"" + spec.name + "\" already exists (version " +
                           (m.version.empty() ? "unknown" : m.version) + "), new version " +
                           (spec.version.empty() ? "unknown" : spec.version);
            job->result = {{"existing_version", m.version}, {"new_version", spec.version}};
            return;
        }
    }
    install(job);
}

std::string Jobs::confirm_import(const std::string& id, bool overwrite) {
    std::shared_ptr<Job> job;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return "job not found";
        job = it->second;
        if (job->state != "conflict") return "job is not waiting for confirmation";
        if (!overwrite) {
            job->state = "cancelled";
            job->message = "";
        }
    }
    if (!overwrite) {
        remove_tree(job->work_dir);
        return "";
    }
    if (supervisor_.is_active(job->model)) return "stop the running model \"" + job->model + "\" first";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->state = "running";
        job->message = "";
    }
    std::thread([this, job] { install(job); }).detach();
    return "";
}

void Jobs::install(std::shared_ptr<Job> job) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->stage = "installing";
        job->progress = -1;
    }
    if (supervisor_.is_active(job->model)) {
        set(job, "conflict", "stop the running model \"" + job->model + "\" first");
        return;
    }
    // 旧版本所在目录（目录名不一定等于模型名）
    std::string target = join_path(config_.models_dir, job->model);
    for (const auto& m : repo_.list()) {
        if (m.name == job->model) target = m.dir;
    }
    const std::string old_dir = join_path(job->work_dir, "old");
    if (path_exists(target) && rename(target.c_str(), old_dir.c_str()) != 0) {
        set(job, "failed", "cannot move the old version away: " + std::string(strerror(errno)));
        return;
    }
    if (rename(job->model_dir.c_str(), target.c_str()) != 0) {
        std::string err = strerror(errno);
        if (path_exists(old_dir)) rename(old_dir.c_str(), target.c_str());  // 回滚
        set(job, "failed", "cannot install model: " + err);
        return;
    }
    remove_tree(job->work_dir);
    repo_.scan();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        job->progress = 1;
    }
    set(job, "done", "");
    log_info("installed model " + job->model + " -> " + target);
}

std::string Jobs::start_verify(const std::string& model_name) {
    auto spec = repo_.find(model_name);
    if (!spec) return "";
    auto job = std::make_shared<Job>();
    job->id = random_hex(8);
    job->kind = "verify";
    job->state = "running";
    job->stage = "verifying";
    job->model = model_name;
    job->created_at = now_unix();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        jobs_[job->id] = job;
    }
    const ModelSpec s = *spec;
    std::thread([this, job, s] {
        if (s.files.empty()) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                job->result = {{"checked", 0}};
                job->progress = 1;
            }
            set(job, "done", "model.json has no file list, nothing to verify");
            return;
        }
        auto r = verify_model_files(s, [&](uint64_t done, uint64_t total) {
            std::lock_guard<std::mutex> lock(mutex_);
            job->progress = total ? double(done) / double(total) : 1.0;
        });
        {
            std::lock_guard<std::mutex> lock(mutex_);
            job->result = {{"checked", r.checked}, {"missing", r.missing}, {"mismatch", r.mismatch}};
            job->progress = 1;
        }
        set(job, r.ok ? "done" : "failed", r.ok ? "" : "some files are missing or modified");
    }).detach();
    return job->id;
}

}  // namespace axbox
