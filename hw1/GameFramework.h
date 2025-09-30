#pragma once
#include "GameTimer.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "RenderManager.h"
//
class Scene;

class GameFramework {
public:
	GameFramework(HINSTANCE hInstance, HWND hWnd, UINT uiWidth, UINT uiHeight, bool bEnableDebugLayer);
	~GameFramework();

public:
	void BuildObjects();
	void ProcessInput();
	void Update();
	void Render();

private:
	void RenderBegin();
	void RenderEnd();
	void Present();
	void MoveToNextFrame();

public:
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

public:
	GameTimer				m_GameTimer{};
	std::shared_ptr<Scene>	m_pScene = nullptr;

	POINT					m_ptOldCursorPos;
	TSTRING					m_tstrFrameRate;

	static std::unique_ptr<ResourceManager> g_pResourceManager;
	static std::unique_ptr<RenderManager> g_pRenderManager;

#pragma region D3D
private:
	void CreateFactory();
	void CreateDevice();
	void CreateFence();
	void CreateSwapChain();
	void CreateCommandList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();



private:
	void WaitForGPUComplete();

private:
	void ChangeSwapChainState();

public:
	static bool g_bMsaa4xEnable;
	static UINT g_nMsaa4xQualityLevels;

	static long g_nClientWidth;
	static long g_nClientHeight;

	static UINT g_uiDescriptorHandleIncrementSize;

	const static UINT g_nSwapChainBuffers = 2;

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// DXGI
	ComPtr<IDXGIFactory4>	m_pdxgiFactory;
	ComPtr<IDXGISwapChain3>	m_pdxgiSwapChain;
	UINT					m_nSwapChainBufferIndex = 0;

	// Device
	ComPtr<ID3D12Device>				m_pd3dDevice;

	// RTV + DSV
	//FrameResource m_FrameResources[g_nSwapChainBuffers];
	static const UINT			m_nSwapChainBuffers = 2;

	ComPtr<ID3D12Resource>				m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap>		m_pd3dRtvDescriptorHeap = NULL;
	UINT								m_nRtvDescriptorIncrementSize;

	ComPtr<ID3D12Resource>				m_pd3dDepthStencilBuffer = NULL;
	ComPtr<ID3D12DescriptorHeap>		m_pd3dDsvDescriptorHeap = NULL;
	UINT								m_nDsvDescriptorIncrementSize;

	// Command List + Allocator
	ComPtr<ID3D12CommandQueue>			m_pd3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pd3dCommandList;
	ComPtr<ID3D12CommandAllocator>		m_pd3dCommandAllocator;

	// Fence
	ComPtr<ID3D12Fence>			m_pd3dFence;
	HANDLE						m_hFenceEvent;

	D3D12_VIEWPORT				m_d3dViewport;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	D3D12_RECT					m_d3dScissorRect;

	bool m_bEnableDebugLayer = false;
#pragma endregion
};

#define RESOURCE GameFramework::g_pResourceManager
#define RENDER GameFramework::g_pRenderManager