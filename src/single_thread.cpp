#include "solution.h"

#include <algorithm>
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
            size_t currentCap = std::min(remCap, static_cast<size_t>(items[i].size));
            upperBound += currentCap * (static_cast<double>(items[i].cost) / items[i].size);
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
            backPermutation[permutation[i]] = i;
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

        std::vector<int> values;
        for (size_t i = 0; i != items.size(); ++i) {
            if (current_.included.find(i) != current_.included.end() || current_.excluded.find(i) != current_.excluded.end()) {
                continue;
            }
            if (current_.capacity + items[i].size <= capacity) {
                const auto index = i;
                InternalResult copy = current_;
                copy.included.insert(index);
                copy.cost += items[index].cost;
                copy.capacity += items[index].size;
                if (calculateUpperBound(items, capacity, copy) >= currentBest_.cost) {
                    values.push_back((int)index + 1);
                }
            }
            {
                const auto index = i;
                InternalResult copy = current_;
                copy.excluded.insert(index);
                if (calculateUpperBound(items, capacity, copy) >= currentBest_.cost) {
                    values.push_back(-(int)index - 1);
                }
            }
        }

        for (const auto& value : values) {
            if (value > 0) {
                current_.included.insert(value - 1);
                current_.capacity += items[value - 1].size;
                current_.cost += items[value - 1].cost;
                run(items, capacity);
                current_.included.erase(value - 1);
                current_.capacity -= items[value - 1].size;
                current_.cost -= items[value - 1].cost;
                continue;
            }
            current_.excluded.insert(-value - 1);
            run(items, capacity);
            current_.excluded.erase(-value - 1);
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {}, {} };
    InternalResult current_ = InternalResult { 0, 0, {}, {} };
};

}

std::unique_ptr<Solution> createSingleThreadSolution()
{
    return std::make_unique<SingleThreadSolution>();
}
