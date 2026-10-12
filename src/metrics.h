#ifndef METRICS_H
#define METRICS_H
#include <stddef.h>
#include <stdint.h>

int metrics_render(const uint64_t hist[64], uint64_t sum_us, char *out, size_t outlen);
#endif
