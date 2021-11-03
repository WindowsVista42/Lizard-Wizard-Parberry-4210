#ifndef INTERPOLATION_H
#define INTERPOLATION_H

// Inclusions
#include "Defines.h"
#include <cmath>

// Vector Specific Interpolation
Vec3 Vec3Lerp(Vec3 from, Vec3 to, f32 percent);
Vec3 Vec3Slerp(Vec3 from, Vec3 to, f32 percent);
Vec3 Vec3NLerp(Vec3 from, Vec3 to, f32 percent);

// Non-Type Specific Interpolation
template <typename T>
T LinearLerp(T& from, T& to, f32 percentage) {
    return from + percentage * (to - from);
}

template <typename T>
T SineLerp() {
    // Note (Ethan) : Will be added in the future.
}

// Note (Ethan) : Never implemented this before but this might work? Hermite Curve Interpolation did not have many resources.
// Check out https://www.cubic.org/docs/hermite.htm for more information.
template <typename T>
T HermiteLerp(T& from, T& to, f32 tan1, f32 tan2, f32 percent) {
    // Math to get a Hermite Curve
    f32 h1 = 2.0f * powf(percent, 3.0f) - 3.0f * powf(percent, 2.0f) - 1.0f;
    f32 h2 = -2.0f * powf(percent, 3.0f) + 3.0f * (percent, 2.0f);
    f32 h3 = powf(percent, 3.0f) - 2.0 * powf(percent, 2.0f) + percent;
    f32 h4 = powf(percent, 3.0f) - powf(percent, 2.0f);

    // Pack all numbers together to get an interpolated point.
    T fromh1 = from * h1;
    T toh2 = to * h2;
    T h3tan1 = T(h3 * tanf(tan1));
    T h4tan2 = T(h4 * tanf(tan2));

    T newPoint = fromh1 + toh2 + h3tan1 + h4tan2;
       // from * h1
       // + to * h2
       // + h3 * tanf(tan1)
       // + h4 * tanf(tan2);

    return newPoint;
}

#endif
