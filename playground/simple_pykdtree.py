import time
import random
import functools
from math import sqrt
from pprint import pprint

def measureTime(function):
    @functools.wraps(function)
    def wrapper(*args, **kwargs):
        start = time.clock()
        output = function(*args, **kwargs)
        end = time.clock()
        duration = end - start
        print("Time: {:.5f} - fps : {:.2f} - Function: {}".format(duration, 1 / max(duration, 1e-10), function.__name__))
        return output
    return wrapper

class Vector:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    @classmethod
    def distance(self, a, b):
        return sqrt((a.x-b.x)**2 + (a.y-b.y)**2 + (a.z-b.z)**2)

    def __repr__(self):
        return f"V({self.x:10f}, {self.y:10f}, {self.z:10f})"

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y and self.z == other.z

    def __hash__(self):
        return hash((self.x, self.y, self.z))

class ClosestNPointsList:
    def __init__(self, center, maxLength):
        self.maxLength = maxLength
        self.pointsWithDistance = []
        self.maxDistance = -1
        self.maxIndex = -1
        self.center = center

    def consider(self, point):
        distance = Vector.distance(self.center, point)
        if len(self.pointsWithDistance) < self.maxLength:
            self.pointsWithDistance.append((point, distance))
            if distance > self.maxDistance:
                self.maxDistance = distance
                self.maxIndex = len(self.pointsWithDistance) - 1
        else:
            if distance < self.maxDistance:
                self.pointsWithDistance[self.maxIndex] = (point, distance)
                self.findMax()

    def findMax(self):
        index = -1
        maxDistance = -1
        for i, (_, distance) in enumerate(self.pointsWithDistance):
            if distance > maxDistance:
                index = i
                maxDistance = distance
        self.maxIndex = index
        self.maxDistance = maxDistance

    def getPoints(self):
        return [p for (p, _) in self.pointsWithDistance]

class KDTree:
    @measureTime
    def __init__(self, points):
        self.root = KDTreeNode(points, "x")

    @measureTime
    def findPointsInRadius(self, center, radius):
        return list(self.root.iterPointsInRadius(center, radius))

    @measureTime
    def findClosestNPoints(self, center, n):
        helper = ClosestNPointsList(center, n)
        self.root.findClosestNPoints(helper)
        return helper.getPoints()

    def __repr__(self):
        return str(self.root)

dimensions = ["x", "y", "z"]
stopSize = 10

class KDTreeNode:
    def __init__(self, points, dimension):
        sortedPoints = sortByDimension(points, dimension)
        leftPoints = sortedPoints[:len(points)//2]
        rightPoints = sortedPoints[len(points)//2:]
        self.dimension = dimension
        self.splitPosition = getattr(rightPoints[0], dimension)

        if len(leftPoints) <= stopSize:
            self.left = KDTreeLeaf(leftPoints)
        else:
            self.left = KDTreeNode(leftPoints, nextDimension(dimension))

        if len(rightPoints) <= stopSize:
            self.right = KDTreeLeaf(rightPoints)
        else:
            self.right = KDTreeNode(rightPoints, nextDimension(dimension))

    def iterPointsInRadius(self, center, radius):
        position = getattr(center, self.dimension)

        if position - radius <= self.splitPosition:
            yield from self.left.iterPointsInRadius(center, radius)
        if position + radius >= self.splitPosition:
            yield from self.right.iterPointsInRadius(center, radius)

    def findClosestNPoints(self, helper):
        position = getattr(helper.center, self.dimension)
        if position < self.splitPosition:
            self.left.findClosestNPoints(helper)
            if position + helper.maxDistance >= self.splitPosition:
                self.right.findClosestNPoints(helper)
        else:
            self.right.findClosestNPoints(helper)
            if position - helper.maxDistance <= self.splitPosition:
                self.left.findClosestNPoints(helper)

    def __repr__(self):
        return "\n".join(self.iterReprLines())

    def iterReprLines(self):
        yield f"Node '{self.dimension}' at {self.splitPosition:5f}"
        yield " Left:"
        yield from ("    " + line for line in self.left.iterReprLines())
        yield " Right:"
        yield from ("    " + line for line in self.right.iterReprLines())

class KDTreeLeaf:
    def __init__(self, points):
        self.points = points

    def iterPointsInRadius(self, center, radius):
        for point in self.points:
            if Vector.distance(point, center) <= radius:
                yield point

    def findClosestNPoints(self, helper):
        for point in self.points:
            helper.consider(point)

    def iterReprLines(self):
        for point in self.points:
            yield str(point)


def sortByDimension(points, dimension):
    return list(sorted(points, key = lambda v: getattr(v, dimension)))

def nextDimension(dimension):
    return dimensions[(dimensions.index(dimension) + 1) % len(dimensions)]

def getDimensionMin(points, dimension):
    return min(getattr(p, dimension) for p in points)

def getDimensionMax(points, dimension):
    return max(getattr(p, dimension) for p in points)

def createPoints(n, seed):
    points = []
    random.seed(seed)
    for _ in range(n):
        points.append(Vector(random.random(), random.random(), random.random()))
    return points

@measureTime
def naiveSearchInRadius(points, center, radius):
    nearPoints = []
    for point in points:
        if Vector.distance(point, center) <= radius:
            nearPoints.append(point)
    return nearPoints

@measureTime
def naiveSearchClosestN(points, center, n):
    sortedPoints = list(sorted(points, key = lambda p: Vector.distance(p, center)))
    return sortedPoints[:n]

def naiveSearchClosestN2(points, center, n):
    helper = ClosestNPointsList(center, n)
    for point in points:
        helper.consider(point)
    return helper.getPoints()



print("start create points")
points = createPoints(100000, 0)
print("start build tree")
tree = KDTree(points)

center = Vector(0.7, 0.5, 0.2)
radius = 0.1
amount = 100

print("start find naive")
nearPointsNaive = naiveSearchInRadius(points, center, radius)
print(f"found {len(nearPointsNaive)} points")

print("start find fast")
nearPointsFast = tree.findPointsInRadius(center, radius)
print(f"found {len(nearPointsFast)} points")

print("start find n naive")
closestPointsNaive = naiveSearchClosestN(points, center, amount)

print("start find n fast")
closestPointsFast = tree.findClosestNPoints(center, amount)


print(set(closestPointsNaive) == set(closestPointsFast))

print("done")

