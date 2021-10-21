/// \file Common.h
/// \brief Interface for the class CCommon.

#ifndef __L4RC_GAME_COMMON_H__
#define __L4RC_GAME_COMMON_H__

//forward declarations to make the compiler less stroppy

class CObjectManager; 
class LSpriteRenderer;
class Renderer;

/// \brief The common variables class.
///
/// CCommon is a singleton class that encapsulates things that are common to
/// different game components, including game state variables. Making it a
/// singleton class means that we can avoid passing its member variables
/// around as parameters, which makes the code minisculely faster, and more
/// importantly, makes the code more readable by reducing function clutter.

class CCommon{
protected:  
    static Renderer* m_pRenderer; ///< Pointer to renderer.
    static CObjectManager* m_pObjectManager; ///< Pointer to object manager.
    //static LSpriteRenderer* m_pRenderer; ///< Pointer to renderer.
}; //CCommon

#endif //__L4RC_GAME_COMMON_H__
