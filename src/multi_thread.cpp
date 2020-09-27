#include "solution.h"

namespace {

class MultiThreadSolution : public Solution {
public:
    MultiThreadSolution()
    {}

    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        return Result {0, {}};
    }
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
