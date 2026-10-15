# axbox-manager

大模型一体机的设备与模型管理平台。服务运行在设备上，用户用浏览器打开设备地址，像管理路由器一样查看设备状态、管理模型；业务系统只通过一个统一的 API 地址、按模型名调用大模型。

当前在 AX650N 开发板上开发和验证，后续迁移到 AX8860。

## 功能

- **登录**：使用设备的系统账号密码（读 `/etc/shadow`，用系统 `crypt()` 校验，支持 yescrypt / sha512）。不提供修改密码；连续失败 5 次锁定 5 分钟。
- **概览**：芯片、系统、SDK、运行时版本；CPU、内存、NPU 内存（按模型分段显示）、NPU 利用率（默认关闭，开关临时开启，到时自动关闭）、温度、存储的实时数据和 10 分钟曲线；运行中模型的最近一次调用（token 占用 / 最大上下文、首字延迟、生成速度）。
- **模型**：扫描模型仓库；启动、停止、重启；开机启动；启动前检查内存；导入模型包（上传、解压、校验 sha256、同名覆盖确认）；校验文件；删除。
- **统一网关**：基于 [openai-api.cpp](https://github.com/ZHEQIUSHUI/openai-api.cpp) 的 `openai_api_gateway`，一个端口按 `model` 字段把请求原样转发给各模型进程（流式、图片、`tool_calls` 等字段都不改写）；可选 API Key。
- **对话测试**：选择运行中的模型直接对话，可调 system prompt、temperature、top_p、max_tokens、思考模式，支持图片；生成 curl / Python 调用示例。设置只在当前页面生效。
- **日志**：管理服务、网关、各模型进程的日志，在线查看和下载。
- **设置**：API Key 开关和重新生成；语言、主题；重启设备（先正常停止所有模型）。
- 中英双语，跟随浏览器语言；适配手机；前端资源全部内置，不依赖外网。

## 架构

```
浏览器 ──► axbox-manager :8080（网页 + /api/*）
              │ ① 启动 / 停止模型进程（setsid，一个模型一个进程）
              │ ② 向网关注册 / 注销上游（POST/DELETE /admin/upstreams，仅本机）
              ▼
业务客户端 ──► openai_api_gateway :8000（可选 API Key）
              │ 按 model 字段透传
      ┌───────┼────────┐
      ▼       ▼        ▼
   axllm    axllm    其他 OpenAI 兼容服务
  :18001   :18002    :180xx      ← 端口由平台分配，iptables 拒绝非本机访问
```

- 管理服务重启（或升级）不影响正在运行的模型：模型进程独立于管理服务，管理服务启动时按 `run/<模型>.json` 记录的 pid 重新接管。
- 停止模型时先从网关注销，再发 SIGINT，超时后 SIGTERM，最后才 SIGKILL（NPU 进程被强杀可能导致 NPU 卡死，需要重启设备）。
- 模型进程意外退出时自动从网关注销并重启；10 分钟内崩溃 3 次则停止重启，标记为异常。
- 同一时间只启动一个模型，避免并发加载时内存检查失准。

## 目录结构

```
ax-box-manager/
├── src/                      管理服务（C++17）
│   ├── main.cpp              HTTP 接口、登录拦截、对话测试转发、静态网页
│   ├── auth.*                系统账号登录、会话、失败锁定
│   ├── sysinfo.*             设备信息与实时采样（/proc/ax_proc、/proc、/sys）
│   ├── model_repo.*          扫描模型仓库、解析 model.json、sha256 校验
│   ├── supervisor.*          网关和模型进程的启停、接管、崩溃重启、端口隔离
│   ├── jobs.*                后台任务：模型包导入、文件校验
│   ├── config.*              config.json
│   └── util.*                文件、日志、sha256、子进程
├── web/                      前端（原生 JS，构建时编进程序）
├── third_party/openai-api.cpp  统一网关（git submodule）
├── packaging/                安装脚本、systemd 服务、示例配置
├── scripts/
│   ├── package.sh            生成发布包
│   ├── make_model_json.py    为模型目录生成 model.json
│   └── mock_model.py         开发用的假模型服务
├── cmake/embed_web.cmake     把 web/ 生成为 C++ 数组
└── docs/model-json.md        model.json 规范
```

## 构建

```bash
git clone --recursive <repo>
cmake -S . -B build && cmake --build build -j
```

交叉编译到 AX650 / AX8860（aarch64）：

```bash
export PATH=/path/to/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin:$PATH
cmake -S . -B build_a64 -DCMAKE_TOOLCHAIN_FILE=/path/to/ax-llm/toolchains/aarch64-none-linux-gnu.toolchain.cmake
cmake --build build_a64 -j
```

产物：`build_a64/axbox-manager`、`build_a64/third_party/openai-api.cpp/openai_api_gateway`。推理运行时 `axllm` 用 [ax-llm](https://github.com/AXERA-TECH/ax-llm) 的 `axllm` 分支单独编译（需要 2026-10-15 的 `5988359` 及以后，修复了同端口重启误报“端口被占用”）。

## 打包和安装

```bash
STRIP=aarch64-none-linux-gnu-strip scripts/package.sh build_a64 /path/to/axllm
# 生成 axbox-0.1.0-aarch64.tar.gz

# 在设备上（root）
tar -xzf axbox-0.1.0-aarch64.tar.gz && cd axbox-0.1.0-aarch64
sh install.sh --models-dir /data/models     # 首次安装生成 /etc/axbox/config.json；升级时保留原配置
```

安装位置：程序 `/opt/axbox/bin/`，配置 `/etc/axbox/config.json`，数据和日志 `/var/lib/axbox/`，systemd 服务 `axbox.service`（`KillMode=process`，重启服务不影响运行中的模型）。

卸载：`sh uninstall.sh`（停止服务和模型，删除程序和防火墙规则，保留配置、数据和模型）。

## 配置 `/etc/axbox/config.json`

| 字段 | 默认值 | 说明 |
|---|---|---|
| `listen_host` / `listen_port` | `0.0.0.0` / `8080` | 管理网页地址 |
| `gateway_host` / `gateway_port` | `0.0.0.0` / `8000` | 统一网关（业务 API）地址 |
| `models_dir` | `/data/models` | 模型仓库 |
| `data_dir` | `/var/lib/axbox` | 设置、日志、运行状态 |
| `gateway_bin` | `/opt/axbox/bin/openai_api_gateway` | 网关程序 |
| `runtimes` | `{"axllm": "/opt/axbox/bin/axllm"}` | `model.json` 里 `runtime` 对应的程序 |
| `env` | `{}` | 所有模型进程的环境变量。AX650 板端需要 `{"LD_LIBRARY_PATH": "/soc/lib"}` |
| `model_port_base` / `model_port_count` | `18001` / `1000` | 模型进程端口段 |
| `isolate_model_ports` | `true` | 用 iptables 拒绝非本机访问模型端口段（见“安全”） |
| `start_timeout_s` | `600` | 等待模型就绪的最长时间 |
| `stop_timeout_s` | `60` | SIGINT 后等待退出的时间 |
| `auth_mode` | `system` | `none` 表示不校验登录，仅用于开发 |
| `allowed_users` / `allowed_groups` | `["root"]` / `[]` | 允许登录网页的账号和组 |
| `session_ttl_s` | `43200` | 登录有效期 |
| `npu_top_minutes` | `10` | NPU 利用率统计默认关闭，在概览页临时开启；开启超过这个时间、或 1 分钟没人查看时自动关闭 |
| `web_dir` | 空 | 非空时从该目录读取网页（前端开发用），否则用内置网页 |

## 接口

管理接口都在管理端口上，需要登录（cookie）；修改类请求必须带请求头 `X-Axbox: 1`（防跨站请求）。

| 接口 | 说明 |
|---|---|
| `POST /api/login` `{user, password}` / `POST /api/logout` / `GET /api/info` | 登录 |
| `GET /api/device` | 设备信息 |
| `GET /api/status?since=<ms>` | 当前采样、历史曲线、网关状态、NPU 统计开关状态 |
| `PUT /api/npu-top` `{enabled}` | 临时开启 / 关闭 NPU 利用率统计 |
| `GET /api/models[?rescan=1]` | 模型列表（含运行状态和网关调用统计） |
| `POST /api/models/{name}/start\|stop\|restart` | 启停（异步，轮询模型列表看状态） |
| `PUT /api/models/{name}/autostart` `{enabled}` | 开机启动 |
| `POST /api/models/{name}/verify` | 校验文件，返回任务 |
| `DELETE /api/models/{name}` | 删除（需先停止） |
| `PUT /api/upload?filename=x.tar.gz` | 上传模型包，请求体就是文件本身，返回任务 |
| `GET /api/jobs/{id}` / `POST /api/jobs/{id}/confirm` `{overwrite}` | 任务进度 / 同名覆盖确认 |
| `GET /api/logs/{manager\|gateway\|模型名}?lines=300[&download=1]` | 日志 |
| `GET /api/settings` / `PUT /api/settings/api-key` `{enabled, generate}` | 设置 |
| `POST /api/system/reboot` | 重启设备 |
| `POST /v1/chat/completions` 等 | 对话测试：带登录 cookie，经网关转发 |

业务调用走网关：`http://<设备IP>:8000/v1/chat/completions`，开启 API Key 后带 `Authorization: Bearer <key>`。

## 安全

- 网页必须登录；网关的 `/admin/upstreams` 只接受本机请求。
- **模型端口**：`axllm serve` 目前固定监听 `0.0.0.0`，局域网可以绕过网关和 API Key 直接访问模型端口。平台默认用 `iptables -I INPUT -p tcp --dport 18001:19000 ! -i lo -j DROP` 拦截。根本的解决办法是让运行时支持只监听本机（如给 axllm 加 `--host 127.0.0.1`），之后可以在 `model.json` 的启动参数里指定，并关闭 `isolate_model_ports`。
- API Key 保存在 `/var/lib/axbox/settings.json`（权限 600）。
- 不做 HTTPS（需求确认不需要）。

## 开发

```bash
cmake --build build -j
cat > dev.json <<EOF
{"listen_port": 18080, "gateway_port": 18000, "model_port_base": 18101,
 "models_dir": "/tmp/axbox/models", "data_dir": "/tmp/axbox/data",
 "gateway_bin": "$PWD/build/third_party/openai-api.cpp/openai_api_gateway",
 "runtimes": {"mock": "/usr/bin/python3"}, "auth_mode": "none", "web_dir": "$PWD/web"}
EOF
./build/axbox-manager -c dev.json
```

`scripts/mock_model.py` 是一个假的 OpenAI 兼容模型服务（支持流式、思考内容、`--crash-after N` 模拟崩溃），在 `model.json` 里用 `"runtime": "mock", "launch": {"args": ["/path/to/mock_model.py", "--port", "{port}"]}` 引用。设置了 `web_dir` 后修改前端只需刷新浏览器。

## 迁移到 AX8860

平台本身不依赖芯片型号，迁移时主要检查：

1. **推理运行时**：用 8860 的 BSP 编译 `axllm`，在 `runtimes` 里指向它；`env` 里的 `LD_LIBRARY_PATH` 改成 8860 板端库目录。
2. **procfs**：NPU 内存读 `/proc/ax_proc/mem_cmm_info`（`CMM_USE_INFO` 汇总行），NPU 利用率读 `/proc/ax_proc/npu/top`（`utilization:xx%`），芯片型号读 `/proc/ax_proc/chip_type`。8860 上路径或格式不同时改 `src/sysinfo.cpp`。
3. **多进程共享 NPU**：AX650N 上已验证 2 个 axllm 进程同时运行（Qwen3-0.6B + Qwen3-1.7B，或 Qwen3-0.6B + Qwen3-VL-2B）；8860 上需要专项验证（之前在 8860 上发现多核同步类模型会导致驱动崩溃）。
4. **系统**：如果量产机是没有 iptables 的精简系统，需要运行时支持只监听本机后再关闭 `isolate_model_ports`；登录依赖 `/etc/shadow` 和 libcrypt。

## 还没做

- 平台和推理运行时的在线升级（需求第二期）
- 网络、时间设置
- 模型在线下载（后续在 Release 维护 `models.json` 清单）
- PAM 登录
