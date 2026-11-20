
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using ll = long long;

#define TREE_LIBRARY_MODE

#include "../src/fenwick/fenwick_2d.cpp"
#include "../src/segment_tree/segment_2d.cpp"

struct Operation {
    bool isUpdate;
    int x1, y1, x2, y2;
    ll delta;
};

vector<vector<ll>> generateMatrix(int rows, int cols, uint32_t seed) {
    mt19937 rng(seed);
    uniform_int_distribution<int> values(-100, 100);

    vector<vector<ll>> matrix(rows, vector<ll>(cols));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = values(rng);
        }
    }

    return matrix;
}

vector<Operation> generateWorkload(
    int rows, int cols, int operations,
    int updatePercent, uint32_t seed
) {
    mt19937 rng(seed);

    uniform_int_distribution<int> rowIndex(1, rows);
    uniform_int_distribution<int> colIndex(1, cols);
    uniform_int_distribution<int> deltas(-100, 100);

    vector<Operation> workload;
    workload.reserve(operations);

    int updateCount = operations * updatePercent / 100;

    for (int i = 0; i < operations; i++) {
        int x1 = rowIndex(rng);
        int x2 = rowIndex(rng);
        int y1 = colIndex(rng);
        int y2 = colIndex(rng);

        if (x1 > x2) swap(x1, x2);
        if (y1 > y2) swap(y1, y2);

        workload.push_back({
            i < updateCount,
            x1, y1, x2, y2,
            deltas(rng)
        });
    }

    shuffle(workload.begin(), workload.end(), rng);

    return workload;
}

template <typename Tree>
ll executeWorkload(Tree& tree, const vector<Operation>& workload) {
    ll checksum = 0;

    for (const Operation& op : workload) {
        if (op.isUpdate) {
            tree.update(op.x1, op.y1, op.delta);
        } else {
            checksum += tree.rectangleSum(
                op.x1, op.y1, op.x2, op.y2
            );
        }
    }

    return checksum;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "Usage: " << argv[0]
             << " <fenwick|segment> <rows> <cols>"
             << " <update_percent> <operations>\n";
        return 1;
    }

    try {
        string algorithm = argv[1];

        int rows = stoi(argv[2]);
        int cols = stoi(argv[3]);
        int updatePercent = stoi(argv[4]);
        int operations = stoi(argv[5]);

        if (algorithm != "fenwick" && algorithm != "segment") {
            throw invalid_argument("Unknown algorithm");
        }

        if (rows <= 0 || cols <= 0 || operations <= 0) {
            throw invalid_argument("Dimensions and operations must be positive");
        }

        if (updatePercent < 0 || updatePercent > 100) {
            throw invalid_argument("Update percentage must be between 0 and 100");
        }

        auto matrix = generateMatrix(rows, cols, 42);

        auto workload = generateWorkload(
            rows, cols, operations, updatePercent, 12345
        );

        ll checksum = 0;

        if (algorithm == "fenwick") {
            Fenwick2D tree(rows, cols);

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    tree.update(i + 1, j + 1, matrix[i][j]);
                }
            }

            checksum = executeWorkload(tree, workload);
        } else {
            SegmentTree2D tree(matrix);
            checksum = executeWorkload(tree, workload);
        }

        cout << "Algorithm: " << algorithm << '\n';
        cout << "Matrix: " << rows << " x " << cols << '\n';
        cout << "Operations: " << operations << '\n';
        cout << "Updates: " << updatePercent << "%\n";
        cout << "Checksum: " << checksum << '\n';

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
