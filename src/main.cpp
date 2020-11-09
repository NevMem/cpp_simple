#include <cassert>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <thread>
#include <vector>

#ifdef USE_OMP
#include <omp.h>
#endif

#include <logger/logger.h>

#include "data.h"
#include "point_generator.h"

typedef std::vector<size_t> Assignment;

std::vector<size_t> generateAssignments(
    const std::vector<Point>& points,
    const std::vector<Point>& centroids)
{
    Assignment assignment(points.size());
    const size_t pointsCount = points.size();

    #ifdef USE_OMP
    #pragma omp parallel num_threads(NUM_THREADS)
    #pragma omp for
    #endif
    for (size_t i = 0; i < pointsCount; ++i) {
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

    std::vector<std::vector<size_t>> indicesByAssignment(countCentroids, std::vector<size_t>());
    for (size_t i = 0; i != points.size(); ++i) {
        indicesByAssignment[assignment[i]].push_back(i);
    }

    #ifdef USE_OMP
    #pragma omp parallel num_threads(NUM_THREADS)
    #pragma omp for
    #endif
    for (size_t i = 0; i < countCentroids; ++i) {
        for (const auto& index : indicesByAssignment[i]) {
            centroids[i].x += points[index].x;
            centroids[i].y += points[index].y;
        }
        if (!indicesByAssignment[i].empty()) {
            centroids[i].x /= indicesByAssignment[i].size();
            centroids[i].y /= indicesByAssignment[i].size();
        }
    }

    return centroids;
}

int main() {
    logger::initializeLogger("run");
#ifndef USE_LOG
    logger::logger()->setLogingEnabled(false);
#endif

    size_t n, k;
    std::cin >> n >> k;
    std::vector<Point> points;
    points.reserve(n);
    for (size_t i = 0; i != n; ++i) {
        Point a;
        std::cin >> a.x >> a.y;
        points.push_back(a);
    }
    logger::logger()->log("loaded all data");

    auto centroids = generator::createRandomPointGenerator(0)->generatePoints(generateBoundingBox(points), k);
    auto assignment = generateAssignments(points, centroids);

    while (true) {
        logger::logger()->log("starting new step");
        auto newCentroids = generateCentroidsWithAssignment(points, assignment, k);
        centroids = std::move(newCentroids);
        auto newAssignment = generateAssignments(points, centroids);
        if (isSameAssignments(assignment, newAssignment)) {
            break;
        }
        assignment = std::move(newAssignment);
    }

    logger::logger()->log("writing output");

    for (size_t i = 0; i != centroids.size(); ++i) {
        std::cout << centroids[i].x << " " << centroids[i].y << std::endl;
    }
    for (size_t i = 0; i != assignment.size(); ++i) {
        std::cout << assignment[i] << std::endl;
    }
}
