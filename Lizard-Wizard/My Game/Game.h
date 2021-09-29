/// \file Game.h
/// \brief Interface for the game class CGame.

#ifndef __L4RC_GAME_GAME_H__
#define __L4RC_GAME_GAME_H__

#include "Component.h"
#include "Common.h"
#include "ObjectManager.h"
#include "Settings.h"
#include <vector>

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


/// \brief The game class.
///
/// The game class is the object-oriented implementation of the game. This class
/// must contain the following public member functions. `Initialize()` does
/// initialization and will be run exactly once at the start of the game.
/// `ProcessFrame()` will be called once per frame to create and render the
/// next animation frame. `Release()` will be called at game exit but before
/// any destructors are run.

// Bind Struct
typedef struct CustomBind {
    i32 bind;
    b8 held, pressed, _pad0, _pad1;
} CustomBind;

typedef struct Projectile {
    char projectile;
} Projectile;

typedef struct RayProjectile {
    Vector3 Pos1;
    Vector3 Pos2;
    XMVECTORF32 Color;
} RayProjectile;

class CGame:
	public LComponent, 
	public LSettings,
	public CCommon
{ 
  private:
    // Bullet3 Declarations
    btDefaultCollisionConfiguration* m_pCollisionConfiguration;
    btCollisionDispatcher* m_pDispatcher;
    btDbvtBroadphase* m_pBroadphaseChache;
    btSequentialImpulseConstraintSolver* m_pSolver;
    btDiscreteDynamicsWorld* m_pDynamicsWorld;
    btAlignedObjectArray<btCollisionShape*> m_pCollisionShapes;
    enum ColliderType { BOX, SPHERE, PLANE };
    std::vector<ColliderType> m_colliderTypes;
    CustomBind m_leftClick;
    CustomBind m_rightClick;

    //StagedBuffer m_physicsScratch;
    

    // UserInput Vector
    CustomBind m_currentBinds;
    
    // Projectile Vector
    std::vector<Projectile*> m_currentProjectiles;
    std::vector<RayProjectile*> m_currentRayProjectiles;


    bool m_bDrawFrameRate = false; ///< Draw the frame rate.
    
    void LoadImages(); ///< Load images.
    void LoadSounds(); ///< Load sounds.
    void BeginGame(); ///< Begin playing the game.
    void CreateObjects(); ///< Create game objects.
    void InputHandler(); ///< The keyboard handler.
    void RenderFrame(); ///< Render an animation frame.
    void DrawFrameRateText(); ///< Draw frame rate text to screen.

    // Projectiles 
    void FireProjectile();
    void FireRaycast();

  public:
    ~CGame(); ///< Destructor.
    void UpdateCustomBindState(CustomBind*);
    void Initialize(); ///< Initialize the game.
    void ProcessFrame(); ///< Process an animation frame.
    void Release(); ///< Release the renderer.
}; //CGame

#endif //__L4RC_GAME_GAME_H__
