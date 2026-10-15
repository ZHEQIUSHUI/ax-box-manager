#pragma once

#include "config.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace axbox {

/**
 * 模型描述（model.json），格式见 docs/model-json.md
 */
struct ModelSpec {
    std::string name;            // API 调用时的 model 名
    std::string dir;             // 模型目录绝对路径
    std::string version;
    std::string type;            // chat / vlm / embedding / asr / tts ...
    std::string description;
    int max_context = 0;
    int mem_cmm_mb = 0;          // 运行所需 NPU 内存（CMM）
    int mem_ram_mb = 0;          // 运行所需系统内存
    std::string runtime;         // 推理程序名，对应配置里的 runtimes
    std::string runtime_min_version;
    std::vector<std::string> args;            // 启动参数模板，支持 {model_dir} {port} {name}
    std::map<std::string, std::string> env;   // 额外环境变量
    std::string health_path = "/v1/models";
    std::string served_model;    // 后端实际的模型名；空表示启动后从后端 /v1/models 读取
    int start_timeout_s = 0;     // 0 表示用全局配置
    std::map<std::string, std::string> files;  // 相对路径 -> sha256

    uint64_t size_bytes = 0;
    std::string error;           // model.json 有问题时的说明；非空表示该模型不可启动

    nlohmann::json to_json() const;
};

// 解析一个模型目录下的 model.json
ModelSpec load_model_spec(const std::string& dir, bool compute_size = true);

// 检查 model.json 列出的文件：存在且 sha256 一致。progress 回调参数为（已处理字节，总字节）
struct VerifyResult {
    bool ok = true;
    std::vector<std::string> missing;
    std::vector<std::string> mismatch;
    int checked = 0;
};
VerifyResult verify_model_files(const ModelSpec& spec, const std::function<void(uint64_t, uint64_t)>& progress);

class ModelRepo {
public:
    explicit ModelRepo(const Config& config);

    // 重新扫描模型目录
    void scan();
    std::vector<ModelSpec> list() const;
    std::optional<ModelSpec> find(const std::string& name) const;

private:
    const Config& config_;
    mutable std::mutex mutex_;
    std::vector<ModelSpec> models_;
};

}  // namespace axbox
