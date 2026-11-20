
#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class FenwickTree {
    int n;
    vector<long long> bit;

public:
    FenwickTree(int size) : n(size), bit(size + 1, 0) {}

    void update(int i, long long delta) {
        if (i < 1 || i > n)
            throw out_of_range("Invalid index");

        while (i <= n) {
            bit[i] += delta;
            i += i & -i;
        }
    }

    long long prefixSum(int i) const {
        if (i < 0 || i > n)
            throw out_of_range("Invalid index");

        long long sum = 0;

        while (i > 0) {
            sum += bit[i];
            i -= i & -i;
        }

        return sum;
    }

    long long rangeSum(int l, int r) const {
        if (l < 1 || r > n || l > r)
            throw out_of_range("Invalid range");

        return prefixSum(r) - prefixSum(l - 1);
    }
};

int main() {
    vector<long long> arr = {2, 1, 3, 5, 4, 6, 7, 8};
    int n = arr.size();

    FenwickTree ft(n);

    for (int i = 0; i < n; i++)
        ft.update(i + 1, arr[i]);

    cout << "Prefix sum [1, 7]: "
         << ft.prefixSum(7) << '\n';

    cout << "Range sum [3, 6]: "
         << ft.rangeSum(3, 6) << '\n';

    return 0;
}
