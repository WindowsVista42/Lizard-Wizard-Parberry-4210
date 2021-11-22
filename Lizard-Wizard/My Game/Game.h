#ifndef GAME_H
#define GAME_H

#include "Component.h"
#include "Settings.h"
#include "CustomBind.h"
#include "Renderer.h"
#include "Math.h"
#include "Interpolation.h"
#include "Defines.h"
#include "Ecs.h"

#include <vector>
#include <set>

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

// Configuration Defines
#define X_ROOMS 40
#define Z_ROOMS 40

// Structs
struct RayProjectile {
    Vec3 Pos1;
    Vec3 Pos2;
    Vec4 Color;
};

struct SimProjectile {
    u32 Bounces;
    u32 MaxBounces;
    SoundIndex::e ProjSound;
    Vec4 Color;
};

struct Transform {
    Vec3 Position;
    Quat Rotation;
    Vec3 Scale;
};

struct NPC {
    NPCBehavior::e Behavior;
    NPCState::e State;
    u32 SearchAttempts;
    Vec3 QueuedMovement;
    Vec3 LastPosition;
    NPC() :
        Behavior(NPCBehavior::TURRET),
        State(NPCState::SLEEPING)
    {}
};

struct Animation {
    Vec3 beginPos;
    Vec3 beginRot;
    Vec3 endPos;
    Vec3 endRot;
    f32 time;
    f32 maxSteps;
    f32 steps;
    f32 percent;
};

struct Mana {
    i32 value;
    i32 max;
    f32 recharge;
    f32 timer;

    f32 Decrement(i32 count) {
        if (this->value - count < 0) { count = this->value; }
        this->value -= count;
        //printf("%f\n", this->recharge * ((f32)this->max - (f32)(this->value)));
        return this->recharge * ((f32)this->max - (f32)(this->value));
    }

    static Mana New(i32 max, f32 recharge) {
        Mana mana;
        mana.value = max;
        mana.max = max;
        mana.recharge = recharge;
        mana.timer = 0.0f;
    
        return mana;
    }
};

struct Health {
    i32 current;
    i32 max;

    static Health New(i32 current, i32 max) {
        Health health;
        health.current = current;
        health.max = max;

        return health;
    }
};

// MENU MANAGER
struct Panel {

    bool Hovering;
    bool Pressed = false;
    Vec3 Tint;
    u32 nextState;

};

// MENU MANAGER
struct PanelText {

    std::string Text;
    Vector2 position;
    Vector4 color;  

};

// Game Class
class CGame:
    public LComponent, 
    public LSettings
{ 
private:
    // Mouse Binds
    CustomBind m_leftClick;
    CustomBind m_rightClick;
    // Timing Table
    Table<f32> m_Timers;

    // Animation Table
    Table<Animation> m_Animations;

    // Menu Table
    Table<SpriteInstance> m_Menu;
    Table<Panel> m_Panel;
    Table<PanelText> m_PanelText;
    Group m_MainMenu;
    Group m_SettingsMenu;
    Group m_PauseMenu;
    bool m_DrawMainMenu = false;

    // Bullet3 Map / Tables
    std::unordered_map<btRigidBody*, Entity> m_RigidBodyMap;
    Table<btRigidBody*> m_RigidBodies;

    // Projectile Cache (MAX 64)
    Table<SimProjectile> m_Projectiles;
    std::unordered_set<Entity> m_DeadProjectiles;
    Group m_ProjectilesCache;
    Group m_ProjectilesActive;
    ProjectileTypes::e m_WeaponSelection = ProjectileTypes::FIRE;

    // Ray Cache (Max 24)
    Table<RayProjectile> m_Rays;
    Group m_RaysCache;
    Group m_RaysActive;

    // AI Cache (MAX 24)
    Table<NPC> m_NPCs;
    Group m_NPCsCache;
    Group m_NPCsActive;

    // Rendering Table
    Table<ModelInstance> m_ModelInstances;
    Group m_ModelsActive;

    // Parenting system
    Table<Entity> m_EntityMapping;

    // Health Table
    Table<Health> m_Healths;

    // Mana Table
    Table<Mana> m_Mana;

    // In-Air Table
    Group m_InAir;

    // Rooms / Generation Tables
    std::array<std::array<bool, Z_ROOMS>, X_ROOMS> m_GameMap; // empty or not empty, could be optimized with a bitfield

    // Collision Table (Simply put, we store all currently colliding objects here.)
    Table<Entity> m_CollisionPairs;
    Group m_CurrentCollisions;
    
    // Projectile Vector
    //std::vector<Projectile> m_currentProjectiles;
    //std::vector<RayProjectile> m_currentRayProjectiles;

    Table<Transform> m_Transforms;

    Table<Particle> m_Particles;
    Table<ParticleInstance> m_ParticleInstances;
    Group m_ParticleInstancesCache;
    Group m_ParticleInstancesActive;

    Group m_TestingLights;
    Group m_TestingWallsFloors;
    Group m_TestingModels;

    Group m_RoomLights;
    Group m_RoomWallsFloors;
    Group m_RoomModels;

    i32 randomSeed;

    bool m_bDrawFrameRate = false; ///< Draw the frame rate.
    f32 m_frameRate;
    bool m_bDrawHelpMessage = true; ///< Draw the help message.

    //TODO(sean): move these out of globals
    f32 player_yaw = 0.0f;
    f32 player_pitch = 0.0f;
    f32 player_sens = 0.0333f;
    u32 player_step_noise = 0; // this is for footstep sounds, lemme know when you change this. (ethan)
    f32 player_step = 0.0f;
    Vec3 player_pos_last_frame = Vec3(0);
    Vec3 staff_tip;

    Vector3 flycam_pos = { -10000.0f, 0.0f, -10000.0f };
    f32 flycam_speed = 6000.0f;
    
    bool flycam_enabled = false;
    u32 render_mode = 1;

    Action m_DashAction;
    Action m_JumpAction;

    bool m_MouseToggled = true;
    bool m_MouseJustToggled = false;

    // some player shit i dont care where this gets put
    Group m_PlayerManaOrbs;
    Group m_PlayerHealthOrbs;
    
    ////////////////////////
    // Internal Functions //
    ////////////////////////

    void LoadImages(); ///< Load images.
    void LoadModels(); ///< Load models.
    void LoadSounds(); ///< Load sounds.
    void BeginGame(); ///< Begin playing the game.
    void InputHandler(); ///< The keyboard handler.
    void RenderFrame(); ///< Render an animation frame.

    //////////////////////////////////////
    // Exterior Functions in load order //
    //////////////////////////////////////

    // PHYSICS MANAGER //
    u32 m_CurrentStep;

    btTransform NewTransform(btCollisionShape*, Vec3);
    btRigidBody* NewRigidBody(
        btCollisionShape*, 
        btTransform, 
        f32, 
        f32, 
        i32, 
        i32
    );
    btRigidBody* CreateSphereObject(
        btScalar, 
        Vec3, 
        f32, 
        f32, 
        i32, 
        i32
    );
    btRigidBody* CreateBoxObject(
        Vec3, 
        Vec3, 
        f32, 
        f32, 
        i32, 
        i32
    );
    btRigidBody* CreateCapsuleObject(
        btScalar, 
        btScalar, 
        Vec3, 
        f32, 
        f32, 
        i32, 
        i32
    );
    btRigidBody* CreateConvexObject(
        f32, 
        f32, 
        i32, 
        i32
    );
    static void PhysicsCollisionCallBack(btDynamicsWorld*, btScalar);
    void CustomPhysicsStep();
    void RemoveRigidBody(btRigidBody*);
    void AddRigidBody(btRigidBody*, i32, i32);
    void RBSetMassFriction(btRigidBody*, f32, f32);
    void RBSetCcd(btRigidBody*, f32, f32);
    void RBTeleportLaunch(btRigidBody*, Vec3, Vec3);
    void RBTeleport(btRigidBody*, Vec3);
    void DestroyPhysicsObject(btCollisionShape*);
    void InitializePhysics();

    // PLAYER MANAGER //
    void InitializePlayer();
    void PlayerInput();
    void UpdatePlayer();
    void RenderPlayer();

    // UI MANAGER //

    // GENERATION MANAGER //
    Vec3 m_roomCenter;
    void GenerateRooms(Vec3, const i32, const i32, const i32);
    void DestroyRooms();
    void InitializeGeneration();
    Vec3 IndexToWorld(u32 x, u32 z);
    bool CheckBounds(i32 x, i32 z);
    Point2 FindClosestPoint(Point2, Point2);
    std::vector<Point2> Pathfind(Point2, Point2);
    std::unordered_set<u64> FindConnected(Point2 start);

    // PROJECTILE MANAGER //
    void GenerateSimProjectile(
        btCollisionObject*,
        const Vec3,
        const Vec3,
        const i32,
        const f32,
        const f32,
        const Vec4,
        const SoundIndex::e,
        const b8
    );

    void GenerateRayProjectile(
        btCollisionObject*,
        const Vec3,
        const Vec3,
        const i32,
        const i32,
        const f32,
        const Vec4,
        const b8,
        const b8
    );

    void CalculateRay(
        btCollisionObject*, 
        RayProjectile&, 
        Vec3, 
        Vec3, 
        i32, 
        Vec4, 
        b8
    );

    void InitializeProjectiles();
    void StripProjectile(Entity);
    void StripRay(Entity);

    // NPC MANAGER //
    void Animate(Entity);
    void Sleep(Entity);
    void Wander(Entity);
    void Move(Entity, Vec3);
    void Pathfind(Entity);
    void Attack(Entity);
    void Search(Entity);
    void DirectNPC(Entity);
    void PlaceNPC(Vec3, Vec3);
    void InitializeNPCs();
    void StripNPC();

    // ANIMATION MANAGER //

    // MENU MANAGER //
    void InitializeMenu();
    void InputMenu();
    void RenderMenu();

    // TESTING ROOM //
    void CreateTestingEnvironment();

    // UPDATE //
    void EcsPreUpdate();
    void Update();
    void EcsUpdate();

    // DRAW //

    void DrawDebugModelsOnRB();

    // PARTICLES //
    void InitializeParticles();
    void SpawnParticles(ParticleInstanceDesc* desc);
    void StripParticle(Entity e);
    void StripParticleInstance(Entity e);

    void ResetGame();
    bool m_reset = false;

public:
    Renderer* m_pRenderer;
    Entity m_Player;

    // Bullet3 Declarations
    btAlignedObjectArray<btCollisionShape*> m_pCollisionShapes;
    btDiscreteDynamicsWorld* m_pDynamicsWorld;
    std::vector<RayProjectile> m_currentRayProjectiles;


    ~CGame();
    void Initialize();
    void ProcessFrame();
    void Release();
};

#endif
