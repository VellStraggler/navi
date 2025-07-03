#include <vector>
#include <cmath>
#include "point.h"
#include "geomlineref.cpp"
#include "myath.h"

class Flit {
public:
    Flit(float x, float y) {
        globalPos = Point(x,y);
        spawnPoint = Point(x,y);
        turnLeft = false;
    }
    /* The FLiT's central, global position */
    Point getXY() const {
        return globalPos;
    }
    Point& getXYRef() {
        return globalPos;
    }
    float getX() const {
        return globalPos.x;
    }
    float getY() const {
        return globalPos.y;
    }
    float getRadius() const {
        return radius;
    }
    void updateOscOffset(float oscOffset) {
        this->oscOffset = oscOffset;
    }
    float getOscOffset() const {
        return oscOffset;
    }
    float getDir() const {
        return direction;
    }
    /* In radians. */
    void addDir(float change) {
        direction += change;
    }
    float getSpeed() {
        return speed;
    }
    float getDX() const {
        return speed * std::cos(direction) * scale;
    }
    float getDY() const {
        return speed * std::sin(direction) * scale;
    }
    void avoidObstacles() {
        int sign = -1;
        bool hit = false;
        if(turnLeft) {
            sign = 1;
        }
        if (globalPos.y < 10) {
            hit = true;
            globalPos.y += 5;
        } else if (globalPos.y > 470) {
            hit = true;
            globalPos.y -= 5;
        } else if (globalPos.x > 710) {
            hit = true;
            globalPos.x -= 5;
        } else if (globalPos.x < 10) {
            hit = true;
            globalPos.x += 5;
        }
        if (hit) {
            addDir(PI_2 * sign);
            explode = true;
        }
        for (GeomLineRef o : obstacles) {
            float ox = o.getX();
            float oy = o.getY();
            float px = globalPos.x;
            float py = globalPos.y;

            // Vector from FLiT to obstacle
            float dx = ox - px;
            float dy = oy - py;

            float angleToObstacle = std::atan2(dy, dx);
            float angleDiff = std::atan2(std::sin(angleToObstacle - direction),
                                        std::cos(angleToObstacle - direction));

            // Only respond if FLiT is facing roughly toward obstacle (< 90 degrees off)
            if (std::abs(angleDiff) > DEGREE) continue;

            float margin = 100;
            bool near = false;

            if (o.vertical) {
                float oy1 = oy;
                float oy2 = oy + o.length;
                if (oy1 > oy2) std::swap(oy1, oy2);

                if (std::abs(px - ox) < margin && py >= oy1 && py <= oy2) {
                    near = true;
                }
            } else {
                float ox1 = ox;
                float ox2 = ox + o.length;
                if (ox1 > ox2) std::swap(ox1, ox2);

                if (std::abs(py - oy) < margin && px >= ox1 && px <= ox2) {
                    near = true;
                }
            }

            if (near) {
                float left = direction + DEGREE;
                float right = direction - DEGREE;

                float testDist = 5.0f;
                float lx = px + std::cos(left) * testDist;
                float ly = py + std::sin(left) * testDist;
                float rx = px + std::cos(right) * testDist;
                float ry = py + std::sin(right) * testDist;

                float distL, distR;

                if (o.vertical) {
                    distL = std::abs(lx - ox);
                    distR = std::abs(rx - ox);
                } else {
                    distL = std::abs(ly - oy);
                    distR = std::abs(ry - oy);
                }

                if (distL > distR) {
                    addDir(DEGREE);
                } else {
                    addDir(-DEGREE);
                }

                // break; // only one avoidance per frame
            }
        }

    }
    void nextPos() {
        // Later feature: bring all flits to center
        // requires numbering them so they line up
        avoidObstacles();
        
        // apply oscillation
        float dxO = oscOffset * std::cos(direction + (PI_2)) * scale;
        float dyO = oscOffset * std::sin(direction + (PI_2)) * scale;
        
        // oscillation not currently applied
        globalPos.x += getDX() + dxO;
        globalPos.y += getDY() + dyO;
        
        if(turnLeft) {
            addDir(.005);
        } else {
            addDir(-.005);
        }
        oscOffset = std::sin(timeAlive/5.0f);
        timeAlive += 1;
        
    }
    
    float radius = 15;
    float boxRadius = std::sqrt((radius*radius)/2);
    float speed = 4;
    std::vector<GeomLineRef> obstacles = {};
    bool explode = false;
    
    private:
    bool turnLeft;
    // This is the point the flit oscillates around
    Point spawnPoint;
    // This is where its drawn
    Point globalPos;
    float oscOffset = 0;
    // scale is affected by resolution
    float scale = 1.0;
    // in radians
    float direction = 0;
    int timeAlive = 0;
    float acc = 1;
    std::vector<int> color = {255, 255, 255};
};