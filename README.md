# Linux eBPF-based System Observability & Performance Monitoring Platform

基于 eBPF 的 Linux 系统可观测性与性能监控平台：内核态采集与聚合，用户态导出指标，配合 Grafana 做监控大盘。

> **状态：早期开发中。** 当前已实现文件访问（openat）采集器：tracepoint + ring buffer，实时输出进程打开的文件。

## 为什么做这个

`top` / `iostat` 只给平均值，看不到延迟长尾，而线上问题往往出在 P99。eBPF 能做到零侵入、低开销、精确到每次调用；但 bpftrace 一行脚本只能"看一眼"，无法长期运行。本项目把采集做成可长期运行、可导出指标的形态。

## 功能

已实现：

- **文件访问采集**：tracepoint `syscalls/sys_enter_openat`，输出 `pid / 进程名 / 文件路径`
- 内核态 **ring buffer** → 用户态实时消费（`BPF_MAP_TYPE_RINGBUF`）
- 一条 `make` 完成：`vmlinux.h` 生成 → BPF 编译 → skeleton 生成 → 用户态链接

Roadmap：

- [ ] 块 IO 延迟直方图（kprobe `blk_account_io_*`）
- [ ] 调度延迟（`sched_wakeup` → `sched_switch`）
- [ ] TCP 重传统计（`tcp_retransmit_skb`）
- [ ] Prometheus `/metrics` 输出
- [ ] 配置文件热重载（inotify）
- [ ] Grafana dashboard JSON

## 环境要求

- Linux >= 5.8，内核需开启 `CONFIG_DEBUG_INFO_BTF=y`（本项目在 `7.0.0-34-generic` 上验证）
- `clang`、`llvm`、`libbpf-dev`、`bpftool`、`linux-tools-$(uname -r)`
- 需要 root 权限（`kernel.unprivileged_bpf_disabled = 2`）

## 快速开始

```bash
sudo apt install -y build-essential clang llvm libbpf-dev bpftool \
    linux-tools-$(uname -r) pkg-config

make
sudo ./build/ebpf-observatory
```

输出示例（真实运行结果）：

```text
PID      COMM             FILENAME
4500     code             /proc/90492/cmdline
524      systemd-oomd     /sys/fs/cgroup/user.slice/user-1000.slice/memory.current
```

`Ctrl-C` 退出。

## 项目结构

```text
bpf/          eBPF 程序（C），vmlinux.h 由 bpftool 生成、不入库
src/          用户态：加载、attach、ring buffer 消费
docs/         设计文档与性能数据
tests/        冒烟测试脚本
Makefile      vmlinux.h + BPF + skeleton + 用户态 一条龙构建
```

## 文档

- [设计文档](docs/design.md)
- [性能数据](docs/benchmark.md)

## License

MIT
