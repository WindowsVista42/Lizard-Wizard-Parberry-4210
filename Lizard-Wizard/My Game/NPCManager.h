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
    ModelIndex::e Model;
};

// Projectile Class (Can be either a Bullet3 ray or a Bullet3 simulated projectile.)
class NPCManager {

public:
    void GenerateNPC(

    );
    void PlaceNPC(

    );
    void InitializeNPCs(

    );
    void StripNPC(Entity);

private:

    ProjectileManager* CurrentProjectileManager;
    Table<NPC>* CurrentNPCs;
    Group* CurrentNPCsCache;
    Group* CurrentNPCsActive;
};


#endif