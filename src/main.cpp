#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <set>
#include <future>

#include <threading/dispatcher/dispatchers.h>
#include <cmd/cmd_utils.h>

#include "solution.h"

std::unique_ptr<Solution> createSolution(int argc, char** argv)
{
    auto solutionType = cmd::getValue(argc, argv, "solutionType");
    if (!solutionType || *solutionType == "single") {
        return createSingleThreadSolution();
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
