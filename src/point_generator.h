#pragma once

#include "data.h"

namespace generator {

class PointGenerator {
public:
    virtual ~PointGenerator() = default;

    virtual std::vector<Point> generatePoints(const BoundingBox& boundingBox, size_t count) = 0;
};

std::shared_ptr<PointGenerator> createSimplePointGenerator();

// Seed will be set before EACH generation of points
std::shared_ptr<PointGenerator> createRandomPointGenerator(int seed);

}
