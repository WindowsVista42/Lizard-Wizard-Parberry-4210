#include "Game.h"
#include "Defines.h"

void CGame::CreateNormalRoom(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    //CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
    CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void CGame::CreateHallway(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four walls.
   // CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // North Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 31); // West Wall
    //CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // South Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 31); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void CGame::CreateBossRoom(Vec3 roomCenter) {
    // Ground then roof
    CreateBoxObject(Vec3(5000.0f, 1.0f, 5000.0f), roomCenter + Vec3(10.0f, 10.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(5000.0f, 1.0f, 5000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four Walls
    CreateBoxObject(Vec3(1.0f, 1000.0f, 5000.0f), roomCenter + Vec3(5000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(5000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 5000.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(1.0f, 1000.0f, 5000.0f), roomCenter + Vec3(-5000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(5000.0f, 1000.0f, 1.0f), roomCenter + Vec3(-0.0f, 1000.0f, -5000.0f), 0.0f, 1.5f, 1, 31);

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::BOSS;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
}

void CGame::CreateSpawnRoom(Vec3 roomCenter) {
    //Ground then roof
    CreateBoxObject(Vec3(4000.0f, 1.0f, 4000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 31);
    CreateBoxObject(Vec3(4000.0f, 1.0f, 4000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 31);

    // Four Walls
    CreateBoxObject(Vec3(1.0f, 1000.0f, 4000.0f), roomCenter + Vec3(4000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // North Wall
    CreateBoxObject(Vec3(4000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 4000.0f), 0.0f, 1.5f, 1, 31); // West Wall
    CreateBoxObject(Vec3(1.0f, 1000.0f, 4000.0f), roomCenter + Vec3(-4000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 31); // South Wall
    CreateBoxObject(Vec3(4000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -4000.0f), 0.0f, 1.5f, 1, 31); // East Wall


    Room newRoom;
    newRoom.currentTag = RoomTag::tag::SPAWN;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
     
}

void CGame::GenerateRooms(Vec3 roomCenter, const i32 roomCount) {
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

void CGame::DestroyRooms() {

}

void CGame::InitializeGeneration() {
    for every(indexX, X_ROOMS) {
        for every(indexY, Y_ROOMS) {
            Room instanceRoom;
            instanceRoom.currentTag = RoomTag::UNFILLED;
            instanceRoom.origin = Vec3(6000.0f * indexX, 0.0f, 6000.0f * indexY);
            currentMap[indexX][indexY] = instanceRoom;
        }
    }
}