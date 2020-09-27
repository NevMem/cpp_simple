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
        } else {
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
