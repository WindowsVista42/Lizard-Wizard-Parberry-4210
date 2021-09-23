/// \file Object.cpp
/// \brief Code for the game object class CObject.

#include "Object.h"
#include "ComponentIncludes.h"

/// \param t Type of sprite.
/// \param pos Initial position of object.
/// \param vel Initial velocity of object.

CObject::CObject(eSpriteType t, const Vector2& pos, const Vector2& vel):
  m_vVelocity(vel){ 
  m_nSpriteIndex = t;
  m_vPos = pos; 
  CreateBoundingShapes();
} //constructor

/// Move and update all bounding shapes.
/// The paddle gets moved by the player, everything
/// else moves an amount that depends on its velocity and the
/// frame time.

void CObject::move(){ 
  const float t = m_pStepTimer->GetElapsedSeconds();

  switch(m_nSpriteIndex){
    case BALL_SPRITE:
      m_vPos += m_vVelocity*t;
      if(m_vPos.y < -m_pRenderer->GetHeight(m_nSpriteIndex)/2){
        kill();
        m_bBallInPlay = false;
        m_pAudio->play(BUZZ_SOUND);
      } //if
    break;

    case PADDLE_SPRITE:
      m_vPos.x += m_fSpeed*t;
    break;
  } //switch

  CreateBoundingShapes();
} //move

/// Create a bounding box and a bounding sphere from the 
/// dmiensions of the object's sprite.

void CObject::CreateBoundingShapes(){
  float w, h; //sprite width and height
  m_pRenderer->GetSize(m_nSpriteIndex, w, h);
  
  //create bounding box
  const Vector2 tl = m_vPos + 0.5f*Vector2(-w, h); //top left corner
  const Vector2 br = m_vPos + 0.5f*Vector2(w, -h); //bottom right corner
  BoundingBox::CreateFromPoints(m_Aabb, tl, br);

  //create bounding sphere
  m_Sphere.Radius = max(w, h)/2;
  m_Sphere.Center = (Vector3)m_vPos;
} //CreateBoundingShapes

/// Perform the appropriate response to a collision.
/// \param b Axially aligned bounding box for the object it collided with.

void CObject::CollisionResponse(const BoundingBox& b){
  float w, h;
  m_pRenderer->GetSize(m_nSpriteIndex, w, h);
  w /= 2; h /= 2;
  
  const float top = b.Center.y + b.Extents.y;
  const float bottom = b.Center.y - b.Extents.y;

  const float left = b.Center.x - b.Extents.x;
  const float right = b.Center.x + b.Extents.x;

  if(m_vPos.y > top && m_vPos.y < top + h){
    m_vPos.y = top + h;
    if(m_vVelocity.y < 0)
      m_vVelocity.y = -m_vVelocity.y;
  } //if

  else if(m_vPos.y < bottom && m_vPos.y > bottom - h){
    m_vPos.y = bottom - h;
    if(m_vVelocity.y > 0)
     m_vVelocity.y = -m_vVelocity.y;
  } //else if

  else if(m_vPos.x < left && m_vPos.x > left - w){
    m_vPos.x = left - w - 0.5f;
    if(m_vVelocity.x > 0)
      m_vVelocity.x = -m_vVelocity.x;
  } //else if

  else if(m_vPos.x > right && m_vPos.x < right + w){
    m_vPos.x = right + w + 0.5f;
    if(m_vVelocity.x < 0)
      m_vVelocity.x = -m_vVelocity.x;
  } //else if
} //CollisionResponse

/// Reader function for position.
/// \return Position.

const Vector2& CObject::GetPos(){
  return m_vPos;
} //GetPos

/// Set the object's speed, assuming that the object
/// moves according to its speed.
/// \param speed Speed.

void CObject::SetSpeed(float speed){
  m_fSpeed = speed;
} //SetVelocity

/// Kill an object by marking its "is dead" flag. The object
/// will get deleted later at the appropriate time.

void CObject::kill(){
  m_bDead = true;
} //kill

/// Reader function for the "is dead" flag.
/// \return true if marked as being dead, that is, ready for disposal.

bool CObject::IsDead(){
  return m_bDead;
} //IsDead


