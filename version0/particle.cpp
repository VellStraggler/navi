#include <cmath>
#include <vector>
#include <sstream>
#include "myath.h"

class Particle {
public:
    static constexpr float DECEL = .997;
    static constexpr float SCALE = 1.0;
    static constexpr float MAX_INTENSITY = 1.0f;
    float x;
    float y;
    float direction;
    float speed;
    float intensity;
    int lifeCycle;
    int timeAlive;
    int radius = 2;

    /* A particle's lifespan is randomized, and it has a set level of deceleration.
    Suggested maxLifeCycle is 750 frames. */
    Particle(float x, float y, float speed, float generalDirection, float intensity, int maxLifeCycle) 
        : x(x), y(y), intensity(intensity) {
        this->speed = speed;
        // improves particle spread
        this->direction = randFloat(generalDirection-(PI_2), generalDirection + (PI-2));
        lifeCycle = randFloat(maxLifeCycle);
        timeAlive = 0;
    }
    std::string toString() {
        std::ostringstream ss;
        ss << "(" << x << ", " << y << ") speed: " << speed << ", dir: " << toDegrees(direction) << "*, intensity: " << intensity;
        return ss.str();
    }
    void update(float volume) {
        // decrease color based on life cycle
        // black particles will be culled
        if (intensity < .5) {
            intensity -= MAX_INTENSITY/lifeCycle;
        }
        intensity -= MAX_INTENSITY/lifeCycle;
        if (speed < 0.1) {
            return;
        }
        // update position based on speed and direction
        x += speed * std::cos(direction) * SCALE;
        y += speed * std::sin(direction) * SCALE;

        // These will be implemented to affect the pixels more when
        // song gets more volatile
        if (volume > .2) {
            float volMult = volume * 2;
            y += randFloat(-volMult,volMult);
            x += randFloat(-volMult,volMult);
        }
        
        speed *= DECEL;
        timeAlive++;
    }
    void addX(float amt) {
        x += amt;
    }
    void addY(float amt) {
        y += amt;
    }
};