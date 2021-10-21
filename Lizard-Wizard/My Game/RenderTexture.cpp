#include "RenderTexture.h"

void CheckDeviceFormatSupport(ID3D12Device* device, DXGI_FORMAT format) {
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
        ABORT_EQ_FORMAT(0, 0, "Check Feature Support");
    }

    u32 required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
    if ((formatSupport.Support1 & required) != required) {
        ABORT_EQ_FORMAT(0, 0, "Format not supported as a Render Texture");
    }
}

RenderTexture::RenderTexture() :
    m_state(D3D12_RESOURCE_STATE_COMMON),
    m_srvDescriptor{},
    m_rtvDescriptor{},
    m_format(DXGI_FORMAT_UNKNOWN),
    m_width(0),
    m_height(0),
    m_clearColor(Colors::Black)
{}

RenderTexture::RenderTexture(DXGI_FORMAT format, Vec4 clear_color) :
    m_state(D3D12_RESOURCE_STATE_COMMON),
    m_srvDescriptor{},
    m_rtvDescriptor{},
    m_format(format),
    m_width(0),
    m_height(0),
    m_clearColor(clear_color)
{}

void RenderTexture::Init(
    ID3D12Device* device,
    D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor,
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor,
    usize width,
    usize height
) {
    CheckDeviceFormatSupport(device, m_format);

    if (!srvDescriptor.ptr || !rtvDescriptor.ptr) {
        ABORT_EQ_FORMAT(0, 0, "Invalid descriptors");
    }

    this->m_srvDescriptor = srvDescriptor;
    this->m_rtvDescriptor = rtvDescriptor;

    if (this->m_width == width && this->m_height == height) {
        return;
    }

    auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(
        m_format,
        (u64)width,
        (u32)height,
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );

    D3D12_CLEAR_VALUE clear_value = { m_format, {} };
    memcpy(clear_value.Color, &m_clearColor, sizeof(clear_value.Color));

    m_state = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create the render target
    ThrowIfFailed(
        device->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &resource_desc,
            m_state,
            &clear_value,
            IID_GRAPHICS_PPV_ARGS(m_resource.ReleaseAndGetAddressOf())
        )
    );

    // Create RTV
    device->CreateRenderTargetView(m_resource.Get(), 0, m_rtvDescriptor);

    // Create SRV
    device->CreateShaderResourceView(m_resource.Get(), 0, m_srvDescriptor);

    this->m_width = width;
    this->m_height = height;
}

void RenderTexture::TransitionTo(
    ID3D12GraphicsCommandList* command_list,
    D3D12_RESOURCE_STATES after_state
) {
    TransitionResource(command_list, m_resource.Get(), m_state, after_state);
    m_state = after_state;
}
