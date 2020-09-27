#include "solution.h"

namespace {

class SingleThreadSolution : public Solution {
public:
    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        run(items, capacity);
        return currentBest_;
    }

private:
    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
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
