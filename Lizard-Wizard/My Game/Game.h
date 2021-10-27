/// \file Game.h
/// \brief Interface for the game class CGame.

#ifndef GAME_H
#define GAME_H

#include "Component.h"
#include "PhysicsManager.h"
#include "ProjectileManager.h"
#include "GenerationManager.h"
#include "NPCManager.h"
#include "Settings.h"
#include "CustomBind.h"
#include "Renderer.h"
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

class CGame:
    public LComponent, 
    public LSettings
{ 
private:
    // Mouse Binds
    CustomBind m_leftClick;
    CustomBind m_rightClick;

    // Managers
    PhysicsManager* m_pPhysicsManager;
    GenerationManager* m_pGenerationManager;
    ProjectileManager* m_pProjectileManager;
    NPCManager* m_pNPCManager;

    // Model Table
    Table<ModelInstance> m_ModelInstances;

    // Timing Table
    Table<f32> m_Timers;

    // Projectile Cache (MAX 64)
    Table<btRigidBody*> m_RigidBodies;
    Group m_ProjectilesCache;
    Group m_ProjectilesActive;

    // AI Cache (MAX 24)
    Table<NPC> m_NPCs;
    Group m_NPCsCache;
    Group m_NPCsActive;

    // UserInput Vector
    //CustomBind m_currentBinds;
    
    // Projectile Vector
    //std::vector<Projectile> m_currentProjectiles;
    //std::vector<RayProjectile> m_currentRayProjectiles;

    i32 randomSeed;

    bool m_bDrawFrameRate = false; ///< Draw the frame rate.
    
    void LoadImages(); ///< Load images.
    void LoadModels(); ///< Load models.
    void LoadSounds(); ///< Load sounds.
    void BeginGame(); ///< Begin playing the game.
    void CreateObjects(); ///< Create game objects.
    void InputHandler(); ///< The keyboard handler.
    void RenderFrame(); ///< Render an animation frame.
    void DrawFrameRateText(); ///< Draw frame rate text to screen.

public:
    Renderer* m_pRenderer; ///< Pointer to renderer.
    Entity player;
    // Bullet3 Declarationsstatic 
    btAlignedObjectArray<btCollisionShape*> m_pCollisionShapes;
    btDiscreteDynamicsWorld* m_pDynamicsWorld;
    std::vector<RayProjectile> m_currentRayProjectiles;
    ~CGame(); ///< Destructor.
    void Initialize(); ///< Initialize the game.
    void ProcessFrame(); ///< Process an animation frame.
    void Release(); ///< Release the renderer.
}; //CGame

#endif //__L4RC_GAME_GAME_H__
