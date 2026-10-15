'use strict';

// ============================================================ 文案
const I18N = {
  zh: {
    'login.title': '登录设备管理',
    'login.user': '用户名',
    'login.password': '密码',
    'login.submit': '登录',
    'login.hint': '使用设备的系统账号登录。',
    'login.bad': '用户名或密码不正确。',
    'login.locked': '失败次数过多，请 {s} 秒后再试。',
    'login.unavailable': '设备无法校验密码，请查看管理服务日志。',
    'nav.overview': '概览',
    'nav.models': '模型',
    'nav.playground': '对话测试',
    'nav.logs': '日志',
    'nav.settings': '设置',
    'nav.logout': '退出',
    'nav.running': '{n} 运行中',
    'common.cancel': '取消',
    'common.copy': '复制',
    'common.copied': '已复制',
    'common.unknown': '未知',
    'common.refresh': '刷新',
    'common.none': '无',
    'state.running': '运行中',
    'state.starting': '启动中',
    'state.stopping': '停止中',
    'state.stopped': '已停止',
    'state.error': '异常',
    'state.invalid': '不可用',
    'ov.title': '设备概览',
    'ov.cmm': 'NPU 内存（CMM）',
    'ov.cmm_used': '已用',
    'ov.cmm_of': '共 {total}',
    'ov.cmm_free': '空闲',
    'ov.cmm_other': '其他占用',
    'ov.cmm_none': '这台设备没有 CMM 信息。',
    'ov.cpu': 'CPU',
    'ov.mem': '系统内存',
    'ov.npu': 'NPU 利用率',
    'ov.npu_na': '已关闭',
    'ov.npu_switch': '临时开启 NPU 利用率统计',
    'ov.npu_off_hint': '统计默认关闭，打开后会自动关闭',
    'ov.npu_auto_off': '{m} 分钟后自动关闭',
    'ov.npu_external': '已由其他程序开启',
    'ov.temp': '芯片温度',
    'ov.disk': '模型存储',
    'ov.disk_free': '剩余 {v}',
    'ov.uptime': '开机时长',
    'ov.load': '负载 {v}',
    'ov.api': 'API 地址',
    'ov.api_hint': '业务系统用这个地址调用，按 model 字段选择模型。',
    'ov.api_key_on': '已开启 API Key 鉴权',
    'ov.api_key_off': '未开启 API Key 鉴权',
    'ov.gateway_down': '网关未运行',
    'ov.device': '设备信息',
    'ov.running_models': '运行中的模型',
    'ov.no_running': '没有运行中的模型。',
    'ov.go_models': '去启动模型',
    'dev.board': '设备型号',
    'dev.chip': '芯片',
    'dev.serial': '序列号',
    'dev.os': '系统',
    'dev.kernel': '内核',
    'dev.sdk': 'SDK',
    'dev.npu': 'NPU 驱动',
    'dev.platform': '平台版本',
    'dev.runtime': '推理运行时',
    'dev.mem': '内存',
    'dev.cores': 'CPU 核数',
    'dev.network': '网络',
    'm.title': '模型',
    'm.sub': '模型目录 {dir}',
    'm.rescan': '重新扫描',
    'm.import': '导入模型包',
    'm.import_hint': '选择或拖入模型压缩包（.tar / .tar.gz / .zip），包里需要有 model.json。也可以直接把模型目录拷进模型目录后点“重新扫描”。',
    'm.choose': '选择文件',
    'm.col.state': '状态',
    'm.col.model': '模型',
    'm.col.ctx': '最大上下文',
    'm.col.mem': '所需内存',
    'm.col.size': '大小',
    'm.col.autostart': '开机启动',
    'm.col.actions': '操作',
    'm.start': '启动',
    'm.stop': '停止',
    'm.restart': '重启',
    'm.details': '详情',
    'm.verify': '校验文件',
    'm.logs': '日志',
    'm.delete': '删除',
    'm.empty': '模型目录里还没有模型。导入一个模型包，或把模型目录拷进 {dir}。',
    'm.mem_value': 'NPU {cmm} / 内存 {ram}',
    'm.port': '端口',
    'm.pid': '进程',
    'm.rss': '进程内存',
    'm.uptime': '已运行',
    'm.requests': '请求 / 出错',
    'm.last_call': '最近一次调用',
    'm.ctx_used': '上下文占用',
    'm.ttft': '首字延迟',
    'm.speed': '生成速度',
    'm.dir': '目录',
    'm.runtime': '运行时',
    'm.served': '后端模型名',
    'm.last_error': '最近错误',
    'm.no_call': '还没有调用',
    'm.usage_unknown': '后端未返回用量',
    'm.confirm_stop': '停止 {name}？',
    'm.confirm_stop_text': '正在进行的调用会中断，API 调用该模型会返回“模型未启动”。',
    'm.confirm_delete': '删除 {name}？',
    'm.confirm_delete_text': '模型目录会从设备上删除，无法恢复。',
    'm.started': '正在启动 {name}，加载模型可能需要几分钟。',
    'm.stopped': '正在停止 {name}',
    'm.deleted': '已删除 {name}',
    'm.verify_ok': '{name}：{n} 个文件校验通过',
    'm.verify_none': '{name} 的 model.json 没有文件清单，无法校验',
    'm.verify_bad': '{name} 校验失败：缺失 {missing}，不一致 {mismatch}',
    'm.verifying': '正在校验 {name}…',
    'job.receiving': '正在上传',
    'job.extracting': '正在解压',
    'job.checking': '正在检查 model.json',
    'job.verifying': '正在校验文件',
    'job.installing': '正在安装',
    'job.done': '已导入 {name}',
    'job.failed': '导入失败：{msg}',
    'job.conflict': '模型 {name} 已存在（当前版本 {old}，新版本 {new}）。覆盖吗？',
    'job.overwrite': '覆盖',
    'job.cancelled': '已取消导入',
    'pg.title': '对话测试',
    'pg.sub': '这里的设置只在本页面生效，不会保存为设备的默认配置。',
    'pg.model': '模型',
    'pg.no_model': '没有运行中的对话模型，先到“模型”页启动一个。',
    'pg.system': 'System prompt',
    'pg.system_ph': '可选',
    'pg.temperature': 'temperature',
    'pg.top_p': 'top_p',
    'pg.max_tokens': 'max_tokens',
    'pg.thinking': '思考模式',
    'pg.thinking_default': '默认',
    'pg.thinking_on': '开',
    'pg.thinking_off': '关',
    'pg.clear': '清空对话',
    'pg.send': '发送',
    'pg.stop': '停止',
    'pg.input_ph': '输入消息，Enter 发送，Shift+Enter 换行',
    'pg.image': '图片',
    'pg.image_attached': '已附加图片',
    'pg.you': '你',
    'pg.thinking_label': '思考过程',
    'pg.stats': '输入 {p} + 输出 {c} = {t} tokens{ctx}，首字 {ttft}，{tps}',
    'pg.example': '调用示例',
    'pg.empty': '选择模型后开始对话。上下文由每次请求自己携带，这里会把本页面的历史消息一起发送。',
    'log.title': '日志',
    'log.target': '日志来源',
    'log.manager': '管理服务',
    'log.gateway': '网关',
    'log.lines': '行数',
    'log.auto': '自动刷新',
    'log.download': '下载完整日志',
    'log.empty': '（暂无日志）',
    'set.title': '设置',
    'set.api': 'API 鉴权',
    'set.api_desc': '开启后，调用 API 需要在请求头带上 Authorization: Bearer <API Key>。修改后网关会重启，进行中的调用会中断。',
    'set.api_enable': '开启 API Key 鉴权',
    'set.api_key': 'API Key',
    'set.show': '显示',
    'set.hide': '隐藏',
    'set.regen': '重新生成',
    'set.regen_confirm': '重新生成 API Key？',
    'set.regen_text': '旧的 API Key 会立即失效，使用它的业务系统需要更新。',
    'set.saved': '已保存，网关正在重启',
    'set.gateway': '统一网关',
    'set.gateway_addr': '地址',
    'set.gateway_state': '状态',
    'set.gateway_running': '运行中，已重启 {n} 次',
    'set.appearance': '界面',
    'set.language': '语言',
    'set.theme': '主题',
    'set.theme_auto': '跟随系统',
    'set.theme_light': '浅色',
    'set.theme_dark': '深色',
    'set.device': '设备',
    'set.reboot': '重启设备',
    'set.reboot_desc': '先正常停止所有模型，再重启系统。开机启动的模型会在重启后自动运行。',
    'set.reboot_confirm': '重启设备？',
    'set.reboot_text': '重启期间 API 不可用，大约需要一到几分钟。',
    'set.rebooting': '设备正在重启。',
    'err.network': '无法连接设备',
    'conn.lost': '与设备的连接已断开，正在重连…',
    'conn.rebooting': '设备正在重启，恢复后页面会自动刷新…',
  },
  en: {
    'login.title': 'Sign in to device console',
    'login.user': 'User name',
    'login.password': 'Password',
    'login.submit': 'Sign in',
    'login.hint': 'Use a system account of this device.',
    'login.bad': 'Wrong user name or password.',
    'login.locked': 'Too many failed attempts. Try again in {s} s.',
    'login.unavailable': 'The device cannot check passwords. See the manager log.',
    'nav.overview': 'Overview',
    'nav.models': 'Models',
    'nav.playground': 'Playground',
    'nav.logs': 'Logs',
    'nav.settings': 'Settings',
    'nav.logout': 'Sign out',
    'nav.running': '{n} running',
    'common.cancel': 'Cancel',
    'common.copy': 'Copy',
    'common.copied': 'Copied',
    'common.unknown': 'Unknown',
    'common.refresh': 'Refresh',
    'common.none': 'None',
    'state.running': 'Running',
    'state.starting': 'Starting',
    'state.stopping': 'Stopping',
    'state.stopped': 'Stopped',
    'state.error': 'Error',
    'state.invalid': 'Unavailable',
    'ov.title': 'Overview',
    'ov.cmm': 'NPU memory (CMM)',
    'ov.cmm_used': 'used',
    'ov.cmm_of': 'of {total}',
    'ov.cmm_free': 'Free',
    'ov.cmm_other': 'Other',
    'ov.cmm_none': 'No CMM information on this device.',
    'ov.cpu': 'CPU',
    'ov.mem': 'System memory',
    'ov.npu': 'NPU utilization',
    'ov.npu_na': 'Off',
    'ov.npu_switch': 'Turn on NPU utilization statistics for a while',
    'ov.npu_off_hint': 'Off by default; turns itself off later',
    'ov.npu_auto_off': 'turns off in {m} min',
    'ov.npu_external': 'turned on by another program',
    'ov.temp': 'Chip temperature',
    'ov.disk': 'Model storage',
    'ov.disk_free': '{v} free',
    'ov.uptime': 'Uptime',
    'ov.load': 'load {v}',
    'ov.api': 'API endpoint',
    'ov.api_hint': 'Applications call this address and pick a model with the model field.',
    'ov.api_key_on': 'API key required',
    'ov.api_key_off': 'API key not required',
    'ov.gateway_down': 'Gateway is not running',
    'ov.device': 'Device',
    'ov.running_models': 'Running models',
    'ov.no_running': 'No models are running.',
    'ov.go_models': 'Start a model',
    'dev.board': 'Board',
    'dev.chip': 'Chip',
    'dev.serial': 'Serial number',
    'dev.os': 'OS',
    'dev.kernel': 'Kernel',
    'dev.sdk': 'SDK',
    'dev.npu': 'NPU driver',
    'dev.platform': 'Platform',
    'dev.runtime': 'Runtimes',
    'dev.mem': 'Memory',
    'dev.cores': 'CPU cores',
    'dev.network': 'Network',
    'm.title': 'Models',
    'm.sub': 'Model directory {dir}',
    'm.rescan': 'Rescan',
    'm.import': 'Import a model package',
    'm.import_hint': 'Choose or drop a package (.tar / .tar.gz / .zip) that contains model.json. You can also copy a model folder into the model directory and click Rescan.',
    'm.choose': 'Choose file',
    'm.col.state': 'State',
    'm.col.model': 'Model',
    'm.col.ctx': 'Max context',
    'm.col.mem': 'Memory needed',
    'm.col.size': 'Size',
    'm.col.autostart': 'Start on boot',
    'm.col.actions': 'Actions',
    'm.start': 'Start',
    'm.stop': 'Stop',
    'm.restart': 'Restart',
    'm.details': 'Details',
    'm.verify': 'Verify files',
    'm.logs': 'Logs',
    'm.delete': 'Delete',
    'm.empty': 'No models yet. Import a package, or copy a model folder into {dir}.',
    'm.mem_value': 'NPU {cmm} / RAM {ram}',
    'm.port': 'Port',
    'm.pid': 'Process',
    'm.rss': 'Process memory',
    'm.uptime': 'Up for',
    'm.requests': 'Requests / errors',
    'm.last_call': 'Last call',
    'm.ctx_used': 'Context used',
    'm.ttft': 'Time to first token',
    'm.speed': 'Speed',
    'm.dir': 'Folder',
    'm.runtime': 'Runtime',
    'm.served': 'Backend model name',
    'm.last_error': 'Last error',
    'm.no_call': 'No calls yet',
    'm.usage_unknown': 'Backend returned no usage',
    'm.confirm_stop': 'Stop {name}?',
    'm.confirm_stop_text': 'Calls in progress are cut off, and API calls to this model return "model not running".',
    'm.confirm_delete': 'Delete {name}?',
    'm.confirm_delete_text': 'The model folder is removed from the device. This cannot be undone.',
    'm.started': 'Starting {name}. Loading can take a few minutes.',
    'm.stopped': 'Stopping {name}',
    'm.deleted': 'Deleted {name}',
    'm.verify_ok': '{name}: {n} files verified',
    'm.verify_none': '{name}: model.json has no file list to verify',
    'm.verify_bad': '{name} failed verification: {missing} missing, {mismatch} modified',
    'm.verifying': 'Verifying {name}…',
    'job.receiving': 'Uploading',
    'job.extracting': 'Extracting',
    'job.checking': 'Checking model.json',
    'job.verifying': 'Verifying files',
    'job.installing': 'Installing',
    'job.done': 'Imported {name}',
    'job.failed': 'Import failed: {msg}',
    'job.conflict': 'Model {name} already exists (installed {old}, new {new}). Replace it?',
    'job.overwrite': 'Replace',
    'job.cancelled': 'Import cancelled',
    'pg.title': 'Playground',
    'pg.sub': 'Settings here apply to this page only. They are not saved as device defaults.',
    'pg.model': 'Model',
    'pg.no_model': 'No chat model is running. Start one on the Models page.',
    'pg.system': 'System prompt',
    'pg.system_ph': 'Optional',
    'pg.temperature': 'temperature',
    'pg.top_p': 'top_p',
    'pg.max_tokens': 'max_tokens',
    'pg.thinking': 'Thinking',
    'pg.thinking_default': 'Default',
    'pg.thinking_on': 'On',
    'pg.thinking_off': 'Off',
    'pg.clear': 'Clear chat',
    'pg.send': 'Send',
    'pg.stop': 'Stop',
    'pg.input_ph': 'Type a message. Enter to send, Shift+Enter for a new line',
    'pg.image': 'Image',
    'pg.image_attached': 'Image attached',
    'pg.you': 'You',
    'pg.thinking_label': 'Thinking',
    'pg.stats': '{p} in + {c} out = {t} tokens{ctx}, first token {ttft}, {tps}',
    'pg.example': 'Code example',
    'pg.empty': 'Pick a model to start. Each request carries its own context; this page sends the conversation so far.',
    'log.title': 'Logs',
    'log.target': 'Source',
    'log.manager': 'Manager',
    'log.gateway': 'Gateway',
    'log.lines': 'Lines',
    'log.auto': 'Auto refresh',
    'log.download': 'Download full log',
    'log.empty': '(empty)',
    'set.title': 'Settings',
    'set.api': 'API authentication',
    'set.api_desc': 'When on, API calls must send Authorization: Bearer <API key>. Saving restarts the gateway and cuts off calls in progress.',
    'set.api_enable': 'Require an API key',
    'set.api_key': 'API key',
    'set.show': 'Show',
    'set.hide': 'Hide',
    'set.regen': 'Generate new key',
    'set.regen_confirm': 'Generate a new API key?',
    'set.regen_text': 'The old key stops working right away. Applications using it need the new one.',
    'set.saved': 'Saved. The gateway is restarting.',
    'set.gateway': 'Gateway',
    'set.gateway_addr': 'Address',
    'set.gateway_state': 'State',
    'set.gateway_running': 'Running, restarted {n} times',
    'set.appearance': 'Appearance',
    'set.language': 'Language',
    'set.theme': 'Theme',
    'set.theme_auto': 'System',
    'set.theme_light': 'Light',
    'set.theme_dark': 'Dark',
    'set.device': 'Device',
    'set.reboot': 'Reboot device',
    'set.reboot_desc': 'Stops all models cleanly, then reboots. Models set to start on boot run again afterwards.',
    'set.reboot_confirm': 'Reboot the device?',
    'set.reboot_text': 'The API is unavailable while the device restarts, usually one to a few minutes.',
    'set.rebooting': 'The device is rebooting.',
    'err.network': 'Cannot reach the device',
    'conn.lost': 'Lost connection to the device. Reconnecting…',
    'conn.rebooting': 'The device is rebooting. This page refreshes when it is back…',
  },
};

function store(key, value) {
  try {
    if (value === undefined) return localStorage.getItem(key);
    localStorage.setItem(key, value);
  } catch (e) { /* 隐私模式等情况下不可用 */ }
  return null;
}

let lang = store('axbox.lang') || ((navigator.language || 'zh').toLowerCase().startsWith('zh') ? 'zh' : 'en');

function t(key, vars) {
  let s = (I18N[lang] && I18N[lang][key]) || I18N.en[key] || key;
  if (vars) for (const k in vars) s = s.split('{' + k + '}').join(vars[k]);
  return s;
}

function applyI18n(root = document) {
  document.documentElement.lang = lang === 'zh' ? 'zh-CN' : 'en';
  root.querySelectorAll('[data-i18n]').forEach((el) => { el.textContent = t(el.dataset.i18n); });
  root.querySelectorAll('[data-i18n-ph]').forEach((el) => { el.placeholder = t(el.dataset.i18nPh); });
  document.querySelectorAll('.lang-seg').forEach((seg) => {
    seg.innerHTML = '';
    for (const [code, label] of [['zh', '中文'], ['en', 'English']]) {
      const b = document.createElement('button');
      b.type = 'button';
      b.textContent = label;
      b.setAttribute('aria-pressed', String(code === lang));
      b.onclick = () => { lang = code; store('axbox.lang', code); applyI18n(); render(); showConn(); };
      seg.appendChild(b);
    }
  });
}

function applyTheme() {
  const theme = store('axbox.theme') || 'auto';
  if (theme === 'auto') document.documentElement.removeAttribute('data-theme');
  else document.documentElement.setAttribute('data-theme', theme);
}

// ============================================================ 工具
const $ = (sel, root = document) => root.querySelector(sel);

function esc(s) {
  return String(s ?? '').replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}

function fmtBytes(b) {
  if (b == null || isNaN(b)) return '–';
  const u = ['B', 'KB', 'MB', 'GB', 'TB'];
  let i = 0;
  while (b >= 1024 && i < u.length - 1) { b /= 1024; i++; }
  return (b >= 100 || i === 0 ? b.toFixed(0) : b.toFixed(1)) + ' ' + u[i];
}
const fmtKB = (kb) => (kb == null ? '–' : fmtBytes(kb * 1024));
const fmtMB = (mb) => (mb ? fmtBytes(mb * 1024 * 1024) : '–');

function fmtDuration(s) {
  if (s == null || s < 0) return '–';
  s = Math.floor(s);
  const d = Math.floor(s / 86400), h = Math.floor((s % 86400) / 3600), m = Math.floor((s % 3600) / 60);
  if (lang === 'zh') {
    if (d) return `${d} 天 ${h} 小时`;
    if (h) return `${h} 小时 ${m} 分`;
    if (m) return `${m} 分 ${s % 60} 秒`;
    return `${s % 60} 秒`;
  }
  if (d) return `${d}d ${h}h`;
  if (h) return `${h}h ${m}m`;
  if (m) return `${m}m ${s % 60}s`;
  return `${s % 60}s`;
}

const fmtMs = (ms) => (ms == null ? '–' : ms >= 1000 ? (ms / 1000).toFixed(2) + ' s' : Math.round(ms) + ' ms');
const fmtTps = (v) => (v == null ? '–' : v.toFixed(1) + ' tokens/s');
const fmtNum = (n) => (n == null ? '–' : Number(n).toLocaleString());

function toast(msg, isError) {
  const el = $('#toast');
  el.textContent = msg;
  el.className = 'toast' + (isError ? ' error' : '');
  el.hidden = false;
  clearTimeout(toast.timer);
  toast.timer = setTimeout(() => { el.hidden = true; }, isError ? 7000 : 3500);
}

function confirmDialog(title, text, okLabel, danger) {
  return new Promise((resolve) => {
    const dlg = $('#confirm');
    $('#confirm-title').textContent = title;
    $('#confirm-text').textContent = text;
    const ok = $('#confirm-ok');
    ok.textContent = okLabel;
    ok.className = 'btn ' + (danger ? 'danger' : 'primary');
    $('#confirm-cancel').textContent = t('common.cancel');
    const done = (v) => { dlg.close(); resolve(v); };
    ok.onclick = () => done(true);
    $('#confirm-cancel').onclick = () => done(false);
    dlg.oncancel = () => resolve(false);
    dlg.showModal();
  });
}

async function copyText(text) {
  try {
    await navigator.clipboard.writeText(text);
  } catch (e) {
    // 非 HTTPS 页面没有 clipboard API
    const ta = document.createElement('textarea');
    ta.value = text;
    document.body.appendChild(ta);
    ta.select();
    document.execCommand('copy');
    ta.remove();
  }
  toast(t('common.copied'));
}

class ApiError extends Error {
  constructor(status, body) {
    super((body && body.error && body.error.message) || ('HTTP ' + status));
    this.status = status;
    this.body = body;
  }
}

async function api(method, path, body) {
  const opts = { method, headers: { 'X-Axbox': '1' }, credentials: 'same-origin' };
  if (body !== undefined) {
    opts.headers['Content-Type'] = 'application/json';
    opts.body = JSON.stringify(body);
  }
  let res;
  try {
    res = await fetch(path, opts);
  } catch (e) {
    setOnline(false);
    throw new ApiError(0, { error: { message: t('err.network') } });
  }
  setOnline(true);
  const text = await res.text();
  let data = null;
  try { data = text ? JSON.parse(text) : null; } catch (e) { data = text; }
  if (res.status === 401 && path !== '/api/login') {
    showLogin();
    throw new ApiError(401, data);
  }
  if (!res.ok) throw new ApiError(res.status, data);
  return data;
}

// ============================================================ 连接状态
// 断线时显示提示并继续轮询；设备重启时等它先断开再恢复，然后整页刷新
const conn = { online: true, rebooting: false, sawDown: false };

function showConn() {
  const el = $('#conn');
  if (!el) return;
  const msg = conn.rebooting ? t('conn.rebooting') : !conn.online ? t('conn.lost') : '';
  el.hidden = !msg;
  if (msg) el.innerHTML = `<span class="led"></span><span>${esc(msg)}</span>`;
}

function setOnline(online) {
  if (!online) conn.sawDown = true;
  if (conn.online === online) return;
  conn.online = online;
  showConn();
}

async function waitForReboot() {
  conn.rebooting = true;
  conn.sawDown = false;
  showConn();
  stopPolling();
  const started = Date.now();
  for (;;) {
    await new Promise((r) => setTimeout(r, 3000));
    try {
      const res = await fetch('/api/info', { cache: 'no-store', credentials: 'same-origin' });
      // 设备还没真正开始重启时请求仍会成功，等它断开过一次再算恢复
      if (res.ok && (conn.sawDown || Date.now() - started > 10 * 60 * 1000)) return location.reload();
    } catch (e) {
      conn.sawDown = true;
    }
  }
}

// ============================================================ 状态
const S = {
  info: null,
  device: null,
  history: [],
  now: {},
  gateway: {},
  models: [],
  settings: null,
  expanded: new Set(),
  job: null,
  chat: { model: '', messages: [], system: '', temperature: 0.7, top_p: 0.9, max_tokens: 1024, thinking: 'default', busy: false, abort: null, image: null },
  log: { target: 'manager', lines: 300, auto: true },
};

const page = () => (location.hash.replace(/^#\//, '').split('/')[0] || 'overview');

function modelState(m) {
  if (m.error) return 'invalid';
  return (m.status && m.status.state) || 'stopped';
}

function maxContext(m) {
  if (m.max_context) return m.max_context;
  const info = m.gateway && m.gateway.model_info;
  return (info && (info.max_token_len || info.max_context)) || 0;
}

function gatewayUrl() {
  return `http://${location.hostname}:${(S.device && S.device.gateway_port) || 8000}/v1`;
}

// ============================================================ 登录
function showLogin() {
  stopPolling();
  $('#app').hidden = true;
  $('#login').hidden = false;
  $('#login-host').textContent = (S.info && S.info.hostname) || location.hostname;
  applyI18n();
  setTimeout(() => $('#login-form [name=password]').focus(), 0);
}

$('#login-form').addEventListener('submit', async (e) => {
  e.preventDefault();
  const f = e.target;
  const err = $('#login-error');
  err.hidden = true;
  try {
    await api('POST', '/api/login', { user: f.user.value.trim(), password: f.password.value });
    f.password.value = '';
    await boot();
  } catch (ex) {
    const code = ex.body && ex.body.error && ex.body.error.code;
    err.textContent = code === 'locked' ? t('login.locked', { s: ex.body.error.retry_after })
      : code === 'unavailable' ? t('login.unavailable')
      : ex.status === 401 ? t('login.bad') : ex.message;
    err.hidden = false;
  }
});

$('#logout').addEventListener('click', async () => {
  try { await api('POST', '/api/logout'); } catch (e) { /* ignore */ }
  S.info.user = null;
  showLogin();
});

// ============================================================ 数据刷新
let pollTimer = null;

async function refreshStatus() {
  const since = S.history.length ? S.history[S.history.length - 1].t : 0;
  const d = await api('GET', '/api/status?since=' + since);
  S.now = d.now || {};
  S.gateway = d.gateway || {};
  S.npuTop = d.npu_top || {};
  S.history = S.history.concat(d.history || []);
  const cutoff = Date.now() - 10 * 60 * 1000;
  S.history = S.history.filter((p) => p.t >= cutoff);
}

async function refreshModels(rescan) {
  const d = await api('GET', '/api/models' + (rescan ? '?rescan=1' : ''));
  S.models = d.data || [];
  const running = S.models.filter((m) => modelState(m) === 'running').length;
  $('#nav-running').textContent = running ? t('nav.running', { n: running }) : '';
}

async function tick() {
  const p = page();
  try {
    // 没有数据要刷新的页面也探测一下连接
    if (!['overview', 'models', 'logs'].includes(p)) await api('GET', '/api/info');
    if (p === 'overview') await Promise.all([refreshStatus(), refreshModels()]);
    else if (p === 'models') await refreshModels();
    else if (p === 'logs' && S.log.auto) await loadLog();
    if (p === 'overview' || p === 'models') render();
  } catch (e) {
    if (e.status !== 401) console.warn(e);
  }
}

function startPolling() {
  stopPolling();
  pollTimer = setInterval(tick, 2500);
}
function stopPolling() {
  if (pollTimer) clearInterval(pollTimer);
  pollTimer = null;
}

// ============================================================ 页面：概览
function sparkline(points, max) {
  if (points.length < 2) return '<svg viewBox="0 0 100 40" preserveAspectRatio="none"></svg>';
  const t0 = Date.now() - 10 * 60 * 1000;
  const xs = (p) => Math.max(0, ((p.t - t0) / (10 * 60 * 1000)) * 100);
  const ys = (v) => 38 - (Math.min(v, max) / max) * 36;
  const line = points.map((p, i) => `${i ? 'L' : 'M'}${xs(p).toFixed(2)},${ys(p.v).toFixed(2)}`).join('');
  const area = `${line}L${xs(points[points.length - 1]).toFixed(2)},40L${xs(points[0]).toFixed(2)},40Z`;
  return `<svg viewBox="0 0 100 40" preserveAspectRatio="none" aria-hidden="true"><path class="spark-area" d="${area}"/><path class="spark-line" d="${line}"/></svg>`;
}

function series(fn) {
  return S.history.map((p) => ({ t: p.t, v: fn(p) })).filter((p) => p.v != null && !isNaN(p.v));
}

function metric(label, value, unit, extra, points, max, na) {
  const extraHtml = extra && extra.html != null ? extra.html : esc(extra || '');
  return `<div class="metric${na ? ' na' : ''}">
    <div class="label"><span>${esc(label)}</span><span>${extraHtml}</span></div>
    <div class="value">${esc(value)}${unit ? `<small>${esc(unit)}</small>` : ''}</div>
    ${points ? sparkline(points, max) : '<svg viewBox="0 0 100 40"></svg>'}
  </div>`;
}

function cmmPanel() {
  const n = S.now;
  if (!n.cmm_total_kb) return `<section class="panel"><h2>${t('ov.cmm')}</h2><p class="muted">${t('ov.cmm_none')}</p></section>`;
  const total = n.cmm_total_kb, used = n.cmm_used_kb;
  const colors = ['--seg-1', '--seg-2', '--seg-3', '--seg-4', '--seg-5'];
  const running = S.models.filter((m) => ['running', 'starting', 'stopping'].includes(modelState(m)));
  let assigned = 0;
  const segs = [];
  running.forEach((m, i) => {
    const kb = (m.mem_required && m.mem_required.cmm_mb ? m.mem_required.cmm_mb : 0) * 1024;
    if (!kb) return;
    const v = Math.min(kb, Math.max(0, used - assigned));
    assigned += v;
    segs.push({ label: m.name, kb: v, color: `var(${colors[i % colors.length]})` });
  });
  const other = Math.max(0, used - assigned);
  if (other > 0) segs.push({ label: t('ov.cmm_other'), kb: other, color: 'var(--seg-other)' });
  const bar = segs.map((s) => `<div style="width:${(s.kb / total) * 100}%;background:${s.color}" title="${esc(s.label)} ${fmtKB(s.kb)}"></div>`).join('');
  const legend = segs.map((s) => `<span><i style="background:${s.color}"></i>${esc(s.label)}<b>${fmtKB(s.kb)}</b></span>`).join('')
    + `<span><i style="background:var(--panel-2);border:1px solid var(--line)"></i>${t('ov.cmm_free')}<b>${fmtKB(total - used)}</b></span>`;
  return `<section class="panel">
    <h2>${t('ov.cmm')}</h2>
    <div class="cmm-figure"><span class="big">${fmtKB(used)}</span><span class="of">${t('ov.cmm_used')} · ${t('ov.cmm_of', { total: fmtKB(total) })}</span></div>
    <div class="cmm-bar" role="img" aria-label="${esc(t('ov.cmm'))}">${bar}</div>
    <div class="cmm-legend">${legend}</div>
  </section>`;
}

function lastCallText(m) {
  const g = m.gateway;
  const lc = g && g.last_call;
  if (!lc) return { ctx: t('m.no_call'), ttft: '–', tps: '–', pct: null };
  const u = lc.usage;
  if (!u || u.prompt_tokens == null) return { ctx: t('m.usage_unknown'), ttft: fmtMs(lc.ttft_ms), tps: fmtTps(lc.tokens_per_second), pct: null };
  const max = maxContext(m);
  const total = u.total_tokens ?? (u.prompt_tokens + (u.completion_tokens || 0));
  const pct = max ? Math.min(100, (total / max) * 100) : null;
  const ctx = `${fmtNum(u.prompt_tokens)} + ${fmtNum(u.completion_tokens || 0)} = ${fmtNum(total)}` + (max ? ` / ${fmtNum(max)} (${pct.toFixed(0)}%)` : '');
  return { ctx, ttft: fmtMs(lc.ttft_ms), tps: fmtTps(lc.tokens_per_second), pct };
}

function stateCell(m) {
  const s = modelState(m);
  return `<span class="state"><span class="led ${s}"></span>${t('state.' + s)}</span>`;
}

function renderOverview() {
  const n = S.now, d = S.device || {};
  const memPct = n.mem_total_kb ? (n.mem_used_kb / n.mem_total_kb) * 100 : null;
  const diskUsedPct = n.disk_total_kb ? (1 - n.disk_free_kb / n.disk_total_kb) * 100 : null;
  const npuTop = S.npuTop || {};
  const npuNa = !npuTop.enabled || n.npu == null;
  const npuNote = !npuTop.available ? '' : !npuTop.enabled ? t('ov.npu_off_hint')
    : npuTop.by_us ? t('ov.npu_auto_off', { m: Math.max(1, Math.ceil((npuTop.auto_off_in_s || 0) / 60)) }) : t('ov.npu_external');
  const npuSwitch = npuTop.available
    ? { html: `<label class="switch" title="${esc(t('ov.npu_switch'))}"><input type="checkbox" id="npu-top" ${npuTop.enabled ? 'checked' : ''} aria-label="${esc(t('ov.npu_switch'))}"></label>` }
    : '';
  const metrics = [
    metric(t('ov.cpu'), n.cpu != null ? n.cpu.toFixed(0) : '–', '%', d.cpu_cores ? `${d.cpu_cores} cores` : '', series((p) => p.cpu), 100),
    metric(t('ov.mem'), memPct != null ? memPct.toFixed(0) : '–', '%', n.mem_total_kb ? `${fmtKB(n.mem_used_kb)} / ${fmtKB(n.mem_total_kb)}` : '',
      series((p) => (p.mem_total_kb ? (p.mem_used_kb / p.mem_total_kb) * 100 : null)), 100),
    metric(t('ov.npu'), npuNa ? t('ov.npu_na') : n.npu.toFixed(0), npuNa ? '' : '%', npuSwitch, npuNa ? null : series((p) => p.npu), 100, npuNa)
      .replace('</div>\n    <div class="value">', `</div>\n    <div class="value">`)
      .replace(/<svg[^]*<\/svg>/, (svg) => (npuNote ? `<div class="metric-note">${esc(npuNote)}</div>` : '') + (npuNa ? '' : svg)),
    metric(t('ov.temp'), n.temp != null ? n.temp.toFixed(1) : '–', '°C', n.fan_rpm != null ? `${n.fan_rpm} rpm` : '', series((p) => p.temp), 110),
    metric(t('ov.disk'), diskUsedPct != null ? diskUsedPct.toFixed(0) : '–', '%', n.disk_free_kb != null ? t('ov.disk_free', { v: fmtKB(n.disk_free_kb) }) : '', null, 100),
    metric(t('ov.uptime'), fmtDuration(n.uptime_s), '', n.load1 != null ? t('ov.load', { v: n.load1.toFixed(2) }) : '', null, 1),
  ].join('');

  const running = S.models.filter((m) => ['running', 'starting', 'stopping', 'error'].includes(modelState(m)) && m.status && (m.status.pid || modelState(m) !== 'error'));
  const rows = running.map((m) => {
    const lc = lastCallText(m);
    const st = m.status;
    return `<tr>
      <td>${stateCell(m)}</td>
      <td class="model-name">${esc(m.name)}<small>${esc(m.type || '')}</small></td>
      <td class="hide-sm">${st.ready_at ? fmtDuration(Date.now() / 1000 - st.ready_at) : esc(st.message || '')}</td>
      <td class="num">${m.gateway ? `${fmtNum(m.gateway.requests)} / ${fmtNum(m.gateway.errors)}` : '–'}</td>
      <td>${esc(lc.ctx)}${lc.pct != null ? `<span class="ctx-meter"><i style="width:${lc.pct}%"></i></span>` : ''}</td>
      <td class="num hide-sm">${lc.ttft}</td>
      <td class="num hide-sm">${lc.tps}</td>
    </tr>`;
  }).join('');

  const net = (d.network || []).map((i) => `${esc(i.name)} ${esc(i.ipv4.join(', '))}${i.mac ? ` <span class="muted">${esc(i.mac)}</span>` : ''}`).join('<br>');
  const runtimes = Object.entries(d.runtimes || {}).map(([k, v]) => `${esc(k)} ${esc(v.version || (v.installed ? '' : '–'))}`).join('<br>') || '–';
  const gw = S.gateway || {};

  $('#main').innerHTML = `
    <div class="page-head"><div><h1>${t('ov.title')}</h1><p class="sub">${esc([d.chip, d.os].filter(Boolean).join('  ·  '))}</p></div></div>
    ${cmmPanel()}
    <section class="panel flush"><div class="metrics">${metrics}</div></section>
    <section class="panel flush">
      <div class="panel-head" style="padding:16px 18px 0"><h2>${t('ov.running_models')}</h2><a class="btn small" href="#/models">${t('nav.models')}</a></div>
      ${rows ? `<div class="table-wrap"><table>
        <thead><tr><th>${t('m.col.state')}</th><th>${t('m.col.model')}</th><th class="hide-sm">${t('m.uptime')}</th><th class="num">${t('m.requests')}</th><th>${t('m.ctx_used')}</th><th class="num hide-sm">${t('m.ttft')}</th><th class="num hide-sm">${t('m.speed')}</th></tr></thead>
        <tbody>${rows}</tbody></table></div>`
        : `<div class="empty">${t('ov.no_running')} <a href="#/models">${t('ov.go_models')}</a></div>`}
    </section>
    <div class="grid-2">
      <section class="panel">
        <h2>${t('ov.api')}</h2>
        <div class="endpoint"><code id="gw-url">${esc(gatewayUrl())}</code><button class="btn small" id="copy-gw">${t('common.copy')}</button></div>
        <p class="sub">${t('ov.api_hint')}</p>
        <p class="sub">${gw.running === false ? `<span class="err-text">${t('ov.gateway_down')}</span>` : gw.api_key_enabled ? t('ov.api_key_on') : t('ov.api_key_off')}</p>
      </section>
      <section class="panel">
        <h2>${t('ov.device')}</h2>
        <dl class="kv">
          <dt>${t('dev.board')}</dt><dd>${esc(d.board || '–')}</dd>
          <dt>${t('dev.chip')}</dt><dd>${esc(d.chip || '–')}</dd>
          <dt>${t('dev.serial')}</dt><dd class="mono">${esc(d.serial || '–')}</dd>
          <dt>${t('dev.os')}</dt><dd>${esc(d.os || '–')} <span class="muted">${esc(d.kernel || '')}</span></dd>
          <dt>${t('dev.sdk')}</dt><dd>${esc(d.sdk_version || '–')} ${d.npu_version ? `<span class="muted">NPU ${esc(d.npu_version)}</span>` : ''}</dd>
          <dt>${t('dev.runtime')}</dt><dd>${runtimes}</dd>
          <dt>${t('dev.mem')}</dt><dd>${fmtKB(d.mem_total_kb)}${d.cmm_total_kb ? ` <span class="muted">+ CMM ${fmtKB(d.cmm_total_kb)}</span>` : ''}</dd>
          <dt>${t('dev.network')}</dt><dd>${net || '–'}</dd>
          <dt>${t('dev.platform')}</dt><dd>${esc(d.platform_version || '')}</dd>
        </dl>
      </section>
    </div>`;
  $('#copy-gw').onclick = () => copyText(gatewayUrl());
  const npuEl = $('#npu-top');
  if (npuEl) {
    npuEl.onchange = async () => {
      try {
        S.npuTop = await api('PUT', '/api/npu-top', { enabled: npuEl.checked });
        await refreshStatus();
        render();
      } catch (e) { toast(e.message, true); npuEl.checked = !npuEl.checked; }
    };
  }
}

// ============================================================ 页面：模型
function modelRow(m) {
  const s = modelState(m);
  const st = m.status || {};
  const busy = ['starting', 'stopping'].includes(s);
  const active = ['running', 'starting'].includes(s) || (s === 'error' && st.pid);
  const name = esc(m.name);
  const actions = m.error ? '' : `
    ${active || busy ? `<button class="btn small" data-act="stop" data-name="${name}" ${s === 'stopping' ? 'disabled' : ''}>${t('m.stop')}</button>`
      : `<button class="btn small primary" data-act="start" data-name="${name}">${t('m.start')}</button>`}
    ${s === 'running' ? `<button class="btn small" data-act="restart" data-name="${name}">${t('m.restart')}</button>` : ''}
    <button class="btn small ghost" data-act="toggle" data-name="${name}" aria-expanded="${S.expanded.has(m.name)}">${t('m.details')}</button>`;
  const msg = m.error || (s !== 'running' && st.message ? st.message : '');
  const mem = m.mem_required || {};
  let html = `<tr>
    <td class="model-name">${name}<small>${esc([m.version && 'v' + m.version, m.type].filter(Boolean).join('  '))}</small>${msg ? `<div class="${s === 'error' || m.error ? 'err-text' : 'muted'}">${esc(msg)}</div>` : ''}</td>
    <td data-label="${esc(t('m.col.state'))}">${stateCell(m)}</td>
    <td class="num" data-label="${esc(t('m.col.ctx'))}">${maxContext(m) ? fmtNum(maxContext(m)) : '–'}</td>
    <td class="nowrap" data-label="${esc(t('m.col.mem'))}">${mem.cmm_mb || mem.ram_mb ? t('m.mem_value', { cmm: fmtMB(mem.cmm_mb), ram: fmtMB(mem.ram_mb) }) : '–'}</td>
    <td class="num nowrap hide-sm" data-label="${esc(t('m.col.size'))}">${fmtBytes(m.size_bytes)}</td>
    <td data-label="${esc(t('m.col.autostart'))}">${m.error ? '' : `<label class="switch"><input type="checkbox" data-act="autostart" data-name="${name}" ${st.autostart ? 'checked' : ''} aria-label="${esc(t('m.col.autostart'))}"></label>`}</td>
    <td data-label="">${actions ? `<div class="btn-row">${actions}</div>` : ''}</td>
  </tr>`;
  if (S.expanded.has(m.name)) {
    const lc = lastCallText(m);
    const g = m.gateway || {};
    html += `<tr class="detail"><td colspan="7">
      <div class="detail-grid">
        <div><span>${t('m.port')}</span>${st.port || '–'}</div>
        <div><span>${t('m.pid')}</span>${st.pid || '–'}</div>
        <div><span>${t('m.rss')}</span>${st.rss_kb ? fmtKB(st.rss_kb) : '–'}</div>
        <div><span>${t('m.uptime')}</span>${st.ready_at && s === 'running' ? fmtDuration(Date.now() / 1000 - st.ready_at) : '–'}</div>
        <div><span>${t('m.requests')}</span>${g.requests != null ? `${fmtNum(g.requests)} / ${fmtNum(g.errors)}` : '–'}</div>
        <div><span>${t('m.ctx_used')}</span>${esc(lc.ctx)}</div>
        <div><span>${t('m.ttft')}</span>${lc.ttft}</div>
        <div><span>${t('m.speed')}</span>${lc.tps}</div>
        <div><span>${t('m.runtime')}</span>${esc((m.runtime && m.runtime.name) || '–')}</div>
        <div><span>${t('m.served')}</span>${esc(st.served_model || '–')}</div>
        <div style="grid-column:1/-1"><span>${t('m.dir')}</span><code>${esc(m.dir)}</code></div>
        ${g.last_error ? `<div style="grid-column:1/-1"><span>${t('m.last_error')}</span><span class="err-text">${esc(g.last_error)}</span></div>` : ''}
      </div>
      <div class="btn-row" style="margin-top:12px">
        <a class="btn small" href="#/logs/${encodeURIComponent(m.name)}">${t('m.logs')}</a>
        <button class="btn small" data-act="verify" data-name="${name}">${t('m.verify')}</button>
        <button class="btn small danger" data-act="delete" data-name="${name}" ${active || busy ? 'disabled' : ''}>${t('m.delete')}</button>
      </div>
    </td></tr>`;
  }
  return html;
}

function jobHtml() {
  const j = S.job;
  if (!j) return '';
  if (j.state === 'done') return `<div class="job">${esc(t('job.done', { name: j.model }))}</div>`;
  if (j.state === 'cancelled') return `<div class="job muted">${t('job.cancelled')}</div>`;
  if (j.state === 'failed') return `<div class="job err-text">${esc(t('job.failed', { msg: j.message }))}</div>`;
  if (j.state === 'conflict') {
    const r = j.result || {};
    return `<div class="job"><p style="margin:0 0 8px">${esc(t('job.conflict', { name: j.model, old: r.existing_version || t('common.unknown'), new: r.new_version || t('common.unknown') }))}</p>
      ${j.message && !r.existing_version && !r.new_version ? `<p class="err-text">${esc(j.message)}</p>` : ''}
      <div class="btn-row"><button class="btn small primary" data-job="overwrite">${t('job.overwrite')}</button><button class="btn small" data-job="cancel">${t('common.cancel')}</button></div></div>`;
  }
  const pct = j.progress >= 0 ? Math.round(j.progress * 100) : null;
  return `<div class="job">${t('job.' + j.stage)}${pct != null ? ` ${pct}%` : ''}${j.model ? `  ${esc(j.model)}` : ''}
    <div class="progress${pct == null ? ' indet' : ''}"><i style="width:${pct || 0}%"></i></div></div>`;
}

function renderModels() {
  const dir = (S.device && S.device.models_dir) || '';
  const rows = S.models.map(modelRow).join('');
  $('#main').innerHTML = `
    <div class="page-head">
      <div><h1>${t('m.title')}</h1><p class="sub">${esc(t('m.sub', { dir }))}</p></div>
      <button class="btn" id="rescan">${t('m.rescan')}</button>
    </div>
    <section class="panel flush">
      ${rows ? `<div class="table-wrap"><table class="models-table">
        <thead><tr><th>${t('m.col.model')}</th><th>${t('m.col.state')}</th><th class="num">${t('m.col.ctx')}</th><th>${t('m.col.mem')}</th><th class="num hide-sm">${t('m.col.size')}</th><th>${t('m.col.autostart')}</th><th>${t('m.col.actions')}</th></tr></thead>
        <tbody id="model-rows">${rows}</tbody></table></div>`
        : `<div class="empty">${esc(t('m.empty', { dir }))}</div>`}
    </section>
    <section class="panel">
      <h2>${t('m.import')}</h2>
      <div class="drop" id="drop">
        <span class="muted">${t('m.import_hint')}</span>
        <label class="btn primary">${t('m.choose')}<input type="file" id="pkg" accept=".tar,.gz,.tgz,.zip" hidden></label>
      </div>
      <div id="job">${jobHtml()}</div>
    </section>`;
  bindModels();
}

function bindModels() {
  $('#rescan').onclick = async () => { await refreshModels(true); render(); };
  $('#main').querySelectorAll('[data-act]').forEach((el) => {
    const name = el.dataset.name;
    const act = el.dataset.act;
    if (act === 'autostart') {
      el.onchange = async () => {
        try { await api('PUT', `/api/models/${encodeURIComponent(name)}/autostart`, { enabled: el.checked }); }
        catch (e) { toast(e.message, true); el.checked = !el.checked; }
      };
      return;
    }
    el.onclick = () => modelAction(act, name);
  });
  const drop = $('#drop');
  const file = $('#pkg');
  file.onchange = () => { if (file.files[0]) upload(file.files[0]); };
  drop.ondragover = (e) => { e.preventDefault(); drop.classList.add('over'); };
  drop.ondragleave = () => drop.classList.remove('over');
  drop.ondrop = (e) => {
    e.preventDefault();
    drop.classList.remove('over');
    if (e.dataTransfer.files[0]) upload(e.dataTransfer.files[0]);
  };
  $('#main').querySelectorAll('[data-job]').forEach((el) => {
    el.onclick = async () => {
      try {
        S.job = await api('POST', `/api/jobs/${S.job.id}/confirm`, { overwrite: el.dataset.job === 'overwrite' });
        renderJob();
        pollJob();
      } catch (e) { toast(e.message, true); }
    };
  });
}

async function modelAction(act, name) {
  try {
    if (act === 'toggle') {
      S.expanded.has(name) ? S.expanded.delete(name) : S.expanded.add(name);
      return render();
    }
    if (act === 'stop' && !(await confirmDialog(t('m.confirm_stop', { name }), t('m.confirm_stop_text'), t('m.stop'), true))) return;
    if (act === 'delete') {
      if (!(await confirmDialog(t('m.confirm_delete', { name }), t('m.confirm_delete_text'), t('m.delete'), true))) return;
      await api('DELETE', `/api/models/${encodeURIComponent(name)}`);
      S.expanded.delete(name);
      toast(t('m.deleted', { name }));
    } else if (act === 'verify') {
      toast(t('m.verifying', { name }));
      let job = await api('POST', `/api/models/${encodeURIComponent(name)}/verify`);
      while (job.state === 'running') {
        await new Promise((r) => setTimeout(r, 1000));
        job = await api('GET', `/api/jobs/${job.id}`);
      }
      const r = job.result || {};
      if (job.state === 'done') toast(r.checked ? t('m.verify_ok', { name, n: r.checked }) : t('m.verify_none', { name }));
      else toast(t('m.verify_bad', { name, missing: (r.missing || []).length, mismatch: (r.mismatch || []).length }), true);
    } else {
      await api('POST', `/api/models/${encodeURIComponent(name)}/${act}`);
      if (act === 'start' || act === 'restart') toast(t('m.started', { name }));
      if (act === 'stop') toast(t('m.stopped', { name }));
    }
    await refreshModels();
    render();
  } catch (e) {
    toast(e.message, true);
  }
}

function renderJob() {
  const el = $('#job');
  if (!el) return;
  el.innerHTML = jobHtml();
  bindModels();
}

function upload(file) {
  S.job = { state: 'running', stage: 'receiving', progress: 0, model: '' };
  renderJob();
  const xhr = new XMLHttpRequest();
  xhr.open('PUT', '/api/upload?filename=' + encodeURIComponent(file.name));
  xhr.setRequestHeader('X-Axbox', '1');
  xhr.upload.onprogress = (e) => {
    if (!e.lengthComputable) return;
    S.job.progress = e.loaded / e.total;
    const bar = $('#job .progress i');
    if (bar) bar.style.width = (S.job.progress * 100).toFixed(1) + '%';
    const txt = $('#job .job');
    if (txt && txt.firstChild) txt.firstChild.textContent = `${t('job.receiving')} ${Math.round(S.job.progress * 100)}% `;
  };
  xhr.onload = () => {
    let data = null;
    try { data = JSON.parse(xhr.responseText); } catch (e) { /* ignore */ }
    if (xhr.status === 401) return showLogin();
    if (xhr.status !== 200) {
      S.job = { state: 'failed', message: (data && data.error && data.error.message) || 'HTTP ' + xhr.status };
      return renderJob();
    }
    S.job = data;
    renderJob();
    pollJob();
  };
  xhr.onerror = () => { S.job = { state: 'failed', message: t('err.network') }; renderJob(); };
  xhr.send(file);
}

async function pollJob() {
  while (S.job && S.job.id && S.job.state === 'running') {
    await new Promise((r) => setTimeout(r, 1000));
    try { S.job = await api('GET', `/api/jobs/${S.job.id}`); } catch (e) { break; }
    if (page() === 'models') renderJob();
  }
  if (S.job && S.job.state === 'done') {
    await refreshModels();
    if (page() === 'models') render();
  }
}

// ============================================================ 页面：对话测试
function chatModels() {
  return S.models.filter((m) => modelState(m) === 'running' && !['embedding', 'asr', 'tts'].includes(m.type));
}

function buildRequest(stream) {
  const c = S.chat;
  const messages = [];
  if (c.system.trim()) messages.push({ role: 'system', content: c.system });
  for (const m of c.messages) {
    if (m.role === 'user' && m.image) {
      messages.push({ role: 'user', content: [{ type: 'text', text: m.content }, { type: 'image_url', image_url: { url: m.image } }] });
    } else if (m.role === 'assistant') {
      // 历史回复只带正文，不带思考过程；出错或被中断的空回复不发送
      const { answer } = splitThink(m);
      if (answer) messages.push({ role: 'assistant', content: answer });
    } else {
      messages.push({ role: m.role, content: m.content });
    }
  }
  const body = { model: c.model, messages, temperature: Number(c.temperature), top_p: Number(c.top_p), max_tokens: Number(c.max_tokens), stream };
  if (stream) body.stream_options = { include_usage: true };
  if (c.thinking !== 'default') {
    body.enable_thinking = c.thinking === 'on';
    body.chat_template_kwargs = { enable_thinking: c.thinking === 'on' };
  }
  return body;
}

function exampleCode(kind) {
  const c = S.chat;
  const key = S.settings && S.settings.api_key_enabled ? 'YOUR_API_KEY' : '';
  const body = { model: c.model || 'MODEL_NAME', messages: [] };
  if (c.system.trim()) body.messages.push({ role: 'system', content: c.system });
  body.messages.push({ role: 'user', content: lang === 'zh' ? '你好' : 'Hello' });
  body.temperature = Number(c.temperature);
  body.top_p = Number(c.top_p);
  body.max_tokens = Number(c.max_tokens);
  if (c.thinking !== 'default') body.enable_thinking = c.thinking === 'on';
  if (kind === 'curl') {
    return `curl ${gatewayUrl()}/chat/completions \\\n  -H "Content-Type: application/json" \\\n${key ? `  -H "Authorization: Bearer ${key}" \\\n` : ''}  -d '${JSON.stringify(body, null, 2).replace(/'/g, "'\\''")}'`;
  }
  const extra = body.enable_thinking != null ? `,\n    extra_body={"enable_thinking": ${body.enable_thinking ? 'True' : 'False'}}` : '';
  const msgs = JSON.stringify(body.messages, null, 4).replace(/\n/g, '\n    ');
  return `from openai import OpenAI\n\nclient = OpenAI(base_url="${gatewayUrl()}", api_key="${key || 'none'}")\nresp = client.chat.completions.create(\n    model="${body.model}",\n    messages=${msgs},\n    temperature=${body.temperature},\n    top_p=${body.top_p},\n    max_tokens=${body.max_tokens}${extra},\n)\nprint(resp.choices[0].message.content)`;
}

// 部分后端把思考过程放在 content 里的 <think>...</think> 中
function splitThink(m) {
  let think = m.reasoning || '';
  let answer = m.content || '';
  const open = answer.indexOf('<think>');
  if (open >= 0 && !answer.slice(0, open).trim()) {
    const close = answer.indexOf('</think>');
    if (close < 0) return { think: think + answer.slice(open + 7), answer: '' };
    think += answer.slice(open + 7, close);
    answer = answer.slice(close + 8);
  }
  return { think: think.trim(), answer: answer.replace(/^\s+/, '') };
}

function msgHtml(m) {
  if (m.role === 'user') {
    return `<div class="msg user"><div class="who">${t('pg.you')}${m.image ? `  ·  ${t('pg.image_attached')}` : ''}</div><div class="body">${esc(m.content)}</div></div>`;
  }
  const { think, answer } = splitThink(m);
  return `<div class="msg"><div class="who">${esc(m.model || '')}</div>
    ${think ? `<div class="think"><div class="who">${t('pg.thinking_label')}</div>${esc(think)}</div>` : ''}
    <div class="body">${esc(answer)}${m.pending ? '▍' : ''}</div>
    ${m.error ? `<div class="err-text">${esc(m.error)}</div>` : ''}
    ${m.stats ? `<div class="stats">${esc(m.stats)}</div>` : ''}</div>`;
}

function renderChatLog() {
  const log = $('#chat-log');
  if (!log) return;
  const stick = log.scrollHeight - log.scrollTop - log.clientHeight < 40;
  log.innerHTML = S.chat.messages.length ? S.chat.messages.map(msgHtml).join('') : `<div class="empty">${t('pg.empty')}</div>`;
  if (stick) log.scrollTop = log.scrollHeight;
}

function renderPlayground() {
  const c = S.chat;
  const models = chatModels();
  if (!models.find((m) => m.name === c.model)) c.model = models.length ? models[0].name : '';
  const exKind = store('axbox.example') || 'curl';
  $('#main').innerHTML = `
    <div class="page-head"><div><h1>${t('pg.title')}</h1><p class="sub">${t('pg.sub')}</p></div></div>
    <div class="play">
      <div class="play-side">
        <section class="panel" style="display:flex;flex-direction:column;gap:12px;margin:0">
          <label class="field"><span>${t('pg.model')}</span>
            <select id="pg-model">${models.map((m) => `<option ${m.name === c.model ? 'selected' : ''}>${esc(m.name)}</option>`).join('')}</select></label>
          ${models.length ? '' : `<p class="muted" style="margin:0">${t('pg.no_model')}</p>`}
          <label class="field"><span>${t('pg.system')}</span><textarea id="pg-system" rows="3" placeholder="${esc(t('pg.system_ph'))}">${esc(c.system)}</textarea></label>
          <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px">
            <label class="field"><span>${t('pg.temperature')}</span><input type="number" id="pg-temperature" min="0" max="2" step="0.1" value="${c.temperature}"></label>
            <label class="field"><span>${t('pg.top_p')}</span><input type="number" id="pg-top_p" min="0" max="1" step="0.05" value="${c.top_p}"></label>
            <label class="field"><span>${t('pg.max_tokens')}</span><input type="number" id="pg-max_tokens" min="1" step="1" value="${c.max_tokens}"></label>
          </div>
          <div class="field"><span>${t('pg.thinking')}</span>
            <span class="seg" id="pg-thinking">${['default', 'on', 'off'].map((v) => `<button type="button" data-v="${v}" aria-pressed="${c.thinking === v}">${t('pg.thinking_' + v)}</button>`).join('')}</span></div>
          <button class="btn" id="pg-clear">${t('pg.clear')}</button>
        </section>
        <section class="panel" style="margin:0">
          <div class="panel-head" style="margin-bottom:0"><h2>${t('pg.example')}</h2>
            <span class="seg" id="pg-ex">${['curl', 'python'].map((v) => `<button type="button" data-v="${v}" aria-pressed="${exKind === v}">${v}</button>`).join('')}</span></div>
          <pre class="code" id="pg-code">${esc(exampleCode(exKind))}</pre>
          <div class="btn-row" style="margin-top:8px"><button class="btn small" id="pg-copy">${t('common.copy')}</button></div>
        </section>
      </div>
      <section class="panel chat">
        <div class="chat-log" id="chat-log"></div>
        <div class="chat-input">
          <label class="btn small" title="${esc(t('pg.image'))}">${t('pg.image')}<input type="file" id="pg-image" accept="image/*" hidden></label>
          <textarea id="pg-input" rows="2" placeholder="${esc(t('pg.input_ph'))}"></textarea>
          <button class="btn primary" id="pg-send" ${c.model ? '' : 'disabled'}>${c.busy ? t('pg.stop') : t('pg.send')}</button>
        </div>
      </section>
    </div>`;
  renderChatLog();
  const upd = () => { $('#pg-code').textContent = exampleCode(store('axbox.example') || 'curl'); };
  $('#pg-model').onchange = (e) => { c.model = e.target.value; upd(); };
  $('#pg-system').oninput = (e) => { c.system = e.target.value; upd(); };
  for (const k of ['temperature', 'top_p', 'max_tokens']) $('#pg-' + k).oninput = (e) => { c[k] = e.target.value; upd(); };
  $('#pg-thinking').querySelectorAll('button').forEach((b) => {
    b.onclick = () => { c.thinking = b.dataset.v; $('#pg-thinking').querySelectorAll('button').forEach((x) => x.setAttribute('aria-pressed', String(x === b))); upd(); };
  });
  $('#pg-ex').querySelectorAll('button').forEach((b) => {
    b.onclick = () => { store('axbox.example', b.dataset.v); $('#pg-ex').querySelectorAll('button').forEach((x) => x.setAttribute('aria-pressed', String(x === b))); upd(); };
  });
  $('#pg-copy').onclick = () => copyText($('#pg-code').textContent);
  $('#pg-clear').onclick = () => { if (c.abort) c.abort.abort(); c.messages = []; renderChatLog(); };
  $('#pg-image').onchange = (e) => {
    const f = e.target.files[0];
    if (!f) return;
    const r = new FileReader();
    r.onload = () => { c.image = r.result; toast(t('pg.image_attached')); };
    r.readAsDataURL(f);
  };
  const input = $('#pg-input');
  input.onkeydown = (e) => { if (e.key === 'Enter' && !e.shiftKey && !e.isComposing) { e.preventDefault(); send(); } };
  $('#pg-send').onclick = () => (c.busy ? c.abort && c.abort.abort() : send());
}

async function send() {
  const c = S.chat;
  const input = $('#pg-input');
  const text = input.value.trim();
  if (!text || !c.model || c.busy) return;
  input.value = '';
  c.messages.push({ role: 'user', content: text, image: c.image });
  c.image = null;
  const reply = { role: 'assistant', model: c.model, content: '', reasoning: '', pending: true };
  const body = buildRequest(true);
  c.messages.push(reply);
  c.busy = true;
  c.abort = new AbortController();
  $('#pg-send').textContent = t('pg.stop');
  renderChatLog();
  const t0 = performance.now();
  let tFirst = null, usage = null;
  try {
    const res = await fetch('/v1/chat/completions', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'X-Axbox': '1' },
      body: JSON.stringify(body),
      signal: c.abort.signal,
      credentials: 'same-origin',
    });
    if (res.status === 401) { showLogin(); throw new Error('401'); }
    if (!res.ok) {
      const txt = await res.text();
      let msg = txt;
      try { msg = JSON.parse(txt).error.message; } catch (e) { /* ignore */ }
      throw new Error(`HTTP ${res.status}: ${msg}`);
    }
    const reader = res.body.getReader();
    const dec = new TextDecoder();
    let buf = '';
    for (;;) {
      const { value, done } = await reader.read();
      if (done) break;
      buf += dec.decode(value, { stream: true });
      let idx;
      while ((idx = buf.indexOf('\n')) >= 0) {
        const line = buf.slice(0, idx).trim();
        buf = buf.slice(idx + 1);
        if (!line.startsWith('data:')) continue;
        const data = line.slice(5).trim();
        if (data === '[DONE]') continue;
        let j;
        try { j = JSON.parse(data); } catch (e) { continue; }
        if (j.error) throw new Error(j.error.message || JSON.stringify(j.error));
        if (j.usage) usage = j.usage;
        const delta = j.choices && j.choices[0] && j.choices[0].delta;
        if (!delta) continue;
        if (delta.reasoning_content) reply.reasoning += delta.reasoning_content;
        if (delta.content) reply.content += delta.content;
        if ((delta.content || delta.reasoning_content) && tFirst == null) tFirst = performance.now();
        renderChatLog();
      }
    }
  } catch (e) {
    if (e.name !== 'AbortError' && e.message !== '401') reply.error = e.message;
  }
  const tEnd = performance.now();
  reply.pending = false;
  const m = S.models.find((x) => x.name === c.model);
  const max = m ? maxContext(m) : 0;
  if (usage) {
    const total = usage.total_tokens ?? (usage.prompt_tokens + usage.completion_tokens);
    const gen = tFirst != null ? (tEnd - tFirst) / 1000 : 0;
    const tps = usage.decode_tps || (gen > 0 && usage.completion_tokens > 1 ? (usage.completion_tokens - 1) / gen : null);
    reply.stats = t('pg.stats', {
      p: fmtNum(usage.prompt_tokens), c: fmtNum(usage.completion_tokens), t: fmtNum(total),
      ctx: max ? ` / ${fmtNum(max)} (${((total / max) * 100).toFixed(0)}%)` : '',
      ttft: fmtMs(tFirst != null ? tFirst - t0 : null), tps: fmtTps(tps),
    });
  } else if (tFirst != null) {
    reply.stats = `${t('m.ttft')} ${fmtMs(tFirst - t0)}`;
  }
  c.busy = false;
  c.abort = null;
  const btn = $('#pg-send');
  if (btn) btn.textContent = t('pg.send');
  renderChatLog();
}

// ============================================================ 页面：日志
async function loadLog() {
  const pre = $('#log-text');
  if (!pre) return;
  try {
    const res = await fetch(`/api/logs/${encodeURIComponent(S.log.target)}?lines=${S.log.lines}`, { credentials: 'same-origin' });
    setOnline(true);
    if (res.status === 401) return showLogin();
    // 进度条用 \r 原地刷新：和终端一样，每行只保留最后一次刷新的内容
    const text = (await res.text()).split('\n').map((l) => l.split('\r').filter((x) => x.trim()).pop() || '').join('\n');
    const stick = pre.scrollHeight - pre.scrollTop - pre.clientHeight < 40;
    pre.textContent = text || t('log.empty');
    if (stick) pre.scrollTop = pre.scrollHeight;
  } catch (e) { setOnline(false); }
}

function renderLogs() {
  const sub = location.hash.split('/')[2];
  if (sub) S.log.target = decodeURIComponent(sub);
  const targets = [['manager', t('log.manager')], ['gateway', t('log.gateway')]].concat(S.models.filter((m) => !m.error).map((m) => [m.name, m.name]));
  $('#main').innerHTML = `
    <div class="page-head"><div><h1>${t('log.title')}</h1></div></div>
    <section class="panel flush">
      <div class="panel-head" style="padding:12px 14px;margin:0;border-bottom:1px solid var(--line)">
        <div class="btn-row" style="align-items:center;gap:12px">
          <label class="field" style="flex-direction:row;align-items:center;gap:8px"><span>${t('log.target')}</span>
            <select id="log-target" style="width:auto">${targets.map(([v, l]) => `<option value="${esc(v)}" ${v === S.log.target ? 'selected' : ''}>${esc(l)}</option>`).join('')}</select></label>
          <label class="field" style="flex-direction:row;align-items:center;gap:8px"><span>${t('log.lines')}</span>
            <select id="log-lines" style="width:auto">${[100, 300, 1000, 5000].map((n) => `<option ${n === S.log.lines ? 'selected' : ''}>${n}</option>`).join('')}</select></label>
          <label class="switch"><input type="checkbox" id="log-auto" ${S.log.auto ? 'checked' : ''}>${t('log.auto')}</label>
        </div>
        <div class="btn-row">
          <button class="btn small" id="log-refresh">${t('common.refresh')}</button>
          <a class="btn small" id="log-dl" href="/api/logs/${encodeURIComponent(S.log.target)}?download=1">${t('log.download')}</a>
        </div>
      </div>
      <pre class="log" id="log-text"></pre>
    </section>`;
  $('#log-target').onchange = (e) => { location.hash = '#/logs/' + encodeURIComponent(e.target.value); };
  $('#log-lines').onchange = (e) => { S.log.lines = Number(e.target.value); loadLog(); };
  $('#log-auto').onchange = (e) => { S.log.auto = e.target.checked; };
  $('#log-refresh').onclick = loadLog;
  loadLog().then(() => { const pre = $('#log-text'); if (pre) pre.scrollTop = pre.scrollHeight; });
}

// ============================================================ 页面：设置
function renderSettings() {
  const s = S.settings || {};
  const d = S.device || {};
  const gw = S.gateway || {};
  const theme = store('axbox.theme') || 'auto';
  const masked = s.api_key ? s.api_key.slice(0, 5) + '•'.repeat(Math.max(8, s.api_key.length - 9)) + s.api_key.slice(-4) : '';
  const runtimes = Object.entries(d.runtimes || {}).map(([k, v]) => `${esc(k)} ${esc(v.version || '')} <span class="muted">${esc(v.path)}</span>`).join('<br>');
  $('#main').innerHTML = `
    <div class="page-head"><div><h1>${t('set.title')}</h1></div></div>
    <section class="panel">
      <h2>${t('set.api')}</h2>
      <p class="sub" style="margin:0 0 12px">${esc(t('set.api_desc'))}</p>
      <label class="switch"><input type="checkbox" id="api-enabled" ${s.api_key_enabled ? 'checked' : ''}>${t('set.api_enable')}</label>
      ${s.api_key ? `<div class="endpoint" style="margin-top:12px">
        <span class="muted">${t('set.api_key')}</span><code id="api-key" data-shown="0">${esc(masked)}</code>
        <button class="btn small" id="api-show">${t('set.show')}</button>
        <button class="btn small" id="api-copy">${t('common.copy')}</button>
        <button class="btn small" id="api-regen">${t('set.regen')}</button></div>` : ''}
    </section>
    <section class="panel">
      <h2>${t('set.gateway')}</h2>
      <dl class="kv">
        <dt>${t('set.gateway_addr')}</dt><dd><code>${esc(gatewayUrl())}</code></dd>
        <dt>${t('set.gateway_state')}</dt><dd>${gw.running ? t('set.gateway_running', { n: gw.restarts || 0 }) : `<span class="err-text">${t('ov.gateway_down')}</span>`}</dd>
      </dl>
    </section>
    <section class="panel">
      <h2>${t('set.appearance')}</h2>
      <dl class="kv" style="align-items:center">
        <dt>${t('set.language')}</dt><dd><span class="seg lang-seg"></span></dd>
        <dt>${t('set.theme')}</dt><dd><span class="seg" id="theme-seg">${['auto', 'light', 'dark'].map((v) => `<button type="button" data-v="${v}" aria-pressed="${theme === v}">${t('set.theme_' + v)}</button>`).join('')}</span></dd>
      </dl>
    </section>
    <section class="panel">
      <h2>${t('set.device')}</h2>
      <dl class="kv" style="margin-bottom:14px">
        <dt>${t('dev.platform')}</dt><dd>${esc(d.platform_version || '')}</dd>
        <dt>${t('dev.runtime')}</dt><dd>${runtimes || '–'}</dd>
        <dt>${t('dev.sdk')}</dt><dd>${esc(d.sdk_version || '–')}</dd>
      </dl>
      <p class="sub" style="margin:0 0 10px">${t('set.reboot_desc')}</p>
      <button class="btn danger" id="reboot">${t('set.reboot')}</button>
    </section>`;
  applyI18n($('#main'));
  const saveKey = async (enabled, generate) => {
    try {
      S.settings = await api('PUT', '/api/settings/api-key', { enabled, generate: !!generate });
      toast(t('set.saved'));
      renderSettings();
    } catch (e) { toast(e.message, true); renderSettings(); }
  };
  $('#api-enabled').onchange = (e) => saveKey(e.target.checked, false);
  if ($('#api-show')) {
    $('#api-show').onclick = () => {
      const el = $('#api-key');
      const shown = el.dataset.shown === '1';
      el.textContent = shown ? masked : s.api_key;
      el.dataset.shown = shown ? '0' : '1';
      $('#api-show').textContent = shown ? t('set.show') : t('set.hide');
    };
    $('#api-copy').onclick = () => copyText(s.api_key);
    $('#api-regen').onclick = async () => {
      if (await confirmDialog(t('set.regen_confirm'), t('set.regen_text'), t('set.regen'), true)) saveKey(s.api_key_enabled, true);
    };
  }
  $('#theme-seg').querySelectorAll('button').forEach((b) => {
    b.onclick = () => { store('axbox.theme', b.dataset.v); applyTheme(); renderSettings(); };
  });
  $('#reboot').onclick = async () => {
    if (!(await confirmDialog(t('set.reboot_confirm'), t('set.reboot_text'), t('set.reboot'), true))) return;
    try {
      await api('POST', '/api/system/reboot');
      toast(t('set.rebooting'));
      waitForReboot();
    } catch (e) { toast(e.message, true); }
  };
}

// ============================================================ 路由
const PAGES = { overview: renderOverview, models: renderModels, playground: renderPlayground, logs: renderLogs, settings: renderSettings };

function render() {
  const p = PAGES[page()] ? page() : 'overview';
  document.querySelectorAll('#nav a').forEach((a) => {
    if (a.dataset.page === p) a.setAttribute('aria-current', 'page');
    else a.removeAttribute('aria-current');
  });
  // 对话测试页正在输入时不整页重绘
  if (p === 'playground' && $('#chat-log') && render.last === p) return;
  render.last = p;
  PAGES[p]();
}

window.addEventListener('hashchange', async () => {
  render.last = null;
  const p = page();
  try {
    if (p === 'playground' || p === 'logs') await refreshModels();
    if (p === 'settings') {
      [S.settings] = await Promise.all([api('GET', '/api/settings'), refreshStatus()]);
    }
    if (p === 'overview') await refreshStatus();
  } catch (e) { /* 401 会跳到登录 */ }
  render();
});

async function boot() {
  applyTheme();
  applyI18n();
  try {
    S.info = await api('GET', '/api/info');
  } catch (e) {
    S.info = { auth: true };
  }
  if (!S.info.user && S.info.auth !== false) return showLogin();
  try {
    S.device = await api('GET', '/api/device');
    await Promise.all([refreshStatus(), refreshModels()]);
    S.settings = await api('GET', '/api/settings');
  } catch (e) {
    if (e.status === 401) return;
    toast(e.message, true);
  }
  $('#login').hidden = true;
  $('#app').hidden = false;
  document.title = (S.device && S.device.hostname) || 'Device Console';
  $('#side-host').textContent = (S.device && S.device.hostname) || '';
  $('#side-chip').textContent = (S.device && (S.device.chip || S.device.board)) || '';
  $('#side-user').textContent = S.info.user || '';
  applyI18n();
  render.last = null;
  render();
  startPolling();
}

boot();
