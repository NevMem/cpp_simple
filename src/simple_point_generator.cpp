#include "point_generator.h"

namespace generator {

namespace {

class SimplePointGenerator : public PointGenerator {
public:
    virtual std::vector<Point> generatePoints(const BoundingBox& bb, size_t count)
    {
        std::vector<Point> result;
        result.reserve(count);

        Point point = bb.bottomLeft;
        result.push_back(point);

        for (size_t i = 0; i + 1 < count; ++i) {
            point.x += (bb.topRight.x - bb.bottomLeft.x) / count;
            point.y += (bb.topRight.y - bb.bottomLeft.y) / count;
            result.push_back(point);
        }

        return result;
    }
};

}

std::shared_ptr<PointGenerator> createSimplePointGenerator()
{
    return std::make_shared<SimplePointGenerator>();
}

}
