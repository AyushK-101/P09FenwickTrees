
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;
using ll = long long;

#define TREE_LIBRARY_MODE

// Rename the linear-build class during inclusion
// to avoid conflicting with other FenwickTree classes.
#define FenwickTree FenwickLinearBuild
#include "../src/fenwick/fenwick_1d_linear_build.cpp"
#undef FenwickTree

#include "../src/fenwick/fenwick_order_statistics.cpp"

// Test O(N) construction against direct prefix sums.
void testLinearBuild() {
    mt19937 rng(42);
    uniform_int_distribution<int> values(-100, 100);

    const int n = 1000;
    vector<ll> a(n);

    for (ll& x : a)
        x = values(rng);

    FenwickLinearBuild ft(a);

    ll expected = 0;

    for (int i = 1; i <= n; i++) {
        expected += a[i - 1];

        if (ft.prefixSum(i) != expected) {
            throw runtime_error(
                "Linear build failed at index " +
                to_string(i)
            );
        }
    }

    cout << "Linear build: 1000 prefix sums passed!\n";
}

// Reference: first index whose prefix sum >= k.
int linearSearch(const vector<ll>& a, ll k) {
    if (k <= 0)
        return -1;

    ll sum = 0;

    for (int i = 0; i < (int)a.size(); i++) {
        sum += a[i];

        if (sum >= k)
            return i + 1;
    }

    return -1;
}

void testOrderStatistics() {
    mt19937 rng(123);
    uniform_int_distribution<int> values(0, 20);

    const int n = 1000;
    const int operations = 10000;

    vector<ll> a(n);

    for (ll& x : a)
        x = values(rng);

    FenwickOrderStatistics ft(a);

    uniform_int_distribution<int> indices(1, n);
    uniform_int_distribution<int> changes(-10, 10);

    for (int t = 0; t < operations; t++) {
        if (rng() % 2 == 0) {
            int i = indices(rng);
            ll delta = changes(rng);

            // Keep frequencies nonnegative.
            if (a[i - 1] + delta < 0)
                delta = -a[i - 1];

            a[i - 1] += delta;
            ft.update(i, delta);
        } else {
            ll total = 0;
            for (ll x : a)
                total += x;

            uniform_int_distribution<ll> targets(0, total + 1);
            ll k = targets(rng);

            int expected = linearSearch(a, k);
            int actual = ft.findByPrefix(k);

            if (expected != actual) {
                cerr << "Order statistics failed at operation "
                     << t << ", target = " << k << '\n';
                cerr << "Expected: " << expected
                     << ", Actual: " << actual << '\n';

                throw runtime_error(
                    "Binary lifting correctness failed"
                );
            }
        }
    }

    cout << "Order statistics: 10000 operations passed!\n";
}

int main() {
    try {
        testLinearBuild();
        testOrderStatistics();

        cout << "\nALL ADVANCED FENWICK TESTS PASSED!\n";
    }
    catch (const exception& e) {
        cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
