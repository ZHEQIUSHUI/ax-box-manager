#!/usr/bin/env python3
"""为一个模型目录生成 model.json（含文件 sha256 清单）。

示例：
  make_model_json.py /data/models/qwen3-0.6b --name qwen3-0.6b --type chat \
      --max-context 2559 --cmm-mb 900 --ram-mb 400

默认的运行时是 axllm，启动参数为 ["serve", "{model_dir}", "--port", "{port}"]。
"""
import argparse
import hashlib
import json
import os
import sys


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("model_dir")
    ap.add_argument("--name", help="model name used in API calls (default: folder name)")
    ap.add_argument("--version", default="1.0")
    ap.add_argument("--type", default="chat", help="chat / vlm / embedding / asr / tts ...")
    ap.add_argument("--description", default="")
    ap.add_argument("--max-context", type=int, default=0)
    ap.add_argument("--cmm-mb", type=int, default=0, help="NPU memory (CMM) needed, MB")
    ap.add_argument("--ram-mb", type=int, default=0, help="system memory needed, MB")
    ap.add_argument("--runtime", default="axllm")
    ap.add_argument("--min-version", default="")
    ap.add_argument("--arg", action="append", dest="args",
                    help="launch argument, repeatable (default for axllm: serve {model_dir} --port {port})")
    ap.add_argument("--no-hash", action="store_true", help="skip the sha256 file list")
    ap.add_argument("-o", "--output", help="output path (default: <model_dir>/model.json)")
    a = ap.parse_args()

    d = os.path.abspath(a.model_dir)
    if not os.path.isdir(d):
        sys.exit(f"not a directory: {d}")
    spec = {
        "name": a.name or os.path.basename(d),
        "version": a.version,
        "type": a.type,
        "description": a.description,
        "max_context": a.max_context,
        "mem_required": {"cmm_mb": a.cmm_mb, "ram_mb": a.ram_mb},
        "runtime": {"name": a.runtime, "min_version": a.min_version},
    }
    if a.args:
        spec["launch"] = {"args": a.args}
    elif a.runtime != "axllm":
        sys.exit("--arg is required for runtimes other than axllm")
    if not a.no_hash:
        files = {}
        for root, _, names in os.walk(d, followlinks=True):
            for n in sorted(names):
                p = os.path.join(root, n)
                rel = os.path.relpath(p, d)
                if rel == "model.json" or n.startswith("."):
                    continue
                print("sha256", rel, file=sys.stderr)
                files[rel] = sha256(p)
        spec["files"] = dict(sorted(files.items()))
    out = a.output or os.path.join(d, "model.json")
    with open(out, "w") as f:
        json.dump(spec, f, ensure_ascii=False, indent=2)
        f.write("\n")
    print(out)


if __name__ == "__main__":
    main()
