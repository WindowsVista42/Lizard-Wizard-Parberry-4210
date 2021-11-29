#include "Game.h"
#include "Renderer.h"

void CGame::InitializeParticles() {
    for every(index, 80) {
        ParticleInstance instance;
        Entity e = m_ParticleInstances.Add(instance);
        m_ParticleInstancesCache.AddExisting(e);
    }
}

void CGame::SpawnParticles(ParticleInstanceDesc* desc) {
    if(m_ParticleInstancesCache.Size() < 1) {
        return;
    }

    Entity instance_e = m_ParticleInstancesCache.RemoveTail();
    m_ParticleInstancesActive.AddExisting(instance_e);

    ParticleInstance* instance = m_ParticleInstances.Get(instance_e);
    instance->initial_light_color = *(Vec4*)&desc->light_color;
    instance->light = m_pRenderer->lights.Add({ *(Vec4*)&desc->initial_pos, *(Vec4*)&desc->light_color });
    instance->model_instance.model = desc->model;
    instance->model_instance.texture = desc->texture;
    instance->model_instance.glow = desc->glow;
    instance->count = desc->count;
    instance->model_scale = desc->model_scale;

    f32 highest_timer = 0.0f;

    for every(index, instance->count) {
        Entity e = Entity();

        Particle particle;
        particle.pos = desc->initial_pos;

        f32 speed = desc->initial_speed * (1.0f - (GameRandom::Randf32() * desc->speed_randomness));

        Vec3 dir = JitterVec3(desc->initial_dir, -2.0f * desc->dir_randomness, 2.0f * desc->dir_randomness);
        particle.vel = speed * dir;

        Vec3 acc = JitterVec3(desc->initial_acc, -2.0f * desc->acc_randomness, 2.0f * desc->acc_randomness);
        particle.acc = acc;

        f32 alive_time = LinearLerp<f32>(desc->min_alive_time, desc->max_alive_time, GameRandom::Randf32());

        m_Particles.AddExisting(e, particle);
        m_EntityMapping.AddExisting(e, instance_e);
        m_Timers.AddExisting(e, alive_time);

        if (alive_time > highest_timer) { highest_timer = alive_time; }
    }

    m_Timers.AddExisting(instance_e, highest_timer);
    instance->highest_timer = highest_timer;
}

void CGame::StripParticleInstance(Entity e) {
    ParticleInstance* instance = m_ParticleInstances.Get(e);
    m_pRenderer->lights.Remove(instance->light);
    m_Timers.Remove(e);

    m_ParticleInstancesCache.AddExisting(e);
}

void CGame::StripParticle(Entity e) {
    Entity instance_e = *m_EntityMapping.Get(e);
    ParticleInstance* instance = m_ParticleInstances.Get(instance_e);
    instance->count -= 1;

    m_EntityMapping.Remove(e);
    m_Timers.Remove(e);
}