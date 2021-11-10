#include "Game.h"

ModelInstance GetBoxyModel(btRigidBody* body) {
    ModelInstance instance = {};
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

    instance.model = ModelIndex::Cube;
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

void CGame::CreateEWHall(Vec3 roomCenter) {
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
        newRoom.currentTag = RoomTag::tag::EWHALL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateNSHall(Vec3 roomCenter) {
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

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::NSHALL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateNewCorridor(Vec3 roomCenter) {
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

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::CORRIDOR;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

    // Lights
    {
        const f32 splat = 100.0f;
        const Vec3 light_scale = Vec3(splat, splat, splat);
        const Vec3 light_color = Colors::LightYellow * 200.0;

        const i32 count = 1;
        const f32 dx = (scale.x * 2.0) / (f32)count;
        const f32 dz = (scale.z * 2.0) / (f32)count;
        for (i32 x = 0; x < count; x += 1) {
            f32 fx = (f32)(x - count / 2);
            for (i32 z = 0; z < count; z += 1) {
                f32 fz = (f32)(z - count / 2);

                Vec3 light_pos = Vec3(
                    roomCenter.x + (fx * dx) + (dx / 2.0),
                    roomCenter.y + scale.y - light_scale.y - 50.0,
                    roomCenter.z + (fz * dz) + (dz / 2.0)
                );

                ModelInstance mi;
                mi.model = ModelIndex::Cube;
                mi.texture = TextureIndex::White;
                mi.world = MoveScaleMatrix(light_pos, light_scale);

                Entity e = Entity();
                m_ModelInstances.AddExisting(e, mi);
                m_ModelsActive.AddExisting(e);
                m_pRenderer->lights.AddExisting(e, { *(Vec4*)&light_pos, *(Vec4*)&light_color });

                m_TestingLights.AddExisting(e);
            }
        }
    }

}

void CGame::CreateNorthWestL(Vec3 roomCenter) {
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

    // North Wall +Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, scale.z), 0.0f, 1.5f, group, flag));
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

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::NWL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateSouthWestL(Vec3 roomCenter) {
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

    // West Wall -X
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(1.0f, scale.y, scale.z), roomCenter + Vec3(-scale.x, scale.y, 0.0f), 0.0f, 1.5f, group, flag));
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
        newRoom.currentTag = RoomTag::tag::SWL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateNorthEastL(Vec3 roomCenter) {
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

    // North Wall +Z
    {
        Entity e = Entity();
        m_RoomWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, 1.0f), roomCenter + Vec3(0.0f, scale.y, scale.z), 0.0f, 1.5f, group, flag));
        m_RoomWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxyModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Tags
    {
        Room newRoom;
        newRoom.currentTag = RoomTag::tag::NEL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}

void CGame::CreateSouthEastL(Vec3 roomCenter) {
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
        newRoom.currentTag = RoomTag::tag::SEL;
        newRoom.origin = roomCenter;
        currentRooms.push_back(newRoom);
    }

}


void CGame::GenerateRooms(Vec3 roomCenter, const i32 roomCount) {
    u32 randomX = GameRandom::Randu32(1, X_ROOMS - 1);
    u32 randomY = GameRandom::Randu32(1, Y_ROOMS - 1);
    u32 randomNum = GameRandom::Randu32(1, 4);

    // Generate Spawn Room
    Room initialRoom;
    initialRoom.origin = roomCenter;
    initialRoom.currentTag = RoomTag::NORMAL;
    CreateNewRoom(roomCenter);
    currentMap[1][1];


    // Generating After Creating First Room
    for every(index, roomCount) {
        u32 recounter = 1;
        b8 placed = false;

        randomX = GameRandom::Randu32(1, X_ROOMS - 2);
        randomY = GameRandom::Randu32(1, Y_ROOMS - 2);
        randomNum = GameRandom::Randu32(1, 4);

        if (currentMap[randomX][randomY].currentTag == RoomTag::UNFILLED) {

            currentMap[randomX][randomY].currentTag = RoomTag::CORRIDOR;
            CreateNewCorridor(currentMap[randomX][randomY].origin);

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

        // Hallway Generation

        switch (randomNum)
        { 
            case 1: // Spawn Hallway North (+Z) of Corridor
                if (currentMap[randomX][randomY].currentExit == ExitTag::NORTHEX)
                {
                    if (currentMap[randomX][randomY + 1].currentTag == RoomTag::NORMAL)
                    {
                        break;
                    }
                    else 
                    {
                        currentMap[randomX][randomY + 1].currentTag = RoomTag::NSHALL;
                        CreateNSHall(currentMap[randomX][randomY + 1].origin);
                    }
                }
                break;

            case 2: // Spawn Hallway East (+X) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::EASTEX)
                {
                    if (currentMap[randomX + 1][randomY].currentTag == RoomTag::NORMAL)
                    {
                        break;
                    }
                    else
                    {
                        currentMap[randomX + 1][randomY].currentTag = RoomTag::EWHALL;
                        CreateEWHall(currentMap[randomX + 1][randomY].origin);
                    }
                }
                break;

            case 3: // Spawn Hallway South (-Z) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::SOUTHEX)
                {
                    if (currentMap[randomX][randomY - 1].currentTag == RoomTag::NORMAL)
                    {
                        break;
                    }
                    else
                    {
                        currentMap[randomX][randomY - 1].currentTag = RoomTag::NSHALL;
                        CreateNSHall(currentMap[randomX][randomY - 1].origin);
                    }                   
                }
                break;

            case 4: // Spawn Hallway West (-X) of Normal Room
                if (currentMap[randomX][randomY].currentExit == ExitTag::WESTEX)
                {
                    if (currentMap[randomX - 1][randomY].currentTag == RoomTag::NORMAL)
                    {
                        break;
                    }
                    else
                    {
                        currentMap[randomX - 1][randomY].currentTag = RoomTag::EWHALL;
                        CreateEWHall(currentMap[randomX - 1][randomY].origin);
                    }
                }
                break;

            default:
                break;

        }
/*
        // L Room Generation

        // Generation Check at (-1, 1)
        if (currentMap[randomX - 1][randomY + 1].currentTag == RoomTag::CORRIDOR || RoomTag::NSHALL || RoomTag::EWHALL)
        {
            // Check left side for empty space
            if (currentMap[randomX - 1][randomY].currentTag == RoomTag::UNFILLED)
            {
                currentMap[randomX - 1][randomY].currentTag = RoomTag::SWL;
                CreateSouthWestL(currentMap[randomX - 1][randomY].origin);
         
            }   
            else if (currentMap[randomX - 1][randomY].currentTag == RoomTag::CORRIDOR || RoomTag::NSHALL || RoomTag::EWHALL)
            {
                break;
            }

            // Check top side for empty space
            if (currentMap[randomX][randomY+1].currentTag == RoomTag::UNFILLED)
            {
                currentMap[randomX][randomY+1].currentTag = RoomTag::NEL;
                CreateNorthEastL(currentMap[randomX][randomY+1].origin);

            }
            else if (currentMap[randomX][randomY+1].currentTag == RoomTag::CORRIDOR || RoomTag::NSHALL || RoomTag::EWHALL)
            {
                break;
            }

            
        }
        */


    }
}

void CGame::DestroyRooms() {
}

void CGame::InitializeGeneration() {

    // Create Spawn Room at bottom left corner
    CreateNewRoom(currentMap[1][1].origin);
    currentMap[1][1].currentTag = RoomTag::SPAWN;

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
    currentMap[X_ROOMS-1][Y_ROOMS-1].currentTag = RoomTag::BOSS;
}
