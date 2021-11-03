#include "Interpolation.h"

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
