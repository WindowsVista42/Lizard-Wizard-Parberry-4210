#include <cstdint>
#include <Renderer3D.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t usize;

typedef bool b8;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t isize;

typedef float f32;
typedef double f64;

typedef VertexPositionNormalTexture VertexPNT;
typedef VertexPositionColor VertexPC;

#define every(name, count) (usize (name) = 0; (name) < (count); (name) += 1)

#define BT_SHAPE_TYPE_BOX 0
#define BT_SHAPE_TYPE_CAPSULE 10
