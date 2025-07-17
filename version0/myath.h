#pragma once
#include <cstdlib> // for std::rand and RAND_MAX
#include <array>
#include <iostream>

constexpr float PI = 3.14159265f;
constexpr float twoPI = PI * 2;
constexpr float PI_2 = PI / 2.0f;
constexpr float DEGREE = PI_2 / 90.0f;

float randFloat(float from, float to) {
    float zeroToOne = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return from + zeroToOne * (to - from);
}
float randFloat(float from0to) {
    return randFloat(0,from0to);
}
float toDegrees(float direction) {
    return 360 * direction / (PI_2*4);
}
template <typename T>
T clamp(T val, T minVal, T maxVal) {
    return std::max(minVal, std::min(val, maxVal));
}
float getMax(float a, float b) {
    return std::max(a, b);
}
float getMin(float a, float b) {
    return std::min(a, b);
}
float getDistance(float x, float y, float x2, float y2) {
    return (sqrt((x2-x)*(x2-x) + (y2-y)*(y2-y)));
}




void fillColorIndexArrays(
    uint8_t (&rIndex)[256],
    uint8_t (&gIndex)[256],
    uint8_t (&bIndex)[256],
    const uint8_t color1[3],
    const uint8_t color2[3],
    const uint8_t color3[3]
) {
    constexpr std::array<int, 5> xs = {0, 50, 127, 205, 255};

    std::array<uint8_t, 5> rVals = {0, color1[0], color2[0], color3[0], 255};
    std::array<uint8_t, 5> gVals = {0, color1[1], color2[1], color3[1], 255};
    std::array<uint8_t, 5> bVals = {0, color1[2], color2[2], color3[2], 255};

    // Helper lambda for linear interpolation between two points
    auto lerp = [](int x0, int x1, uint8_t y0, uint8_t y1, int x) -> uint8_t {
        if (x <= x0) return y0;
        if (x >= x1) return y1;
        float t = float(x - x0) / float(x1 - x0);
        return static_cast<uint8_t>(y0 + t * (y1 - y0) + 0.5f);
    };

    for (int i = 0; i < 256; ++i) {
        // Find the interval i is in:
        int segment = 0;
        while (segment < 4 && i > xs[segment + 1])
            ++segment;

        rIndex[i] = lerp(xs[segment], xs[segment + 1], rVals[segment], rVals[segment + 1], i);
        gIndex[i] = lerp(xs[segment], xs[segment + 1], gVals[segment], gVals[segment + 1], i);
        bIndex[i] = lerp(xs[segment], xs[segment + 1], bVals[segment], bVals[segment + 1], i);
    }
}
