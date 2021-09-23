/// \file ObjectManager.h
/// \brief Interface for the object manager CObjectManager.

#pragma once

#include <list>

#include "Object.h"

#include "Component.h"
#include "Common.h"
#include "Settings.h"

using namespace std;

/// \brief The object manager.
///
/// A collection of all of the game objects.

class CObjectManager: 
  public CComponent, 
  public CCommon, 
  public CSettings{

  private:
    list<CObject*> m_stdObjectList; ///< Object list.

    void BroadPhase(); ///< Broad phase collision detection and response.
    void NarrowPhase(CObject* p0, CObject* p1); ///< Narrow phase collision detection and response.
    void CullDeadObjects(); ///< Cull dead objects.

  public:
    CObjectManager(); ///< Constructor.
    ~CObjectManager(); ///< Destructor.

    CObject* create(eSpriteType t, const Vector2& pos, 
      const Vector2& vel=Vector2::Zero); ///< Create new object.

    void clear(); ///< Reset to initial conditions.
    void move(); ///< Move all objects.
    void draw(); ///< Draw all objects.
}; //CObjectManager