#pragma once

#include <vector>

struct Point {
    double x;
    double y;
};

struct BoundingBox {
    Point bottomLeft;
    Point topRight;
};

BoundingBox generateBoundingBox(const std::vector<Point>& points);

double distance(const Point& a, const Point& b);
