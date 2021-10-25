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

        // PUSH TO COLLISION ARRAY
        btCollisionShape* projectile = new btSphereShape(btScalar(50.f));
        currentSimProjectiles.push_back(projectile);
        Vec3 newDirection = JitterVec3(lookDirection, -0.3, 0.3);

        // CREATE DYNAMIC OBJECT
        btTransform startTransform;
        startTransform.setIdentity();
        f32 mass = 0.1f; //NOTE(sean): for things that the player might be able to interact with, we want the mass to be smaller
        f32 friction = 0.5f;
        bool isDynamic = (mass != 0.0f);
        Vec3 localInertia(lookDirection * 500.0f);
        if (isDynamic) {
            projectile->calculateLocalInertia(mass, btVector3(0.0f, 0.0f, 0.0f));
        }
        Vec3 projectilePos = Vec3(startPos + newDirection * 250.0f);
        startTransform.setOrigin(projectilePos);

        // MOTIONSTATE AND RIGIDBODY
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, projectile, localInertia);
        rbInfo.m_friction = friction;
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setAngularFactor(Vec3(0., 0., 0.));
        f32 newVelocity = projectileVelocity * 15000.0f;
        body->applyForce(Vec3(lookDirection * newVelocity), startPos);
        body->setIgnoreCollisionCheck(caster, ignoreCaster);
        currentWorld->addRigidBody(body, 3, 31);
    }
}

void ProjectileManager::CalculateRay(btCollisionObject* caster, RayProjectile& newRay, Vec3 Pos1, Vec3 btLookDirection, i32 rayBounces, Vec4 color, b8 ignoreCaster) {
    newRay.Pos1 = Pos1;

    btCollisionWorld::ClosestRayResultCallback rayResults(Pos1, Vec3(Pos1 + btLookDirection * 5000.));
    if (ignoreCaster) {
        rayResults.m_collisionFilterGroup = 3;
        rayResults.m_collisionFilterMask = 31;

    }
    currentWorld->rayTest(Pos1, Vec3(Pos1 + btLookDirection * 5000.), rayResults);

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
        Vec3 newDirection = JitterVec3(lookDirection, -0.03, 0.03);
        CalculateRay(caster, newRay, startPos, newDirection, rayBounces, Colors::Peru, ignoreCaster);

        currentRayProjectiles->push_back(newRay);
    }
}

void ProjectileManager::InitializeProjectiles(btAlignedObjectArray<btCollisionShape*> gameSimProjectiles, std::vector<RayProjectile>* gameRayProjectiles, btDiscreteDynamicsWorld* gameWorld) {
    currentSimProjectiles = gameSimProjectiles;
    currentRayProjectiles = gameRayProjectiles;
    currentWorld = gameWorld;
}

void ProjectileManager::DestroyAllProjectiles() {

}

