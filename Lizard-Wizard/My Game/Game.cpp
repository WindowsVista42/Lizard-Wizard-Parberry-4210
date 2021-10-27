/// \file Game.cpp
/// \brief Code for the game class CGame.

#include "Game.h"
#include "Math.h"
#include "Renderer.h"
#include "SpriteRenderer.h"
#include "ComponentIncludes.h"
#include "PhysicsManager.h"
#include "ProjectileManager.h"
#include "Helpers.h"
#include "shellapi.h"
#include <vector>
#include <iostream>

// HEY LOL ITS ME IM DOING A TEST AHAHAH *BITES LIP*

/// Delete the object manager. The renderer needs to be deleted before this
/// destructor runs so it will be done elsewhere.

//TODO(sean): move these out of globals
static f32 yaw = 0.0f;
static f32 pitch = 0.0f;
const f32 sensitivity = 0.0333f;

static Vector3 player_pos = { 0.0f, 1500.0f, -2500.0f };
const f32 move_speed = 50.0f;

static ModelInstance model_instance;

static bool flycam_enabled = false;
static bool debug_rendering_enabled = true;
static u32 render_mode = 3;

CGame::~CGame() {}

/// Create the renderer and the object manager, load images and sounds, and
/// begin the game.
void CGame::Initialize() {
    m_pRenderer = new Renderer();
    m_pRenderer->Initialize();

    LoadImages(); //load images from xml file list
    LoadModels(); //load models from xml file list

    // Create Managers
    m_pPhysicsManager = new PhysicsManager();
    m_pGenerationManager = new GenerationManager();
    m_pProjectileManager = new ProjectileManager();
    m_pNPCManager = new NPCManager();
    LoadSounds(); //load the sounds for this game
    // Create Raycast Vector. Note(Ethan) : Remove this and prepare a raycast-texture pipeline.
    m_currentRayProjectiles = std::vector<RayProjectile>();

    // Initialize Managers
    player = Entity();
    m_pPhysicsManager->InitializePhysics(
        &m_pDynamicsWorld,
        &m_pCollisionShapes,
        &m_RigidBodies,
        &player
    );

    m_pGenerationManager->InitializeGeneration(m_pPhysicsManager);

    m_pProjectileManager->InitializeProjectiles(
        m_pCollisionShapes,
        &m_currentRayProjectiles,
        m_pDynamicsWorld,
        m_pPhysicsManager,
        &m_RigidBodies,
        &m_pRenderer->lights,
        &m_Timers,
        &m_ProjectilesCache,
        &m_ProjectilesActive
    );

    m_pNPCManager->InitializeNPCs(
        m_pPhysicsManager,
        m_pProjectileManager,
        &m_NPCs,
        &m_pRenderer->lights,
        &m_Timers,
        &m_NPCsCache,
        &m_NPCsActive
    );

    // Test NPC
    {
        Entity e = m_NPCs.Entities()[0];
        m_pNPCManager->PlaceNPC(e, Vec3(6000.0f, 500.0f,0));
    }

    // Room Collider
    {
        /*
        Vec3 roomPos = Vec3(0, 0, 0);
        for (int i = 0; i < 4; ++i)
        {
            if (i == 0)
            {
                m_pGenerationManager->CreateNormalRoom(roomPos);
            }
            
            if(i>0)
            {
                m_pGenerationManager->CreateHallway(roomPos);
                roomPos.x = roomPos.x + 6000.0f;
            }
        }

        roomPos = Vec3(-10000.0f, 0, -10000.0f);
        m_pGenerationManager->CreateBossRoom(roomPos);

        roomPos = Vec3(10000.0f, 0, 10000.0f);
        m_pGenerationManager->CreateSpawnRoom(roomPos);
        */

        m_pGenerationManager->GenerateRooms(Vec3(0,0,0), 50);
    }

    // Lets bind this action to to the user's mouse. For key values : https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    m_leftClick = CustomBind::New(VK_LBUTTON);
    m_rightClick = CustomBind::New(VK_RBUTTON);

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
}

void CGame::LoadImages(){}

/// Initialize the audio player and load game sounds.

void CGame::LoadSounds(){
  m_pAudio->Initialize(SoundIndex::Size);
  m_pAudio->Load(SoundIndex::Grunt, "grunt");
  m_pAudio->Load(SoundIndex::Clang, "clang");
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
    m_pKeyboard->GetState(); //get current keyboard state 

    if (m_pKeyboard->TriggerDown(VK_F1)) //help
        ShellExecute(0, 0, "https://larc.unt.edu/code/blank/", 0, 0, SW_SHOW);

    if (m_pKeyboard->TriggerDown(VK_F2)) //toggle frame rate 
        m_bDrawFrameRate = !m_bDrawFrameRate;

    if (m_pKeyboard->TriggerDown(VK_SPACE) && !flycam_enabled) {
        btCollisionObject* pObj = m_pDynamicsWorld->getCollisionObjectArray()[0];
        pObj->activate(true);
        btRigidBody* pBody = btRigidBody::upcast(pObj);
        btVector3 jumpVector = {0, 5000, 0};
        pBody->applyCentralImpulse(jumpVector);
    }

    if (m_pKeyboard->TriggerUp(VK_SPACE)) //play sound
        //m_pAudio->play(eSound::Grunt);

    if (m_pKeyboard->TriggerDown(VK_BACK)) //restart game
        BeginGame(); //restart game


      //TODO(sean): make this framerate dependant
    if (m_pKeyboard->Down(VK_RIGHT))
        yaw += sensitivity;
    if (m_pKeyboard->Down(VK_LEFT))
        yaw -= sensitivity;
    if (m_pKeyboard->Down(VK_UP))
        pitch -= sensitivity;
    if (m_pKeyboard->Down(VK_DOWN))
        pitch += sensitivity;

    // Flycam toggle
    if (m_pKeyboard->TriggerDown('F')) {
        flycam_enabled = !flycam_enabled;
    }

    // Render mode toggle
    if (m_pKeyboard->TriggerDown('G')) {
        render_mode %= 3;
        render_mode += 1;
    }

    // Print screenshot button thing that will do stuff
    if (m_pKeyboard->TriggerDown('P')) {
        m_pRenderer->m_screenShot = true;
    }

    {
        Vector3 delta_movement = Vector3::Zero;
        Vector3 lookdir = m_pRenderer->m_pCamera->GetViewVector();
        lookdir = { lookdir.x, 0.0f, lookdir.z };
        Vector3 normal = { lookdir.z, 0.0f, -lookdir.x };

        if (m_pKeyboard->Down('D')) {
            delta_movement += normal;
        }

        if (m_pKeyboard->Down('A')) {
            delta_movement -= normal;
        }

        if (m_pKeyboard->Down('W')) {
            delta_movement += lookdir;
        }

        if (m_pKeyboard->Down('S')) {
            delta_movement -= lookdir;
        }

        if (flycam_enabled) {
            if (m_pKeyboard->Down(VK_SPACE)) {
                delta_movement += Vector3::UnitY;
            }
            if (m_pKeyboard->Down(VK_LCONTROL)) {
                delta_movement -= Vector3::UnitY;
            }
        }

        if (delta_movement != Vector3::Zero) {
            delta_movement.Normalize();

            if (!flycam_enabled) {
                btCollisionObject* pObj = m_pDynamicsWorld->getCollisionObjectArray()[0];
                pObj->activate(true);
                btRigidBody* pBody = btRigidBody::upcast(pObj);
                delta_movement *= 17500.0;


                btVector3 velocity = pBody->getLinearVelocity();
                btScalar magnitude = velocity.length();
                btScalar max_speed = 1200.0;

                if (magnitude <= max_speed) {
                    pBody->applyCentralForce(*(btVector3*)&delta_movement);
                }

            } else {
                player_pos += delta_movement * move_speed;

            }
        }
    }

    //TODO(sean): Ignore input if user has just refocused on the window
    if(m_pRenderer->GetHwnd() == GetFocus()) { // check if focused window is us
        // I see you looking at 
        //    _____this
        //     . .
        //
        // i came, i saw, i praised, the lord, then break the law.
        // you flipped it upside down :(

        RECT rect;
        GetWindowRect(m_pRenderer->GetHwnd(), &rect);
        POINT center = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
        const f32 mouse_sensitivity = 0.00333;

        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        SetCursorPos(center.x, center.y);

        // Mouse Click Testing
        m_leftClick.UpdateState();
        m_rightClick.UpdateState();

        if (m_leftClick.pressed) {
            m_pProjectileManager->GenerateSimProjectile(m_pDynamicsWorld->getCollisionObjectArray()[0], m_pRenderer->m_pCamera->GetPos(), m_pRenderer->m_pCamera->GetViewVector(), 3, 8000.0, 0.5, Colors::IndianRed, true);
        }

        if (m_rightClick.pressed) {
            m_pProjectileManager->GenerateRayProjectile(m_pDynamicsWorld->getCollisionObjectArray()[0], m_pRenderer->m_pCamera->GetPos(), m_pRenderer->m_pCamera->GetViewVector(), 3, 2, 0.05, Colors::IndianRed, false, true);
        }

        Vector2 delta = { (f32)(cursor_pos.x - center.x), (f32)(cursor_pos.y - center.y) };
        delta *= mouse_sensitivity;

        static const f32 pitch_bounds = XM_2PI / 4.0 - 0.05;

        yaw += delta.x;
        if (yaw > XM_2PI) {
            yaw -= XM_2PI;
        } else if (yaw < 0.0) {
            yaw += XM_2PI;
        }

        pitch += delta.y;
        if (pitch > pitch_bounds) {
            pitch = pitch_bounds;
        } else if (pitch < -pitch_bounds) {
            pitch = -pitch_bounds;
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

/// Ask the object manager to draw the game objects. The renderer is notified
/// of the start and end of the frame so that it can let Direct3D do its
/// pipelining jiggery-pokery.
void CGame::RenderFrame() {
    //NOTE(sean): Everything out here will technically be known
    // *before* we start rendering, so I dont want to give the impression
    // that we create it *while* rendering, although you certainly can...

    m_pRenderer->m_pCamera->SetYaw(yaw);
    m_pRenderer->m_pCamera->SetPitch(pitch);

    // This handles the timers tables.
    std::vector<Entity> toRemove;
    for every(index, m_Timers.Size()) {
        m_Timers.Components()[index] -= m_pTimer->GetFrameTime();
    }

    // This handles NPCs and lighting.
    for every(index, m_NPCsActive.Size()) {
        Entity e = m_NPCsActive.Entities()[index];
        btTransform trans;

        (*m_NPCs.Get(e)).Body->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();

        m_pNPCManager->DirectNPC(e, *m_RigidBodies.Get(player));
    }

    // This handles projectiles and lighting.
    for every(index, m_ProjectilesActive.Size()) {
        Entity e = m_ProjectilesActive.Entities()[index];
        btTransform trans;

        (*m_RigidBodies.Get(e))->getMotionState()->getWorldTransform(trans);
        m_pRenderer->lights.Get(e)->position = *(Vec4*)&trans.getOrigin();
        
        if (*m_Timers.Get(e) < 0.0) {
            toRemove.push_back(e);
        }
    }

    // Strip Projectiles.
    for every(index, toRemove.size()) {
        m_pProjectileManager->StripProjectile(toRemove[index]);
    }

    if (!flycam_enabled) {
        btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[0];
        btCollisionShape* shape = obj->getCollisionShape();
        btRigidBody* body = btRigidBody::upcast(obj);
        btTransform trans;

        if (body && body->getMotionState()) {
            body->getMotionState()->getWorldTransform(trans);
        }
        else {
            trans = obj->getWorldTransform();
        }

        m_pRenderer->m_pCamera->MoveTo(*(Vector3*)&trans.getOrigin());
    } else {
        m_pRenderer->m_pCamera->MoveTo(player_pos);
    }

    static f32 time = 0.0;
    time += m_pTimer->GetFrameTime();

    m_pRenderer->BeginFrame();


    // 0x01
    // 0x10
    // 0x11

    if (render_mode & 1) {
        m_pRenderer->BeginDrawing();
        {
            //NOTE(sean): Model instance rendering test.
            // You can use this as a baseline for how to render real 3d models.
            {
                ModelInstance instance = {};
                instance.model = ModelIndex::Suzanne;
                instance.texture = 0;
                f32 xoff = 400.0f * cosf(m_pTimer->GetTime());
                f32 zoff = 400.0f * sinf(m_pTimer->GetTime());
                instance.world = MoveScaleMatrix(Vector3(xoff, 50.0f, zoff), Vector3(100.0f, 100.0f, 100.0f));
                m_pRenderer->DrawModelInstance(&instance);
            }

            //NOTE(sean): test for rendering model instances onto bullet objects
            for every(j, m_pDynamicsWorld->getNumCollisionObjects()) {
                btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
                btCollisionShape* shape = obj->getCollisionShape();
                btRigidBody* body = btRigidBody::upcast(obj);
                btTransform trans;

                if (body && body->getMotionState()) {
                    body->getMotionState()->getWorldTransform(trans);
                }
                else {
                    trans = obj->getWorldTransform();
                }

                switch (shape->getShapeType()) {
                case(BT_SHAPE_TYPE_BOX): {
                    btBoxShape* castratedObject = reinterpret_cast<btBoxShape*>(shape);

                        ModelInstance instance = {};
                        instance.model = (u32)ModelIndex::Cube;
                        //instance.world = MoveScaleMatrix(trans.getOrigin(), castratedObject->getHalfExtentsWithMargin());
                        instance.world = MoveRotateScaleMatrix(trans.getOrigin(), *(Quat*)&trans.getRotation(), castratedObject->getHalfExtentsWithMargin());
                        instance.texture = 0;
                        m_pRenderer->DrawModelInstance(&instance);
                    } break;

                case(BT_SHAPE_TYPE_CAPSULE): {} break;

                default: {
                    ModelInstance instance = {};
                    instance.model = ModelIndex::Suzanne;
                    instance.world = MoveScaleMatrix(trans.getOrigin(), Vector3(25.0f));
                    instance.texture = 0;
                    m_pRenderer->DrawModelInstance(&instance);
                } break;
                }
            }
        }

        m_pRenderer->EndDrawing();
    } else {
        m_pRenderer->BeginDrawing();
        m_pRenderer->EndDrawing();
    }

    if (render_mode & 2) {
        m_pRenderer->BeginDebugDrawing();
        {
            //NOTE(sean): the reason batching all of this together works, is that we're doing all the vertex calculations on the cpu instead of the gpu
            m_pRenderer->BeginDebugLineBatch();
            {
                for every(j, m_pDynamicsWorld->getNumCollisionObjects()) {
                    btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
                    btCollisionShape* shape = obj->getCollisionShape();
                    btRigidBody* body = btRigidBody::upcast(obj);
                    btTransform trans;

                    if (body && body->getMotionState()) {
                        body->getMotionState()->getWorldTransform(trans);
                    }
                    else {
                        trans = obj->getWorldTransform();
                    }

                    switch (shape->getShapeType()) {
                    case(BT_SHAPE_TYPE_BOX): {
                        //NOTE(sean): render box
                        btBoxShape* castratedObject = reinterpret_cast<btBoxShape*>(shape);
                        BoundingBox box = BoundingBox(*(Vector3*)&trans.getOrigin(), *(Vector3*)&(castratedObject->getHalfExtentsWithMargin()));
                        m_pRenderer->DrawDebugAABB(box, Colors::Red);

                        //NOTE(sean): render grid on box
                        Vector3 ext = *(Vector3*)&castratedObject->getHalfExtentsWithMargin();
                        Vector3 x_axis(ext.x, 0, 0);
                        Vector3 y_axis(0.0, 0.0, ext.z);
                        Vector3 origin = *(Vector3*)&trans.getOrigin();
                        origin.y += ext.y;

                        m_pRenderer->DrawDebugGrid(x_axis, y_axis, origin, 4, 4, Colors::GreenYellow);
                    } break;

                    case(BT_SHAPE_TYPE_CAPSULE): {
                        btCapsuleShape* castratedObject = reinterpret_cast<btCapsuleShape*>(shape);
                        m_pRenderer->DrawDebugCapsule(*(Vector3*)&trans.getOrigin(), castratedObject->getRadius(), castratedObject->getHalfHeight(), 32, Colors::Purple);
                    } break;

                    default: {
                        btSphereShape* castratedObject = reinterpret_cast<btSphereShape*>(shape);
                        BoundingSphere sphere = BoundingSphere(*(Vector3*)&trans.getOrigin(), castratedObject->getRadius());
                        m_pRenderer->DrawDebugSphere(sphere, 16, Colors::CadetBlue);
                    } break;
                    }
                }

                for every(j, m_currentRayProjectiles.size()) {
                    //m_pRenderer->DrawDebugRay(m_currentRayProjectiles[j].Pos1, m_currentRayProjectiles[j].Pos2, 50000, m_currentRayProjectiles[j].Color);
                    m_pRenderer->DrawDebugLine(m_currentRayProjectiles[j].Pos1, m_currentRayProjectiles[j].Pos2, m_currentRayProjectiles[j].Color);
                }
            }
            m_pRenderer->EndDebugBatch();
        }

        m_pRenderer->tint_color = Vec3(1.0f, 1.0f, 1.0f);
        m_pRenderer->blur_amount = 0.0f;
        m_pRenderer->saturation_amount = 1.0f;
        m_pRenderer->EndDebugDrawing();
    }

    m_pRenderer->EndFrame();
}

/// This function will be called regularly to process and render a frame
/// of animation, which involves the following. Handle keyboard input.
/// Notify the  audio player at the start of each frame so that it can prevent
/// multiple copies of a sound from starting on the same frame.  
/// Move the game objects. Render a frame of animation.

void CGame::ProcessFrame(){
    InputHandler(); //handle keyboard input
    m_pAudio->BeginFrame(); //notify audio player that frame has begun
    m_pTimer->Tick([&]() { //all time-dependent function calls should go here
        m_pDynamicsWorld->stepSimulation(m_pTimer->GetFrameTime(), 10); // Step Physics
    });
    RenderFrame(); //render a frame of animation
}
