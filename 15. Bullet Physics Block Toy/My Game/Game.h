/// \file Game.h
/// \brief Interface for the game class CGame.

#ifndef __L4RC_GAME_GAME_H__
#define __L4RC_GAME_GAME_H__

#include "Component.h"
#include "Common.h"
#include "Settings.h"
#include "Object.h"

/// \brief The game class.
///
/// The game class encapsulates our game/toy/whatever.

class CGame: 
  public LComponent, 
  public LSettings,
  public CCommon
{
  private:
    float m_fStartTime = 0; ///< Game start time.
    
    void LoadSounds(); ///< Load sounds. 
    void BeginGame(); ///< Begin playing the game.
    void KeyboardHandler(); ///< The keyboard handler.
    void ControllerHandler(); ///< The controller handler.
    void RenderFrame(); ///< Render an animation frame.
    void CreateObjects(); ///< Create game objects.
    void InitBulletPhysics(); ///< Initialize Bullet Physics engine.
    void Fire(); ///< Fire a ball.

  public:
    ~CGame(); ///< Destructor.

    void Initialize(); ///< Initialize the game.
    void ProcessFrame(); ///< Process an animation frame.
    void Release(); ///< Release the renderer.
}; //CGame

#endif //__L4RC_GAME_GAME_H__
