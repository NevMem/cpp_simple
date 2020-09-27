#include "solution.h"

#include <numeric>
#include <iostream>

namespace {

class SingleThreadSolution : public Solution {
public:
    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        std::vector<Item> sortedItems = items;
        std::sort(sortedItems.begin(), sortedItems.end(), [](const Item& first, const Item& second) {
            return first.cost * second.size > second.cost * first.size;
        });
        run(sortedItems, capacity);
        return currentBest_;
    }

private:
    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
        }

        {
            const auto wholeRemainingCost = std::accumulate(
                items.begin() + minUnusedIndex,
                items.end(),
                0,
                [](size_t cost, const Item& item) { return cost + item.cost; });
            if (current_.cost + wholeRemainingCost <= currentBest_.cost) {
                return;
            }
        }

        {
            std::vector<int> can(capacity + 1, false);
            can[current_.capacity] = true;
            for (size_t i = minUnusedIndex; i != items.size(); ++i) {
                for (int cap = capacity; cap >= static_cast<int>(current_.capacity); --cap) {
                    if (can[cap]) {
                        int afterAdd = cap + items[i].size;
                        if (afterAdd <= capacity) {
                            can[afterAdd] = true;
                        }
                    }
                }
            }
            bool canAdd = false;
            for (size_t i = current_.capacity + 1; i != capacity + 1; ++i) {
                canAdd |= can[i];
            }
            if (!canAdd) {
                return;
            }
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

std::unique_ptr<Solution> createSingleThreadSolution()
{
    return std::make_unique<SingleThreadSolution>();
}
