// Inclusions
#include "PhysicsManager.h"
#include "Game.h"
#include "Component.h"
#include "ComponentIncludes.h"
#include "Settings.h"
#include "CustomBind.h"
#include <stdio.h>
#include <vector>
#include "Helpers.h"
#include "Math.h"
#include "Audio.h"

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

/* Note(Ethan) : This is the physics manager, it helps us clean up the game.cpp and further simplify how we use Bullet3.
*
            '   '
            _____

     wachu looking at bud

*/
static PhysicsManager* Self;

// Note(Ethan) : These functions help in the creation of Bullet3 physics objects.
btTransform PhysicsManager::NewTransform(btCollisionShape* shape, Vec3 origin) {
    btTransform startTransform;
    startTransform.setIdentity();
    f32 mass = 0.1f; //NOTE(sean): for things that the player might be able to interact with, we want the mass to be smaller
    f32 friction = 0.5f;
    bool isDynamic = (mass != 0.0f);
    if (isDynamic) {
        shape->calculateLocalInertia(mass, btVector3(0.0f, 0.0f, 0.0f));
    }
    startTransform.setOrigin(origin);
    return startTransform;
}

btRigidBody* PhysicsManager::NewRigidBody(
    btCollisionShape* shape,
    btTransform startTransform,
    f32 mass,
    f32 friction,
    i32 group,
    i32 mask) {

    // Configure and add to Bullet3
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    Vec3 localInertia(Vec3(0, 0, 0));
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    rbInfo.m_friction = friction;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setAngularFactor(Vec3(0., 0., 0.));
    CurrentWorld->addRigidBody(body, group, mask);

    // Add to ECS
    Entity e = Entity();
    body->setUserPointer(&e);

    return body;
}

// Note(Ethan) : Will speed up the creation of commonly used shapes.
btRigidBody* PhysicsManager::CreateSphereObject(
    btScalar radius, 
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btSphereShape(radius);
    CurrentShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}

btRigidBody* PhysicsManager::CreateBoxObject(
    Vec3 size, 
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btBoxShape(size);
    CurrentShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}


btRigidBody* PhysicsManager::CreateCapsuleObject(
    btScalar radius,
    btScalar height,
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btCapsuleShape(radius, height);
    CurrentShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}

btRigidBody* PhysicsManager::CreateConvexObject(f32 mass, f32 friction, i32 group, i32 mask) {
    return 0;
}

// Note(Ethan) : We need this for impact noises and bounce impulses.
void PhysicsManager::PhysicsCollisionCallBack(btDynamicsWorld* p, btScalar t) {
    UNREFERENCED_PARAMETER(t);
    const u32 numManifolds = (u32)p->getDispatcher()->getNumManifolds();

    for every(manifold, numManifolds) {
        btDispatcher* pDispatcher = p->getDispatcher();
        btPersistentManifold* pManifold = pDispatcher->getManifoldByIndexInternal(manifold);

        // Get bodys
        const btRigidBody* pBody0 = btRigidBody::upcast(pManifold->getBody0());
        const btRigidBody* pBody1 = btRigidBody::upcast(pManifold->getBody1());

        // Get entities
        Entity pObj0 = *(Entity*)pBody0->getUserPointer();
        Entity pObj1 = *(Entity*)pBody1->getUserPointer();

        const int numContacts = pManifold->getNumContacts();
        if (Self->CurrentRigidBodies->Get(pObj0) && Self->CurrentRigidBodies->Get(pObj1) && numContacts > 0) { //guard
            for every(contact, numContacts){
                btManifoldPoint& pt = pManifold->getContactPoint(contact);
                Collision newCollisionA;
                newCollisionA.CollisionPos = pt.getPositionWorldOnA();
                Collision newCollisionB;
                newCollisionB.CollisionPos = pt.getPositionWorldOnB();

                Self->CurrentCollisions->AddExisting(pObj0, newCollisionA);
                Self->CurrentCollisions->AddExisting(pObj1, newCollisionB);
            }
        }
    }
}

void PhysicsManager::PhysicsManagerStep() {
    // This plays a sound for all collisions. (NOTE) Ethan : Will be moved into PhysicsManager once I move all physics stepping into a new custom function.
    for every(index, CurrentCollisions->Size()) {
        Entity e = CurrentCollisions->Entities()[index];
        Entity player = CurrentRigidBodies->Entities()[0];
        Vec3 pos = CurrentCollisions->Get(e)->CollisionPos;

        if (e == player) {
        }
        else {
            /*
            printf("Collision detected at : (%f, %f, %f)\n", pos.x, pos.y, pos.z);
            CurrentAudio->play(SoundIndex::Clang, pos, 0.25, 0.0);
            */
        }
    }
    CurrentCollisions->Clear();
}

// Helper Functions, these will be helpful when we don't want to keep passing the dynamics world to managers.
void PhysicsManager::RemoveRigidBody(btRigidBody* body) {
    CurrentWorld->removeRigidBody(body);
}


void PhysicsManager::AddRigidBody(btRigidBody* body, i32 group, i32 mask) {
    CurrentWorld->addRigidBody(body, group, mask);
}


void PhysicsManager::DestroyPhysicsOBject(btCollisionShape* shape) {
    // Nothing for now, figure out a good way to destroy any given Bullet3 objects.
}


void PhysicsManager::InitializePhysics(
    btDiscreteDynamicsWorld** GameWorld,
    btAlignedObjectArray<btCollisionShape*>* GameShapes,
    Table<btRigidBody*>* GameRigidBodies,
    Table<Collision>* GameCollisions,
    Entity* GamePlayer,
    LSound* GameAudio

) {

    // Note(Ethan) : Effectively works the same as before, just call this function and it will handle the initialization.
    CurrentConfiguration = new btDefaultCollisionConfiguration();
    CurrentDispatcher = new btCollisionDispatcher(CurrentConfiguration);
    CurrentBroadphaseCache = new btDbvtBroadphase();
    CurrentSolver = new btSequentialImpulseConstraintSolver;
    *GameWorld = new btDiscreteDynamicsWorld(CurrentDispatcher, CurrentBroadphaseCache, CurrentSolver, CurrentConfiguration);
    *GameShapes = btAlignedObjectArray<btCollisionShape*>();
    CurrentRigidBodies = GameRigidBodies;
    CurrentCollisions = GameCollisions;
    CurrentWorld = *GameWorld;
    CurrentShapes = *GameShapes;
    CurrentWorld->setGravity(btVector3(0.0, -5000.0, 0.0));
    CurrentPlayer = GamePlayer;
    CurrentAudio = GameAudio;

    // Static Self Assignment
    Self = this;

    // Collision Callback
    CurrentWorld->setInternalTickCallback(PhysicsCollisionCallBack);

    // Player Rigidbody | (Note) : Create this first, as the player is currently indexed as [0] in the collision table.
    {
        btRigidBody* rb = CreateCapsuleObject(100.0f, 250.0f, Vec3(0, 1500, 0), 1.0f, 0.5f, 2, 0b00001);
        CurrentRigidBodies->AddExisting(*CurrentPlayer, rb);
    }
}