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

void ProjectileManager::GenerateRayProjectile(Vector3 startPos, Vector3 lookDirection, i32 rayCount, i32 rayAccuracy, XMVECTORF32 rayColor) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, texture.
    */

    f32 negativeAccuracy = rayAccuracy * -1;
    f32 range = (rayAccuracy - negativeAccuracy + 1);
    for (i32 i = 0; i < rayCount; i++) {
        srand(randomSeed);
        randomSeed++;
        if (randomSeed > 999) {
            randomSeed = 0;
        }
        lookDirection = Vector3(lookDirection.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.), lookDirection.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.));
        btScalar param(0.5);
        RayProjectile newRay = RayProjectile();
        newRay.Pos1 = startPos;
        newRay.Pos2 = lookDirection;
        newRay.Color = rayColor;
        //currentWorld->getDebugDrawer()->drawLine(*(btVector3*)&startPos, *(btVector3*)&lookDirection, btVector4(0, 0, 0, 1));
        btCollisionWorld::AllHitsRayResultCallback allResults(*(btVector3*)&startPos, *(btVector3*)&lookDirection);
        //btCollisionWorld::ClosestRayResultCallback closestResults(*(btVector3*)&startPos, *(btVector3*)&lookDirection);
        //closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
        allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
        allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
        currentWorld->rayTest(btVector3(lookDirection.x * 300, lookDirection.y * 300, lookDirection.z * 300), btVector3(lookDirection.x * 50000, lookDirection.y * 50000, lookDirection.z * 50000), allResults);

        for (int i = 0; i < allResults.m_hitFractions.size(); i++)
        {
            btVector3 hitPosition = allResults.m_hitPointWorld[i];
            btScalar dotProduct = allResults.m_hitPointWorld[i].dot(allResults.m_hitNormalWorld[i]);
            //XMVECTOR Result = XMVector3Dot(Incident, Normal);
            // Result = XMVectorAdd(Result, Result);
            //Result = XMVectorNegativeMultiplySubtract(Result, Normal, Incident);
        }

        /*
        if (closestResults.hasHit())
        {
            btVector3 hitPosition = closestResults.m_hitNormalWorld;
            printf("Target hit at (%f, %f, %f)", hitPosition.getX(), hitPosition.getY(), hitPosition.getZ());
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
            startTransform.setOrigin(*(btVector3*)&hitPosition);
        }
        */
        currentRayProjectiles->push_back(newRay);
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

