#pragma once

#include <random>

namespace mathf
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TAU = 6.28318530717958647692f;
    constexpr float DEG2RAD = 0.017453292519943295769f;
    constexpr float RAD2DEG = 57.2957795130823f;
    
    inline float RandomRange(float min, float max)
    {
        float t = (float)rand() / RAND_MAX;
        return min + (max - min) * t;
    }
}