#ifndef ProjectileManager_H
#define ProjectileManager_H

// Inclusions
#include <stdio.h>
#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include "CustomBind.h"
#include <vector>

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

// Respective Structs
struct RayProjectile {
    Vector3 Pos1;
    Vector3 Pos2;
    XMVECTORF32 Color;
};

// Projectile Class (Can be either a Bullet3 ray or a Bullet3 simulated projectile.)
class ProjectileManager {

    public:
        void GenerateSimProjectile(const Vec3, const Vec3, const f32, const i32, const i32, const Vec4);
        void GenerateRayProjectile(const Vec3, const Vec3, const i32, const i32, const i32, const Vec4, const b8);
        void InitializeProjectiles(btAlignedObjectArray<btCollisionShape*>, std::vector<RayProjectile>*, btDiscreteDynamicsWorld*);
        void DestroyAllProjectiles();

    private:
        void CalculateRay(RayProjectile&, Vec3, Vec3, Vec3, i32, Vec4);

        btAlignedObjectArray<btCollisionShape*> currentSimProjectiles;
        std::vector<RayProjectile>* currentRayProjectiles;
        btDiscreteDynamicsWorld* currentWorld;
        i32 randomSeed;
};


#endif // !PROJECTILES_H
