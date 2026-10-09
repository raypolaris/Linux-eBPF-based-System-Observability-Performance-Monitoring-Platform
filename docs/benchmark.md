# 性能数据

## 测试方法

```bash
# 1. 记录基线 CPU
uptime; top -bn1 | head -5

# 2. 开启采集器，施加负载
sudo ./build/ebpf-observatory &
stress-ng --io 4 --timeout 60s      # 或用 fio 做 IO 压测

# 3. 记录采集开销与事件丢失
#    （丢失统计：ring buffer reserve 失败次数，待加入计数器）
```

## 结果

| 场景 | CPU 占用 | 事件丢失 | 备注 |
|---|---|---|---|
| 空闲（仅 openat 采集器） | 待测 | 待测 | |
| 4 个采集器全开 | 待测 | 待测 | Roadmap 完成后补 |

> 数据格式说明：CPU 占用取 60 秒窗口的 `top` 平均值，事件丢失通过内核态计数器统计。
