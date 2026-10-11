#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, __u64);
    __type(value, __u64);
} start SEC(".maps");

SEC("kprobe/blk_mq_start_request")
int BPF_KPROBE(io_start, struct request *req) {
    __u64 t = bpf_ktime_get_ns();
    bpf_map_update_elem(&start, &req, &t, BPF_ANY);
    return 0;
}

SEC("kprobe/blk_mq_complete_request")
int BPF_KPROBE(io_end, struct request *rq) {
    __u64 *t = bpf_map_lookup_elem(&start, &rq);
    if (!t)
        return 0;
    __u64 delta_us = (bpf_ktime_get_ns() - *t) / 1000;
    bpf_printk("io done: %llu us", delta_us); // 调试用
    bpf_map_delete_elem(&start, &rq);
    return 0;
}