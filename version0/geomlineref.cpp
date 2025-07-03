#include "point.h"
/* A vertical or horizontal line starting from
 a top-left point. Created for collision detection.
 I want this to store a reference to the coordinates of*/
 class GeomLineRef {
public:
    float* x;
    float* y;
    float length;
    bool vertical;
    GeomLineRef(Point& p, float length, bool vertical) 
        : x(&p.x), y(&p.y), length(length), vertical(vertical) {}
    
    float getX() const { return *x; }
    float getY() const { return *y; }
};