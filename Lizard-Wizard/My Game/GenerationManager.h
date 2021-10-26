#ifndef GenerationManager_H
#define GenerationManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "PhysicsManager.h"
#include <vector>

// Room Configuration Defines
#define X_ROOMS 6
#define Y_ROOMS 6

// Respective Structs
namespace RoomTag {
    enum tag : u32 {
        UNFILLED, SPAWN, NORMAL, BOSS
};}

struct Room {
    RoomTag::tag currentTag;
    Vec3 origin;
};

// Physics Class
class GenerationManager {

public:
    void CreateNormalRoom(Vec3);
    void CreateBossRoom(Vec3);
    void CreateSpawnRoom(Vec3);
    void CreateHallway(Vec3);
    void GenerateRooms(Vec3, const i32);
    void DestroyRooms();
    void InitializeGeneration(PhysicsManager*);

private:
    PhysicsManager* currentPhysicsManager;
    std::vector<Room> currentRooms;
    Room currentMap[X_ROOMS][Y_ROOMS];
};


#endif
