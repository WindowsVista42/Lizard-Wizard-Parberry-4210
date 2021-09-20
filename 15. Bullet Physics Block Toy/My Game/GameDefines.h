/// \file GameDefines.h
/// \brief Game specific defines.

#pragma once

#include "Defines.h"
#include "btBulletDynamicsCommon.h" 

//////////////////////////////////////////////////////////////////////////////
// Physics World to Render World

/// \brief Mesh type.
///
/// Mesh enumerated type.

enum class eMesh: UINT{
  BigSphere, Sphere, Plane, Box, Tetrahedron, Teapot, Icosahedron, Dodecahedron,
  Model, Unknown, Size //must be last
}; //eMesh

/// \brief Model type.
///
/// Model enumerated type.

enum class eModel: UINT{
  Container, Crate, Unknown
}; //eModel

/// \brief Object type.
///
/// Object enumerated type.

enum class eObject: UINT{
  Moon, Ball, Floor, Box, Tetrahedron, Teapot, Icosahedron, Dodecahedron,
  Container, Crate, Unknown,
  Size //MUST be last
}; //eObject

/// \brief Collision shape type.
///
/// Collision shape enumerated type.

enum class eCollisionShape: UINT{
  Sphere, Box, ConvexHull, None
}; //eCollisionShape

/// \brief Sound enumerated type.
///
/// An enumerated type for the sounds, which will be cast to an unsigned
/// integer and used for the index of the corresponding sample. `Size` must 
/// be last.

enum class eSound: UINT{ 
  Clang, Click, TapLight, TapHard, ThumpLight, ThumpMedium, ThumpHard,
  Size //MUST be last
}; //eSound

//////////////////////////////////////////////////////////////////////////////
// Physics World to Render World

const float fPRV = 1.0f; ///< Physics World to Render World rescale value.

/// \brief Physics World to Render World units for a float.
inline float PW2RW(btScalar x){return float(x)*fPRV;}; 

/// \brief Physics World to Render World units for a vector.
inline Vector3 PW2RW(const btVector3& v){
  return Vector3((float)v.getX(), (float)v.getY(), (float)v.getZ())*fPRV;
}; //PW2RW

/// \brief Physics World to Render World units for a quaternion.
inline Quaternion PW2RW(const btQuaternion& q){
  return Quaternion(q.x(), q.y(), q.z(), q.w());
}; //PW2RW

//////////////////////////////////////////////////////////////////////////////
// Render World to Physics World

/// \brief Render World to Physics World units for a float.
inline btScalar RW2PW(float x){return btScalar(x/fPRV);}; 

/// \brief Render World to Physics World units for an int.
inline btScalar RW2PW(int x){return btScalar(x/fPRV);}; 

/// \brief Render World to Physics World units for a vector provided as a pair of ints.
inline btVector3 RW2PW(int x, int y, int z){
  return btVector3(btScalar(x), btScalar(y), btScalar(z))/fPRV;
}; //RW2PW

/// \brief Render World to Physics World units for a vector.
inline btVector3 RW2PW(const Vector3& v){
  return btVector3(btScalar(v.x), btScalar(v.y), btScalar(v.z))/fPRV;
}; //RW2PW

/// \brief Physics World to Render World units for a quaternion.
inline btQuaternion RW2PW(const Quaternion& q){
  return btQuaternion(q.x, q.y, q.z, q.w);
}; //PW2RW
