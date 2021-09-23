/// \file Renderer.cpp
/// \brief Code for the renderer class CRenderer.

#include "Renderer.h"
#include "ComponentIncludes.h"

#include "Abort.h"

CRenderer::CRenderer():
  CSpriteRenderer(Batched2D){
} //constructor

/// Load the specific images needed for this game.
/// This is where eSpriteType values from GameDefines.h get
/// tied to the names of sprite tags in gamesettings.xml. Those
/// sprite tags contain the name of the corresponding image file.
/// If the image tag or the image file are missing, then
/// the game should abort from deeper in the Engine code,
/// leaving you with a dialog box that tells you what
/// went wrong.

void CRenderer::LoadImages(){  
  BeginResourceUpload();

  Load(BACKGROUND_SPRITE, "background"); 
  Load(BRICK_SPRITE, "brick");
  Load(BALL_SPRITE, "ball");
  Load(PADDLE_SPRITE, "paddle");

  EndResourceUpload();
} //LoadImages


