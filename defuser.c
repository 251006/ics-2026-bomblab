#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}


int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t low  = (uint32_t)challenge;
    uint32_t mid  = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t step2 = (high + low * 8u) ^ 0x5f7du;
    uint32_t step0 = (low - mid) * 5u + 0x5697u;
    values[0] = (step2 + 0x7e71u) ^ step0;

    return write_answer(out, cap, values, count);
}


// int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
//     uint32_t values[MAX_FIELDS] = {0};
//     size_t count = 0;
//     (void)challenge;

//     // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
//     return write_answer(out, cap, values, count);
// }

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 31) & 0x1fu);
    for (uint32_t i = 0; i < 5; ++i) {
        uint32_t shift = 19u + 4u * i;
        uint32_t part = (uint32_t)((challenge >> shift) & 0xfu);
        values[i + 1] = values[i] * 5u + (part ^ (13u + i));
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t selector = (uint32_t)challenge & 0x7u;
    uint32_t data = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t result;

    switch (selector) {
        case 0:
            result = data * 5u + 0x2620fu;
            break;
        case 1:
            result = (data ^ 0x2217u) + (data >> 2);
            break;
        case 2:
            result = data * 4u ^ 0x7c45u;
            break;
        case 3:
            result = (data ^ 0x9436u) + (data >> 1);
            break;
        case 4:
            result = data * 5u + 0x6fb5u;
            break;
        case 5:
            result = data * 5u + 0x16b43u;
            break;
        case 6:
            result = data * 4u ^ 0x7b78u;
            break;
        default:
            result = data * 3u + 0xb612u;
            break;
    }

    values[0] = selector;
    values[1] = result;
    return write_answer(out, cap, values, count);
}

static uint32_t phase_4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt) {
    uint32_t mid = low + (high - low) / 2u;
    if (target == mid) return (mid + 0x6au) ^ salt;

    uint32_t path_value = mid + salt;
    if (target < mid) {
        uint32_t result = phase_4_walk(target, low, mid - 1u, salt);
        return (result * 5u ^ path_value) ^ 0xau;
    }

    uint32_t result = phase_4_walk(target, mid + 1u, high, salt);
    return (result * 5u ^ path_value) ^ 0x1u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase_4_walk(target, 0u, 0x7fu, salt);

    return write_answer(out, cap, values, count);
}

typedef struct Phase5Node {
    const struct Phase5Node *next;
    uint32_t value;
    uint32_t tag;
} Phase5Node;

static const Phase5Node phase_5_nodes[12] = {
    {&phase_5_nodes[5],  0x094du, 0x01adu},
    {&phase_5_nodes[2],  0x033bu, 0x8aaau},
    {&phase_5_nodes[8],  0x0a63u, 0x36d8u},
    {&phase_5_nodes[4],  0x081bu, 0x1a08u},
    {NULL,               0x0565u, 0x8d6eu},
    {&phase_5_nodes[6],  0x0481u, 0x57f1u},
    {&phase_5_nodes[9],  0x0718u, 0xdabau},
    {&phase_5_nodes[10], 0x0109u, 0xfd9cu},
    {&phase_5_nodes[0],  0x0266u, 0x9e06u},
    {&phase_5_nodes[11], 0x0078u, 0xfa3au},
    {&phase_5_nodes[3],  0x0b6eu, 0xa24bu},
    {&phase_5_nodes[7],  0x0633u, 0x17c2u},
};

static const Phase5Node *const phase_5_heads[8] = {
    &phase_5_nodes[1],
    &phase_5_nodes[2],
    &phase_5_nodes[8],
    &phase_5_nodes[0],
    &phase_5_nodes[5],
    &phase_5_nodes[6],
    &phase_5_nodes[9],
    &phase_5_nodes[11],
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t head_index = (uint32_t)challenge & 0x7u;
    uint32_t steps = (uint32_t)(challenge >> 3) & 0x3u;
    const Phase5Node *node = phase_5_heads[head_index];

    uint32_t result = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    result ^= node->tag;
    result ^= 0x72eau;
    result += node->value;

    for (uint32_t i = 0; i < steps; ++i) {
        node = node->next;
        result ^= node->tag;
        result += node->value;
    }

    values[0] = result;
    values[1] = node->value;
    return write_answer(out, cap, values, count);
}

typedef uint32_t (*Phase6Op)(uint32_t);

static uint32_t phase_6_op_0(uint32_t x) {
    return ((x << 7) + 0xc3b4u) ^ (x * 3u);
}

static uint32_t phase_6_op_1(uint32_t x) {
    uint32_t rotated = (x << 2) | (x >> 30);
    return rotated ^ 0xed9cu;
}

static uint32_t phase_6_op_2(uint32_t x) {
    return ((x >> 5) ^ x) + 0x5aedu;
}

static uint32_t phase_6_op_3(uint32_t x) {
    return (x + 0x8693u) ^ (x >> 5);
}

static uint32_t phase_6_op_4(uint32_t x) {
    return (x >> 7) | (x << 25);
}

static uint32_t phase_6_op_5(uint32_t x) {
    return (x ^ 0x3c27u) + x * 8u;
}

static const Phase6Op phase_6_ops[8] = {
    phase_6_op_1,
    phase_6_op_5,
    phase_6_op_5,
    phase_6_op_0,
    phase_6_op_4,
    phase_6_op_5,
    phase_6_op_2,
    phase_6_op_3,
};

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t first_index = (uint32_t)challenge & 0x7u;
    uint32_t second_index = (uint32_t)(challenge >> 3) & 0x7u;
    uint32_t input = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);

    uint32_t result = phase_6_ops[first_index](input);
    result ^= 0xf82au;
    result = phase_6_ops[second_index](result);
    values[0] = result + 0x85u;

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t first = (uint32_t)context & 3u;
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t x = a ^ b;
    uint32_t rotated = (x >> 7) | (x << 25);
    uint32_t second = rotated ^ 0x8ad4u ^
                      (first * 0x5d0cu) ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (b + first + 0xd7d8u + (a >> 7));

    // 拷贝起点到标记相差 0x20 字节；整数文本从偏移 0x24 开始。
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32
                        " %" PRIu32 " %" PRIu32,
                        first, second, third);
    return size < 0 || (size_t)size >= cap;
}
