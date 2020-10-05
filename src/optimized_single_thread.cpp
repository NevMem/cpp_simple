#include "solution.h"

#include <algorithm>
#include <unordered_map>

namespace {

double calculateUpperBound(
    const std::vector<Item>& items,
    size_t capacity,
    const Result& current,
    size_t minUnusedIndex)
{
    double upperBound = current.cost;
    size_t remainingCap = capacity - current.capacity;
    for (size_t i = minUnusedIndex; i != items.size() && remainingCap != 0; ++i) {
        size_t capNow = std::min(static_cast<uint32_t>(remainingCap), items[i].size);
        upperBound += capNow * (static_cast<double>(items[i].cost) / items[i].size);
        remainingCap -= capNow;
    }
    return upperBound;
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
    inline Result toResult(const Result& result, const std::vector<size_t>& permutation)
    {
        std::unordered_map<size_t, size_t> backPermutation;
        for (size_t i = 0; i != permutation.size(); ++i) {
            backPermutation[permutation[i]] = i;
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

        if (calculateUpperBound(items, capacity, current_, minUnusedIndex) <= currentBest_.cost) {
            return;
        }

        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity) {
                current_.capacity += items[i].size;
                current_.cost += items[i].cost;
                current_.indices.insert(i);

                run(items, capacity, i + 1);

                current_.indices.erase(i);
                current_.capacity -= items[i].size;
                current_.cost -= items[i].cost;
            }
        }
    }

    Result currentBest_ = Result { 0, 0, {} };
    Result current_ = Result { 0, 0, {} }; // Used while running
};

}

std::unique_ptr<Solution> createOptimizedSingleThreadSolution()
{
    return std::make_unique<OptimizedSingleThreadSolution>();
}
