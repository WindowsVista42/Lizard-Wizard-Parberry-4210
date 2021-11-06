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
}

void CGame::LoadImages(){
    m_pRenderer->BeginResourceUpload();

    //m_pRenderer->LoadTextureI("test", TextureIndex::Other);

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
    m_pKeyboard->GetState();

    if (m_pKeyboard->TriggerDown(VK_F1))
        ShellExecute(0, 0, "https://larc.unt.edu/code/blank/", 0, 0, SW_SHOW);

    if (m_pKeyboard->TriggerDown(VK_F2))
        m_bDrawFrameRate = !m_bDrawFrameRate;

    if (m_pKeyboard->TriggerDown(VK_F3))
        m_bDrawHelpMessage = !m_bDrawHelpMessage;


    if (m_pKeyboard->TriggerUp(VK_SPACE)) {//play sound
       //m_pAudio->play(eSound::Grunt);
    }

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
}

/// Draw the current frame rate to a hard-coded position in the window.
/// The frame rate will be drawn in a hard-coded position using the font
/// specified in gamesettings.xml.

void CGame::DrawFrameRateText(){
  const std::string s = std::to_string(m_pTimer->GetFPS()) + " fps"; //frame rate
  const Vector2 pos(m_nWinWidth - 128.0f, 30.0f); //hard-coded position
  //m_pRenderer->DrawScreenText(s.c_str(), pos); //draw to screen
}

void CGame::EcsUpdate() {
    m_pAudio->SetListener(m_pRenderer->m_pCamera); // Update Audio

    // This handles the timers tables.
    for every(index, m_Timers.Size()) {
        m_Timers.Components()[index] -= m_pTimer->GetFrameTime();
    }

    static std::vector<Entity> toRemove;
    toRemove.clear();

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

    // This handles projectiles and lighting.
    for every(index, m_ProjectilesActive.Size()) {
        Entity e = m_ProjectilesActive.Entities()[index];
        btTransform trans;

        (*m_RigidBodies.Get(e))->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();
        
        (*m_ModelInstances.Get(e)).world = MoveScaleMatrix((*m_RigidBodies.Get(e))->getWorldTransform().getOrigin(), Vector3(25.0f));

        if (*m_Timers.Get(e) < 0.0) {
            toRemove.push_back(e);
        }
    }

    CustomPhysicsStep();

    // Strip Projectiles.
    for every(index, toRemove.size()) {
        StripProjectile(toRemove[index]);
    }

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
}

/// Ask the object manager to draw the game objects. The renderer is notified
/// of the start and end of the frame so that it can let Direct3D do its
/// pipelining jiggery-pokery.
void CGame::RenderFrame() {
    //NOTE(sean): Everything out here will technically be known
    // *before* we start rendering, so I dont want to give the impression
    // that we create it *while* rendering, although you certainly can...
    m_pRenderer->m_pCamera->SetYaw(player_yaw);
    m_pRenderer->m_pCamera->SetPitch(player_pitch);

    static f32 time = 0.0;
    time += m_pTimer->GetFrameTime();

    m_pRenderer->BeginFrame();

    if (render_mode & 0b01) {
        m_pRenderer->BeginDrawing();

        for every(index, m_ModelsActive.Size()) {
            Entity e = m_ModelsActive.Entities()[index];
            m_pRenderer->DrawModelInstance(m_ModelInstances.Get(e));
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
