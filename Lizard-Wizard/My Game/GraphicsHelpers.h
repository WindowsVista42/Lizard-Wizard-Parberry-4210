#include "Defines.h"

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
struct VBOData {
    u32 vertex_count;
    u32 index_count;
    VertexPNT* vertices;
    u16* indices;

    void free() {
        delete[] vertices;
        delete[] indices;
    }
};

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
void LoadVBO(const char* fpath, VBOData* model) {
    #define CHECK(value) ABORT_EQ_FORMAT((value), 0, "Corrupt VBO file!")

    FILE* fp = fopen(fpath, "rb"); CHECK(fp);

    CHECK(fread(&model->vertex_count, sizeof(u32), 1, fp));
    CHECK(fread(&model->index_count, sizeof(u32), 1, fp));

    model->vertices = new VertexPNT[model->vertex_count]; CHECK(model->vertices); 
    model->indices = new u16[model->index_count]; CHECK(model->indices);

    CHECK(fread(model->vertices, sizeof(VertexPNT), model->vertex_count, fp));
    CHECK(fread(model->indices, sizeof(u16), model->index_count, fp));

    fclose(fp);
}

void CreateRenderTextureHeaps(
    ID3D12Device* device,
    std::unique_ptr<DescriptorHeap>* resource_descs,
    std::unique_ptr<DescriptorHeap>* render_descs,
    usize count
) {
   *resource_descs = std::make_unique<DescriptorHeap>(
       device, count
   );

   *render_descs = std::make_unique<DescriptorHeap>(
       device,
       D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
       D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
       count
   );
}

template <typename T>
void CreateBufferAndView(T* data, usize count, GraphicsResource& resource, std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW>& view) {
    isize size = sizeof(T) * count;

    resource = GraphicsMemory::Get().Allocate(size);
    memcpy(resource.Memory(), data, size); // i like this function

    view = std::make_shared<D3D12_VERTEX_BUFFER_VIEW>();
    view->BufferLocation = resource.GpuAddress();
    view->StrideInBytes = sizeof(T);
    view->SizeInBytes = (u32)resource.Size();
}

template <typename T>
void CreateBufferAndView(T* data, usize count, GraphicsResource& resource, std::shared_ptr<D3D12_INDEX_BUFFER_VIEW>& view) {
    isize size = sizeof(T) * count;

    resource = GraphicsMemory::Get().Allocate(size);
    memcpy(resource.Memory(), data, size);
    
    view = std::make_shared<D3D12_INDEX_BUFFER_VIEW>();
    view->BufferLocation = resource.GpuAddress();
    view->SizeInBytes = (u32)resource.Size();
    view->Format = DXGI_FORMAT_R32_UINT;
}

