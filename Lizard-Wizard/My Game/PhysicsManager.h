#ifndef PhysicsManager_H
#define PhysicsManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include "CustomBind.h"
#include <vector>

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

struct ProjectileCollisionFilter : public btOverlapFilterCallback
{
    // return true when pairs need collision.
    virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
    {
        bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
        collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

        return collides;
    }
};

// Physics Class
class PhysicsManager {

public:
    void CreatePhysicsObject();
    void DestroyPhysicsObject();
    void InitializePhysics(btDiscreteDynamicsWorld**, btAlignedObjectArray<btCollisionShape*>*);

private:
    btDefaultCollisionConfiguration* CurrentConfiguration;
    btCollisionDispatcher* CurrentDispatcher;
    btDbvtBroadphase* CurrentBroadphaseCache;
    btSequentialImpulseConstraintSolver* CurrentSolver;
    btAlignedObjectArray<btCollisionShape*> CurrentShapes;
    btDiscreteDynamicsWorld* CurrentWorld;
};


#endif
