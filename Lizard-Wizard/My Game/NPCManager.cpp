/// Inclusions
#include "Game.h"
#include "Interpolation.h"

/* Note(Ethan) : This is a prototype test for the NPC system, I will reduce comments once we get to making the release version.
*
            '   '
            _____

     wachu looking at bud

*/

btMatrix3x3 NpcLookAt(btRigidBody* npc, btRigidBody* player) {
    Vec3 origin = npc->getWorldTransform().getOrigin();
    Vec3 lookAt = player->getWorldTransform().getOrigin() + player->getLinearVelocity() / 4;
    return *(btMatrix3x3*)&XMMatrixLookAtLH(origin, lookAt, Vec3(0, 1.0f, 0));
};

void SetNPCRender(btRigidBody* npcBody, Vec3 origin, btMatrix3x3 basis) {
    npcBody->getWorldTransform().setBasis(basis);
    npcBody->getWorldTransform().setOrigin(origin);
}

ModelInstance GetNPCModel(btRigidBody* body) {
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 origin = body->getWorldTransform().getOrigin();
    Quat rotation = body->getWorldTransform().getRotation();
    Vec3 scale = boxShape->getHalfExtentsWithMargin();

    ModelInstance instance = {};
    instance.model = ModelIndex::Obelisk;
    instance.texture = TextureIndex::White;
    instance.world = MoveRotateScaleMatrix(origin, XMQuaternionNormalize(rotation), scale);

    return instance;
}

void CGame::Animate(Entity e) {
    Animation* currentAnimation = m_Animations.Get(e);
    NPC* currentNPC = m_NPCs.Get(e);
    btRigidBody* npcBody = *m_RigidBodies.Get(e);
    btRigidBody* playerBody = *m_RigidBodies.Get(m_Player);
    if (currentAnimation->steps == currentAnimation->maxSteps) {
        m_Animations.Remove(e);
        //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());

    	btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    	SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);

        currentNPC->State = NPCState::SEARCHING;
    } else {
        currentAnimation->percent = currentAnimation->steps / currentAnimation->maxSteps;
        currentAnimation->steps = currentAnimation->steps + 1.0f;

        Vec3 origin = LinearLerp(currentAnimation->beginPos, currentAnimation->endPos, currentAnimation->percent);
        //SetNPCRender(npcBody, origin, npcBody->getWorldTransform().getBasis());

    	btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    	SetNPCRender(npcBody, origin, mat);
    }
}

void CGame::Sleep(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());
    if (distance < 7500.0f) {
        currentNPC->State = NPCState::ATTACKING;
    }

    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
} 

void CGame::Wander(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    std::vector<Point2> path = Pathfind(WorldToIndex(npcBody->getWorldTransform().getOrigin()), WorldToIndex(playerBody->getWorldTransform().getOrigin()));
    Vec3 goal = playerBody->getWorldTransform().getOrigin();
    if (path.size() > 0) {
        goal = IndexToWorld(path[path.size() - 1].first, path[path.size() - 1].second);
    }

    u32 max_tries = 5;
    while (max_tries > 0) {
        currentNPC->QueuedMovement = npcBody->getWorldTransform().getOrigin() + Vec3((GameRandom::Randf32() * 1000.0f) * Vec3(BiasedPointIn2DPlane(0.5f, npcBody->getWorldTransform().getOrigin(), goal)));
        Point2 p = WorldToIndex(currentNPC->QueuedMovement);
        currentNPC->QueuedMovement.Print();
        if (CheckBounds(p.first, p.second) && m_GameMap[p.first][p.second]) { break; }
        max_tries -= 1;
    }

    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);
    currentNPC->State = NPCState::MOVING;
}

void CGame::Move(Entity e, Vec3 moveTo) {
    // Ensuring object is placed correctly in world.
    //btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);
    btRigidBody* playerBody = *m_RigidBodies.Get(m_Player);

    //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);

    // Animation
    Animation newAnimation;

    newAnimation.beginPos = npcBody->getWorldTransform().getOrigin();
    newAnimation.beginRot = Vec3(0);

    newAnimation.endPos = moveTo;
    newAnimation.endRot = Vec3(0);

    newAnimation.maxSteps = 20.0f;
    newAnimation.steps = 0.0f;

    newAnimation.time = 5.0f;
    newAnimation.percent = 0.0f;

    m_Animations.AddExisting(e, newAnimation);
    m_NPCs.Get(e)->State = NPCState::ANIMATING;
}

void CGame::Pathfind(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    std::vector<Point2> path = Pathfind(WorldToIndex(npcBody->getWorldTransform().getOrigin()), WorldToIndex(playerBody->getWorldTransform().getOrigin()));

    if (path.size() > 1) {
        currentNPC->QueuedMovement = IndexToWorld(path[path.size() - 1].first, path[path.size() - 1].second) + 500.0f * Vec3(GameRandom::Randf32() - 0.5f, 0.0f, GameRandom::Randf32() - 0.5f);
    } else {
        currentNPC->QueuedMovement = npcBody->getWorldTransform().getOrigin();
    }

    //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);
    currentNPC->State = NPCState::MOVING;
}

void CGame::Attack(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4;
    btMatrix3x3 newMat = NpcLookAt(npcBody, playerBody); 

    f32 waitTimer;

    SetNPCRender(npcBody, origin, newMat);

    f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());
    if (distance < 7500.0f) {
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
                SoundIndex::FireImpact1,
                true
            );
        }
    } else {
        currentNPC->State = NPCState::SEARCHING;
    }
}

void CGame::Search(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    if (currentNPC->SearchAttempts >= 5) {
        if (currentNPC->SearchAttempts >= 10) {
            currentNPC->State = NPCState::SLEEPING;
            currentNPC->SearchAttempts = 0;
        }
        currentNPC->State = NPCState::PATHFINDING;
        currentNPC->SearchAttempts++;
    } else {
        f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());
        if (distance < 4000.0f) {
            currentNPC->State = NPCState::ATTACKING;
            currentNPC->SearchAttempts = 0;
        } else if (distance < 15000.0f) {
            currentNPC->State = NPCState::WANDER;
            currentNPC->SearchAttempts++;
        }
    }
    //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);
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
void CGame::DirectNPC(Entity e) {

    // Update associated light position
    btRigidBody* body = *m_RigidBodies.Get(e);

    // Sean: this made me very sad
    m_NPCs.Get(e)->LastPosition = body->getWorldTransform().getOrigin();
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&m_NPCs.Get(e)->LastPosition;

    switch (m_NPCs.Get(e)->State)
    {
        case NPCState::SLEEPING :
            Sleep(e);
            break;
        case NPCState::ANIMATING :
            Animate(e);
            break;
        case NPCState::WANDER :
            Wander(e);
            break;
        case NPCState::MOVING :
            Move(e, m_NPCs.Get(e)->QueuedMovement);
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
void CGame::PlaceNPC(Vec3 startPos, Vec3 lookDirection) {
    Entity e = m_NPCsCache.RemoveTail();
    m_NPCsActive.AddExisting(e);

    btRigidBody* body = *m_RigidBodies.Get(e);
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 newPos = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.0f;

    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getWorldTransform().setOrigin(Vec3(newPos.x, -1000.0f, newPos.z));
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, 2, 0b00001);
    m_Timers.AddExisting(e, 10.0f);
    body->activate();

    m_pRenderer->lights.Get(e)->position = *(Vec4*)&body->getWorldTransform().getOrigin();

    // Add model to world
    (*m_ModelInstances.Get(e)).world = 
        MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), 
            *(Quat*)&body->getWorldTransform().getRotation(), 
            boxShape->getHalfExtentsWithMargin()
        );
    m_ModelsActive.AddExisting(e);

    SetNPCRender(body, body->getWorldTransform().getOrigin(), body->getWorldTransform().getBasis());
}

// Places a cached NPC.
void CGame::PlaceNPC2(Vec3 startPos) {
    Entity e = m_NPCsCache.RemoveTail();
    m_NPCsActive.AddExisting(e);

    btRigidBody* body = *m_RigidBodies.Get(e);
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 newPos = Vec3(startPos.x, startPos.y, startPos.z);

    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set attributes.
    body->getWorldTransform().setOrigin(Vec3(newPos.x, -1000.0f, newPos.z));
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, 2, 0b00001);
    m_Timers.AddExisting(e, 10.0f);
    body->activate();

    m_pRenderer->lights.Get(e)->position = *(Vec4*)&body->getWorldTransform().getOrigin();

    // Add model to world
    (*m_ModelInstances.Get(e)).world = 
        MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), 
            *(Quat*)&body->getWorldTransform().getRotation(), 
            boxShape->getHalfExtentsWithMargin()
        );
    m_ModelsActive.AddExisting(e);

    SetNPCRender(body, body->getWorldTransform().getOrigin(), body->getWorldTransform().getBasis());
}

// Strips an NPC and re-adds them to the cache.
void CGame::StripNPC() {
    Entity e = m_NPCsActive.RemoveTail();
    m_NPCsCache.AddExisting(e);
    btRigidBody* body = *m_RigidBodies.Get(e);
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    // Set attributes.
    Vec3 orig = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    RBTeleport(body, orig);

    // Move lights
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&orig;

    // Disable rendering
    m_ModelsActive.Remove(e);

    // Removes rigidbody
    RemoveRigidBody(body);

    RBSetMassFriction(body, 0.0f, 0.0f);
}


void CGame::InitializeNPCs() {
    for every(index, NPC_CACHE_SIZE) {
        // Create Rigidbody, get ECS identifier, and create new NPC
        btRigidBody* newBody = CreateBoxObject(Vec3(150.f, 150.f, 150.f), Vec3(FLT_MAX, FLT_MAX, FLT_MAX), 0.0f, 0.0f, 3, 0b00001);
        Entity e = m_RigidBodyMap.at(newBody);
        NPC newNPC = NPC();
        RemoveRigidBody(newBody);

        // Prepare NPC
        newNPC.SearchAttempts = 0;

        // Prepare light
        Light newLight = { Vec4(99999.f,99999.f,99999.f,0), Vec4{10.0f, 30.0f, 500.0f, 0} };

        // Prepare model
        m_ModelInstances.AddExisting(e, GetNPCModel(*m_RigidBodies.Get(e)));

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_NPCs.AddExisting(e, newNPC);
        m_NPCsCache.AddExisting(e);
    }
}