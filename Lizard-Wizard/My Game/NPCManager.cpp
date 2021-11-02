// Inclusions
#include "Game.h"

/* Note(Ethan) : This is a prototype test for the NPC system, I will reduce comments once we get to making the release version.
*
            '   '
            _____

     wachu looking at bud

*/

void CGame::Sleep(Entity e) {
    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4;

    btTransform newTransform;
    newTransform.setBasis(*(btMatrix3x3*)&XMMatrixLookAtLH(origin, Vec3(1.0f,0,0), Vec3(0, 1.0f, 0)));
    newTransform.setOrigin(npcBody->getWorldTransform().getOrigin());

    npcBody->getMotionState()->setWorldTransform(newTransform);
    npcBody->setWorldTransform(newTransform);

    f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());
    if (distance < 5000.0f) {
        printf("Player within range. Distance : %F\n", distance);
        Attack(e);
    }
} 

void CGame::Wander(Entity e) {
    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4;

    btTransform newTransform;
    newTransform.setBasis(*(btMatrix3x3*)&XMMatrixLookAtLH(origin, Vec3(1.0f, 0, 0), Vec3(0, 1.0f, 0)));
    newTransform.setOrigin(RandomPointInRadius(npcBody->getWorldTransform().getOrigin(), 100.0f));

    npcBody->getMotionState()->setWorldTransform(newTransform);
    npcBody->setWorldTransform(newTransform);
}

void CGame::Move(Entity e) {
    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4;

    btTransform newTransform;
    newTransform.setBasis(*(btMatrix3x3*)&XMMatrixLookAtLH(origin, Vec3(1.0f, 0, 0), Vec3(0, 1.0f, 0)));
    newTransform.setOrigin(RandomPointInRadius(npcBody->getWorldTransform().getOrigin(), 100.0f));

    npcBody->getMotionState()->setWorldTransform(newTransform);
    npcBody->setWorldTransform(newTransform);
}

void CGame::Pathfind(Entity e) {
    // (Note) Ethan : I have this implanted in the diagram but I need a more complete generation system before I can implement this.
}

void CGame::Attack(Entity e) {
    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);
    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4;
    btTransform newTransform;
    newTransform.setBasis(*(btMatrix3x3*)&XMMatrixLookAtLH(origin, lookAt, Vec3(0, 1.0f, 0)));
    f32 waitTimer;
    newTransform.setOrigin(npcBody->getWorldTransform().getOrigin());
    npcBody->getMotionState()->setWorldTransform(newTransform);
    npcBody->setWorldTransform(newTransform);
    waitTimer = *m_Timers.Get(e);
    if (waitTimer < 0.0f) {
        m_Timers.Remove(e);
        m_Timers.AddExisting(e, 3.0);
        GenerateSimProjectile(
            npcBody,
            npcBody->getWorldTransform().getOrigin(),
            -XMVector3Normalize(origin - lookAt),
            1,
            20000.0,
            0.05,
            Colors::LavenderBlush,
            true
        );
    }
}

void CGame::Search(Entity e) {

}

/*
void CGame::DetermineBehavior(Entity e) {
    switch (m_NPCs.Get(e)->Behavior) {
    case NPCBehavior::MELEE:
        printf("Melee Behavior\n");
        break;
    case NPCBehavior::RANGED:
        printf("Ranged Behavior\n");
        break;
    case NPCBehavior::TURRET:
    default:
        return;
    }
}
*/

// Contains most of the logical code for handling NPCs
void CGame::DirectNPC(Entity e, btRigidBody* player) {
    switch (m_NPCs.Get(e)->State)
    {
        case NPCState::SLEEPING :
            Sleep(e);
            break;
        case NPCState::WANDER :
            Wander(e);
            break;
        case NPCState::MOVING :
            Move(e);
            break;
        case NPCState::ATTACKING :
            Attack(e);
            break;
        case NPCState::SEARCHING :
            Search(e);
            break;
        case NPCState::PATHFINDING :
            Pathfind(e);
            break;
        default:
            return;
    }
}

// Places a cached NPC.
void CGame::PlaceNPC(Entity e, Vec3 startPos, Vec3 lookDirection) {
    m_NPCsCache.RemoveTail();
    m_NPCsActive.AddExisting(e);
    btRigidBody* body = *m_RigidBodies.Get(e);
    btTransform trans;
    Vec3 newPos = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.0f;
    trans.setOrigin(Vec3(newPos.x, 500.0f, newPos.z));
    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getMotionState()->setWorldTransform(trans);
    body->setWorldTransform(trans);
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, 2, 0b00001);
    m_Timers.AddExisting(e, 10.0f);
    body->activate();
}

// Strips an NPC and re-adds them to the cache.
void CGame::StripNPC(Entity e) {
    m_NPCsActive.Remove(e);
    m_NPCsCache.AddExisting(e);
    btRigidBody* body = *m_RigidBodies.Get(e);

    // Removes rigidbody from world to edit.
    RemoveRigidBody(body);
    body->clearForces();

    btTransform trans;
    trans.setOrigin(Vec3(99999.f, 99999.f, 99999.f));
    f32 mass = 0.0f;
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getMotionState()->setWorldTransform(trans);
    body->setWorldTransform(trans);
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, 2, 0b00001);

    // Set light position
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();
}


void CGame::InitializeNPCs() {
    for every(index, NPC_CACHE_SIZE) {

        // Create Rigidbody, get ECS identifier, and create new NPC
        btRigidBody* newBody = CreateBoxObject(Vec3(150.f, 150.f, 150.f), Vec3(99999.f, 99999.f, 99999.f), 0.0f, 0.0f, 3, 0b00001);
        Entity e = m_RigidBodyMap.at(newBody);
        NPC newNPC = NPC();
        RemoveRigidBody(newBody);

        // Prepare light
        Light newLight = { Vec4(99999.f,99999.f,99999.f,0), Vec4{10.0f, 30.0f, 500.0f, 0} };

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_NPCs.AddExisting(e, newNPC);
        m_NPCsCache.AddExisting(e);
    }
}