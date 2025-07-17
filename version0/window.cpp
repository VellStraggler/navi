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
#include "myath.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <fstream>

/* SETTINGS */

const char* audioFileName = "./version0/audio.wav";
const char* outputFileName = "./version0/output.mp4";

constexpr float MAX_INTENSITY = 1.0f;
constexpr float MAX_LIGHT_REACH = 100.0f;
constexpr int HEIGHT = 480;
constexpr int WIDTH = 720;
constexpr int FLIT_COUNT = 3;
constexpr int NEW_FRAME_PARTICLES = 15;
constexpr int FRAMERATE = 60; // rendered video is still fixed to 30 :/
constexpr int VOLUME_MULT = 4;
constexpr float TEMPO = 120;

// https://www.rapidtables.com/web/color/RGB_Color.html
constexpr uint8_t COLOR1[] = {255,255,100};
constexpr uint8_t COLOR2[] = {100,255,100};
constexpr uint8_t COLOR3[] = {127,255,255};

constexpr bool RENDER = false;

/* PROGRAM COMMANDS //
(from ".../Navi Project")

cmake --build build

./build/MyGLFWAPP.exe

ffmpeg -framerate 30 -i version0/frames/frame_%05d.ppm -i version0/audio.wav -c:v libx264 -pix_fmt yuv420p -c:a aac -shortest version0/output.mp4

/* -------- */

constexpr uint8_t BLACK[] = {0,0,0};
constexpr uint8_t WHITE[] = {25,255,255};

uint8_t R_INDEX[256]; 
uint8_t G_INDEX[256]; 
uint8_t B_INDEX[256];

std::atomic<float> g_currentVolume(0.0f);
GLuint screenTexID;
std::vector<uint8_t> pixelBuffer(WIDTH * HEIGHT * 3);
ma_decoder decoder;

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
                flits[i].obstacles.push_back(flits[j]);
            }
        }
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
    double frame_out = t1 + (1000.0/FRAMERATE);
    int64_t second_out = t1 + 1000;
    bool hasRenderedAtLeastOnce = false;
    float lastVol = 0;
    int sameVolCount = 0; // at 10: rudimentary way to know when the song is over

    int particleCount = 0;

    float currentVolume;
    
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
    const float radius = flit.getRadius();
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

        
        if (p.radius > 1) {
            for (int x = intx - 1; x < intx + 1; x++) {
                for (int y = inty - 1; y < inty + 1; y++) {
                    if (w.getPixel(x, y) < p.intensity * 80) {
                        w.setPixel(x, y, p.intensity * 80);
                    }
                }
            }
        }
        if (w.getPixel(intx, inty) < p.intensity * 100) {
            w.setPixel(intx, inty, p.intensity * 100);
        }
        // always pick the brighter intensity
        //update pixel for future draws
        p.update(w.currentVolume);
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
            int index = (y * WIDTH + x) * 3;
            if (i > 0) {
                // DIMMING
                w.setPixel(x, y, i - 1);
                // DRAWING
                // my super special, epictacular light equation
                i = i * i * (i / 500000.0f);
                float value = getMin(i, 1.0f);
                uint8_t g = static_cast<uint8_t>(value * 255.0f);
                pixelBuffer[index + 0] = R_INDEX[g];
                pixelBuffer[index + 1] = G_INDEX[g];
                pixelBuffer[index + 2] = B_INDEX[g];

            } else {
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
    dist = getMin(MAX_LIGHT_REACH, dist);
    return dist;
}
/* Draws onto w.pixels. Brightens up a circular section of the screen */
void drawLight(Window& w, int x, int y, float radius) {
    // no point will get darker because of this
    w.setPixel(x, y, MAX_INTENSITY);
    int blockRadius = getMin(MAX_LIGHT_REACH,  MAX_LIGHT_REACH * radius * VOLUME_MULT);
    // draw a whole gradient circle around it
    for (int sy = -blockRadius; sy <= blockRadius; ++sy) {
        for (int sx = -blockRadius; sx <= blockRadius; ++sx) {
            float intensity = blockRadius - distBetween(x,y,x+sx,y+sy);
            if (y + sy > 0 && y + sy < HEIGHT && x + sx > 0 && x + sx < WIDTH) {
                w.setPixel(x+sx, y+sy, getMax(w.getPixel(x+sx, y+sy), intensity));
            }
        }
    }
}

/*Create a count of particles matching a flit in direction
    and speed (somewhat)
*/
void spawnParticlesAtFlit(Flit& flit, std::vector<Particle>& particles, int count, float currentVolume) {
    // default color of white
    for(int i = 0; i < count; i++) {
        // future implementation: volume determines particle speed multiplier
        Particle p = Particle(flit.getX(), flit.getY(), 1 + currentVolume, flit.getDir(), MAX_INTENSITY, 750);
        particles.push_back(p);
        for(int j = 0; j < 4; j++) {
            int x = flit.getX() + (flit.getRadius() * std::cos(flit.getDir() + (PI_2 * j)));
            int y = flit.getY() + (flit.getRadius() * std::sin(flit.getDir() + (PI_2 * j)));
            Particle p = Particle(x, y, 1, flit.getDir(), MAX_INTENSITY, 250);
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
void printSecondInfo(Window& w, int volI) {
    w.t1 = currentTimeMs();
    if (w.t1 > w.second_out) {
        std::cout << (w.frames) << " fps, " << volI << " total frames" <<std::endl;
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

            p.speed = getMin((1 - blend) * p.speed + blend * fspeed, 1.0f);
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

bool frameTooSoon(Window& w) {
    w.t1 = currentTimeMs();
    const double frameDuration = 1000.0 / FRAMERATE;
    

    // Force the first frame through, even if it's "too soon"
    if (!w.hasRenderedAtLeastOnce) {
        // w.frame_out = w.t1 + frameDuration;
        w.hasRenderedAtLeastOnce = true;
        return false;
    }

    if (w.t1 >= w.frame_out) {
        w.frame_out += frameDuration;
        return false;
    }
    return true;
}

void savePixelBufferAsPPM(int frameNumber) {
    char filename[64];
    std::snprintf(filename, sizeof(filename), "version0/frames/frame_%05d.ppm", frameNumber);
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

    // PPM format is top-to-bottom, left-to-right, RGB
    // pixelBuffer is already in that format but upside-down,
    // so we flip vertically:
    for (int y = 0; y < HEIGHT; ++y) {
        int rowStart = (HEIGHT - 1 - y) * WIDTH * 3;
        out.write(reinterpret_cast<char*>(&pixelBuffer[rowStart]), WIDTH * 3);
    }
}

int liveRender() {
    Window w = Window();
    std::vector<Flit> flits = getFlits(FLIT_COUNT);
    std::vector<Particle> particles = {};
    int volI = 0;

    GLFWwindow* window = initGLFWWindow();
    initTexture();

    // PRINT LOOP
    float vol;
    // the volume never hits 0, but it does flatten to a very very small number
    // we'll look for a flattening to see when the song ends
    while (!glfwWindowShouldClose(window) && w.sameVolCount < 10) {

        // lock to given framerate
        while (frameTooSoon(w)) {
            glfwWaitEventsTimeout(0.001);
        }
        printSecondInfo(w, volI);

        // clearScreen(); //unnecessary

        // Begin drawing
        drawParticles(w, particles);
        cullBlackPixels(particles);
        
        // update Flits
        w.lastVol = vol;
        vol = g_currentVolume.load(std::memory_order_relaxed);
        if (w.lastVol == vol) {
            w.sameVolCount++;
        } else {
            w.sameVolCount = 0;
            vol = (w.lastVol + vol)/2;
        }
        w.currentVolume = vol;
        // std::cout<< vol << " ";
        float centX = 0;
        float centY = 0;
        for(Flit flit: flits) {
            centX += flit.getX();
            centY += flit.getY();
        }
        float centDx = (WIDTH/2) - (centX/flits.size());
        float centDy = (HEIGHT/2) - (centY/flits.size());

        for(Flit& flit: flits){
            // place the shared middle of all Flits in the center of the screen
            flit.setX(flit.getX() + (centDx * 0.01f));  // small adjustment factor
            flit.setY(flit.getY() + (centDy * 0.01f));

            flit.implementVolume(vol);
            flit.nextPos();
            affectParticles(flit, w, particles);
            spawnParticlesAtFlit(flit, particles, NEW_FRAME_PARTICLES, vol);
            drawLight(w, (int)flit.getX(), (int)flit.getY(), vol);
        }
        
        drawAndFadePixels(w);
        if (RENDER) {
            savePixelBufferAsPPM(volI);
        }
        loadNextScreen(window);
        volI++;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

void audio_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    ma_decoder* decoder = (ma_decoder*)device->pUserData;

    ma_uint64 framesRead = 0;
    ma_result result = ma_decoder_read_pcm_frames(decoder, output, frameCount, &framesRead);

    if (result != MA_SUCCESS || framesRead == 0) {
        // Fill remaining with silence
        memset(output, 0, frameCount * device->playback.channels * sizeof(float));
        return;
    }

    float* samples = (float*)output;
    float maxVol = 0.0f;
    for (ma_uint64 i = 0; i < framesRead * device->playback.channels; ++i) {
        maxVol = getMax(maxVol, std::fabs(samples[i]));
    }

    g_currentVolume.store(maxVol, std::memory_order_relaxed);

    // Fill any remaining part if underrun
    if (framesRead < frameCount) {
        memset(samples + framesRead * device->playback.channels, 0,
                    (frameCount - framesRead) * device->playback.channels * sizeof(float));
    }
}

int main() {
    // create index sheet for choosing colors quickly from intensity
    fillColorIndexArrays(R_INDEX, G_INDEX, B_INDEX, COLOR1, COLOR2, COLOR3);

    //////////////////////
    // AUDIO PROCESSING //
    //////////////////////

    // Force decoder to output float32 format
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_result result = ma_decoder_init_file(audioFileName, &config, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio file.\n";
        return -1;
    }

    // Configure playback device with float32 format
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = audio_callback;
    deviceConfig.pUserData         = &decoder;

    // Initialize playback device
    ma_device device;
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device.\n";
        ma_decoder_uninit(&decoder);
        return -1;
    }

    // Start playback
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start playback device.\n";
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    // Run visualizer
    liveRender();

    // Cleanup
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}