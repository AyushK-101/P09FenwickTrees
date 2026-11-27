
#include <iostream>
#include <vector>
#include <stdexcept>
#include "linear_build.hpp"
using namespace std;

class FenwickOrderStatistics {
    int n;
    vector<long long> bit;
    vector<long long> values;

public:
    FenwickOrderStatistics(const vector<long long>& arr) {
        n = arr.size();
        bit.assign(n + 1, 0);
        values = arr;

        for (long long x : arr)
            if (x < 0)
                throw invalid_argument("Values must be nonnegative");

        fenwick_detail::buildLinear(bit, [&](size_t i) { return arr[i]; });
    }

    void update(int i, long long delta) {
        if (i < 1 || i > n)
            throw out_of_range("Invalid index");

        if (values[i - 1] + delta < 0)
            throw invalid_argument("Negative value not allowed");

        values[i - 1] += delta;

        while (i <= n) {
            bit[i] += delta;
            i += i & -i;
        }
    }

    long long prefixSum(int i) const {
        long long sum = 0;

        while (i > 0) {
            sum += bit[i];
            i -= i & -i;
        }

        return sum;
    }

    // First index with prefixSum(index) >= k
    int findByPrefix(long long k) const {
        if (k <= 0 || k > prefixSum(n))
            return -1;

        int pos = 0;
        long long accumulated = 0;

        // Largest power of 2 <= n
        int step = 1;
        while (step <= n / 2)
            step *= 2;

        while (step > 0) {
            int next = pos + step;

            if (next <= n &&
                accumulated + bit[next] < k) {
                accumulated += bit[next];
                pos = next;
            }

            step /= 2;
        }

        return pos + 1;
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<long long> arr = {2, 1, 3, 5, 4, 6, 7, 8};

    FenwickOrderStatistics ft(arr);

    cout << "First prefix >= 12: "
         << ft.findByPrefix(12) << '\n';

    cout << "First prefix >= 20: "
         << ft.findByPrefix(20) << '\n';

    ft.update(3, 5);

    cout << "After update, first prefix >= 12: "
         << ft.findByPrefix(12) << '\n';

    return 0;
}
#endif
