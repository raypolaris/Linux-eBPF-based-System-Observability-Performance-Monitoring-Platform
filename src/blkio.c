#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "blkio.skel.h"

static volatile sig_atomic_t stop;

static void on_signal(int sig) {
    (void)sig;
    stop = 1;
}

static void print_hist(int map_fd) {
    static __u64 prev[64]; /* 上一轮的累计值，用来算增量 */
    __u64 cur, delta, d[64], max = 0, total = 0;
    __u32 i;

    /* 读全部 64 个桶，同时算"本区间增量" */
    for (i = 0; i < 64; i++) {
        cur = 0;
        bpf_map_lookup_elem(map_fd, &i, &cur);
        delta = cur - prev[i]; /* 本区间新增的数量 */
        prev[i] = cur;
        d[i] = delta;
        total += delta;
        if (delta > max)
            max = delta;
    }

    if (!total) { /* 这一轮没有任何 IO */
        printf("(no block IO in last 2s)\n");
        return;
    }

    printf("  latency(us)      count   distribution        (total: %llu, max: %llu)\n", total, max);

    for (i = 0; i < 64; i++) {
        __u64 lo, hi;
        int bar;

        if (!d[i]) /* 空桶跳过*/
            continue;

        lo = (i == 0) ? 0 : (1ULL << i);
        hi = (1ULL << (i + 1)) - 1;
        bar = (int)(d[i] * 40 / max);
        if (bar < 1)
            bar = 1;

        printf("%6llu -> %-6llu %6llu   |%-40.*s|\n", lo, hi, d[i], bar,
               "################################################");
    }
}

int main(void) {
    struct blkio_bpf *skel;

    int err = 0;

    libbpf_set_print(NULL);

    skel = blkio_bpf__open_and_load();
    if (!skel) {
        fprintf(stderr, "failed to open/load BPF skeleton\n");
        return 1;
    }

    err = blkio_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "attach failed: %d\n", err);
        goto cleanup;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    while (!stop) {
        sleep(2);
        print_hist(bpf_map__fd(skel->maps.hist));
    }

cleanup:

    blkio_bpf__destroy(skel);
    return err < 0;
}
