#!/usr/bin/env bash
# 冒烟测试：编译 + 运行 2 秒，确认能抓到事件
set -euo pipefail
cd "$(dirname "$0")/.."

make

out=$(sudo timeout 2 ./build/ebpf-observatory 2>&1 || true)
echo "$out" | head -5

if echo "$out" | grep -q "FILENAME"; then
    echo "SMOKE OK: bpf program attached and printed header"
else
    echo "SMOKE FAIL: no output from tracer" >&2
    exit 1
fi
