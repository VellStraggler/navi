#include <cmath>
#include <vector>

class Particle {
public:
    static constexpr float PI_2 = 3.14159265f / 2.0f;
    static constexpr float DECEL = .999;
    static constexpr float SCALE = 1.0;
    float x;
    float y;
    float direction;
    float speed;
    float color[3];
    float initColor[3];
    int lifeCycle;
    Particle(float x, float y, float speed, float direction, float color[3]) {
        this->x = x;
        this->y = y;
        this->speed = speed - (std::rand()%1000)/10;
        this->direction = direction + ((std::rand() % 110) - 50)/(PI_2*40);
        for(int i=0; i <3; i++) {
            this->color[i] = color[i];
            this->initColor[i] = color[i];
        }
        lifeCycle = (std::rand() % 1000);
    }
    void update() {
        // update position based on speed and direction
        x += speed * std::cos(direction - PI_2) * SCALE / 1000;
        y += speed * std::sin(direction - PI_2) * SCALE / 1000;
        // y += ((std::rand() % 11) - 5)/5;
        // x += ((std::rand() % 11) - 5)/5;
        

        if(speed > 0) {
            speed *= DECEL;
            // speed -= .1;
        }

        for(int i = 0; i < 3; i ++) {
            color[i] -= initColor[i]/lifeCycle;
        }
    }
};