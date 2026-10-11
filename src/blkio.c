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
        sleep(1);
    }

cleanup:

    blkio_bpf__destroy(skel);
    return err < 0;
}
