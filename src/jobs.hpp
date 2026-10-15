#pragma once

#include "config.hpp"
#include "model_repo.hpp"
#include "supervisor.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace axbox {

/**
 * 后台任务：模型包导入、模型文件校验
 *
 * 导入流程：上传的压缩包 → 解压到 .incoming/<id>/ → 检查 model.json → 校验 sha256
 *          → 同名模型已存在时等待用户确认覆盖 → 移入模型目录
 */
class Jobs {
public:
    Jobs(const Config& config, ModelRepo& repo, Supervisor& supervisor);

    // 删除上次没有完成的导入
    void cleanup_incoming();

    // 新建导入任务，返回任务 id 和压缩包应写入的路径
    std::string create_import(const std::string& filename, std::string* archive_path, std::string* error);
    void fail(const std::string& id, const std::string& message);
    void start_import(const std::string& id);   // 压缩包写完后调用
    std::string confirm_import(const std::string& id, bool overwrite);

    std::string start_verify(const std::string& model_name);

    nlohmann::json get(const std::string& id) const;
    nlohmann::json list() const;

private:
    struct Job {
        std::string id;
        std::string kind;        // import / verify
        std::string state;       // running / done / failed / conflict / cancelled
        std::string stage;       // receiving / extracting / checking / verifying / installing
        double progress = 0;     // 0~1，未知时为 -1
        std::string message;
        std::string model;
        std::string work_dir;
        std::string archive;
        std::string model_dir;   // 解压出的模型目录
        int64_t created_at = 0;
        nlohmann::json result;
    };

    void run_import(std::shared_ptr<Job> job);
    void install(std::shared_ptr<Job> job);
    void set(const std::shared_ptr<Job>& job, const std::string& state, const std::string& message);
    nlohmann::json to_json(const Job& job) const;

    const Config& config_;
    ModelRepo& repo_;
    Supervisor& supervisor_;

    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<Job>> jobs_;
};

}  // namespace axbox
