#include "solution.h"

bool operator < (const Result& first, const Result& second)
{
    if (first.capacity < second.capacity) {
        return true;
    }
    if (first.capacity > second.capacity) {
        return false;
    }
    return first.cost > second.cost;
}
