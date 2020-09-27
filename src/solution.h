#pragma once

#include <cstddef>
#include <set>
#include <vector>
#include <memory>

struct Item {
    uint32_t cost;
    uint32_t size;
};

struct Result {
    uint32_t cost;
    uint32_t capacity;
    std::set<unsigned int> indices;
};

bool operator < (const Result& first, const Result& second);

class Solution {
public:
    virtual ~Solution() = default;

    virtual Result solve(const std::vector<Item>& items, size_t capacity) = 0;
};

std::unique_ptr<Solution> createSingleThreadSolution();
std::unique_ptr<Solution> createOptimizedSingleThreadSolution();
std::unique_ptr<Solution> createMultiThreadSolution();
