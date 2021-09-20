/// \file ObjectManager.h
/// \brief Interface for the object manager CObjectManager.

#ifndef __L4RC_GAME_OBJECTMANAGER_H__
#define __L4RC_GAME_OBJECTMANAGER_H__

#include <list>

#include "Object.h"

#include "Component.h"
#include "Common.h"
#include "Settings.h"

/// \brief The object manager.
///
/// A collection of all of the game objects.

class CObjectManager:
  public CCommon,  
  public LComponent, 
  public LSettings{

  private:
    std::list<CObject*> m_stdObjectList; ///< Object list.

    void CullDeadObjects(); ///< Cull dead objects.

  public:
    CObjectManager(); ///< Constructor.
    ~CObjectManager(); ///< Destructor.

    CObject* Create(eObject, const CInstanceDesc&); ///< Create an object.
    void Update(); ///< Update.
    void Clear(); ///< Reset to initial conditions.
    void Draw(); ///< Draw all objects.
}; //CObjectManager

#endif //__L4RC_GAME_OBJECTMANAGER_H__
