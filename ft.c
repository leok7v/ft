#include "includes.h"
#ifndef rt_header_included
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#endif

// https://en.wikipedia.org/wiki/Fenwick_tree

#define ft_max_bits 31

static_assert(ft_max_bits <= 31, "uint32_t is used to index arrays");

static inline ft_lsb(size_t ix) { // least significant bit only
    assert(ix < (1u << ft_max_bits));
    const int32_t i = (int32_t)ix;
    assert(i != 0); // cannot be 0 - will lead to endless loop
    assert((i & (~i + 1)) == (i & -i));
    return i & (~i + 1);
}

static void ft_update(uint64_t tree[], size_t n, int32_t i, uint64_t inc) {
    assert(2 <= n && n <= (1u << ft_max_bits));
    const int32_t m = (int32_t)n;
    assert(0 <= i && i < m);
    i++;  // Convert to 1-based indexing
    assert(i < m + 1);
    while (i < m + 1) {
        assert(tree[i - 1] <= UINT64_MAX - inc);
        tree[i - 1] += inc;
        i += ft_lsb(i);  // Move to the sibling
    }
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

static uint64_t ft_query(const uint64_t tree[], size_t n, int32_t i) {
    // ft_query() cumulative sum of all a[j] for j < i
    assert(2 <= n && n <= (1u << ft_max_bits));
    if (i == -1) { return 0; }  // index can be -1
    const int32_t m = (int32_t)n;
    i++;  // Convert to 1-based indexing
    uint64_t sum = 0;
    while (i > 0) {
        assert(0 <= i - 1 && i - 1 < m);
        sum += tree[i - 1];
        i -= ft_lsb(i);  // Clear lsb - move to the parent.
    }
    return sum;
}

static int32_t ft_index_of(uint64_t tree[], size_t n, uint64_t const sum) {
    // returns index 'i' of an element such that sum of all a[j] for j < i
    // is less of equal to sum.
    // returns -1 if sum is less than any element of a[]
    assert(2 <= n && n <= (1u << ft_max_bits));
    assert((n & (n - 1)) == 0); // only works for power 2
    if (sum >= tree[n - 1]) { return (int32_t)n - 1; }
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
    return (i == 0 && value < sum) ? -1 : (int32_t)i - 1;
}

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

// Why ft_max_bits is 31 and int32_t is used for array indexing?
// (for 16-bit ft_max_bits may be easily replaced by 15 or 16)
// Older Microsoft C89 compiler cl.exe for x86 used 32-bit signed
// int to index arrays (it is possible to overcome that limitation
// by using *(pointer + size_t) instead if absolutely necessary but
// it will make code a bit uglier.

// test:

#include <stdio.h>

#ifndef countof
#define countof(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define ft_max_test_bits 10 // because ft_test() is exponentially expensive
#define ft_max_test_n (1U << ft_max_test_bits)

static uint64_t a[ft_max_test_n];
static uint64_t tree[ft_max_test_n];

static void test(bool verbose) {
    static_assert(ft_max_test_bits <= ft_max_bits, "");
    static_assert(ft_max_test_bits <= 12, "ft_max_bits <= 12");
    for (int8_t pass = 0; pass < 2; pass++) {
        for (uint32_t i = 0; i < countof(a); i++) { a[i] = i + pass; }
        const size_t n = 2;
        ft_init(tree, n, a);
        assert(ft_index_of(tree, n, tree[n - 1] + 1) == n - 1);
        assert(ft_index_of(tree, n, UINT64_MAX) == n - 1);
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

int main(void) {
    test(false);
    return 0;
}


/*
    = Legalize:

    This code and the accompanying materials are made available under the
    terms of the CC0 license, which accompanies this distribution. The full
    text of the license may be found at:
    https://creativecommons.org/publicdomain/zero/1.0/legalcode

    = Disclaimer:

    There is K&R C, ANSI C, C89, C99, C11, C18, C2x and there is "C as I like it".

    There is language itself, preprocessor, ancient runtime, several bickering
    standardization committees corporate and open source flavors and plethora of
    coding styles (e.g. MISRA C).

    I have chosen to stick to the "C as I like it" style because it pleases _my_
    eye as long as it does not offend wider audience too much. I am aware that
    it is not the most popular style. I listen to critics and I am willing to
    improve the code to make it more readable and more portable.

    * '''if (foo) { bar(); }''' without putting the compound block at a l
      ine of its own might be considered less readable.
    * using enum {} for constants is at mercy of compiler ```int``` type
      bitness but at least it does not leak elements names into global
      scope as ```#define``` does.
    * The code is not always suitable for 16 and 8 bit architectures some
      tweaks may be required.
    * #define CONSTANTS_IN_SHOUTING_ALL_CAPS is a great tradition but not
      a requirement. As a person who had first hand experience with FORTRAN
      in 1976 I beg to be excused for ALL_CAPS `readability' requirements.
    * #define null ((void*)0) - I've invented this decades before C++
      nullptr was even discussed. It's mine and I will stick with it.
    * IMHO countof() should be part of the language and yes pay attention
      to the fact that it should not be used on arrays decaying to pointers.
    * assert(bool, "printf formatted message", argv) is super easy to
      implement and super useful in debugging. If you don't like it
      ```#define assert(b, ...) assert(b)``` is a good compromise.
    * swear(bool, "printf formatted message", argv) is release mode
      fail fast fatal assertion.

*/