#!/bin/sh
# 生成发布包：scripts/package.sh <build_dir> <axllm 可执行文件> [输出目录]
#   发布包内容：bin/{axbox-manager,openai_api_gateway,axllm} + 安装脚本 + systemd 服务 + 示例配置
set -e
BUILD=$1
AXLLM=$2
OUT=${3:-.}
[ -n "$BUILD" ] && [ -n "$AXLLM" ] || { echo "usage: $0 <build_dir> <axllm> [out_dir]"; exit 1; }
ROOT=$(cd "$(dirname "$0")/.." && pwd)
VERSION=$(sed -n 's/.*kVersion = "\(.*\)".*/\1/p' "$ROOT/src/version.hpp")
ARCH=$(file -b "$BUILD/axbox-manager" | grep -q aarch64 && echo aarch64 || uname -m)
NAME=axbox-$VERSION-$ARCH
STRIP=${STRIP:-strip}
TMP=$(mktemp -d)
mkdir -p "$TMP/$NAME/bin"
cp "$BUILD/axbox-manager" "$BUILD/third_party/openai-api.cpp/openai_api_gateway" "$AXLLM" "$TMP/$NAME/bin/"
$STRIP "$TMP/$NAME/bin/"* 2>/dev/null || true
cp "$ROOT/packaging/install.sh" "$ROOT/packaging/uninstall.sh" "$ROOT/packaging/axbox.service" \
   "$ROOT/packaging/config.example.json" "$TMP/$NAME/"
tar -C "$TMP" -czf "$OUT/$NAME.tar.gz" "$NAME"
rm -rf "$TMP"
echo "$OUT/$NAME.tar.gz"
