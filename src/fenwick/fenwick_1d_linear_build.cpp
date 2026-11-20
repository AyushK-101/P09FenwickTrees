
#include <iostream>
#include <vector>
using namespace std;

class FenwickTree {
    int n;
    vector<long long> bit;

public:
    FenwickTree(const vector<long long>& arr) {
        n = arr.size();
        bit.assign(n + 1, 0);

        // O(N) construction
        for (int i = 1; i <= n; i++) {
            bit[i] += arr[i - 1];

            int parent = i + (i & -i);

            if (parent <= n)
                bit[parent] += bit[i];
        }
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
