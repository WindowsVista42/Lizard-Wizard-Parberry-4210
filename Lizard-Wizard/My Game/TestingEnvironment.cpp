#include "Game.h"
#include "Defines.h"
#include "Ecs.h"

ModelInstance GetBoxModel(btRigidBody* body) {
    ModelInstance instance = {};
    btCollisionShape* currentShape = body->getCollisionShape();
    btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);


    instance.model = (u32)ModelIndex::Cube;
    instance.world = MoveRotateScaleMatrix(body->getWorldTransform().getOrigin(), *(Quat*)&body->getWorldTransform().getRotation(), boxShape->getHalfExtentsWithMargin());
    instance.texture = 1;

    return instance;
}

void CGame::CreateTestingEnvironment() {
    const Vec3 position = Vec3(-10000.0, 0.0, -10000.0);
    const Vec3 scale = Vec3(6000.0, 1000.0, 6000.0);

    const f32 thickness = 50.0f;
    const u32 group = 0b00001;
    const u32 flag = 0b11111;

    // floor
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, thickness, scale.z), position + Vec3(0.0, -scale.y, 0.0), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        // Note (Ethan) :  For now we just have to deal with this.
        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // ceiling
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, thickness, scale.z), position + Vec3(0.0, scale.y, 0.0), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // wall -X
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(thickness, scale.y, scale.z), position + Vec3(-scale.x, 0.0, 0.0), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // wall +X
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(thickness, scale.y, scale.z), position + Vec3(scale.x, 0.0, 0.0), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // wall -Z
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, thickness), position + Vec3(0.0, 0.0, -scale.z), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // wall +Z
    {
        Entity e = Entity();
        m_TestingWallsFloors.AddExisting(e);
        m_RigidBodies.AddExisting(e, CreateBoxObject(Vec3(scale.x, scale.y, thickness), position + Vec3(0.0, 0.0, scale.z), 0.0, 1.0, group, flag));
        m_TestingWallsFloors.AddExisting(e);

        m_ModelInstances.AddExisting(e, GetBoxModel(*m_RigidBodies.Get(e)));
        m_ModelsActive.AddExisting(e);
    }

    // Lights
    {
        const f32 splat = 100.0f;
        const Vec3 light_scale = Vec3(splat, splat, splat);
        const Vec3 light_color = Colors::LightYellow * 200.0;

        const i32 count = 4;
        const f32 dx = (scale.x * 2.0) / (f32)count;
        const f32 dz = (scale.z * 2.0) / (f32)count;
        for (i32 x = 0; x < count; x += 1) {
            f32 fx = (f32)(x - count/2);
            for (i32 z = 0; z < count; z += 1) {
                f32 fz = (f32)(z - count/2);

                Vec3 light_pos = Vec3(
                    position.x + (fx * dx) + (dx / 2.0),
                    position.y + scale.y - light_scale.y - 50.0,
                    position.z + (fz * dz) + (dz / 2.0)
                );

                ModelInstance mi;
                mi.model = ModelIndex::Cube;
                mi.texture = 0u;
                mi.world = MoveScaleMatrix(light_pos, light_scale);

                Entity e = Entity();
                m_ModelInstances.AddExisting(e, mi);
                m_ModelsActive.AddExisting(e);
                m_pRenderer->lights.AddExisting(e, {*(Vec4*)&light_pos, *(Vec4*)&light_color});

                m_TestingLights.AddExisting(e);
            }
        }
    }

    // Model showcase
    {
        const f32 splat = 200.0f;
        const Vec3 model_scale = Vec3(splat, splat, splat);

        const i32 count = ModelIndex::Count;
        for every(index, ModelIndex::Count) {
            i32 iz = (i32)index;
            f32 dz = (model_scale.z * 6.0) * (f32)(iz - (count / 2));

            Vec3 model_pos = Vec3(position.x - scale.x + splat + 300.0f, position.y - scale.y + splat + 100.0f, position.z + dz);

            ModelInstance mi;
            mi.model = index;
            mi.texture = 0u;
            mi.world = MoveRotateScaleMatrix(model_pos, Quaternion::CreateFromAxisAngle(Vec3(0.0, 1.0, 0.0), M_PI / 2.0f), model_scale);

            Entity e = Entity();
            m_ModelInstances.AddExisting(e, mi);
            m_ModelsActive.AddExisting(e);
            m_pRenderer->lights.AddExisting(e, {*(Vec4*)&model_pos, Vec4(0.2, 0.98, 0.5, 1.0) * 200.0f});

            m_TestingModels.AddExisting(e);
        }
    }
}
