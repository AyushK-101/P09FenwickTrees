
#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class Fenwick2D {
    int n, m;
    vector<vector<long long>> bit;

public:
    Fenwick2D(int rows, int cols)
        : n(rows), m(cols),
          bit(rows + 1, vector<long long>(cols + 1, 0)) {}

    // Add delta to cell (x, y)
    void update(int x, int y, long long delta) {
        if (x < 1 || x > n || y < 1 || y > m)
            throw out_of_range("Invalid coordinates");

        for (int i = x; i <= n; i += i & -i) {
            for (int j = y; j <= m; j += j & -j) {
                bit[i][j] += delta;
            }
        }
    }

    // Sum of rectangle (1,1) to (x,y)
    long long prefixSum(int x, int y) const {
        long long sum = 0;

        for (int i = x; i > 0; i -= i & -i) {
            for (int j = y; j > 0; j -= j & -j) {
                sum += bit[i][j];
            }
        }

        return sum;
    }

    // Sum of rectangle (x1,y1) to (x2,y2)
    long long rectangleSum(
        int x1, int y1, int x2, int y2
    ) const {
        if (x1 < 1 || y1 < 1 ||
            x2 > n || y2 > m ||
            x1 > x2 || y1 > y2)
            throw out_of_range("Invalid rectangle");

        return prefixSum(x2, y2)
             - prefixSum(x1 - 1, y2)
             - prefixSum(x2, y1 - 1)
             + prefixSum(x1 - 1, y1 - 1);
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<vector<long long>> arr = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    int n = arr.size();
    int m = arr[0].size();

    Fenwick2D ft(n, m);

    // Build by inserting each element
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            ft.update(i + 1, j + 1, arr[i][j]);
        }
    }

    cout << "Total sum: "
         << ft.rectangleSum(1, 1, 3, 3) << '\n';

    cout << "Bottom-right 2x2 sum: "
         << ft.rectangleSum(2, 2, 3, 3) << '\n';

    ft.update(2, 2, 10);

    cout << "After adding 10 at (2,2): "
         << ft.rectangleSum(1, 1, 3, 3) << '\n';

    return 0;
}
#endif