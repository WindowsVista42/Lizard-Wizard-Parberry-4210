/// \file Game.cpp
/// \brief Code for the game class CGame.

#include "Game.h"
#include "Math.h"
#include "Renderer.h"
#include "GameDefines.h"
#include "SpriteRenderer.h"
#include "ComponentIncludes.h"
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
const f32 move_speed = 10.0f;

static ModelInstance model_instance;

static bool flycam_enabled = false;

CGame::~CGame(){
  delete m_pObjectManager;
}

/// Create the renderer and the object manager, load images and sounds, and
/// begin the game.
/// 
void CGame::Initialize(){
    m_pRenderer = new CRenderer();
    m_pRenderer->Initialize();
  
    LoadImages(); //load images from xml file list
    LoadModels(); //load models from xml file list

    m_pObjectManager = new CObjectManager; //set up the object manager
    m_pProjectileManager = new ProjectileManager(); // set up projectile manager
    LoadSounds(); //load the sounds for this game

    //TODO(ethan): Move this into its own init function
    // Bullet3 Initialize
    {
        m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
        m_pDispatcher = new btCollisionDispatcher(m_pCollisionConfiguration);
        m_pBroadphaseChache = new btDbvtBroadphase();
        m_pSolver = new btSequentialImpulseConstraintSolver;
        m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pBroadphaseChache, m_pSolver, m_pCollisionConfiguration);
        m_pDynamicsWorld->setGravity(btVector3(0.0, -5000.0, 0.0));
        m_pCollisionShapes = btAlignedObjectArray<btCollisionShape*>();
        m_currentRayProjectiles = std::vector<RayProjectile>();
        m_pProjectileManager->InitializeProjectiles(m_pCollisionShapes, &m_currentRayProjectiles, m_pDynamicsWorld);
        randomSeed = 0;
        //m_physicsScratch = StagedBuffer(16 * 1024);   

        // Ground Collider
        {
            // Ground
            //btCollisionShape* collisionShape = m_physicsScratch.Alloc(sizeof(btBoxShape));
            btCollisionShape* collisionShape = new btBoxShape(btVector3(3000., 50., 3000.));
            m_pCollisionShapes.push_back(collisionShape);

            // Transforms
            btTransform collisionTransform;
            collisionTransform.setIdentity();
            collisionTransform.setOrigin(btVector3(0, -200, 0));

            // Mass
            btScalar mass(0.);
            btScalar restitution(0.f);
            btScalar friction(1.5);

            // Rigidbody is dynamic if and only if mass is non zero, otherwise static
            bool isDynamic = (mass != 0.f);

            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                collisionShape->calculateLocalInertia(mass, localInertia);

            // Motion state, is optional.
            btDefaultMotionState* myMotionState = new btDefaultMotionState(collisionTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionShape, localInertia);
            rbInfo.m_restitution = restitution;
            rbInfo.m_friction = friction;
            btRigidBody* body = new btRigidBody(rbInfo);


            // Adds body to the dynamic world.
            m_pDynamicsWorld->addRigidBody(body);
         }

        // Player Rigidbody
        // NOTE(ethan): This ideally will act as the players physical presence in the world, has the shape of a capsule.
        {
            // Creates our players body / hitbox.
            btCollisionShape* playerShape = new btCapsuleShape(btScalar(100.), btScalar(250.));
            m_pCollisionShapes.push_back(playerShape);

            // Implicating playerShape as a dynamic object.
            btTransform startTransform;
            startTransform.setIdentity();
            btScalar mass(2.);
            btScalar friction(0.5);
            bool isDynamic = (mass != 0.f);
            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
               playerShape->calculateLocalInertia(mass, localInertia);
            startTransform.setOrigin(btVector3(0, 1500, 0));


            // Motionstate again.
            btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, playerShape, localInertia);
            rbInfo.m_friction = friction;
            btRigidBody* body = new btRigidBody(rbInfo);
            body->setAngularFactor(btVector3(0, 0, 0));

            m_pDynamicsWorld->addRigidBody(body);
        }
        

        // Wall Collider
        {
            // Wall
            //btCollisionShape* collisionShape = m_physicsScratch.Alloc(sizeof(btBoxShape));
            btCollisionShape* collisionShape = new btBoxShape(btVector3(150., 3000., 3000.));
            m_pCollisionShapes.push_back(collisionShape);

            // Transforms
            btTransform collisionTransform;
            collisionTransform.setIdentity();
            collisionTransform.setOrigin(btVector3(-3000, 2800, 0));

            // Mass
            btScalar mass(0.);
            btScalar restitution(0.f);
            btScalar friction(1.5);

            // Rigidbody is dynamic if and only if mass is non zero, otherwise static
            bool isDynamic = (mass != 0.f);

            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                collisionShape->calculateLocalInertia(mass, localInertia);

            // Motion state, is optional.
            btDefaultMotionState* myMotionState = new btDefaultMotionState(collisionTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionShape, localInertia);
            rbInfo.m_restitution = restitution;
            rbInfo.m_friction = friction;
            btRigidBody* body = new btRigidBody(rbInfo);


            // Adds body to the dynamic world.
            m_pDynamicsWorld->addRigidBody(body);
        }
    }

    //NOTE(sean): testing debug model loading
    //getting this to work will be pretty similar to actual 3d models
    {
        const VertexPC verts[9] = {
            // position and color
            { {{-1.f, -1.f, -1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{ 1.f, -1.f, -1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{ 1.f, -1.f,  1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{-1.f, -1.f,  1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{-1.f,  1.f, -1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{ 1.f,  1.f, -1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{ 1.f,  1.f,  1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{-1.f,  1.f,  1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} },
            { {{ 1.f,  3.f,  1.f, 0.f }}, {{1.0, 1.0, 1.0, 1.0}} }
        };

        // this is probably 0 in this contrived case
        u32 handle = m_pRenderer->AddDebugModel(&DebugModel(verts, 9, DebugModelType::TRIANGLE_LIST));
        model_instance = ModelInstance(handle);
        Vector3 scale = { 100.0, 100.0, 100.0 };
        model_instance.m_worldMatrix = MoveScaleMatrix(Vector3::Zero, scale);
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
    m_pRenderer->LoadAllModels();
}

void CGame::LoadImages(){
    /*
    m_pRenderer->BeginResourceUpload();

    m_pRenderer->Load(eSprite::Background, "background"); 
    m_pRenderer->Load(eSprite::TextWheel,  "textwheel"); 

    m_pRenderer->EndResourceUpload();
    */
}

/// Initialize the audio player and load game sounds.

void CGame::LoadSounds(){
  m_pAudio->Initialize(eSound::Size);
  m_pAudio->Load(eSound::Grunt, "grunt");
  m_pAudio->Load(eSound::Clang, "clang");
}

/// Release all of the DirectX12 objects by deleting the renderer.

void CGame::Release(){
  delete m_pRenderer;
  m_pRenderer = nullptr; //for safety
}

/// Ask the object manager to create the game objects. There's only one in this
/// game, the rotating wheel o' text centered at the center of the window.

void CGame::CreateObjects(){
  m_pObjectManager->create(eSprite::TextWheel, m_vWinCenter); 
}

/// Call this function to start a new game. This should be re-entrant so that
/// you can restart a new game without having to shut down and restart the
/// program. All we need to do is delete any old objects out of the object
/// manager and create some new ones.

void CGame::BeginGame(){  
  m_pObjectManager->clear(); //clear old objects
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
        btCollisionObject* pObj = m_pDynamicsWorld->getCollisionObjectArray()[1];
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

    if (m_pKeyboard->TriggerDown('F')) {
        flycam_enabled = !flycam_enabled;
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
                btCollisionObject* pObj = m_pDynamicsWorld->getCollisionObjectArray()[1];
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
            m_pProjectileManager->GenerateSimProjectile(m_pDynamicsWorld->getCollisionObjectArray()[1], m_pRenderer->m_pCamera->GetPos(), m_pRenderer->m_pCamera->GetViewVector(), 3, 4.0, 5.0, Colors::IndianRed, true);
        }

        if (m_rightClick.pressed) {
            m_pProjectileManager->GenerateRayProjectile(m_pDynamicsWorld->getCollisionObjectArray()[1], m_pRenderer->m_pCamera->GetPos(), m_pRenderer->m_pCamera->GetViewVector(), 3, 3, 5.0, Colors::IndianRed, false, true);
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

    if (!flycam_enabled) {
        btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[1];
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
    {
        {
            for every(j, m_pDynamicsWorld->getNumCollisionObjects()) {
                btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
                btCollisionShape* shape = obj->getCollisionShape();
                btRigidBody* body = btRigidBody::upcast(obj);
                btTransform trans;

                if (body && body->getMotionState()) {
                    body->getMotionState()->getWorldTransform(trans);
                } else {
                    trans = obj->getWorldTransform();
                }

                switch(shape->getShapeType()) {
                case(BT_SHAPE_TYPE_BOX): {} break;

                case(BT_SHAPE_TYPE_CAPSULE): {} break;

                default: {
                    ModelInstance instance = {};
                    instance.m_modelIndex = (u32)ModelType::Cube;
                    instance.m_worldMatrix = MoveScaleMatrix(trans.getOrigin(), Vector3(10.0f, 10.0f, 10.0f));
                    m_pRenderer->DrawModelInstance(&instance);
                } break;
                }
            }
        }

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
                } else {
                    trans = obj->getWorldTransform();
                }

                switch(shape->getShapeType()) {
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
                    m_pRenderer->DrawDebugCapsule(*(Vector3*)&trans.getOrigin(), castratedObject->getRadius(), castratedObject->getHalfHeight() , 32, Colors::Purple);
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

        {
            ModelInstance instance = {};
            instance.m_modelIndex = (u32)ModelType::Cube;
            f32 xoff = 400.0f * cosf(m_pTimer->GetTime());
            f32 zoff = 400.0f * sinf(m_pTimer->GetTime());
            instance.m_worldMatrix = MoveScaleMatrix(Vector3(xoff, 100.0f, zoff), Vector3(100.0f, 100.0f, 100.0f));
            m_pRenderer->DrawModelInstance(&instance);
        }
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
        m_pObjectManager->move(); //move all objects
        m_pDynamicsWorld->stepSimulation(m_pTimer->GetFrameTime(), 10); // Step Physics
    });
    RenderFrame(); //render a frame of animation
}
