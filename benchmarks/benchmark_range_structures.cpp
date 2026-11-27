#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#define TREE_LIBRARY_MODE
#include "../src/fenwick/fenwick_1d.cpp"
#include "../src/segment_tree/segment_1d.cpp"
#include "../src/fenwick/range_fenwick_1d.hpp"
#include "../src/segment_tree/lazy_segment_1d.hpp"

namespace {
using Clock = std::chrono::steady_clock;
using Values = std::vector<long long>;
namespace fs = std::filesystem;

constexpr std::array sizes{10'000, 100'000, 1'000'000};
constexpr std::array proportions{10, 50, 90};
constexpr int trials = 6;
constexpr int operations = 100'000;
constexpr unsigned inputSeed = 42;
constexpr std::uint64_t initialChecksum = 14695981039346656037ULL;

enum class Workload { PointAdd, RangeAdd, RangeAssign };
enum class Span { Point, Short, Wide };

struct Operation {
    bool update;
    int left;
    int right;
    int value;
};

struct Replay {
    double totalNs = 0;
    double updateNs = 0;
    double queryNs = 0;
    std::uint64_t checksum = initialChecksum;
};

long long elapsedNs(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

std::uint64_t hashAnswer(std::uint64_t hash, long long answer) {
    return (hash ^ static_cast<std::uint64_t>(answer)) * 1099511628211ULL;
}

Values generateInput(int n) {
    std::mt19937 rng(inputSeed);
    std::uniform_int_distribution<int> value(-100, 100);
    Values input(n);
    for (auto& x : input)
        x = value(rng);
    return input;
}

std::vector<Operation> generateOperations(int n, int updateCount,
                                         Span span, unsigned seed) {
    std::mt19937 typesRng(seed);
    std::vector<unsigned char> updates(operations);
    std::fill_n(updates.begin(), updateCount, 1);
    std::shuffle(updates.begin(), updates.end(), typesRng);

    // Generate all coordinates regardless of operation type, so profiles
    // share the same query ranges and one sampled endpoint for each update.
    std::mt19937 valuesRng(seed + 1);
    std::uniform_int_distribution<int> index(1, n);
    std::uniform_int_distribution<int> offset(0, 31);
    std::uniform_int_distribution<int> value(-100, 100);
    std::vector<Operation> stream;
    stream.reserve(operations);
    for (bool update : updates) {
        const int start = index(valuesRng);
        int end = index(valuesRng);
        const int shortEnd = std::min(n, start + offset(valuesRng));
        const int change = value(valuesRng);
        if (update && span == Span::Point)
            end = start;
        if (update && span == Span::Short)
            end = shortEnd;
        stream.push_back({update, std::min(start, end), std::max(start, end), change});
    }
    return stream;
}

struct RepeatedFenwick {
    explicit RepeatedFenwick(const Values& input)
        : tree(static_cast<int>(input.size())) {
        for (int i = 0; i < static_cast<int>(input.size()); ++i)
            tree.update(i + 1, input[i]);
    }

    long long rangeSum(int left, int right) { return tree.rangeSum(left, right); }
    std::size_t storageBytes() const { return tree.storageBytes(); }
    FenwickTree tree;
};

template <Workload Kind, typename Tree>
void applyUpdate(Tree& tree, const Operation& op) {
    if constexpr (Kind == Workload::PointAdd)
        tree.update(op.left, op.value);
    else if constexpr (Kind == Workload::RangeAdd)
        tree.rangeAdd(op.left, op.right, op.value);
    else
        tree.rangeAssign(op.left, op.right, op.value);
}

template <Workload Kind, bool MeasureLatency, typename Tree>
Replay replay(Tree& tree, const std::vector<Operation>& stream, int n) {
    Replay result;
    long long updateTotal = 0;
    long long queryTotal = 0;
    int updates = 0;
    int queries = 0;
    const auto start = Clock::now();
    for (const auto& op : stream) {
        if (op.update) {
            Clock::time_point startOperation;
            if constexpr (MeasureLatency)
                startOperation = Clock::now();
            applyUpdate<Kind>(tree, op);
            if constexpr (MeasureLatency) {
                updateTotal += elapsedNs(startOperation, Clock::now());
                ++updates;
            }
        } else {
            Clock::time_point startOperation;
            if constexpr (MeasureLatency)
                startOperation = Clock::now();
            const auto answer = tree.rangeSum(op.left, op.right);
            if constexpr (MeasureLatency) {
                queryTotal += elapsedNs(startOperation, Clock::now());
                ++queries;
            }
            result.checksum = hashAnswer(result.checksum, answer);
        }
    }
    const auto finish = Clock::now();
    if constexpr (MeasureLatency) {
        result.updateNs = static_cast<double>(updateTotal) / updates;
        result.queryNs = static_cast<double>(queryTotal) / queries;
    } else {
        result.totalNs = static_cast<double>(elapsedNs(start, finish));
    }
    result.checksum = hashAnswer(result.checksum, tree.rangeSum(1, n));
    return result;
}

template <typename Tree>
std::uint64_t runConstruction(std::ostream& csv, const char* name,
                              const Values& input, int trial) {
    const auto start = Clock::now();
    Tree tree(input);
    const auto finish = Clock::now();
    auto checksum = initialChecksum;
    const int n = static_cast<int>(input.size());
    for (int i = 1; i <= n; ++i) {
        const auto answer = tree.rangeSum(i, i);
        if (answer != input[i - 1])
            throw std::runtime_error("Construction did not reproduce the input");
        checksum = hashAnswer(checksum, answer);
    }
    csv << name << ',' << n << ',' << inputSeed << ',' << trial << ','
        << elapsedNs(start, finish) / 1e6 << ',' << tree.storageBytes() << ','
        << checksum << '\n';
    return checksum;
}

double timerPairNs() {
    constexpr int samples = 10'000;
    long long total = 0;
    for (int i = 0; i < samples; ++i) {
        const auto start = Clock::now();
        total += elapsedNs(start, Clock::now());
    }
    return static_cast<double>(total) / samples;
}

template <Workload Kind, typename Tree>
std::uint64_t runExperiment(std::ostream& csv, const char* name, const Values& input,
                            const std::vector<Operation>& stream, const char* span,
                            int ratio, unsigned workloadSeed, int trial, double clockNs) {
    const int n = static_cast<int>(input.size());
    double buildMs;
    std::size_t bytes;
    Replay mixed;
    {
        const auto start = Clock::now();
        Tree tree(input);
        const auto finish = Clock::now();
        buildMs = elapsedNs(start, finish) / 1e6;
        bytes = tree.storageBytes();
        mixed = replay<Kind, false>(tree, stream, n);
    }
    Tree latencyTree(input);
    const auto latency = replay<Kind, true>(latencyTree, stream, n);
    if (mixed.checksum != latency.checksum)
        throw std::runtime_error("Timing replays returned different checksums");

    csv << name << ',' << n << ',' << span << ',' << operations << ','
        << ratio << ',' << inputSeed << ',' << workloadSeed << ',' << trial << ','
        << buildMs << ',' << latency.updateNs << ',' << latency.queryNs << ','
        << mixed.totalNs / 1e6 << ',' << operations * 1e9 / mixed.totalNs << ','
        << bytes << ',' << clockNs << ',' << mixed.checksum << '\n';
    return mixed.checksum;
}

std::ofstream csvFile(const fs::path& path, const char* header) {
    std::ofstream csv;
    csv.exceptions(std::ios::failbit | std::ios::badbit);
    csv.open(path);
    csv.imbue(std::locale::classic());
    csv << header << '\n' << std::fixed << std::setprecision(6);
    return csv;
}

constexpr const char* experimentHeader =
    "algorithm,n,update_span,operations,update_percent,input_seed,workload_seed,"
    "trial,build_ms,update_ns,query_ns,total_ms,throughput_ops_sec,storage_bytes,"
    "clock_pair_ns,checksum";

void benchmarkConstruction(const fs::path& path) {
    auto csv = csvFile(path, "algorithm,n,input_seed,trial,build_ms,storage_bytes,checksum");
    using Measure = std::uint64_t (*)(std::ostream&, const char*, const Values&, int);
    const std::array<std::pair<const char*, Measure>, 3> algorithms{{
        {"Fenwick (linear)", runConstruction<FenwickTree>},
        {"Segment (iterative)", runConstruction<SegmentTree>},
        {"Fenwick (repeated)", runConstruction<RepeatedFenwick>},
    }};

    for (int n : sizes) {
        const auto input = generateInput(n);
        for (int trial = 1; trial <= trials; ++trial) {
            std::uint64_t expected = 0;
            const std::size_t first = static_cast<std::size_t>(trial - 1) % algorithms.size();
            for (std::size_t i = 0; i < algorithms.size(); ++i) {
                const auto offset = trial <= 3 ? i : algorithms.size() - i;
                const auto& [name, measure] = algorithms[(first + offset) % algorithms.size()];
                const auto checksum = measure(csv, name, input, trial);
                if (i > 0 && checksum != expected)
                    throw std::runtime_error("Construction checksum mismatch");
                expected = checksum;
            }
        }
    }
    csv.close();
}

template <Workload Kind, typename First, typename Second = First>
void benchmarkGroup(const fs::path& path, const char* firstName,
                    const char* secondName = nullptr) {
    auto csv = csvFile(path, experimentHeader);
    constexpr std::array rangeSpans{Span::Short, Span::Wide};
    constexpr std::array pointSpans{Span::Point};
    const auto& spans = [&]() -> const auto& {
        if constexpr (Kind == Workload::PointAdd)
            return pointSpans;
        else
            return rangeSpans;
    }();
    for (int n : sizes) {
        const auto input = generateInput(n);
        for (int ratio : proportions) {
            for (Span span : spans) {
                const char* spanName = span == Span::Short ? "short"
                                     : span == Span::Wide ? "wide" : "point";
                for (int trial = 1; trial <= trials; ++trial) {
                    const auto seed = 12345U + 100U * trial;
                    const int updates = operations * ratio / 100;
                    const auto stream = generateOperations(n, updates, span, seed);
                    const double clockNs = timerPairNs();
                    const auto first = [&] {
                        return runExperiment<Kind, First>(csv, firstName, input, stream,
                            spanName, ratio, seed, trial, clockNs);
                    };
                    if (secondName == nullptr) {
                        first();
                        continue;
                    }
                    const auto second = [&] {
                        return runExperiment<Kind, Second>(csv, secondName, input, stream,
                            spanName, ratio, seed, trial, clockNs);
                    };
                    const auto one = trial % 2 == 1 ? first() : second();
                    const auto two = trial % 2 == 1 ? second() : first();
                    if (one != two)
                        throw std::runtime_error("Paired experiment checksum mismatch");
                }
            }
        }
    }
    csv.close();
}

template <Workload Kind, typename Tree>
void profileReplay(Span span, int repeats) {
    constexpr int n = sizes.back();
    const auto input = generateInput(n);
    const auto stream = generateOperations(n, operations / 2, span, 12445);
    Tree tree(input);
    double replayNs = 0;
    std::uint64_t checksum = initialChecksum;
#ifdef _WIN32
    ULONG64 startCycles = 0;
    const bool cycleClockAvailable = QueryThreadCycleTime(GetCurrentThread(), &startCycles) != 0;
#endif
    for (int i = 0; i < repeats; ++i) {
        const auto result = replay<Kind, false>(tree, stream, n);
        replayNs += result.totalNs;
        checksum = (checksum ^ result.checksum) * 1099511628211ULL;
    }
#ifdef _WIN32
    ULONG64 finishCycles = 0;
    const bool cyclesMeasured = cycleClockAvailable &&
        QueryThreadCycleTime(GetCurrentThread(), &finishCycles) != 0 &&
        finishCycles >= startCycles;
#endif
    std::cout << "operations=" << static_cast<long long>(operations) * repeats
              << " checksum=" << checksum
              << " replay_ms=" << replayNs / 1e6
              << " storage_bytes=" << tree.storageBytes();
#ifdef _WIN32
    if (cyclesMeasured)
        std::cout << " thread_cycles=" << finishCycles - startCycles;
#endif
    std::cout << '\n';
}

void profile(const std::string& algorithm, int repeats) {
    if (repeats < 1 || repeats > 1000)
        throw std::invalid_argument("Profile repeats must be in [1, 1000]");

    if (algorithm == "point-fenwick")
        profileReplay<Workload::PointAdd, FenwickTree>(Span::Point, repeats);
    else if (algorithm == "point-segment")
        profileReplay<Workload::PointAdd, SegmentTree>(Span::Point, repeats);
    else if (algorithm == "range-fenwick")
        profileReplay<Workload::RangeAdd, RangeFenwickTree>(Span::Wide, repeats);
    else if (algorithm == "range-lazy")
        profileReplay<Workload::RangeAdd, LazySegmentTree>(Span::Wide, repeats);
    else if (algorithm == "assign-lazy")
        profileReplay<Workload::RangeAssign, LazySegmentTree>(Span::Wide, repeats);
    else
        throw std::invalid_argument("Unknown profile workload");
}
}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2 && std::string(argv[1]) == "--profile") {
            if (argc != 4)
                throw std::invalid_argument("Usage: benchmark_range_structures --profile <mode> <repeats>");
            const std::string repeatArgument(argv[3]);
            std::size_t parsed = 0;
            const int repeats = std::stoi(repeatArgument, &parsed);
            if (parsed != repeatArgument.size())
                throw std::invalid_argument("Profile repeats must be an integer");
            profile(argv[2], repeats);
            return 0;
        }
        if (argc != 2)
            throw std::invalid_argument("Usage: benchmark_range_structures <new-output-directory>");
        const fs::path output(argv[1]);
        if (!fs::create_directories(output))
            throw std::runtime_error("Choose a new output directory");
        benchmarkConstruction(output / "construction.csv");
        benchmarkGroup<Workload::PointAdd, FenwickTree, SegmentTree>(
            output / "experiment_a.csv", "Fenwick (linear)", "Segment (iterative)");
        benchmarkGroup<Workload::RangeAdd, RangeFenwickTree, LazySegmentTree>(
            output / "experiment_b.csv", "Fenwick (two BITs)", "Segment (lazy)");
        benchmarkGroup<Workload::RangeAssign, LazySegmentTree>(
            output / "experiment_c.csv", "Segment (lazy)");
        std::cout << "Validated benchmark CSVs written to " << output << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
