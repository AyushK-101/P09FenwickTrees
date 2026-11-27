#pragma once

#include <cstddef>
#include <vector>

namespace fenwick_detail {

inline std::size_t lowBit(std::size_t i) {
    return i & -i;
}

// bit[0] is unused. A slot has received all of its children's
// contributions when its turn in this ascending pass arrives.
template <typename ValueAt>
void buildLinear(std::vector<long long>& bit, ValueAt valueAt) {
    for (std::size_t i = 1; i < bit.size(); ++i) {
        bit[i] += valueAt(i - 1);
        const std::size_t parent = i + lowBit(i);
        if (parent < bit.size())
            bit[parent] += bit[i];
    }
}

}  // namespace fenwick_detail
