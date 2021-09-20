/// \file Descriptors.h
/// \brief Interface for descriptors.

#ifndef __L4RC_GAME_DESCRIPTORS_H__
#define __L4RC_GAME_DESCRIPTORS_H__

#include "GameDefines.h"
#include "Renderer3D.h"
#include "Common.h"

/// \brief Model descriptor.
///
/// Describes various properties of a 3D model using the `Model` type from
/// DirectXTK12. This descriptor uses a `unique_ptr` to a `Model` and
/// `shared_ptr`s to `IEffect`s because DirectXTK12 insists that we use managed
/// pointers, like it or not.

class CModelDesc{
  public:
    eModel m_eModelType = eModel::Unknown; ///< Model type.
    std::unique_ptr<Model> m_pModel; ///< Pointer to model.
    Vector3 m_vPos; ///< Position of center in Object Space.
    Vector3 m_vExtents; ///< Extents of model in Render World coordinates.
    std::vector<std::shared_ptr<IEffect>> m_Effects; ///< Model effects.
    std::unique_ptr<EffectTextureFactory> m_pResources; ///< Pointer to model resources.
}; //CModelDesc

/// \brief Mesh descriptor.
///
/// Describes various properties of the mesh for a geometric primitive from
/// DirectXTK12. This descriptor uses a `unique_ptr` to a `GeometricPrimitive`
/// because DirectXTK12 insists that we use managed pointers, like it or not.

class CMeshDesc{
  public:
    eMesh m_eMeshType = eMesh::Unknown; ///< Mesh type.
    UINT m_nTriangulationLevel = 1; ///< Triangulation level.

    std::unique_ptr<GeometricPrimitive> m_pPrimitive = nullptr;  ///< Pointer to primitive.
    std::vector<VertexPositionNormalTexture> m_vecVertexBuffer; ///< Copy of vertex buffer.
    std::vector<uint16_t> m_vecIndexBuffer; ///< Copy of index buffer.
    BasicEffect* m_pEffect = nullptr; ///< Effect pointer.

    Vector3 m_vExtents; ///< Extents of primitive in Render World coordinates.
    float m_fRadius = 0.0f; ///< Radius in Render World coordinates.
    float m_fScale = 1.0f; ///< Scale.

    CMeshDesc(const eMesh); ///< Constructor.
    ~CMeshDesc(); ///< Destructor.
}; //CMeshDesc

/// \brief Instance descriptor.
///
/// Describes properties of a mesh or model instance. There can be many
/// instances of a mesh or model in a game, and these properties are the ones
/// that can be varied from instance to instance.

class CInstanceDesc{
  public:
    eMesh m_eMeshType = eMesh::Unknown; ///< Mesh type.
    eModel m_eModelType = eModel::Unknown; ///< Model type.

    Vector3 m_vPos; ///< Position.
    Quaternion m_qOrientation; ///< Orientation.
    float m_fScale = 1.0f; ///< Scale.
    Vector3 m_vExtents; ///< Extents of primitive in Render World coordinates.

    float m_fMass = 0.0f; ///< Mass.
    float m_fRestitution = 0.5f; ///< Restitution.
    float m_fFriction = 0.5f; ///< Friction.
}; //CMeshInstanceDesc

/// \brief Contact descriptor.
///
/// Describes some of the properties that Bullet Physics will tell us about
/// contacts, that is, the number of contact pointss (which can be between 0
/// and 4) and a magnitude for the impulse experienced by this contact. Bullet
/// Physics does in fact give us more information than this, but this is what's
/// needed for now to generate collision sounds.

class CContactDesc{
  public:
    UINT m_nNumContacts = 0 ; ///< Number of contact points.
    float m_fImpulse = 0.0f; ///< Impulse magnitude.

    CContactDesc(); ///< Default constructor.
    CContactDesc(const UINT, const float=0.0f); ///< Constructor.
}; //CContactDesc

#endif //__L4RC_GAME_DESCRIPTORS_H__