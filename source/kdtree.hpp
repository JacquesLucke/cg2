#pragma once

#include <iostream>
#include <cassert>
#include <array>
#include <vector>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <chrono>
#include <queue>
#include <cmath>
#include <thread>
#include <fstream>
#include <string>
#include <limits>

#include <glm/glm.hpp>

#include "bounding_box.hpp"
#include "timer.hpp"
#include "random.hpp"
#include "utils.hpp"

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

template<
    class Point,
    int ndim,
    float (*CalcDistance)(Point &a, Point &b)>
class KDTree {
    Point *points;
    int length;
    int bucketSize;
    int threadDepth;

public:

    KDTree(Point *points, int length, int bucketSize) {
        assert(ndim >= 1);
        assert(length >= 0);
        assert(bucketSize >= 1);

        this->points = points;
        this->length = length;
        this->bucketSize = bucketSize;
        this->threadDepth = (int)std::log2(getCpuCoreCount()) + 1;
    }

    KDTree(std::vector<Point> &points, int buckedSize)
        : KDTree(points.data(), points.size(), buckedSize) {}

    void balance() {
        TIMEIT("balance")
        sort(0, length - 1, 0);
    }

    std::vector<Point> collectInRadius(Point &origin, float radius) {
        InRadiusCollector collector(origin, radius);
        collectInRadius(0, length - 1, 0, collector);
        return collector.getPoints();;
    }

    std::vector<Point> collectKNearest(Point &origin, int k) {
        KNearestCollector collector(origin, k);
        collectKNearest(0, length - 1, 0, collector);
        return collector.getPoints();
    }

    std::vector<Point> collectInRadius_Naive(Point &origin, float radius) {
        return collectNaive(InRadiusCollector(origin, radius));
    }

    std::vector<Point> collectKNearest_Naive(Point &origin, int k) {
        return collectNaive(KNearestCollector(origin, k));
    }

    struct BoundingBoxWithDepth {
        BoundingBox<ndim> box;
        int depth;

        BoundingBoxWithDepth(BoundingBox<ndim> box, int depth)
            : box(box), depth(depth) {}
    };

    std::vector<BoundingBoxWithDepth> getBoundingBoxes_All() {
        std::vector<BoundingBoxWithDepth> boxes;
        insertBoundingBoxes_All(0, length - 1, 0, getBoundingBox(), boxes);
        return boxes;
    }

    std::vector<BoundingBoxWithDepth> getBoundingBoxes_Radius(Point &origin, float radius) {
        std::vector<BoundingBoxWithDepth> boxes;
        InRadiusCollector collector(origin, radius);
        insertBoundingBoxes_Radius(0, length - 1, 0, collector, getBoundingBox(), boxes);
        return boxes;
    }

    BoundingBox<ndim> getBoundingBox() {
        return findBoundingBox<Point, ndim>(points, length);
    }


private:

    /* Sort Point Array
    *****************************************************/

    void sort(int left, int right, int depth) {
        if (isLeaf(left, right)) return;

        int axis = getAxis(depth);
        int medianIndex = fixateMedian(left, right, axis);

        if (depth < threadDepth) {
            std::thread thread(&KDTree::sort, this, left, medianIndex - 1, depth + 1);
            sort(medianIndex + 1, right, depth + 1);
            thread.join();
        } else {
            sort(left, medianIndex - 1, depth + 1);
            sort(medianIndex + 1, right, depth + 1);
        }
    }

    int fixateMedian(int left, int right, int axis) {
        int medianIndex = getMedianIndex(left, right);
        quickselect_Iterative(left, right, medianIndex, axis);
        return medianIndex;
    }


    void quickselect_Iterative(int left, int right, int k, int axis) {
        while (left < right) {
            int pivotIndex = selectPivotIndex(left, right);
            int split = partition(left, right, axis, pivotIndex);
            if (k < split) {
                right = split - 1;
            } else if (k > split) {
                left = split + 1;
            } else {
                return;
            }
        }
    }

    int partition(int left, int right, int axis, int pivotIndex) {
        float pivotValue = getValue(pivotIndex, axis);

        // move pivot to the end
        swap(pivotIndex, right);

        // swap values that are smaller than the pivot to the front
        int index = left;
        for (int i = left; i < right; i++) {
            if (getValue(i, axis) < pivotValue) {
                swap(index, i);
                index++;
            }
        }

        // move pivot to correct position
        swap(right, index);

        // try to move the split index closer to the median if possible
        // this is important when there are many points on an axis aligned line
        int medianIndex = getMedianIndex(left, right);
        while (index < medianIndex && getValue(index + 1, axis) == pivotValue) {
            index++;
        }

        return index;
    }

    inline int selectPivotIndex(int left, int right) {
        return left + randomInt_Positive(left ^ right) % (right - left);
    }



    /* Collect in Radius
    *********************************************/

    struct InRadiusCollector {
        Point origin;
        float radius;
        std::vector<Point> points;

        InRadiusCollector(Point origin, float radius) {
            this->origin = origin;
            this->radius = radius;
        }

        void consider(Point &point) {
            if (CalcDistance(origin, point) <= radius) {
                points.push_back(point);
            }
        }

        std::vector<Point> getPoints() {
            return points;
        }
    };

    void collectInRadius(int left, int right, int depth, InRadiusCollector &collector) {
        if (isLeaf(left, right)) {
            considerPointsInBucket(left, right, collector);
            return;
        }

        int axis = getAxis(depth);
        int medianIndex = getMedianIndex(left, right);

        Point &splitPoint = points[medianIndex];
        collector.consider(splitPoint);

        float splitPos = splitPoint[axis];
        float originPos = collector.origin[axis];

        if (originPos - collector.radius <= splitPos) {
            collectInRadius(left, medianIndex - 1, depth + 1, collector);
        }
        if (originPos + collector.radius >= splitPos) {
            collectInRadius(medianIndex + 1, right, depth + 1, collector);
        }
    }



    /* Collect k Nearest
    *************************************************/

    struct PointWithDistance {
        Point point;
        float distance;

        PointWithDistance(Point point, float distance)
            : point(point), distance(distance) {}

        friend bool operator<(const PointWithDistance &p1, const PointWithDistance &p2) {
            return p1.distance < p2.distance;
        }
    };

    struct KNearestCollector {
        unsigned int k;
        Point origin;
        float maxDistance;
        std::priority_queue<PointWithDistance> queue;

        KNearestCollector(Point origin, int k) {
            assert(k >= 0);
            this->k = k;
            this->origin = origin;
            this->maxDistance = -1;
        }

        void consider(Point &point) {
            float distance = CalcDistance(origin, point);
            if (queue.size() < k) {
                queue.push(PointWithDistance(point, distance));
                maxDistance = getCurrentMaxDistance();
            } else if (distance < maxDistance) {
                queue.pop();
                queue.push(PointWithDistance(point, distance));
                maxDistance = getCurrentMaxDistance();
            }
        }

        float getCurrentMaxDistance() {
            return queue.top().distance;
        }

        std::vector<Point> getPoints() {
            std::vector<Point> points;
            while (!queue.empty()) {
                points.push_back(queue.top().point);
                queue.pop();
            }
            return points;
        }
    };

    void collectKNearest(int left, int right, int depth, KNearestCollector &collector) {
        if (isLeaf(left, right)) {
            considerPointsInBucket(left, right, collector);
            return;
        }

        int axis = getAxis(depth);
        int medianIndex = getMedianIndex(left, right);

        Point &splitPoint = points[medianIndex];

        float splitPos = splitPoint[axis];
        float originPos = collector.origin[axis];

        if (originPos <= splitPos) {
            collectKNearest(left, medianIndex - 1, depth + 1, collector);
            if (originPos + collector.maxDistance >= splitPos) {
                collectKNearest(medianIndex + 1, right, depth + 1, collector);
            }
        } else {
            collectKNearest(medianIndex + 1, right, depth + 1, collector);
            if (originPos - collector.maxDistance <= splitPos) {
                collectKNearest(left, medianIndex - 1, depth + 1, collector);
            }
        }

        collector.consider(splitPoint);
    }


    /* Simulate Collect and store Bounding Boxes
    ************************************************/

    void insertBoundingBoxes_All(int left, int right, int depth, BoundingBox<ndim> outerBox, std::vector<BoundingBoxWithDepth> &boxes) {
        boxes.push_back(BoundingBoxWithDepth(outerBox, depth));
        if (isLeaf(left, right)) return;

        int axis = getAxis(depth);
        int medianIndex = getMedianIndex(left, right);
        float splitPos = getValue(medianIndex, axis);

        BoundingBox<ndim> leftBox = outerBox;
        leftBox.max[axis] = splitPos;
        insertBoundingBoxes_All(left, medianIndex - 1, depth + 1, leftBox, boxes);

        BoundingBox<ndim> rightBox = outerBox;
        rightBox.min[axis] = splitPos;
        insertBoundingBoxes_All(medianIndex + 1, right, depth + 1, rightBox, boxes);
    }

    void insertBoundingBoxes_Radius(int left, int right, int depth, InRadiusCollector &collector, BoundingBox<ndim> outerBox, std::vector<BoundingBoxWithDepth> &boxes) {
        boxes.push_back(BoundingBoxWithDepth(outerBox, depth));
        if (isLeaf(left, right)) return;

        int axis = getAxis(depth);
        int medianIndex = getMedianIndex(left, right);

        Point &splitPoint = points[medianIndex];

        float splitPos = splitPoint[axis];
        float originPos = collector.origin[axis];

        if (originPos - collector.radius <= splitPos) {
            BoundingBox<ndim> box = outerBox;
            box.max[axis] = splitPos;
            insertBoundingBoxes_Radius(left, medianIndex - 1, depth + 1, collector, box, boxes);
        }
        if (originPos + collector.radius >= splitPos) {
            BoundingBox<ndim> box = outerBox;
            box.min[axis] = splitPos;
            insertBoundingBoxes_Radius(medianIndex + 1, right, depth + 1, collector, box, boxes);
        }
    }


    /* Utils
    *************************************************/

    template<class Collector>
    void considerPointsInBucket(int left, int right, Collector &collector) {
        for (int i = left; i <= right; i++) {
            collector.consider(points[i]);
        }
    }

    template<class Collector>
    std::vector<Point> collectNaive(Collector collector) {
        for (int i = 0; i < length; i++) {
            collector.consider(points[i]);
        }
        return collector.getPoints();
    }

    inline bool isLeaf(int left, int right) {
        return left + bucketSize > right;
    }

    inline int getAxis(int depth) {
        return depth % ndim;
    }

    inline void swap(int a, int b) {
        Point tmp = points[a];
        points[a] = points[b];
        points[b] = tmp;
    }

    inline float getValue(int index, int axis) {
        return points[index][axis];
    }

};


inline float getVec3DistanceSquared(glm::vec3 &a, glm::vec3 &b) {
    float xDiff = a.x - b.x;
    float yDiff = a.y - b.y;
    float zDiff = a.z - b.z;
    return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
}

inline float getVec3Distance(glm::vec3 &a, glm::vec3 &b) {
    return sqrt(getVec3DistanceSquared(a, b));
}

using KDTreeVec3 = KDTree<glm::vec3, 3, getVec3Distance>;