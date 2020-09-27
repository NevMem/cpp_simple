#include "solution.h"

#include <numeric>
#include <iostream>

#include <cmd/cmd_utils.h>

namespace {

class SingleThreadSolution : public Solution {
public:
    SingleThreadSolution(int argc, char** argv)
    : useSimpleOpt_(cmd::hasValueInArgs(argc, argv, "useSimpleOpt"))
    {}

    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        std::vector<Item> sortedItems = items;
        std::sort(sortedItems.begin(), sortedItems.end(), [](const Item& first, const Item& second) {
            return first.cost * second.size > second.cost * first.size;
        });
        run(sortedItems, capacity);
        return toResult(currentBest_);
    }

private:
    struct InternalResult {
        uint32_t cost;
        uint32_t capacity;
        std::vector<unsigned int> indices;
    };

    inline Result toResult(const InternalResult& result)
    {
        return Result { result.cost, result.capacity, {result.indices.begin(), result.indices.end()} };
    }

    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
        } else if (useSimpleOpt_) {
            double upperBound = current_.cost;
            size_t remainingCap = capacity - current_.capacity;
            for (size_t i = minUnusedIndex; i != items.size(); ++i) {
                if (remainingCap == 0)
                    break;
                size_t capNow = std::min(static_cast<uint32_t>(remainingCap), items[i].size);
                upperBound += capNow * (static_cast<double>(items[i].cost) / items[i].size);
                remainingCap -= capNow;
            }
            if (upperBound <= currentBest_.cost) {
                return;
            }
        }

        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity) {
                current_.capacity += items[i].size;
                current_.cost += items[i].cost;
                current_.indices.push_back(i);

                run(items, capacity, i + 1);

                current_.indices.pop_back();
                current_.capacity -= items[i].size;
                current_.cost -= items[i].cost;
            }
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {} };
    InternalResult current_ = InternalResult { 0, 0, {} }; // Used while running

    bool useSimpleOpt_;
};

}

std::unique_ptr<Solution> createSingleThreadSolution(int argc, char** argv)
{
    return std::make_unique<SingleThreadSolution>(argc, argv);
}
