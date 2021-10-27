// Inclusions
#include "NPCManager.h"
#include "ProjectileManager.h"
#include "PhysicsManager.h"
#include "Game.h"
#include "Component.h"
#include "Settings.h"
#include "CustomBind.h"
#include <time.h>
#include <stdio.h>
#include <vector>
#include "Helpers.h"
#include "Math.h"

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

/* Note(Ethan) : This is a prototype test for the projectile system, I will reduce comments once we get to making the release version.
*
            '   '
            _____

     wachu looking at bud

*/

// Places a cached NPC.
void NPCManager::PlaceNPC(Entity e, Vec3 origin) {
    CurrentNPCsCache->RemoveTail();
    CurrentNPCsActive->AddExisting(e);
    btRigidBody* body = CurrentNPCs->Get(e)->Body;
    btTransform trans;
    trans.setOrigin(origin);
    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getMotionState()->setWorldTransform(trans);
    body->setWorldTransform(trans);
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    CurrentPhysicsManager->AddRigidBody(body, 2, 0b00001);
    body->activate();
}

// Strips an NPC and re-adds them to the cache.
void NPCManager::StripNPC(Entity e) {
    CurrentNPCsActive->Remove(e);
    CurrentNPCsCache->AddExisting(e);
    btRigidBody* body = CurrentNPCs->Get(e)->Body;


    // Removes rigidbody from world to edit.
    CurrentPhysicsManager->RemoveRigidBody(body);
    body->clearForces();

    btTransform trans;
    trans.setOrigin(Vec3(99999.f, 99999.f, 99999.f));
    f32 mass = 0.0f;
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getMotionState()->setWorldTransform(trans);
    body->setWorldTransform(trans);
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    //CurrentWorld->addRigidBody(projectile);

    // Set light position
    CurrentLights->Get(e)->position = *(Vec4*)&trans.getOrigin();
}


void NPCManager::InitializeNPCs(
    PhysicsManager* GamePhysicsManager,
    ProjectileManager* GameProjectileManager,
    Table<NPC>* GameNPCs,
    Table<Light>* GameLights,
    Group* GameNPCsCache,
    Group* GameNPCsActive
) {

    CurrentPhysicsManager = GamePhysicsManager;
    CurrentProjectileManager = GameProjectileManager;
    CurrentNPCs = GameNPCs;
    CurrentLights = GameLights;
    CurrentNPCsCache = GameNPCsCache;
    CurrentNPCsActive = GameNPCsActive;

    for every(index, NPC_CACHE_SIZE) {
        Entity e = Entity();
        NPC newNPC = NPC();

        newNPC.Body = CurrentPhysicsManager->CreateBoxObject(Vec3(150.f, 150.f, 150.f), Vec3(99999.f, 99999.f, 99999.f), 0.0f, 0.0f, 3, 0b00001);
        CurrentPhysicsManager->RemoveRigidBody(newNPC.Body);

        Light newLight = { Vec4(99999.f,99999.f,99999.f,0), Vec4{10.0f, 30.0f, 500.0f, 0} };
        CurrentLights->AddExisting(e, newLight);

        CurrentNPCs->AddExisting(e, newNPC);
        CurrentNPCsCache->AddExisting(e);
    }
}