#include "GenerationManager.h"
#include "PhysicsManager.h"

void GenerationManager::GenerateRoom(Vec3 roomCenter) {
    // Ground then roof.
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, -1);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, -1);

    // Four walls.
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, -1);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, -1);
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, -1);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, -1);

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
<<<<<<< HEAD
    currentRooms.push_back(newRoom);
=======
    currentRooms->push_back(&newRoom);
>>>>>>> a0db107b91f79f59b316698ac23cf91aeb2a3b4e
}

void GenerationManager::GenerateRooms(const i32 roomCount) {
    
    

}

void GenerationManager::DestroyRooms() {

}

void GenerationManager::InitializeGeneration(PhysicsManager* gamePhysicsManager) {
    currentPhysicsManager = gamePhysicsManager;
}