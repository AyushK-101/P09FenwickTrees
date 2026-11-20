
#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class SegmentTree {
    int n;
    vector<long long> tree;

public:
    SegmentTree(const vector<long long>& arr) {
        n = arr.size();
        tree.assign(2 * n, 0);

        // Copy original array into leaves
        for (int i = 0; i < n; i++)
            tree[n + i] = arr[i];

        // Build parent nodes
        for (int i = n - 1; i > 0; i--)
            tree[i] = tree[2 * i] + tree[2 * i + 1];
    }

    // Add delta to 1-based index i
    void update(int i, long long delta) {
        if (i < 1 || i > n)
            throw out_of_range("Invalid index");

        int p = n + i - 1;

        tree[p] += delta;

        // Recalculate ancestors
        while (p > 1) {
            p /= 2;
            tree[p] = tree[2 * p] + tree[2 * p + 1];
        }
    }

    // Inclusive 1-based range sum
    long long rangeSum(int l, int r) const {
        if (l < 1 || r > n || l > r)
            throw out_of_range("Invalid range");

        // Convert to half-open leaf interval
        l = n + l - 1;
        r = n + r;

        long long sum = 0;

        while (l < r) {
            if (l & 1)
                sum += tree[l++];

            if (r & 1)
                sum += tree[--r];

            l /= 2;
            r /= 2;
        }

        return sum;
    }

    long long prefixSum(int i) const {
        if (i < 0 || i > n)
            throw out_of_range("Invalid prefix");

        if (i == 0)
            return 0;

        return rangeSum(1, i);
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<long long> arr = {2, 1, 3, 5, 4, 6, 7, 8};

    SegmentTree st(arr);

    cout << "Prefix sum [1, 7]: "
         << st.prefixSum(7) << '\n';

    cout << "Range sum [3, 6]: "
         << st.rangeSum(3, 6) << '\n';

    st.update(3, 5);

    cout << "After adding 5 at index 3:\n";

    cout << "Prefix sum [1, 7]: "
         << st.prefixSum(7) << '\n';

    cout << "Range sum [3, 6]: "
         << st.rangeSum(3, 6) << '\n';

    return 0;
}

#endif
