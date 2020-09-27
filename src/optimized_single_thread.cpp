#include "solution.h"

#include <unordered_map>

namespace {

double calculateUpperBound(
    const std::vector<Item>& items,
    size_t capacity,
    size_t minUnusedIndex,
    size_t currentCapacity,
    double startBound)
{
    size_t remainingCap = capacity - currentCapacity;
    for (size_t i = minUnusedIndex; i != items.size() && remainingCap != 0; ++i) {
        size_t capNow = std::min(static_cast<uint32_t>(remainingCap), items[i].size);
        startBound += capNow * (static_cast<double>(items[i].cost) / items[i].size);
        remainingCap -= capNow;
    }
    return startBound;
}

class OptimizedSingleThreadSolution : public Solution {
public:
    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        std::vector<Item> sortedItems = items;
        std::vector<size_t> permutation;
        permutation.reserve(items.size());
        for (size_t i = 0; i != items.size(); ++i) {
            permutation.push_back(i);
        }
        std::sort(permutation.begin(), permutation.end(), [&items](size_t first, size_t second) {
            return items[first].cost * items[second].size > items[second].cost * items[first].size;
        });
        for (size_t i = 0; i != items.size(); ++i) {
            sortedItems[permutation[i]] = items[i];
        }
        run(sortedItems, capacity);
        return toResult(currentBest_, permutation);
    }

private:
    struct InternalResult {
        uint32_t cost;
        uint32_t capacity;
        std::vector<unsigned int> indices;
    };

    inline Result toResult(const InternalResult& result, const std::vector<size_t>& permutation)
    {
        std::unordered_map<size_t, size_t> backPermutation;
        for (size_t i = 0; i != permutation.size(); ++i) {
            backPermutation[i] = permutation[i];
        }
        std::set<unsigned int> res;
        for (const auto& value : result.indices) {
            res.insert(backPermutation[value]);
        }
        return Result { result.cost, result.capacity, res };
    }

    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
        }

        size_t index = items.size();
        double maxBound = 0;
        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity) {
                double upperBound = calculateUpperBound(
                    items,
                    capacity,
                    i + 1,
                    current_.capacity + items[i].size,
                    current_.cost + items[i].cost);
                if (upperBound > maxBound) {
                    maxBound = upperBound;
                    index = i;
                }
            }
        }
        if (index == items.size()) {
            return;
        }
        current_.capacity += items[index].size;
        current_.cost += items[index].cost;
        current_.indices.push_back(index);

        run(items, capacity, index + 1);

        current_.indices.pop_back();
        current_.capacity -= items[index].size;
        current_.cost -= items[index].cost;
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {} };
    InternalResult current_ = InternalResult { 0, 0, {} }; // Used while running
};

}

std::unique_ptr<Solution> createOptimizedSingleThreadSolution()
{
    return std::make_unique<OptimizedSingleThreadSolution>();
}
