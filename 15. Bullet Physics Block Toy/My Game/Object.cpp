/// \file Object.cpp
/// \brief Code for the game object class CObject.

#include "Object.h"
#include "ComponentIncludes.h"
#include "Renderer.h"

///////////////////////////////////////////////////////////////////////////////
// Constructors and destructors.

/// Constructor.  Make an instance of a geometric primitive object with a 
/// shape for collision detection and response. Add a rigid body to the physics
/// world with user pointer pointing to `this` so that we can later map
/// collisions between physics bodies to the corresponding objects.
/// \param t Object type.
/// \param d Instance descriptor.

CObject::CObject(const eObject t, const CInstanceDesc& d):
  m_eObjectType(t)
{
  m_eMeshType = d.m_eMeshType;
  m_eModelType = d.m_eModelType;
  m_fScale = d.m_fScale;

  //physics body
  
  btCollisionShape* pShape = nullptr; //for collision shape pointer
  
  switch(m_eMeshType){
    case eMesh::Model:
      pShape = new btBoxShape(RW2PW(m_fScale*m_pRenderer->GetExtents(m_eModelType)));
      m_btCollisionShapes.push_back(pShape);
      break;

    case eMesh::BigSphere:
    case eMesh::Sphere:
      pShape = new btSphereShape(RW2PW(d.m_fScale));
      m_btCollisionShapes.push_back(pShape);
      break;
    
    case eMesh::Box:
      pShape = new btBoxShape(RW2PW(0.5f*d.m_vExtents));
      m_btCollisionShapes.push_back(pShape);
      break;

    default: pShape = ComputeConvexHull();
  } //switch
    
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(RW2PW(d.m_vPos));
  transform.setRotation(RW2PW(d.m_qOrientation));

  btVector3 inertia(0, 0, 0); //inertia

  if(d.m_fMass != 0.0f)//dynamic if and only if mass is non zero, otherwise static
    pShape->calculateLocalInertia(d.m_fMass, inertia);

  //motion state

  btDefaultMotionState* ms = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(d.m_fMass, ms, pShape, inertia);
  m_pBody = new btRigidBody(rbInfo);
  m_pBody->setRestitution(d.m_fRestitution);
  m_pBody->setFriction(d.m_fFriction);

  m_pBody->setUserPointer((void*)this); //give the physics body an object pointer

  m_pPhysicsWorld->addRigidBody(m_pBody); 
} //constructor

/// Destructor. This is mainly just cleaning up after Bullet Physics by
/// deleting the physics body, motion state, and collision object.

CObject::~CObject(){
  if(m_pBody != nullptr){
    if(m_pBody->getMotionState())
      delete m_pBody->getMotionState();
    m_pPhysicsWorld->removeCollisionObject(m_pBody);
    delete m_pBody;
  } //if
} //destructor

///////////////////////////////////////////////////////////////////////////////
// General functions.

/// Compute the convex hull for a primitive of type m_eMeshType. A convex hull
/// is a triangle mesh that fits closely to the original mesh but is convex.
/// Bullet Physics has the ability to compute a convex hull from the vertices
/// of a mesh and use it as a collision shape.
/// Append a pointer to it to m_btCollisionShapes.
/// \return Pointer to the convex hull created.

btConvexHullShape* CObject::ComputeConvexHull(){
  btConvexHullShape* pbtConvexHull = new btConvexHullShape;

  CMeshDesc* pDesc = m_pRenderer->GetPrimitiveDesc(m_eMeshType); //ptr to mesh descriptor
  const auto& vb = pDesc->m_vecVertexBuffer; //shorthand for vertex buffer
  const auto& xb = pDesc->m_vecIndexBuffer; //shorthand for index buffer
  const size_t nIndices = xb.size(); //number of vertex indices

  for(size_t i=0; i<nIndices; i+=3) //for each triangle
    for(size_t j=0; j<3; j++) //for each point in the triangle
      pbtConvexHull->addPoint(RW2PW(vb[xb[i + j]].position)); //add to convex hull
  
  m_btCollisionShapes.push_back(pbtConvexHull);
  return pbtConvexHull;
} //ComputeConvexHull

/// Apply an impulse through a point.
/// \param force Direction and magnitude of impulse.
/// \param pos Point through which the impulse is applied.

void CObject::ApplyImpulse(const Vector3& force, const Vector3& pos){
  if(m_pBody)
    m_pBody->applyImpulse(RW2PW(force), RW2PW(pos));
} //ApplyImpulse

/// Determine whether this object and another object have the right types, 
/// ignoring the order of the type parameters.
/// \param p Pointer to an object.
/// \param t0 First object type.
/// \param t1 Second object type.
/// \return true if this object and *p have types t0 and t1 in any order.

const bool CObject::ContactType(CObject* p, eObject t0, eObject t1) const{
  if(p == nullptr)return false; //bail and fail
  return (m_eObjectType == t0 && p->GetObjectType() == t1) || //forwards or
    (m_eObjectType == t1 && p->GetObjectType() == t0); //backwards
} //ContactType

/// Determine whether this object and another object have the right types, 
/// ignoring the order of the type parameters.
/// \param p Pointer to an object.
/// \param t Object type.
/// \return true if this object or *p has types t.

const bool CObject::ContactType(CObject* p, eObject t) const{
  if(p == nullptr)return false; //bail and fail
  return m_eObjectType == t || p->GetObjectType() == t;
} //ContactType

///////////////////////////////////////////////////////////////////////////////
// Reader functions

/// Get the position of the object center. If it has a physics body, then get
/// it from the physics engine, otherwise return the zero vector.
/// \return The position of the object center.

const Vector3 CObject::GetPosition() const{
  if(!m_pBody || !m_pBody->getMotionState())
    return Vector3::Zero;

  btTransform trans;
  m_pBody->getMotionState()->getWorldTransform(trans);
  return PW2RW(trans.getOrigin());
} //GetPosition

/// Get the object orientation. If it has a physics body, then get it from the
/// physics engine, otherwise from m_qOrientation.
/// \return The object orientation quaternion.

const Quaternion CObject::GetOrientation() const{
  if(!m_pBody || !m_pBody->getMotionState())
    return m_qOrientation;

  btTransform trans;
  m_pBody->getMotionState()->getWorldTransform(trans);
  return PW2RW(trans.getRotation());
} //GetOrientation

/// Reader function for the object type.

const eObject CObject::GetObjectType() const{
  return m_eObjectType;
} //GetObjectType

//////////////////////////////////////////////////////////////////////////////
// Object death.

/// Kill an object by marking its "is dead" flag. The object
/// will get deleted later at the appropriate time.

void CObject::kill(){
  m_bDead = true;
} //kill

/// Reader function for the "is dead" flag.
/// \return true if marked as being dead, that is, ready for disposal.

const bool CObject::IsDead() const{
  return m_bDead;
} //IsDead

//////////////////////////////////////////////////////////////////////////////
// Contact processing.

/// Add a contact to the current contact map. This function is called by a 
/// callback function `myTickCallback()`, which is called from Bullet Physics
/// whenever a contact between two bodies is detected.
/// \param pObj Pointer to object in contact.
/// \param d Contact descriptor for the contact.

void CObject::AddContact(CObject* pObj, const CContactDesc& d){
  m_mapContact[m_nContactIndex].insert(std::pair<CObject*, CContactDesc>(pObj, d));
} //AddContact

/// Play collision sounds, which are managed as follows. There are two contact
/// maps, `m_mapContact[0]` and `m_mapContact[1]`, which map a pointer to the
/// object that this object is in contact with to a contact descriptor for
/// the contact. The current contact map is `m_mapContact[m_nContactIndex]`,
/// where `m_nContactIndex` flips from zero to one each time this function
/// is called. The other contact map `m_mapContact[m_nContactIndex^1]` is the
/// contact map from the previous frame. Entries are place in the current
/// contact map by a callback function `myTickCallback()` calling function
/// `AddContact()` above. This function is then called at the end of the frame
/// to play the appropriate sound when either (1) there is an entry in the 
/// current contact map for an object that does not have an entry in the 
/// previous contact map, or, (2) the entry for an object in the current contact
/// map has more points of contact than the one in the previous contact map.

void CObject::PlayCollisionSounds(){
  std::map<CObject*, CContactDesc>& mapCurr = m_mapContact[m_nContactIndex];
  std::map<CObject*, CContactDesc>& mapPrev = m_mapContact[m_nContactIndex^1];

  if(!m_bCollisionSoundsMuted)
    for(const auto& contactCurr: mapCurr){
      CObject* pObj = contactCurr.first; //pointer to currently contacted object
      const CContactDesc dCurr = contactCurr.second; //current contact descriptor

      auto contactPrev = mapPrev.find(pObj); //previous contact, if any
      CContactDesc dPrev; //for previous contact descriptor
      if(contactPrev != mapPrev.end()) //if there was a previous contact with this object
        dPrev = contactPrev->second; //initialize previous contact descriptor

      //if we have a new contact or one with more contact points than previously,
      //then play the appropriate sound for the object types.

      if(contactPrev == mapPrev.end() || dCurr.m_nNumContacts > dPrev.m_nNumContacts){
        const float volume = 0.05f*dCurr.m_fImpulse;
        const Vector3 vPos = GetPosition();
      
        if(volume > 0.1f){
          if(ContactType(pObj, eObject::Ball, eObject::Moon))
            m_pAudio->play((int)eSound::ThumpHard, vPos, volume);  

          else if(ContactType(pObj, eObject::Ball, eObject::Box))
            m_pAudio->play((int)eSound::TapHard, vPos, volume);  
          
          else if(ContactType(pObj, eObject::Ball, eObject::Container))
            m_pAudio->play((int)eSound::Clang, vPos, volume); 

          else if(ContactType(pObj, eObject::Ball, eObject::Teapot))
            m_pAudio->play((int)eSound::TapLight, vPos, volume, 0.5f);    

          else if(ContactType(pObj, eObject::Ball, eObject::Floor))
            m_pAudio->play((int)eSound::ThumpMedium, vPos, 0.5f*volume, -0.1f); 
      
          else if(ContactType(pObj, eObject::Floor, eObject::Box))
            m_pAudio->play((int)eSound::TapHard, vPos, volume, 0.2f);  
      
          else if(ContactType(pObj, eObject::Floor, eObject::Container))
            m_pAudio->play((int)eSound::Clang, vPos, volume, -0.2f);    

          else if(ContactType(pObj, eObject::Ball) && m_eObjectType != eObject::Ball)
            m_pAudio->play((int)eSound::ThumpLight, vPos, volume, -0.5f);  
        } //if
      } //if
    } //for

  m_nContactIndex = m_nContactIndex^1; //flip contact maps, current with old
  m_mapContact[m_nContactIndex].clear(); //clear current contact map
} //PlayCollisionSounds

