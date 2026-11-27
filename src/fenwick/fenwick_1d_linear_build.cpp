
#include <iostream>
#include <vector>
#include "linear_build.hpp"
using namespace std;

class FenwickTree {
    int n;
    vector<long long> bit;

public:
    FenwickTree(const vector<long long>& arr) {
        n = arr.size();
        bit.assign(n + 1, 0);

        fenwick_detail::buildLinear(bit, [&](size_t i) { return arr[i]; });
    }

    void display() const {
        cout << "Fenwick Tree: ";

        for (int i = 1; i <= n; i++)
            cout << bit[i] << " ";

        cout << '\n';
    }

    long long prefixSum(int i) const {
        long long sum = 0;

        while (i > 0) {
            sum += bit[i];
            i -= i & -i;
        }

        return sum;
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<long long> arr = {2, 1, 3, 5, 4, 6, 7, 8};

    FenwickTree ft(arr);

    ft.display();

    cout << "Total sum: "
         << ft.prefixSum(arr.size()) << '\n';

    return 0;
}

#endif
