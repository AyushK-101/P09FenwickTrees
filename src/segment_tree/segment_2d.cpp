
#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class SegmentTree2D {
    int n, m;
    vector<vector<long long>> tree;

public:
    SegmentTree2D(const vector<vector<long long>>& arr) {
        n = arr.size();
        m = n ? arr[0].size() : 0;

        if (n == 0 || m == 0)
            throw invalid_argument("Empty matrix");

        for (const auto& row : arr)
            if ((int)row.size() != m)
                throw invalid_argument("Nonrectangular matrix");

        tree.assign(
            2 * n,
            vector<long long>(2 * m, 0)
        );

        // Build leaves of row trees
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++)
                tree[n + i][m + j] = arr[i][j];

            // Build columns for each leaf row
            for (int j = m - 1; j > 0; j--)
                tree[n + i][j] =
                    tree[n + i][2 * j] +
                    tree[n + i][2 * j + 1];
        }

        // Build internal row nodes
        for (int i = n - 1; i > 0; i--) {
            for (int j = 1; j < 2 * m; j++)
                tree[i][j] =
                    tree[2 * i][j] +
                    tree[2 * i + 1][j];
        }
    }

    // Add delta at 1-based cell (x,y)
    void update(int x, int y, long long delta) {
        if (x < 1 || x > n || y < 1 || y > m)
            throw out_of_range("Invalid coordinates");

        int row = n + x - 1;
        int col = m + y - 1;

        // Update every row and column ancestor
        for (int i = row; i > 0; i /= 2) {
            for (int j = col; j > 0; j /= 2) {
                tree[i][j] += delta;
            }
        }
    }

    // Inclusive 1-based rectangle sum
    long long rectangleSum(
        int x1, int y1, int x2, int y2
    ) const {
        if (x1 < 1 || y1 < 1 ||
            x2 > n || y2 > m ||
            x1 > x2 || y1 > y2)
            throw out_of_range("Invalid rectangle");

        // Convert rows to half-open leaf interval
        int top = n + x1 - 1;
        int bottom = n + x2;

        long long sum = 0;

        while (top < bottom) {
            if (top & 1)
                sum += queryColumns(top++, y1, y2);

            if (bottom & 1)
                sum += queryColumns(--bottom, y1, y2);

            top /= 2;
            bottom /= 2;
        }

        return sum;
    }

private:
    long long queryColumns(int row, int y1, int y2) const {
        int left = m + y1 - 1;
        int right = m + y2;

        long long sum = 0;

        while (left < right) {
            if (left & 1)
                sum += tree[row][left++];

            if (right & 1)
                sum += tree[row][--right];

            left /= 2;
            right /= 2;
        }

        return sum;
    }
};

#ifndef TREE_LIBRARY_MODE

int main() {
    vector<vector<long long>> arr = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    SegmentTree2D st(arr);

    cout << "Total sum: "
         << st.rectangleSum(1, 1, 3, 3) << '\n';

    cout << "Bottom-right 2x2 sum: "
         << st.rectangleSum(2, 2, 3, 3) << '\n';

    st.update(2, 2, 10);

    cout << "After adding 10 at (2,2): "
         << st.rectangleSum(1, 1, 3, 3) << '\n';

    return 0;
}
#endif