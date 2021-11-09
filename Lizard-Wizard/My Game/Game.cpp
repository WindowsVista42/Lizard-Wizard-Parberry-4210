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

// HEY LOL ITS ME IM DOING A TEST AHAHAH *BITES LIP*

/// Delete the object manager. The renderer needs to be deleted before this
/// destructor runs so it will be done elsewhere.


CGame::~CGame() {}
/// Create the renderer and the object manager, load images and sounds, and
/// begin the game.
void CGame::Initialize() {
    m_pRenderer = new Renderer();
    m_pRenderer->Initialize();

    LoadImages(); //load images from xml file list
    LoadModels(); //load models from xml file list
    LoadSounds(); //load the sounds for this game

    // Create Raycast Vector. Note(Ethan) : Remove this and prepare a raycast-texture pipeline.
    m_currentRayProjectiles = std::vector<RayProjectile>();

    // Initialize Managers
    m_Player = Entity();
    InitializePhysics();
    InitializeGeneration();
    InitializeProjectiles();
    InitializeNPCs();
    InitializePlayer();

    GenerateRooms(Vec3(0,0,0), 50);

    // Lets bind this action to to the user's mouse. For key values : https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    m_leftClick = CustomBind::New(VK_LBUTTON);
    m_rightClick = CustomBind::New(VK_RBUTTON);

    CreateTestingEnvironment();

    BeginGame();
}

/// Load the specific images needed for this game. This is where `eSprite`
/// values from `GameDefines.h` get tied to the names of sprite tags in
/// `gamesettings.xml`. Those sprite tags contain the name of the corresponding
/// image file. If the image tag or the image file are missing, then the game
/// should abort from deeper in the Engine code leaving you with an error
/// message in a dialog box.

void CGame::LoadModels() {
    //m_pRenderer->LoadDebugModel("cube", Colors::Peru, DebugModelIndex::);
    m_pRenderer->LoadModel("cube", ModelIndex::Cube);
    m_pRenderer->LoadModel("suzanne", ModelIndex::Suzanne);
    m_pRenderer->LoadModel("obelisk", ModelIndex::Obelisk);
    m_pRenderer->LoadModel("sentry", ModelIndex::Sentry);
    m_pRenderer->LoadModel("staff", ModelIndex::Staff);
    m_pRenderer->LoadModel("quad", ModelIndex::Quad);
}

void CGame::LoadImages(){
    m_pRenderer->BeginResourceUpload();

    m_pRenderer->LoadTextureI("sample", TextureIndex::Other);
    m_pRenderer->LoadTextureI("white", TextureIndex::White);

    m_pRenderer->EndResourceUpload();
}

/// Initialize the audio player and load game sounds.

void CGame::LoadSounds(){
  m_pAudio->Initialize(SoundIndex::Size);
  m_pAudio->Load(SoundIndex::Grunt, "grunt");
  m_pAudio->Load(SoundIndex::Clang, "clang");
  m_pAudio->Load(SoundIndex::Impact, "impact");
}

/// Release all of the DirectX12 objects by deleting the renderer.

void CGame::Release() {
    delete m_pRenderer;
}

/// Ask the object manager to create the game objects. There's only one in this
/// game, the rotating wheel o' text centered at the center of the window.

void CGame::CreateObjects(){}

/// Call this function to start a new game. This should be re-entrant so that
/// you can restart a new game without having to shut down and restart the
/// program. All we need to do is delete any old objects out of the object
/// manager and create some new ones.

void CGame::BeginGame(){  
  CreateObjects(); //create new objects 
}


/// Poll the keyboard state and respond to the key presses that happened since
/// the last frame
void CGame::InputHandler() {
    if (!(m_pRenderer->GetHwnd() == GetFocus())) { return; }

    m_pKeyboard->GetState();

    if (m_pKeyboard->TriggerDown(VK_F1))
        ShellExecute(0, 0, "https://larc.unt.edu/code/blank/", 0, 0, SW_SHOW);

    if (m_pKeyboard->TriggerDown(VK_F2))
        m_bDrawFrameRate = !m_bDrawFrameRate;

    if (m_pKeyboard->TriggerDown(VK_F3))
        m_bDrawHelpMessage = !m_bDrawHelpMessage;

    if (m_pKeyboard->TriggerDown(VK_BACK)) {//restart game
        BeginGame(); //restart game
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
        desc.count = 10;
        desc.initial_speed = 100.0f;
        desc.light_color = Vec3(50.0f, 20.0f, 40.0f);
        desc.model = ModelIndex::Cube;
        desc.texture = TextureIndex::White;
        desc.model_scale = Vec3(10.0f);
        desc.origin = m_pRenderer->m_pCamera->GetPos() + m_pRenderer->m_pCamera->GetViewVector() * 500.0f;
        desc.randomness = 1.0f;
        desc.glow = Vec3(1.2f, 1.0f, 0.8f);

        m_ParticleInstances.Add(m_pRenderer->CreateParticleInstance(&desc));
    }
}

/// Draw the current frame rate to a hard-coded position in the window.
/// The frame rate will be drawn in a hard-coded position using the font
/// specified in gamesettings.xml.

void CGame::DrawFrameRateText(){
  const std::string s = std::to_string(m_pTimer->GetFPS()) + " fps"; //frame rate
  const Vector2 pos(m_nWinWidth - 128.0f, 30.0f); //hard-coded position
  //m_pRenderer->DrawScreenText(s.c_str(), pos); //draw to screen
}

void CGame::EcsPreUpdate() {
    m_pAudio->SetListener(m_pRenderer->m_pCamera); // Update Audio

    // This handles the timers tables.
    for every(index, m_Timers.Size()) {
        m_Timers.Components()[index] -= m_pTimer->GetFrameTime();
    }

    for every(index, m_Mana.Size()) {
        Mana* mana = &m_Mana.Components()[index];

        mana->timer -= m_pTimer->GetFrameTime();
        btClamp<f32>(mana->timer, 0.0, FLT_MAX);
        mana->value = (i32)(((mana->recharge * (f32)mana->max) - mana->timer) / mana->recharge);
    }
}


void CGame::EcsUpdate() {
    // This handles NPCs and lighting.
    for every(index, m_NPCsActive.Size()) {
        Entity e = m_NPCsActive.Entities()[index];
        btTransform trans;
        btCollisionShape* currentShape = (*m_RigidBodies.Get(e))->getCollisionShape();
        btBoxShape* boxShape = reinterpret_cast<btBoxShape*>(currentShape);

        (*m_RigidBodies.Get(e))->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();

        (*m_ModelInstances.Get(e)).world = MoveRotateScaleMatrix(
            (*m_RigidBodies.Get(e))->getWorldTransform().getOrigin(), 
            *(Quat*)&(*m_RigidBodies.Get(e))->getWorldTransform().getRotation(), 
            boxShape->getHalfExtentsWithMargin()
        );

        DirectNPC(e);
    }

    Ecs::ApplyEvery(m_ProjectilesActive, [=](Entity e) {
        btTransform trans;
    
        (*m_RigidBodies.Get(e))->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();
        
        (*m_ModelInstances.Get(e)).world = MoveScaleMatrix((*m_RigidBodies.Get(e))->getWorldTransform().getOrigin(), Vector3(25.0f));
    });

    //Sean: we have this disabled because its lashing out and crashing
    CustomPhysicsStep();

    Ecs::RemoveConditionally(m_ProjectilesActive, [=](Entity e) { return *m_Timers.Get(e) <= 0.0; }, [=](Entity e) { StripProjectile(e); });

    m_pRenderer->UpdateParticles();
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

        for every(index, m_ParticleInstances.Size()) {
            ParticleInstance* p = &m_ParticleInstances.Components()[index];
            m_pRenderer->DrawParticleInstance(p);
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
                L"Toggle this message with 'F3'."
                , Colors::White
            );
        }

        if (m_bDrawFrameRate) {
            char buffer[64];
            sprintf(buffer, "%.2f", m_frameRate);
            m_pRenderer->DrawScreenText(buffer, Vector2(m_nWinWidth - 200.0, 50.0), Colors::White);
        }
        m_pRenderer->EndUIDrawing();
    }

    m_pRenderer->tint_color = Vec3(1.0f, 1.0f, 1.0f);
    m_pRenderer->blur_amount = 0.0f;
    m_pRenderer->saturation_amount = 1.0f;

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

void CGame::ProcessFrame(){
    InputHandler(); // handle keyboard input

    m_pAudio->BeginFrame(); // notify audio player that frame has begun

    Update(); // update internal data

    RenderFrame(); // render a frame of animation
}

Mana CGame::NewMana(i32 max, f32 recharge) {
    Mana mana;
    mana.value = max;
    mana.max = max;
    mana.recharge = recharge;
    mana.timer = 0.0f;

    return mana;
}
