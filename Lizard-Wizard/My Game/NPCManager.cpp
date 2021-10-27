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

/* Note(Ethan) : This is a prototype test for the NPC system, I will reduce comments once we get to making the release version.
*
            '   '
            _____

     wachu looking at bud

*/

btQuaternion LookAt(Vec3 origin, Vec3 lookAt) {
    Vec3 forwardVector = XMVector3Normalize(origin - lookAt);
    Vec3 rotAxis = XMVector3Cross(Vec3(1.0f, 0, 0), forwardVector);
    float dot = forwardVector.Dot(Vec3(1.0f, 0, 0));

    btQuaternion q;
    q.setX(rotAxis.x);
    q.setY(rotAxis.y);
    q.setZ(rotAxis.z);
    q.setW(dot + 1);

    return q.normalize();
}

// Contains most of the logical code for handling NPCs
void NPCManager::DirectNPC(Entity e, btRigidBody* player) {
    btRigidBody* body = CurrentNPCs->Get(e)->Body;
    Vec3 origin = body->getWorldTransform().getOrigin();
    Vec3 lookAt = player->getWorldTransform().getOrigin() + player->getLinearVelocity() / 4;
    btQuaternion newAngles = LookAt(origin, lookAt);
    btTransform newTransform;
    f32 waitTimer;
    switch (CurrentNPCs->Get(e)->Behavior) {
        case NPCBehavior::MELEE :
            printf("Melee Behavior\n");
            break;
        case NPCBehavior::RANGED :
            printf("Ranged Behavior\n");
            break;
        case NPCBehavior::TURRET :
            newTransform.setOrigin(body->getWorldTransform().getOrigin());
            newTransform.setRotation(newAngles);
            body->getMotionState()->setWorldTransform(newTransform);
            body->setWorldTransform(newTransform);
            waitTimer = *CurrentTimers->Get(e);
            if (waitTimer < 0.0f) {
                CurrentTimers->Remove(e);
                CurrentTimers->AddExisting(e, 3.0);
                CurrentProjectileManager->GenerateSimProjectile(
                    body, 
                    body->getWorldTransform().getOrigin(), 
                    -XMVector3Normalize(origin - lookAt),
                    1,
                    20000.0, 
                    0.05, 
                    Colors::LightGreen, 
                    true
                );
            }

            break;
        default :
        return;
    }
}

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
    CurrentTimers->AddExisting(e, 10.0f);
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
    Table<f32>* GameTimers,
    Group* GameNPCsCache,
    Group* GameNPCsActive
) {

    CurrentPhysicsManager = GamePhysicsManager;
    CurrentProjectileManager = GameProjectileManager;
    CurrentNPCs = GameNPCs;
    CurrentLights = GameLights;
    CurrentTimers = GameTimers;
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