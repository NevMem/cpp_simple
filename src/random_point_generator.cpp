#include "point_generator.h"

namespace generator {

namespace {

class RandomPointGenerator : public PointGenerator {
public:
    RandomPointGenerator(int seed)
    : seed_(seed)
    {}

    virtual std::vector<Point> generatePoints(const BoundingBox& box, size_t count)
    {
        std::vector<Point> result;
        result.reserve(count);
        srand(seed_);
        for (size_t i = 0; i != count; ++i) {
            result.push_back(generatePointInBoundingBox(box));
        }
        return result;
    }

private:
    static Point generatePointInBoundingBox(const BoundingBox& b)
    {
        static constexpr double MAX_VALUE = static_cast<long long>(RAND_MAX) * RAND_MAX;
        double x_d = ((rand() * RAND_MAX) + rand()) / MAX_VALUE;
        double y_d = ((rand() * RAND_MAX) + rand()) / MAX_VALUE;
        return Point {
            b.bottomLeft.x + x_d * (b.topRight.x - b.bottomLeft.x),
            b.bottomLeft.y + y_d * (b.topRight.y - b.bottomLeft.y)
        };
    }

    const int seed_;
};

}

std::shared_ptr<PointGenerator> createRandomPointGenerator(int seed)
{
    return std::make_shared<RandomPointGenerator>(seed);
}

}
