#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "openat.skel.h"

struct event {
    __u32 pid;
    char comm[16];
    char fname[128];
};

static volatile sig_atomic_t stop;

static void on_signal(int sig) {
    (void)sig;
    stop = 1;
}

static int on_event(void *ctx, void *data, size_t len) {
    const struct event *e = data;
    (void)ctx;
    (void)len;
    printf("%-8u %-16s %s\n", e->pid, e->comm, e->fname);
    return 0;
}

int main(void) {
    struct openat_bpf *skel;
    struct ring_buffer *rb = NULL;
    int err = 0;

    libbpf_set_print(NULL);

    skel = openat_bpf__open_and_load();
    if (!skel) {
        fprintf(stderr, "failed to open/load BPF skeleton\n");
        return 1;
    }

    err = openat_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "attach failed: %d\n", err);
        goto cleanup;
    }

    rb = ring_buffer__new(bpf_map__fd(skel->maps.events), on_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "ring_buffer__new failed\n");
        err = -1;
        goto cleanup;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    printf("PID      COMM             FILENAME\n");
    while (!stop) {
        err = ring_buffer__poll(rb, 100);
        if (err == -EINTR) {
            err = 0;
            break;
        }
        if (err < 0) {
            fprintf(stderr, "poll error: %d\n", err);
            break;
        }
    }

cleanup:
    ring_buffer__free(rb);
    openat_bpf__destroy(skel);
    return err < 0;
}
