#include <cassert>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <thread>
#include <vector>
#include <omp.h>

#include "data.h"
#include "point_generator.h"

typedef std::vector<size_t> Assignment;

std::vector<size_t> generateAssignments(
    const std::vector<Point>& points,
    const std::vector<Point>& centroids)
{
    Assignment assignment(points.size());
    #pragma omp for
    for (size_t i = 0; i != points.size(); ++i) {
        double minDistance = distance(points[i], centroids[0]);
        size_t assign = 0;
        for (size_t j = 1; j != centroids.size(); ++j) {
            if (minDistance > distance(points[i], centroids[j])) {
                minDistance = distance(points[i], centroids[j]);
                assign = j;
            }
        }
        assignment[i] = assign;
    }
    return assignment;
}

bool isSameAssignments(const Assignment& first, const Assignment& second)
{
    assert(first.size() == second.size());

    bool isSame = true;
    for (size_t i = 0; i != first.size(); ++i) {
        if (first[i] != second[i]) {
            isSame = false;
        }
    }
    return isSame;
}

std::vector<Point> generateCentroidsWithAssignment(
    const std::vector<Point>& points,
    const Assignment& assignment,
    size_t countCentroids)
{
    std::vector<Point> centroids(countCentroids, {0, 0});
    std::vector<size_t> countPoints(countCentroids);

    for (size_t i = 0; i != points.size(); ++i) {
        countPoints[assignment[i]] += 1;
        centroids[assignment[i]].x += points[i].x;
        centroids[assignment[i]].y += points[i].y;
    }

    for (size_t i = 0; i != centroids.size(); ++i) {
        if (countPoints[i] > 0) {
            centroids[i].x /= countPoints[i];
            centroids[i].y /= countPoints[i];
        }
    }

    return centroids;
}

int main() {
    size_t n, k;
    std::cin >> n >> k;
    std::vector<Point> points;
    points.reserve(n);
    for (size_t i = 0; i != n; ++i) {
        Point a;
        std::cin >> a.x >> a.y;
        points.push_back(a);
    }

    auto centroids = generator::createRandomPointGenerator(0)->generatePoints(generateBoundingBox(points), k);
    auto assignment = generateAssignments(points, centroids);

    while (true) {
        auto newCentroids = generateCentroidsWithAssignment(points, assignment, k);
        centroids = std::move(newCentroids);
        auto newAssignment = generateAssignments(points, centroids);
        if (isSameAssignments(assignment, newAssignment)) {
            break;
        }
        assignment = std::move(newAssignment);
    }

    for (size_t i = 0; i != centroids.size(); ++i) {
        std::cout << centroids[i].x << " " << centroids[i].y << std::endl;
    }
    for (size_t i = 0; i != assignment.size(); ++i) {
        std::cout << assignment[i] << std::endl;
    }
}
