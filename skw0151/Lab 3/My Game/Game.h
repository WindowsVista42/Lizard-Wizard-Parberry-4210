/// \file Game.h
/// \brief Interface for the game class CGame.

#pragma once

#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"

/// \brief The game class.

class CGame: 
  public CComponent, 
  public CSettings,
  public CCommon{ 

  private:
    CObject* m_pPaddle = nullptr; ///< Pointer to the paddle object.

    void BeginGame(); ///< Begin playing the game.
    void KeyboardHandler(); ///< The keyboard handler.
    void RenderFrame(); ///< Render an animation frame.
    void CreateObjects(); ///< Create game objects.

  public:
    ~CGame(); ///< Destructor.

    void Initialize(); ///< Initialize the game.
    void ProcessFrame(); ///< Process an animation frame.
    void Release(); ///< Release the renderer.
}; //CGame
