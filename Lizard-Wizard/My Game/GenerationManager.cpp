#include "Game.h"

// Sean todo:
// - Add visual scatter (ground)
// - Add functional scatter (cover, blocks on walls)
// - Add obstacles
// - Add particles
// - Impl Pathfinding
// - Impl Flood-filling
// - Impl Remove connected points
// - Performance: batch geometry

static const Vec3 room_scale = Vec3(1000.0f, 1000.0f, 1000.0f);
static const u32 room_group = 0b00001;
static const u32 room_flag = 0b11111;
static const f32 room_mass = 0.0f;
static const f32 room_friction= 0.0f;

void CGame::GenerateRooms(Vec3 roomCenter, const i32 roomCount) {
    roomCenter.y -= room_scale.y;

    auto indexToWorld = [&](u32 x, u32 z) -> Vec3 {
        return roomCenter + 2.0f * Vec3((f32)x * room_scale.x, 0.0f, (f32)z * room_scale.z);
    };

    auto bounds = [&](i32 x, i32 z) {
        return x > 0 && x < X_ROOMS && z > 0 && z < Z_ROOMS;
    };

    // GENERATION CODE
    std::vector<std::pair<i32, i32>> largeRooms;

    // large room pass
    for every(i, roomCount) {
        i32 x = (i32)GameRandom::Randu32(1, X_ROOMS - 2);
        i32 z = (i32)GameRandom::Randu32(1, Z_ROOMS - 2);
        
        f32 fx = (f32)x;
        f32 fz = (f32)z;

        for (i32 ix = -1; ix <= 1; ix += 1) {
            i32 cx = ix + x;
            for (i32 iz = -1; iz <= 1; iz += 1) {
                i32 cz = iz + z;

                if (cx > 0 && cx < X_ROOMS && cz > 0 && cz < Z_ROOMS) {
                    currentMap[cx][cz] = true;
                    Vec3 origin = 2.0f * Vec3(cx * room_scale.x, 0.0f, cz * room_scale.z);
                    largeRooms.push_back(std::make_pair(cx, cz));
                }
            }
        }
    }

    // Sean: randomly remove singular cells
    for every(i, roomCount * 4) {
        i32 x = (i32)GameRandom::Randu32(0, X_ROOMS - 1);
        i32 z = (i32)GameRandom::Randu32(0, Z_ROOMS - 1);

        currentMap[x][z] = false;
    }

    // Sean: build connections to disconnected parts
    for every(x, X_ROOMS) {
        for every(z, Z_ROOMS) {
            if (currentMap[x][z] == false) {
                if (bounds(x - 1, z) && currentMap[x - 1][z]
                    && bounds(x + 1, z) && currentMap[x + 1][z]
                ) {  // check left / right
                    currentMap[x][z] = true;
                }

                if (bounds(x, z - 1) && currentMap[x][z - 1]
                    && bounds(x, z + 1) && currentMap[x][z + 1]
                ) {  // check up / down
                    currentMap[x][z] = true;
                }
            }
        }
    }

    // Sean: draw a line from one x,z to another x,z
    // Sean: currently broken
    //auto DrawLine = [&](std::pair<i32, i32> start, std::pair<i32, i32> end) {
    //    i32 dirx = start.first > end.first ? -1 : 1;
    //    i32 dirz = start.second > end.second ? -1 : 1;

    //    i32 tx = 0;
    //    i32 tz = 0;

    //    i32 x = start.first;
    //    i32 z = start.second;

    //    while (x != end.first && z != end.second) {
    //        if (abs(start.first - (x + dirx)) > abs(start.second - (z + dirz))) { // right or left
    //            currentMap[x][z] = true;
    //            x += dirx;
    //        } else { // up or down
    //            currentMap[x][z] = true;
    //            z += dirz;
    //        }
    //    }
    //};

    //for every(index, largeRooms.size()) {
    //    i32 first = (i32)GameRandom::Randu32(0, roomCount - 1);
    //    i32 second = (i32)GameRandom::Randu32(0, roomCount - 1);

    //    DrawLine(largeRooms[first], largeRooms[second]);
    //}

    // PLACEMENT CODE

    // Sean: local function to place a side of a room
    auto Place = [&](Vec3 side, i32 x, i32 z) {
        Vec3 origin = 2.0f * Vec3(x * room_scale.x, 0.0f, z * room_scale.z);

        if (side.x == -1.0f) {
            origin.x -= room_scale.x;
        } else if (side.x == 1.0f) {
            origin.x += room_scale.x;
        } else if (side.z == -1.0f) {
            origin.z -= room_scale.z;
        } else if (side.z == 1.0f) {
            origin.z += room_scale.z;
        } else if (side.y == -1.0f) {
            origin.y -= room_scale.y;
        } else if (side.y == 1.0f) {
            origin.y += room_scale.y;
        } else {
            ABORT_EQ_FORMAT(0, 0, "Input placement was not valid!");
        }

        Vec3 body_scale;
        if (fabs(side.x) == 1.0f) {
            body_scale = Vec3(2.0f, room_scale.y, room_scale.z);
        } else if (fabs(side.y) == 1.0f) {
            body_scale = Vec3(room_scale.x, 2.0f, room_scale.z);
        } else if (fabs(side.z) == 1.0f) {
            body_scale = Vec3(room_scale.x, room_scale.y, 2.0f);
        } else {
            ABORT_EQ_FORMAT(0, 0, "Input placement was not valid!");
        }

        Entity e = Entity();

        btRigidBody* body = CreateBoxObject(body_scale, origin, room_mass, room_friction, room_group, room_flag);
        m_RigidBodies.AddExisting(e, body);

        Quat rotation = Quat::CreateFromAxisAngle(side, M_PI / 2.0f);
        Quat turn = side.y != 1.0f ? Quat::CreateFromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), -M_PI / 2.0f) : Quat::CreateFromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), M_PI);
        rotation *= turn;

        ModelInstance instance;
        instance.glow = 0.0f;
        instance.model = ModelIndex::Quad;
        instance.texture = TextureIndex::White;
        instance.world = MoveRotateScaleMatrix(origin, rotation, Vec3(room_scale.x, 2.0f, room_scale.z));
        m_ModelInstances.AddExisting(e, instance);
        m_ModelsActive.AddExisting(e);
    };

    std::vector<std::pair<i32, i32>> rooms;
    auto PlaceConditionally = [&](u32 x, i32 dx, u32 z, i32 dz) {
        if (bounds(x + dx, z + dz) == false) {
            Place(Vec3((f32)dx, 0.0f, (f32)dz), x, z);
        } else if (bounds(x + dx, z + dz) && currentMap[x + dx][z + dz] == false) {
            Place(Vec3((f32)dx, 0.0f, (f32)dz), x, z);
        }
    };

    // Sean:
    // Build floors, ceilings, and walls
    for every(x, X_ROOMS) {
        for every(z, Z_ROOMS) {
            if (bounds(x, z) && currentMap[x][z]) { // here
                rooms.push_back(std::make_pair(x, z));
                Place(Vec3(0.0f, -1.0f, 0.0f), x, z); // floor
                Place(Vec3(0.0f,  1.0f, 0.0f), x, z); // ceiling

                i32 dx;
                i32 dz;

                dx = -1; dz = 0; // left
                PlaceConditionally(x, dx, z, dz);

                dx = 1; dz = 0; // right
                PlaceConditionally(x, dx, z, dz);

                dx = 0; dz = -1; // down
                PlaceConditionally(x, dx, z, dz);

                dx = 0; dz = 1; // up
                PlaceConditionally(x, dx, z, dz);
            }
        }
    }

    // Sean:
    // randomly choose tiles to place lights into
    // dont choose the same tile twice
    #define LIGHT_COUNT_ROOMS 40
    for every(i, LIGHT_COUNT_ROOMS) {
        u32 index = GameRandom::Randu32(1, rooms.size() - 1);

        // swap back
        auto ipos = rooms[index];
        rooms[index] = rooms[rooms.size() - 1];
        rooms.pop_back();

        {
            Vec3 pos = indexToWorld(ipos.first, ipos.second); 

            pos.Print();

            Entity e = Entity();

            Light light;
            light.color = Vec4(100.0f, 100.0f, 100.0f, 0.0f);
            light.position = Vec4(pos.x, pos.y, pos.z, 0.0f);
            m_pRenderer->lights.AddExisting(e, light);
            m_TestingLights.AddExisting(e);

            ModelInstance model;
            model.glow = 2.0f;
            model.model = ModelIndex::Cube;
            model.texture = TextureIndex::White;
            model.world = MoveScaleMatrix(pos, Vec3(50.0f));
            m_ModelInstances.AddExisting(e, model);
            m_ModelsActive.AddExisting(e);
        }
    }
}

void CGame::DestroyRooms() {
}

void CGame::InitializeGeneration() {
}

// Sean: returns end if end reached, otherwise it returns the closest point it could find to end
// Performance: flood-fills until all locations are filled, kinda slow
std::pair<u32, u32> CGame::FindClosestPoint(std::pair<u32, u32> start, std::pair<u32, u32> end) {
}

// Sean: removes connected (can pathfind to) points from starting position and input array
void CGame::RemoveConnectedPoints(std::pair<u32, u32> start, std::vector<std::pair<u32, u32>>& points) {
}

// Sean: returns a list of points for going from start to end, vector length is ZERO if a path is not found.
// Performance: A* pathfinding, not slow relatively, but pathfinding is still slow in general
std::vector<std::pair<u32, u32>> CGame::Pathfind(std::pair<u32, u32> start, std::pair<u32, u32> end) {
}
