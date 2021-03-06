/// Inclusions
#include "Game.h"
#include <ComponentIncludes.h>
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

void CGame::BossAttack1(Entity e) { // Ice attack, 3 projectiles in a 15 degree spread. Each deals 2 damage.
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    i32 numProjectiles = 3;

    f32 offset = 0.225f;
    f32 angle = -0.225f;
    f32 rotX;
    f32 rotZ;
    Vec3 origin = npcBody->getWorldTransform().getOrigin() + Vec3(800.0f, 750.0f, 0.0f);
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 3.5;
    Vec3 face = -XMVector3Normalize(origin - lookAt);
    Vec2 rotVec;

    for every(index, numProjectiles) {
        rotVec = Vec2(face.x, face.z);
        rotX = (rotVec.x * cos(angle)) - (rotVec.y * sin(angle));
        rotZ = (rotVec.x * sin(angle)) + (rotVec.y * cos(angle));
        face.x = rotX; face.z = rotZ;
        angle += offset;

        GenerateSimProjectile(
            npcBody,
            origin,
            face,
            1,
            14000.0,
            0.0,
            Vec4(20.0f, 150.0f, 500.0f, 0) / 105.0f,
            SoundIndex::IceImpact1,
            true,
            PROJECTILE_PHYSICS_GROUP,
            NPC_PROJECTILE_PHYSICS_MASK,
            1
        );
        face = -XMVector3Normalize(origin - lookAt);
        m_pAudio->play(SoundIndex::IceCast, staff_tip, 0.35f, 0.5);
    }
}

void CGame::BossAttack2(Entity e) { // Fire attack, 5 projectiles in a 5 degree spread. Each deals 1 damage.
    i32 numProjectiles = 6;
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    f32 offset = 0.15f;
    f32 angle = -0.30f;
    f32 rotX;
    f32 rotZ;
    Vec3 origin = npcBody->getWorldTransform().getOrigin() + Vec3(-800.0f, 750.0f, 0.0f);
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 3.5;
    Vec3 face = -XMVector3Normalize(origin - lookAt);
    Vec2 rotVec;

    for every(index, numProjectiles) {
        rotVec = Vec2(face.x, face.z);
        rotX = (rotVec.x * cos(angle)) - (rotVec.y * sin(angle));
        rotZ = (rotVec.x * sin(angle)) + (rotVec.y * cos(angle));
        face.x = rotX; face.z = rotZ;
        angle += offset;

        GenerateSimProjectile(
            npcBody,
            origin,
            face,
            1,
            14000.0,
            0.0,
            Vec4(500.0f, 40.0f, 100.0f, 0) / 105.0f,
            SoundIndex::FireImpact1,
            true,
            PROJECTILE_PHYSICS_GROUP,
            NPC_PROJECTILE_PHYSICS_MASK,
            1
        );
        face = -XMVector3Normalize(origin - lookAt);
        m_pAudio->play(SoundIndex::FireCast, staff_tip, 0.65f, 0.5);
    }
}

void CGame::BossAttack3(Entity e) { // Lightning attack, 3 bolts of lightning in a random assortment. Each deals 1 damage. Bounces twice.
    i32 numProjectiles = 3;
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    f32 offset = 0.4f;
    f32 angle = -0.4f;
    f32 rotX;
    f32 rotZ;
    Vec3 origin = npcBody->getWorldTransform().getOrigin() + Vec3(0.0f, 750.0f, 0.0f);
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin();
    Vec3 face = -XMVector3Normalize(origin - lookAt);
    Vec2 rotVec;

    for every(index, numProjectiles) {
        rotVec = Vec2(face.x, face.z);
        rotX = (rotVec.x * cos(angle)) - (rotVec.y * sin(angle));
        rotZ = (rotVec.x * sin(angle)) + (rotVec.y * cos(angle));
        face.x = rotX; face.z = rotZ;
        angle += offset;

        GenerateRayProjectile(
            *m_RigidBodies.Get(m_Player),
            origin,
            face,
            1,
            2,
            0.05,
            Colors::IndianRed,
            false,
            true,
            PROJECTILE_PHYSICS_GROUP,
            PLAYER_PROJECTILE_PHYSICS_MASK,
            1
        );
        m_pAudio->play(SoundIndex::LightningCast, staff_tip, 0.025f, 0.5);
        face = -XMVector3Normalize(origin - lookAt);
    }
}

b8 CGame::PlayerInView(btRigidBody* body) {
    Vec3 from = body->getWorldTransform().getOrigin();
    Vec3 to = (*m_RigidBodies.Get(m_Player))->getWorldTransform().getOrigin();
    btCollisionWorld::ClosestRayResultCallback rayResults(from, to);
    rayResults.m_collisionFilterGroup = PROJECTILE_PHYSICS_GROUP;
    rayResults.m_collisionFilterMask = PLAYER_PROJECTILE_PHYSICS_MASK;
    m_pDynamicsWorld->rayTest(from, to, rayResults);

    if (rayResults.hasHit()) {
        // Warning (Ethan) : DO NOT EDIT at this pointer.
        btCollisionObject* hitObject = const_cast<btCollisionObject*>(rayResults.m_collisionObject);
        return false;
    }

    return true;
}

ModelInstance GetNPCModel(btRigidBody* body) {
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 origin = body->getWorldTransform().getOrigin();
    Quat rotation = body->getWorldTransform().getRotation();
    Vec3 scale = boxShape->getHalfExtentsWithMargin();

    scale = Vec3(scale.x / 1.5f, scale.y / 4.0f, scale.z / 1.5f);

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
    if (distance < NPC_ATTACK_RADIUS && PlayerInView(npcBody)) {
        currentNPC->State = NPCState::ATTACKING;
    }

    if (PlayerInView(npcBody)) {
        currentNPC->State = NPCState::WANDER;
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
        if (CheckBounds(p.first, p.second) && m_GameMap[p.first][p.second]) { break; }
        max_tries -= 1;
    }
    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    origin.y = currentNPC->SpawnOffset.y;
    SetNPCRender(npcBody, origin, npcBody->getWorldTransform().getBasis());
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
        Vec3 moveTo = IndexToWorld(path[path.size() - 1].first, path[path.size() - 1].second) + 500.0f * Vec3(GameRandom::Randf32() - 0.5f, currentNPC->SpawnOffset.y, GameRandom::Randf32() - 0.5f);
        moveTo.y = currentNPC->SpawnOffset.y;
        currentNPC->QueuedMovement = moveTo;
    } else {
        currentNPC->QueuedMovement = npcBody->getWorldTransform().getOrigin();
    }

    //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);
    currentNPC->State = NPCState::MOVING;

    f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());
    if (distance < NPC_ATTACK_RADIUS && PlayerInView(npcBody)) {
        currentNPC->State = NPCState::ATTACKING;
    }

}

void CGame::Attack(Entity e) {
    NPC* currentNPC = m_NPCs.Get(e);

    btRigidBody* playerBody = *(m_RigidBodies.Get(m_Player));
    btRigidBody* npcBody = *m_RigidBodies.Get(e);

    Vec3 origin = npcBody->getWorldTransform().getOrigin();
    Vec3 lookAt = playerBody->getWorldTransform().getOrigin() + playerBody->getLinearVelocity() / 4.5;
    btMatrix3x3 newMat = NpcLookAt(npcBody, playerBody); 

    f32 waitTimer;

    SetNPCRender(npcBody, origin, newMat);

    f32 distance = npcBody->getWorldTransform().getOrigin().distance(playerBody->getWorldTransform().getOrigin());

    // Normal Enemies
    if (currentNPC->Type == NPCType::BOSS) {
        if (distance < NPC_ATTACK_RADIUS && PlayerInView(npcBody)) {
            waitTimer = *m_Timers.Get(e);
            if (waitTimer < 0.0f) {
                m_Timers.Remove(e);
                m_Timers.AddExisting(e, 4.0);

                switch (currentNPC->WeaponCycle)
                {
                    case 1:
                        BossAttack2(e);
                        currentNPC->WeaponCycle += 1;
                        break;

                    case 2:
                        BossAttack3(e);
                        currentNPC->WeaponCycle = 0;
                        break;
                    default:
                        BossAttack1(e);
                        currentNPC->WeaponCycle += 1;
                        break;
                }
            }
            currentNPC->State = NPCState::WANDER;
        } else {
            currentNPC->State = NPCState::SEARCHING;
        }
    } else {
        if (distance < NPC_ATTACK_RADIUS && PlayerInView(npcBody)) {
            waitTimer = *m_Timers.Get(e);
            if (waitTimer < 0.0f) {
                m_Timers.Remove(e);
                m_Timers.AddExisting(e, 3.0);

                GenerateSimProjectile(
                    npcBody,
                    npcBody->getWorldTransform().getOrigin(),
                    -XMVector3Normalize(origin - lookAt),
                    1,
                    15000.0,
                    0.05,
                    currentNPC->LightColor / 105.0f,
                    currentNPC->ProjectileSound,
                    true,
                    PROJECTILE_PHYSICS_GROUP,
                    NPC_PROJECTILE_PHYSICS_MASK,
                    1
                );
                m_pAudio->play(currentNPC->CastSound, origin, 1.85f, 0.5);
            }
        }
        else {
            currentNPC->State = NPCState::SEARCHING;
        }
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
        if (distance < NPC_ATTACK_RADIUS && PlayerInView(npcBody)) {
            currentNPC->State = NPCState::ATTACKING;
            currentNPC->SearchAttempts = 0;
        } else if (distance < NPC_DETECTION_RADIUS) {
            currentNPC->State = NPCState::WANDER;
            currentNPC->SearchAttempts++;
        }
    }
    //SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), npcBody->getWorldTransform().getBasis());
    btMatrix3x3 mat = NpcLookAt(npcBody, playerBody);
    SetNPCRender(npcBody, npcBody->getWorldTransform().getOrigin(), mat);
}

// Contains most of the logical code for handling NPCs
void CGame::DirectNPC(Entity e) {

    // Update associated light position
    btRigidBody* body = *m_RigidBodies.Get(e);

    // Sean: this made me very sad
    m_NPCs.Get(e)->LastPosition = body->getWorldTransform().getOrigin();
    m_pRenderer->lights.Get(e)->position = *(Vec4*)&m_NPCs.Get(e)->LastPosition;

    Health* health = m_Healths.Get(e);
    m_pRenderer->lights.Get(e)->color = m_NPCs.Get(e)->LightColor * ((f32)health->current / (f32)health->max);

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
void CGame::PlaceNPC(Vec3 startPos, Vec3 lookDirection, NPCType::e npcType) {
    Entity e = m_NPCsCache.RemoveTail();
    NPC* currNPC = m_NPCs.Get(e);
    NPC* baseNPC = &m_NPCStatsMap.at(npcType);
    m_NPCsActive.AddExisting(e);

    btRigidBody* body = *m_RigidBodies.Get(e);
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 newPos = Vec3(startPos.x, startPos.y, startPos.z) + lookDirection * 5000.0f;

    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set NPC Stuff
    currNPC->CastSound = baseNPC->CastSound;
    currNPC->DeathSound = baseNPC->DeathSound;
    currNPC->ImpactSound = baseNPC->ImpactSound;
    currNPC->ProjectileSound = baseNPC->ProjectileSound;
    currNPC->SpawnOffset = baseNPC->SpawnOffset;
    currNPC->LightColor = baseNPC->LightColor;
    currNPC->Type = baseNPC->Type;

    // Set attributes.
    body->getWorldTransform().setOrigin(Vec3(newPos.x, currNPC->SpawnOffset.y, newPos.z));
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, NPC_PHYSICS_GROUP, NPC_PHYSICS_MASK);
    m_Timers.AddExisting(e, 10.0f);
    body->activate();

    m_pRenderer->lights.Get(e)->position = *(Vec4*)&body->getWorldTransform().getOrigin();

    // Set stats / health
    m_Healths.Get(e)->current = baseNPC->BaseHealth;
    m_Healths.Get(e)->max = baseNPC->BaseHealth;


    // Add model to world
    (*m_ModelInstances.Get(e)).world = 
        MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), 
            *(Quat*)&body->getWorldTransform().getRotation(), 
            boxShape->getHalfExtentsWithMargin()
        );

    (*m_ModelInstances.Get(e)).model = baseNPC->Model;
    m_ModelsActive.AddExisting(e);

    SetNPCRender(body, body->getWorldTransform().getOrigin(), body->getWorldTransform().getBasis());
}

// Places a cached NPC.
Entity CGame::PlaceNPC2(Vec3 startPos, NPCType::e npcType) {
    Entity e = m_NPCsCache.RemoveTail();
    NPC* currNPC = m_NPCs.Get(e);
    NPC* baseNPC = &m_NPCStatsMap.at(npcType);
    m_NPCsActive.AddExisting(e);

    btRigidBody* body = *m_RigidBodies.Get(e);
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    Vec3 newPos = Vec3(startPos.x, startPos.y, startPos.z);

    f32 mass = 0.0f; // For now were making this static until we get a proper NPC movement system.
    f32 friction = 0.0f;
    btVector3 inertia;

    // Set NPC Stuff
    currNPC->CastSound = baseNPC->CastSound;
    currNPC->DeathSound = baseNPC->DeathSound;
    currNPC->ImpactSound = baseNPC->ImpactSound;
    currNPC->ProjectileSound = baseNPC->ProjectileSound;
    currNPC->SpawnOffset = baseNPC->SpawnOffset;
    currNPC->LightColor = baseNPC->LightColor;
    currNPC->Type = baseNPC->Type;

    // Set attributes.
    body->getWorldTransform().setOrigin(Vec3(newPos.x, currNPC->SpawnOffset.y, newPos.z));
    body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    body->setMassProps(mass, inertia);
    body->setFriction(friction);

    // Re-add regidbody to world after edit.
    AddRigidBody(body, NPC_PHYSICS_GROUP, NPC_PHYSICS_MASK);
    m_Timers.AddExisting(e, 10.0f);
    body->activate();

    m_pRenderer->lights.Get(e)->position = *(Vec4*)&body->getWorldTransform().getOrigin();

    // Set stats / health
    m_Healths.Get(e)->current = baseNPC->BaseHealth;
    m_Healths.Get(e)->max = baseNPC->BaseHealth;

    // Add model to world
    (*m_ModelInstances.Get(e)).world = 
        MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), 
            *(Quat*)&body->getWorldTransform().getRotation(), 
            boxShape->getHalfExtentsWithMargin()
     );

    (*m_ModelInstances.Get(e)).model = baseNPC->Model;
    m_ModelsActive.AddExisting(e);

    SetNPCRender(body, body->getWorldTransform().getOrigin(), body->getWorldTransform().getBasis());

    return e;
}

// Manually strips an NPC
void CGame::ForceStripNPC(Entity e) {
    m_NPCsActive.Remove(e);
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
    NPC npc;

    // Obelisk (FIRE) (Default Configuration of NPC)
    npc.Type = NPCType::OBELISK;
    npc.BaseHealth = 4;
    npc.LightColor = Vec4(500.0f, 40.0f, 100.0f, 0);
    npc.CastSound = SoundIndex::EnemyCast1;
    npc.DeathSound = SoundIndex::ObeliskDeath;
    npc.ProjectileSound = SoundIndex::FireImpact1;
    npc.ImpactSound = SoundIndex::EnemyImpactMetal1;
    npc.Model = ModelIndex::Obelisk;
    npc.SpawnOffset = Vec3(0.0f, -500.0f, 0.0f);
    m_NPCStatsMap.insert(std::make_pair(NPCType::OBELISK, npc));

    // Crystal (ICE)
    npc.Type = NPCType::CRYSTAL;
    npc.BaseHealth = 8;
    npc.LightColor = Vec4(20.0f, 150.0f, 500.0f, 0);
    npc.CastSound = SoundIndex::EnemyCast2;
    npc.DeathSound = SoundIndex::CrystalDeath;
    npc.ProjectileSound = SoundIndex::IceImpact1;
    npc.ImpactSound = SoundIndex::EnemyImpactCrystal1;
    npc.Model = ModelIndex::Crystal;
    npc.SpawnOffset = Vec3(0.0f, 0.0f, 0.0f);
    m_NPCStatsMap.insert(std::make_pair(NPCType::CRYSTAL, npc));

    // Boss (Variation of ICE and FIRE attacks)
    npc.Type = NPCType::BOSS;
    npc.BaseHealth = 50;
    npc.LightColor = Vec4(1000.0f, 30.0f, 1000.0f, 0);
    npc.CastSound = SoundIndex::EnemyCast2;
    npc.DeathSound = SoundIndex::ObeliskDeath;
    npc.ProjectileSound = SoundIndex::LightningCast;
    npc.ImpactSound = SoundIndex::EnemyImpactMetal2;
    npc.Model = ModelIndex::Boss;
    npc.SpawnOffset = Vec3(0.0f, -500.0f, 0.0f);
    m_NPCStatsMap.insert(std::make_pair(NPCType::BOSS, npc));

    // Create all NPCs
    for every(index, NPC_CACHE_SIZE) {
        // Create Rigidbody, get ECS identifier, and create new NPC
        btRigidBody* newBody = CreateBoxObject(Vec3(450.f, 600.f, 300.f), Vec3(FLT_MAX, FLT_MAX, FLT_MAX), 0.0f, 0.0f, 3, 0b00001);
        Entity e = m_RigidBodyMap.at(newBody);
        NPC newNPC = NPC();
        RemoveRigidBody(newBody);

        // Prepare NPC
        newNPC.SearchAttempts = 0;
        newNPC.BaseHealth = 4;
        newNPC.CastSound = SoundIndex::EnemyCast1;
        newNPC.DeathSound = SoundIndex::ObeliskDeath;
        newNPC.SpawnOffset = Vec3(0.0f, 0.0f, 0.0f);

        // Prepare light
        newNPC.LightColor = Vec4(10.0f, 30.0f, 500.0f, 0);
        Light newLight = { Vec4(99999.f,99999.f,99999.f,0), newNPC.LightColor };

        // Health
        Health npcHealth = Health::New(newNPC.BaseHealth, newNPC.BaseHealth);

        // Prepare model
        m_ModelInstances.AddExisting(e, GetNPCModel(*m_RigidBodies.Get(e)));

        // Insert into tables / groups
        m_pRenderer->lights.AddExisting(e, newLight);
        m_NPCs.AddExisting(e, newNPC);
        m_NPCsCache.AddExisting(e);
        m_Healths.AddExisting(e, npcHealth);
    }
}