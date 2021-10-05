// Inclusions
#include "ProjectileManager.h"
#include "Game.h"
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include "CustomBind.h"
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

void ProjectileManager::GenerateSimProjectile(Vector3 startPos, Vector3 lookDirection, i32 projectileVelocity, XMVECTORF32 projectileColor) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), requested velocity, color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, model, texture, and color.
    */
    btCollisionShape* projectile = new btSphereShape(btScalar(50.));
    currentSimProjectiles.push_back(projectile);

    // Implicating playerShape as a dynamic object.
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

void ProjectileManager::GenerateRayProjectile(Vector3 startPos, Vector3 lookDirection, XMVECTORF32 rayColor) {
    /* Note(Ethan) :
       currently includes :
       Vector3 for position, Vector3 for lookvector (direction), color.

       will expand to include :
       accuracy, # of projectiles, # of bounces, texture.
    */

    btScalar param(0.5);
    RayProjectile newRay = RayProjectile();
    newRay.Pos1 = startPos;
    newRay.Pos2 = lookDirection;
    newRay.Color = rayColor;
    currentRayProjectiles->push_back(newRay);
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

