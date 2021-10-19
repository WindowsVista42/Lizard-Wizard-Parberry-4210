// Inclusions
#include "PhysicsManager.h"
#include "Game.h"
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include "CustomBind.h"
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

/* Note(Ethan) : This is the physics manager, it helps us clean up the game.cpp and further simplify how we use Bullet3.
*
            '   '
            _____

     wachu looking at bud

*/

void PhysicsManager::CreatePhysicsObject() {
    // Note(Ethan) : This lets us create a new objects much easier than before.
}

void PhysicsManager::DestroyPhysicsObject() {
    // Note(Ethan) : This lets us destroy objects much easier than before.

}

void PhysicsManager::InitializePhysics(btDiscreteDynamicsWorld** GameWorld, btAlignedObjectArray<btCollisionShape*>* GameShapes) {
    // Note(Ethan) : Effectively works the same as before, just call this function and it will handle the initialization.
    CurrentConfiguration = new btDefaultCollisionConfiguration();
    CurrentDispatcher = new btCollisionDispatcher(CurrentConfiguration);
    CurrentBroadphaseCache = new btDbvtBroadphase();
    CurrentSolver = new btSequentialImpulseConstraintSolver;
    *GameWorld = new btDiscreteDynamicsWorld(CurrentDispatcher, CurrentBroadphaseCache, CurrentSolver, CurrentConfiguration);
    *GameShapes = btAlignedObjectArray<btCollisionShape*>();
    CurrentWorld = *GameWorld;
    CurrentShapes = *GameShapes;
    CurrentWorld->setGravity(btVector3(0.0, -5000.0, 0.0));

    // Collision Callback
    btOverlapFilterCallback* filterCallback = new ProjectileCollisionFilter();
    CurrentWorld->getPairCache()->setOverlapFilterCallback(filterCallback);
}