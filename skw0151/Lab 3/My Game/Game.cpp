/// \file Game.cpp
/// \brief Code for the game class CGame.

#include "Game.h"

#include "GameDefines.h"
#include "Renderer.h"
#include "ComponentIncludes.h"

/// Delete the renderer and the object manager.

CGame::~CGame(){
  delete m_pRenderer;
  delete m_pObjectManager;
} //destructor

/// Initialize the renderer and the object manager, load 
/// images and sounds, and begin the game.

void CGame::Initialize(){
  m_pRenderer = new CRenderer; 
  m_pRenderer->Initialize(NUM_SPRITES); 
  m_pRenderer->LoadImages(); //load images from xml file list

  m_pObjectManager = new CObjectManager; //set up object manager 
  m_pAudio->Load(); //load the sounds for this game

  BeginGame();
} //Initialize

/// Release all of the DirectX12 objects by deleting the renderer.

void CGame::Release(){
  delete m_pRenderer;
  m_pRenderer = nullptr; //for safety
} //Release

/// Ask the object manager to create the game objects.

void CGame::CreateObjects(){
  m_pPaddle = m_pObjectManager->create(PADDLE_SPRITE, 
    Vector2(m_vWinCenter.x, 32)); //the paddle

  //bricks

  float w, h; //sprite width and height
  m_pRenderer->GetSize(BRICK_SPRITE, w, h); //get sprite dimensions
  Vector2 vStartPos(w/2, m_nWinHeight - h/2); //start position of row

  for(int i=0; i<5; i++){ //one row per iteration
    Vector2 vPos(vStartPos); //position of leftmost brick in row
    if(i & 1)vPos.x -= w/2; //offset odd rows by half a brick

    while(vPos.x <= m_nWinWidth + w/2){ //fill the row
      m_pObjectManager->create(BRICK_SPRITE, vPos); //lay a brick
      vPos.x += w; //move right
    } //while

    vStartPos.y -= h; //move down for next row
  } //for
} //CreateObjects

/// Call this function to start a new game. This
/// should be re-entrant so that you can restart a
/// new game without having to shut down and restart the
/// program, but it's moot here. All we really need
/// to do is clear any old objects out of the object manager
/// and create some new ones.

void CGame::BeginGame(){  
  m_pObjectManager->clear(); //clear old objects
  CreateObjects(); //create new objects 
  m_bBallInPlay = false; //no ball in play yet
} //BeginGame

/// Poll the keyboard state and respond to the
/// key presses that happened since the last frame.

void CGame::KeyboardHandler(){
  m_pKeyboard->GetState(); //get current keyboard state 
  
  if(m_pKeyboard->TriggerDown(VK_RIGHT))
    m_pPaddle->SetSpeed(350.0f);

  if(m_pKeyboard->TriggerUp(VK_RIGHT))
    m_pPaddle->SetSpeed(0.0f);

  if(m_pKeyboard->TriggerDown(VK_LEFT))
    m_pPaddle->SetSpeed(-350.0f);

  if(m_pKeyboard->TriggerUp(VK_LEFT))
    m_pPaddle->SetSpeed(0.0f);
  
  if(m_pKeyboard->TriggerDown(VK_SPACE))
    if(!m_bBallInPlay){
      float theta = 3*XM_PI/8 + m_pRandom->randf()*XM_PI/4;

      if(theta <= XM_PI/2 && theta > 7*XM_PI/16)
        theta -= XM_PI/16;
      else if(theta > XM_PI/2 && theta < 9*XM_PI/16)
        theta += XM_PI/16;

      const Vector2 vel(Vector2(cos(theta), sin(theta)));
      const Vector2 pos(m_vWinCenter.x, 64.0f);

      m_pObjectManager->create(BALL_SPRITE, pos, 500.0f*vel);
      m_bBallInPlay = true;
      m_pAudio->play(LAUNCH_SOUND);
    } //if
  
  if(m_pKeyboard->TriggerUp(VK_RETURN))
    BeginGame();
} //KeyboardHandler

/// Ask the object manager to draw the game objects. The renderer
/// is notified of the start and end of the frame so
/// that it can let Direct3D do its pipelining jiggery-pokery.

void CGame::RenderFrame(){
  m_pRenderer->BeginFrame();
    m_pRenderer->Draw(BACKGROUND_SPRITE, m_vWinCenter); //draw background
    m_pObjectManager->draw(); 
  m_pRenderer->EndFrame();
} //RenderFrame

/// Handle keyboard input, move the game objects and render 
/// them in their new positions and orientations. Notify the 
/// audio player at the start of each frame so that it can 
/// prevent multiple copies of a sound from starting on the
/// same frame. Notify the timer of the start and end of the
/// frame so that it can calculate frame time. 

void CGame::ProcessFrame(){
  KeyboardHandler(); //handle keyboard input
  m_pAudio->BeginFrame(); //notify audio player that frame has begun
  
  m_pStepTimer->Tick([&](){ 
    m_pObjectManager->move(); //move all objects
  });

  RenderFrame(); //render a frame of animation
} //ProcessFrame