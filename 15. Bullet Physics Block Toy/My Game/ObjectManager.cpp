/// \file ObjectManager.cpp
/// \brief Code for the object manager class CObjectManager.

#include "ObjectManager.h"
#include "ComponentIncludes.h"
#include "ParticleEngine.h"
#include "Renderer.h"

/// Default constructor.

CObjectManager::CObjectManager(){
} //constructor

/// Destruct all of the objects in the object list.

CObjectManager::~CObjectManager(){
  for(auto const& p: m_stdObjectList) //for each object
    delete p; //delete object
} //destructor

/// Create an object instance and insert it into the object list.
/// \param obj Object type.
/// \param d Mesh descriptor.
/// \return Pointer to the object created.

CObject* CObjectManager::Create(eObject obj, const CInstanceDesc& d){
  CObject* pObj = new CObject(obj, d);
  m_stdObjectList.push_back(pObj); 
  return pObj;
} //Create

/// Delete all of the objects managed by the object manager. This involves
/// deleting all of the CObject instances pointed to by the object list, then
/// clearing the object list.

void CObjectManager::Clear(){
  for(auto& p: m_stdObjectList) //for each object
    delete p; //delete object

  m_stdObjectList.clear(); //clear the object list
} //clear

/// Draw all objects.

void CObjectManager::Draw(){
  for(auto const& p: m_stdObjectList) //for each object
    m_pRenderer->Render(p);
} //Draw

/// Update all objects. This function has two tasks, playing collision sounds
/// and deleting delete all objects that have fallen off the plane and dropped
/// far enough.

void CObjectManager::Update(){
  for(auto& p: m_stdObjectList){ //for each object
    p->PlayCollisionSounds();
    if(p->GetPosition().y < -500.0f) //fell off the ground
      p->kill(); //die
  } //for
  
  CullDeadObjects();
} //Update

/// Iterate through the objects and check whether their "is dead"
/// flag has been set. If so, then delete its pointer from
/// the object list and destruct the object.

void CObjectManager::CullDeadObjects(){
  for(auto i=m_stdObjectList.begin(); i!=m_stdObjectList.end();){
    if((*i)->IsDead()){ //"He's dead, Dave." --- Holly, Red Dwarf
      delete *i; //delete object
      i = m_stdObjectList.erase(i); //remove from object list and advance
    } //if
      
    else ++i; //advance to next object
  } //for
} //CullDeadObjects
