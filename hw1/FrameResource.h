#pragma once

struct FrameResource {

	void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<IDXGISwapChain> pdxgiSwapChain, UINT nRTVIndex);
	void ResourceBarrier(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, D3D12_RESOURCE_STATES d3dStateAfter);
private:
	ComPtr<ID3D12Resource> pd3dRenderTargetBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRTVHandle;
	D3D12_RESOURCE_STATES d3dCurrentState;
	UINT nFenceValue = 0;

	static ComPtr<ID3D12Resource> pd3dDepthStencilBuffer;
	static ComPtr<ID3D12DescriptorHeap> pd3dRTVDescriptorHeap;
	static ComPtr<ID3D12DescriptorHeap> pd3dDSVDescriptorHeap;
	static UINT nRTVDescriptorIncrementSize;
	static UINT nDSVDescriptorIncrementSize;

	friend class GameFramework;

private:
	void CreateRTVandDSVDescriptorHeap(ComPtr<ID3D12Device> pd3dDevice);
		
};

