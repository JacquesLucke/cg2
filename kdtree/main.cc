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

/* Timer
***************************************************/

struct Timer {
    const char *name;
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::duration<float> duration;

    Timer(const char *name = "") {
        this->name = name;
        this->start = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        double ms = duration.count() * 1000.0f;
        std::cout << "Timer '" << name << "' took " << ms << " ms" << std::endl;
    }
};

#define TIMEIT Timer t(__FUNCTION__);


/* Vector
***************************************************/

template<int N>
struct Vector {
    std::array<float, N> data;

public:

    friend std::ostream& operator<<(std::ostream& stream, const Vector& p) {
        stream << "(";
        for (size_t i = 0; i < N; i++) {
            stream << p.data[i];
            if (i < N - 1) stream << ", ";
        }
        stream << ")";
        return stream;
    }

    float& operator[](const int i) {
        return this->data[i];
    }

    static float distanceSquared(Vector &a, Vector &b) {
        float sum = 0;
        for (int i = 0; i < N; i++) {
            float value = a.data[i] - b.data[i];
            sum += value * value;
        }
        return sum;
    }

    static float distance(Vector &a, Vector &b) {
        return sqrt(distanceSquared(a, b));
    }
};

template<int N>
std::vector<Vector<N>> generateRandomVectors(int amount, int seed = 0) {
    std::vector<Vector<N>> points;
    for (int i = 0; i < amount; i++) {
        auto point = Vector<N>();
        for (int j = 0; j < N; j++) {
            point[j] = randomFloat_Range(seed, 10);
            seed += 5233;
        }
        points.push_back(point);
    }

    return points;
}

inline int randomInt(int x) {
    x = (x<<13) ^ x;
    return x * (x * x * 15731 + 789221) + 1376312589;
}

inline int randomInt_Positive(int x) {
    return randomInt(x) & 0x7fffffff;
}

inline float randomFloat_Range(int x, float scale) {
    return randomInt(x) / 2147483648.0 * scale;
}

template<int N>
void printVectors(Vector<N> *vectors, int length) {
    for (int i = 0; i < length; i++) {
        std::cout << vectors[i] << std::endl;
    }
}

template<int N>
void printVectors(std::vector<Vector<N>> &vectors) {
    printVectors(vectors.data(), vectors.size());
}

template<int N>
float getIndex(Vector<N> &point, int axis) {
    return point[axis];
}



/* KDTree
******************************************************/

template<int N>
struct BoundingBox {
    std::array<float, N> min;
    std::array<float, N> max;

    friend std::ostream& operator<<(std::ostream& stream, const BoundingBox& box) {
        stream << "[";
        for (size_t i = 0; i < N; i++) {
            stream << "(" << box.min[i] << ", " << box.max[i] << ")";
            if (i < N - 1) stream << ", ";
        }
        stream << "]";
        return stream;
    }
};

template<
    class Point,
    int ndim,
    float (*GetKey)(Point &p, int axis),
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
        this->threadDepth = std::log2(getCpuCoreCount()) + 1;
    }

    void balance() {
        TIMEIT
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

    std::vector<BoundingBoxWithDepth> getBoundingBoxes(BoundingBox<ndim> outerBox) {
        TIMEIT
        std::vector<BoundingBoxWithDepth> boxes;
        insertBoundingBoxes(0, length - 1, 0, outerBox, boxes);
        return boxes;
    }

    void insertBoundingBoxes(int left, int right, int depth, BoundingBox<ndim> outerBox, std::vector<BoundingBoxWithDepth> &boxes) {
        if (isLeaf(left, right)) return;

        int axis = getAxis(depth);
        int medianIndex = getMedianIndex(left, right);
        float splitPos = getValue(medianIndex, axis);

        boxes.push_back(BoundingBoxWithDepth(outerBox, depth));

        BoundingBox<ndim> leftBox = outerBox;
        leftBox.max[axis] = splitPos;
        insertBoundingBoxes(left, medianIndex - 1, depth + 1, leftBox, boxes);

        BoundingBox<ndim> rightBox = outerBox;
        rightBox.min[axis] = splitPos;
        insertBoundingBoxes(medianIndex + 1, right, depth + 1, rightBox, boxes);
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

    void quickselect_Recursive(int left, int right, int k, int axis) {
        if (left >= right) return;

        int pivotIndex = selectPivotIndex(left, right);
        int split = partition(left, right, axis, pivotIndex);
        if (k < split) {
            quickselect_Recursive(left, split - 1, k, axis);
        } else if (k > split) {
            quickselect_Recursive(split + 1, right, k, axis);
        }
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

    void quicksort(int left, int right, int axis) {
        if (left >= right) return;
        int split = partition(left, right, axis, selectPivotIndex_Small(left, right));
        quicksort(left, split - 1, axis);
        quicksort(split + 1, right, axis);
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

    int partition_WithoutMedian(int left, int right, int axis, float pivot) {
        int pivotIndex = -1;

        int index = right;
        int i = left;
        while (i < index) {
            float value = getValue(i, axis);
            if (value > pivot) {
                swap(i, index);
                index--;
            } else if (value < pivot) {
                i++;
            } else {
                pivotIndex = i;
                i++;
            }
        }

        if (pivotIndex >= 0) {
            if (getValue(index, axis) < pivot) {
                swap(pivotIndex, index);
            } else {
                swap(pivotIndex, index - 1);
                index--;
            }
        }

        return index;
    }

    struct ValueWithIndex {
        float value;
        int index;
        ValueWithIndex() {}
        ValueWithIndex(float value, int index) : value(value), index(index) {}

        friend bool operator<(const ValueWithIndex &v1, const ValueWithIndex &v2) {
            return v1.value < v2.value;
        }
    };

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

        float splitPos = GetKey(splitPoint, axis);
        float originPos = GetKey(collector.origin, axis);

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
        int k;
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

        float splitPos = GetKey(splitPoint, axis);
        float originPos = GetKey(collector.origin, axis);

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
        return GetKey(points[index], axis);
    }

};

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

int getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

template<int N>
BoundingBox<N> findBoundingBox(Vector<N> *points, int length) {
    TIMEIT
    BoundingBox<N> box;
    for (int i = 0; i < N; i++) {
        box.min[i] = +std::numeric_limits<float>::infinity();
        box.max[i] = -std::numeric_limits<float>::infinity();
    }

    for (int i = 0; i < length; i++) {
        for (int j = 0; j < N; j++) {
            float value = points[i][j];
            if (value < box.min[j]) box.min[j] = value;
            if (value > box.max[j]) box.max[j] = value;
        }
    }

    return box;
}


/* Main
*******************************************/

#define NDIM 3

template<int N>
using VectorKDTree = KDTree<Vector<N>, N, getIndex<N>, Vector<N>::distance>;

void findPointsInRadius(VectorKDTree<NDIM> &tree, std::vector<Vector<NDIM>> &points) {
    TIMEIT
    for (int i = 0; i < 100000; i++) {
        tree.collectInRadius(points[i], 0.1);
        // std::cout << tree.collectInRadius(points[i], 1).size() << std::endl;
    }
}

void findKNearestPoints(VectorKDTree<NDIM> &tree, std::vector<Vector<NDIM>> &points) {
    TIMEIT
    for (int i = 0; i < 100; i++) {
        tree.collectKNearest(points[i], 1000);
        // std::cout << tree.collectKNearest(points[i], 5).size() << std::endl;
    }
}

struct OffFileData {
    std::vector<Vector<3>> vertices;
};

OffFileData *readOffFile(std::string path) {
    TIMEIT
    OffFileData *data = new OffFileData();

    std::ifstream fs(path);

    std::string line;
    getline(fs, line);
    if (line != "OFF") {
        std::cerr << "file does not start with 'OFF" << std::endl;
        return nullptr;
    }

    getline(fs, line);
    int vertexAmount = std::stoi(line);
    for (int i = 0; i < vertexAmount; i++) {
        getline(fs, line);
        int split1 = line.find(" ");
        int split2 = line.find(" ", split1 + 1);

        Vector<3> point;
        point[0] = std::stof(line.c_str() + 0);
        point[1] = std::stof(line.c_str() + split1);
        point[2] = std::stof(line.c_str() + split2);
        data->vertices.push_back(point);
    }

    fs.close();

    return data;
}

void saveBoundingBoxesWithDepth(std::string path, std::vector<VectorKDTree<NDIM>::BoundingBoxWithDepth> &boxes) {
    TIMEIT
    std::ofstream fs(path);
    for (int i = 0; i < boxes.size(); i++) {
        for (int j = 0; j < NDIM; j++) {
            fs << boxes[i].box.min[j] << " " << boxes[i].box.max[j] << std::endl;
        }
        fs << boxes[i].depth << std::endl;
    }
    fs.close();
}


int main(int arc, char const *argv[]) {
    OffFileData *input = readOffFile("off_files\\dragon.off");
    if (input == nullptr) {
        std::cout << "Could not read file." << std::endl;
        return 1;
    }

    std::cout << "Vertex Amount: " << input->vertices.size() << std::endl;

    //auto points = generateRandomVectors<NDIM>(100'000, 42);

    auto points = input->vertices;
    BoundingBox<NDIM> bounds = findBoundingBox(points.data(), points.size());
    VectorKDTree<NDIM> tree(points.data(), points.size(), 10);
    tree.balance();

    auto boxes = tree.getBoundingBoxes(bounds);
    saveBoundingBoxesWithDepth("boxes.txt", boxes);

    //printVectors(points);
    //findPointsInRadius(tree, *points);
    //findKNearestPoints(tree, *points);
    //auto result = tree.collectInRadius((*points)[435345], 0.15);
    //printVectors(&result);
    std::cout << "Done." << std::endl;
    return 0;
}