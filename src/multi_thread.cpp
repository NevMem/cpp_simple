#include "solution.h"

#include <algorithm>
#include <unordered_map>
#include <mutex>

#include <threading/dispatcher/dispatchers.h>

namespace {

struct InternalResult {
    uint32_t cost;
    uint32_t capacity;
    std::set<size_t> included;
};
    
double calculateUpperBound(const std::vector<Item>& items, size_t capacity, size_t currentCost, size_t currentCapacity, size_t minUnusedIndex)
{
    size_t remCap = capacity - currentCapacity;
    double upperBound = currentCost;
    for (size_t i = minUnusedIndex; i != items.size() && remCap != 0; ++i) {
        size_t currentCap = std::min(remCap, static_cast<size_t>(items[i].size));
        upperBound += currentCap * (static_cast<double>(items[i].cost) / items[i].size);
        remCap -= currentCap;
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
        run(sortedItems, capacity, 0);
        return toResult(currentBest_, permutation);
    }

private:
    inline Result toResult(const InternalResult& result, const std::vector<size_t>& permutation) const
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

    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex)
    {
        {
            if (currentBest_.cost < current_.cost) {
                currentBest_ = current_;
            }
        }

        std::vector<std::future<void>> futures;
        std::vector<std::pair<double, size_t>> values;
        std::mutex writeMutex;
        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            futures.push_back(threading::dispatcher::computation()->async([this, &items, &writeMutex, capacity](size_t index, std::vector<std::pair<double, size_t>>& values) {
                if (current_.capacity + items[index].size <= capacity) {
                    const auto upperBound = calculateUpperBound(items, capacity, current_.cost + items[index].cost, current_.capacity + items[index].size, index + 1);
                    if (upperBound >= currentBest_.cost) {
                        std::lock_guard<std::mutex> guard(writeMutex);
                        values.push_back({ upperBound, index });
                    }
                }
            }, i, std::ref(values)));
        }

        for (auto& future : futures) {
            future.get();
        }

        std::sort(values.rbegin(), values.rend());

        for (const auto& value : values) {
            current_.included.insert(value.second);
            current_.capacity += items[value.second].size;
            current_.cost += items[value.second].cost;
            run(items, capacity, value.second + 1);
            current_.included.erase(value.second);
            current_.capacity -= items[value.second].size;
            current_.cost -= items[value.second].cost;
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {} };
    InternalResult current_ = InternalResult { 0, 0, {} };
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
