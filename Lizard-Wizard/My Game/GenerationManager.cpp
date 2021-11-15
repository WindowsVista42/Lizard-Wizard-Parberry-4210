#include "Game.h"
#include <map>
#include "BinaryHeap.h"

// Sean todo:
// - Add visual scatter (ground)
// - Add functional scatter (cover, blocks on walls)
// - Add obstacles
// - Add particles
// - Performance: batch geometry (by material)

static const Vec3 room_scale = Vec3(1000.0f, 1000.0f, 1000.0f);
static const u32 room_group = 0b00001;
static const u32 room_flag = 0b11111;
static const f32 room_mass = 0.0f;
static const f32 room_friction= 0.0f;

typedef std::pair<u32, u32> Point2;

Vec3 CGame::IndexToWorld(u32 x, u32 z) {
    return m_roomCenter + 2.0f * Vec3((f32)x * room_scale.x, 0.0f, (f32)z * room_scale.z);
}

bool CGame::CheckBounds(i32 x, i32 z) {
    return x >= 0 && x < X_ROOMS && z >= 0 && z < Z_ROOMS;
}

// Sean: hack to get around hashing
u64 make_key(u32 x, u32 z) {
    return (u64)x << 32 | (u32)z;
}

void CGame::GenerateRooms(Vec3 roomCenter, const i32 roomCount, const i32 randomRemoval, const i32 randomAddition) {
    roomCenter.y -= room_scale.y;
    m_roomCenter = roomCenter;

    // GENERATION CODE

    auto Place3x3 = [&](i32 x, i32 z) {
        for (i32 ix = -1; ix <= 1; ix += 1) {
            i32 cx = ix + x;
            for (i32 iz = -1; iz <= 1; iz += 1) {
                i32 cz = iz + z;

                if (CheckBounds(cx, cz)) {
                    m_GameMap[cx][cz] = true;
                    Vec3 origin = 2.0f * Vec3(cx * room_scale.x, 0.0f, cz * room_scale.z);
                }
            }
        }
    };

    // Sean: Large room pass
    for every(i, roomCount) {
        i32 x = (i32)GameRandom::Randu32(1, X_ROOMS - 2);
        i32 z = (i32)GameRandom::Randu32(1, Z_ROOMS - 2);
        
        f32 fx = (f32)x;
        f32 fz = (f32)z;

        Place3x3(x, z);
    }

    // Sean: randomly remove singular cells
    for every(i, randomRemoval) {
        i32 x = (i32)GameRandom::Randu32(0, X_ROOMS - 1);
        i32 z = (i32)GameRandom::Randu32(0, Z_ROOMS - 1);

        m_GameMap[x][z] = false;
    }

    // Sean: randomly add singular cells
    for every(i, randomAddition) {
        i32 x = (i32)GameRandom::Randu32(0, X_ROOMS - 1);
        i32 z = (i32)GameRandom::Randu32(0, Z_ROOMS - 1);

        m_GameMap[x][z] = true;
    }

    // Sean: Build connections to disconnected parts
    // [X][ ][X] --> [X][X][X]
    for every(x, X_ROOMS) {
        for every(z, Z_ROOMS) {
            if (m_GameMap[x][z] == false) {
                // check left / right
                if (CheckBounds(x - 1, z) && m_GameMap[x - 1][z]
                    && CheckBounds(x + 1, z) && m_GameMap[x + 1][z]
                ) {
                    m_GameMap[x][z] = true;
                }

                // check up / down
                if (CheckBounds(x, z - 1) && m_GameMap[x][z - 1]
                    && CheckBounds(x, z + 1) && m_GameMap[x][z + 1]
                ) {
                    m_GameMap[x][z] = true;
                }
            }
        }
    }

    Place3x3(1, 1); // spawn
    Place3x3(X_ROOMS - 2, Z_ROOMS - 2); // boss

    // Sean: draw a line from one x,z to another x,z
    auto DrawLineUntilOccupied = [&](std::pair<i32, i32> start, std::pair<i32, i32> end) {
        i32 dirx = start.first > end.first ? -1 : 1;
        i32 dirz = start.second > end.second ? -1 : 1;

        i32 tx = 0;
        i32 tz = 0;

        i32 x = start.first;
        i32 z = start.second;

        do {
            i32 xdiff = abs(end.first - x);
            i32 zdiff = abs(end.second - z);

            i32 cx = x;
            i32 cz = z;

            if (xdiff > zdiff) {
                if (CheckBounds(cx + dirx, cz)) {
                    x += dirx;
                } else if (CheckBounds(cx, cz + dirz)) {
                    z += dirz;
                }
            } else {
                if (CheckBounds(cx, cz + dirz)) {
                    z += dirz;
                } else if (CheckBounds(cx + dirx, cz)) {
                    x += dirx;
                }
            }

            m_GameMap[cx][cz] = true;

        } while (!m_GameMap[x][z] && (x != end.first || z != end.second));
    };

    // Sean: Fix gaps by repeatedly adding hallways
    // Sean: I messed up the algo somewhere so this has to be WIDTH - 2 and HEIGHT - 2 :|, still gives correct results :)
    Point2 nearest_to_end = FindClosestPoint(std::make_pair(1, 1), std::make_pair(X_ROOMS - 2, Z_ROOMS - 2));
    while (nearest_to_end != std::make_pair<u32, u32>(X_ROOMS - 2, Z_ROOMS - 2)) {
        Point2 nearest_to_nearest = FindClosestPoint(std::make_pair(X_ROOMS - 2, Z_ROOMS - 2), nearest_to_end);
        DrawLineUntilOccupied(nearest_to_end, nearest_to_nearest);
        nearest_to_end = FindClosestPoint(std::make_pair(1, 1), std::make_pair(X_ROOMS - 2, Z_ROOMS - 2));
    }

    // Sean: Remove dangling rooms
    std::vector<Point2> connected_all;
    std::vector<Point2> connected_walls;
    {
        std::unordered_set<u64> connected_set = FindConnected(std::make_pair(1, 1));
        for every(x, X_ROOMS) {
            for every(z, Z_ROOMS) {
                if (connected_set.find(make_key(x, z)) == connected_set.end()) {
                    m_GameMap[x][z] = false;
                }
            }
        }

        for (auto it = connected_set.begin(); it != connected_set.end(); it++) {
            u64 v = (*it);
            u32 x = v >> 32;
            u32 z = (v << 32) >> 32;
            connected_all.push_back(std::make_pair(x, z));

            if (CheckBounds(x - 1, z) && !m_GameMap[x - 1][z]) {
                connected_walls.push_back(std::make_pair(x, z));
            } else if (CheckBounds(x + 1, z) && !m_GameMap[x + 1][z]) {
                connected_walls.push_back(std::make_pair(x, z));
            } else if (CheckBounds(x, z - 1) && !m_GameMap[x][z - 1]) {
                connected_walls.push_back(std::make_pair(x, z));
            } else if (CheckBounds(x, z + 1) && !m_GameMap[x][z + 1]) {
                connected_walls.push_back(std::make_pair(x, z));
            }
        }
    }

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
        const f32 thickness = 10.0f;
        if (fabs(side.x) == 1.0f) {
            body_scale = Vec3(thickness, room_scale.y, room_scale.z);
        } else if (fabs(side.y) == 1.0f) {
            body_scale = Vec3(room_scale.x, thickness, room_scale.z);
        } else if (fabs(side.z) == 1.0f) {
            body_scale = Vec3(room_scale.x, room_scale.y, thickness);
        } else {
            ABORT_EQ_FORMAT(0, 0, "Input placement was not valid!");
        }

        Entity e = Entity();

        btRigidBody* body = CreateBoxObject(body_scale, origin, room_mass, room_friction, room_group, room_flag);
        m_RigidBodies.AddExisting(e, body);

        Quat rotation = Quat::CreateFromAxisAngle(side, M_PI / 2.0f);
        Quat turn = side.y != 1.0f ? 
            Quat::CreateFromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), -M_PI / 2.0f) :
            Quat::CreateFromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), M_PI);
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
        if (CheckBounds(x + dx, z + dz) == false) {
            Place(Vec3((f32)dx, 0.0f, (f32)dz), x, z);
        } else if (CheckBounds(x + dx, z + dz) && m_GameMap[x + dx][z + dz] == false) {
            Place(Vec3((f32)dx, 0.0f, (f32)dz), x, z);
        }
    };

    // Sean:
    // Build floors, ceilings, and walls
    for every(x, X_ROOMS) {
        for every(z, Z_ROOMS) {
            if (CheckBounds(x, z) && m_GameMap[x][z]) { // here
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

    auto ScatterHorizontal = [&](
        const usize scatter_count,
        const f32 y, 
        const u32 s_lower, const u32 s_upper,
        const u32 sy_lower, const u32 sy_upper,
        const u32 h_lower
    ) {
        for every(i, scatter_count) {
            u32 idx = GameRandom::Randu32(0, connected_all.size() - 1);

            u32 x = connected_all[idx].first;
            u32 z = connected_all[idx].second;
    
            f32 fx = (f32)GameRandom::Randu32(0, 2 * (u32)room_scale.x) - (room_scale.x / 2.0f);
            f32 fz = (f32)GameRandom::Randu32(0, 2 * (u32)room_scale.x) - (room_scale.x / 2.0f);
    
            f32 s = (f32)GameRandom::Randu32(s_lower, s_upper);
            f32 sy = (f32)GameRandom::Randu32(sy_lower, sy_upper);
            f32 h = (f32)GameRandom::Randu32(h_lower, (u32)sy) - (sy / 2.0f);
    
            Vec3 position = IndexToWorld(x, z);
            position.x += fx;
            position.z += fz;
            position.y = y + h;
    
            Entity e = Entity();
            ModelInstance mi;
            mi.model = ModelIndex::Cube;
            mi.texture = TextureIndex::White;
            mi.glow = Vec3(0.0f);
            mi.world = MoveScaleMatrix(position, Vec3(s, sy, s));
    
            m_ModelInstances.AddExisting(e, mi);
            m_ModelsActive.AddExisting(e);
        }
    };

    ScatterHorizontal(600, -room_scale.y, 100, 700, 5, 30, 5);
    ScatterHorizontal(600, room_scale.y, 100, 700, 5, 30, 5);

    auto ScatterWalls = [&](
        const usize scatter_count
    ) {
        for every(i, scatter_count) {
            u32 idx = GameRandom::Randu32(0, connected_walls.size() - 1);

            u32 x = connected_walls[idx].first;
            u32 z = connected_walls[idx].second;

        }
    };

    ScatterWalls(1000);

    // Sean:
    // randomly choose tiles to place lights into
    // but dont choose the same tile twice
    std::unordered_set<u64> light_placements;
    #define LIGHT_COUNT_ROOMS 40
    for every(i, LIGHT_COUNT_ROOMS) {
        u32 idx = GameRandom::Randu32(0, connected_walls.size() - 1);

        u32 x = connected_walls[idx].first;
        u32 z = connected_walls[idx].second;

        auto Check = [&](u32 cx, u32 cz) {
            return light_placements.find(make_key(x, z)) != light_placements.end();
        };

        if (Check(x, z) && Check(x - 1, z) && Check(x + 1, z) && Check(x, z - 1) && Check(x, z + 1)) {
            i -= 1; continue;
        }

        light_placements.insert(make_key(x, z));

        Vec3 pos = IndexToWorld(x, z); 

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

    { // spawn light in player room
        Vec3 pos = IndexToWorld(1, 1);

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

void CGame::DestroyRooms() {
}

void CGame::InitializeGeneration() {
}

f32 heuristic(Point2 a, Point2 b) {
    #define MAGIC_NUMBER 1.0f
    f32 out = fabs((f32)a.first - (f32)b.first) + fabs((f32)a.second - (f32)b.second);
    return out * MAGIC_NUMBER;
};

// Sean: returns end if end reached, otherwise it returns the closest point it could find to end
// Performance: flood-fills until all locations are filled, kinda slow
Point2 CGame::FindClosestPoint(Point2 start, Point2 end) {
    static std::unordered_set<u64> traversed = std::unordered_set<u64>();
    static std::vector<Point2> frontier =  std::vector<Point2>();

    if (start == end) { return start; } // same point
    if (m_GameMap[start.first][start.second] == false) { return start; } // start is invalid

    f32 best_dist = FLT_MAX;
    Point2 best_point = start;

    frontier.push_back(start);

    while (!frontier.empty()) {
        // Iterate current.

        Point2 current = frontier[frontier.size() - 1];
        if (current == end) { return current; }

        frontier.pop_back();
        traversed.insert(make_key(current.first, current.second));

        f32 dist = heuristic(current, end);
        if (dist < best_dist) {
            best_dist = dist;
            best_point = current;
        }

        // Find and add valid neighbors.

        auto AddConditionally = [&](i32 x, i32 z) {
            if (CheckBounds(x, z) && m_GameMap[x][z] && traversed.find(make_key(x, z)) == traversed.end()) {
                frontier.push_back(std::make_pair(x, z));
            }
        };

        i32 cx, cz;

        cx = current.first - 1; cz = current.second;
        AddConditionally(cx, cz);

        cx = current.first + 1; cz = current.second;
        AddConditionally(cx, cz);

        cx = current.first; cz = current.second - 1;
        AddConditionally(cx, cz);

        cx = current.first; cz = current.second + 1;
        AddConditionally(cx, cz);
    }

    traversed.clear();
    frontier.clear();

    return best_point;
}

std::unordered_set<u64> CGame::FindConnected(Point2 start) {
    static std::vector<Point2> frontier = std::vector<Point2>();
    std::unordered_set<u64> traversed = std::unordered_set<u64>();

    if (m_GameMap[start.first][start.second] == false) { return traversed; } // start is invalid

    frontier.push_back(start);

    while (!frontier.empty()) {
        // Iterate current.
        Point2 current = frontier[frontier.size() - 1];

        frontier.pop_back();
        traversed.insert(make_key(current.first, current.second));

        // Find and add valid neighbors.

        auto AddConditionally = [&](i32 x, i32 z) {
            if (CheckBounds(x, z) && m_GameMap[x][z] && traversed.find(make_key(x, z)) == traversed.end()) {
                frontier.push_back(std::make_pair(x, z));
            }
        };

        i32 cx, cz;

        cx = current.first - 1; cz = current.second;
        AddConditionally(cx, cz);

        cx = current.first + 1; cz = current.second;
        AddConditionally(cx, cz);

        cx = current.first; cz = current.second - 1;
        AddConditionally(cx, cz);

        cx = current.first; cz = current.second + 1;
        AddConditionally(cx, cz);
    }

    frontier.clear();

    return traversed;
}

// Sean: there are smarter ways to do this, like only resetting values that were touched
// or storing an associated bounding box of touched areas.
// This should be fast enough considering we're not storing a huge list.
template<typename T>
void Clear2dArray(std::array<std::array<T, Z_ROOMS>, X_ROOMS>& array2d, const T clear_value) {
    for every(x, array2d.size()) {
        for every(z, array2d[0].size()) {
            array2d[x][z] = clear_value;
        }
    }
}

// Sean: returns a list of points for going from start to end, vector length is ZERO if a path is not found.
// Performance: A* pathfinding, not slow relatively, but pathfinding is still slow in general
std::vector<Point2> CGame::Pathfind(Point2 start, Point2 end) {
    struct MinCostNode {
        f32 cost;
        f32 estimated;
        Point2 position;

        bool operator > (const MinCostNode& other) {
            return this->estimated > other.estimated;
        }

        bool operator < (const MinCostNode& other) {
            return this->estimated < other.estimated;
        }

        bool operator == (const MinCostNode& other) {
            return this->cost == other.cost && this->estimated == other.estimated;
        }

        static MinCostNode default() {
            MinCostNode d;
            d.cost = 0.0f;
            d.estimated = 0.0f;
            d.position = std::make_pair(0, 0);

            return d;
        }
    };

    // Sean: We declare these as static so their memory can be reused.
    static std::array<std::array<Point2, Z_ROOMS>, X_ROOMS> trail;
    static std::array<std::array<f32, Z_ROOMS>, X_ROOMS> f_score;
    static std::array<std::array<f32, Z_ROOMS>, X_ROOMS> g_score;
    static BinaryHeap<MinCostNode> open;

    // Sean: this is more relevant for subsequent calls
    Clear2dArray(trail, std::make_pair(UINT_MAX, UINT_MAX));
    Clear2dArray(f_score, FLT_MAX);
    Clear2dArray(g_score, FLT_MAX);
    open.Clear();

    bool found_path = false;
    g_score[start.first][start.second] = 0.0f;
    f_score[end.first][end.second] = heuristic(start, end);

    open.Push({FLT_MAX, FLT_MAX, start});

    Point2 last = start;

    // Trail building
    while (open.Size() != 0) {
        MinCostNode current = open.Pop();

        // Exit condition
        if (current.position == end) {
            trail[current.position.first][current.position.second] = last;
            found_path = true;
            break;
        }

        auto AddConditionally = [&](i32 x, i32 z) {
            if (!CheckBounds(x, z)) { return; }

            const f32 cost = 1.0f;
            f32 g = g_score[current.position.first][current.position.second] + cost;

            if (m_GameMap[x][z] && g < g_score[x][z]) {
                f32 f = g + heuristic(std::make_pair(x, z), end);

                if (trail[x][z] == std::make_pair(UINT_MAX, UINT_MAX)) {
                    MinCostNode node;
                    node.estimated = f;
                    node.cost = f;
                    node.position = std::make_pair(x, z);

                    open.Push(node);

                }

                trail[x][z] = current.position;
                g_score[x][z] = g;
                f_score[x][z] = f;
            }
        };

        i32 cx, cz;

        cx = (i32)current.position.first - 1; cz = (i32)current.position.second;
        AddConditionally(cx, cz);

        cx = (i32)current.position.first + 1; cz = (i32)current.position.second;
        AddConditionally(cx, cz);

        cx = (i32)current.position.first; cz = (i32)current.position.second - 1;
        AddConditionally(cx, cz);

        cx = (i32)current.position.first; cz = (i32)current.position.second + 1;
        AddConditionally(cx, cz);

        last = current.position;
    }

    // Path reconstruction
    std::vector<Point2> path;

    if (found_path) {
        Point2 current = end;
        Point2 previous = trail[current.first][current.second];

        while (previous != start) {
            path.push_back(current);
            current = previous;
            previous = trail[previous.first][previous.second];
        }
    }

    // Sean todo: make it so we dont have to reverse
    std::reverse(path.begin(), path.end());

    return path;
}
