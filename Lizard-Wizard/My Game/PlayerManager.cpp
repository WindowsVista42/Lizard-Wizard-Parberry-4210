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
static Entity Hande;

static Group mana_orbs;
static Group health_orbs;

const static f32 move_timer = 0.05f;

static Vec3 staff_tip;

static Entity player_health_timer;
static Entity player_recast_timer;

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
    staff_mi.world = XMMatrixIdentity();
    staff_mi.glow = Vec3(0.0f);
    m_ModelInstances.AddExisting(Staffe, staff_mi);
    m_ModelsActive.AddExisting(Staffe);

    Cubee = Entity();
    ModelInstance cube_mi;
    cube_mi.model = ModelIndex::Cube;
    cube_mi.texture = TextureIndex::White;
    cube_mi.world = XMMatrixIdentity();
    cube_mi.glow = Vec3(0.7f, 0.7f, 0.7f);
    m_ModelInstances.AddExisting(Cubee, cube_mi);
    m_ModelsActive.AddExisting(Cubee);

    for every(index, m_DashAction.max_cooldown) {
        Entity e = mana_orbs.Add();

        ModelInstance mi;
        mi.model = ModelIndex::Cube;
        mi.texture = TextureIndex::White;
        mi.world = XMMatrixIdentity();
        mi.glow = Vec3(0.4f, 0.4f, 0.4f);
        m_ModelInstances.AddExisting(e, mi);
        m_ModelsActive.AddExisting(e);
    }

    //Hande = Entity();
    //ModelInstance hand_mi;
    //hand_mi.model = ModelIndex::Hand;
    //hand_mi.texture = TextureIndex::White;
    //hand_mi.world = XMMatrixIdentity();
    //hand_mi.glow = Vec3(0.0f);
    //m_ModelInstances.AddExisting(Hande, hand_mi);
    //m_ModelsActive.AddExisting(Hande);

    Health player_health = Health::New(4, 4);

    for every(index, player_health.max) {
        Entity e = health_orbs.Add();

        ModelInstance mi;
        mi.model = ModelIndex::Cube;
        mi.texture = TextureIndex::White;
        mi.world = XMMatrixIdentity();
        mi.glow = Vec3(1.0f, 0.4f, 0.4f);
        m_ModelInstances.AddExisting(e, mi);
        m_ModelsActive.AddExisting(e);
    }

    m_Healths.AddExisting(m_Player, player_health);
    m_Mana.AddExisting(m_Player, Mana::New(4, 0.5f));

    m_pRenderer->lights.AddExisting(m_Player, { Vec4(0), Vec4(50.0f, 10.0f, 5.0f, 0.0f)});
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

    // Swap Weapon
    if (m_pKeyboard->TriggerDown('1')) {
        m_WeaponSelection = ProjectileTypes::FIRE;
    }

    if (m_pKeyboard->TriggerDown('2')) {
        m_WeaponSelection = ProjectileTypes::ICE;
    }

    if (m_pKeyboard->TriggerDown('3')) {
        m_WeaponSelection = ProjectileTypes::LIGHTNING;
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
        Entity e = mana_orbs.Entities()[index];

        if (Ecs::ActivateAction(m_Timers, m_DashAction, e)) {
            // play sound

            // update entity to use
            index += 1;
            index %= mana_orbs.Size();
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

    {
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

        if (rayResults.hasHit() && m_InAir.Contains(m_Player)) {
            m_InAir.Remove(m_Player);
        } else if(!m_InAir.Contains(m_Player)) {
            m_InAir.AddExisting(m_Player);
        }


        if (!m_InAir.Contains(m_Player)) {
            m_JumpAction.timers.Clear();
        }
    }

    Mana* player_mana = m_Mana.Get(m_Player);
    f32* mana_timer = &player_mana->timer;
    btRigidBody* player_body = *m_RigidBodies.Get(m_Player);
    player_body->activate();


    // Projectiles
    if (m_leftClick.pressed && player_mana->value > 0) {
        switch (m_WeaponSelection) {
            case ProjectileTypes::ICE :
                GenerateSimProjectile(
                    *m_RigidBodies.Get(m_Player),
                    staff_tip,
                    m_pRenderer->m_pCamera->GetViewVector(),
                    1,
                    32000.0,
                    0.01,
                    Colors::DarkCyan,
                    true
                );
                break;
            case ProjectileTypes::LIGHTNING :
                GenerateSimProjectile(
                    *m_RigidBodies.Get(m_Player),
                    staff_tip,
                    m_pRenderer->m_pCamera->GetViewVector(),
                    3,
                    32000.0,
                    0.01,
                    Vec4(3.0f, 1.0f, 0.5f, 0.0f),
                    true
                );
                break;
            default:
                GenerateSimProjectile(
                    *m_RigidBodies.Get(m_Player),
                    staff_tip,
                    m_pRenderer->m_pCamera->GetViewVector(),
                    4,
                    12000.0,
                    0.1,
                    Colors::Red,
                    true
                );
                break;
        }
        *mana_timer = player_mana->Decrement(1);
    }

    // Note (Ethan) : Disabled until model casting is fixed.
    /*
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
    */

    if (flycam_enabled) {
        flycam_pos += movedir * flycam_speed * m_pTimer->GetFrameTime();
    }

    m_pRenderer->m_pCamera->SetYaw(player_yaw);
    m_pRenderer->m_pCamera->SetPitch(player_pitch);

    if (!flycam_enabled) {
        Light* light = m_pRenderer->lights.Get(m_Player);

        m_pRenderer->m_pCamera->MoveTo(Vec3(player_body->getWorldTransform().getOrigin()));
        light->position = *(Vec4*)&m_pRenderer->m_pCamera->GetPos();
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
            //f32 factor = 0.0f;
            //if (*time > 0.0f) { factor = 1.0f; }
            movedir += normal * factor;
        }

        {
            f32* time = m_Timers.Get(Ae);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            //f32 factor = 0.0f;
            //if (*time > 0.0f) { factor = 1.0f; }
            movedir -= normal * factor;
        }

        {
            f32* time = m_Timers.Get(We);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            //f32 factor = 0.0f;
            //if (*time > 0.0f) { factor = 1.0f; }
            movedir += lookdir * factor;
        }

        {
            f32* time = m_Timers.Get(Se);
            f32 factor = WindupWinddown(*time, 0.02f, 0.02f, move_timer);
            //f32 factor = 0.0f;
            //if (*time > 0.0f) { factor = 1.0f; }
            movedir -= lookdir * factor;
        }
    }

    if (movedir != Vec3(0)) { movedir.Normalize(); }

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

    // jump action timer connection
    Ecs::ApplyEvery(m_JumpAction.active, [=](Entity e) {
        f32* time = m_Timers.Get(e);
        if (*time < 0.0f) { return; }

        f32 factor = WindupWinddown(*time, m_JumpAction.windup, m_JumpAction.winddown, m_JumpAction.duration);

        Vec3 v = player_body->getLinearVelocity();
        player_body->setLinearVelocity(Vec3(v.x, 2000.0f * factor + 2000.0f, v.z));
    });

    // staff
    Vec3 staff_pos;
    Quat staff_rot;

    {
        ModelInstance* mi = m_ModelInstances.Get(Staffe);
        LBaseCamera* cam = m_pRenderer->m_pCamera;

        staff_pos = RotatePointAroundOrigin(
            cam->GetPos(), 
            Vec3(80.0f, -60.0f, 100.0f), 
            Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch(), cam->GetRoll())
        );

        staff_rot = Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch() + 0.6f, 0.0f);
        Vec3 scl = Vec3(100.0f);

        mi->world = MoveRotateScaleMatrix(staff_pos, staff_rot, scl);
    }

    // cube on top of staff
    {
        ModelInstance* mi = m_ModelInstances.Get(Cubee);

        Vec3 particle_pos = RotatePointAroundOrigin(
            staff_pos,
            Vec3(0.0f, 100.0f, 0.0f),
            staff_rot
        );

        Vec3 scl = Vec3(10.0f);
        Vec4 clr;
        staff_tip = particle_pos;

        f32 lum = 50.0f;

        // Swap light color based on projectile selection.
        switch (m_WeaponSelection) {
        case ProjectileTypes::ICE:
            clr = Vec4(0.1f, 0.6f, 1.0f, 0.0f);
            break;
        case ProjectileTypes::LIGHTNING:
            clr = Vec4(0.85f, 0.85f, 0.1f, 0.0f);
            break;
        default:
            clr = Colors::Red;
            break;
        }

        static Entity light = m_pRenderer->lights.Add({Vec4(0), Vec4(70.0, 8.0, 2.5, 0.0)});
        m_pRenderer->lights.Get(light)->position = *(Vec4*)&particle_pos;
        m_pRenderer->lights.Get(light)->color = clr * lum;
        mi->world = MoveScaleMatrix(particle_pos, scl);
        mi->glow = Vec3((f32)player_mana->value / (f32)player_mana->max); // switch back to this if we find its easier to tell

        //printf("%d\n", player_mana->value);
    }

    // cubes that spin around staff
    f32 index = 1.0f;
    Ecs::ApplyEvery(mana_orbs, [&](Entity e) {
        ModelInstance* mi = m_ModelInstances.Get(e);

        auto RotationTranslation = [](Vec3 origin, Vec3 offset, Quat quat) {
            Mat4x4 rot = XMMatrixRotationQuaternion(quat);
            return origin + Vec3(XMVector3Transform(offset, rot));
        };

        f32 t = m_pTimer->GetTime() + (M_PI * 2.0f * (index / (f32)mana_orbs.Size()));

        f32 radius = 20.0f;
        f32 height = 40.0f;
        Vec3 particle_offset = Vec3(radius * sinf(t), height, radius * cosf(t));

        Vec3 particle_pos = RotationTranslation(staff_pos, particle_offset, staff_rot);
        Vec3 scl = Vec3(4.0f);

        mi->world = MoveScaleMatrix(particle_pos, scl);
        mi->glow = Vec3(1.0f);

        index += 1.0f;
    });

    // Hand
    Vec3 hand_pos;
    Quat hand_rot;

    //auto SetModelFancy = [&](ModelInstance* mi, Vec3 origin, Vec3 offset, Vec3 orientation, Vec3 orientation_offset, Vec3 scale) {
    //    Vec3 pos = RotatePointAroundOrigin(
    //        origin, 
    //        offset, 
    //        Quat::CreateFromYawPitchRoll(orientation.x, orientation.y, orientation.z)
    //    );

    //    Quat rot = Quat::CreateFromYawPitchRoll(
    //        orientation.x + orientation_offset.x, 
    //        orientation.y + orientation_offset.y, 
    //        orientation.z + orientation_offset.z
    //    );

    //    Vec3 scl = scale;

    //    mi->world = MoveRotateScaleMatrix(pos, rot, scl);
    //};

    //{
    //    ModelInstance* mi = m_ModelInstances.Get(Hande);
    //    LBaseCamera* cam = m_pRenderer->m_pCamera;

    //    //SetModelFancy(
    //    //     mi,
    //    //     cam->GetPos(),
    //    //     Vec3(-80.0f, -60.0f, 100.0f),
    //    //     Vec3(cam->GetYaw(), cam->GetPitch(), cam->GetRoll()),
    //    //     Vec3(0.0f, M_PI / 2.0f, 0.0f), Vec3(100.0f)
    //    //);

    //    hand_pos = RotatePointAroundOrigin(
    //        cam->GetPos(), 
    //        Vec3(-40.0f, -60.0f, 100.0f), 
    //        Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch(), cam->GetRoll())
    //    );

    //    hand_rot = Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch(), 0.0f);
    //    Vec3 scl = Vec3(100.0f);

    //    mi->world = MoveRotateScaleMatrix(hand_pos, hand_rot, scl);
    //}

    // Cubes that spin around hand
    {
        Health* player_health = m_Healths.Get(m_Player);
    }

    // dash action timers connection to cubes that spin
    u32 i = 0;
    Ecs::ApplyEvery(m_DashAction.timers, [&](Entity e) {
        ModelInstance* mi = m_ModelInstances.Get(e);
        f32 t = *m_Timers.Get(e);

        mi->glow = Vec3(LinearLerp<f32>(1.0f, 0.0f, t / m_DashAction.cooldown));

        i += 1;
    });

    // remove old timers
    auto CheckTimer = [=](Entity e) { return *m_Timers.Get(e) <= 0.0f; };
    auto CheckTimerDash = [=](Entity e) { return *m_Timers.Get(e) <= -m_DashAction.delay; };
    auto CheckTimerJump = [=](Entity e) { return *m_Timers.Get(e) <= -m_JumpAction.delay; };
    auto RemoveTimer = [=](Entity e) { m_Timers.Remove(e); };

    // Check Jump Timer
    Ecs::RemoveConditionally(m_JumpAction.active, CheckTimer, RemoveTimer);
    Ecs::RemoveConditionally(m_JumpAction.timers, CheckTimerJump, RemoveTimer);

    // Check Dash Timer
    Ecs::RemoveConditionally(m_DashAction.active, CheckTimer, RemoveTimer);
    Ecs::RemoveConditionally(m_DashAction.timers, CheckTimerDash, RemoveTimer);
}

void CGame::RenderPlayer() {
}