#pragma once

#include <stdexcept>
#include "linear_build.hpp"

// Inclusive, 1-based ranges. Arithmetic must fit in long long, including
// the differences, weighted differences, and prefix intermediates.
class RangeFenwickTree {
public:
    explicit RangeFenwickTree(const std::vector<long long>& values)
        : differences_(values.size() + 1, 0),
          weighted_(values.size() + 1, 0) {
        const auto difference = [&](std::size_t i) {
            return i == 0 ? values[0] : values[i] - values[i - 1];
        };
        fenwick_detail::buildLinear(differences_, difference);
        fenwick_detail::buildLinear(weighted_, [&](std::size_t i) {
            return static_cast<long long>(i) * difference(i);
        });
    }

    void rangeAdd(std::size_t left, std::size_t right, long long delta) {
        checkRange(left, right);
        addBoundary(left, delta);
        if (right + 1 < differences_.size())
            addBoundary(right + 1, -delta);
    }

    long long rangeSum(std::size_t left, std::size_t right) const {
        checkRange(left, right);
        return prefixSum(right) - prefixSum(left - 1);
    }

    std::size_t storageBytes() const {
        return (differences_.capacity() + weighted_.capacity()) * sizeof(long long);
    }

private:
    std::vector<long long> differences_;
    std::vector<long long> weighted_;

    void checkRange(std::size_t left, std::size_t right) const {
        if (left == 0 || left > right || right >= differences_.size())
            throw std::out_of_range("Invalid range");
    }

    void addBoundary(std::size_t i, long long delta) {
        const long long weightedDelta = static_cast<long long>(i - 1) * delta;
        for (; i < differences_.size(); i += fenwick_detail::lowBit(i)) {
            differences_[i] += delta;
            weighted_[i] += weightedDelta;
        }
    }

    long long prefixSum(std::size_t i) const {
        const auto length = static_cast<long long>(i);
        long long differenceSum = 0;
        long long weightedSum = 0;
        for (; i > 0; i -= fenwick_detail::lowBit(i)) {
            differenceSum += differences_[i];
            weightedSum += weighted_[i];
        }
        return length * differenceSum - weightedSum;
    }
};
