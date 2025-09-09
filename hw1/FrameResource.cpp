#include "stdafx.h"
#include "FrameResource.h"
#include "GameFramework.h"

ComPtr<ID3D12Resource> FrameResource::pd3dDepthStencilBuffer = nullptr;
ComPtr<ID3D12DescriptorHeap> FrameResource::pd3dRTVDescriptorHeap = nullptr;
ComPtr<ID3D12DescriptorHeap> FrameResource::pd3dDSVDescriptorHeap = nullptr;
UINT FrameResource::nRTVDescriptorIncrementSize = 0;
UINT FrameResource::nDSVDescriptorIncrementSize = 0;

void FrameResource::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<IDXGISwapChain> pdxgiSwapChain, UINT nRTVIndex) 
{
	if (!pd3dRTVDescriptorHeap) {
		CreateRTVandDSVDescriptorHeap(pd3dDevice);
	}

	HRESULT hr;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRTVCPUDescriptorHandle = pd3dRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRTVCPUDescriptorHandle.ptr += nRTVDescriptorIncrementSize * (nRTVIndex);

	hr = pdxgiSwapChain->GetBuffer(nRTVIndex, IID_PPV_ARGS(pd3dRenderTargetBuffer.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	pd3dDevice->CreateRenderTargetView(pd3dRenderTargetBuffer.Get(), NULL, d3dRTVCPUDescriptorHandle);
	d3dRTVHandle = d3dRTVCPUDescriptorHandle;

	d3dCurrentState = D3D12_RESOURCE_STATE_COMMON;
}

void FrameResource::ResourceBarrier(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, D3D12_RESOURCE_STATES d3dStateAfter)
{
	assert(d3dCurrentState != d3dStateAfter);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	{
		d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		d3dResourceBarrier.Transition.pResource = pd3dRenderTargetBuffer.Get();
		d3dResourceBarrier.Transition.StateBefore = d3dCurrentState;
		d3dResourceBarrier.Transition.StateAfter = d3dStateAfter;
		d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	d3dCurrentState = d3dStateAfter;
}

void FrameResource::CreateRTVandDSVDescriptorHeap(ComPtr<ID3D12Device> pd3dDevice)
{
	HRESULT hr{};

	// RTV Heap
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc{};
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	{
		d3dDescriptorHeapDesc.NumDescriptors = GameFramework::g_nSwapChainBuffers;
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		d3dDescriptorHeapDesc.NodeMask = 0;
	}
	hr = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(pd3dRTVDescriptorHeap.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	// DSV Heap
	{
		d3dDescriptorHeapDesc.NumDescriptors = 1;
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	}
	hr = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(pd3dDSVDescriptorHeap.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}


	// DSV Resource
	D3D12_RESOURCE_DESC d3dResourceDesc{};
	{
		d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3dResourceDesc.Alignment = 0;
		d3dResourceDesc.Width = GameFramework::g_uiClientWidth;
		d3dResourceDesc.Height = GameFramework::g_uiClientHeight;
		d3dResourceDesc.DepthOrArraySize = 1;
		d3dResourceDesc.MipLevels = 1;
		d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dResourceDesc.SampleDesc.Count = (GameFramework::g_bMsaa4xEnable) ? 4 : 1;
		d3dResourceDesc.SampleDesc.Quality = (GameFramework::g_bMsaa4xEnable) ? (GameFramework::g_nMsaa4xQualityLevels - 1) : 0;
		d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	{
		d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		d3dHeapProperties.CreationNodeMask = 1;
		d3dHeapProperties.VisibleNodeMask = 1;
	}

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.f;
	d3dClearValue.DepthStencil.Stencil = 0;

	// Create Depth Stencil Buffer
	hr = pd3dDevice->CreateCommittedResource(
		&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&d3dClearValue,
		IID_PPV_ARGS(pd3dDepthStencilBuffer.GetAddressOf())
	);

	// Create Depth Stencil Buffer View
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDSVCPUDescriptorHandle = pd3dDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pd3dDevice->CreateDepthStencilView(pd3dDepthStencilBuffer.Get(), NULL, d3dDSVCPUDescriptorHandle);

	nDSVDescriptorIncrementSize = pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	nRTVDescriptorIncrementSize = pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}
