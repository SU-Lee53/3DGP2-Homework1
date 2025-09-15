#pragma once
#include "GameTimer.h"
#include "FrameResource.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "RenderManager.h"

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

class Scene;

class GameFramework {
public:
	GameFramework(HINSTANCE hInstance, HWND hWnd, UINT uiWidth, UINT uiHeight, bool bEnableDebugLayer);

public:
	void BuildObjects();
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

private:
	void WaitForGPUComplete();

private:
	void ChangeSwapChainState();

public:
	static bool g_bMsaa4xEnable;
	static UINT g_nMsaa4xQualityLevels;

	static UINT g_uiClientWidth;
	static UINT g_uiClientHeight;

	const static UINT g_nSwapChainBuffers = 2;

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// DXGI
	ComPtr<IDXGIFactory4>	m_pdxgiFactory;
	ComPtr<IDXGISwapChain3>	m_pdxgiSwapChain;
	UINT					m_nSwapChainBufferIndex = 0;

	// RTV + DSV
	FrameResource m_FrameResources[g_nSwapChainBuffers];

	// Device
	ComPtr<ID3D12Device> m_pd3dDevice;

	// Command List + Allocator
	ComPtr<ID3D12CommandQueue>			m_pd3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pd3dCommandList;
	ComPtr<ID3D12CommandAllocator>		m_pd3dCommandAllocator;

	// Fence
	ComPtr<ID3D12Fence> m_pd3dFence;
	HANDLE				m_hFenceEvent;

	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;

	bool m_bEnableDebugLayer = false;
#pragma endregion
};

#define RESOURCE GameFramework::g_pResourceManager
#define RENDER GameFramework::g_pRenderManager