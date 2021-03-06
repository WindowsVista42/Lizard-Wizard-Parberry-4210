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

static bool god_mode_enabled = false;
static bool god_mode_just_enabled = false;
static bool muted = false;
static f32 restart_timer_on_win = 10.0f;

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
    m_pRenderer->LoadModel("crystal", ModelIndex::Crystal);
    m_pRenderer->LoadModel("boss", ModelIndex::Boss);
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
    m_pAudio->Load(SoundIndex::EnemyCast1, "EnemyCast1");
    m_pAudio->Load(SoundIndex::EnemyCast2, "EnemyCast2");


    // Impact Noises
    m_pAudio->Load(SoundIndex::FireImpact1, "FireImpact1");
    m_pAudio->Load(SoundIndex::IceImpact1, "IceImpact1");
    m_pAudio->Load(SoundIndex::PlayerImpact1, "PlayerImpact1");
    m_pAudio->Load(SoundIndex::EnemyImpactMetal1, "EnemyImpactMetal1");
    m_pAudio->Load(SoundIndex::EnemyImpactMetal2, "EnemyImpactMetal2");
    m_pAudio->Load(SoundIndex::EnemyImpactCrystal1, "EnemyImpactCrystal1");

    // Player Noises
    m_pAudio->Load(SoundIndex::Dash1, "Dash1");
    m_pAudio->Load(SoundIndex::Dash2, "Dash2");
    m_pAudio->Load(SoundIndex::Dash3, "Dash3");
    m_pAudio->Load(SoundIndex::Dash4, "Dash4");
    m_pAudio->Load(SoundIndex::PlayerWalk1, "PlayerWalk1");
    m_pAudio->Load(SoundIndex::PlayerWalk2, "PlayerWalk2");
    m_pAudio->Load(SoundIndex::PlayerLand1, "PlayerLand1");

    // Death Noises
    m_pAudio->Load(SoundIndex::CrystalDeath, "CrystalDeath");
    m_pAudio->Load(SoundIndex::ObeliskDeath, "ObeliskDeath");
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

    m_NPCStatsMap.clear();

    m_reset = false;

    flycam_enabled = false;
    god_mode_enabled = false;
    m_PlayerWon = false;
    restart_timer_on_win = 10.0f;

    for every(z, Z_ROOMS) {
        for every(x, X_ROOMS) {
            m_PlayerVisibilityMap[x][z] = false;
        }
    }

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

    // Shift+Delete
    if (m_pKeyboard->Down(VK_DELETE)) { // stop game
        exit(EXIT_SUCCESS);
    }

    // Toggles Main Menu
    if (m_pKeyboard->TriggerDown(VK_ESCAPE)) {
        m_DrawMainMenu = !m_DrawMainMenu;

       // m_MouseToggled = !m_MouseToggled;
       // m_MouseJustToggled = true;

        //std::cout << "Menu Status: " << m_DrawMainMenu << std::endl;
    }
  //  else {
   //     m_MouseJustToggled = false;   
   // }

    if (m_pKeyboard->TriggerDown('M')) {
        m_pAudio->mute();
        muted = !muted;
    }

    god_mode_just_enabled = false;
    if (m_pKeyboard->TriggerDown('G')) {
        god_mode_enabled = !god_mode_enabled;
        god_mode_just_enabled = true;
    }

    // Render mode toggle
    //if (m_pKeyboard->TriggerDown('G')) {
    //    render_mode %= 3;
    //    render_mode += 1;
    //}

    // Mouse Click Testing
    m_leftClick.UpdateState();
    m_rightClick.UpdateState();

    PlayerInput();

    MenuInput();
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
        Vec3 origin = npc->LastPosition;

        btCollisionShape* currentShape = body->getCollisionShape();
        btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);
        Vec3 scale = boxShape->getHalfExtentsWithMargin();

        m_pRenderer->lights.Get(e)->position = *(Vec4*)&origin;
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&npc->LightColor;

        (*m_ModelInstances.Get(e)).world = MoveRotateScaleMatrix(
            Vec3(origin.x, origin.y - 300.0f, origin.z),
            body->getWorldTransform().getRotation(),
            Vec3(scale.x / 2.75f, scale.y / 4.0f, scale.z / 1.5f)
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
        // quaternion conjugate  ?? ? ? ? ?

        (*m_ModelInstances.Get(e)).world = MoveRotateScaleMatrix(
            origin,
            rotation,
            bounds
        );
    });

    auto TimerLEZero = [&](Entity e) { return *m_Timers.Get(e) <= 0.0f; };

    Ecs::RemoveConditionally(m_ProjectilesActive, TimerLEZero, [=](Entity e) { StripProjectile(e);});
    Ecs::RemoveConditionally(m_RaysActive, TimerLEZero, [=](Entity e) { StripRay(e); });
    Ecs::RemoveConditionally(m_Particles, TimerLEZero, [&](Entity e) { StripParticle(e); });
    Ecs::RemoveConditionally(m_RaycheckActive, TimerLEZero, [=](Entity e) { StripRaycheck(e); });
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

    if (m_Healths.Get(m_Player)->current <= 0) {
        m_reset = true;
    }

    // Manage NPC Healths, strip if lessthen/equal to 0.
    Ecs::ApplyEvery(m_NPCsActive, [&](Entity e) {
        if (m_Healths.Get(e)->current <= 0) {
            if (m_NPCsActive.Size() > 0) {
                NPC* currNPC = m_NPCs.Get(e);
                SoundIndex::e DeathSound = currNPC->DeathSound;
                Vec3 origin = (*m_RigidBodies.Get(e))->getWorldTransform().getOrigin();
                m_pAudio->play(DeathSound, origin, 2.0f, 0.5);

                // Create Particle for Impact (Very ugly, will clean up later.)
                ParticleInstanceDesc particle;
                particle.count = 200;
                particle.initial_pos = currNPC->LastPosition - Vec3(0.0f, 300.0f, 0.0f);
                particle.initial_dir = Vec3::Up;
                particle.light_color = Vec3(0.7f, 0.5, 0.1f) * 30.0f;
                particle.model = ModelIndex::Cube;
                particle.texture = TextureIndex::White;
                particle.glow = *(Vec3*)&currNPC->LightColor / 50.0f;
                particle.model_scale = Vec3(18.0f);
                particle.initial_speed = 1600.0f;
                particle.dir_randomness = 0.8f;
                particle.speed_randomness = 0.95f;
                particle.initial_acc = Vec3(0.0f, -1000.0f, 0.0f);
                particle.acc_randomness = 0.7f;
                particle.min_alive_time = 0.5f;
                particle.max_alive_time = 1.1f;

                SpawnParticles(&particle);

                ForceStripNPC(e);
            }

            if (m_Healths.Get(m_Player)->current > 0 && m_Healths.Get(m_Player)->current < 4) {
                m_Healths.Get(m_Player)->current += 1;
                m_PlayerRejuvinationTimer = 0.5f;
            }
        }
    });

    // reset player pos if something breaks
    if ((*m_RigidBodies.Get(m_Player))->getWorldTransform().getOrigin().y() < -2000.0f) {
        Vec3 tpos = IndexToWorld(1, 1);
        tpos.y = -500.0f;
        RBTeleportLaunch(*m_RigidBodies.Get(m_Player), tpos, Vec3(0));
    }

    if (god_mode_just_enabled) {
        if (god_mode_enabled) {
            m_Healths.Get(m_Player)->current = 20000000;
            m_Healths.Get(m_Player)->max = 20000000;
        } else {
            m_Healths.Get(m_Player)->current = 4;
            m_Healths.Get(m_Player)->max = 4;
        }
    }

    if (god_mode_enabled) {
        m_Mana.Get(m_Player)->value = 4;
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

        if(flycam_enabled) {
            //ModelInstance mi;
            //mi.model = ModelIndex::Cube;
            //mi.texture = TextureIndex::White;
            //mi.glow = Vec3(-1.0f);
            //mi.world = MoveScaleMatrix(Vec3(0.0f), Vec3(1000.0f));
            //m_pRenderer->DrawModelInstance(&mi);
            ModelInstance mi;
            mi.model = ModelIndex::Cube;
            mi.texture = TextureIndex::White;
            mi.glow = Vec3(-1.0f);
            for every(z, Z_ROOMS) {
                for every(x, X_ROOMS) {
                    if(!(m_GameMap[x][z] && m_PlayerVisibilityMap[x][z])){
                        Vec3 pos = IndexToWorld(x, z);
                        pos.y = 2100.0f;
                        mi.world = MoveScaleMatrix(pos, Vec3(1000.0f));
                        m_pRenderer->DrawModelInstance(&mi);
                    }
                }
            }

            mi.glow = Vec3(10.0f, -1.0f, -1.0f);
            Vec3 player_pos = (*m_RigidBodies.Get(m_Player))->getWorldTransform().getOrigin();
            player_pos.y = 4000.0f;
            mi.world = MoveScaleMatrix(player_pos, Vec3(200.0f));
            m_pRenderer->DrawModelInstance(&mi);

            Ecs::ApplyEvery(m_NPCsActive, [&](Entity e) {
                mi.glow = *(Vec3*)&(m_NPCs.Get(e)->LightColor / 40.0f);

                Vec3 npc_pos = (*m_RigidBodies.Get(e))->getWorldTransform().getOrigin();
                npc_pos.y = 4000.0f;
                mi.world = MoveScaleMatrix(npc_pos, Vec3(200.0f));
                m_pRenderer->DrawModelInstance(&mi);
            });
        } else {
            LBaseCamera* cam = m_pRenderer->m_pCamera;
    
            f32 offset = 0.0f;
            f32 yoffset = 0.0f;
            ModelInstance mi;
            mi.model = ModelIndex::Cube;
            mi.texture = TextureIndex::White;
            Ecs::ApplyEvery(m_NPCsActive, [&](Entity e) {
                NPC* npc = m_NPCs.Get(e);
                mi.glow = *(Vec3*)&(npc->LightColor / 200.0f);
    
                Vec3 pos = RotatePointAroundOrigin(
                    staff_pos, 
                    Vec3(offset - 15.0f, 10.0f + yoffset, 10.0f), 
                    staff_rot
                );
        
                Quat rot = Quat::Identity;
        
                Vec3 scl = Vec3(0.75f);
        
                mi.world = MoveRotateScaleMatrix(pos, rot, scl);

                m_pRenderer->DrawModelInstance(&mi);
    
                offset += 2.5f;
                if (offset > 15.0f) {
                    offset = 0.0f;
                    yoffset += 2.5f;
                }
            });

            Vec3 p = RotatePointAroundOrigin(
                cam->GetPos(), 
                Vec3(0.0f, 0.0f, 80.0f), 
                Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch(), cam->GetRoll())
            );
        
            Quat r = Quat::CreateFromYawPitchRoll(cam->GetYaw(), cam->GetPitch(), cam->GetRoll());
        
            Vec3 s = Vec3(0.25f);

            mi.world = MoveRotateScaleMatrix(p, r, s);
            mi.glow = Vec3(1.0f);

            m_pRenderer->DrawModelInstance(&mi);
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
            m_pRenderer->DrawScreenText(
                L"Press 'WASD' and 'Space' and 'Left Shift' for movement.\n"
                L"Press 'Left Click' for primary attack.\n"
                L"Press '1' or '2' or '3' for spell type.\n"
                L"Press 'F' to toggle the map.\n"
                L"Press 'G' to toggle god mode.\n"
                L"Press 'M' to toggle audio.\n"
                L"Press 'Escape' to toggle escape main menu.\n"
                L"Press 'Backspace' to quickly restart a level.\n"
                L"Press 'Delete' to quickly close the game.\n"
                L"Press 'F2' to toggle the fps display.\n"
                L"Press '0' to take a screenshot.\n"
                L"Toggle this message with 'F3'.",
                Vec2(50.0f, 50.0f),
                Colors::White
            );
        }

        if (m_bDrawFrameRate) {
            char buffer[64];
            sprintf(buffer, "%.2f", m_frameRate);
            m_pRenderer->DrawScreenText(buffer, Vector2(m_nWinWidth - 200.0, 100.0), Colors::White);
        }

        if (muted) {
            m_pRenderer->DrawScreenText("Muted\n", Vector2(m_nWinWidth - 200.0, 150.0), Colors::White);
        }

        if (god_mode_enabled) {
            m_pRenderer->DrawScreenText(L"God Mode\n", Vec2(m_nWinWidth - 200.0f, 200.0f), Colors::White);
        }

        if (!m_NPCsActive.Contains(m_BossEntity)) {
            char buffer[64];
            sprintf(buffer, "You Won!\nRestarting game in %.f seconds.\n", restart_timer_on_win);
            m_pRenderer->DrawCenteredText(buffer, Colors::White);
            restart_timer_on_win -= m_pTimer->GetFrameTime();
            if (restart_timer_on_win <= 0.0f) {
                m_reset = true;
            }
        }

        m_pRenderer->EndUIDrawing();
    }

    RenderMenu();

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
   // MenuInput(); 

    m_pAudio->BeginFrame(); // notify audio player that frame has begun

    Update(); // update internal data

    RenderFrame(); // render a frame of animation

    // reset game on reset
    if (m_reset) { ResetGame(); }
}
