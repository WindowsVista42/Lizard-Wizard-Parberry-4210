#ifndef PhysicsManager_H
#define PhysicsManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Settings.h"
#include "CustomBind.h"
#include <vector>
#include "Ecs.h"

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
    btTransform NewTransform(btCollisionShape*, Vec3);
    btRigidBody* NewRigidBody(btCollisionShape*, btTransform, f32, f32, i32, i32);
    btRigidBody* CreateSphereObject(btScalar, Vec3, f32, f32, i32, i32);
    btRigidBody* CreateBoxObject(Vec3, Vec3, f32, f32, i32, i32);
    btRigidBody* CreateCapsuleObject(btScalar, btScalar, Vec3, f32, f32, i32, i32);
    btRigidBody* CreateConvexObject(f32, f32, i32, i32);
    void DestroyPhysicsObject();
    void InitializePhysics(btDiscreteDynamicsWorld**, btAlignedObjectArray<btCollisionShape*>*, Table<btRigidBody*>*);

private:
    btDefaultCollisionConfiguration* CurrentConfiguration;
    btCollisionDispatcher* CurrentDispatcher;
    btDbvtBroadphase* CurrentBroadphaseCache;
    btSequentialImpulseConstraintSolver* CurrentSolver;
    btAlignedObjectArray<btCollisionShape*> CurrentShapes;
    btDiscreteDynamicsWorld* CurrentWorld;
    Table<btRigidBody*>* CurrentRigidBodies;
};


#endif
