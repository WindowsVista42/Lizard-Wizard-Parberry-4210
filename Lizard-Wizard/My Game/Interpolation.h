// Inclusions
#include "Game.h"
#include "Defines.h"
#include <ctime>
#include <random>
#include <chrono>
#include <iostream>

// Vector Specific
Vec3 Vec3Lerp(Vec3 from, Vec3 to, f32 percent);
Vec3 Vec3Slerp(Vec3 from, Vec3 to, f32 percent);
Vec3 Vec3NLerp(Vec3 from, Vec3 to, f32 percent);

// Generic
template <typename T> T LinearLerp(T& from, T& to, f32 percentage);
template <typename T> T SineLerp();
template <typename T> T HermiteLerp(T& from, T& to, f32 tan1, f32 tan2, u32 percent);