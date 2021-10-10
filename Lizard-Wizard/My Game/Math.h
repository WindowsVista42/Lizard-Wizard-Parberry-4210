#include "Defines.h"

XMMATRIX MoveRotateScaleMatrix(Vector3 position, Quaternion rotation, Vector3 scale);
XMMATRIX MoveScaleMatrix(Vector3 position, Vector3 scale);

Vector3 JitterVector3(Vector3 input, f32 negativeAccuracy, f32 range);
