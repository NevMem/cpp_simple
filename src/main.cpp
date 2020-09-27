#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <set>
#include <future>

#include <threading/dispatcher/dispatchers.h>
#include <cmd/cmd_utils.h>

#include "solution.h"

namespace {

class IllegalStateException : public std::exception {
public:
    IllegalStateException(const std::string& message)
    : message_(message)
    {}

    virtual const char* what() const noexcept
    {
        return message_.data();
    }

private:
    const std::string message_;
};

}

std::unique_ptr<Solution> createSolution(int argc, char** argv)
{
    auto solutionType = cmd::getValue(argc, argv, "mode");
    if (!solutionType || *solutionType == "single") {
        return createSingleThreadSolution();
    }
    if (solutionType && *solutionType == "single_opt") {
        return createOptimizedSingleThreadSolution();
    }
    if (!solutionType || *solutionType != "multi") {
        throw IllegalStateException("Strange mode");
    }
    return createMultiThreadSolution();    
}

int main(int argc, char** argv) {
    threading::dispatcher::initializeDispatchers();

    size_t count = 0, capacity = 0;
    std::cin >> count >> capacity;
    std::vector<Item> items;
    items.reserve(count);
    for (size_t i = 0; i != count; ++i) {
        uint32_t cost, size;
        cost = size = 0;
        std::cin >> cost >> size;
        items.push_back({ cost, size });
    }

    auto solution = createSolution(argc, argv);

    auto result = threading::dispatcher::computation()->async([](
            Solution* const solution,
            const std::vector<Item>& items,
            size_t capacity) -> Result {
        return solution->solve(items, capacity);
    }, solution.get(), std::cref(items), capacity).get();

    std::cout << result.cost << "\n";
    for (const auto& index : result.indices) {
        std::cout << index + 1 << " ";
    }
    std::cout << std::endl;
}
