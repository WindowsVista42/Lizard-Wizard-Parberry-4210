// Inclusions
#include "Game.h"

/* Note(Ethan) : This is a prototype test for the projectile system, I will reduce comments once we get to making the release version.
* 
            '   '
            _____

     wachu looking at bud 

*/

ModelInstance GetSphereModel(btRigidBody* body) {
    ModelInstance instance = {};
    btCollisionShape* currentShape = body->getCollisionShape();
    btSphereShape* sphereShape = reinterpret_cast<btSphereShape*>(currentShape);


    instance.model = (u32)ModelIndex::Suzanne;
    instance.world = MoveScaleMatrix(body->getWorldTransform().getOrigin(), Vector3(25.0f));
    instance.texture = 1;

    return instance;
}

void CGame::GenerateSimProjectile(
    btCollisionObject* caster, 
    const Vec3 startPos, 
    const Vec3 lookDirection, 
    const i32 projectileCount, 
    const f32 projectileVelocity, 
    const f32 projectileAccuracy, 
    const Vec4 projectileColor, 
    const b8 ignoreCaster
) {
    /* Note(Ethan) :
       will expand to include :
       model, texture.
    */
    for (i32 i = 0; i < projectileCount; i++) {
        // New Projectile System, uses caching to improve stuff.
        if (m_ProjectilesCache.Size() < projectileCount - i) {
            return;
        }

        static u32 count = 0;
        count += 1;

        Entity e = m_ProjectilesCache.RemoveTail();
        m_ModelsActive.AddExisting(e);
        m_ProjectilesActive.AddExisting(e);
        m_Timers.AddExisting(e, 2.0f);

        btRigidBody* projectile = *m_RigidBodies.Get(e);

        Vec3 newDirection = JitterVec3(lookDirection, -projectileAccuracy, projectileAccuracy);
        Vec3 velDirection = JitterVec3(lookDirection, -0.02f, 0.02f);

        projectile->clearForces();

        Vec3 offset = Vec3(100.0f, 100.0f, 50.0f);

        btTransform trans;
        // UPDATE(sean): changed 300.0f to 100.0f, if you looked down, projectiles were spawning in the ground which would cause crashes on debug builds.
        // Because the projectiles are spawning so close to the camera it looks really jarring, so we're shifting them up 100 units.
        // In the future, we can probably tell the projectiles to spawn from the players wand.
        Vec3 orig = Vec3(Vec3(startPos + newDirection * 100.0f) + Vec3(0, 100.0f, 0));

        // Set static attributes.
        RBSetMassFriction(projectile, 0.5f, 0.5f);
        projectile->setRestitution(6.5f);

        // Re-add regidbody to world after static attribute edit.
        m_pDynamicsWorld->addRigidBody(projectile, 2, 0b00001);

        // Set real-time attributes.
        RBTeleportLaunch(projectile, orig, Vec3(velDirection * projectileVelocity));

        // Continuous Convex Collision (NOTE) Ethan : This is expensive, so only use it for projectiles.
        //SetRigidBodyCcd(projectile, 10.0, 75.0f);

        f32 lum = 100.0f;
        m_pRenderer->lights.Get(e)->color = Vec4(projectileColor.x * lum, projectileColor.y * lum, projectileColor.z * lum, 0);

        projectile->activate();
    }
}

void CGame::CalculateRay(
    btCollisionObject* caster, 
    RayProjectile& newRay, 
    Vec3 Pos1, 
    Vec3 btLookDirection,
    i32 rayBounces,
    Vec4 color,
    b8 ignoreCaster
) {

    newRay.Pos1 = Pos1;

    btCollisionWorld::ClosestRayResultCallback rayResults(Pos1, Vec3(Pos1 + btLookDirection * 5000.));
    if (ignoreCaster) {
        rayResults.m_collisionFilterGroup = 0b00100;
		rayResults.m_collisionFilterMask = 0b00001;

    }
    m_pDynamicsWorld->rayTest(Pos1, Vec3(Pos1 + btLookDirection * 5000.), rayResults);

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

void CGame::GenerateRayProjectile(
    btCollisionObject* caster, 
    const Vec3 startPos, 
    const Vec3 lookDirection, 
    const i32 rayCount, 
    const i32 rayBounces, 
    const f32 rayAccuracy, 
    const Vec4 rayColor, 
    const b8 recursed, 
    const b8 ignoreCaster
) {

    /* Note(Ethan) :
       will expand to include :
       texture.
    */
    RayProjectile newRay;
    newRay.Pos1 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;
    newRay.Pos2 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.;
    newRay.Color = rayColor;

    if (!recursed) {
        for (i32 i = 0; i < rayCount; i++) {
            Vec3 newDirection = JitterVec3(lookDirection, -0.2, 0.2);
            CalculateRay(caster, newRay, startPos, newDirection, rayBounces, Colors::Peru, ignoreCaster);

            m_currentRayProjectiles.push_back(newRay);
        }
    }
    else {
        for (i32 i = 0; i < rayCount; i++) {
            CalculateRay(caster, newRay, startPos, lookDirection, rayBounces, Colors::Peru, ignoreCaster);

            m_currentRayProjectiles.push_back(newRay);
        }
    }
}

void CGame::StripProjectile(Entity e) {
    m_Timers.Remove(e);
    //m_ProjectilesActive.Remove(e); // dont uncomment, youll end up removing things twice which is obviously bad
    m_ProjectilesCache.AddExisting(e);
    btRigidBody* projectile = *m_RigidBodies.Get(e);

    // Change Attributes
    Vec3 orig = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    RBTeleport(projectile, orig);

    // Move lights
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&orig;

    // Disable Rendering
    (*m_ModelInstances.Get(e)).world = MoveScaleMatrix(orig, Vector3(25.0f));
    m_ModelsActive.Remove(e);

    // Removes rigidbody from world to edit constant attributes.
    m_pDynamicsWorld->removeRigidBody(projectile);

    RBSetMassFriction(projectile, 0.0f, 0.0f);
}


void CGame::InitializeProjectiles() {
    for every(index, PROJECTILE_CACHE_SIZE) {
        // Create Rigidbody and get ECS identifier
        btRigidBody* newBody = CreateSphereObject(50.f, Vec3(FLT_MAX, FLT_MAX, FLT_MAX), 0.0f, 0.0f, 3, 0b00001);
        Entity e = m_RigidBodyMap.at(newBody);
        m_pDynamicsWorld->removeRigidBody(newBody);

        // Continuous Convex Collision (NOTE) Ethan : This is expensive, so only use it for projectiles.
        // DISABLED UNTIL FIXED IN DEBUG
        RBSetCcd(newBody, 0.0001, 75.0f);

        // Prepare light
        Light newLight = { Vec4(FLT_MAX, FLT_MAX, FLT_MAX ,0), Vec4{150.0f, 30.0f, 10.0f, 0} };

        // Prepare model
        m_ModelInstances.AddExisting(e, GetSphereModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_RigidBodies.AddExisting(e, newBody);
        m_ProjectilesCache.AddExisting(e);
    }
}
