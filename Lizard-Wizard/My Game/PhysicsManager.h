#ifndef PhysicsManager_H
#define PhysicsManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Settings.h"
#include "CustomBind.h"
#include <vector>
#include <map>
#include "Ecs.h"

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

// Collision
struct Collision {
    Vec3 CollisionPos;
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
    static void PhysicsCollisionCallBack(btDynamicsWorld*, btScalar);
    void PhysicsManagerStep();
    void RemoveRigidBody(btRigidBody*);
    void AddRigidBody(btRigidBody*, i32, i32);
    void DestroyPhysicsOBject(btCollisionShape*);
    void InitializePhysics(btDiscreteDynamicsWorld**, btAlignedObjectArray<btCollisionShape*>*, Table<btRigidBody*>*, Table<Collision>*, Entity*, LSound*);

    btDefaultCollisionConfiguration* CurrentConfiguration;
    btCollisionDispatcher* CurrentDispatcher;
    btDbvtBroadphase* CurrentBroadphaseCache;
    btSequentialImpulseConstraintSolver* CurrentSolver;
    btAlignedObjectArray<btCollisionShape*> CurrentShapes;
    btDiscreteDynamicsWorld* CurrentWorld;
    Table<btRigidBody*>* CurrentRigidBodies;
    Table<Collision>* CurrentCollisions;
    Entity* CurrentPlayer;
    LSound* CurrentAudio;
};


#endif
