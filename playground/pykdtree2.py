import random
import sys
from pprint import pprint
from math import sqrt
from random import randrange
import time

def timeit(method):
    def timed(*args, **kw):
        ts = time.clock()
        result = method(*args, **kw)
        te = time.clock()
        if 'log_time' in kw:
            name = kw.get('log_name', method.__name__.upper())
            kw['log_time'][name] = int((te - ts) * 1000)
        else:
            print('%r  %2.5f ms' % \
                  (method.__name__, (te - ts) * 1000))
        return result
    return timed

class KDTree:
    def __init__(self, points, dimensions, bucketSize):
        self.points = points
        self.dimensions = dimensions
        self.bucketSize = bucketSize

        self.sortPoints()

    @timeit
    def sortPoints(self):
        def recursiveSort(left, right, depth):
            if left + self.bucketSize >= right:
                return

            key = self.getAxisGetter(depth)
            medianIndex = getMedianIndex(left, right)
            quickselect(self.points, left, right, medianIndex, key)

            recursiveSort(left, medianIndex - 1, depth + 1)
            recursiveSort(medianIndex + 1, right, depth + 1)

        recursiveSort(0, len(self.points) - 1, 0)

    @timeit
    def collectInRadius(self, origin, radius):
        def recursiveSearch(left, right, depth):
            if left + self.bucketSize >= right:
                for i in range(left, right + 1):
                    helper.consider(self.points[i])
                return

            originCoordinate = self.getActivePointCoordinate(origin, depth)
            medianIndex = getMedianIndex(left, right)
            medianPoint = self.points[medianIndex]
            split = self.getActivePointCoordinate(medianPoint, depth)
            helper.consider(medianPoint)
            if originCoordinate - radius <= split:
                recursiveSearch(left, medianIndex - 1, depth + 1)
            if originCoordinate + radius >= split:
                recursiveSearch(medianIndex + 1, right, depth + 1)

        helper = CollectInRadiusHelper(origin, radius)
        recursiveSearch(0, len(self.points) - 1, 0)
        return helper.points

    def getAxisGetter(self, depth):
        axis = self.getAxis(depth)
        return lambda x: x[axis]

    def getActivePointCoordinate(self, point, depth):
        return point[self.getAxis(depth)]

    def getAxis(self, depth):
        return depth % self.dimensions

class CollectInRadiusHelper:
    def __init__(self, origin, radius):
        self.origin = origin
        self.radius = radius
        self.points = []
        self.consideredAmount = 0

    def consider(self, point):
        self.consideredAmount += 1
        if distance(point, self.origin) <= self.radius:
            self.points.append(point)

def getMedianIndex(left, right):
    return (left + right) // 2

def quickselect(array, left, right, position, key):
    if left == right:
        return

    split = partition(array, left, right, key)
    if position < split:
        quickselect(array, left, split - 1, position, key)
    elif position > split:
        quickselect(array, split + 1, right, position, key)
    else:
        return

def partition(array, left, right, key):
    pivotIndex = selectPivotIndex(array, left, right)
    pivotValue = array[pivotIndex]

    swap(array, pivotIndex, right)
    storeIndex = left
    for i in range(left, right):
        if key(array[i]) < key(pivotValue):
            swap(array, storeIndex, i)
            storeIndex += 1

    swap(array, right, storeIndex)

    # improve split index when there are many similar values
    medianIndex = getMedianIndex(left, right)
    while storeIndex < medianIndex and key(array[storeIndex+1]) == key(pivotValue):
        storeIndex += 1

    return storeIndex

def swap(array, i1, i2):
    array[i1], array[i2] = array[i2], array[i1]

def selectPivotIndex(array, start, end):
    return randrange(start, end)

def distance(a, b):
    return sqrt(sum((ai - bi) ** 2 for ai, bi in zip(a, b)))


def createRandomTuples(amount, dimensions):
    return [tuple(random.uniform(-10, 10) for _ in range(dimensions)) for _ in range(amount)]
    # return [tuple(randrange(-10, 10) for _ in range(dimensions)) for _ in range(amount)]

@timeit
def collectInRadius_Naive(points, origin, radius):
    found = []
    for point in points:
        if distance(point, origin) <= radius:
            found.append(point)
    return found

random.seed(100)
dimensions = 3
amount = 1000000
origin = (0, 0)
radius = 1

points = createRandomTuples(amount, dimensions)
tree = KDTree(points, dimensions, bucketSize = 5)

for _ in range(10):
    origin = createRandomTuples(1, dimensions)[0]
    inRadiusFast = tree.collectInRadius(origin, radius)
    #inRadiusSlow = collectInRadius_Naive(points, origin, radius)
    #print(len(inRadiusFast), len(inRadiusSlow))
    #print("Collect in Radius is correct?", set(inRadiusFast) == set(inRadiusSlow))