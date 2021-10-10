#include "Convert.h"

Vector3 convert(btVector3 input) {
    return *(Vector3*)&input;
}

btVector3 convert(Vector3 input) {
    return *(btVector3*)&input;
}
