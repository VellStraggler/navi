#include <vector>
#include <cmath>
#include "point.h"
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
        float midX = WIDTH/2;
        float midY = HEIGHT/2;
        float pull = .4;
        float dirToCenter = atan2(midY - getY(), midX - getX());
        float newDirX = (1 - pull) * cos(direction) + pull * cos(dirToCenter);
        float newDirY = (1 - pull) * sin(direction) + pull * sin(dirToCenter);
        // gently bring flit towards center. Promotes synchronization
        globalPos.x += cos(dirToCenter) * pull;
        globalPos.y += cos(dirToCenter) * pull;


        if (globalPos.y < 0) {
            globalPos.y += 5;
            direction = atan2(newDirY, newDirX);
        } else if (globalPos.y > HEIGHT) {
            globalPos.y -= 5;
            direction = atan2(newDirY, newDirX);
        }
        if (globalPos.x > WIDTH) {
            globalPos.x -= 5;
            direction = atan2(newDirY, newDirX);
        } else if (globalPos.x < 0) {
            globalPos.x += 5;
            direction = atan2(newDirY, newDirX);
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

    }
    //volume affects radius, turning, and speed
    void implementVolume(float vol) {
        setRadius(vol*30);
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

        if (radius >= maxRadius) {
            maxRadius = radius;
        }
        if (maxRadius - radius < 0.3) {
            explode = true;
        }

        oscOffset = std::sin((timeAlive/30.0f) * (tempo / 60.f) * 2 * PI) * oscillation_mult;
        timeAlive += 1;        
    }

    float speed = BASE_SPEED; // was 4 for 60fps
    std::vector<Flit> obstacles = {};
    bool explode = false;
    
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
    std::vector<uint8_t> color = {255, 255, 255};
};