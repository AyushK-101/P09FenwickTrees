
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

using ll = long long;
using Clock = chrono::steady_clock;

// Include the actual tree implementations without their main functions.
#define TREE_LIBRARY_MODE

#include "../src/fenwick/fenwick_2d.cpp"
#include "../src/segment_tree/segment_2d.cpp"

struct Operation {
    bool isUpdate;
    int x1, y1;
    int x2, y2;
    ll delta;
};

struct Result {
    double buildMs = 0;
    double updateNs = 0;
    double queryNs = 0;
    double totalMs = 0;
    double throughput = 0;
    ll checksum = 0;
};

volatile ll checksumSink = 0;

double elapsedNs(
    Clock::time_point start,
    Clock::time_point end
) {
    return chrono::duration<double, nano>(
        end - start
    ).count();
}

// Generate fixed-size batches of updates or queries.
// Shuffling the batches gives both trees the same operation order.
vector<Operation> generateWorkload(
    int n,
    int m,
    int operations,
    int updatePercent,
    int batchSize,
    uint32_t seed
) {
    mt19937 rng(seed);

    uniform_int_distribution<int> rowDist(1, n);
    uniform_int_distribution<int> colDist(1, m);
    uniform_int_distribution<int> deltaDist(-100, 100);

    int updateCount = operations * updatePercent / 100;

    vector<int> types;
    types.reserve(operations);

    for (int i = 0; i < operations; i += batchSize) {
        bool isUpdate = i < updateCount;

        int count = min(batchSize, operations - i);

        for (int j = 0; j < count; j++) {
            types.push_back(isUpdate ? 1 : 0);
        }
    }

    // Shuffle complete batches, not individual operations.
    vector<vector<int>> batches;

    for (int i = 0; i < operations; i += batchSize) {
        int end = min(i + batchSize, operations);

        batches.emplace_back(
            types.begin() + i,
            types.begin() + end
        );
    }

    shuffle(batches.begin(), batches.end(), rng);

    vector<Operation> workload;
    workload.reserve(operations);

    for (const auto& batch : batches) {
        for (int type : batch) {
            Operation op{};

            op.isUpdate = (type == 1);

            op.x1 = rowDist(rng);
            op.y1 = colDist(rng);

            if (op.isUpdate) {
                op.x2 = op.x1;
                op.y2 = op.y1;
                op.delta = deltaDist(rng);
            } else {
                op.x2 = rowDist(rng);
                op.y2 = colDist(rng);

                if (op.x1 > op.x2) {
                    swap(op.x1, op.x2);
                }

                if (op.y1 > op.y2) {
                    swap(op.y1, op.y2);
                }
            }

            workload.push_back(op);
        }
    }

    return workload;
}

// Construct the 2D Fenwick Tree from the input matrix.
Fenwick2D buildFenwick(
    const vector<vector<ll>>& matrix
) {
    int n = static_cast<int>(matrix.size());
    int m = static_cast<int>(matrix[0].size());

    Fenwick2D tree(n, m);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            tree.update(i + 1, j + 1, matrix[i][j]);
        }
    }

    return tree;
}

// Time consecutive operations of the same type together.
template <typename Tree>
Result runWorkload(
    Tree& tree,
    const vector<Operation>& workload
) {
    Result result;

    double updateTotalNs = 0;
    double queryTotalNs = 0;

    long long updateCount = 0;
    long long queryCount = 0;

    size_t i = 0;

    auto totalStart = Clock::now();

    while (i < workload.size()) {
        bool updateBatch = workload[i].isUpdate;

        size_t end = i;

        while (
            end < workload.size() &&
            workload[end].isUpdate == updateBatch
        ) {
            end++;
        }

        auto start = Clock::now();

        if (updateBatch) {
            for (size_t j = i; j < end; j++) {
                const Operation& op = workload[j];

                tree.update(
                    op.x1,
                    op.y1,
                    op.delta
                );
            }
        } else {
            for (size_t j = i; j < end; j++) {
                const Operation& op = workload[j];

                ll answer = tree.rectangleSum(
                    op.x1,
                    op.y1,
                    op.x2,
                    op.y2
                );

                result.checksum += answer;
            }
        }

        auto finish = Clock::now();

        double duration = elapsedNs(start, finish);

        if (updateBatch) {
            updateTotalNs += duration;
            updateCount += end - i;
        } else {
            queryTotalNs += duration;
            queryCount += end - i;
        }

        i = end;
    }

    auto totalEnd = Clock::now();

    result.totalMs =
        elapsedNs(totalStart, totalEnd) / 1e6;

    if (updateCount > 0) {
        result.updateNs =
            updateTotalNs / updateCount;
    }

    if (queryCount > 0) {
        result.queryNs =
            queryTotalNs / queryCount;
    }

    if (result.totalMs > 0) {
        result.throughput =
            workload.size() * 1000.0 / result.totalMs;
    }

    checksumSink = result.checksum;

    return result;
}

Result benchmarkFenwick(
    const vector<vector<ll>>& matrix,
    const vector<Operation>& workload
) {
    auto start = Clock::now();

    Fenwick2D tree = buildFenwick(matrix);

    auto finish = Clock::now();

    Result result = runWorkload(tree, workload);

    result.buildMs =
        elapsedNs(start, finish) / 1e6;

    return result;
}

Result benchmarkSegment(
    const vector<vector<ll>>& matrix,
    const vector<Operation>& workload
) {
    auto start = Clock::now();

    SegmentTree2D tree(matrix);

    auto finish = Clock::now();

    Result result = runWorkload(tree, workload);

    result.buildMs =
        elapsedNs(start, finish) / 1e6;

    return result;
}

void writeCSV(
    ofstream& csv,
    const string& algorithm,
    int n,
    int m,
    int operations,
    int updatePercent,
    int trial,
    const Result& result
) {
    csv << algorithm << ','
        << n << ','
        << m << ','
        << operations << ','
        << updatePercent << ','
        << trial << ','
        << fixed << setprecision(4)
        << result.buildMs << ','
        << result.updateNs << ','
        << result.queryNs << ','
        << result.totalMs << ','
        << result.throughput << ','
        << result.checksum << '\n';
}

int main() {
    filesystem::create_directories("results/raw");

    ofstream csv("results/raw/benchmark_2d.csv");

    if (!csv) {
        cerr << "Cannot create benchmark CSV\n";
        return 1;
    }

    csv << "algorithm,rows,cols,operations,update_percent,"
           "trial,build_ms,update_ns,query_ns,total_ms,"
           "throughput_ops_sec,checksum\n";

    const vector<int> sizes = {
        50, 100, 250
    };

    const vector<int> ratios = {
        90, 50, 10
    };

    const int operations = 10000;
    const int batchSize = 100;
    const int trials = 5;

    cout << fixed << setprecision(4);
    cout << "2D Fenwick vs Segment Tree Benchmark\n";

    for (int n : sizes) {
        int m = n;

        mt19937 rng(42);

        uniform_int_distribution<int> valueDist(
            -100, 100
        );

        vector<vector<ll>> matrix(
            n, vector<ll>(m)
        );

        for (auto& row : matrix) {
            for (ll& value : row) {
                value = valueDist(rng);
            }
        }

        for (int ratio : ratios) {
            vector<Operation> workload =
                generateWorkload(
                    n,
                    m,
                    operations,
                    ratio,
                    batchSize,
                    12345
                );

            cout << "\nMatrix: " << n << " x " << m
                 << " | Updates: " << ratio
                 << "% | Queries: " << 100 - ratio
                 << "%\n";

            for (int trial = 1; trial <= trials; trial++) {
                Result fenwick;
                Result segment;

                // Alternate execution order to reduce systematic bias.
                if (trial % 2 == 1) {
                    fenwick = benchmarkFenwick(
                        matrix, workload
                    );

                    segment = benchmarkSegment(
                        matrix, workload
                    );
                } else {
                    segment = benchmarkSegment(
                        matrix, workload
                    );

                    fenwick = benchmarkFenwick(
                        matrix, workload
                    );
                }

                if (fenwick.checksum != segment.checksum) {
                    cerr << "Checksum mismatch at "
                         << n << " x " << m
                         << ", ratio " << ratio
                         << ", trial " << trial << '\n';

                    cerr << "Fenwick: "
                         << fenwick.checksum << '\n';

                    cerr << "Segment: "
                         << segment.checksum << '\n';

                    return 1;
                }

                writeCSV(
                    csv, "Fenwick2D",
                    n, m, operations,
                    ratio, trial, fenwick
                );

                writeCSV(
                    csv, "Segment2D",
                    n, m, operations,
                    ratio, trial, segment
                );

                cout << "Trial " << trial
                     << " | Fenwick: "
                     << fenwick.totalMs << " ms"
                     << " | Segment: "
                     << segment.totalMs << " ms"
                     << " | Checksum: MATCH\n";
            }
        }
    }

    csv.close();

    cout << "\nBenchmark completed.\n";
    cout << "Results: results/raw/benchmark_2d.csv\n";

    return 0;
}
