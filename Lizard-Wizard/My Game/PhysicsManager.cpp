// Inclusions
#include "Game.h"
#include <ComponentIncludes.h>

/* Note(Ethan) : This is the physics manager, it helps us clean up the game.cpp and further simplify how we use Bullet3.
*
            '   '
            _____

     wachu looking at bud

*/
static CGame* Self;

void CGame::RBSetCcd(btRigidBody* body, f32 threshold, f32 radius) {
    body->setCcdMotionThreshold(threshold);
    body->setCcdSweptSphereRadius(radius);
}

// Note(Ethan) : These functions help in the creation of Bullet3 physics objects.
btTransform CGame::NewTransform(btCollisionShape* shape, Vec3 origin) {
    btTransform startTransform;
    startTransform.setIdentity();
    f32 mass = 0.1f; //NOTE(sean): for things that the m_Player might be able to interact with, we want the mass to be smaller
    f32 friction = 0.5f;
    bool isDynamic = (mass != 0.0f);
    if (isDynamic) {
        shape->calculateLocalInertia(mass, btVector3(0.0f, 0.0f, 0.0f));
    }
    startTransform.setOrigin(origin);
    return startTransform;
}

btRigidBody* CGame::NewRigidBody(
    btCollisionShape* shape,
    btTransform startTransform,
    f32 mass,
    f32 friction,
    i32 group,
    i32 mask) {

    // Configure and add to Bullet3
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    Vec3 localInertia(Vec3(0, 0, 0));
    f32 restitution = 0.1f;
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    rbInfo.m_friction = friction;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setAngularFactor(Vec3(0., 0., 0.));
    body->setRestitution(restitution);
    m_pDynamicsWorld->addRigidBody(body, group, mask);

    // Add to ECS
    Entity e = Entity();
    m_RigidBodies.AddExisting(e, body);
    m_RigidBodyMap.insert(std::make_pair(body, e));

    return body;
}

// Note(Ethan) : Will speed up the creation of commonly used shapes.
btRigidBody* CGame::CreateSphereObject(
    btScalar radius, 
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btSphereShape(radius);
    m_pCollisionShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}

btRigidBody* CGame::CreateBoxObject(
    Vec3 size, 
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btBoxShape(size);
    m_pCollisionShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}

btRigidBody* CGame::CreateCapsuleObject(
    btScalar radius,
    btScalar height,
    Vec3 origin, 
    f32 mass, 
    f32 friction, 
    i32 group, 
    i32 mask
) {

    // Push to the collision array.
    btCollisionShape* shape = new btCapsuleShape(radius, height);
    m_pCollisionShapes.push_back(shape);

    // Create object and return the pointer so further adjustments can be made.
    return NewRigidBody(shape, NewTransform(shape, origin), mass, friction, group, mask);
}

btRigidBody* CGame::CreateConvexObject(f32 mass, f32 friction, i32 group, i32 mask) {
    return 0;
}

// Note(Ethan) : We need this for impact noises and bounce impulses.
void CGame::PhysicsCollisionCallBack(btDynamicsWorld* p, btScalar t) {
    UNREFERENCED_PARAMETER(t);
    btDispatcher* pDispatcher = p->getDispatcher();
    const u32 numManifolds = (u32)pDispatcher->getNumManifolds();
    b8 ignoreCollision = false;

    for every(manifold, numManifolds) {
        btPersistentManifold* pManifold = pDispatcher->getManifoldByIndexInternal(manifold);

        // Get bodys
        btRigidBody* pBody0 = const_cast<btRigidBody*>(btRigidBody::upcast(pManifold->getBody0()));
        btRigidBody* pBody1 = const_cast<btRigidBody*>(btRigidBody::upcast(pManifold->getBody1()));
        Entity pEntity0 = Self->m_RigidBodyMap.at(pBody0);
        Entity pEntity1 = Self->m_RigidBodyMap.at(pBody1);


        // Determine whether we need to ignore the given collision for a few frames, we might need to make a method
        // to completely disable collision announcements between two objects incase something is resting on another object.

        if (Self->m_CollisionPairs.Contains(pEntity0)) { // Check pEntity0
            if (Self->m_CollisionPairs.Get(pEntity0) == &pEntity1) {
                ignoreCollision = true;
            }
        }

        if (Self->m_CollisionPairs.Contains(pEntity1)) { // Check pEntity1
            if (Self->m_CollisionPairs.Get(pEntity1) == &pEntity0) {
                ignoreCollision = true;
            }
        }

        // Add to the collision table if we don't want to ignore this collision.
        if (ignoreCollision == false) {
            const i32 numContacts = pManifold->getNumContacts();
            b8 duplicate = false;

            // Check for duplicate collisions
            if (Self->m_CollisionPairs.Contains(pEntity0)) {
                if (*(Self->m_CollisionPairs.Get(pEntity0)) == pEntity1) {
                    duplicate = true;
                }
            }
            if (Self->m_CollisionPairs.Contains(pEntity1)) {
                if (*(Self->m_CollisionPairs.Get(pEntity1)) == pEntity0) {
                    duplicate = true;
                }
            }

            if (numContacts > 0 && !Self->m_CurrentCollisions.Contains(pEntity0) && !duplicate) {
                // Note (Ethan) : Honestly we only need one contact point, none of our objects are large enough to justify more than that.
                btManifoldPoint& pt = pManifold->getContactPoint(0);

                // Add new collisions to the table.
                Self->m_CurrentCollisions.AddExisting(pEntity0);
                Self->m_CurrentCollisions.AddExisting(pEntity1);

                // Add collisions to ignore table to prevent spamming of collisions.
                Self->m_CollisionPairs.AddExisting(pEntity0, pEntity0);
                Self->m_CollisionPairs.AddExisting(pEntity1, pEntity1);
            }
        } else { // Ignore this collision because we don't want to spam collisions multiple times.
            return;
        }
    }
}

void CGame::CustomPhysicsStep() {
    // This plays a sound for all collisions. (NOTE) Ethan : Will be moved into CGame once I move all physics stepping into a new custom function.
    for every(index, m_CollisionPairs.Size()) {
        Entity e = m_CollisionPairs.Entities()[index];

        btRigidBody* body0 = (*m_RigidBodies.Get(e));
        btRigidBody* body1 = (*m_RigidBodies.Get(*m_CollisionPairs.Get(e)));

        Vec3 pos0 = body0->getWorldTransform().getOrigin();
        Vec3 pos1 = body1->getWorldTransform().getOrigin();

        Vec3 lVelocity0 = body0->getLinearVelocity();
        Vec3 lVelocity1 = body1->getLinearVelocity();

        f32 volume = lVelocity0.Length() / 5000.0f;
        btClamp(volume, 0.0f, 1.0f);

        //////////////////////
        // COLLISION EVENTS //
        //////////////////////

        // PROJECTILES //
        if (m_ProjectilesActive.Contains(e)) {
            SimProjectile* proj =  m_Projectiles.Get(e);

            // Determine Bounces
            proj->Bounces++;
            if (proj->MaxBounces < proj->Bounces) {
                *m_Timers.Get(e) = 0.0f;
            }

            // Play Audio
            m_pAudio->play(m_Projectiles.Get(e)->ProjSound, pos0, volume, 0.5);
        }

        // OBJECT HIT BY PROJECTILES //
        if (m_ProjectilesActive.Contains(*m_CollisionPairs.Get(e))) {
            // Create Particles
            SimProjectile* proj = m_Projectiles.Get(*m_CollisionPairs.Get(e));
            Vec4 projColor = proj->Color;

            // Create Particle for Impact (Very ugly, will clean up later.)
            ParticleInstanceDesc particle;
            particle.count = 25;
            particle.initial_pos = pos0;
            particle.initial_dir = XMVector3Normalize(lVelocity1 - lVelocity0);
            particle.light_color = Vec3(projColor.x, projColor.y, projColor.z);
            particle.model = ModelIndex::Cube;
            particle.texture = TextureIndex::White;
            particle.glow = Vec3(0.5f, 0.5f, 0.5f);
            particle.model_scale = Vec3(4.0f);
            particle.initial_speed = 500.0f;
            particle.dir_randomness = 0.7f;
            particle.speed_randomness = 0.5f;
            particle.initial_acc = Vec3(0.0f, -1000.0f, 0.0f);
            particle.acc_randomness = 0.2f;
            particle.min_alive_time = 0.2f;
            particle.max_alive_time = 1.7f;

            // Spawn Particle
            SpawnParticles(&particle);
        }

        // NPC HIT BY PROJECTILES //
        if (m_NPCsActive.Contains(e)) {
            // Stub for now, but we can implement damage stuff in here.
        }

        // PLAYER HIT BY PROJECTILES //
        if (e == m_Player) {
            // Stub for now, but we can implement damage stuff in here.
        }
    }

    // Clear old collisions once were done with the current pass.
    m_CurrentCollisions.Clear();
    m_CollisionPairs.Clear();
}

// Helper Functions, these will be helpful when we don't want to keep passing the dynamics world to managers.
void CGame::RemoveRigidBody(btRigidBody* body) {
    m_pDynamicsWorld->removeRigidBody(body);
}


void CGame::AddRigidBody(btRigidBody* body, i32 group, i32 mask) {
    m_pDynamicsWorld->addRigidBody(body, group, mask);
}

void CGame::RBSetMassFriction(btRigidBody* body, f32 mass, f32 friction) {
    btVector3 inertia;
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);
}

void CGame::RBTeleport(btRigidBody* body, Vec3 origin) {
    body->getWorldTransform().setOrigin(origin);
    body->clearForces();
}

void CGame::RBTeleportLaunch(btRigidBody* body, Vec3 origin, Vec3 velocity) {
    body->getWorldTransform().setOrigin(origin);
    body->clearForces();
    body->setLinearVelocity(velocity);
}

void CGame::DestroyPhysicsObject(btCollisionShape* shape) {
    // Nothing for now, figure out a good way to destroy any given Bullet3 objects.
}


void CGame::InitializePhysics() {

    // Create Config
    btDefaultCollisionConfiguration* CurrentConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* CurrentDispatcher = new btCollisionDispatcher(CurrentConfiguration);
    btDbvtBroadphase* CurrentBroadphaseCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* CurrentSolver = new btSequentialImpulseConstraintSolver;

    // Assign Config and Array
    m_pDynamicsWorld = new btDiscreteDynamicsWorld(CurrentDispatcher, CurrentBroadphaseCache, CurrentSolver, CurrentConfiguration);
    m_pCollisionShapes = btAlignedObjectArray<btCollisionShape*>();

    // Set other attributes
    m_pDynamicsWorld->setGravity(btVector3(0.0, -5000.0, 0.0));

    // Static Self Assignment
    Self = this;

    // Collision Callback
    m_pDynamicsWorld->setInternalTickCallback(PhysicsCollisionCallBack);

    // m_Player Rigidbody | (Note) : Create this first, as the m_Player is currently indexed as [0] in the collision table.
    {
        btRigidBody* rb = CreateCapsuleObject(100.0f, 250.0f, IndexToWorld(1, 1), 1.0f, 0.5f, 2, 0b00001);
        RBSetMassFriction(rb, 1.0, 0.1);
        RBSetCcd(rb, 1e-7, 200.0f);
        m_Player = m_RigidBodyMap.at(rb);
    }

    // Set initial step
    m_CurrentStep = 0;
}