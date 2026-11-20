
#include <iostream>
#include <vector>
using namespace std;

class FenwickTree {
private:
    int n;
    vector<long long> bit;

public:
    FenwickTree(int size) {
        n = size;
        bit.assign(n + 1, 0);
    }

    // Point update: add delta at index i
    void update(int i, long long delta) {
        while (i <= n) {
            bit[i] += delta;
            i += (i & -i);
        }
    }

    // Print internal Fenwick array
    void display() {
        cout << "Fenwick Tree: ";

        for (int i = 1; i <= n; i++) {
            cout << bit[i] << " ";
        }

        cout << '\n';
    }
};

int main() {
    int n = 8;

    FenwickTree ft(n);

    // Insert initial values
    vector<int> arr = {2, 1, 3, 5, 4, 6, 7, 8};

    for (int i = 0; i < n; i++) {
        ft.update(i + 1, arr[i]);
    }

    cout << "Before update:\n";
    ft.display();

    // Add 5 to index 3
    ft.update(3, 5);

    cout << "After update(3, 5):\n";
    ft.display();

    return 0;
}
