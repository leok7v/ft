// Copyright (c) 2024, "Leo" Dmitry Kuznetsov
// This code and the accompanying materials are made available under the terms
// of BSD-3 license, which accompanies this distribution. The full text of the
// license may be found at https://opensource.org/license/bsd-3-clause

#ifndef ft_header_included
#define ft_header_included

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

// https://en.wikipedia.org/wiki/Fenwick_tree

#define ft_max_bits 31

static_assert(ft_max_bits <= 31, "arrays are indexed by int32_t");

static inline int32_t ft_lsb(int32_t i) { // least significant bit only
    assert(0 < i && i < (1uLL << ft_max_bits)); // 0 will lead to endless loop
    return i & (~i + 1); // (i & -i)
}

static void ft_init(uint64_t tree[], size_t n, uint64_t a[]) {
    assert(2 <= n && n <= (1u << ft_max_bits));
    const int32_t m = (int32_t)n;
    for (int32_t i = 0; i <  m; i++) { tree[i] = a[i]; }
    for (int32_t i = 1; i <= m; i++) {
        int32_t parent = i + ft_lsb(i);
        if (parent <= m) {
            assert(tree[parent - 1] < UINT64_MAX - tree[i - 1]);
            tree[parent - 1] += tree[i - 1];
        }
    }
}

static void ft_update(uint64_t tree[], size_t n, int32_t i, uint64_t inc) {
    assert(2 <= n && n <= (1u << ft_max_bits));
    const int32_t m = (int32_t)n;
    assert(0 <= i && i < m);
    while (i < m) {
        assert(tree[i] <= UINT64_MAX - inc);
        tree[i] += inc;
        i += ft_lsb(i + 1); // Move to the sibling
    }
}

static uint64_t ft_query(const uint64_t tree[], size_t n, int32_t i) {
    // ft_query() cumulative sum of all a[j] for j < i
    assert(2 <= n && n <= (1u << ft_max_bits));
    uint64_t sum = 0; // ft_query(tree, n, -1) == 0
    while (i >= 0) {  // grandparent can be in the tree when parent is not
        if (i < (int32_t)n) {
            sum += tree[i];
        }
        i -= ft_lsb(i + 1); // Clear lsb - move to the parent.
    }
    return sum;
}

static int32_t ft_index_of(uint64_t tree[], size_t n, uint64_t const sum) {
    // returns index 'i' of an element such that sum of all a[j] for j < i
    // is less of equal to sum.
    // returns -1 if sum is less than any element of a[]
    assert(2 <= n && n <= (1u << ft_max_bits));
    assert((n & (n - 1)) == 0); // only works for power 2
    if (sum >= tree[n - 1]) { return (int32_t)(n - 1); }
    uint64_t value = sum;
    uint32_t i = 0;
    uint32_t mask = (uint32_t)(n >> 1);
    while (mask != 0) {
        uint32_t t = i + mask;
        if (t <= n && value >= tree[t - 1]) {
            i = t;
            value -= tree[t - 1];
        }
        mask >>= 1;
    }
    return i == 0 && value < sum ? -1 : (int32_t)(i - 1);
}

// Why ft_max_bits is 31 and int32_t is used for array indexing?
// (for 16-bit ft_max_bits may be easily replaced by 15 or 16)
// Older Microsoft C89 compiler cl.exe for x86 used 32-bit signed
// int to index arrays (it is possible to overcome that limitation
// by using *(pointer + size_t) instead if absolutely necessary but
// it will make code a bit uglier.

#endif

