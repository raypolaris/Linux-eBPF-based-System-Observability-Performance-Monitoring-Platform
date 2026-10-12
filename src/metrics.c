#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "metrics.h"
int metrics_render(const uint64_t hist[64], uint64_t sum_us, char *out, size_t outlen) {
    size_t off = 0;
    uint64_t cumulative = 0;

    off += snprintf(out + off, outlen - off,
                    "# HELP ebpf_blkio_latency_seconds Block IO latency\n"
                    "# TYPE ebpf_blkio_latency_seconds histogram\n");

    for (int i = 0; i < 64; i++) {
        double le_s;
        uint64_t hi_us;

        cumulative += hist[i];
        if (i == 63)
            break;

        if (cumulative == 0)
            continue;

        hi_us = (1ull << (i + 1)) - 1;
        le_s = (double)hi_us / 1e6;

        off += snprintf(out + off, outlen - off,
                        "ebpf_blkio_latency_seconds_bucket{le=\"%.6f\"} %llu\n", le_s,
                        (unsigned long long)cumulative);
        if (off >= outlen) /* 缓冲区不够，不让 outlen - off 下溢 */
            return -1;
    }

    off +=
        snprintf(out + off, outlen - off, "ebpf_blkio_latency_seconds_bucket{le=\"+Inf\"} %llu\n",
                 (unsigned long long)cumulative);
    off += snprintf(out + off, outlen - off, "ebpf_blkio_latency_seconds_count %llu\n",
                    (unsigned long long)cumulative);
    off += snprintf(out + off, outlen - off, "ebpf_blkio_latency_seconds_sum %.6f\n",
                    (double)sum_us / 1e6);

    return (int)off;
}