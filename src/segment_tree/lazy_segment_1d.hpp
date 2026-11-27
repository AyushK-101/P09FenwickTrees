#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

// Inclusive, 1-based ranges. Sums and tag arithmetic must fit in long long.
class LazySegmentTree {
public:
    explicit LazySegmentTree(const std::vector<long long>& values)
        : size_(values.size()),
          nodes_(size_ == 0 ? 0 : size_ + (size_ - 1)) {
        if (size_ != 0)
            build(0, 1, size_, values);
    }

    void rangeAdd(std::size_t left, std::size_t right, long long delta) {
        checkRange(left, right);
        update<false>(0, 1, size_, left, right, delta);
    }

    void rangeAssign(std::size_t left, std::size_t right, long long value) {
        checkRange(left, right);
        update<true>(0, 1, size_, left, right, value);
    }

    long long rangeSum(std::size_t left, std::size_t right) {
        checkRange(left, right);
        return query(0, 1, size_, left, right);
    }

    std::size_t storageBytes() const {
        return nodes_.capacity() * sizeof(Node);
    }

private:
    struct Node {
        long long sum = 0;
        long long pending = 0;
        bool assigns = false;
    };

    std::size_t size_;
    std::vector<Node> nodes_;

    void checkRange(std::size_t left, std::size_t right) const {
        if (left == 0 || left > right || right > size_)
            throw std::out_of_range("Invalid range");
    }

    // In preorder, the right child follows the 2 * leftLength - 1
    // nodes of the entire left subtree.
    static std::size_t rightChild(std::size_t node, std::size_t leftLength) {
        return node + 2 * leftLength;
    }

    void build(std::size_t node, std::size_t low, std::size_t high,
               const std::vector<long long>& values) {
        if (low == high) {
            nodes_[node].sum = values[low - 1];
            return;
        }
        const auto mid = low + (high - low) / 2;
        const auto right = rightChild(node, mid - low + 1);
        build(node + 1, low, mid, values);
        build(right, mid + 1, high, values);
        nodes_[node].sum = nodes_[node + 1].sum + nodes_[right].sum;
    }

    template <bool Assign>
    void apply(std::size_t node, std::size_t length, long long value) {
        auto& current = nodes_[node];
        const auto sum = static_cast<long long>(length) * value;
        if constexpr (Assign) {
            current.sum = sum;
            current.pending = value;
            current.assigns = true;
        } else {
            current.sum += sum;
            current.pending += value;
        }
    }

    void push(std::size_t node, std::size_t right,
              std::size_t leftLength, std::size_t rightLength) {
        auto& current = nodes_[node];
        if (current.assigns) {
            apply<true>(node + 1, leftLength, current.pending);
            apply<true>(right, rightLength, current.pending);
        } else if (current.pending != 0) {
            apply<false>(node + 1, leftLength, current.pending);
            apply<false>(right, rightLength, current.pending);
        } else {
            return;
        }
        current.pending = 0;
        current.assigns = false;
    }

    template <bool Assign>
    void update(std::size_t node, std::size_t low, std::size_t high,
                std::size_t left, std::size_t right, long long value) {
        if (left <= low && high <= right) {
            apply<Assign>(node, high - low + 1, value);
            return;
        }
        const auto mid = low + (high - low) / 2;
        const auto rightNode = rightChild(node, mid - low + 1);
        push(node, rightNode, mid - low + 1, high - mid);
        if (left <= mid)
            update<Assign>(node + 1, low, mid, left, right, value);
        if (right > mid)
            update<Assign>(rightNode, mid + 1, high, left, right, value);
        nodes_[node].sum = nodes_[node + 1].sum + nodes_[rightNode].sum;
    }

    long long query(std::size_t node, std::size_t low, std::size_t high,
                    std::size_t left, std::size_t right) {
        if (left <= low && high <= right)
            return nodes_[node].sum;

        const auto mid = low + (high - low) / 2;
        const auto rightNode = rightChild(node, mid - low + 1);
        push(node, rightNode, mid - low + 1, high - mid);
        long long sum = 0;
        if (left <= mid)
            sum += query(node + 1, low, mid, left, right);
        if (right > mid)
            sum += query(rightNode, mid + 1, high, left, right);
        return sum;
    }
};
