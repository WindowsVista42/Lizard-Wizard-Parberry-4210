#include "GenerationManager.h"
#include "PhysicsManager.h"
#include "Defines.h"

void GenerationManager::CreateNormalRoom(Vec3 roomCenter) {
    // Ground then roof.
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 50.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    //currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
    currentPhysicsManager->CreateBoxObject(Vec3(50.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    currentPhysicsManager->CreateBoxObject(Vec3(3000.0f, 1000.0f, 50.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall

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

void GenerationManager::GenerateRooms(Vec3 roomCenter, const i32 roomCount) {
    u32 randomX = GameRandom::Randu32(1, X_ROOMS - 1);
    u32 randomY = GameRandom::Randu32(1, Y_ROOMS - 1);

    // Old variables for generating from a room.
    /*
    i32 boundsX[4] = { 0, 1, 0, -1};
    i32 boundsY[4] = { -1, 0, 1, 0};
    i32 currentX = randomX;
    i32 currentY = randomY;
    Vec3 currentCenter = roomCenter;
    */

    // Generate First Room
    Room initialRoom;
    initialRoom.origin = roomCenter;
    initialRoom.currentTag = RoomTag::NORMAL;
    CreateNormalRoom(roomCenter);
    currentMap[randomX][randomY];


    // Generating After Creating First Room
    for every(index, roomCount) {
        u32 recounter = 1;
        b8 placed = false;

        randomX = GameRandom::Randu32(1, X_ROOMS - 1);
        randomY = GameRandom::Randu32(1, Y_ROOMS - 1);
        if (currentMap[randomX][randomY].currentTag == RoomTag::UNFILLED) {
            currentMap[randomX][randomY].currentTag = RoomTag::NORMAL;
            CreateNormalRoom(currentMap[randomX][randomY].origin);
        }

        // Old switchcase statement.
        /*
        switch (nextRoom) {
        case 0:
            if (currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag == RoomTag::UNFILLED) {
                currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag = RoomTag::NORMAL;
                currentCenter = Vec3(currentCenter.x, currentCenter.y, currentCenter.z - 6000.0f);
                CreateNormalRoom(currentCenter);
            }
            break;
        case 1:
            if (currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag == RoomTag::UNFILLED) {
                currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag = RoomTag::NORMAL;
                currentCenter = Vec3(currentCenter.x + 6000.0f, currentCenter.y, currentCenter.z);
                CreateNormalRoom(currentCenter);
            }
            break;
        case 2:
            if (currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag == RoomTag::UNFILLED) {
                currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag = RoomTag::NORMAL;
                currentCenter = Vec3(currentCenter.x, currentCenter.y, currentCenter.z + 6000.0f);
                CreateNormalRoom(currentCenter);
            }
            break;
        case 3:
            if (currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag == RoomTag::UNFILLED) {
                currentMap[currentX + boundsX[nextRoom]][currentY + boundsY[nextRoom]].currentTag = RoomTag::NORMAL;
                currentCenter = Vec3(currentCenter.x - 6000.0f, currentCenter.y, currentCenter.z);
                CreateNormalRoom(currentCenter);
            }
            break;
        default:
            break;
        }
        */
    }
}

void GenerationManager::DestroyRooms() {

}

void GenerationManager::InitializeGeneration(PhysicsManager* gamePhysicsManager) {
    currentPhysicsManager = gamePhysicsManager;

    for every(indexX, X_ROOMS) {
        for every(indexY, Y_ROOMS) {
            Room instanceRoom;
            instanceRoom.currentTag = RoomTag::UNFILLED;
            instanceRoom.origin = Vec3(6000.0f * indexX, 0.0f, 6000.0f * indexY);
            currentMap[indexX][indexY] = instanceRoom;
        }
    }
}