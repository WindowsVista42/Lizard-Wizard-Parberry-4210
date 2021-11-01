#include "Defines.h"

Mat4x4 MoveRotateScaleMatrix(Vec3 position, Quat rotation, Vec3 scale);
Mat4x4 MoveScaleMatrix(Vec3 position, Vec3 scale);

Vec3 JitterVec3(Vec3 input, f32 negativeAccuracy, f32 range);

btQuaternion CreateFromAxisAngles(Vec3 axis, f32 angle);
btQuaternion LookAt(Vec3 origin, Vec3 lookAt);