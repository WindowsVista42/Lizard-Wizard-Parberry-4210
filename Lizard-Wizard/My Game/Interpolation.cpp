#include "Interpolation.h"
#include "Defines.h"

// Vector Specific Interpolation
Vec3 Vec3Lerp(Vec3 from, Vec3 to, f32 percent) {
    Vec3 newVector = Vec3(
        from.x + (to.x - from.x) * percent,
        from.y + (to.y - from.y) * percent,
        from.z + (to.z - from.z) * percent
    );
    return newVector;
}

Vec3 Vec3Slerp(Vec3 from, Vec3 to, f32 percent) {
    // Note (Ethan) : A very nice way to interpolate object rotation in 3D space, somewhat expensive. (Can account for velocity)

    // Never Slerp values close to zero.
    if (percent < 0.01f) return Vec3Lerp(from, to, percent);

    // Get Angles
    Vec3 fromAngles = XMVector3Normalize(from);
    Vec3 toAngles = XMVector3Normalize(to);
    f32 theta = *(f32*)&XMVector3AngleBetweenVectors(from, to);

    // Calculate Changes
    f32 a = sinf((1.0f - percent) * theta) / sinf(theta);
    f32 b = sinf(percent * theta) / sinf(theta);

    return fromAngles * a + toAngles * b;
}

Vec3 Vec3NLerp(Vec3 from, Vec3 to, f32 percent) {
    // Note (Ethan) : Quite literally an approximation of SLerp, use this when you wanna save some gas. (Doesn't account for velocity)

    Vec3 newVector = Vec3(
        from.x + (to.x - from.x) * percent,
        from.y + (to.y - from.y) * percent,
        from.z + (to.z - from.z) * percent
    );
    return XMVector3Normalize(newVector);
}

// Non-Type Specific Interpolation
template <typename T> T LinearLerp(T& from, T& to, f32 percentage) {
    return from + percentage * (to - from);
}

template <typename T> T SineLerp() {
    // Note (Ethan) : Will be added in the future.
}

// Note (Ethan) : Never implemented this before but this might work? Hermite Curve Interpolation did not have many resources.
// Check out https://www.cubic.org/docs/hermite.htm for more information.
template <typename T> T HermiteLerp(T& from, T& to, f32 tan1, f32 tan2, u32 percent) {
    // Math to get a Hermite Curve
    f32 h1 = 2 * pow(percent, 3) - 3 * pow(percent, 2) - 1;
    f32 h2 = -2 * pow(percent, 3) + 3 * (percent, 2);
    f32 h3 = pow(percent, 3) - 2 * pow(percent, 2) + percent;
    f32 h4 = pow(percent, 3) - pow(percent, 2);

    // Pack all numbers together to get an interpolated point.
    T newPoint = h1 * from +
        h2 * to +
        h3 * tan(tan1) +
        h4 * tan(tan2);

    return newPoint;
}