#!/usr/bin/env python3
"""开发用的假模型服务：OpenAI 兼容的 /v1/models 和 /v1/chat/completions（支持流式）。

用法：mock_model.py --port 18001 [--name mock] [--load-seconds 2] [--crash-after N]
"""
import argparse
import json
import os
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, required=True)
ap.add_argument("--name", default="mock")
ap.add_argument("--load-seconds", type=float, default=2)
ap.add_argument("--crash-after", type=int, default=0, help="exit after N chat requests")
args = ap.parse_args()

print(f"loading {args.name} ...", flush=True)
time.sleep(args.load_seconds)
calls = 0


class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, fmt, *a):
        print(fmt % a, flush=True)

    def _json(self, obj, status=200):
        body = json.dumps(obj).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/v1/models":
            return self._json({"object": "list", "data": [{"id": args.name, "object": "model", "max_token_len": 4096}]})
        self._json({"error": {"message": "not found"}}, 404)

    def do_POST(self):
        global calls
        n = int(self.headers.get("Content-Length", 0))
        req = json.loads(self.rfile.read(n) or b"{}")
        calls += 1
        if args.crash_after and calls > args.crash_after:
            print("crashing on purpose", flush=True)
            os._exit(3)
        text = "你好！我是测试模型 " + args.name + "，收到 " + str(len(req.get("messages", []))) + " 条消息。"
        usage = {"prompt_tokens": 20 * len(req.get("messages", [])), "completion_tokens": len(text), "total_tokens": 0}
        usage["total_tokens"] = usage["prompt_tokens"] + usage["completion_tokens"]
        if not req.get("stream"):
            return self._json({"id": "x", "object": "chat.completion", "model": req.get("model"),
                               "choices": [{"index": 0, "message": {"role": "assistant", "content": text}, "finish_reason": "stop"}],
                               "usage": usage})
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Transfer-Encoding", "chunked")
        self.end_headers()

        def chunk(obj):
            data = ("data: " + json.dumps(obj, ensure_ascii=False) + "\n\n").encode()
            self.wfile.write(b"%x\r\n%s\r\n" % (len(data), data))
            self.wfile.flush()

        if req.get("enable_thinking"):
            for c in "先想一想。":
                chunk({"choices": [{"index": 0, "delta": {"reasoning_content": c}}]})
                time.sleep(0.02)
        for c in text:
            chunk({"choices": [{"index": 0, "delta": {"content": c}}]})
            time.sleep(0.03)
        chunk({"choices": [{"index": 0, "delta": {}, "finish_reason": "stop"}], "usage": usage})
        data = b"data: [DONE]\n\n"
        self.wfile.write(b"%x\r\n%s\r\n0\r\n\r\n" % (len(data), data))


ThreadingHTTPServer(("127.0.0.1", args.port), H).serve_forever()
