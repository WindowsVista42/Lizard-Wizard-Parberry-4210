/// \file Game.cpp
/// \brief Code for the game class CGame.

#include "Game.h"

#include "GameDefines.h"
#include "Renderer.h"
#include "ComponentIncludes.h"
#include "ObjectManager.h"
#include "Descriptors.h"

#include "shellapi.h"

/// \brief Callback function for the end of a physics tick.
///
/// This function gets called by Bullet Physics at the end of a physics tick,
/// which may be more than once a frame. We use it to notify objects of 
/// contacts with other objects. The objects are then responsible for playing
/// their collision sounds. The user pointers of the two physics bodies that
/// are in contact are assumed to point to two corresponding `CObject`s. 
/// \param p Pointer to Physics World.
/// \param t Time step (unused).

void myTickCallback(btDynamicsWorld *p, btScalar t){
  UNREFERENCED_PARAMETER(t);

  const UINT nManifolds = (UINT)p->getDispatcher()->getNumManifolds();

  for(UINT i=0; i<nManifolds; i++){ //for each manifold
    btDispatcher* pDispatcher = p->getDispatcher(); //pointer to dispatcher
    btPersistentManifold* pManifold = pDispatcher->getManifoldByIndexInternal(i);

    //get body pointers from manifold
    const btRigidBody* pBody0 = btRigidBody::upcast(pManifold->getBody0()); 
    const btRigidBody* pBody1 = btRigidBody::upcast(pManifold->getBody1()); 
    
    //get object pointers from body pointers
    CObject* pObj0 = (CObject*)pBody0->getUserPointer();
    CObject* pObj1 = (CObject*)pBody1->getUserPointer(); 
    
    const int nContacts = pManifold->getNumContacts(); //number of contact points

    if(pObj0 && pObj1 && nContacts > 0){ //guard
      CContactDesc d(nContacts); //contact descriptor

      //compute impulse from contact points

      for(int j=0; j<nContacts; j++){ 
        btManifoldPoint& pt = pManifold->getContactPoint(j);
        d.m_fImpulse = std::max(d.m_fImpulse, pt.getAppliedImpulse());
      } //for

      pObj0->AddContact(pObj1, d); //add contact to one object only
    } //if
  } //for
} //myTickCallback

////////////////////////////////////////////////////////////////////////////////
// CGame functions.

/// Destructor.

CGame::~CGame(){
  delete m_pObjectManager;

  //delete collision shapes
  for(int j=0; j<m_btCollisionShapes.size(); j++){
    btCollisionShape* shape = m_btCollisionShapes[j];
    m_btCollisionShapes[j] = 0;
    delete shape;
  } //for
  
  delete m_pPhysicsWorld;
  delete m_pSolver;
  delete m_pBroadphase;
  delete m_pDispatcher;
  delete m_pConfig;
} //destructor

/// Initialize the Bullet Physics engine.

void CGame::InitBulletPhysics(){
  m_pConfig     = new btDefaultCollisionConfiguration();
  m_pDispatcher = new btCollisionDispatcher(m_pConfig);
  m_pBroadphase = new btDbvtBroadphase();
  m_pSolver     = new btSequentialImpulseConstraintSolver;

  m_pPhysicsWorld = new btDiscreteDynamicsWorld(
    m_pDispatcher, m_pBroadphase, m_pSolver, m_pConfig);

  m_pPhysicsWorld->setGravity(btVector3(0, -20.0f, 0));
} //InitBulletPhysics

/// Initialize the renderer, the physics engine, and the step timer.

void CGame::Initialize(){
  m_pRenderer = new CRenderer; 
  m_pRenderer->Initialize();
  m_pRenderer->LoadGeometricPrimitives(); //meshes for the spheres and boxes

  m_pRenderer->LoadModel(eModel::Container,
    L"Media\\Meshes\\Cargo_container_01\\Cargo_container_01.sdkmesh");
  
  m_pRenderer->LoadModel(eModel::Crate,
    L"Media\\Meshes\\Beer Crate\\Beer_Crate.sdkmesh");
  
  m_pObjectManager = new CObjectManager; //set up object manager 

  m_pTimer->SetFixedTimeStep();
  m_pTimer->SetFrameTime(1/60.0f);
  
  LoadSounds(); //load the sounds

  InitBulletPhysics(); //init bullet physics engine
  m_pPhysicsWorld->setInternalTickCallback(myTickCallback);

  LBaseCamera* pCamera = m_pRenderer->GetCameraPtr(); //camera pointer
  pCamera->MoveTo(Vector3(0, m_vPlanePos.y + 15.0f, 0)); //reset camera position
  m_pAudio->SetScale(64.0f);
  m_pAudio->SetListener(pCamera);

  BeginGame();
} //Initialize

/// Initialize the audio player and load game sounds.

void CGame::LoadSounds(){
  m_pAudio->Initialize(eSound::Size);

  m_pAudio->Load(eSound::Clang, "clang");
  m_pAudio->Load(eSound::Click, "click");
  m_pAudio->Load(eSound::TapLight, "taplight");
  m_pAudio->Load(eSound::TapHard, "taphard");
  m_pAudio->Load(eSound::ThumpLight, "thumplight");
  m_pAudio->Load(eSound::ThumpMedium, "thumpmedium");
  m_pAudio->Load(eSound::ThumpHard,"thumphard");
} //LoadSounds


/// Release all of the DirectX12 objects by deleting the renderer.

void CGame::Release(){
  delete m_pRenderer;
  m_pRenderer = nullptr; //for safety
} //Release

/// Create the game objects.

void CGame::CreateObjects(){
  CInstanceDesc d; //mesh instance descriptor

  //the floor, a plane

  d.m_eMeshType = eMesh::Plane;
  d.m_vPos = m_vPlanePos;
  d.m_vExtents = m_vPlaneSize;
  d.m_fMass = 0.0f; //static object
  d.m_fRestitution = 0.75f;
  d.m_fFriction = 0.5f;

  m_pObjectManager->Create(eObject::Floor, d); //the floor

  //the moon

  d.m_eMeshType = eMesh::BigSphere;
  d.m_vPos = Vector3(-120.0f, 25.5f, 200.0f);
  d.m_fScale = m_fMoonRadius;
  d.m_fMass = 1.2f;
  d.m_fRestitution = 0.9f;
  d.m_fFriction = 0.5f;

  m_pObjectManager->Create(eObject::Moon, d);

  //a pyramid of boxes

  const float delta = 0.1f; //gap between boxes
  size_t w = 8; //number of boxes wide
  size_t h = 8; //number of boxes high

  //position the first box so the pyramid is centered

  Vector3 pos = Vector3(0.0f, 3.5f, 140.0f); //initial box position
  const float x0 = pos.x - 0.5f*(w - 1)*(1 + delta)*m_vBoxSize.x;
  pos.x = x0; //correct the x-coordinate

  d.m_eMeshType = eMesh::Box;
  d.m_vExtents = m_vBoxSize;
  d.m_fMass = 0.5f;
  d.m_fRestitution = 0.75f;
  d.m_fFriction = 0.5f;

  for(size_t j=0; j<h; j++){ //for each row
    for(size_t i=0; i<w; i++){ //for each box in this row
      d.m_vPos = pos;
      m_pObjectManager->Create(eObject::Box, d); //make a box
      pos.x += m_vBoxSize.x*(1 + delta); //next box to the right
    } //for

    pos.x = x0 + 0.5f*(j + 1)*m_vBoxSize.x*(1 + delta); //reset to start of row
    pos.y += m_vBoxSize.y; //one row higher
    --w; //one less box in the next row
  } //for

  //a couple of containers, the kind you find on the back of trucks

  const float y = m_vPlanePos.y + m_vPlaneSize.y/2.0f;
  
  const float m_fContainerHalfHt = m_pRenderer->GetExtents(eModel::Container).y;

  d.m_eMeshType = eMesh::Model;
  d.m_eModelType = eModel::Container;
  d.m_fScale = 2.0f;
  d.m_vPos = Vector3(-100.0f, y + d.m_fScale*m_fContainerHalfHt, 50.0f);
  d.m_fMass = 8.0f;
  d.m_fRestitution = 0.5f;
  d.m_fFriction = 0.5f;

  m_pObjectManager->Create(eObject::Container, d);

  d.m_vPos.y += 2.0f*d.m_fScale*m_fContainerHalfHt;
  d.m_qOrientation = Quaternion::CreateFromAxisAngle(Vector3::UnitY, XM_PI/2.0f);
  m_pObjectManager->Create(eObject::Container, d);

  //a pile of plastic crates
  
  const float m_fCrateHalfHt = m_pRenderer->GetExtents(eModel::Crate).y;

  d.m_eModelType = eModel::Crate;
  d.m_fScale = 8.0f;
  d.m_vPos = Vector3(100.0f, y + d.m_fScale*m_fCrateHalfHt, 50.0f);
  d.m_qOrientation = Quaternion::Identity;
  d.m_fMass = 1.0f;
  d.m_fRestitution = 0.2f;
  d.m_fFriction = 0.3f;
  
  for(size_t i=0; i<4; i++){
    m_pObjectManager->Create(eObject::Crate, d);
    d.m_vPos.y += 2.0f*d.m_fScale*m_fCrateHalfHt;
  } //for

  //a tetrahedron
  
  d.m_eMeshType = eMesh::Tetrahedron;
  d.m_fScale = m_fTetrahedronSize;
  d.m_vPos = Vector3(-40.0f, d.m_fScale/3.0f + m_fFloorHt, -100.0f);
  d.m_qOrientation = Quaternion::CreateFromYawPitchRoll(XM_PI, -XM_PIDIV2, 0);
  d.m_fMass = 3.0f;
  d.m_fRestitution = 0.5f;
  d.m_fFriction = 0.7f;

  m_pObjectManager->Create(eObject::Tetrahedron, d);

  //a dodecahedron
  
  const float SQRT5 = sqrtf(5.0f);
  
  d.m_eMeshType = eMesh::Dodecahedron;
  d.m_fScale = m_fDodecahedronSize;
  const float dr = d.m_fScale*sqrtf(750.0f + 330.0f*SQRT5)/(40.0f*(SQRT5 - 1.0f));
  d.m_vPos = Vector3(-80.0f, dr + m_fFloorHt, -100.0f);
  const float da = XM_PI - atanf(2.0f); //dodecahedron dihedral angle
  d.m_qOrientation = Quaternion::CreateFromAxisAngle(Vector3::UnitZ, da/2.0f);
  d.m_fMass = 3.0f;
  d.m_fRestitution = 0.6f;
  d.m_fFriction = 0.6f;

  m_pObjectManager->Create(eObject::Dodecahedron, d);

  //an icosahedron
  
  d.m_eModelType = eModel::Unknown;
  d.m_eMeshType = eMesh::Icosahedron;
  d.m_fScale = m_fIcosahedronSize;
  d.m_vPos = Vector3(-140.0f, d.m_fScale + m_fFloorHt, -100.0f);
  const float ida = XM_PI - acosf(SQRT5/3.0f); //icosahedron dihedral angle
  d.m_qOrientation = Quaternion::CreateFromAxisAngle(Vector3::UnitX, ida/2.0f);
  d.m_fMass = 3.0f;
  d.m_fRestitution = 1.0f;
  d.m_fFriction = 0.9f;

  m_pObjectManager->Create(eObject::Icosahedron, d);

  //a teapot
  
  d.m_eMeshType = eMesh::Teapot;
  d.m_vPos = Vector3(60.0f, 10.0f, -100.0f);
  d.m_qOrientation = Quaternion::Identity;
  d.m_fScale = m_fTeapotSize;
  d.m_fMass = 0.05f;
  d.m_fRestitution = 0.0f;
  d.m_fFriction = 0.75f;
  
  m_pObjectManager->Create(eObject::Teapot, d);
} //CreateObjects

/// Start a new game. Clear the object manager and create new game objects.

void CGame::BeginGame(){  
  m_fStartTime = m_pTimer->GetTime(); //mark time
  m_bCollisionSoundsMuted = true; //mute sounds
  m_pObjectManager->Clear(); //clear old objects
  CreateObjects(); //create new objects 
} //BeginGame

/// Fire a small sphere from the camera location in the direction of its view
/// vector using an impulse with a fixed magnitude.

void CGame::Fire(){
  LBaseCamera* pCamera = m_pRenderer->GetCameraPtr();

  CInstanceDesc d; //instance descriptor for soccer ball
  d.m_vPos = pCamera->GetPos() + 4*m_fBallRadius*pCamera->GetViewVector();
  d.m_eMeshType = eMesh::Sphere;
  d.m_fScale = m_fBallRadius;
  d.m_fMass = 0.1f;
  d.m_fRestitution = 0.9f;
  d.m_fFriction = 0.5f;

  CObject* p = m_pObjectManager->Create(eObject::Ball, d); //create it
  p->ApplyImpulse(10.0f*pCamera->GetViewVector(), Vector3::Zero); //kick it
} //Fire

/// Keyboard handler.

void CGame::KeyboardHandler(){
  if(m_pKeyboard == nullptr)return; //no keyboard
  if(m_pController && m_pController->IsConnected())return; //use controller

  m_pKeyboard->GetState(); //get current keyboard state 
  
  if(m_pKeyboard->TriggerDown(VK_F1)) //help
    ShellExecute(0, 0, "https://larc.unt.edu/code/physics/block3d/", 
    0, 0, SW_SHOW);

  const float t = m_pTimer->GetFrameTime();
  LBaseCamera* pCamera = m_pRenderer->GetCameraPtr();
  
  if(m_pKeyboard->TriggerDown(VK_SPACE))
    Fire();

  if(m_pKeyboard->TriggerDown(VK_BACK))
    BeginGame();
  
  if(pCamera){ //camera navigation
    if(m_pKeyboard->Down(VK_LEFT)) pCamera->AddToYaw(-t/2);
    if(m_pKeyboard->Down(VK_RIGHT))pCamera->AddToYaw(t/2); 
    if(m_pKeyboard->Down(VK_UP))   pCamera->AddToPitch(-t/4);
    if(m_pKeyboard->Down(VK_DOWN)) pCamera->AddToPitch(t/4);
    
    const Vector3 vLookat = pCamera->GetViewVector(); //view vector
    const Vector3 vRight(vLookat.z, 0, -vLookat.x); //right relative to view vector

    const float fSpeed = 20.0f; //speed multiplier
    const Vector3 vRightDelta = 0.8f*fSpeed*t*vRight; //position delta rightwards
    if(m_pKeyboard->Down('A'))pCamera->AddToPos(-vRightDelta); //strafe left
    if(m_pKeyboard->Down('D'))pCamera->AddToPos(vRightDelta); //strafe right
      
    const Vector3 vForwardDelta = fSpeed*t*vLookat; //position delta forwards
    if(m_pKeyboard->Down('W'))pCamera->AddToPos(vForwardDelta); //move forwards
    if(m_pKeyboard->Down('S'))pCamera->AddToPos(-vForwardDelta); //move backwards
  } //if
} //KeyboardHandler

/// Controller handler. Poll the controller and react to the relevant controls.

void CGame::ControllerHandler(){
  if(!m_pController)return; //no device driver
  if(!m_pController->IsConnected())return; //no controller plugged in

  const float t = m_pTimer->GetFrameTime(); //frame time
  LBaseCamera* pCamera = m_pRenderer->GetCameraPtr(); //camera pointer
  const Vector3 vLookat = pCamera->GetViewVector(); //view vector
  const Vector3 vRight(vLookat.z, 0, -vLookat.x); //right relative to view vector

  if(m_pController->GetButtonRSToggle()) //RS button
    Fire(); //fire a sphere

  if(m_pController->GetButtonAToggle()) //button A
    BeginGame(); //reset

  if(pCamera){ //camera navigation
    //camera orientation - right thumb
    const Vector2 rThumb = m_pController->GetRThumb(); //right thumb
    const float ydelta = rThumb.x*t; //yaw amount this frame
    const float pdelta = rThumb.y*t; //pitch amount this frame

    if(ydelta != 0)pCamera->AddToYaw(ydelta); //apply yaw
    if(pdelta != 0)pCamera->AddToPitch(-pdelta); //apply pitch

    //camera translation - digital pad
    const float fSpeed = 20.0f; //speed multiplier
    const Vector3 vRightDelta = 0.8f*fSpeed*t*vRight; //position delta rightwards

    if(m_pController->GetDPadLeft()) //strafe left
      pCamera->AddToPos(-vRightDelta);

    if(m_pController->GetDPadRight()) //strafe right
      pCamera->AddToPos(vRightDelta);

    const Vector3 vForwardDelta = fSpeed*t*vLookat; //position delta forwards
  
    if(m_pController->GetDPadUp()) //move forwards
      pCamera->AddToPos(vForwardDelta);

    if(m_pController->GetDPadDown()) //move backwards
      pCamera->AddToPos(-vForwardDelta);
  
  } //if
} //ControllerHandler

/// Render an animation frame. 

void CGame::RenderFrame(){
  const float t = m_pTimer->GetFrameTime(); //frame time
  m_pRenderer->GetCameraPtr()->Move(t); //move camera

  m_pRenderer->BeginFrame();
  m_pObjectManager->Draw(); //draw objects
  m_pRenderer->EndFrame();
} //RenderFrame

/// Process an animation frame. Handle keyboard and controller input, advance
/// the timer a tick, perform a physics world step, update all objects, and
/// finally render them.

void CGame::ProcessFrame(){
  ControllerHandler(); //handle controller input
  KeyboardHandler(); //handle keyboard input
  m_pAudio->BeginFrame(); //notify audio player that frame has begun
  m_pAudio->SetListener(m_pCamera); //move listener to camera

  m_pTimer->Tick([&](){ 
    const float t = m_pTimer->GetFrameTime(); //frame time
    m_pPhysicsWorld->stepSimulation(t, 4); //physics world step
    m_pObjectManager->Update(); //update all objects
  });

  //prevent initial contact sounds from playing

  if(m_pTimer->GetTime() - m_fStartTime > 0.5f)
    m_bCollisionSoundsMuted = false;

  RenderFrame(); //render a frame of animation
} //ProcessFrame