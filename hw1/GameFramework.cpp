#include "stdafx.h"
#include "GameFramework.h"

bool GameFramework::g_bMsaa4xEnable = false;
UINT GameFramework::g_nMsaa4xQualityLevels = 0;
UINT GameFramework::g_uiClientWidth = 0;
UINT GameFramework::g_uiClientHeight = 0;

GameFramework::GameFramework(HINSTANCE hInstance, HWND hWnd, UINT uiWidth, UINT uiHeight, bool bEnableDebugLayer)
{
	m_hWnd = hWnd;
	m_hInstance = hInstance;
	m_bEnableDebugLayer = bEnableDebugLayer;

	g_uiClientWidth = uiWidth;
	g_uiClientHeight = uiHeight;

	CreateFactory();
	CreateDevice();
	CreateFence();
	CreateCommandList();
	CreateSwapChain();

	m_d3dViewport = { 0.f, 0.f, (float)g_uiClientWidth, (float)g_uiClientHeight, 0.f, 1.f };
	m_d3dScissorRect = { 0, 0, (LONG)g_uiClientWidth, (LONG)g_uiClientHeight };

	m_tstrFrameRate = L"3DGP2-Homework1";

	m_pScene = std::make_shared<Scene>();
	m_pScene->BuildObjects(m_pd3dCommandList);

}

void GameFramework::CreateFactory()
{
	HRESULT hr;

	UINT ndxgiFactoryFlags = 0;
	if (m_bEnableDebugLayer) {
		ComPtr<ID3D12Debug> pd3dDebugController = nullptr;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(pd3dDebugController.GetAddressOf()));
		if (FAILED(hr)) {
			__debugbreak();
		}
		ndxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		if (pd3dDebugController) {
			pd3dDebugController->EnableDebugLayer();
		}
	}

	CreateDXGIFactory2(ndxgiFactoryFlags, IID_PPV_ARGS(m_pdxgiFactory.GetAddressOf()));


}

void GameFramework::CreateDevice()
{
	HRESULT hr;

	// Create DXGIAdapter
	ComPtr<IDXGIAdapter> pdxgiAdapter = nullptr;
	DXGI_ADAPTER_DESC1 adapterDesc{};
	size_t bestMemory = 0;
	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		ComPtr<IDXGIAdapter1> pCurAdapter = nullptr;
		if (m_pdxgiFactory->EnumAdapters1(adapterIndex, pCurAdapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND) break;

		DXGI_ADAPTER_DESC1 curAdapterDesc{};
		if (FAILED(pCurAdapter->GetDesc1(&curAdapterDesc))) __debugbreak();

		size_t curMemory = curAdapterDesc.DedicatedVideoMemory / (1024 * 1024);

		if (curMemory > bestMemory)
		{
			bestMemory = curMemory;
			pdxgiAdapter = pCurAdapter;
			adapterDesc = curAdapterDesc;
		}
	}

	std::array<D3D_FEATURE_LEVEL, 3> featureLevels =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	// Device
	for (int i = 0; i < featureLevels.size(); i++) {
		hr = D3D12CreateDevice(pdxgiAdapter.Get(), featureLevels[i], IID_PPV_ARGS(m_pd3dDevice.GetAddressOf()));
		if (SUCCEEDED(hr))
			break;
	}

	if (!m_pd3dDevice) {
		m_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pdxgiAdapter.GetAddressOf()));
		hr = D3D12CreateDevice(pdxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pd3dDevice.GetAddressOf()));
		if (FAILED(hr)) {
			__debugbreak();
		}
	}

	// MSAA
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	{
		d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dMsaaQualityLevels.SampleCount = 4;
		d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		d3dMsaaQualityLevels.NumQualityLevels = 0;
	}

	hr = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	if (SUCCEEDED(hr)) {
		g_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
		g_bMsaa4xEnable = (g_nMsaa4xQualityLevels > 1) ? true : false;
	}
	else {
		::OutputDebugString(L"Device doesn't support MSAA4x");
	}
}

void GameFramework::CreateFence()
{
	HRESULT hr;

	hr = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pd3dFence.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

}

void GameFramework::CreateSwapChain()
{

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	{
		dxgiSwapChainDesc.BufferCount = g_nSwapChainBuffers;
		dxgiSwapChainDesc.BufferDesc.Width = GameFramework::g_uiClientWidth;
		dxgiSwapChainDesc.BufferDesc.Height = GameFramework::g_uiClientHeight;
		dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxgiSwapChainDesc.OutputWindow = m_hWnd;
		dxgiSwapChainDesc.SampleDesc.Count = (g_bMsaa4xEnable) ? 4 : 1;
		dxgiSwapChainDesc.SampleDesc.Quality = (g_bMsaa4xEnable) ? (g_nMsaa4xQualityLevels - 1) : 0;
		dxgiSwapChainDesc.Windowed = TRUE;

		// Set backbuffer resolution as fullscreen resolution.
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}

	ComPtr<IDXGISwapChain> pSwapChain;
	HRESULT hr = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(), &dxgiSwapChainDesc, pSwapChain.GetAddressOf());
	if (FAILED(hr)) {
		__debugbreak();
	}

	pSwapChain->QueryInterface(IID_PPV_ARGS(m_pdxgiSwapChain.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	for (int i = 0; i < g_nSwapChainBuffers; ++i) {
		m_FrameResources[i].Create(m_pd3dDevice, m_pdxgiSwapChain, i);
	}

}

void GameFramework::CreateCommandList()
{
	HRESULT hr;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	{
		cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	}
	hr = m_pd3dDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(m_pd3dCommandQueue.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	// Create Command Allocator
	hr = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pd3dCommandAllocator.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	// Create Command List
	hr = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS(m_pd3dCommandList.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}

	// Close Command List(default is opened)
	hr = m_pd3dCommandList->Close();
}

void GameFramework::WaitForGPUComplete()
{
	UINT64 nFenceValue = ++m_FrameResources[m_nSwapChainBufferIndex].nFenceValue;
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void GameFramework::ChangeSwapChainState()
{
	WaitForGPUComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = g_uiClientWidth;
	dxgiTargetParameters.Height = g_uiClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < g_nSwapChainBuffers; i++) 
		if (m_FrameResources[i].pd3dRenderTargetBuffer)
			m_FrameResources[i].pd3dRenderTargetBuffer.Reset();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(g_nSwapChainBuffers, g_uiClientWidth, g_uiClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	for (int i = 0; i < g_nSwapChainBuffers; ++i) {
		m_FrameResources[i].Create(m_pd3dDevice, m_pdxgiSwapChain, i);
	}
}

void GameFramework::Update()
{
	m_GameTimer.Tick(0.0f);

	m_pScene->Update(m_GameTimer.GetTimeElapsed());
}

void GameFramework::RenderBegin()
{
	HRESULT hr;

	hr = m_pd3dCommandAllocator->Reset();
	hr = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);
	if (FAILED(hr)) {
		__debugbreak();
	}

	m_FrameResources[m_nSwapChainBufferIndex].ResourceBarrier(m_pd3dCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDSVHandle = FrameResource::pd3dDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dCommandList->OMSetRenderTargets(1, &m_FrameResources[m_nSwapChainBufferIndex].d3dRTVHandle, TRUE, &d3dDSVHandle);

	float pfClearColor[4] = { 0.f, 0.f, 0.f, 1.f };
	m_pd3dCommandList->ClearRenderTargetView(m_FrameResources[m_nSwapChainBufferIndex].d3dRTVHandle, pfClearColor, 0, NULL);
	m_pd3dCommandList->ClearDepthStencilView(d3dDSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);

}

void GameFramework::Render()
{
	RenderBegin();

	{
		// TODO: Render Logic
	}

	RenderEnd();
	Present();
	MoveToNextFrame();

	TSTRING tstrFrameRate;
	m_GameTimer.GetFrameRate(L"3DGP-Homework1", tstrFrameRate);
	::SetWindowText(m_hWnd, tstrFrameRate.data());
}

void GameFramework::RenderEnd()
{
	m_FrameResources[m_nSwapChainBufferIndex].ResourceBarrier(m_pd3dCommandList, D3D12_RESOURCE_STATE_PRESENT);

	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGPUComplete();
}

void GameFramework::Present()
{
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;

	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);



}

void GameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_FrameResources[m_nSwapChainBufferIndex].nFenceValue;
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void GameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void GameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			//m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		case VK_F5:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT GameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}

	return 0;
}
