// Inclusions
#include "Game.h"

/* Note(Ethan) : This is the physics manager, it helps us clean up the game.cpp and further simplify how we use Bullet3.
*
            '   '
            _____

     wachu looking at bud

*/
static CGame* Self;

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
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    rbInfo.m_friction = friction;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setAngularFactor(Vec3(0., 0., 0.));
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
    const u32 numManifolds = (u32)p->getDispatcher()->getNumManifolds();

    for every(manifold, numManifolds) {
        btDispatcher* pDispatcher = p->getDispatcher();
        btPersistentManifold* pManifold = pDispatcher->getManifoldByIndexInternal(manifold);

        // Get bodys
        btRigidBody* pBody0 = const_cast<btRigidBody*>(btRigidBody::upcast(pManifold->getBody0()));
        btRigidBody* pBody1 = const_cast<btRigidBody*>(btRigidBody::upcast(pManifold->getBody1()));
        Entity pEntity0 = Self->m_RigidBodyMap.at(pBody0);
        Entity pEntity1 = Self->m_RigidBodyMap.at(pBody1);

        const int numContacts = pManifold->getNumContacts();
        if (numContacts > 0 && !Self->m_CurrentCollisions.Contains(pEntity0)) { //guard
            for every(contact, numContacts){
                btManifoldPoint& pt = pManifold->getContactPoint(contact);
                Self->m_CurrentCollisions.AddExisting(pEntity0);
                Self->m_CurrentCollisions.AddExisting(pEntity1);
            }
        }
    }
}

void CGame::CustomPhysicsStep() {
    // This plays a sound for all collisions. (NOTE) Ethan : Will be moved into CGame once I move all physics stepping into a new custom function.
    for every(index, m_CurrentCollisions.Size()) {
        Entity e = m_CurrentCollisions.Entities()[index];
        Vec3 pos = (*m_RigidBodies.Get(e))->getWorldTransform().getOrigin();

        // Collision event example
        // (Warning) Ethan : This can get fairly expensive if we stack if-statements and switch-statements so try to tie everything together in the ECS to remain efficient.
        if (m_ProjectilesActive.Contains(e)) {
            //printf("Projectile collision detected at : (%f, %f, %f)\n", pos.x, pos.y, pos.z);
            //std::cout << "Collision ID :" << e.id << std::endl;
            //m_pAudio->play(SoundIndex::Clang, pos, 0.25, 0.0);
        }
        //m_pAudio->play(SoundIndex::Clang, pos, 0.25, 0.0);
    }
    m_CurrentCollisions.Clear();
}

// Helper Functions, these will be helpful when we don't want to keep passing the dynamics world to managers.
void CGame::RemoveRigidBody(btRigidBody* body) {
    m_pDynamicsWorld->removeRigidBody(body);
}


void CGame::AddRigidBody(btRigidBody* body, i32 group, i32 mask) {
    m_pDynamicsWorld->addRigidBody(body, group, mask);
}


void CGame::DestroyPhysicsOBject(btCollisionShape* shape) {
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
        btRigidBody* rb = CreateCapsuleObject(100.0f, 250.0f, Vec3(0, 1500, 0), 1.0f, 0.5f, 2, 0b00001);
        m_RigidBodies.AddExisting(m_Player, rb);
    }
}