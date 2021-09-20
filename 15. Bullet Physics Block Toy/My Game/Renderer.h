/// \file Renderer.h 
/// \brief Interface for the renderer class CRenderer.

#ifndef __L4RC_GAME_RENDERER_H__
#define __L4RC_GAME_RENDERER_H__

#include "GameDefines.h"
#include "Renderer3D.h"
#include "SpriteRenderer.h"
#include "Common.h"
#include "Object.h"
#include "Descriptors.h"

#include<map>

/// \brief The renderer.
///
/// CRenderer handles the game-specific rendering tasks, relying on
/// the base class to do all of the actual API-specific rendering.

class CRenderer: public LRenderer3D, public CCommon{
  private:
    CMeshDesc** m_pPrimitive; ///< Array of primitive descriptors.
    LTextureDesc* m_pTextureDesc = nullptr; ///< Array of texture descriptors.
    size_t m_nNumTextures = 0; ///< Number of textures.

    std::map<eModel, CModelDesc*> m_mapModelDesc; ///< Model descriptor map.

    void BeginResourceUpload(); ///< Begin resource upload.
    void CRenderer::EndResourceUpload(); ///< End resource upload.

    void UploadTexture(char*, eMesh); ///< Upload texture to GPU.
    void UploadDefaultEffect(CMeshDesc*); ///< Upload default effect to GPU.

    void UploadSphereMesh(CMeshDesc*); ///< Upload sphere mesh to GPU.
    void UploadBoxMesh(CMeshDesc*); ///< Upload box mesh to GPU.

    void UploadPrimitiveMesh(CMeshDesc*); ///< Upload geometric primitive mesh to GPU.
    
  public:
    CRenderer(); ///< Constructor.
    ~CRenderer(); ///< Destructor.

    void Initialize(); ///< Initialize.
    void LoadGeometricPrimitives(); ///< Load meshes.
    void LoadModel(eModel, const std::wstring&); ///< Load model from file.

    void Render(CObject*); ///< Render an object.

    LBaseCamera* GetCameraPtr() const; ///< Get the camera pointer.
    ID3D12Device* GetD3DDevice() const; ///< Get the D3D device pointer.
    const Vector3& GetExtents(const eModel) const; ///< Get model extents.
    CMeshDesc* GetPrimitiveDesc(const eMesh) const; ///< Get mesh descriptor.
}; //CRenderer

#endif //__L4RC_GAME_RENDERER_H__
