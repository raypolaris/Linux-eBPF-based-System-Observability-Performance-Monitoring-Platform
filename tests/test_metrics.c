#include <stdio.h>
#include <string.h>
#include "metrics.h"

static int failures;

#define CHECK(cond, msg)                                                                           \
    do {                                                                                           \
        if (cond) {                                                                                \
            printf("  ok   %s\n", msg);                                                            \
        } else {                                                                                   \
            printf("  FAIL %s (line %d)\n", msg, __LINE__);                                        \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

static void test_empty(void) {
    uint64_t hist[64] = {0};
    char buf[8192];

    printf("test_empty:\n");
    metrics_render(hist, 0, buf, sizeof buf);
    CHECK(strstr(buf, "# TYPE ebpf_blkio_latency_seconds histogram"), "有 TYPE 行");
    CHECK(strstr(buf, "bucket{le=\"+Inf\"} 0"), "无数据时 +Inf 为 0");
    CHECK(strstr(buf, "_count 0"), "无数据时 _count 为 0");
}

static void test_cumulative(void) {
    uint64_t hist[64] = {0};
    char buf[8192];

    hist[0] = 1;  /* 0~1us   */
    hist[2] = 3;  /* 4~7us   */
    hist[5] = 10; /* 32~63us ，总 14 */

    printf("test_cumulative:\n");
    metrics_render(hist, 1000, buf, sizeof buf);
    CHECK(strstr(buf, "bucket{le=\"0.000001\"} 1"), "≤1us  累计 = 1");
    CHECK(strstr(buf, "bucket{le=\"0.000007\"} 4"), "≤7us  累计 = 1+3 = 4");
    CHECK(strstr(buf, "bucket{le=\"0.000063\"} 14"), "≤63us 累计 = 14");
    CHECK(strstr(buf, "_count 14"), "_count = 14");
    CHECK(strstr(buf, "_sum 0.001000"), "_sum 换算成秒");
}

int main(void) {
    test_empty();
    test_cumulative();

    if (failures) {
        printf("\n%d 个断言失败\n", failures);
        return 1; /* 非 0 = 失败，CI 能感知 */
    }
    printf("\nALL TESTS PASSED\n");
    return 0;
}