// Inclusions
#include "ProjectileManager.h"
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

void ProjectileManager::GenerateSimProjectile(btCollisionObject* caster, const Vec3 startPos, const Vec3 lookDirection, const i32 projectileCount, const f32 projectileVelocity, const f32 projectileAccuracy, const Vec4 projectileColor, const b8 ignoreCaster) {
    /* Note(Ethan) :
       will expand to include :
       model, texture.
    */
    for (i32 i = 0; i < projectileCount; i++) {
        // New Projectile System, uses caching to improve stuff.
        if (CurrentProjectilesCache->Size() < projectileCount - i) {
            return;
        }
        Entity e = CurrentProjectilesCache->RemoveTail();
        CurrentProjectilesActive->AddExisting(e);
        CurrentTimers->AddExisting(e, 2.0f);
        btRigidBody* projectile = *CurrentRigidBodies->Get(e);
        Vec3 newDirection = JitterVec3(lookDirection, -projectileAccuracy, projectileAccuracy);

        // Removes rigidbody from world to edit.
        //CurrentWorld->removeRigidBody(projectile);
        projectile->clearForces();

        Vec3 offset = Vec3(100.0f, 100.0f, 50.0f);

        btTransform trans;
        // UPDATE(sean): changed 300.0f to 100.0f, if you looked down, projectiles were spawning in the ground which would cause crashes on debug builds.
        // Because the projectiles are spawning so close to the camera it looks really jarring, so we're shifting them up 100 units.
        // In the future, we can probably tell the projectiles to spawn from the players wand.
        trans.setOrigin(Vec3(Vec3(startPos + newDirection * 100.0f) + Vec3(0, 100.0f, 0)));
        f32 mass = 0.5f;
        f32 friction = 0.5f;
        btVector3 inertia;

        Vec3 velDirection = JitterVec3(lookDirection, -0.02f, 0.02f);

        // Set attributes.
        projectile->getMotionState()->setWorldTransform(trans);
        projectile->setWorldTransform(trans);
        projectile->getCollisionShape()->calculateLocalInertia(mass, inertia);
        projectile->setMassProps(mass, inertia);
        projectile->setLinearVelocity(Vec3(velDirection * projectileVelocity));
        projectile->setFriction(friction);

        // Re-add regidbody to world after edit.
        CurrentWorld->addRigidBody(projectile, 2, 0b00001);
        projectile->activate();
    }
}

void ProjectileManager::CalculateRay(btCollisionObject* caster, RayProjectile& newRay, Vec3 Pos1, Vec3 btLookDirection, i32 rayBounces, Vec4 color, b8 ignoreCaster) {
    newRay.Pos1 = Pos1;

    btCollisionWorld::ClosestRayResultCallback rayResults(Pos1, Vec3(Pos1 + btLookDirection * 5000.));
    if (ignoreCaster) {
        rayResults.m_collisionFilterGroup = 3;
		rayResults.m_collisionFilterMask = 0b00001;

    }
    CurrentWorld->rayTest(Pos1, Vec3(Pos1 + btLookDirection * 5000.), rayResults);

    if (rayResults.hasHit()) {
        // Note (Ethan) : this is neccesary to get the object being hit, for some reason this pointer is const; this isn't problematic as long as we DO NOT EDIT at this pointer.
        btCollisionObject* hitObject = const_cast<btCollisionObject*>(rayResults.m_collisionObject);
        Vec3 hitPosition = rayResults.m_hitPointWorld;
        f32 dotProduct = Pos1.Dot(Vec3(rayResults.m_hitNormalWorld));
        Vec3 incomingDirection = (hitPosition - Pos1); incomingDirection.Normalize();
        Vec3 reflectedDirection = btLookDirection - 2. * (btLookDirection * rayResults.m_hitNormalWorld) * rayResults.m_hitNormalWorld;
        newRay.Pos2 = Vec3(hitPosition);
        rayBounces = rayBounces - 1;
        if (rayBounces > 0) {
            GenerateRayProjectile(caster, Vec3(hitPosition), Vec3(reflectedDirection), 1, 1, rayBounces, color, true, ignoreCaster);
        }
    } else {
        newRay.Pos2 = Vec3(Pos1 + btLookDirection * 5000.0);
    }
}

void ProjectileManager::GenerateRayProjectile(btCollisionObject* caster, const Vec3 startPos, const Vec3 lookDirection, const i32 rayCount, const i32 rayBounces, const f32 rayAccuracy, const Vec4 rayColor, const b8 recursed, const b8 ignoreCaster) {
    /* Note(Ethan) :
       will expand to include :
       texture.
    */
    RayProjectile newRay;
    newRay.Pos1 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;
    newRay.Pos2 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.;
    newRay.Color = rayColor;

    for (i32 i = 0; i < rayCount; i++) {
        Vec3 newDirection = JitterVec3(lookDirection, -rayAccuracy, rayAccuracy);
        CalculateRay(caster, newRay, startPos, newDirection, rayBounces, Colors::Peru, ignoreCaster);

        CurrentRayProjectiles->push_back(newRay);
    }
}

void ProjectileManager::InitializeProjectiles(
    btAlignedObjectArray<btCollisionShape*> GameSimProjectiles, 
    std::vector<RayProjectile>* GameRayProjectiles,
    btDiscreteDynamicsWorld* GameWorld,
    PhysicsManager* GamePhysicsManager,
    Table<btRigidBody*>* GameRigidBodies,
    Table<Light>* GameLights,
    Table<f32>* GameTimers,
    Group* GameProjectilesCached, 
    Group* GameProjectilesActive
) {

    CurrentSimProjectiles = GameSimProjectiles;
    CurrentRayProjectiles = GameRayProjectiles;
    CurrentPhysicsManager = GamePhysicsManager;
    CurrentRigidBodies = GameRigidBodies;
    CurrentLights = GameLights;
    CurrentTimers = GameTimers;
    CurrentProjectilesCache = GameProjectilesCached;
    CurrentProjectilesActive = GameProjectilesActive;
    CurrentWorld = GameWorld;

    for every(index, PROJECTILE_CACHE_SIZE) {
        Entity e = Entity();
        btRigidBody* newBody = CurrentPhysicsManager->CreateSphereObject(50.f, Vec3(99999.f, 99999.f, 99999.f), 0.0f, 0.0f, 3, 0b00001);
        CurrentWorld->removeRigidBody(newBody);
        Light newLight = { Vec4(99999.f,99999.f,99999.f,0), Vec4{150.0f, 30.0f, 10.0f, 0} };
        CurrentLights->AddExisting(e, newLight);
        CurrentRigidBodies->AddExisting(e, newBody);
        CurrentProjectilesCache->AddExisting(e);
    }
}

void ProjectileManager::StripProjectile(Entity e) {
    CurrentProjectilesActive->Remove(e);
    CurrentTimers->Remove(e);
    CurrentProjectilesCache->AddExisting(e);
    btRigidBody* projectile = *CurrentRigidBodies->Get(e);


    // Removes rigidbody from world to edit.
    CurrentWorld->removeRigidBody(projectile);
    projectile->clearForces();

    btTransform trans;
    trans.setOrigin(Vec3(99999.f, 99999.f, 99999.f));
    f32 mass = 0.0f;
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    projectile->getMotionState()->setWorldTransform(trans);
    projectile->setWorldTransform(trans);
    projectile->getCollisionShape()->calculateLocalInertia(mass, inertia);
    projectile->setMassProps(mass, inertia);
    projectile->setFriction(friction);

    // Re-add regidbody to world after edit.
    //CurrentWorld->addRigidBody(projectile);

    // Set light position
    CurrentLights->Get(e)->position = *(Vec4*)&trans.getOrigin();
}

