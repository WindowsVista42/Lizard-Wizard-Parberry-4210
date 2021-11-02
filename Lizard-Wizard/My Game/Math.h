#include "Defines.h"

Mat4x4 MoveRotateScaleMatrix(Vec3 position, Quat rotation, Vec3 scale);
Mat4x4 MoveScaleMatrix(Vec3 position, Vec3 scale);

Vec3 JitterVec3(Vec3 input, f32 negativeAccuracy, f32 range);

// Bullet Trajectory
btQuaternion CalculateVelocity(Vec3 origin, Vec3 target, f32 time);

// NPC Pathing
Vec3 RandomPointInRadius(Vec3 origin, f32 radius);