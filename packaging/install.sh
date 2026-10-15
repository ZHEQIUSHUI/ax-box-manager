#!/bin/sh
# 在设备上安装或升级 axbox（以 root 运行，在解压后的发布包目录里执行）
#   sh install.sh [--models-dir /data/models] [--port 8080] [--no-start]
set -e

PREFIX=/opt/axbox
MODELS_DIR=
PORT=
START=1
while [ $# -gt 0 ]; do
    case "$1" in
        --models-dir) MODELS_DIR=$2; shift 2 ;;
        --port) PORT=$2; shift 2 ;;
        --no-start) START=0; shift ;;
        *) echo "unknown option: $1"; exit 1 ;;
    esac
done

[ "$(id -u)" = 0 ] || { echo "run as root"; exit 1; }
cd "$(dirname "$0")"

mkdir -p "$PREFIX/bin" /etc/axbox
for f in bin/*; do
    # 先写临时文件再改名：正在运行的程序也能替换
    install -m 0755 "$f" "$PREFIX/bin/.$(basename "$f").new"
    mv -f "$PREFIX/bin/.$(basename "$f").new" "$PREFIX/bin/$(basename "$f")"
done

if [ ! -f /etc/axbox/config.json ]; then
    cp config.example.json /etc/axbox/config.json
    [ -n "$MODELS_DIR" ] && sed -i "s#\"models_dir\": \"[^\"]*\"#\"models_dir\": \"$MODELS_DIR\"#" /etc/axbox/config.json
    [ -n "$PORT" ] && sed -i "s#\"listen_port\": [0-9]*#\"listen_port\": $PORT#" /etc/axbox/config.json
    echo "created /etc/axbox/config.json"
else
    echo "keeping existing /etc/axbox/config.json"
fi

if command -v systemctl > /dev/null 2>&1 && [ -d /etc/systemd/system ]; then
    cp axbox.service /etc/systemd/system/axbox.service
    systemctl daemon-reload
    systemctl enable axbox.service > /dev/null 2>&1
    if [ "$START" = 1 ]; then
        systemctl restart axbox.service
        echo "axbox started: systemctl status axbox"
    fi
else
    echo "systemd not found: start $PREFIX/bin/axbox-manager -c /etc/axbox/config.json from your init system"
fi
