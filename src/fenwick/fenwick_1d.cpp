
#include <iostream>
#include <vector>
#include "linear_build.hpp"
using namespace std;

class FenwickTree {
private:
    int n;
    vector<long long> bit;

public:
    // Constructor
    FenwickTree(int size) {
        n = size;
        bit.assign(n + 1, 0);
    }

    explicit FenwickTree(const vector<long long>& arr)
        : n(static_cast<int>(arr.size())), bit(arr.size() + 1, 0) {
        fenwick_detail::buildLinear(bit, [&](size_t i) { return arr[i]; });
    }

    size_t storageBytes() const {
        return bit.capacity() * sizeof(long long);
    }

    // Add delta to position i
    void update(int i, long long delta) {
        while (i <= n) {
            bit[i] += delta;
            i += (i & -i);
        }
    }

    // Calculate sum from index 1 to i
    long long prefixSum(int i) {
        long long sum = 0;

        while (i > 0) {
            sum += bit[i];
            i -= (i & -i);
        }

        return sum;
    }

    // Calculate sum from index l to r
    long long rangeSum(int l, int r) {
        return prefixSum(r) - prefixSum(l - 1);
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<int> arr = {2, 1, 3, 5, 4, 6, 7, 8};

    int n = arr.size();

    FenwickTree ft(n);

    // Build Fenwick Tree
    for (int i = 0; i < n; i++) {
        ft.update(i + 1, arr[i]);
    }

    cout << "Prefix sum till index 7: "
         << ft.prefixSum(7) << '\n';

    cout << "Range sum [3, 6]: "
         << ft.rangeSum(3, 6) << '\n';

    // Add 5 to index 3
    ft.update(3, 5);

    cout << "After updating index 3 by +5:\n";

    cout << "Prefix sum till index 7: "
         << ft.prefixSum(7) << '\n';

    cout << "Range sum [3, 6]: "
         << ft.rangeSum(3, 6) << '\n';

    return 0;
}

#endif
