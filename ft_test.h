#ifndef ft_test_header_included
#define ft_test_header_included

// Copyright (c) 2024, "Leo" Dmitry Kuznetsov
// This code and the accompanying materials are made available under the terms
// of BSD-3 license, which accompanies this distribution. The full text of the
// license may be found at https://opensource.org/license/bsd-3-clause

#include "ft.h"
#include <stdio.h>

#ifndef countof
#define countof(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define ft_max_test_bits 10 // because ft_test() is exponentially expensive
#define ft_max_test_n (1U << ft_max_test_bits)

static uint64_t sum_of(uint64_t* a, int i, int j) { // [i..j] inclusive freq[j]
    uint64_t sum = 0;
    for (int32_t k = i; k <= j; k++) { sum += a[k]; }
    return sum;
}

static void ft_test(uint64_t tree[], size_t n, uint64_t* a, bool verbose) {
    assert(2 <= n && n <= (1u << ft_max_bits));
    const int32_t m = (int32_t)n;
    ft_init(tree, n, a);
    const uint64_t total = tree[n - 1];
    assert(ft_query(tree, n, -1) == 0);
    assert(sum_of(a, 0, -1) == 0);
    for (int32_t i = 0; i < m; i++) {
        if (verbose) { printf("sum_of[0,%2d]: %3lld\n", i, sum_of(a, 0, i)); }
        assert(sum_of(a, 0, i) == ft_query(tree, n, i));
        for (int32_t j = i + 1; j < m + 1; j++) {
            assert(sum_of(a, i, j - 1) == ft_query(tree, n, j - 1) - ft_query(tree, n, i - 1));
        }
    }
    for (uint64_t sum = 0; sum <= total; sum++) {
        int32_t i = ft_index_of(tree, n, sum);
        if (verbose) {
            printf("sum: %3lld: sum_of[0,%2d]: %3lld ft_query(%2d): %3lld\n",
                    sum, i, sum_of(a, 0, i), i, ft_query(tree, n, i));
        }
        assert(ft_query(tree, n, i) <= sum);
        if (i + 1 < (int32_t)m) {
            assert(sum < ft_query(tree, n, i + 1));
        }
    }
    if (verbose) { printf("%lld total: %lld\n", (int64_t)n, total); }
}

static uint64_t a[ft_max_test_n];
static uint64_t tree[ft_max_test_n];

static void ft_tests(bool verbose) {
    static_assert(ft_max_test_bits <= ft_max_bits, "");
    static_assert(ft_max_test_bits <= 12, "ft_max_bits <= 12");
    for (int8_t pass = 0; pass < 2; pass++) {
        for (uint32_t i = 0; i < countof(a); i++) { a[i] = i + pass; }
        const size_t n = 2;
        ft_init(tree, n, a);
        assert(ft_index_of(tree, n, tree[n - 1] + 1) == (int32_t)(n - 1));
        assert(ft_index_of(tree, n, UINT64_MAX) == (int32_t)(n - 1));
        // ft_index_of() only returns -1 if all a[i] > 0
        uint32_t zeros = 0;
        for (uint32_t i = 0; i < countof(a); i++) { zeros += (a[i] == 0); }
        if (zeros == 0) {
            assert(ft_index_of(tree, n, 0) == -1);
        } else {
            assert(ft_index_of(tree, n, 0) == 0);
        }
    }
    for (int8_t bits = 1; bits <= ft_max_test_bits; bits++) {
        const size_t n = (size_t)(1uLL << bits);
        ft_test(tree, n, a, verbose);
    }
    {
        const size_t n = 4;
        ft_test(tree, n, a, true);
    }
}

#endif
