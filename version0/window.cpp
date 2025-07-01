// HOW TO RUN

// be in Navi Project, cmd: cmake --build build
// then cmd: ./build/MyGLFWApp.exe

/*
COPY AND RUN: OLD
g++ window.cpp C:/GLFW/build/src/libglfw3.a -lopengl32 -lgdi32 -o window.exe
*/
// edited things to include GLFW and take it out of this command ^ (original is shown below)
// g++ window.cpp -IC:/GLFW/include C:/GLFW/build/src/libglfw3.a -lopengl32 -lgdi32 -o window.exe

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include "flit.cpp"
#include "particle.cpp"
#include <cmath>

constexpr float PI = 3.14159265f;
constexpr float MAX_INTENSITY = 100.0f;
constexpr int HEIGHT = 480;
constexpr int WIDTH = 720;

std::vector<Flit> getFlits(int count) {
    std::vector<Flit> flits = {};
    float spacing = WIDTH / (float)(count+1);
    for(int i = 0; i < count; i++) {
        int x = spacing * (i + 1);
        Flit flit = Flit(x, (float)(HEIGHT)/2.0f);
        flits.push_back(flit);
    }
    return flits;
}

int64_t currentTimeMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

/* Contains all the methods and constants required for this */
class Window {
public:
    double x = 0;
    int frames = 0;
    int64_t t1 = currentTimeMs();
    int64_t second_out = currentTimeMs() + 1000;

    int particleCount = 0;
    
    /*Stores all pixels (y,x format) as an intensity number from 0f to 100.0f*/
    float pixels[HEIGHT][WIDTH] = {};
    
    Window() = default;
};
void drawCircle(float x, float y, float radius) {
    // get optimal segment count
    const int segments = std::ceil(std::pow(radius,0.5)*6);

    glBegin(GL_TRIANGLE_FAN); // Circle made of pizza slices
    glVertex2f(x,y);

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float dx = radius * std::cos(angle);
        float dy = radius * std::sin(angle);
        glVertex2f(x + dx, y + dy);
    }

    glEnd();
}
void drawFlitCircle(const Flit& flit) {
    // Set color to white
    glColor3f(1.0f,1.0f,1.0f); // WHITE
    const float x = flit.getGlobalPosX();
    const float y = flit.getGlobalPosY();
    const float radius = flit.radius;
}
/* Draws particles to w.pixels, NOT directly to the screen*/
void drawParticles(Window& w, std::vector<Particle>& ps) {
    // infrequently give size
    if (ps.size()%100 == 0) {
        if (ps.size() != w.particleCount) {
            std::cout << ps.size() << std::endl;
            w.particleCount = ps.size();
        }
    }
    for (int i = 0; i < ps.size(); i++) {
        // This creates a copy of a particle \/
        // Particle p = ps.at(i);
        // This uses a reference to it instead \/
        Particle& p = ps.at(i);

        int intx = (int)p.x;
        int inty = (int)p.y;
        // always pick the brighter intensity
        if (w.pixels[inty][intx] < ps.at(i).color[0] * 100) {
            w.pixels[inty][intx] = ps.at(i).color[0] * 100;
        }
    

        //update the pixel at the end for printing in the future
        p.update();
    }
}
/* 
    Draws all pixels as they are stored in w.pixels
    Converts linear brightness into a higher contrast version as well.
*/
void drawPixels(const Window& w) {
    glBegin(GL_POINTS);
    // Draw a pixel to represent the flit
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            float i = w.pixels[y][x];
            if (i > 0) {
                i = i * i * (i/5000);
                i = std::min(MAX_INTENSITY, i);
                float value = std::min(i / MAX_INTENSITY, 1.0f);
                glColor3f(value, value, value);
                glVertex2f(x,y);
            }
        }
    }
    glEnd();
}
float distBetween(int x1, int y1, int x2, int y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = std::sqrt(dx * dx + dy * dy);
    dist = std::min(MAX_INTENSITY, dist);
    return dist;
}
/* Lowers the brightness of every pixel on screen by 0.5*/
void fadePixels(Window& w) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if(w.pixels[y][x] > 0) {
                w.pixels[y][x] -= 0.5;
            }
        }
    }
}
/* Draws onto w.pixels. Brightens up a circular section of the screen */
void drawLight(Window& w, int x, int y, int radius) {
    // no point should get darker because of this
    w.pixels[y][x] = MAX_INTENSITY;
    int blockRadius = (int)MAX_INTENSITY;
    // draw a whole gradient circle around it
    for (int sy = -blockRadius; sy <= blockRadius; ++sy) {
        for (int sx = -blockRadius; sx <= blockRadius; ++sx) {
            float intensity = blockRadius - distBetween(x,y,x+sx,y+sy);
            if (y + sy > 0 && y + sy < HEIGHT && x + sx > 0 && x + sx < WIDTH) {
                if (w.pixels[y+sy][x+sx] < intensity) {
                    w.pixels[y+sy][x+sx] = intensity;
                }
            }
        }
    }
}

/*Create a count of particles matching a flit in direction
    and speed (somewhat)
*/
void spawnParticlesAtFlit(Flit& flit, std::vector<Particle>& particles, int count) {
    // default color of white
    for(int i = 0; i < count; i++) {
        float newColor[3] = {1.0f,1.0f,1.0f};
        Particle p = Particle(flit.getGlobalPosX(), flit.getGlobalPosY(), flit.getSpeed(), flit.getDir(), newColor);
        particles.push_back(p);
    }
}
void cullBlackPixels(std::vector<Particle>& particles) {
    // remove the black pixels
    for(int i = particles.size()-1; i > -1; i--) {
        if (particles[i].color[0] < .01f) {
            particles.erase(particles.begin() + i);
        } else if (particles[i].x > WIDTH - 1 || particles[i].x < 1) {
            particles.erase(particles.begin() + i);
        } else if (particles[i].y > HEIGHT - 1 || particles[i].y < 1) {
            particles.erase(particles.begin() + i);
        }
    }
}
GLFWwindow* initGLFWWindow() {
    // DEAL WITH LOADING ERRORS
    if (!glfwInit()) {
        std::cerr << "Fail to init GLFW\n";
    }
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello GLFW", nullptr, nullptr);
    if (!window) {
        std::cerr << "Fail to create window\n";
        glfwTerminate();
    }

    // CREATE WINDOW
    glfwMakeContextCurrent(window);

    // Allow for pixel coordinate drawing
    glViewport(0, 0, WIDTH, HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Allows for pixel drawing instead of screen percentage drawing
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return window;
}
void trackSpeed(Window& w) {
    w.t1 = currentTimeMs();
    if (w.t1 > w.second_out) {
        std::cout << "speed: " << (w.frames) << std::endl;
        w.second_out = w.t1 + 1000;
        w.frames = 0;
    }
    w.frames++;
}
/* may not need to as every pixel is being taken care of */
void clearScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void loadNextScreen(GLFWwindow* window) {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main() {
    Window w = Window();
    std::vector<Flit> flits = getFlits(5);
    std::vector<Particle> particles = {};

    GLFWwindow* window = initGLFWWindow();

    // PRINT LOOP
    while (!glfwWindowShouldClose(window)) {

        trackSpeed(w);

        clearScreen();

        // Begin drawing
        drawPixels(w);
        drawParticles(w, particles);
        fadePixels(w);   
        // drawFlitCircle(flit); // light is already obvious

        // update objects
        for(int i = 0; i < flits.size(); i++){
            Flit &flit = flits[i];
            flit.nextPos();
            spawnParticlesAtFlit(flit, particles, 10);
            drawLight(w, (int)flit.getGlobalPosX(), (int)flit.getGlobalPosY(), (int)MAX_INTENSITY);
        }

        cullBlackPixels(particles);
        
        loadNextScreen(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}