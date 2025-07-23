#include <cstdint>

/* SETTINGS */

const char* audioFileName = "./resources/first-song.wav";
const char* songTitle = "moon moon";
const char* outputFileName = "./output.mp4";
const char* transcriptFileName = "./resources/transcript.txt";

// maximum of 510,000 pixels with numbers divisible by 4
// 712 max for square screen
// 948x536 for 16:9 screen
constexpr int WIDTH = 948;
constexpr int HEIGHT = 536;

constexpr int FLIT_COUNT = 4;
constexpr int FRAMERATE = 200; // rendered video is still fixed to 30 :/
constexpr float TEMPO = 120;
uint32_t SEED = 12345;

constexpr int NEW_FRAME_PARTICLES = 1;
constexpr float MAX_LIGHT_REACH = 100.0f;
constexpr int VOLUME_MULT = 4;
constexpr int FLIT_INFLUENCE_RANGE = 100;

// https://www.rapidtables.com/web/color/RGB_Color.html
constexpr uint8_t COLOR1[] = {0,0,204};
constexpr uint8_t COLOR2[] = {102,0,104};
constexpr uint8_t COLOR3[] = {228,255,0};

constexpr bool EXPORT_VIDEO = false;
constexpr bool TRANSCRIBE = false;
constexpr bool VOLUME_AFFECTS_COLOR = true;
constexpr uint8_t VOLUME_COLOR_EFFECT[] = {0,0,255};