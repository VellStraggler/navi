// HOW TO RUN

// be in Navi Project, cmd: cmake --build build
// then cmd: ./build/MyGLFWApp.exe

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include "flit.cpp"
#include "particle.cpp"
#include <cmath>
#include "processedaudio.h"
#include <algorithm>

constexpr float MAX_INTENSITY = 1.0f;
constexpr float MAX_LIGHT_REACH = 100.0f;
constexpr int HEIGHT = 480;
constexpr int WIDTH = 720;
constexpr int FLIT_COUNT = 10;
constexpr int NEW_FRAME_PARTICLES = 8;
GLuint screenTexID;
std::vector<uint8_t> pixelBuffer(WIDTH * HEIGHT * 3);

/*All Flits start out in the center of the screen.*/
std::vector<Flit> getFlits(int count) {
    std::vector<Flit> flits = {};
    for(int i = 0; i < count; i++) {
        Flit flit = Flit(WIDTH/2.0f, HEIGHT/2.0f);
        flit.addDir((twoPI/count)*i);
        flits.push_back(flit);
    }
    // Add each other as obstacles
    for(int i =0; i < count;i++) {
        for(int j = 0; j < count; j++) {
            if(i != j) {
                GeomLineRef ref = GeomLineRef(flits[j].getXYRef(), flits[j].radius, true);
                flits[i].obstacles.push_back(ref);
            }
        }
        // walls
        // add screen edges as collision points
        Point topLeft(0, HEIGHT);
        Point bottomLeft(0,0);
        Point bottomRight(WIDTH,0);
        flits[i].obstacles.push_back(GeomLineRef(bottomLeft, WIDTH, false));
        flits[i].obstacles.push_back(GeomLineRef(topLeft, WIDTH, false));
        flits[i].obstacles.push_back(GeomLineRef(bottomLeft, HEIGHT, true));
        flits[i].obstacles.push_back(GeomLineRef(bottomRight, HEIGHT, true));
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
    
    float getPixel(int x, int y) const {
        if(x < 1 || x > WIDTH -1 || y < 1 || y > HEIGHT -1) {
            return (0.0f);
        } else {
            return pixels[y][x];
        }
    }
    void setPixel(int x, int y, float val) {
        if(x > 0 && x < WIDTH && y > 0 && y < HEIGHT) {
            pixels[y][x] = val;
        }
    }
    Window() = default;
private:
    /*Stores all pixels (y,x format) as an intensity number from 0f to 1.0f*/
    float pixels[HEIGHT][WIDTH] = {0.0f};
    
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
    const float x = flit.getX();
    const float y = flit.getY();
    const float radius = flit.radius;
}
/* Draws particles to w.pixels, NOT directly to the screen*/
void drawParticles(Window& w, std::vector<Particle>& ps) {
    // DEBUG LINE: infrequently give size
    if (ps.size()%100 == 0) {
        if (ps.size() != w.particleCount) {
            std::cout << ps.size() << " particles rendered" << std::endl;
            w.particleCount = ps.size();
        }
    }
    for (Particle& p : ps) {
        int intx = (int)p.x;
        int inty = (int)p.y;
        // always pick the brighter intensity
        if (w.getPixel(intx, inty) < p.intensity * 100) {
            w.setPixel(intx, inty, p.intensity * 100);
        }
    
        //update pixel for future draws
        p.update();
    }
}
/* 
    Draws all pixels to the screen as they are stored in w.pixels.
    Converts linear brightness into a higher contrast version as well.
*/
void drawAndFadePixels(Window& w) {
    // fill buffer with our array
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            float i = w.getPixel(x, y);
            if (i > 0) {
                // DIMMING
                w.setPixel(x, y, i - 1);
                // DRAWING
                i = i * i * (i / 500000.0f);
                float value = std::min(i, 1.0f);
                uint8_t gray = static_cast<uint8_t>(value * 255.0f);
                int index = (y * WIDTH + x) * 3;
                pixelBuffer[index + 0] = gray;
                pixelBuffer[index + 1] = gray;
                pixelBuffer[index + 2] = gray;

            } else {
                int index = (y * WIDTH + x) * 3;
                pixelBuffer[index + 0] = 0;
                pixelBuffer[index + 1] = 0;
                pixelBuffer[index + 2] = 0;
            }
        }
    }
    // upload it to a texture
    glBindTexture(GL_TEXTURE_2D, screenTexID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer.data());

    // render the texture to screen as a textured quad
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, screenTexID);
    glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f); // bottom left
        glTexCoord2f(1.0f, 0.0f); glVertex2f(WIDTH, 0.0f); // bottom right
        glTexCoord2f(1.0f, 1.0f); glVertex2f(WIDTH, HEIGHT); // top right
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, HEIGHT); // top left
    
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
float distBetween(int x1, int y1, int x2, int y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = std::sqrt(dx * dx + dy * dy);
    dist = std::min(MAX_LIGHT_REACH, dist);
    return dist;
}
/* Draws onto w.pixels. Brightens up a circular section of the screen */
void drawLight(Window& w, int x, int y, int radius) {
    // no point should get darker because of this
    w.setPixel(x, y, MAX_INTENSITY);
    int blockRadius = MAX_LIGHT_REACH;
    // draw a whole gradient circle around it
    for (int sy = -blockRadius; sy <= blockRadius; ++sy) {
        for (int sx = -blockRadius; sx <= blockRadius; ++sx) {
            float intensity = blockRadius - distBetween(x,y,x+sx,y+sy);
            if (y + sy > 0 && y + sy < HEIGHT && x + sx > 0 && x + sx < WIDTH) {
                if (w.getPixel(x+sx, y+sy) < intensity) {
                    w.setPixel(x+sx, y+sy, intensity);
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
        // future implementation: volume determines particle speed multiplier
        Particle p = Particle(flit.getX(), flit.getY(), 1, flit.getDir(), MAX_INTENSITY, 750);
        particles.push_back(p);
        for(int j = 0; j < 4; j++) {
            int x = flit.getX() + (flit.boxRadius * std::cos(flit.getDir() + (PI_2 * j)));
            int y = flit.getY() + (flit.boxRadius * std::sin(flit.getDir() + (PI_2 * j)));
            Particle p = Particle(x, y,
                1, flit.getDir(), MAX_INTENSITY, 250);
            particles.push_back(p);
        }
        if (flit.explode) {
            flit.explode = false;
            int booms = 100;
            for(int j = 0; j < booms; j++) {
                Particle p = Particle(flit.getX(), flit.getY(), 3, j*(twoPI/booms), MAX_INTENSITY, 200);
                p.timeAlive = -30;
                particles.push_back(p);
            }
        }
    }
}
void cullBlackPixels(std::vector<Particle>& particles) {
    for (size_t i = 0; i < particles.size(); ) {
        Particle& p = particles[i];
        if (p.intensity < 0.01f || p.x > WIDTH - 1 || p.x < 1 || p.y > HEIGHT - 1 || p.y < 1) {
            // replace a dead pixel with the last in the list and pop it out
            particles[i] = particles.back();
            particles.pop_back();
        } else {
            // this only moves up if it does not find a dead pixel
            ++i;
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
    // glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return window;
}
void trackSpeed(Window& w) {
    w.t1 = currentTimeMs();
    if (w.t1 > w.second_out) {
        std::cout << (w.frames) << " fps" << std::endl;
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
/*FLiTs gently pull nearby particles along, like gravity*/
void affectParticles(const Flit& flit, Window& w, std::vector<Particle>& particles) {
    float fx = flit.getX();
    float fy = flit.getY();
    float fdx = flit.getDX();
    float fdy = flit.getDY();
    float fspeed = flit.speed;
    float fdirection = flit.getDir();
    float influence = 75;

    for(Particle& p : particles) {
        if (p.timeAlive < 0) {
            continue;
        }
        float absDist = std::sqrt((p.x - fx)*(p.x - fx) + (p.y - fy)*(p.y - fy));
        if (absDist < influence) {
            float percent = (influence - absDist)/influence;
            float blend = (std::pow(percent,2));
            float angleToCenter = std::atan2(fy - p.y, fx - p.x);
            
            float dirDiff = std::atan2(std::sin(angleToCenter - p.direction),
                                       std::cos(angleToCenter - p.direction));
            p.direction += dirDiff * blend * 4.5f;

            p.speed = std::min((1 - blend) * p.speed + blend * fspeed, 1.0f);
        }
    }
}

void initTexture() {
    glGenTextures(1, &screenTexID);
    glBindTexture(GL_TEXTURE_2D, screenTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

int liveRender() {
    Window w = Window();
    std::vector<Flit> flits = getFlits(FLIT_COUNT);
    std::vector<Particle> particles = {};

    GLFWwindow* window = initGLFWWindow();
    initTexture();

    // PRINT LOOP
    while (!glfwWindowShouldClose(window)) {

        trackSpeed(w);

        // clearScreen();

        // Begin drawing
        drawParticles(w, particles);
        cullBlackPixels(particles);
        
        // update Flits
        for(int i = 0; i < flits.size(); i++){
            Flit &flit = flits[i];
            flit.nextPos();
            affectParticles(flit, w, particles);
            spawnParticlesAtFlit(flit, particles, NEW_FRAME_PARTICLES);
            drawLight(w, (int)flit.getX(), (int)flit.getY(), (int)MAX_INTENSITY);
        }
        
        drawAndFadePixels(w);
        loadNextScreen(window);

        // DEBUG LINE
        // std::cout << particles[0].toString() << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
int main() {
    // liveRender();
    // ProcessedAudio pa = ProcessedAudio("audio.wav");
    // pa.run();
    const char* audioFileName = "./version0/audio.wav";

    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_decoder decoder;
    ma_result result = ma_decoder_init_file(audioFileName, &config, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio file.\n";
        return -1;
    }

    std::cout << "Decoder ready. Channels: " << decoder.outputChannels
              << ", Format: " << decoder.outputFormat << "\n";

    const int UPS = 30;
    float buffer[decoder.outputChannels * 1024];

    while (true) {
        std::fill(buffer, buffer + 1024, 0.0f); // optional

        ma_uint64 framesRead = 0;
        result = ma_decoder_read_pcm_frames(&decoder, buffer, 1024, &framesRead);
        if (result != MA_SUCCESS) {
            std::cerr << "Decoder read failed.\n";
            break;
        }

        if (framesRead == 0 || decoder.outputChannels == 0) {
            std::cout << "No more frames.\n";
            break;
        }

        std::cout << "Frames read: " << framesRead << "\n";

        float sum = 0.0f;
        for (ma_uint64 i = 0; i < framesRead * decoder.outputChannels; ++i) {
            sum += std::fabs(buffer[i]);
        }

        float avg = sum / (framesRead * decoder.outputChannels);
        std::cout << "Volume: " << avg << "\n";

        // 1/60 sec delay
        // not necessary for pre-processing
        // int64_t t1 = currentTimeMs() + (1000.0 / UPS);
        // while (currentTimeMs() < t1);
    }

    ma_decoder_uninit(&decoder);
    return 0;
}