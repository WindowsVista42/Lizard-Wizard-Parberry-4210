/// \file Common.h
/// \brief Interface for the class CCommon.

#ifndef __L4RC_GAME_COMMON_H__
#define __L4RC_GAME_COMMON_H__

#include "btBulletDynamicsCommon.h" 
#include "GameDefines.h"

//forward declarations
 
class CRenderer;
class LBaseCamera;
class CObjectManager;

class btDiscreteDynamicsWorld;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btCollisionShape;

/// \brief The common variables class.
///
/// CCommon is a singleton class that encapsulates things that are common to
/// different game components, including game state variables. Making it a
/// singleton class means that we can avoid passing its member variables
/// around as parameters, which makes the code minisculely faster, and more
/// importantly, reduces function clutter.

class CCommon{
  protected:  
    const float m_fFloorHt = 0.5f; ///< Floor height.
    const Vector3 m_vPlanePos = Vector3(0.0f, 0.0f, 100.0f); ///< Plane position.
    const Vector3 m_vPlaneSize = Vector3(600.0f, 2.0f*m_fFloorHt, 600.0f); ///< Plane size.

    const float m_fMoonRadius = 25.0f; ///< Moon radius.
    const float m_fBallRadius = 2.0f; ///< Ball radius.
    
    const Vector3 m_vBoxSize = Vector3(9.0f, 6.0f, 6.0f); ///< Box size.  
    const float m_fTetrahedronSize = 16.0f; ///< Tetrahedron size.
    const float m_fIcosahedronSize = 16.0f; ///< Icosahedron size.
    const float m_fDodecahedronSize = 16.0f; ///< Dodecahedron size.
    const float m_fTeapotSize = 8.0f; ///< Teapot size.

    ////////////////////////////////////////////////////////////////////////////

    static CRenderer* m_pRenderer; ///< Pointer to the renderer.
    static LBaseCamera* m_pCamera; ///< Pointer to the camera.
    static CObjectManager* m_pObjectManager; ///< Pointer to object manager.

    static btDiscreteDynamicsWorld* m_pPhysicsWorld; ///< Pointer to physics engine.

    static btDefaultCollisionConfiguration* m_pConfig; ///< Bullet physics configuration.
    static btCollisionDispatcher* m_pDispatcher; ///< Bullet physics dispatcher.
    static btBroadphaseInterface* m_pBroadphase; ///< Bullet physics broadphase.
    static btSequentialImpulseConstraintSolver* m_pSolver;///< Bullet physics constraint solver.

    static btAlignedObjectArray<btCollisionShape*> m_btCollisionShapes; ///< Collision shapes.

    static bool m_bCollisionSoundsMuted; ///< Mute collision sounds.
}; //CCommon

#endif //__L4RC_GAME_COMMON_H__
