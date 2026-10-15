#!/bin/sh
# 卸载 axbox：停止服务和所有模型进程，删除程序；保留模型目录、/etc/axbox 和 /var/lib/axbox
set -e
[ "$(id -u)" = 0 ] || { echo "run as root"; exit 1; }
if command -v systemctl > /dev/null 2>&1; then
    systemctl disable --now axbox.service 2>/dev/null || true
    rm -f /etc/systemd/system/axbox.service
    systemctl daemon-reload
fi
# 正常结束模型进程和网关（SIGINT，避免强杀 NPU 进程）
for f in /var/lib/axbox/run/*.json; do
    [ -f "$f" ] || continue
    pid=$(sed -n 's/.*"pid":\([0-9]*\).*/\1/p' "$f")
    [ -n "$pid" ] && kill -INT "$pid" 2>/dev/null || true
done
# 删除管理服务添加的模型端口防火墙规则（端口段与 config.json 默认值一致）
for t in iptables; do
    command -v $t > /dev/null 2>&1 && $t -w -D INPUT -p tcp --dport 18001:19000 ! -i lo -j DROP 2>/dev/null || true
done
rm -rf /opt/axbox
echo "removed /opt/axbox (kept /etc/axbox, /var/lib/axbox and the model directory)"
