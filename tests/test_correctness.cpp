
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;
using ll = long long;

// Use actual source implementations.
// TREE_LIBRARY_MODE excludes their main() functions.
#define TREE_LIBRARY_MODE

#include "../src/fenwick/fenwick_1d.cpp"
#include "../src/fenwick/fenwick_2d.cpp"
#include "../src/segment_tree/segment_1d.cpp"
#include "../src/segment_tree/segment_2d.cpp"

// 1D CORRECTNESS TEST

void test1D() {
    mt19937 rng(42);

    const int n = 100;
    const int operations = 10000;

    uniform_int_distribution<int> values(-100, 100);
    uniform_int_distribution<int> indices(1, n);

    vector<ll> a(n);

    for (ll& x : a) {
        x = values(rng);
    }

    // Your FenwickTree constructor accepts size.
    FenwickTree ft(n);

    // Build Fenwick Tree using point updates.
    for (int i = 0; i < n; i++) {
        ft.update(i + 1, a[i]);
    }

    // Segment Tree constructor accepts vector.
    SegmentTree st(a);

    for (int t = 0; t < operations; t++) {

        // Randomly choose update or query.
        if (rng() % 2 == 0) {

            int i = indices(rng);
            ll delta = values(rng);

            // Update reference array.
            a[i - 1] += delta;

            // Update both actual implementations.
            ft.update(i, delta);
            st.update(i, delta);

        } else {

            int l = indices(rng);
            int r = indices(rng);

            if (l > r) {
                swap(l, r);
            }

            // Calculate expected result manually.
            ll expected = 0;

            for (int i = l; i <= r; i++) {
                expected += a[i - 1];
            }

            // Query actual implementations.
            ll fenwickResult = ft.rangeSum(l, r);
            ll segmentResult = st.rangeSum(l, r);

            if (fenwickResult != expected ||
                segmentResult != expected) {

                cerr << "\n1D TEST FAILED!\n";
                cerr << "Operation: " << t << '\n';
                cerr << "Range: [" << l << ", " << r << "]\n";
                cerr << "Expected: " << expected << '\n';
                cerr << "Fenwick: " << fenwickResult << '\n';
                cerr << "Segment: " << segmentResult << '\n';

                throw runtime_error("1D correctness failed");
            }
        }
    }

    cout << "1D: " << operations
         << " operations passed!\n";
}


// 2D CORRECTNESS TEST

void test2D() {
    mt19937 rng(123);

    const int n = 20;
    const int m = 25;
    const int operations = 5000;

    uniform_int_distribution<int> values(-50, 50);
    uniform_int_distribution<int> rows(1, n);
    uniform_int_distribution<int> cols(1, m);

    vector<vector<ll>> a(n, vector<ll>(m));

    for (auto& row : a) {
        for (ll& x : row) {
            x = values(rng);
        }
    }

    // Your 2D Fenwick Tree accepts rows and columns.
    Fenwick2D ft(n, m);

    // Build using point updates.
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            ft.update(i + 1, j + 1, a[i][j]);
        }
    }

    // Your 2D Segment Tree accepts a matrix.
    SegmentTree2D st(a);

    for (int t = 0; t < operations; t++) {

        if (rng() % 2 == 0) {

            int x = rows(rng);
            int y = cols(rng);
            ll delta = values(rng);

            // Update reference matrix.
            a[x - 1][y - 1] += delta;

            // Update actual implementations.
            ft.update(x, y, delta);
            st.update(x, y, delta);

        } else {

            int x1 = rows(rng);
            int x2 = rows(rng);
            int y1 = cols(rng);
            int y2 = cols(rng);

            if (x1 > x2) {
                swap(x1, x2);
            }

            if (y1 > y2) {
                swap(y1, y2);
            }

            // Calculate expected rectangle sum.
            ll expected = 0;

            for (int i = x1; i <= x2; i++) {
                for (int j = y1; j <= y2; j++) {
                    expected += a[i - 1][j - 1];
                }
            }

            // Query actual implementations.
            ll fenwickResult =
                ft.rectangleSum(x1, y1, x2, y2);

            ll segmentResult =
                st.rectangleSum(x1, y1, x2, y2);

            if (fenwickResult != expected ||
                segmentResult != expected) {

                cerr << "\n2D TEST FAILED!\n";
                cerr << "Operation: " << t << '\n';
                cerr << "Rectangle: ("
                     << x1 << "," << y1 << ") to ("
                     << x2 << "," << y2 << ")\n";

                cerr << "Expected: " << expected << '\n';
                cerr << "Fenwick: " << fenwickResult << '\n';
                cerr << "Segment: " << segmentResult << '\n';

                throw runtime_error("2D correctness failed");
            }
        }
    }

    cout << "2D: " << operations
         << " operations passed!\n";
}

// MAIN


int main() {
    try {
        
        cout << " FENWICK VS SEGMENT TREE TESTS\n";
        

        test1D();
        test2D();

        
        cout << "\n\nALL CORRECTNESS TESTS PASSED!\n";
        
    }
    catch (const exception& e) {
        cerr << "\nERROR: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
