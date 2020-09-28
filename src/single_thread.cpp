#include "solution.h"

#include <numeric>
#include <iostream>
#include <unordered_map>

#include <cmd/cmd_utils.h>

namespace {

struct InternalResult {
    uint32_t cost;
    uint32_t capacity;
    std::set<size_t> included;
    std::set<size_t> excluded;
};
    
double calculateUpperBound(const std::vector<Item>& items, size_t capacity, const InternalResult& result)
{
    size_t remCap = capacity - result.capacity;
    double upperBound = result.cost;
    for (size_t i = 0; i != items.size(); ++i) {
        if (result.included.find(i) == result.included.end()
                && result.excluded.find(i) == result.excluded.end()) {
            size_t currentCap = std::min(remCap, static_cast<size_t>(result.capacity));
            upperBound += currentCap * (static_cast<double>(result.cost) / result.capacity);
            remCap -= currentCap;
        }
    }
    return upperBound;
}

class SingleThreadSolution : public Solution {
public:
    SingleThreadSolution()
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
    inline Result toResult(const InternalResult& result, const std::vector<size_t>& permutation)
    {
        std::unordered_map<size_t, size_t> backPermutation;
        for (size_t i = 0; i != permutation.size(); ++i) {
            backPermutation[i] = permutation[i];
        }
        std::set<unsigned int> res;
        for (const auto& value : result.included) {
            res.insert(backPermutation[value]);
        }
        return Result { result.cost, result.capacity, res };
    }

    void run(const std::vector<Item>& items, size_t capacity)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
        }

        if (calculateUpperBound(items, capacity, current_) <= currentBest_.cost) {
            return;
        }

        for (size_t i = 0; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity
                    && current_.included.find(i) == current_.included.end()
                    && current_.excluded.find(i) == current_.excluded.end()) {
                current_.capacity += items[i].size;
                current_.cost += items[i].cost;
                current_.included.insert(i);

                run(items, capacity);

                current_.included.erase(i);
                current_.capacity -= items[i].size;
                current_.cost -= items[i].cost;
            }
            if (current_.excluded.find(i) == current_.excluded.end() && current_.included.find(i) == current_.included.end()){
                current_.excluded.insert(i);
                run(items, capacity);
                current_.excluded.erase(i);
            }
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {}, {} };
    InternalResult current_ = InternalResult { 0, 0, {}, {} }; // Used while running
};

}

std::unique_ptr<Solution> createSingleThreadSolution()
{
    return std::make_unique<SingleThreadSolution>();
}
