#include "Game.h"

ModelInstance GetBoxyModel(btRigidBody* body) {
    ModelInstance instance = {};
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    instance.model = (u32)ModelIndex::Cube;
    instance.world = MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), *(Quat*)&body->getWorldTransform().getRotation(), boxShape->getHalfExtentsWithMargin());
    instance.texture = TextureIndex::White;

    return instance;
}


void CGame::CreateNewRoom(Vec3 roomCenter) {
    const Vec3 scale = Vec3(3000.0f, 1000.0f, 3000.0f);

    const u32 group = 0b00001;
    const u32 flag = 0b11111;

    // Floor
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, 1.0f, scale.z), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Ceiling
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, 1.0f, scale.z), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // East Wall +X
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(1.0f, scale.y, scale.z), roomCenter + Vec3(scale.x, scale.y, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // West Wall -X
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(1.0f, scale.y, scale.z), roomCenter + Vec3(-scale.x, scale.y, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }
    
    // North Wall +Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, scale.z), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // South Wall -Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, -scale.z), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::NORMAL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateNewHall(Vec3 roomCenter) {
    const Vec3 scale = Vec3(2000.0f, 500.0f, 2000.0f);

    const u32 group = 0b00001;
    const u32 flag = 0b11111;

    // Floor
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, 1.0f, scale.z), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Ceiling
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, 1.0f, scale.z), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // East Wall +X
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(1.0f, scale.y, scale.z), roomCenter + Vec3(scale.x, scale.y, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // West Wall -X
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(1.0f, scale.y, scale.z), roomCenter + Vec3(-scale.x, scale.y, 0.0f), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // North Wall +Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, scale.z), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // South Wall -Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, -scale.z), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::NORMAL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}


/*void CGame::CreateCorridorRoom(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    //CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    //CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
   // CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    //CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::CORRIDOR;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);

    /*
    Room newExit;
    newExit.currentExit = ExitTag::exit::NORTHEX;
    newExit.origin = roomCenter;
    currentRooms.push_back(newExit);
    
}

void CGame::CreateEWHall(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    //CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
    //CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::EWHALL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);

    /*
    Room newExit;
    newExit.currentExit = ExitTag::exit::NORTHEX;
    newExit.origin = roomCenter;
    currentRooms.push_back(newExit);
    

}

void CGame::CreateNSHall(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    //CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
    CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    //CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall

    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NSHALL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);
    /*
    Room newExit;
    newExit.currentExit = ExitTag::exit::NORTHEX;
    newExit.origin = roomCenter;
    currentRooms.push_back(newExit);
    
}

void CGame::CreateNormalRoom(Vec3 roomCenter) {
    // Ground then roof.
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);
    CreateBoxObject(Vec3(3000.0f, 1.0f, 3000.0f), roomCenter + Vec3(0.0f, 2000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111);

    // Four walls.
    CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 0, 0b11111); // North Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, 3000.0f), 0.0f, 1.5f, 1, 0b11111); // West Wall
    CreateBoxObject(Vec3(1.0f, 1000.0f, 3000.0f), roomCenter + Vec3(-3000.0f, 1000.0f, 0.0f), 0.0f, 1.5f, 1, 0b11111); // South Wall
    CreateBoxObject(Vec3(3000.0f, 1000.0f, 1.0f), roomCenter + Vec3(0.0f, 1000.0f, -3000.0f), 0.0f, 1.5f, 1, 0b11111); // East Wall


    Room newRoom;
    newRoom.currentTag = RoomTag::tag::NORMAL;
    newRoom.origin = roomCenter;
    currentRooms.push_back(newRoom);   

}
*/
void CGame::GenerateRooms(Vec3 roomCenter, const i32 roomCount) {
    u32 randomX = GameRandom::Randu32(1, X_ROOMS - 1);
    u32 randomY = GameRandom::Randu32(1, Y_ROOMS - 1);
    u32 randomNum = GameRandom::Randu32(1, 4);

    // Generate Spawn Room
    Room initialRoom;
    initialRoom.origin = roomCenter;
    initialRoom.currentTag = RoomTag::NORMAL;
    CreateNewRoom(roomCenter);
    currentMap[randomX][randomY];

    // Generate Boss Room


    // Generating After Creating First Room
    for every(index, roomCount) {
        u32 recounter = 1;
        b8 placed = false;

        randomX = GameRandom::Randu32(1, X_ROOMS - 2);
        randomY = GameRandom::Randu32(1, Y_ROOMS - 2);
        randomNum = GameRandom::Randu32(1, 4);

        if (currentMap[randomX][randomY].currentTag == RoomTag::UNFILLED) {

            currentMap[randomX][randomY].currentTag = RoomTag::NORMAL;
            CreateNewRoom(currentMap[randomX][randomY].origin);
            
            // Note: the randomNum is only assigning the south tag

            // Randomly assigns exit tag
            if (randomNum == 1) {
                currentMap[randomX][randomY].currentExit = ExitTag::NORTHEX; // +Z
            }
            else if (randomNum == 2) {
                currentMap[randomX][randomY].currentExit = ExitTag::EASTEX; // +X
            }
            else if (randomNum == 3) {
                currentMap[randomX][randomY].currentExit = ExitTag::SOUTHEX; //-Z
            }
            else // randomNum == 4
                currentMap[randomX][randomY].currentExit = ExitTag::WESTEX; //-X
            
        }

        switch (randomNum)
        { //Note: Redo the room tags for the hallways segements
            case 1: // Spawn Hallway North (+Z) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::NORTHEX)
                {
                   
                        currentMap[randomX][randomY + 1].currentTag = RoomTag::NSHALL;
                        CreateNewHall(currentMap[randomX][randomY + 1].origin);
                    
                    
                }
                break;

            case 2: // Spawn Hallway East (+X) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::EASTEX)
                {
                   
                        currentMap[randomX + 1][randomY].currentTag = RoomTag::EWHALL;
                        CreateNewHall(currentMap[randomX + 1][randomY].origin);
                    

                }
                break;

            case 3: // Spawn Hallway South (-Z) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::SOUTHEX)
                {
                    
                    currentMap[randomX][randomY - 1].currentTag = RoomTag::NSHALL;
                    CreateNewHall(currentMap[randomX][randomY - 1].origin);

                }
                break;

            case 4: // Spawn Hallway North (-X) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::WESTEX)
                {
                        currentMap[randomX - 1][randomY].currentTag = RoomTag::EWHALL;
                        CreateNewHall(currentMap[randomX - 1][randomY].origin);
                    

                }
                break;

            default:
                break;

        }
    }
}

void CGame::DestroyRooms() {

}

void CGame::InitializeGeneration() {

    // Create Spawn Room at bottom left corner
    CreateNewRoom(currentMap[1][1].origin);
    //currentMap[1][1].currentTag = RoomTag::SPAWN;

    // Initialiez Generation
    for every(indexX, X_ROOMS) {
        for every(indexY, Y_ROOMS) {
            Room instanceRoom;
            instanceRoom.currentTag = RoomTag::UNFILLED;
            instanceRoom.origin = Vec3(6000.0f * indexX, 0.0f, 6000.0f * indexY);
            currentMap[indexX][indexY] = instanceRoom;
        }
    }

    // Spawn Boss Room at top right corner
    CreateNewRoom(currentMap[X_ROOMS-1][Y_ROOMS-1].origin);
    //currentMap[X_ROOMS-1][Y_ROOMS-1].currentTag = RoomTag::BOSS;
}
