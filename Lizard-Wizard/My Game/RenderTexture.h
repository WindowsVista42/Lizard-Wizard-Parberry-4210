#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include "Defines.h"
#include "Abort.h"

//NOTE(sean): Rendering internals, ask me about it if you have questions.
//This is very similar to https://github.com/microsoft/DirectXTK12/wiki/RenderTexture

/// A basic class that lets you create a texture than can be rendered to.
/// Be aware that the resource should be in the right state before using it.
/// Ask me any questions you might have.
struct RenderTexture {
    RenderTexture();
    RenderTexture(DXGI_FORMAT format, Vec4 clear_color);

    void Init(
        ID3D12Device* device, 
        D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor, 
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor,
		usize width,
		usize height
    );

    void TransitionTo(ID3D12GraphicsCommandList* command_list, D3D12_RESOURCE_STATES after_state);

    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_state;
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDescriptor;
    DXGI_FORMAT m_format;
    Vec4 m_clearColor;
    usize m_width;
    usize m_height;
};

#endif