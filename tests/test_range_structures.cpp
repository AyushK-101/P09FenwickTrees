#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#define TREE_LIBRARY_MODE
#include "../src/fenwick/fenwick_1d.cpp"
#include "../src/segment_tree/segment_1d.cpp"
#include "../src/fenwick/range_fenwick_1d.hpp"
#include "../src/segment_tree/lazy_segment_1d.hpp"

namespace {
using Values = std::vector<long long>;

long long directSum(const Values& values, int left, int right) {
    return std::accumulate(values.begin() + left - 1, values.begin() + right, 0LL);
}

template <typename Tree>
void checkSum(Tree& tree, const Values& values, int left, int right,
              const char* implementation) {
    const auto expected = directSum(values, left, right);
    const auto actual = tree.rangeSum(left, right);
    if (actual != expected) {
        throw std::runtime_error(std::string(implementation) + " at [" +
            std::to_string(left) + ", " + std::to_string(right) + "]: expected " +
            std::to_string(expected) + ", got " + std::to_string(actual));
    }
}

void checkConstructionAndPointUpdates(int n, std::mt19937& rng) {
    std::uniform_int_distribution<int> value(-300, 300);
    std::uniform_int_distribution<int> index(1, n);
    Values values(n);
    for (auto& x : values)
        x = value(rng);

    FenwickTree linear(values);
    FenwickTree repeated(n);
    SegmentTree segment(values);
    RangeFenwickTree twoBit(values);
    LazySegmentTree lazy(values);
    for (int i = 1; i <= n; ++i)
        repeated.update(i, values[i - 1]);

    const auto checkAll = [&](int left, int right) {
        checkSum(linear, values, left, right, "linear Fenwick");
        checkSum(repeated, values, left, right, "repeated Fenwick");
        checkSum(segment, values, left, right, "iterative segment");
        checkSum(twoBit, values, left, right, "two-BIT Fenwick");
        checkSum(lazy, values, left, right, "lazy segment");
    };
    for (int i = 1; i <= n; ++i) {
        checkAll(i, i);
        checkAll(1, i);
    }
    for (int step = 0; step < 1000; ++step) {
        const int i = index(rng);
        const auto delta = value(rng);
        values[i - 1] += delta;
        linear.update(i, delta);
        repeated.update(i, delta);
        segment.update(i, delta);
        twoBit.rangeAdd(i, i, delta);
        lazy.rangeAdd(i, i, delta);
        const int j = index(rng);
        checkAll(std::min(i, j), std::max(i, j));
    }
}

void checkRandomRangeUpdates(int n, std::mt19937& rng) {
    std::uniform_int_distribution<int> value(-300, 300);
    std::uniform_int_distribution<int> index(1, n);
    std::uniform_int_distribution<int> operation(0, 2);
    Values shared(n);
    for (auto& x : shared)
        x = value(rng);
    Values mixed = shared;
    RangeFenwickTree twoBit(shared);
    LazySegmentTree lazyAdd(shared);
    LazySegmentTree lazyMixed(mixed);

    for (int step = 0; step < 4000; ++step) {
        int left = index(rng);
        int right = index(rng);
        if (left > right)
            std::swap(left, right);
        const int delta = value(rng);
        switch (operation(rng)) {
        case 0:
            twoBit.rangeAdd(left, right, delta);
            lazyAdd.rangeAdd(left, right, delta);
            lazyMixed.rangeAdd(left, right, delta);
            for (int i = left - 1; i < right; ++i) {
                shared[i] += delta;
                mixed[i] += delta;
            }
            break;
        case 1:
            lazyMixed.rangeAssign(left, right, delta);
            std::fill(mixed.begin() + left - 1, mixed.begin() + right, delta);
            break;
        case 2:
            checkSum(twoBit, shared, left, right, "two-BIT Fenwick");
            checkSum(lazyAdd, shared, left, right, "lazy range addition");
            checkSum(lazyMixed, mixed, left, right, "lazy mixed updates");
            break;
        }
    }
    for (int i = 1; i <= n; ++i) {
        checkSum(twoBit, shared, i, i, "two-BIT Fenwick");
        checkSum(lazyAdd, shared, i, i, "lazy range addition");
        checkSum(lazyMixed, mixed, i, i, "lazy mixed updates");
    }
}

void checkTagComposition() {
    LazySegmentTree lazy({3, -4, 1, 5, 6});
    lazy.rangeAdd(2, 4, -7);
    lazy.rangeAssign(1, 5, 0);  // Overwrite previously deferred additions.
    lazy.rangeAssign(1, 5, 0);  // Zero is a real assignment, including on repetition.
    lazy.rangeAdd(1, 5, -3);    // Fold an addition into a pending assignment.
    lazy.rangeAssign(2, 4, 5);
    lazy.rangeAdd(3, 5, -2);    // Overlap a pending assignment on a partial range.
    lazy.rangeAssign(4, 4, 0);
    lazy.rangeAssign(4, 4, 0);
    const Values expected = {-3, 5, 3, 0, -5};
    for (int left = 1; left <= 5; ++left)
        for (int right = left; right <= 5; ++right)
            checkSum(lazy, expected, left, right, "lazy tag composition");

    RangeFenwickTree twoBit({3, -4, 1, 5, 6});
    twoBit.rangeAdd(1, 5, -3);
    twoBit.rangeAdd(2, 4, 4);
    twoBit.rangeAdd(5, 5, -9);
    checkSum(twoBit, {0, -3, 2, 6, -6}, 1, 5, "two-BIT full range");
    checkSum(twoBit, {0, -3, 2, 6, -6}, 5, 5, "two-BIT last element");
}

template <typename Action>
void expectInvalidRange(Action action) {
    try {
        action();
    } catch (const std::out_of_range&) {
        return;
    }
    throw std::runtime_error("Invalid range accepted");
}

void checkBoundaries() {
    FenwickTree linear(Values{});
    if (linear.prefixSum(0) != 0)
        throw std::runtime_error("Empty Fenwick prefix failed");
    LazySegmentTree emptyLazy(Values{});
    RangeFenwickTree emptyBit(Values{});
    expectInvalidRange([&] { emptyLazy.rangeSum(1, 1); });
    expectInvalidRange([&] { emptyBit.rangeAdd(1, 1, 0); });
    LazySegmentTree lazy({7});
    RangeFenwickTree twoBit({7});
    expectInvalidRange([&] { lazy.rangeAssign(0, 1, 0); });
    expectInvalidRange([&] { lazy.rangeAdd(1, 2, 0); });
    expectInvalidRange([&] { twoBit.rangeSum(1, 0); });
    expectInvalidRange([&] { twoBit.rangeAdd(0, 1, 0); });
    lazy.rangeAssign(1, 1, 0);
    lazy.rangeAdd(1, 1, -5);
    twoBit.rangeAdd(1, 1, -12);
    checkSum(lazy, {-5}, 1, 1, "lazy singleton");
    checkSum(twoBit, {-5}, 1, 1, "two-BIT singleton");
}
}  // namespace

int main() {
    try {
        checkTagComposition();
        checkBoundaries();
        for (unsigned seed : {27, 42, 2026}) {
            std::mt19937 rng(seed);
            for (int n : {1, 2, 3, 7, 16, 31, 64, 129, 1000}) {
                checkConstructionAndPointUpdates(n, rng);
                checkRandomRangeUpdates(n, rng);
            }
        }
        std::cout << "Range structure tests passed (135,000 randomized steps across 27 cases).\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
