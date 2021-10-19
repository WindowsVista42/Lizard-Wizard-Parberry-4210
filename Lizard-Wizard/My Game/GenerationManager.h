#ifndef GenerationManager_H
#define GenerationManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "PhysicsManager.h"
#include <vector>

// Respective Structs
namespace RoomTag {
    enum tag : u32 {
        SPAWN, NORMAL, BOSS
};}

struct Room {
    RoomTag::tag currentTag;
    Vec3 origin;
};

// Physics Class
class GenerationManager {

public:
    void GenerateRoom(Vec3);
    void GenerateRooms(const i32);
    void DestroyRooms();
    void InitializeGeneration(PhysicsManager*);

private:
    PhysicsManager* currentPhysicsManager;
    std::vector<Room*>* currentRooms;
};


#endif
