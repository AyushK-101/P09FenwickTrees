
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

using ll = long long;
using Clock = chrono::steady_clock;

#define TREE_LIBRARY_MODE

#include "../src/fenwick/fenwick_1d.cpp"
#include "../src/segment_tree/segment_1d.cpp"

struct Operation {
    bool isUpdate;
    int l;
    int r;
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

// Generate the same operations for both trees.
vector<Operation> generateWorkload(
    int n,
    int operations,
    int updatePercent,
    int batchSize,
    uint32_t seed
) {
    mt19937 rng(seed);

    uniform_int_distribution<int> indexDist(1, n);
    uniform_int_distribution<int> deltaDist(-100, 100);

    int updateCount = operations * updatePercent / 100;

    vector<vector<Operation>> batches;

    for (int i = 0; i < operations; i += batchSize) {
        bool isUpdate = i < updateCount;

        vector<Operation> batch;

        int count = min(batchSize, operations - i);

        for (int j = 0; j < count; j++) {
            int l = indexDist(rng);
            int r = indexDist(rng);

            if (l > r) {
                swap(l, r);
            }

            batch.push_back({
                isUpdate,
                l,
                r,
                deltaDist(rng)
            });
        }

        batches.push_back(move(batch));
    }

    shuffle(batches.begin(), batches.end(), rng);

    vector<Operation> workload;
    workload.reserve(operations);

    for (const auto& batch : batches) {
        workload.insert(
            workload.end(),
            batch.begin(),
            batch.end()
        );
    }

    return workload;
}

// Construct the Fenwick Tree using the existing point-update API.
FenwickTree buildFenwick(const vector<ll>& initial) {
    int n = static_cast<int>(initial.size());

    FenwickTree tree(n);

    for (int i = 0; i < n; i++) {
        tree.update(i + 1, initial[i]);
    }

    return tree;
}

// Time consecutive operations of the same type.
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

                tree.update(op.l, op.delta);
            }
        } else {
            for (size_t j = i; j < end; j++) {
                const Operation& op = workload[j];

                result.checksum += tree.rangeSum(
                    op.l, op.r
                );
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
    const vector<ll>& initial,
    const vector<Operation>& workload
) {
    auto start = Clock::now();

    FenwickTree tree = buildFenwick(initial);

    auto finish = Clock::now();

    Result result = runWorkload(tree, workload);

    result.buildMs =
        elapsedNs(start, finish) / 1e6;

    return result;
}

Result benchmarkSegment(
    const vector<ll>& initial,
    const vector<Operation>& workload
) {
    auto start = Clock::now();

    SegmentTree tree(initial);

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
    int operations,
    int updatePercent,
    int trial,
    const Result& result
) {
    csv << algorithm << ','
        << n << ','
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

    ofstream csv("results/raw/benchmark_1d.csv");

    if (!csv) {
        cerr << "Cannot create benchmark CSV\n";
        return 1;
    }

    csv << "algorithm,n,operations,update_percent,trial,"
           "build_ms,update_ns,query_ns,total_ms,"
           "throughput_ops_sec,checksum\n";

    const vector<int> sizes = {
        10000, 100000, 1000000
    };

    const vector<int> ratios = {
        90, 50, 10
    };

    const int operations = 100000;
    const int batchSize = 100;
    const int trials = 5;

    cout << fixed << setprecision(4);
    cout << "1D Fenwick vs Segment Tree Benchmark\n";

    for (int n : sizes) {
        mt19937 rng(42);

        uniform_int_distribution<int> valueDist(
            -100, 100
        );

        vector<ll> initial(n);

        for (ll& value : initial) {
            value = valueDist(rng);
        }

        for (int ratio : ratios) {
            vector<Operation> workload =
                generateWorkload(
                    n,
                    operations,
                    ratio,
                    batchSize,
                    12345
                );

            cout << "\nArray size: " << n
                 << " | Updates: " << ratio
                 << "% | Queries: " << 100 - ratio
                 << "%\n";

            for (int trial = 1; trial <= trials; trial++) {
                Result fenwick;
                Result segment;

                // Alternate execution order between trials.
                if (trial % 2 == 1) {
                    fenwick = benchmarkFenwick(
                        initial, workload
                    );

                    segment = benchmarkSegment(
                        initial, workload
                    );
                } else {
                    segment = benchmarkSegment(
                        initial, workload
                    );

                    fenwick = benchmarkFenwick(
                        initial, workload
                    );
                }

                if (fenwick.checksum != segment.checksum) {
                    cerr << "Checksum mismatch at N="
                         << n
                         << ", ratio=" << ratio
                         << ", trial=" << trial << '\n';

                    return 1;
                }

                writeCSV(
                    csv, "Fenwick",
                    n, operations,
                    ratio, trial, fenwick
                );

                writeCSV(
                    csv, "Segment",
                    n, operations,
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
    cout << "Results: results/raw/benchmark_1d.csv\n";

    return 0;
}
