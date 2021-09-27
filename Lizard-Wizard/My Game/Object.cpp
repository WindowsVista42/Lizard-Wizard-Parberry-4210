/// \file Object.cpp
/// \brief Code for the game object class CObject.

#include "Object.h"
#include "ComponentIncludes.h"

/// Create an object, given its sprite type and initial position.
/// \param t Type of sprite.
/// \param p Initial position of object.

CObject::CObject(eSprite t, const Vector2& p):
  LBaseObject(t, p){ 
} //constructor

/// The only object in this game is the text wheel, which slowly rotates at 1/8
/// revolutions per second. This is achieved by adding a small amount to its
/// roll angle, proportional to frame time.

void CObject::move(){ 
  const float t = m_pTimer->GetFrameTime(); //frame interval in seconds
  m_fRoll += 0.125f*XM_2PI*t; //rotate at 1/8 RPS
} //move

/// Ask the renderer to draw the sprite described in the sprite descriptor.
/// Note that `CObject` is derived from `LBaseObject` which is inherited from
/// `LSpriteDesc2D`. Therefore `LSpriteRenderer::Draw(const LSpriteDesc2D*)`
/// will accept `this` as a parameter, automatically down-casting it from
/// `CObject*` to `LSpriteDesc2D*`, effectively drawing the object from its
/// sprite descriptor.

void CObject::draw(){ 
   //m_pRenderer->Draw(this);
} //draw
