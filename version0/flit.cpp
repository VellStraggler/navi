#include <vector>
#include <cmath>

constexpr float PI_2 = 3.14159265f / 2.0f;

class Flit {
public:
    Flit(float x, float y) {
        globalPos = {x,y};
        orbitCenter = {x,y};
    }
    std::vector<float> getGlobalPos() const {
        return globalPos;
    }
    float getGlobalPosX() const {
        return globalPos[0];
    }
    float getGlobalPosY() const {
        return globalPos[1];
    }
    std::vector<float> getOrbitCenter() const {
        return orbitCenter;
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
    void addDir(float change) {
        direction += change;
    }
    float getSpeed() {
        return speed;
    }
    float radius = 15;
    float speed = 150;
    void nextPos() {
        float dx = speed * std::cos(direction - PI_2) * scale;
        float dy = speed * std::sin(direction - PI_2) * scale;
        // apply oscillation
        // float dxO = oscOffset * std::cos(dir - PI_2) * scale * 35;
        // float dyO = oscOffset * std::sin(dir - PI_2) * scale * 35;

        // orbitCenter = {orbitCenter[0] + dx, orbitCenter[1] + dy};
        globalPos = {orbitCenter[0] + dx, orbitCenter[1] + dy};

        addDir(.02);

    }
private:
    // This is the point the flit oscillates around
    std::vector<float> orbitCenter = {0,0};
    // This is where its drawn
    std::vector<float> globalPos = {0,0};
    float oscOffset = 0;
    // scale is affected by resolution
    float scale = 1.0;
    // in radians
    float direction = 0;
    // float direction = 150;
    // float radius = 15;
    float acc = 1;
    std::vector<int> color = {255, 255, 255};
};