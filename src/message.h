#pragma once

#include <cstddef>

template<int tag>
struct Message {
    static const int messageTag = tag;
};

/**
 * fromPoint respond to fromIndex
 * toPoint respond to toIndex
 * deltaTime - delta time
 * deltaPoints - distance between points, so (fromPoint - toPoint) / deltaPoins == toPoint - fromPoint
 */
struct Task : Message<0> {
    Task()
    {}

    Task(double from, double to, int fromI, int toI, double deltaT, double deltaS, size_t iters, double tCoef)
    : fromPoint(from)
    , toPoint(to)
    , fromIndex(fromI)
    , toIndex(toI)
    , deltaTime(deltaT)
    , deltaSpace(deltaS)
    , iterationsCount(iters)
    , coef(tCoef)
    {}

    Task(const Task& other)
    : Task(other.fromPoint, other.toPoint, other.fromIndex, other.toIndex, other.deltaTime,
        other.deltaSpace, other.iterationsCount, other.coef)
    {}

    double fromPoint, toPoint;
    int fromIndex, toIndex;
    double deltaTime, deltaSpace;
    size_t iterationsCount;
    double coef;
};

struct Value : Message<1> {
    Value()
    {}

    Value(double result)
    : value(result)
    {}

    double value;
};

struct ResultStreamEntity : Message<2> {
    ResultStreamEntity()
    {}

    ResultStreamEntity(double p, double v, int i)
    : point(p)
    , value(v)
    , index(i)
    {}

    double point;
    double value;
    int index;
};
