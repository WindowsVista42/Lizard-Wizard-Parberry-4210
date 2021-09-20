/// \file Common.cpp
/// \brief Code for the class CCommon.
///
/// This file contains declarations and initial values
/// for CCommon's static member variables.

#include "Common.h"

CRenderer* CCommon::m_pRenderer = nullptr;
LBaseCamera* CCommon::m_pCamera = nullptr;
CObjectManager* CCommon::m_pObjectManager= nullptr; 

btDiscreteDynamicsWorld* CCommon::m_pPhysicsWorld = nullptr;

btDefaultCollisionConfiguration* CCommon::m_pConfig = nullptr;
btCollisionDispatcher* CCommon::m_pDispatcher = nullptr;
btBroadphaseInterface* CCommon::m_pBroadphase = nullptr;
btSequentialImpulseConstraintSolver* CCommon::m_pSolver = nullptr;

btAlignedObjectArray<btCollisionShape*> CCommon::m_btCollisionShapes;

bool CCommon::m_bCollisionSoundsMuted = false; 