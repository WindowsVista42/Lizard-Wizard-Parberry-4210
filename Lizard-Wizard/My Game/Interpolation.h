// This file exist to offer some customing interpolation options for a broad scope of purposes.

// Inclusions
#include "Game.h"
#include "Defines.h"
#include <ctime>
#include <random>
#include <chrono>
#include <iostream>

// Set template
template <typename T>

// Interpolation
class Animatiom {
    T LinearLerp() {

    }

    T SineLerp() {

    }

    // Note (Ethan) : Never implemented this before but this might work? Hermite Curve Interpolation did not have many resources.
    // Check out https://www.cubic.org/docs/hermite.htm for more information.
    T HermiteLerp(T&From , T&To, i32 tan1, i32 tan2, u32 steps) {
        for every(step, steps) {

            // Percentage of LERP
            f32 completion = (f32)step / (f32)steps;

            // Dense math to get a Hermite Curve
            f32 h1 = 2 * pow(completion, 3) - 3 * pow(completion, 2) - 1;
            f32 h2 = -2 * pow(completion, 3) + 3 * (completion, 2);
            f32 h3 = pow(completion, 3) - 2 * pow(completion, 2) + completion;
            f32 h4 = pow(completion, 3) - pow(completion, 2);

            // Pack all numbers together to get an interpolated point.
            T newPoint = h1 * From +
                         h2 * To +
                         h3 * tan(tan1) +
                         h4 * tan(tan2);
        }
    }
};