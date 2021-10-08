// Inclusions
#include "ProjectileManager.h"
#include "Game.h"
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include "CustomBind.h"
#include <time.h>
#include <stdio.h>
#include <vector>

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

void ProjectileManager::GenerateSimProjectile(Vector3 startPos, Vector3 lookDirection, i32 projectileVelocity, i32 projectileCount, i32 projectileAccuracy, XMVECTORF32 projectileColor) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), requested velocity, color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, model, texture, and color.
    */

    // Note(Ethan) : We need the negative range or else projectile spread will be goofy looking.
    f32 negativeAccuracy = projectileAccuracy * -1;
    f32 range = (projectileAccuracy - negativeAccuracy + 1);

    // Note(Ethan) : This will spawn n-projectiles based on projectileCount, it gets really laggy though. Projectile cache will help
    // fix this lag, but it may not be enough. Find a way to optimize the rendering and physics pipeline somehow.
    for (i32 i = 0; i < projectileCount; i++) {
        // Note(Ethan) : This randomizes the spread of all projectiles, it looks really ugly, I bet there is a better way to do this.
        srand(randomSeed);
        randomSeed++;
        if (randomSeed > 999) {
            randomSeed = 0;
        }
        /*
        f32 xSpread = ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.);
        f32 ySpread = (negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.;
        f32 zSpread = (negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.;
        */
        //printf("Lookvector = (%f, %f, %f)\n", lookDirection.x, lookDirection.y, lookDirection.z);
        lookDirection = Vector3(lookDirection.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.));
        //printf("Lookvector = (%f, %f, %f)\n", lookDirection.x, lookDirection.y, lookDirection.z);

        // Push back a collision shape into the array.
        btCollisionShape* projectile = new btSphereShape(btScalar(50.));
        currentSimProjectiles.push_back(projectile);

        // Implicating projectile as a dynamic object.
        btTransform startTransform;
        startTransform.setIdentity();
        btScalar mass(0.1); //NOTE(sean): for things that the player might be able to interact with, we want the mass to be smaller
        btScalar friction(0.5);
        bool isDynamic = (mass != 0.f);
        btVector3 localInertia(lookDirection.x * 500., lookDirection.y * 500., lookDirection.z * 500.);
        if (isDynamic) {
            projectile->calculateLocalInertia(mass, localInertia);
        }
        startTransform.setOrigin(*(btVector3*)&startPos + btVector3(lookDirection.x * 250., lookDirection.y * 250., lookDirection.z * 250.));

        // std::cout << "{"  << lookdir.x << ", " << lookdir.y << ", " << lookdir.z << "}" << std::endl;
        // Motionstate again.
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, projectile, localInertia);
        rbInfo.m_friction = friction;
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setAngularFactor(btVector3(0., 0., 0.));
        currentWorld->addRigidBody(body);
        body->applyForce(btVector3((projectileVelocity * 25000.) * lookDirection.x, (projectileVelocity * 25000.) * lookDirection.y, (projectileVelocity * 25000.) * lookDirection.z), *(btVector3*)&startPos);
    }
}

void ProjectileManager::GenerateRayProjectile(Vector3 startPos, Vector3 lookDirection, i32 rayCount, i32 rayAccuracy, i32 rayBounces, XMVECTORF32 rayColor, b8 recursed) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, texture.
    */

    f32 negativeAccuracy = rayAccuracy * -1;
    f32 range = (rayAccuracy - negativeAccuracy + 1);
    RayProjectile newRay = RayProjectile();


    newRay.Pos1 = Vector3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;
    newRay.Pos2 = Vector3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.;


    newRay.Color = rayColor;
    if (recursed == true) {
        newRay.Pos1 = Vector3(startPos.x, startPos.y, startPos.z) + Vector3(lookDirection.x, lookDirection.y, lookDirection.z);
        btCollisionWorld::ClosestRayResultCallback rayResults(*(btVector3*)&startPos + *(btVector3*)&lookDirection * 500., *(btVector3*)&startPos + *(btVector3*)&lookDirection * 5000.);
        currentWorld->rayTest(*(btVector3*)&startPos + *(btVector3*)&lookDirection * 500., *(btVector3*)&startPos + *(btVector3*)&lookDirection * 5000., rayResults);
        if (rayResults.hasHit()) {
            printf("Ray hit at (%f, %f, %f)\n", rayResults.m_hitPointWorld.getX(), rayResults.m_hitPointWorld.getY(), rayResults.m_hitPointWorld.getZ());
            btVector3 hitPosition = rayResults.m_hitPointWorld;
            btScalar dotProduct = (*(btVector3*)&startPos).dot(rayResults.m_hitNormalWorld);
            btVector3 incomingDirection = btVector3(hitPosition - *(btVector3*)&startPos).normalize();
            btVector3 reflectedDirection = *(btVector3*)&lookDirection - 2. * (*(btVector3*)&lookDirection * rayResults.m_hitNormalWorld) * rayResults.m_hitNormalWorld;
            if (rayBounces > 0) {
                GenerateRayProjectile(*(Vector3*)&hitPosition, *(Vector3*)&reflectedDirection, 1, 1, rayBounces - 1, Colors::PeachPuff, true);
            }
            newRay.Pos2 = *(Vector3*)&hitPosition;
        }
        currentRayProjectiles->push_back(newRay);
    } else {
        for (i32 i = 0; i < rayCount; i++) {
            srand(randomSeed);
            randomSeed++;
            if (randomSeed > 999) {
                randomSeed = 0;
            }
            lookDirection = Vector3(lookDirection.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.));
            printf("Lookdirection at (%f, %f, %f)\n", lookDirection.x, lookDirection.y, lookDirection.z);
            newRay.Pos1 = Vector3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;

            btCollisionWorld::ClosestRayResultCallback rayResults(*(btVector3*)&startPos + *(btVector3*)&lookDirection * 500., *(btVector3*)&startPos + *(btVector3*)&lookDirection * 5000.);
            currentWorld->rayTest(*(btVector3*)&startPos + *(btVector3*)&lookDirection * 500., *(btVector3*)&startPos + *(btVector3*)&lookDirection * 5000., rayResults);
            if (rayResults.hasHit()) {
                printf("Ray hit at (%f, %f, %f)\n", rayResults.m_hitPointWorld.getX(), rayResults.m_hitPointWorld.getY(), rayResults.m_hitPointWorld.getZ());
                btVector3 hitPosition = rayResults.m_hitPointWorld;
                btScalar dotProduct = (*(btVector3*)&startPos).dot(rayResults.m_hitNormalWorld);
                btVector3 incomingDirection = btVector3(hitPosition - *(btVector3*)&startPos).normalize();
                btVector3 reflectedDirection = *(btVector3*)&lookDirection - 2. * (*(btVector3*)&lookDirection * rayResults.m_hitNormalWorld) * rayResults.m_hitNormalWorld;
                if (rayBounces > 0) {
                    GenerateRayProjectile(*(Vector3*)&hitPosition, *(Vector3*)&reflectedDirection, 1, 1, rayBounces - 1, Colors::LightPink, true);
                }
                newRay.Pos2 = *(Vector3*)&hitPosition;
            }
            currentRayProjectiles->push_back(newRay);
        }
    }
}

void ProjectileManager::InitializeProjectiles(btAlignedObjectArray<btCollisionShape*> gameSimProjectiles, std::vector<RayProjectile>* gameRayProjectiles, btDiscreteDynamicsWorld* gameWorld) {
    // Note(Ethan) : this simply connects the ProjectileManager.cpp storage arrays to the Game.cpp storage arrays.
    /*
    this->currentSimProjectiles;
    this->currentRayProjectiles;
    this->currentWorld;
    */
    currentSimProjectiles = gameSimProjectiles;
    currentRayProjectiles = gameRayProjectiles;
    currentWorld = gameWorld;

}

void ProjectileManager::DestroyAllProjectiles() {
    // Note(Ethan) : might be extraneous, but we could use this to wipe an array of projectiles whenever needed.


}

