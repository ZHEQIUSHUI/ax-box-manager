# model.json 规范

模型仓库（默认 `/data/models`）里每个模型一个子目录，目录里放一个 `model.json`。管理平台只认 `model.json`，不针对具体模型写死逻辑。

```
/data/models/
├── qwen3-0.6b/
│   ├── model.json
│   ├── config.json          # axllm 自己的配置
│   └── *.axmodel ...
└── qwen3-vl-2b/
    ├── model.json
    └── ...
```

## 字段

| 字段 | 必填 | 说明 |
|---|---|---|
| `name` | 否 | API 调用时的模型名（请求里的 `model`）。只能用字母、数字和 `. _ -`。默认是目录名 |
| `version` | 否 | 模型版本，导入同名模型时用于提示 |
| `type` | 否 | `chat` / `vlm` / `embedding` / `asr` / `tts` 等，默认 `chat`。对话测试页只列出 `chat`、`vlm` 等可对话的类型 |
| `description` | 否 | 说明 |
| `max_context` | 否 | 最大上下文长度。为 0 时显示后端 `/v1/models` 返回的 `max_token_len` |
| `mem_required.cmm_mb` | 否 | 运行所需 NPU 内存（CMM），MB。启动前检查，不够则拒绝启动 |
| `mem_required.ram_mb` | 否 | 运行所需系统内存，MB |
| `runtime` | 是 | 推理程序名，对应管理服务配置 `runtimes` 里的键，如 `axllm`。可以写成字符串，或 `{"name": "axllm", "min_version": ""}` |
| `launch.args` | 否 | 启动参数模板。`runtime` 为 `axllm` 时默认 `["serve", "{model_dir}", "--port", "{port}"]`，其他运行时必填 |
| `launch.env` | 否 | 额外环境变量 |
| `launch.health_path` | 否 | 就绪检查路径，默认 `/v1/models`；空字符串表示进程启动即视为就绪 |
| `launch.served_model` | 否 | 后端实际使用的模型名。不填时启动后从后端 `/v1/models` 读取第一个模型 id，网关转发时自动改写 |
| `launch.start_timeout_s` | 否 | 等待就绪的最长时间，默认用管理服务配置（600 秒） |
| `files` | 否 | `{相对路径: sha256}`，导入和“校验文件”时检查 |

启动参数里可以使用的变量：

| 变量 | 值 |
|---|---|
| `{model_dir}` | 模型目录的绝对路径 |
| `{port}` | 平台分配的端口（从 18001 开始） |
| `{name}` | 模型名 |

## 示例

最简（axllm 模型）：

```json
{
  "name": "qwen3-0.6b",
  "version": "1.0",
  "type": "chat",
  "max_context": 2559,
  "mem_required": {"cmm_mb": 1300, "ram_mb": 300},
  "runtime": "axllm"
}
```

其他 OpenAI 兼容服务：

```json
{
  "name": "sensevoice",
  "type": "asr",
  "runtime": "asr-server",
  "launch": {
    "args": ["--model", "{model_dir}", "--host", "127.0.0.1", "--port", "{port}"],
    "env": {"OMP_NUM_THREADS": "2"}
  }
}
```

`asr-server` 需要在管理服务配置的 `runtimes` 里登记可执行文件路径。

## mem_required 怎么填

在设备上单独启动模型，看概览页“NPU 内存”增加了多少，再留一些余量。实测（AX650N，axllm 5988359）：

| 模型 | CMM |
|---|---|
| Qwen3-0.6B（w8a16） | 约 1255 MB |
| Qwen3-1.7B（w8a16） | 约 2480 MB |
| Qwen3-VL-2B（GPTQ int4，c128 p1152） | 约 2340 MB |
| SmolLM2-360M | 约 720 MB（含 axllm 的 128 MB 保护余量） |

填小了也不会损坏设备：axllm 自带内存保护，加载中途发现不够会安全退出，平台显示“启动时进程退出”和日志。

## 生成 model.json

```bash
scripts/make_model_json.py /data/models/qwen3-0.6b --name qwen3-0.6b --type chat \
    --max-context 2559 --cmm-mb 1300 --ram-mb 300
```

默认会计算目录下所有文件的 sha256 写进 `files`（大模型需要一些时间），加 `--no-hash` 跳过。

## 模型包

导入用的模型包是一个 `.tar` / `.tar.gz` / `.tgz` / `.zip`，里面是模型目录本身（或者直接是目录里的文件）。

```bash
tar -czf qwen3-0.6b.tar.gz -C /path/to/models qwen3-0.6b
```

导入时平台会解压、检查 `model.json`、按 `files` 校验 sha256，再放进模型仓库。同名模型已存在时会先询问是否覆盖；正在运行的模型需要先停止。
