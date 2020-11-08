#include "data.h"

#include <cmath>

BoundingBox generateBoundingBox(const std::vector<Point>& points)
{
    BoundingBox result;
    if (points.empty()) {
        return result;
    }

    result.bottomLeft = points[0];
    result.topRight = points[0];

    for (const auto& elem : points) {
        result.bottomLeft.x = std::min(result.bottomLeft.x, elem.x);
        result.bottomLeft.y = std::min(result.bottomLeft.y, elem.y);

        result.topRight.x = std::max(result.topRight.x, elem.x);
        result.topRight.y = std::max(result.topRight.y, elem.y);
    }

    return result;
}

double distance(const Point& a, const Point& b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
