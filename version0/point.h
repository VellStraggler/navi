#ifndef POINT_H
#define POINT_H

class Point {
public:
    float x;
    float y;

    Point(float x, float y) : x(x), y(y) {}
    Point() : x(0), y(0) {}
};

#endif