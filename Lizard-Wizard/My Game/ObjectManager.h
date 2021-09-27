/// \file ObjectManager.h
/// \brief Interface for the object manager CObjectManager.

#ifndef __L4RC_GAME_OBJECTMANAGER_H__
#define __L4RC_GAME_OBJECTMANAGER_H__

#include "BaseObjectManager.h"
#include "Object.h"
#include "Common.h"

/// \brief The object manager.
///
/// A collection of all of the game objects.

class CObjectManager: 
  public LBaseObjectManager<CObject>,
  public CCommon{
  public:
    CObject* create(eSprite, const Vector2&); ///< Create new object.
}; //CObjectManager

#endif //__L4RC_GAME_OBJECTMANAGER_H__
