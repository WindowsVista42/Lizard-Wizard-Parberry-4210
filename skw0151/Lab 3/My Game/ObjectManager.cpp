/// \file ObjectManager.cpp
/// \brief Code for the the object manager class CObjectManager.

#include "ObjectManager.h"
#include "ComponentIncludes.h"
#include "ParticleEngine.h"

/// Create the tile manager;

CObjectManager::CObjectManager(){
} //constructor

/// Destruct all of the objects in the object list.

CObjectManager::~CObjectManager(){
  for(auto const& p: m_stdObjectList) //for each object
    delete p; //delete object
} //destructor

/// Create an object and put a pointer to it on the object list.
/// \param t Sprite type.
/// \param pos Initial position.
/// \param vel Initial velocity.
/// \return Pointer to the object created.

CObject* CObjectManager::create(eSpriteType t, 
  const Vector2& pos, const Vector2& vel)
{
  CObject* p = new CObject(t, pos, vel); 
  m_stdObjectList.push_back(p); 
  return p;
} //create

/// Delete all of the objects managed by the object manager. 
/// This involves deleting all of the CObject instances pointed
/// to by the object list, then clearing the object list itself.

void CObjectManager::clear(){
  for(auto const& p: m_stdObjectList) //for each object
    delete p; //delete object

  m_stdObjectList.clear(); //clear the object list
} //clear

/// Draw the tiled background and all of the objects.

void CObjectManager::draw(){
  for(auto const& p: m_stdObjectList){ //for each object
    m_pRenderer->Draw(*(CSpriteDesc2D*)p);
  } //for
} //draw

/// Move all of the objects and perform 
/// broad phase collision detection and response.

void CObjectManager::move(){
  for(auto const& p: m_stdObjectList) //for each object
    p->move(); //move it

  //now do object-object collision detection and response and
  //remove any dead objects from the object list.

  BroadPhase(); //broad phase collision detection and response
  CullDeadObjects(); //remove dead objects from object list
} //move

/// This is a "bring out yer dead" Monty Python type of thing.
/// Iterate through the objects and check whether their "is dead"
/// flag has been set. If so, then delete its pointer from
/// the object list and destruct the object.

void CObjectManager::CullDeadObjects(){
  for(auto i=m_stdObjectList.begin(); i!=m_stdObjectList.end();){
    if((*i)->IsDead()){ //"He's dead, Dave." --- Holly, Red Dwarf
      delete *i; //delete object
      i = m_stdObjectList.erase(i); //remove from object list and advance to next object
    } //if
      
    else ++i; //advance to next object
  } //for
} //CullDeadObjects

/// Perform collision detection and response for all pairs
/// of objects in the object list, making sure that each
/// pair is processed only once.

void CObjectManager::BroadPhase(){
  for(auto i=m_stdObjectList.begin(); i!=m_stdObjectList.end(); i++){
    CObject* p = *i; //shorthand

    if(p->m_nSpriteIndex == BALL_SPRITE || p->m_nSpriteIndex == PADDLE_SPRITE){
      float w, h; //sprite width and height
      m_pRenderer->GetSize(p->m_nSpriteIndex, w, h);
      Vector2& pos = p->m_vPos; //shorthand
      Vector2& vel = p->m_vVelocity; //shorthand
      
      if(pos.x - w/2 < 0){ //left edge of window
        pos.x = w/2; vel.x = -vel.x;
        p->CreateBoundingShapes();
        m_pAudio->play(BOOP_SOUND);
      } //if

      else if(pos.x + w/2 > m_nWinWidth){ //right edge of window
        pos.x = m_nWinWidth - w/2; vel.x = -vel.x;
        p->CreateBoundingShapes();
        m_pAudio->play(BOOP_SOUND);
      } //else if

      else if(pos.y + h/2 > m_nWinHeight){ //top edge of window
        pos.y = m_nWinHeight - h/2; vel.y = -vel.y;
        p->CreateBoundingShapes();
        m_pAudio->play(BOOP_SOUND);
      } //else if
    } //if

    for(auto j=next(i); j!=m_stdObjectList.end(); j++)
      NarrowPhase(*i, *j);
  } //for
} //BroadPhase

/// Perform collision detection and response for a pair of objects.
/// \param p0 Pointer to the first object.
/// \param p1 Pointer to the second object.

void CObjectManager::NarrowPhase(CObject* p0, CObject* p1){
  if(p1->m_nSpriteIndex == BALL_SPRITE)
    swap(p0, p1);
  
  const int t0 = p0->m_nSpriteIndex;
  const int t1 = p1->m_nSpriteIndex;

  if(!p1->m_bDead && t0 == BALL_SPRITE && 
    (t1 == PADDLE_SPRITE || t1 == BRICK_SPRITE))
  {
    const BoundingSphere& sphere = p0->m_Sphere;
    const BoundingBox& aabb = p1->m_Aabb;

    if(sphere.Intersects(aabb)){
      p0->CollisionResponse(aabb);

      if(t1 == BRICK_SPRITE){
        p1->kill();
        m_pAudio->play(CRUNCH_SOUND);
      } //if

      else if(t1 == PADDLE_SPRITE)
        m_pAudio->play(BEEP_SOUND);
    } //if
  } //if
} //NarrowPhase
