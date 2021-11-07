#include "Game.h"
#include "Defines.h"
#include "Interpolation.h"
#include <ComponentIncludes.h>
#include <vector>

static Vec3 movedir = Vec3::Zero;
static Entity De;
static Entity Ae;
static Entity We;
static Entity Se;

static Entity Staffe;
static Entity Cubee;

static Group orbs;

const static f32 move_timer = 0.05f;

static Vec3 staff_tip;

f32 WindupWinddown(const f32 time, const f32 windup, const f32 winddown, const f32 duration) {
    if (time < 0.0f) {
        return 0.0f;
    } else if (time < winddown) {
        return time / winddown;
    } else if (time > (duration - windup)) {
        return (duration - time) / windup;
    } else {
        return 1.0f;
    }
}

void CGame::InitializePlayer() {
    m_DashAction.max_active = 1;
    m_DashAction.max_cooldown = 3;
    m_DashAction.duration = 0.2f;
    m_DashAction.cooldown = 4.0f;
    m_DashAction.windup = 0.05f;
    m_DashAction.winddown = 0.05f;
    m_DashAction.delay = 0.0f;

    m_JumpAction.max_active = 1;
    m_JumpAction.max_cooldown = 1;
    m_JumpAction.duration = 0.2f;
    m_JumpAction.cooldown = 128.0f;
    m_JumpAction.windup = 0.02f;
    m_JumpAction.winddown = 0.1f;
    m_JumpAction.delay = 0.5f;

    De = m_Timers.Add(move_timer);
    Ae = m_Timers.Add(move_timer);
    We = m_Timers.Add(move_timer);
    Se = m_Timers.Add(move_timer);

    Staffe = Entity();
    ModelInstance staff_mi;
    staff_mi.model = ModelIndex::Staff;
    staff_mi.texture = TextureIndex::White;
    staff_mi.world = MoveScaleMatrix(Vec3(0.0f), Vec3(1000.0));
    m_ModelInstances.AddExisting(Staffe, staff_mi);
    m_ModelsActive.AddExisting(Staffe);

    Cubee = Entity();
    ModelInstance cube_mi;
    cube_mi.model = ModelIndex::Cube;
    cube_mi.texture = TextureIndex::White;
    cube_mi.world = MoveScaleMatrix(Vec3(0.0f), Vec3(1000.0));
    cube_mi.glow = Vec3(0.7, 0.7, 0.7);
    m_ModelInstances.AddExisting(Cubee, cube_mi);
    m_ModelsActive.AddExisting(Cubee);

    for every(index, m_DashAction.max_cooldown) {
        Entity e = orbs.Add();

        ModelInstance mi;
        mi.model = ModelIndex::Cube;
        mi.texture = TextureIndex::White;
        mi.world = XMMatrixIdentity();
        mi.glow = Vec3(0.4, 0.4, 0.4);

        m_ModelInstances.AddExisting(e, mi);
        m_ModelsActive.AddExisting(e);
    }
}

void CGame::PlayerInput() {
      //TODO(sean): make this framerate dependant
    if (m_pKeyboard->Down(VK_RIGHT))
        player_yaw += player_sens;
    if (m_pKeyboard->Down(VK_LEFT))
        player_yaw -= player_sens;
    if (m_pKeyboard->Down(VK_UP))
        player_pitch -= player_sens;
    if (m_pKeyboard->Down(VK_DOWN))
        player_pitch += player_sens;

    // Flycam toggle
    if (m_pKeyboard->TriggerDown('F')) {
        flycam_enabled = !flycam_enabled;
    }

    // Print screenshot button thing that will do stuff
    if (m_pKeyboard->TriggerDown('P')) {
        m_pRenderer->m_screenShot = true;
    }

    {
        if (m_pKeyboard->TriggerDown('D')) {
            *m_Timers.Get(De) = move_timer;
        } else if (m_pKeyboard->Down('D')) {
            f32* time = m_Timers.Get(De);
            if (*time < 0.1f) {
                *time = move_timer / 2.0f;
            }
        }

        if (m_pKeyboard->TriggerDown('A')) {
            *m_Timers.Get(Ae) = move_timer;
        } else if (m_pKeyboard->Down('A')) {
            f32* time = m_Timers.Get(Ae);
            if (*time < 0.1f) {
                *time = move_timer / 2.0f;
            }
        }

        if (m_pKeyboard->TriggerDown('W')) {
            *m_Timers.Get(We) = move_timer;
        } else if (m_pKeyboard->Down('W')) {
            f32* time = m_Timers.Get(We);
            if (*time < 0.1f) {
                *time = move_timer / 2.0f;
            }
        }

        if (m_pKeyboard->TriggerDown('S')) {
            *m_Timers.Get(Se) = move_timer;
        } else if (m_pKeyboard->Down('S')) {
            f32* time = m_Timers.Get(Se);
            if (*time < 0.1f) {
                *time = move_timer / 2.0f;
            }
        }

        if (flycam_enabled) {
            if (m_pKeyboard->Down(VK_SPACE)) {
                movedir += Vector3::UnitY;
            }
            if (m_pKeyboard->Down(VK_LCONTROL)) {
                movedir -= Vector3::UnitY;
            }
        }
    }

    if (m_pKeyboard->TriggerDown(VK_LSHIFT)) {
        static u32 index = 0;
        Entity e = orbs.Entities()[index];

        if (Ecs::ActivateAction(m_Timers, m_DashAction, e)) {
            // play sound

            // update entity to use
            index += 1;
            index %= orbs.Size();
        };
    }

    if (m_pKeyboard->TriggerDown(VK_SPACE)) {
        if (Ecs::ActivateAction(m_Timers, m_JumpAction)) {
            // play sound
        }
    }

    //TODO(sean): Ignore input if user has just refocused on the window
    static bool mouse_toggle = true;
    static bool just_mouse_toggle = false;
    if (m_pKeyboard->TriggerDown(VK_ESCAPE)) {
        mouse_toggle = !mouse_toggle;
        just_mouse_toggle = true;
    } else {
        just_mouse_toggle = false;
    }

    if(m_pRenderer->GetHwnd() == GetFocus()) { // check if focused window is us
        // I see you looking at 
        //    _____   this
        //     . .
        //
        // i came, i saw, i praised, the lord, then break the law.
        // you flipped it upside down :(

        RECT rect;
        GetWindowRect(m_pRenderer->GetHwnd(), &rect);
        POINT center = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
        const f32 mouse_sensitivity = 0.00333;

        // dont do anything if mouse is not hidden
        POINT cursor_pos;
        if (mouse_toggle) {
            GetCursorPos(&cursor_pos);
            SetCursorPos(center.x, center.y);
            if (just_mouse_toggle) {
                ShowCursor(0); // NOTE(sean): win32 hide the cursor
                cursor_pos.x = center.x;
                cursor_pos.y = center.y;
            }
        } else {
            cursor_pos.x = center.x;
            cursor_pos.y = center.y;

            if (just_mouse_toggle) {
                ShowCursor(1); // NOTE(sean): win32 hide the cursor
            }
        }

        Vector2 delta = { (f32)(cursor_pos.x - center.x), (f32)(cursor_pos.y - center.y) };
        delta *= mouse_sensitivity;

        static const f32 pitch_bounds = XM_2PI / 4.0 - 0.05;

        player_yaw += delta.x;
        if (player_yaw > XM_2PI) {
            player_yaw -= XM_2PI;
        } else if (player_yaw < 0.0) {
            player_yaw += XM_2PI;
        }

        player_pitch += delta.y;
        if (player_pitch > pitch_bounds) {
            player_pitch = pitch_bounds;
        } else if (player_pitch < -pitch_bounds) {
            player_pitch = -pitch_bounds;
        }
    }
}

void CGame::UpdatePlayer() {
    // Check Player Location
    {
        Vec3 Pos = m_pRenderer->m_pCamera->GetPos();
        Vec3 Direction = Pos + Vec3(0, -1.0f, 0) * 255.0f;

        btCollisionWorld::ClosestRayResultCallback rayResults(Pos, Direction);
        m_pDynamicsWorld->rayTest(Pos, Direction, rayResults);
        rayResults.m_collisionFilterGroup = 0b00100;
        rayResults.m_collisionFilterMask = 0b00001;

        btCollisionObject* hitObject = const_cast<btCollisionObject*>(rayResults.m_collisionObject);

        Vec3 hitPosition = rayResults.m_hitPointWorld;

        if (rayResults.hasHit()) {
            printf("On ground.\n");
        } else {
            printf("In air.\n");
        }

        if (rayResults.hasHit() && m_InAir.Contains(m_Player)) {
            printf("Player back on ground.\n");
            m_InAir.Remove(m_Player);
        } else if(rayResults.hasHit()) {
            printf("On ground.\n");
        } else {
            printf("In air.\n");
            if (!m_InAir.Contains(m_Player)) {
                m_InAir.AddExisting(m_Player);
            }
        }

        if (!m_InAir.Contains(m_Player)) {
            m_JumpAction.timers.Clear();
        }
    }


    if (m_leftClick.pressed) {
        GenerateSimProjectile(
            *m_RigidBodies.Get(m_Player),
            staff_tip,
            m_pRenderer->m_pCamera->GetViewVector(), 
            3, 
            8000.0, 
            0.5, 
            Colors::PaleVioletRed,
            true
        );
    }

    if (m_rightClick.pressed) {
        GenerateRayProjectile(
            *m_RigidBodies.Get(m_Player),
            staff_tip, 
            m_pRenderer->m_pCamera->GetViewVector(), 
            3, 
            2, 
            0.05, 
            Colors::IndianRed, 
            false, 
            true
        );
    }

    if (flycam_enabled) {
        flycam_pos += movedir * flycam_speed * m_pTimer->GetFrameTime();
    }

    m_pRenderer->m_pCamera->SetYaw(player_yaw);
    m_pRenderer->m_pCamera->SetPitch(player_pitch);

    if (!flycam_enabled) {
        btRigidBody* body = *m_RigidBodies.Get(m_Player);

        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        Light* light = m_pRenderer->lights.Get(m_Player);

        m_pRenderer->m_pCamera->MoveTo(*(Vector3*)&trans.getOrigin());
        light->position = *(Vec4*)&trans.getOrigin();

    } else {
        m_pRenderer->m_pCamera->MoveTo(flycam_pos);
    }

    {
        movedir = Vec3::Zero;
        Vec3 lookdir = m_pRenderer->m_pCamera->GetViewVector();
        lookdir = { lookdir.x, 0.0f, lookdir.z };
        Vec3 normal = { lookdir.z, 0.0f, -lookdir.x };

        {
            f32* time = m_Timers.Get(De);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            movedir += normal * factor;
        }

        {
            f32* time = m_Timers.Get(Ae);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            movedir -= normal * factor;
        }

        {
            f32* time = m_Timers.Get(We);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            movedir += lookdir * factor;
        }

        {
            f32* time = m_Timers.Get(Se);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            movedir -= lookdir * factor;
        }
    }

    if (movedir != Vec3(0)) { movedir.Normalize(); }

    btRigidBody* player_body = *m_RigidBodies.Get(m_Player);
    player_body->activate();

    static const f32 move_speed = 3000.0f;

    Vec3 newvel = Vec3(Vec3(0, player_body->getLinearVelocity().y(), 0) + Vec3(movedir * move_speed));
    player_body->setLinearVelocity(newvel);

    // Not sure about this syntax, might change because it might explode compile times
    Ecs::ApplyEvery(m_DashAction.active, [=](Entity e) {
        f32* time = m_Timers.Get(e);
        if (*time < 0.0f) { return; }

        f32 factor = WindupWinddown(*time, m_DashAction.windup, m_DashAction.winddown, m_DashAction.duration);;

        Vec3 bodyvel = Vec3(newvel.x, 0.0f, newvel.z);
        player_body->setLinearVelocity(Vec3(bodyvel + (movedir * 10000.0f * factor)));
    });

    Ecs::ApplyEvery(m_JumpAction.active, [=](Entity e) {
        f32* time = m_Timers.Get(e);
        if (*time < 0.0f) { return; }

        f32 factor = WindupWinddown(*time, m_JumpAction.windup, m_JumpAction.winddown, m_JumpAction.duration);

        Vec3 v = player_body->getLinearVelocity();
        player_body->setLinearVelocity(Vec3(v.x, 2000.0f * factor + 2000.0f, v.z));
    });

    auto CheckTimer = [=](Entity e) {return *m_Timers.Get(e) <= 0.0f; };
    auto CheckTimerDash = [=](Entity e) { return *m_Timers.Get(e) <= -m_DashAction.delay; };
    auto CheckTimerJump = [=](Entity e) { return *m_Timers.Get(e) <= -m_JumpAction.delay; };
    auto RemoveTimer = [=](Entity e) { m_Timers.Remove(e); };

    Vec3 staff_pos;
    Quat staff_rot;
    {
        ModelInstance* mi = m_ModelInstances.Get(Staffe);
        LBaseCamera* cam = m_pRenderer->m_pCamera;

        Vec3 staff_offset = Vec3(80.0f, -60.0f, 100.0f);
        Mat4x4 camera_rotation = XMMatrixRotationRollPitchYaw(cam->GetPitch(), cam->GetYaw(), cam->GetRoll());

        staff_pos = cam->GetPos() + Vec3(XMVector3Transform(staff_offset, camera_rotation));
        staff_rot = Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch() + 0.6f, 0.0f);
        Vec3 scl = Vec3(100.0f);

        mi->world = MoveRotateScaleMatrix(staff_pos, staff_rot, scl);
    }

    {
        ModelInstance* mi = m_ModelInstances.Get(Cubee);

        Vec3 particle_offset = Vec3(0.0f, 100.0f, 0.0f);
        Mat4x4 staff_rotation = XMMatrixRotationQuaternion(staff_rot);

        Vec3 particle_pos = staff_pos + Vec3(XMVector3Transform(particle_offset, staff_rotation));
        Vec3 scl = Vec3(10.0f);

        staff_tip = particle_pos;

        static Entity light = m_pRenderer->lights.Add({Vec4(0), Vec4(15.0, 8.0, 2.5, 0.0)});
        m_pRenderer->lights.Get(light)->position = *(Vec4*)&particle_pos;
        mi->world = MoveScaleMatrix(particle_pos, scl);
    }

    f32 index = 1.0f;
    Ecs::ApplyEvery(orbs, [&](Entity e) {
        ModelInstance* mi = m_ModelInstances.Get(e);

        auto RotationTranslation = [](Vec3 origin, Vec3 offset, Quat quat) {
            Mat4x4 rot = XMMatrixRotationQuaternion(quat);
            return origin + Vec3(XMVector3Transform(offset, rot));
        };

        f32 t = m_pTimer->GetTime() + (M_PI * 2.0f * (index / (f32)orbs.Size()));

        f32 radius = 20.0f;
        f32 height = 40.0f;
        Vec3 particle_offset = Vec3(radius * sinf(t), height, radius * cosf(t));

        Vec3 particle_pos = RotationTranslation(staff_pos, particle_offset, staff_rot);
        Vec3 scl = Vec3(4.0f);

        mi->world = MoveScaleMatrix(particle_pos, scl);
        mi->glow = Vec3(1.0f);

        index += 1.0f;
    });

    u32 i = 0;
    Ecs::ApplyEvery(m_DashAction.timers, [&](Entity e) {
        ModelInstance* mi = m_ModelInstances.Get(e);
        f32 t = *m_Timers.Get(e);

        mi->glow = Vec3(LinearLerp<f32>(1.0f, 0.0f, t / m_DashAction.cooldown));

        i += 1;
    });

    // Check Jump Timer
    Ecs::RemoveConditionally(m_JumpAction.active, CheckTimer, RemoveTimer);
    Ecs::RemoveConditionally(m_JumpAction.timers, CheckTimerJump, RemoveTimer);

    // Check Dash Timer
    Ecs::RemoveConditionally(m_DashAction.active, CheckTimer, RemoveTimer);
    Ecs::RemoveConditionally(m_DashAction.timers, CheckTimerDash, RemoveTimer);
}

void CGame::RenderPlayer() {
}