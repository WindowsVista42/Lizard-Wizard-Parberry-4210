/// \file Object.h
/// \brief Interface for the game object class CObject.

#pragma once

#include "GameDefines.h"
#include "Renderer.h"
#include "Common.h"
#include "Component.h"
#include "SpriteDesc.h"

/// \brief The game object. 
///
/// CObject is the abstract representation of an object.

class CObject:
  public CCommon,
  public CComponent,
  public CSpriteDesc2D
{
  friend class CObjectManager;

  private:
    Vector2 m_vVelocity = Vector2::Zero; ///< Velocity.
    float m_fSpeed = 0.0f; ///< Speed.

    BoundingSphere m_Sphere; ///< Bounding sphere.
    BoundingBox m_Aabb; ///< Axially aligned bounding box.

    bool m_bDead = false; ///< Is dead or not.

    void CreateBoundingShapes(); ///< Create bounding shapes.

  public:
    CObject(eSpriteType t, const Vector2& pos, 
      const Vector2& vel=Vector2::Zero); ///< Constructor.

    void move(); ///< Move object.
    void CollisionResponse(const BoundingBox& b); ///< Collision response.
    void SetSpeed(float speed); ///< Set speed.
    void kill(); ///< Kill me.
    bool IsDead(); ///< Query whether dead.

    const Vector2& GetPos(); //< Get position.
}; //CObject