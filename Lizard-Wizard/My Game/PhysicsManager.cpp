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
    i32 mask
) {

    // Configure and add to Bullet3
    btDefaultMotionState* myMotionState = m_PhysicsAllocator.Create(btDefaultMotionState(startTransform));

    Vec3 localInertia(Vec3(0, 0, 0));
    f32 restitution = 0.1f;
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    rbInfo.m_friction = friction;

    btRigidBody* body = m_PhysicsAllocator.Create(btRigidBody(rbInfo));
    //btRigidBody* body = new(ptr_rb) btRigidBody(rbInfo);

    body->setAngularFactor(Vec3(0., 0., 0.));
    body->setRestitution(restitution);
    m_pDynamicsWorld->addRigidBody(body, group, mask);

    // Add to ECS
    Entity e = Entity();
    m_RigidBodies.AddExisting(e, body);
    m_RigidBodyMap.insert(std::make_pair(body, e));

    // Self point
    body->setUserPointer(&body);

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
    btCollisionShape* shape = m_PhysicsAllocator.Create(btSphereShape(radius));

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
    btCollisionShape* shape = m_PhysicsAllocator.Create(btBoxShape(size));

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
    btCollisionShape* shape = m_PhysicsAllocator.Create(btCapsuleShape(radius, height));
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
                // Add new collisions to the table.
                Self->m_CurrentCollisions.AddExisting(pEntity0);
                Self->m_CurrentCollisions.AddExisting(pEntity1);

                // Add collisions to ignore table to prevent spamming of collisions.
                Self->m_CollisionPairs.AddExisting(pEntity0, pEntity1);
                Self->m_CollisionPairs.AddExisting(pEntity1, pEntity0);
            }
        } else { // Ignore this collision because we don't want to spam collisions multiple times.
            return;
        }
    }
}

void CGame::CustomPhysicsStep() {
    // This plays a sound for all collisions. (NOTE) Ethan : Will be moved into CGame once I move all physics stepping into a new custom function.

    for every(index, m_CollisionPairs.Size()) {
        Entity causeObject = m_CollisionPairs.Entities()[index];
        Entity hitObject = *m_CollisionPairs.Get(causeObject);

        btRigidBody* body0 = (*m_RigidBodies.Get(causeObject));
        btRigidBody* body1 = (*m_RigidBodies.Get(hitObject));

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
        if (m_ProjectilesActive.Contains(hitObject)) {
            SimProjectile* proj = m_Projectiles.Get(hitObject);
            if(proj->IgnoreList.find(causeObject) ==  proj->IgnoreList.end()) {
                player_ignore_list.insert(causeObject);

                // Create Particles
                Vec4 projColor = proj->Color;

                // Create Particle for Impact (Very ugly, will clean up later.)
                ParticleInstanceDesc particle;
                particle.count = 25;
                particle.initial_pos = pos1;
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

                // rbuhrbhuiihrbuashuikbdrshikuagdr
                if (m_Player == causeObject) {
                    // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                    if (player_ignore_list.find(hitObject) == player_ignore_list.end()) {
                        player_ignore_list.insert(hitObject);
                        m_Healths.Get(m_Player)->current -= proj->Damage;
                        m_PlayerHitTimer = 0.5f;
                        m_pAudio->play(SoundIndex::PlayerImpact1, staff_tip, 0.45f, 0.5);
                    }
                }

                if (m_NPCsActive.Contains(causeObject)) {
                    // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                    NPC* npc = m_NPCs.Get(causeObject);
                    if (npc->IgnoreList.find(hitObject) == npc->IgnoreList.end()) {
                        npc->IgnoreList.insert(hitObject);
                        m_Healths.Get(causeObject)->current -= proj->Damage;
                        m_pAudio->play(npc->ImpactSound, npc->LastPosition, 0.55f, 0.5);
                    }
                }

                // Spawn Particle
                SpawnParticles(&particle);

                // Play Audio
                m_pAudio->play(m_Projectiles.Get(hitObject)->ProjSound, pos1, volume, 0.5);
            }

        } else if (m_ProjectilesActive.Contains(causeObject)) {
            SimProjectile* proj = m_Projectiles.Get(causeObject);
            if (proj->IgnoreList.find(hitObject) == proj->IgnoreList.end()) {
                player_ignore_list.insert(hitObject);

                // Create Particles
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

                // rbuhrbhuiihrbuashuikbdrshikuagdr
                if (m_Player == hitObject) {
                    // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                    if (player_ignore_list.find(causeObject) == player_ignore_list.end()) {
                        player_ignore_list.insert(causeObject);
                        m_Healths.Get(m_Player)->current -= proj->Damage;
                        m_PlayerHitTimer = 0.5f;
                        m_pAudio->play(SoundIndex::PlayerImpact1, staff_tip, 0.45f, 0.5);
                    }
                }

                if (m_NPCsActive.Contains(hitObject)) {
                    // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                    NPC* npc = m_NPCs.Get(hitObject);
                    if (npc->IgnoreList.find(causeObject) == npc->IgnoreList.end()) {
                        npc->IgnoreList.insert(causeObject);
                        m_Healths.Get(hitObject)->current -= proj->Damage;
                        m_pAudio->play(npc->ImpactSound, npc->LastPosition, 0.55f, 0.5);
                    }
                }

                // Spawn Particle
                SpawnParticles(&particle);

                // Determine Bounces
                proj->Bounces++;
                if (proj->MaxBounces < proj->Bounces) {
                    *m_Timers.Get(causeObject) = 0.0f;
                }

                // Play Audio
                m_pAudio->play(m_Projectiles.Get(causeObject)->ProjSound, pos0, volume, 0.5);

            }
        }

        if (m_RaycheckActive.Contains(hitObject)) {
            // rbuhrbhuiihrbuashuikbdrshikuagdr
            if (m_Player == causeObject) {
                // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                if (player_ignore_list.find(hitObject) == player_ignore_list.end()) {
                    player_ignore_list.insert(hitObject);
                    m_Healths.Get(m_Player)->current -= 1;
                    m_PlayerHitTimer = 0.5f;
                    m_pAudio->play(SoundIndex::PlayerImpact1, staff_tip, 0.45f, 0.5);
                }
            }

            if (m_NPCsActive.Contains(causeObject)) {
                // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                NPC* npc = m_NPCs.Get(causeObject);
                if (npc->IgnoreList.find(hitObject) == npc->IgnoreList.end()) {
                    npc->IgnoreList.insert(hitObject);
                    m_Healths.Get(causeObject)->current -= 1;
                    m_pAudio->play(npc->ImpactSound, npc->LastPosition, 0.55f, 0.5);
                }
            }
        } else if(m_RaycheckActive.Contains(causeObject)) {
            // rbuhrbhuiihrbuashuikbdrshikuagdr
            if (m_Player == hitObject) {
                // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                if (player_ignore_list.find(causeObject) == player_ignore_list.end()) {
                    player_ignore_list.insert(causeObject);
                    m_Healths.Get(m_Player)->current -= 1;
                    m_PlayerHitTimer = 0.5f;
                    m_pAudio->play(SoundIndex::PlayerImpact1, staff_tip, 0.45f, 0.5);
                }
            }

            if (m_NPCsActive.Contains(hitObject)) {
                // Ignore duplicate hits, previous solution did not work well so this juryrig should fix it for final release :(
                NPC* npc = m_NPCs.Get(hitObject);
                if (npc->IgnoreList.find(causeObject) == npc->IgnoreList.end()) {
                    npc->IgnoreList.insert(causeObject);
                    m_Healths.Get(hitObject)->current -= 2;
                    m_pAudio->play(npc->ImpactSound, npc->LastPosition, 0.55f, 0.5);
                }
            }
        }
    }

    // Clear old collisions once were done with the current pass.
    m_CurrentCollisions.Clear();

    // Check to update collision pairs every four frames.
    m_CollisionPairs.Clear();

    // Check Collision Table Validity
    m_CurrentStep++;
    if (m_CurrentStep % 24 == 0) {
        m_CurrentStep = 0;
        player_ignore_list.clear();
        Ecs::ApplyEvery(m_NPCsActive, [=](Entity e) {
            NPC* npc = m_NPCs.Get(e);
            npc->IgnoreList.clear();
        });

        Ecs::ApplyEvery(m_ProjectilesActive, [=](Entity e) {
            SimProjectile* projectile = m_Projectiles.Get(e);
            projectile->IgnoreList.clear();
        });
    }

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

    static bool alloced = false;
    static btDefaultCollisionConfiguration* CurrentConfiguration = 0;
    static btCollisionDispatcher* CurrentDispatcher = 0;
    static btDbvtBroadphase* CurrentBroadphaseCache = 0;
    static btSequentialImpulseConstraintSolver* CurrentSolver = 0;

    if (!alloced) {
        alloced = true;
        CurrentConfiguration = new btDefaultCollisionConfiguration();
        CurrentDispatcher = new btCollisionDispatcher(CurrentConfiguration);
        CurrentBroadphaseCache = new btDbvtBroadphase();
        CurrentSolver = new btSequentialImpulseConstraintSolver;
        m_pDynamicsWorld = new btDiscreteDynamicsWorld(CurrentDispatcher, CurrentBroadphaseCache, CurrentSolver, CurrentConfiguration);
    }

    // Set other attributes
    m_pDynamicsWorld->setGravity(btVector3(0.0, -5000.0, 0.0));

    // Static Self Assignment
    Self = this;

    // Collision Callback
    m_pDynamicsWorld->setInternalTickCallback(PhysicsCollisionCallBack);

    // m_Player Rigidbody | (Note) : Create this first, as the m_Player is currently indexed as [0] in the collision table.
    {
        btRigidBody* rb = CreateBoxObject(Vec3(300.f, 400.f, 300.f), IndexToWorld(1, 1), 1.0f, 0.5f, PLAYER_PHYSICS_GROUP, PLAYER_PHYSICS_MASK); //CreateCapsuleObject(250.0f, 300.0f, IndexToWorld(1, 1), 1.0f, 0.5f, PLAYER_PHYSICS_GROUP, PLAYER_PHYSICS_MASK);
        RBSetMassFriction(rb, 1.0, 0.1);
        RBSetCcd(rb, 1e-7, 200.0f);
        m_Player = m_RigidBodyMap.at(rb);
    }

    // Set initial step
    m_CurrentStep = 0;
}