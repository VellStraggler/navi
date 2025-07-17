#include <vector>
#include <cmath>
#include "point.h"
#include "geomlineref.cpp"
#include "myath.h"

class Flit {
public:
    constexpr static float BASE_SPEED = 3;
    constexpr static float BASE_TURNING_SPEED = .01;
    constexpr static float BASE_OSCILLATION_MULT = 1;

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
    void setX(float x) {
        globalPos.x = x;
    }
    float getY() const {
        return globalPos.y;
    }
    void setY(float y) {
        globalPos.y = y;
    }
    float getBoxRadius() {
        return boxRadius;
    }
    float getRadius() const {
        return radius;
    }
    float setRadius(float r) {
        radius = r;
        boxRadius = std::sqrt((radius*radius)/2);
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
        // healthy range of 0 to 200
        float midX = 240;
        float midY = 360;
        float distFromCenter = getDistance(getX(), getY(), midX, midY);
        float pull = getMin(1, distFromCenter/5000);
        if (distFromCenter > 600) {
            pull = 1;
        }
        float dirToCenter = atan2(midY - getY(), midX - getX());
        float newDirX = (1 - pull) * cos(direction) + pull * cos(dirToCenter);
        float newDirY = (1 - pull) * sin(direction) + pull * sin(dirToCenter);
        direction = atan2(newDirY, newDirX);
        // globalPos.x += cos(dirToCenter) * pull;
        // globalPos.y += cos(dirToCenter) * pull;


        if (globalPos.y < 0 + radius) {
            globalPos.y += 5;
            // addDir(-.01);
        } else if (globalPos.y > 480 - radius) {
            globalPos.y -= 5;
            // addDir(-.01);
        }
        if (globalPos.x > 720 - radius) {
            globalPos.x -= 5;
            // addDir(-.01);
        } else if (globalPos.x < 0 + radius) {
            globalPos.x += 5;
            // addDir(-.01);
        }
        for (Flit f : obstacles) {
            if(getDistance(globalPos.x, globalPos.y, f.getX(), f.getY()) < radius) {
                hit = true;
                break;
            }
        }

        if (hit) {
            turnLeft = !turnLeft;
            addDir(PI_2 * sign);
            explode = true;
        }
        // for (GeomLineRef o : obstacles) {
        //     float ox = o.getX();
        //     float oy = o.getY();
        //     float px = globalPos.x;
        //     float py = globalPos.y;

        //     // Vector from FLiT to obstacle
        //     float dx = ox - px;
        //     float dy = oy - py;

        //     float angleToObstacle = std::atan2(dy, dx);
        //     float angleDiff = std::atan2(std::sin(angleToObstacle - direction),
        //                                 std::cos(angleToObstacle - direction));

        //     // Only respond if FLiT is facing roughly toward obstacle (< 90 degrees off)
        //     if (std::abs(angleDiff) > DEGREE) continue;

        //     float margin = 100;
        //     bool near = false;

        //     if (o.vertical) {
        //         float oy1 = oy;
        //         float oy2 = oy + o.length;
        //         if (oy1 > oy2) std::swap(oy1, oy2);

        //         if (std::abs(px - ox) < margin && py >= oy1 && py <= oy2) {
        //             near = true;
        //         }
        //     } else {
        //         float ox1 = ox;
        //         float ox2 = ox + o.length;
        //         if (ox1 > ox2) std::swap(ox1, ox2);

        //         if (std::abs(py - oy) < margin && px >= ox1 && px <= ox2) {
        //             near = true;
        //         }
        //     }

        //     if (near) {
        //         float left = direction + DEGREE;
        //         float right = direction - DEGREE;

        //         float testDist = 5.0f;
        //         float lx = px + std::cos(left) * testDist;
        //         float ly = py + std::sin(left) * testDist;
        //         float rx = px + std::cos(right) * testDist;
        //         float ry = py + std::sin(right) * testDist;

        //         float distL, distR;

        //         if (o.vertical) {
        //             distL = std::abs(lx - ox);
        //             distR = std::abs(rx - ox);
        //         } else {
        //             distL = std::abs(ly - oy);
        //             distR = std::abs(ry - oy);
        //         }

        //         if (distL > distR) {
        //             addDir(DEGREE);
        //         } else {
        //             addDir(-DEGREE);
        //         }

        //         // break; // only one avoidance per frame
        //     }
        // }

    }
    //volume affects radius, turning, and speed
    void implementVolume(float vol) {
        setRadius(vol);
        speed = BASE_SPEED * (1 + vol + vol);
        turningSpeed = BASE_TURNING_SPEED * (1 - (vol*3));
        oscillation_mult = BASE_OSCILLATION_MULT * pow(1 + vol, 5);
    } 
    void nextPos() {
        // Later feature: bring all flits to center
        // requires numbering them so they line up
        avoidObstacles();
        
        // apply oscillation
        float dxO = oscOffset * std::cos(direction + (PI_2)) * scale;
        float dyO = oscOffset * std::sin(direction + (PI_2)) * scale;
        
        globalPos.x += getDX() + dxO;
        globalPos.y += getDY() + dyO;
        
        // TURNING IS ALL WRONG ANYHOW
        // if(turnLeft) {
        //     addDir(turningSpeed);
        // } else {
        //     addDir(-turningSpeed);
        // }

        if (radius >= maxRadius) {
            maxRadius = radius;
        }
        if (maxRadius - radius < 14.7) {
            explode = true;
        }

        oscOffset = std::sin((timeAlive/30.0f) * (tempo / 60.f) * 2 * PI) * oscillation_mult;
        timeAlive += 1;        
    }

    float speed = BASE_SPEED; // was 4 for 60fps
    std::vector<Flit> obstacles = {};
    bool explode = false;
    bool gotLoud = false;
    
private:

    float tempo = 60; //by default
    float oscillation_mult = BASE_OSCILLATION_MULT;
    float maxRadius = 15;
    float turningSpeed = .01;
    float radius = 15;
    float boxRadius = std::sqrt((radius*radius)/2);
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