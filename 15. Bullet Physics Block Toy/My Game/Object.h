/// \file Object.h
/// \brief Interface for the game object class CObject.

#ifndef __L4RC_GAME_OBJECT_H__
#define __L4RC_GAME_OBJECT_H__

#include "GameDefines.h"
#include "Common.h"
#include "Component.h"
#include "Renderer3D.h"
#include "Descriptors.h"

#include <map>

/// \brief The game object. 
///
/// CObject is the abstract representation of an object.

class CObject:
  public CCommon,
  public LComponent
{
  friend class CRenderer;

  private:
    eObject m_eObjectType = eObject::Unknown; ///< Object type.
    eMesh m_eMeshType = eMesh::Unknown; ///< Mesh type.
    eModel m_eModelType = eModel::Unknown; ///< Model type.
    float m_fScale = 1.0f; ///< Scale (used for models).
    
    UINT m_nShapeIndex = 0; ///< Shape index.
    UINT m_nEffectIndex = 0; ///< Effect index.

    Quaternion m_qOrientation; ///< Initial orientation.

    bool m_bDead = false; ///< Is dead or not.

    btRigidBody* m_pBody = nullptr; ///< Pointer to physics body.
    std::map<CObject*, CContactDesc> m_mapContact[2]; ///< Contact lists.
    UINT m_nContactIndex = 0; ///< Contact index.

    btConvexHullShape* ComputeConvexHull(); ///< Compute convex hull.
    const bool ContactType(CObject*, eObject, eObject) const; ///< Contact type test.
    const bool ContactType(CObject*, eObject) const; ///< Contact type test.

  public:  
    CObject(const eObject, const CInstanceDesc&); ///< Constructor.
    ~CObject(); ///< Destructor.

    void ApplyImpulse(const Vector3&, const Vector3&); ///< Apply impulse.

    void AddContact(CObject*, const CContactDesc&); ///< Add a contact.
    void PlayCollisionSounds(); ///< Play collision sounds.

    void kill(); ///< Kill me.
    const bool IsDead() const; ///< Query whether dead.
    
    const eObject GetObjectType() const; /// Get object type.
    const Vector3 GetPosition() const;///< Get position.
    const Quaternion GetOrientation() const; ///< Get orientation.
}; //CObject

#endif //__L4RC_GAME_OBJECT_H__