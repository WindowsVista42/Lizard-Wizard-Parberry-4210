#include "GenerationManager.h"
#include "PhysicsManager.h"

void GenerationManager::CreateNormalRoom(Vec3 roomCenter) {
    // Ground then roof.
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four walls.
    //currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // North Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 31); // West Wall
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // South Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 31); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void GenerationManager::CreateHallway(Vec3 roomCenter) {
    // Ground then roof.
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four walls.
   // currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // North Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 31); // West Wall
    //currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // South Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 31); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void GenerationManager::CreateBossRoom(Vec3 roomCenter) {
    // Ground then roof
    currentPhysicsManager->CreateBoxObject(Vec3(5000.0f, 50.0f, 5000.0f), roomCenter + Vec3(10.0f, 10.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(5000.0f, 50.0f, 5000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four Walls
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 5000.0f), roomCenter + Vec3(5000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(5000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 5000.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 5000.0f), roomCenter + Vec3(-5000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(5000.0f, 1000.0f, 50.0f), roomCenter + Vec3(-0.0f, 1000.0f, -5000.0f), 0.0f, 1.5f, 1, 31);

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::BOSS;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void GenerationManager::CreateSpawnRoom(Vec3 roomCenter) {
    //Ground then roof
    currentPhysicsManager->CreateBoxObject(Vec3(4000.0f, 50.0f, 4000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    currentPhysicsManager->CreateBoxObject(Vec3(4000.0f, 50.0f, 4000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four Walls
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 4000.0f), roomCenter + Vec3(4000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // North Wall
    currentPhysicsManager->CreateBoxObject(Vec3(4000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 4000.0f), 0.0f, 1.5f, 1, 31); // West Wall
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 4000.0f), roomCenter + Vec3(-4000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // South Wall
    currentPhysicsManager->CreateBoxObject(Vec3(4000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, -4000.0f), 0.0f, 1.5f, 1, 31); // East Wall


    Room newRoom;
    newRoom.currentTag = RoomTag::tag::SPAWN;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
     
}

void GenerationManager::GenerateRooms(const i32 roomCount) {
    
    

}

void GenerationManager::DestroyRooms() {

}

void GenerationManager::InitializeGeneration(PhysicsManager* gamePhysicsManager) {
    currentPhysicsManager = gamePhysicsManager;
}