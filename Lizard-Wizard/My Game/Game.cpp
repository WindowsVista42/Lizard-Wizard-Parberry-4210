/// \file Game.cpp
/// \brief Code for the game class CGame.

#include "Game.h"
#include "Math.h"
#include "Renderer.h"
#include "SpriteRenderer.h"
#include "ComponentIncludes.h"
#include "Helpers.h"
#include "shellapi.h"
#include "Interpolation.h"
#include <vector>
#include <iostream>

CGame::~CGame() {
    // The OS will clean up allocations much faster than I could ever imagine.
}

void CGame::Initialize() {
    m_pRenderer = new Renderer();
    m_pRenderer->Initialize();

    {
        constexpr usize alloc_size = 4 * 1024 * 1024; // 4 MB
        u8* ptr = new u8[alloc_size];
        m_PhysicsAllocator.Init(ptr, alloc_size);
    }

    LoadImages(); // load images from xml file list
    LoadModels(); // load models from xml file list
    LoadSounds(); // load the sounds for this game

    // Lets bind this action to to the user's mouse.
    // For key values : https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    m_leftClick = CustomBind::New(VK_LBUTTON);
    m_rightClick = CustomBind::New(VK_RBUTTON);

    BeginGame();
}

void CGame::LoadModels() {
    //m_pRenderer->LoadDebugModel("cube", Colors::Peru, DebugModelIndex::);
    m_pRenderer->LoadModel("cube", ModelIndex::Cube);
    m_pRenderer->LoadModel("suzanne", ModelIndex::Suzanne);
    m_pRenderer->LoadModel("obelisk", ModelIndex::Obelisk);
    m_pRenderer->LoadModel("sentry", ModelIndex::Sentry);
    m_pRenderer->LoadModel("staff", ModelIndex::Staff);
    m_pRenderer->LoadModel("hand", ModelIndex::Hand);
    m_pRenderer->LoadModel("quad", ModelIndex::Quad);
    m_pRenderer->LoadModel("cool_cube", ModelIndex::CoolCube);
}

void CGame::LoadImages() {
    m_pRenderer->BeginResourceUpload();

    m_pRenderer->LoadTextureI("sample", TextureIndex::Other);
    m_pRenderer->LoadTextureI("white", TextureIndex::White);

    m_pRenderer->EndResourceUpload();
}

void CGame::LoadSounds(){
    m_pAudio->Initialize(SoundIndex::Size);
    // ???
    m_pAudio->Load(SoundIndex::Grunt, "grunt");
    m_pAudio->Load(SoundIndex::Clang, "clang");
    m_pAudio->Load(SoundIndex::Impact, "impact");

    // Casting Noises
    m_pAudio->Load(SoundIndex::FireCast, "FireCast");
    m_pAudio->Load(SoundIndex::IceCast, "IceCast");
    m_pAudio->Load(SoundIndex::LightningCast, "LightningCast");

    // Impact Noises
    m_pAudio->Load(SoundIndex::FireImpact1, "FireImpact1");
    m_pAudio->Load(SoundIndex::IceImpact1, "IceImpact1");
    m_pAudio->Load(SoundIndex::PlayerImpact1, "PlayerImpact1");
    m_pAudio->Load(SoundIndex::EnemyImpactMetal1, "EnemyImpactMetal1");
    m_pAudio->Load(SoundIndex::EnemyImpactMetal2, "EnemyImpactMetal2");

    // Player Noises
    m_pAudio->Load(SoundIndex::Dash1, "Dash1");
    m_pAudio->Load(SoundIndex::Dash2, "Dash2");
    m_pAudio->Load(SoundIndex::Dash3, "Dash3");
    m_pAudio->Load(SoundIndex::Dash4, "Dash4");
    m_pAudio->Load(SoundIndex::PlayerWalk1, "PlayerWalk1");
    m_pAudio->Load(SoundIndex::PlayerWalk2, "PlayerWalk2");
    m_pAudio->Load(SoundIndex::PlayerLand1, "PlayerLand1");
}

void CGame::Release() {
    // Release all of the DirectX12 objects by deleting the renderer.
    delete m_pRenderer;
}

void CGame::ResetGame() {
    m_Timers.Clear();
    m_Animations.Clear();
    m_Menu.Clear();
    m_Panel.Clear();
    m_PanelText.Clear();
    m_MainMenu.Clear();
    m_SettingsMenu.Clear();
    m_PauseMenu.Clear();
    m_RigidBodyMap.clear();

    for every(index, m_RigidBodies.Size()) {
        m_pDynamicsWorld->removeRigidBody(m_RigidBodies.Components()[index]);
    }
    m_PhysicsAllocator.Reset();
    m_RigidBodies.Clear();

    m_Projectiles.Clear();
    m_ProjectilesCache.Clear();
    m_ProjectilesActive.Clear();
    m_WeaponSelection = ProjectileTypes::FIRE;
    m_Rays.Clear();
    m_RaysCache.Clear();
    m_RaysActive.Clear();
    m_NPCs.Clear();
    m_NPCsCache.Clear();
    m_NPCsActive.Clear();
    m_ModelInstances.Clear();
    m_ModelsActive.Clear();
    m_EntityMapping.Clear();
    m_Healths.Clear();
    m_Mana.Clear();
    m_InAir.Clear();

    m_CollisionPairs.Clear();
    m_CurrentCollisions.Clear();
    m_Transforms.Clear();
    m_Particles.Clear();
    m_ParticleInstances.Clear();
    m_ParticleInstancesCache.Clear();
    m_ParticleInstancesActive.Clear();
    m_TestingLights.Clear();
    m_TestingWallsFloors.Clear();
    m_TestingModels.Clear();
    m_TestingModels.Clear();
    m_RoomLights.Clear();
    m_RoomWallsFloors.Clear();
    m_RoomModels.Clear();

    m_pRenderer->lights.Clear();

    m_DashAction.active.Clear();
    m_DashAction.timers.Clear();

    m_JumpAction.active.Clear();
    m_JumpAction.timers.Clear();

    m_PlayerManaOrbs.Clear();
    m_PlayerHealthOrbs.Clear();

    //for every(index, m_pCollisionShapes.size()) {
    //    delete m_pCollisionShapes[index];
    //}
    m_pCollisionShapes.clear();

    //delete m_pDynamicsWorld;
    m_currentRayProjectiles.clear();

    m_reset = false;

    BeginGame();
}

void CGame::BeginGame() {
    // Initialize Managers
    m_Player = Entity();
    InitializePhysics();
    InitializeGeneration();
    InitializeProjectiles();
    InitializeNPCs();
    InitializePlayer();
    InitializeMenu();
    InitializeParticles();

    GenerateRooms(Vec3(0, 0, 0), 100, 300, 100);

    RBTeleportLaunch(*m_RigidBodies.Get(m_Player), Vec3(0.0f, -500.0f, 0.0f), Vec3(0));
    if (!flycam_enabled) {
        player_yaw = 0.0f;
        player_pitch = 0.0f;
    }
}

/// Poll for player input
void CGame::InputHandler() {
    if (!(m_pRenderer->GetHwnd() == GetFocus())) { return; }

    m_pKeyboard->GetState();

    if (m_pKeyboard->TriggerDown(VK_F1))
        ShellExecute(0, 0, "https://larc.unt.edu/code/blank/", 0, 0, SW_SHOW);

    if (m_pKeyboard->TriggerDown(VK_F2))
        m_bDrawFrameRate = !m_bDrawFrameRate;

    if (m_pKeyboard->TriggerDown(VK_F3))
        m_bDrawHelpMessage = !m_bDrawHelpMessage;

    if (m_pKeyboard->TriggerDown(VK_BACK)) { // restart game
        m_reset = true;
    }

    if (m_pKeyboard->Down(VK_DELETE)) { // stop game
        exit(EXIT_SUCCESS);
    }

    if (m_pKeyboard->TriggerDown('U')) {
        m_Healths.Get(m_Player)->current -= 1;
    }

    // Toggles Main Menu
    if (m_pKeyboard->TriggerDown('P')) {
        m_DrawMainMenu = !m_DrawMainMenu;
        std::cout << "Menu Status: " << m_DrawMainMenu << std::endl;
    }

    if (m_pKeyboard->TriggerDown('M')) {
        m_pAudio->mute();
    }

    // Render mode toggle
    if (m_pKeyboard->TriggerDown('G')) {
        render_mode %= 3;
        render_mode += 1;
    }

    // Spawn NPC
    if (m_pKeyboard->TriggerDown('N')) {
        if (m_NPCsCache.Size() > 0) {
            PlaceNPC(m_pRenderer->m_pCamera->GetPos(), m_pRenderer->m_pCamera->GetViewVector());
        }
    }

    // Destroy last NPC
    if (m_pKeyboard->TriggerDown('Z')) {
        if (m_NPCsActive.Size() > 0) {
            StripNPC();
        }
    }

    if (m_pKeyboard->TriggerDown('H')) {
        btRigidBody* player_body = *m_RigidBodies.Get(m_Player);
        RBTeleportLaunch(player_body, Vec3(flycam_pos), Vec3(0));
        RBSetCcd(player_body, 0.0, 500.0);
    }

    // Mouse Click Testing
    m_leftClick.UpdateState();
    m_rightClick.UpdateState();

    PlayerInput();

    {
        static f32 intensity = 1.0f;
        bool toggled = false;

        if (m_pKeyboard->Down('J')) {
            toggled = true;
            intensity -= 0.02;
        }

        if (m_pKeyboard->Down('K')) {
            toggled = true;
            intensity += 0.02;
        }

        if (toggled) {
            btClamp<f32>(intensity, 0.0, 1.0);
            Vec4 color = Colors::LightYellow * intensity;

            for every(index, m_TestingLights.Size()) {
                Entity e = m_TestingLights.Entities()[index];
                m_pRenderer->lights.Get(e)->color = color * 200.0f;
            }
        }
    }

    if (m_pKeyboard->TriggerDown('O')) {
        ParticleInstanceDesc desc = {};
        desc.count = 500;

        desc.light_color = Vec3(100.0f, 20.0f, 20.0f);

        desc.model = ModelIndex::Cube;
        desc.texture = TextureIndex::White;

        desc.glow = Vec3(1.2f, 1.0f, 0.8f);
        desc.model_scale = Vec3(5.0f);

        desc.initial_pos = m_pRenderer->m_pCamera->GetPos() + m_pRenderer->m_pCamera->GetViewVector() * 500.0f;

        desc.initial_speed = 800.0f;
        desc.initial_dir = Vec3(0.0f, 1.0f, 0.0f);
        desc.dir_randomness = 0.1f;
        desc.speed_randomness = 0.5f;

        desc.initial_acc = Vec3(0.0f, -1000.0f, 0.0f);
        desc.acc_randomness = 0.0f;

        desc.min_alive_time = 0.5f;
        desc.max_alive_time = 1.5f;

        SpawnParticles(&desc);
    }
}

void CGame::EcsPreUpdate() {
    m_pAudio->SetListener(m_pRenderer->m_pCamera); // Update Audio
}

void CGame::EcsUpdate() {
    CustomPhysicsStep();

    // This handles NPCs and lighting.
    Ecs::ApplyEvery(m_NPCsActive, [=](Entity e) {
        NPC* npc = m_NPCs.Get(e);
        btRigidBody* body = *m_RigidBodies.Get(e);
        Vec3 origin = body->getWorldTransform().getOrigin();

        btCollisionShape* currentShape = body->getCollisionShape();
        btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

        m_pRenderer->lights.Get(e)->position = *(Vec4*)&origin;

        (*m_ModelInstances.Get(e)).world = MoveRotateScaleMatrix(
            npc->LastPosition,
            body->getWorldTransform().getRotation(),
            boxShape->getHalfExtentsWithMargin()
        );

        DirectNPC(e);
    });

    Ecs::ApplyEvery(m_ProjectilesActive, [=](Entity e) {
        btTransform trans;
    
        (*m_RigidBodies.Get(e))->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();
        
        (*m_ModelInstances.Get(e)).world = MoveScaleMatrix((*m_RigidBodies.Get(e))->getWorldTransform().getOrigin(), Vector3(25.0f));
    });

    Ecs::ApplyEvery(m_RaysActive, [=](Entity e) {
        RayProjectile currentRay = *m_Rays.Get(e);

        f32 distance, t;

        Vec3 pos1 = currentRay.Pos1;
        Vec3 pos2 = currentRay.Pos2;
        Vec3 origin = (pos1 + pos2) / 2.0f;
        Vec3 direction = pos2 - pos1; direction.Normalize();
        distance = DistanceBetweenVectors(currentRay.Pos1, currentRay.Pos2);
        Vec3 bounds = Vec3(5.0f, 5.0f, distance / 2.0f);
        Quat rotation;

        // we need a small light for each ray
        m_pRenderer->lights.Get(e)->position = *(Vec4*)& origin;

        // get rotation matrix
        XMMATRIX lookAt = XMMatrixLookAtRH(pos1, pos2, Vector3::Up);

        // note (ethan) : i dont know why, but this apparently took me 6 hours to achieve... i need to get some geometric algebra in my brain ASAP.
        rotation = XMQuaternionRotationMatrix(lookAt);
        rotation.x = -(rotation.x);
        rotation.y = -(rotation.y);
        rotation.z = -(rotation.z);

        (*m_ModelInstances.Get(e)).world = MoveRotateScaleMatrix(
            origin + (Vec3)((*m_RigidBodies.Get(m_Player))->getLinearVelocity() / 9.5f),
            rotation,
            bounds
        );
    });

    auto TimerLEZero = [&](Entity e) { return *m_Timers.Get(e) <= 0.0f; };

    Ecs::RemoveConditionally(m_ProjectilesActive, TimerLEZero, [=](Entity e) { StripProjectile(e);});
    Ecs::RemoveConditionally(m_RaysActive, TimerLEZero, [=](Entity e) { StripRay(e); });
    Ecs::RemoveConditionally(m_Particles, TimerLEZero, [&](Entity e) { StripParticle(e); });
    Ecs::RemoveConditionally(m_ParticleInstancesActive, [&](Entity e) { return m_ParticleInstances.Get(e)->count <= 0; }, [&](Entity e) { StripParticleInstance(e); });

    f32 dt = m_pTimer->GetFrameTime();
    for every(index, m_Particles.Size()) {
        Particle* p = &m_Particles.Components()[index];

        p->vel += p->acc * dt;
        p->pos += p->vel * dt;
    }

    Ecs::ApplyEvery(m_ParticleInstancesActive, [&](Entity e) {
        ParticleInstance* instance = m_ParticleInstances.Get(e);
        Light* light = m_pRenderer->lights.Get(instance->light);
        f32* timer = m_Timers.Get(e);

        light->color = (*timer /instance->highest_timer) * instance->initial_light_color;
    });

    // This handles the timers tables.
    for every(index, m_Timers.Size()) {
        m_Timers.Components()[index] -= dt;
    }

    for every(index, m_Mana.Size()) {
        Mana* mana = &m_Mana.Components()[index];

        mana->timer -= dt;
        btClamp<f32>(mana->timer, 0.0, FLT_MAX);
        mana->value = (i32)(((mana->recharge * (f32)mana->max) - mana->timer) / mana->recharge);
    }
}

/// Ask the object manager to draw the game objects. The renderer is notified
/// of the start and end of the frame so that it can let Direct3D do its
/// pipelining jiggery-pokery.
void CGame::RenderFrame() {
    //NOTE(sean): Everything out here will technically be known
    // *before* we start rendering, so I dont want to give the impression
    // that we create it *while* rendering, although you certainly can...
    m_pRenderer->BeginFrame();

    if (render_mode & 0b01) {
        m_pRenderer->BeginDrawing();

        Ecs::ApplyEvery(m_ModelsActive, [&](Entity e) {
            m_pRenderer->DrawModelInstance(m_ModelInstances.Get(e));
        });

        for every(index, m_Particles.Size()) {
            Particle* particle = &m_Particles.Components()[index];
            Entity e = m_Particles.Entities()[index];
            Entity instance_e = *m_EntityMapping.Get(e);
            ParticleInstance* pi = m_ParticleInstances.Get(instance_e);
            ModelInstance* mi = &pi->model_instance;

            mi->world = MoveScaleMatrix(particle->pos, pi->model_scale);
            m_pRenderer->DrawModelInstance(mi);
        }

        m_pRenderer->EndDrawing();
    } else {
        m_pRenderer->BeginDrawing();
        m_pRenderer->EndDrawing();
    }

    if (render_mode & 0b10) {
        m_pRenderer->BeginDebugDrawing();

        DrawDebugModelsOnRB();

        m_pRenderer->EndDebugDrawing();
    }

    {
        m_pRenderer->BeginUIDrawing();

        if (m_bDrawHelpMessage) {
            m_pRenderer->DrawCenteredText(
                L"Press 'Escape' to toggle mouse cursor.\n"
                L"Press 'M' to toggle audio.\n"
                L"Press 'P' to toggle Main Menu.\n"
                L"Toggle this message with 'F3'."
                , Colors::White
            );
        }

        if (m_bDrawFrameRate) {
            char buffer[64];
            sprintf(buffer, "%.2f", m_frameRate);
            m_pRenderer->DrawScreenText(buffer, Vector2(m_nWinWidth - 200.0, 50.0), Colors::White);
        }
        /*
        SpriteInstance sprite_instance;
        sprite_instance.position = Vec2(100.0f, 100.0f);
        sprite_instance.roll = 0.0f;
        sprite_instance.scale = Vec2(100.0f, 100.0f);
        sprite_instance.rgba = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        sprite_instance.texture_index = TextureIndex::White;

        m_pRenderer->DrawSpriteInstance(&sprite_instance);

        sprite_instance.scale = Vec2(10.0f, 10.0f);
        sprite_instance.rgba = Vec4(1.0f, 1.0f, 0.0f, 1.0f);

        m_pRenderer->DrawSpriteInstance(&sprite_instance);
        */
        m_pRenderer->EndUIDrawing();
    }

    // MAIN MENU
    {
        m_pRenderer->BeginUIDrawing();

        if (m_DrawMainMenu == true) {

            Panel panel;
            PanelText panel_text;


            //m_pRenderer->DrawCenteredText(L"Main Menu.\n", Colors::White);

            SpriteInstance sprite_instance;
/*
            // Background
            sprite_instance.position = Vec2(100.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(800.0f, 800.0f);
            sprite_instance.rgba = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);
*/
            // Resume Button -- Yellow
            sprite_instance.position = Vec2(100.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(100.0f, 100.0f);
            sprite_instance.rgba = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);

            if (panel.Pressed == true) {
                m_pRenderer->DrawCenteredText(

                    L"Press 'P' to toggle Main Menu.\n"

                    , Colors::White
                );
            }

            // Settings Button -- Red
            sprite_instance.position = Vec2(450.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(100.0f, 100.0f);
            sprite_instance.rgba = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);
            
            panel_text.position = Vector2(100.0f, 100.0f);


            // Exit Button -- Pink
            sprite_instance.position = Vec2(800.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(100.0f, 100.0f);
            sprite_instance.rgba = Vec4(1.0f, 0.0f, 1.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);


        }





        m_pRenderer->EndUIDrawing();
    }

    m_pRenderer->EndFrame();
}

/// This function will be called regularly to process and render a frame
/// of animation, which involves the following. Handle keyboard input.
/// Notify the  audio player at the start of each frame so that it can prevent
/// multiple copies of a sound from starting on the same frame.  
/// Move the game objects. Render a frame of animation.

void CGame::Update() {
    m_pTimer->Tick([&]() { //all time-dependent function calls should go here
        m_pDynamicsWorld->stepSimulation(m_pTimer->GetFrameTime(), 10); // Step Physics
        m_frameRate = m_pTimer->GetFPS();

        EcsPreUpdate();
        UpdatePlayer();
        EcsUpdate();
    });
}

void CGame::ProcessFrame() {
    InputHandler(); // handle keyboard input
    InputMenu(); 

    m_pAudio->BeginFrame(); // notify audio player that frame has begun

    Update(); // update internal data

    RenderFrame(); // render a frame of animation

    // reset game on reset
    if (m_reset) { ResetGame(); }
}
