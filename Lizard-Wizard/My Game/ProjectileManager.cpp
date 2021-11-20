// Inclusions
#include "Game.h"
#include <ComponentIncludes.h>

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

    instance.model = ModelIndex::Cube;
    instance.world = MoveScaleMatrix(body->getWorldTransform().getOrigin(), Vector3(25.0f));
    instance.texture = TextureIndex::White;

    return instance;
}

ModelInstance GetCubeModel(btRigidBody* body) {
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 origin = body->getWorldTransform().getOrigin();
    Quat rotation = body->getWorldTransform().getRotation();
    Vec3 scale = boxShape->getHalfExtentsWithMargin();

    ModelInstance instance = {};
    instance.model = ModelIndex::Cube;
    instance.texture = TextureIndex::White;
    instance.world = MoveRotateScaleMatrix(origin, rotation, scale);

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
    const SoundIndex::e projectileSound,
    const b8 ignoreCaster
) {
    for (i32 i = 0; i < projectileCount; i++) {
        // Check cache for open projectiles.
        if (m_ProjectilesCache.Size() < projectileCount - i) {
            return;
        }

        static u32 count = 0;
        count += 1;

        // Instance Variables
        Entity e = m_ProjectilesCache.RemoveTail();
        m_ModelsActive.AddExisting(e);
        m_ProjectilesActive.AddExisting(e);
        m_Timers.AddExisting(e, 5.0f);

        m_Projectiles.Get(e)->ProjSound = projectileSound;
        m_Projectiles.Get(e)->Color = projectileColor * 10.0f;

        btRigidBody* projectile = *m_RigidBodies.Get(e);
        btTransform trans;

        Vec3 newDirection = JitterVec3(lookDirection, -projectileAccuracy, projectileAccuracy);
        Vec3 velDirection = JitterVec3(lookDirection, -projectileAccuracy, projectileAccuracy);
        Vec3 orig = startPos + lookDirection * 200.0f;

        // Clear forces
        projectile->clearForces();

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
    if (m_RaysCache.Size() < 1) {
        return;
    }

    // Get ECS information
    Entity e = m_RaysCache.RemoveTail();
    RayProjectile* currentRay = m_Rays.Get(e);
    m_ModelsActive.AddExisting(e);
    m_RaysActive.AddExisting(e); 
    m_Timers.AddExisting(e, 0.1f);


    // Set ray information
    currentRay->Pos1 = Pos1;

    btCollisionWorld::ClosestRayResultCallback rayResults(Pos1, Vec3(Pos1 + btLookDirection * 15000.0));
    if (ignoreCaster) {
        rayResults.m_collisionFilterGroup = 0b00100;
		rayResults.m_collisionFilterMask = 0b00001;

    }
    m_pDynamicsWorld->rayTest(Pos1, Vec3(Pos1 + btLookDirection * 15000.0), rayResults);

    if (rayResults.hasHit()) {
        // Warning (Ethan) : DO NOT EDIT at this pointer.
        btCollisionObject* hitObject = const_cast<btCollisionObject*>(rayResults.m_collisionObject);
        Vec3 hitPosition = rayResults.m_hitPointWorld;
        Vec3 incomingDirection = (hitPosition - Pos1); incomingDirection.Normalize();
        Vec3 reflectedDirection = btLookDirection - 2. * (btLookDirection * rayResults.m_hitNormalWorld) * rayResults.m_hitNormalWorld;

        f32 dotProduct = Pos1.Dot(Vec3(rayResults.m_hitNormalWorld));
        f32 distance = DistanceBetweenVectors(Pos1, hitPosition);

        currentRay->Pos2 = Vec3(hitPosition);
        newRay.Pos2 = Vec3(hitPosition);

        rayBounces = rayBounces - 1;
        Vec3 origin = (hitPosition + Pos1) / 2;

        m_pAudio->play(SoundIndex::LightningCast, hitPosition, 0.75f, 0.5);

        // Create Particle for Impact (Very ugly, will clean up later.)
        ParticleInstanceDesc particle;
        particle.count = 30;
        particle.initial_pos = hitPosition;
        particle.initial_dir = reflectedDirection;
        particle.light_color = Vec3(0.7f, 0.5, 0.1f) * 30.0f;
        particle.model = ModelIndex::Cube;
        particle.texture = TextureIndex::White;
        particle.glow = Vec3(0.5f, 0.5f, 0.5f);
        particle.model_scale = Vec3(8.0f);
        particle.initial_speed = 300.0f;
        particle.dir_randomness = 0.7f;
        particle.speed_randomness = 0.5f;
        particle.initial_acc = Vec3(0.0f, -1000.0f, 0.0f);
        particle.acc_randomness = 0.2f;
        particle.min_alive_time = 0.2f;
        particle.max_alive_time = 1.7f;

        // Spawn Particle
        SpawnParticles(&particle);

        if (rayBounces > 0) {
            GenerateRayProjectile(caster, Vec3(hitPosition), Vec3(reflectedDirection), 1, 1, rayBounces, color, true, ignoreCaster);
        }

    } else {
        Vec3 origin = Vec3(Pos1 + btLookDirection * 15000.0);

        currentRay->Pos2 = origin;
        newRay.Pos2 = origin;
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
    RayProjectile newRay;
    newRay.Pos1 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 500.;
    newRay.Pos2 = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 15000.0;
    newRay.Color = rayColor;

    if (!recursed) {
        for (i32 i = 0; i < rayCount; i++) {
            Vec3 newDirection = JitterVec3(lookDirection, -rayAccuracy, rayAccuracy);
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
    m_ProjectilesCache.AddExisting(e);

    btRigidBody* projectile = *m_RigidBodies.Get(e);
    SimProjectile* proj = m_Projectiles.Get(e);

    // Change Attributes
    Vec3 orig = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    RBTeleport(projectile, orig);

    // Move lights
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&orig;

    // Prepare projectile
    proj->Bounces = 0;
    proj->MaxBounces = 3;

    // Disable Rendering
    (*m_ModelInstances.Get(e)).world = MoveScaleMatrix(orig, Vector3(25.0f));
    m_ModelsActive.Remove(e);

    // Removes rigidbody from world to edit constant attributes.
    m_pDynamicsWorld->removeRigidBody(projectile);

    RBSetMassFriction(projectile, 0.0f, 0.0f);
}

void CGame::StripRay(Entity e) {
    m_Timers.Remove(e);
    m_RaysCache.AddExisting(e);

    // Disable Rendering
    (*m_ModelInstances.Get(e)).world = MoveScaleMatrix(Vec3(FLT_MAX, FLT_MAX, FLT_MAX), Vector3(25.0f));
    m_ModelsActive.Remove(e);
}


void CGame::InitializeProjectiles() {
    for every(index, PROJECTILE_CACHE_SIZE) {
        // Create Rigidbody and get ECS identifier
        btRigidBody* newBody = CreateSphereObject(50.f, Vec3(FLT_MAX, FLT_MAX, FLT_MAX), 0.0f, 0.0f, 3, 0b00001);
        Entity e = m_RigidBodyMap.at(newBody);
        SimProjectile newProj;
        RemoveRigidBody(newBody);

        // Continuous Convex Collision (NOTE) Ethan : This is expensive, so only use it for projectiles.
        RBSetCcd(newBody, 1e-7f, 0.50f);

        // Prepare light
        Light newLight = { Vec4(FLT_MAX, FLT_MAX, FLT_MAX ,0), Vec4{150.0f, 30.0f, 10.0f, 0} };

        // Prepare projectile
        newProj.ProjSound = SoundIndex::FireImpact1;
        newProj.Bounces = 0;
        newProj.MaxBounces = 3;

        // Prepare model
        m_ModelInstances.AddExisting(e, GetSphereModel(*m_RigidBodies.Get(e)));

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_Projectiles.AddExisting(e, newProj);
        m_RigidBodies.AddExisting(e, newBody);
        m_ProjectilesCache.AddExisting(e);
    }

    for every(index, RAY_CACHE_SIZE) {
        // Create Rigidbody and get ECS identifier
        Entity e = Entity();
        RayProjectile newRay;

        // Prepare light
        Light newLight = { Vec4(FLT_MAX, FLT_MAX, FLT_MAX ,0), Vec4(0.7f, 0.5, 0.1f, 0.0f) * 35.0f };

        Vec3 origin = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        Vec3 side = Vec3(0.0f, 1.0f, 0.0f);

        Quat rotation = Quat::CreateFromAxisAngle(side, M_PI / 2.0f);
        Quat turn = side.y != 1.0f ?
            Quat::CreateFromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), -M_PI / 2.0f) :
            Quat::CreateFromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), M_PI);
        rotation *= turn;

        newRay.Pos1 = origin;
        newRay.Pos2 = origin - Vec3(0.0f, -1.0f, 0.0f);

        ModelInstance instance;
        instance.glow = 10.0f;
        instance.model = ModelIndex::Cube;
        instance.texture = TextureIndex::White;
        instance.world = MoveRotateScaleMatrix(origin, rotation, Vec3(15.0f, 2.0f, 15.0f));

        // Prepare model
        m_ModelInstances.AddExisting(e, instance);

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_Rays.AddExisting(e, newRay);
        m_RaysCache.AddExisting(e);
    }
}
