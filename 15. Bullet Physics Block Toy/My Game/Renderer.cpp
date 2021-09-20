/// \file Renderer.cpp
/// \brief Code for the renderer class CRenderer.

#include "Renderer.h"
#include "ComponentIncludes.h"

#include "Abort.h"

/// Constructor.

CRenderer::CRenderer(): LRenderer3D(){
  m_pTextureDesc = new LTextureDesc[32];
  m_pCamera = new LBaseCamera;

  m_pPrimitive = new CMeshDesc*[(UINT)eMesh::Size]; 
  for(size_t i=0; i<(size_t)eMesh::Size; i++)
    m_pPrimitive[i] = nullptr;
} //constructor

/// Destructor.

CRenderer::~CRenderer(){
  delete m_pCamera;

  delete m_pTextureDesc;

  for(auto& p: m_mapModelDesc)
    delete p.second;

  for(size_t i=0; i<(size_t)eMesh::Size; i++)
    delete m_pPrimitive[i];
  delete [] m_pPrimitive;

  if(m_pDeviceResources)
    m_pDeviceResources->WaitForGpu();
} //destructor

///////////////////////////////////////////////////////////////////////////////

/// Initialize the renderer, set the background color, and create a
/// perspective camera.

void CRenderer::Initialize(){
  SetBgColor(Colors::PowderBlue);
  LRenderer3D::Initialize();

  const float fAspect = (float)m_nWinWidth/m_nWinHeight; //aspect ratio
  m_pCamera->SetPerspective(fAspect, XM_PI/4.0f, 1.0f, 1000.0f); 
} //Initialize

/// Begin resource upload.

void CRenderer::BeginResourceUpload(){
  m_pResourceUpload->Begin();
} //BeginResourceUpload

/// Wait for the upload thread to terminate.

void CRenderer::EndResourceUpload(){  
  auto q = m_pDeviceResources->GetCommandQueue(); //command queue
  m_pResourceUpload->End(q).wait(); //wait for upload thread to terminate
} //EndResourceUpload

/// Upload texture to GPU.
/// \param name Name string found in gamesettings.xml.
/// \param t Texture type.

void CRenderer::UploadTexture(char* name, eMesh t){
  LoadTexture(name, m_pTextureDesc[(UINT)t]);
} //UploadTexture

/// Upload default effect to GPU.
/// \param pDesc Pointer to geometric primitive descriptor.

void CRenderer::UploadDefaultEffect(CMeshDesc* pDesc){
  EffectPipelineStateDescription psd(
	  &GeometricPrimitive::VertexType::InputLayout,
    CommonStates::NonPremultiplied, 
    CommonStates::DepthDefault, 
    CommonStates::CullNone, 
    m_RenderTargetState);

  BasicEffect* pEffect = new BasicEffect(m_pD3DDevice,
    EffectFlags::PerPixelLighting | EffectFlags::Texture, psd);
  pEffect->SetProjection(XMLoadFloat4x4(&m_projection));

  LTextureDesc desc = m_pTextureDesc[(UINT)pDesc->m_eMeshType];
          
  pEffect->SetTexture(
    m_pDescriptorHeap->GetGpuHandle(desc.m_nResourceDescIndex), 
    m_pStates->AnisotropicWrap()); 

  pEffect->EnableDefaultLighting();

  pEffect->SetLightEnabled(0, true);
  pEffect->SetLightEnabled(1, true);
  pEffect->SetLightEnabled(2, true);

  pEffect->SetAmbientLightColor({0.4f, 0.4f, 0.4f, 1.0f});

  pDesc->m_pEffect = pEffect;
} //UploadDefaultEffect

/// Upload sphere mesh to GPU. The sphere gets its own function because
/// it has its own level of triangulation, which will be larger for
/// large spheres.
/// \param pDesc Pointer to geometric primitive descriptor.

void CRenderer::UploadSphereMesh(CMeshDesc* pDesc){
  pDesc->m_pPrimitive = GeometricPrimitive::CreateSphere(
    2.0f*pDesc->m_fRadius, pDesc->m_nTriangulationLevel);
  m_pPrimitive[(UINT)pDesc->m_eMeshType] = pDesc;
} //UploadSphereMesh

/// Upload box mesh to GPU. This gets its own function because we will
/// be repeating the texture across each face.
/// \param pDesc Pointer to geometric primitive descriptor.

void CRenderer::UploadBoxMesh(CMeshDesc* pDesc){
  GeometricPrimitive::CreateBox(
    pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer, pDesc->m_vExtents);
  
  for(auto& v: pDesc->m_vecVertexBuffer){ //modify texture coordinates
    v.textureCoordinate.x *= pDesc->m_nTriangulationLevel;
    v.textureCoordinate.y *= pDesc->m_nTriangulationLevel;
  } //for

  pDesc->m_pPrimitive = GeometricPrimitive::CreateCustom(
    pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer);
  
  m_pPrimitive[(UINT)pDesc->m_eMeshType] = pDesc;
} //UploadBoxMesh

/// General function for uploading a geometric primitive mesh. Creates
/// a new primitive mesh descriptor and fills it with mesh information
/// including a local copy of vertices, and stores a pointer to it in 
/// m_vecPrimitives[t] for later use, for example, in physics calculations
/// such as collision detection and response. 
/// \param pDesc Pointer to geometric primitive descriptor.

void CRenderer::UploadPrimitiveMesh(CMeshDesc* pDesc){
  switch(pDesc->m_eMeshType){
    case eMesh::Icosahedron: GeometricPrimitive::CreateIcosahedron(
      pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer, pDesc->m_fScale);
      break;
      
    case eMesh::Tetrahedron: GeometricPrimitive::CreateTetrahedron(
      pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer, pDesc->m_fScale);
      break;
      
    case eMesh::Dodecahedron: GeometricPrimitive::CreateDodecahedron(
      pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer, pDesc->m_fScale);
      break;

    case eMesh::Teapot: GeometricPrimitive::CreateTeapot(
      pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer, pDesc->m_fScale, 3);
      break;
  } //switch

  //create the primitive from the vertex and index buffers
  
  pDesc->m_pPrimitive = GeometricPrimitive::CreateCustom(
    pDesc->m_vecVertexBuffer, pDesc->m_vecIndexBuffer);
  
  m_pPrimitive[(UINT)pDesc->m_eMeshType] = pDesc;
} //UploadPrimitiveMesh

/// Load meshes, textures, and effects for geometric primitives.
    
void CRenderer::LoadGeometricPrimitives(){ 
  CMeshDesc* pDesc = nullptr;

  BeginResourceUpload();
  
  pDesc = new CMeshDesc(eMesh::BigSphere);
  pDesc->m_fRadius = m_fMoonRadius;
  pDesc->m_nTriangulationLevel = 128;
  UploadTexture("moon", eMesh::BigSphere);
  UploadDefaultEffect(pDesc);
  UploadSphereMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Sphere);
  pDesc->m_fRadius = m_fBallRadius;
  pDesc->m_nTriangulationLevel = 32;
  UploadTexture("ball", eMesh::Sphere);
  UploadDefaultEffect(pDesc);
  UploadSphereMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Plane);
  pDesc->m_vExtents = m_vPlaneSize;
  pDesc->m_nTriangulationLevel = 16;
  UploadTexture("plane", eMesh::Plane);
  UploadDefaultEffect(pDesc);
  UploadBoxMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Box);
  pDesc->m_vExtents = m_vBoxSize;
  pDesc->m_nTriangulationLevel = 1;
  UploadTexture("wood", eMesh::Box);
  UploadDefaultEffect(pDesc);
  UploadBoxMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Tetrahedron);
  pDesc->m_fScale = m_fTetrahedronSize;
  UploadTexture("metal", eMesh::Tetrahedron);
  UploadDefaultEffect(pDesc);
  UploadPrimitiveMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Teapot);
  pDesc->m_fScale = m_fTeapotSize;
  UploadTexture("teapot", eMesh::Teapot);
  UploadDefaultEffect(pDesc);
  UploadPrimitiveMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Icosahedron);
  pDesc->m_fScale = m_fIcosahedronSize;
  UploadTexture("granite", eMesh::Icosahedron);
  UploadDefaultEffect(pDesc);
  UploadPrimitiveMesh(pDesc);
  
  pDesc = new CMeshDesc(eMesh::Dodecahedron);
  pDesc->m_fScale = m_fDodecahedronSize;
  UploadTexture("camo", eMesh::Dodecahedron);
  UploadDefaultEffect(pDesc);
  UploadPrimitiveMesh(pDesc);

  EndResourceUpload();
} //LoadGeometricPrimitives

/// Load a model mesh and texture from a file to the GPU.
/// \param t Model type.
/// \param strPath Wide string containing path and file name.

void CRenderer::LoadModel(eModel t, const std::wstring& strPath){
  size_t n = strPath.find_last_of('\\');
  std::wstring strFolder = strPath.substr(0, n);

  CModelDesc* pModelDesc = new CModelDesc;
  pModelDesc->m_eModelType = t;

  pModelDesc->m_pModel = Model::CreateFromSDKMESH(m_pD3DDevice, strPath.c_str());
  m_pResourceUpload->Begin();
  pModelDesc->m_pResources = pModelDesc->m_pModel->
    LoadTextures(m_pD3DDevice, *m_pResourceUpload, strFolder.c_str());

  EffectPipelineStateDescription psd(
    nullptr, CommonStates::Opaque, CommonStates::DepthDefault,  
    /*CommonStates::Wireframe,*/ CommonStates::CullNone,  m_RenderTargetState);

  EffectPipelineStateDescription alphapsd(nullptr, CommonStates::NonPremultiplied, 
    CommonStates::DepthRead, CommonStates::CullNone, m_RenderTargetState);

  pModelDesc->m_Effects = pModelDesc->m_pModel-> 
    CreateEffects(psd, alphapsd, pModelDesc->m_pResources->Heap(), 
    m_pStates->Heap());
    
  for(auto effect: pModelDesc->m_Effects){
    auto lights = dynamic_cast<IEffectLights*>(effect.get());
    if(lights){
      lights->SetLightEnabled(0, true);
      lights->SetLightEnabled(1, true);
      lights->SetLightEnabled(2, true);
      lights->SetAmbientLightColor({0.4f, 0.4f, 0.4f, 1.0f});
    } //if
  } //for
  
  auto finished = m_pResourceUpload->End(m_pDeviceResources->GetCommandQueue());
  finished.wait(); //wait for the upload thread to terminate

  //get bounding box
  
  auto& pMeshes = pModelDesc->m_pModel->meshes;
  BoundingBox b = pMeshes[0]->boundingBox; 

  for(size_t i=1; i<pMeshes.size(); i++)
    BoundingBox::CreateMerged(b, b, pMeshes[i]->boundingBox);

  pModelDesc->m_vExtents = Vector3(b.Extents); 
  pModelDesc->m_vPos = Vector3(b.Center);

  m_mapModelDesc.insert(std::make_pair(t, pModelDesc));
} //LoadModel

/// Render an object.
/// \param pObj Pointer to the object to be rendered. May be nullptr.

void CRenderer::Render(CObject* pObj){
  if(pObj == nullptr)return;

  const eMesh t = pObj->m_eMeshType;

  if(t == eMesh::Model){ //model
    auto it = m_mapModelDesc.find(pObj->m_eModelType);
    if(it != m_mapModelDesc.end()){
      CModelDesc* pModelDesc = it->second;
      const float s = pObj->m_fScale;
      const Vector3 dv = pObj->GetPosition();
      const float dy = it->second->m_vExtents.y;
      const XMVECTORF32 translate = {dv.x, dv.y, dv.z};
      const XMVECTOR rotate = pObj->GetOrientation();
      const XMVECTORF32 scale = {s, s, s};
  
      XMMATRIX local = XMMatrixReflect(Vector3::UnitX) *
        XMMatrixTranslationFromVector(Vector3(0, -dy, 0)) * 
        XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero,
        rotate, translate);
      XMMATRIX view = XMLoadFloat4x4(&m_view);
      XMMATRIX project = XMLoadFloat4x4(&m_projection);

      Model::UpdateEffectMatrices(pModelDesc->m_Effects, local, view, project);

      m_pHeaps[0] = pModelDesc->m_pResources->Heap();
      m_pCommandList->SetDescriptorHeaps(_countof(m_pHeaps), m_pHeaps);

      pModelDesc->m_pModel->Draw(m_pCommandList, pModelDesc->m_Effects.begin());
      
      m_pHeaps[0] = m_pDescriptorHeap->Heap();
      m_pHeaps[1] = m_pStates->Heap();
      m_pCommandList->SetDescriptorHeaps(_countof(m_pHeaps), m_pHeaps);
    } //if
  } //if

  else{ //shape
    CMeshDesc* pDesc = m_pPrimitive[(UINT)t];
    const XMMATRIX translate = XMMatrixTranslationFromVector(pObj->GetPosition());
    const XMMATRIX rot = XMMatrixRotationQuaternion(pObj->GetOrientation());
    auto pEffect = pDesc->m_pEffect; 

    pEffect->SetWorld(rot*translate);
    pEffect->SetView(XMLoadFloat4x4(&m_view));
    pEffect->Apply(m_pCommandList);
    
    //m_pHeaps[0] = m_pDescriptorHeap->Heap();
    //m_pHeaps[1] = m_pStates->Heap();
    //m_pCommandList->SetDescriptorHeaps(_countof(m_pHeaps), m_pHeaps);

    pDesc->m_pPrimitive->Draw(m_pCommandList);
  } //shape
} //Render

///////////////////////////////////////////////////////////////////////////////
// Reader functions.

/// Reader function for the camera pointer.
/// \return The camera pointer.

LBaseCamera* CRenderer::GetCameraPtr() const{
  return m_pCamera;
} //GetCameraPtr

/// Reader function for the D3D device pointer.
/// \return the D3D device pointer.

ID3D12Device* CRenderer::GetD3DDevice() const{
  return m_pD3DDevice;
} //GetD3DDevice

/// Reader function for the model extents (half width, height, and depth).
/// \param t Model type.
/// \return Const reference to the bounding box extents of a model of type t.

const Vector3& CRenderer::GetExtents(const eModel t) const{
  auto it = m_mapModelDesc.find(t);
  if(it == m_mapModelDesc.end())return Vector3::Zero;
  return it->second->m_vExtents;
} //GetExtents

/// Get a pointer to the geometric primitive descriptor for a mesh type.
/// \param t Mesh type.
/// \return Pointer to the geometric primitive descriptor for the mesh type.

CMeshDesc* CRenderer::GetPrimitiveDesc(const eMesh t) const{
  return m_pPrimitive[(UINT)t]; 
} //GetPrimitiveDesc
