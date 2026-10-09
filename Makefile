CLANG   ?= clang
BPFTOOL ?= bpftool
ARCH    := $(shell uname -m | sed "s/x86_64/x86/;s/aarch64/arm64/")
GLIBC   := $(shell uname -m)-linux-gnu
OUT     := build

BPF_CFLAGS  := -Wno-missing-declarations -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH) -I bpf -I /usr/include/$(GLIBC)
USER_CFLAGS := -g -Wall -Wextra -I $(OUT)

all: $(OUT)/ebpf-observatory

bpf/vmlinux.h:
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

$(OUT)/%.bpf.o: bpf/%.bpf.c bpf/vmlinux.h | $(OUT)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

$(OUT)/%.skel.h: $(OUT)/%.bpf.o
	$(BPFTOOL) gen skeleton $< > $@

$(OUT)/ebpf-observatory: src/main.c $(OUT)/openat.skel.h | $(OUT)
	$(CC) $(USER_CFLAGS) -o $@ src/main.c -lbpf -lelf -lz

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -rf $(OUT) bpf/vmlinux.h

.PHONY: all clean
