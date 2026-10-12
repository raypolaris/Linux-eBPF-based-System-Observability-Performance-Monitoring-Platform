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

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 64);
    __type(key, __u32);
    __type(value, __u64);
} hist SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, __u64);
} total_us SEC(".maps");

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
    __u32 slot =
        delta_us ? (63 - __builtin_clzll(delta_us)) : 0; /* 求 floor(log2(值))，libbpf 自带 */
    if (slot >= 64)
        slot = 63;
    __u64 *cnt = bpf_map_lookup_elem(&hist, &slot);
    if (cnt)
        __sync_fetch_and_add(cnt, 1);
    __u32 zero = 0;
    __u64 *sum = bpf_map_lookup_elem(&total_us, &zero);
    if (sum)
        __sync_fetch_and_add(sum, 1);
    bpf_map_delete_elem(&start, &rq);
    return 0;
}