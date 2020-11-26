#pragma once

#include "data_accessor.h"
#include "data_provider.h"
#include "message.h"

#include <memory>
#include <vector>

struct AtIndexResult {
    size_t pointIndex;
    double point;
    double result;
};

class Solution {
public:
    virtual ~Solution() = default;

    virtual std::vector<AtIndexResult> run(
        const Task& task,
        const std::shared_ptr<DataAccessor>& dataAccessor,
        const std::shared_ptr<DataProvider>& dataProvider) const = 0;
};

std::shared_ptr<Solution> createDefaultSolution();
