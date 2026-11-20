
#include <bits/stdc++.h>

using namespace std;

using ll = long long;

#define TREE_LIBRARY_MODE

#include "../src/fenwick/fenwick_1d.cpp"
#include "../src/segment_tree/segment_1d.cpp"

struct Operation {
    bool isUpdate;
    int l;
    int r;
    ll delta;
};

vector<ll> generateArray(int n, uint32_t seed) {
    mt19937 rng(seed);
    uniform_int_distribution<int> values(-100, 100);

    vector<ll> a(n);

    for (ll& x : a) {
        x = values(rng);
    }

    return a;
}

vector<Operation> generateWorkload(
    int n,
    int operations,
    int updatePercent,
    uint32_t seed
) {
    mt19937 rng(seed);

    uniform_int_distribution<int> indices(1, n);
    uniform_int_distribution<int> deltas(-100, 100);

    vector<Operation> workload;
    workload.reserve(operations);

    int updateCount =
        operations * updatePercent / 100;

    for (int t = 0; t < operations; t++) {
        int l = indices(rng);
        int r = indices(rng);

        if (l > r) {
            swap(l, r);
        }

        workload.push_back({
            t < updateCount,
            l,
            r,
            deltas(rng)
        });
    }

    shuffle(workload.begin(), workload.end(), rng);

    return workload;
}

template <typename Tree>
ll executeWorkload(
    Tree& tree,
    const vector<Operation>& workload
) {
    ll checksum = 0;

    for (const Operation& op : workload) {
        if (op.isUpdate) {
            tree.update(op.l, op.delta);
        } else {
            checksum += tree.rangeSum(op.l, op.r);
        }
    }

    return checksum;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0]
             << " <fenwick|segment> <n>"
             << " <update_percent> <operations>\n";
        return 1;
    }

    string algorithm = argv[1];

    int n = stoi(argv[2]);
    int updatePercent = stoi(argv[3]);
    int operations = stoi(argv[4]);

    if (n <= 0 ||
        operations <= 0 ||
        updatePercent < 0 ||
        updatePercent > 100) {
        cerr << "Invalid parameters\n";
        return 1;
    }

    if (algorithm != "fenwick" &&
        algorithm != "segment") {
        cerr << "Unknown algorithm\n";
        return 1;
    }

    vector<ll> initial = generateArray(n, 42);

    vector<Operation> workload = generateWorkload(
        n, operations, updatePercent, 12345
    );

    ll checksum = 0;

    if (algorithm == "fenwick") {
        FenwickTree tree(n);

        for (int i = 0; i < n; i++) {
            tree.update(i + 1, initial[i]);
        }

        checksum = executeWorkload(tree, workload);
    } else {
        SegmentTree tree(initial);

        checksum = executeWorkload(tree, workload);
    }

    cout << "Algorithm: " << algorithm << '\n';
    cout << "Array size: " << n << '\n';
    cout << "Update percentage: "
         << updatePercent << '\n';
    cout << "Operations: " << operations << '\n';
    cout << "Checksum: " << checksum << '\n';

    return 0;
}
