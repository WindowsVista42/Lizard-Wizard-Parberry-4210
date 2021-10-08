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

Vector3 JitterVector3(Vector3 input, f32 negativeAccuracy, f32 range) {
    return Vector3(
        input.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.)
    );
}

Vector3 into(btVector3 input) {
    return *(Vector3*)&input;
}

btVector3 into(Vector3 input) {
    return *(btVector3*)&input;
}

void ProjectileManager::GenerateSimProjectile(Vector3 startPos, Vector3 lookDirection, f32 projectileVelocity, i32 projectileCount, i32 projectileAccuracy, XMVECTORF32 projectileColor) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), requested velocity, color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, model, texture, and color.
    */

    // Note(Ethan) : We need the negative range or else projectile spread will be goofy looking.
    f32 negativeAccuracy = -projectileAccuracy;
    f32 range = (projectileAccuracy - negativeAccuracy + 1.0f);

    // Note(Ethan) : This will spawn n-projectiles based on projectileCount, it gets really laggy though. Projectile cache will help
    // fix this lag, but it may not be enough. Find a way to optimize the rendering and physics pipeline somehow.
    for (i32 i = 0; i < projectileCount; i++) {
        // Note(Ethan) : This randomizes the spread of all projectiles, it looks really ugly, I bet there is a better way to do this.
        srand(randomSeed);
        randomSeed++;
        if (randomSeed > 999) {
            randomSeed = 0;
        }

        lookDirection = JitterVector3(lookDirection, negativeAccuracy, range);

        // Push back a collision shape into the array.
        btCollisionShape* projectile = new btSphereShape(btScalar(50.f));
        currentSimProjectiles.push_back(projectile);

        // Implicating projectile as a dynamic object.
        btTransform startTransform;
        startTransform.setIdentity();
        btScalar mass(0.1f); //NOTE(sean): for things that the player might be able to interact with, we want the mass to be smaller
        btScalar friction(0.5f);
        bool isDynamic = (mass != 0.0f);
        btVector3 localInertia(lookDirection.x * 500.0f, lookDirection.y * 500.0f, lookDirection.z * 500.);
        if (isDynamic) {
            projectile->calculateLocalInertia(mass, localInertia);
        }
        startTransform.setOrigin(into(startPos) + btVector3(lookDirection.x * 250., lookDirection.y * 250., lookDirection.z * 250.));

        // std::cout << "{"  << lookdir.x << ", " << lookdir.y << ", " << lookdir.z << "}" << std::endl;
        // Motionstate again.
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, projectile, localInertia);
        rbInfo.m_friction = friction;
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setAngularFactor(btVector3(0., 0., 0.));
        currentWorld->addRigidBody(body);

        projectileVelocity = projectileVelocity * 250.0f;
        body->applyForce(btVector3(projectileVelocity * lookDirection.x, projectileVelocity * lookDirection.y, projectileVelocity * lookDirection.z), into(startPos));
    }
}

void ProjectileManager::CalculateRay(RayProjectile& newRay, Vector3 Pos1, btVector3 btStartPos, btVector3 btLookDirection, i32 rayBounces, XMVECTORF32 color) {
    newRay.Pos1 = Pos1;

    btCollisionWorld::ClosestRayResultCallback rayResults(btStartPos + btLookDirection * 500., btStartPos + btLookDirection * 5000.);
    currentWorld->rayTest(btStartPos + btLookDirection * 500., btStartPos + btLookDirection * 5000., rayResults);

    if (rayResults.hasHit()) {

        btVector3 hitPosition = rayResults.m_hitPointWorld;
        btScalar dotProduct = btStartPos.dot(rayResults.m_hitNormalWorld);
        btVector3 incomingDirection = (hitPosition - btStartPos).normalize();
        btVector3 reflectedDirection = btLookDirection - 2. * (btLookDirection * rayResults.m_hitNormalWorld) * rayResults.m_hitNormalWorld;

        if (rayBounces > 0) {
            GenerateRayProjectile(into(hitPosition), into(reflectedDirection), 1, 1, rayBounces - 1, color, true);
        }

        newRay.Pos2 = into(hitPosition);
    } else {
        newRay.Pos2 = into(btStartPos + btLookDirection * 5000.0);
    }
}

void ProjectileManager::GenerateRayProjectile(Vector3 startPos, Vector3 lookDirection, i32 rayCount, i32 rayAccuracy, i32 rayBounces, XMVECTORF32 rayColor, b8 recursed) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, texture.
    */

    f32 negativeAccuracy = rayAccuracy * -1.0f;
    f32 range = (rayAccuracy - negativeAccuracy + 1.0f);

    RayProjectile newRay;
    newRay.Pos1 = Vector3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;
    newRay.Pos2 = Vector3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.;
    newRay.Color = rayColor;

    if (recursed) {
        CalculateRay(newRay, (startPos + lookDirection), into(startPos), into(lookDirection), rayBounces, Colors::PeachPuff);

        currentRayProjectiles->push_back(newRay);
    } else {
        for (i32 i = 0; i < rayCount; i++) {
            srand(randomSeed);
            randomSeed++;
            if (randomSeed > 999) {
                randomSeed = 0;
            }

            lookDirection = JitterVector3(lookDirection, negativeAccuracy, range);
            CalculateRay(newRay, (startPos + lookDirection * 500.0f), into(startPos), into(lookDirection), rayBounces, Colors::Peru);

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

