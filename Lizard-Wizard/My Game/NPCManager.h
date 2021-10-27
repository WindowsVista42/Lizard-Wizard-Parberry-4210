#ifndef NPCManager_H
#define NPCManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Settings.h"
#include "CustomBind.h"
#include <vector>
#include "Ecs.h"
#include "PhysicsManager.h"
#include "ProjectileManager.h"
#include "Defines.h"

// Respective Structs
struct NPC {
    f32 Health;
    f32 Speed;

    Vec3 Position;
    Vec3 Orientation;

    NPCBehavior::e Behavior;

    btRigidBody* Body;
    //ModelIndex::e Model;

    NPC() : 
        Health(0),
        Speed(0),
        Position(Vec3(0, 0, 0)),
        Orientation(Vec3(0, 0, 0)),
        Behavior(NPCBehavior::PASSIVE)
    {}
};

// Projectile Class (Can be either a Bullet3 ray or a Bullet3 simulated projectile.)
class NPCManager {

public:
    void GenerateNPC();
    void PlaceNPC(Entity, Vec3);
    void InitializeNPCs(
        PhysicsManager*,
        ProjectileManager*,
        Table<NPC>*,
        Table<Light>*,
        Group*,
        Group* 
    );
    void StripNPC(Entity);

private:
    PhysicsManager* CurrentPhysicsManager;
    ProjectileManager* CurrentProjectileManager;
    Table<NPC>* CurrentNPCs;
    Table<Light>* CurrentLights;
    Group* CurrentNPCsCache;
    Group* CurrentNPCsActive;
};


#endif