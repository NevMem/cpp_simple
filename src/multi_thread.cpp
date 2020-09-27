#include "solution.h"

#include <unordered_map>

namespace {

double calculateUpperBound(
    const std::vector<Item>& items,
    size_t capacity,
    size_t currentCapactity,
    size_t currentCost,
    size_t minUnusedIndex)
{
    double upperBound = currentCost;
    size_t remainingCap = capacity - currentCapactity;
    for (size_t i = minUnusedIndex; i != items.size() && remainingCap != 0; ++i) {
        size_t capNow = std::min(static_cast<uint32_t>(remainingCap), items[i].size);
        upperBound += capNow * (static_cast<double>(items[i].cost) / items[i].size);
        remainingCap -= capNow;
    }
    return upperBound;
}

class MultiThreadSolution : public Solution {
public:
    MultiThreadSolution()
    {}

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
    inline Result toResult(const Result& result, const std::vector<size_t>& permutation) const
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

        std::vector<size_t> shouldTry;

        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity) {
                if (calculateUpperBound(
                    items,
                    capacity,
                    current_.capacity + items[i].size,
                    current_.cost + items[i].cost,
                    i + 1) >= currentBest_.cost) {
                        shouldTry.push_back(i);
                    }
            }
        }

        for (const auto& index : shouldTry) {
            current_.capacity += items[index].size;
            current_.cost += items[index].cost;
            current_.indices.insert(index);

            run(items, capacity, index + 1);

            current_.indices.erase(index);
            current_.capacity -= items[index].size;
            current_.cost -= items[index].cost;
        }
    }

    Result currentBest_ = Result { 0, 0, {} };
    Result current_ = Result { 0, 0, {} };
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
