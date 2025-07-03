#pragma once
#include <cstdlib> // for std::rand and RAND_MAX

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