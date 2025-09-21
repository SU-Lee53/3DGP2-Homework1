#include "stdafx.h"
#include "GameFramework.h"

bool GameFramework::g_bMsaa4xEnable = false;
UINT GameFramework::g_nMsaa4xQualityLevels = 0;
long GameFramework::g_nClientWidth = 0;
long GameFramework::g_nClientHeight = 0;
UINT GameFramework::g_uiDescriptorHandleIncrementSize = 0;

std::unique_ptr<ResourceManager> GameFramework::g_pResourceManager = nullptr;
std::unique_ptr<RenderManager> GameFramework::g_pRenderManager = nullptr;

GameFramework::GameFramework(HINSTANCE hInstance, HWND hWnd, UINT uiWidth, UINT uiHeight, bool bEnableDebugLayer)
{
	m_hWnd = hWnd;
	m_hInstance = hInstance;
	m_bEnableDebugLayer = bEnableDebugLayer;

	g_nClientWidth = uiWidth;
	g_nClientHeight = uiHeight;

	CreateFactory();
	CreateDevice();
	CreateFence();
	CreateCommandList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	m_d3dViewport = { 0.f, 0.f, (float)g_nClientWidth, (float)g_nClientHeight, 0.f, 1.f };
	m_d3dScissorRect = { 0, 0, (LONG)g_nClientWidth, (LONG)g_nClientHeight };

	m_tstrFrameRate = L"3DGP2-Homework1";

	g_pResourceManager = std::make_unique<ResourceManager>();
	g_pRenderManager = std::make_unique<RenderManager>(m_pd3dDevice, m_pd3dCommandList);
	g_uiDescriptorHandleIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	BuildObjects();
}

void GameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	// Build
	{
		m_pScene = std::make_shared<Scene>(m_pd3dDevice, m_pd3dCommandList);
		m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	}

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get()};
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGPUComplete();

	m_pScene->ReleaseUploadBuffers();
}

void GameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	
	if (GetKeyboardState(pKeysBuffer) && m_pScene) {
		bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	}
		
	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;
		if (pKeysBuffer['W'] & 0xF0)	dwDirection |= MOVE_DIR_FORWARD;
		if (pKeysBuffer['S'] & 0xF0)	dwDirection |= MOVE_DIR_BACKWARD;
		if (pKeysBuffer['A'] & 0xF0)	dwDirection |= MOVE_DIR_LEFT;
		if (pKeysBuffer['D'] & 0xF0)	dwDirection |= MOVE_DIR_RIGHT;
		if (pKeysBuffer['E'] & 0xF0)	dwDirection |= MOVE_DIR_UP;
		if (pKeysBuffer['Q'] & 0xF0)	dwDirection |= MOVE_DIR_DOWN;

		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			auto pPlayer = m_pScene->GetPlayer();
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			if (dwDirection) pPlayer->Move(dwDirection, 1.5f, true);
		}
	}
}

void GameFramework::Update()
{
	//OutputDebugStringA(std::format("==================\nTimeElapsed : {}\n==================\n", m_GameTimer.GetTimeElapsed()).c_str());
	m_GameTimer.Tick(0.0f);
	ProcessInput();
	m_pScene->Update(m_GameTimer.GetTimeElapsed());
}

void GameFramework::Render()
{
	RENDER->Clear();

	RenderBegin();

	{
		// TODO: Render Logic
		m_pScene->Render(m_pd3dDevice, m_pd3dCommandList);

		RENDER->Render(m_pd3dCommandList);
	}

	RenderEnd();
	Present();
	MoveToNextFrame();

	TSTRING tstrFrameRate;
	m_GameTimer.GetFrameRate(L"3DGP-Homework1", tstrFrameRate);

	tstrFrameRate = std::format(L"{} (Instance Count : {})", tstrFrameRate, m_pScene->m_nInstance);

	::SetWindowText(m_hWnd, tstrFrameRate.data());
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
		dxgiSwapChainDesc.BufferDesc.Width = GameFramework::g_nClientWidth;
		dxgiSwapChainDesc.BufferDesc.Height = GameFramework::g_nClientHeight;
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

	CreateRenderTargetViews();

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

void GameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void GameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = g_nClientWidth;
	d3dResourceDesc.Height = g_nClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (g_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (g_bMsaa4xEnable) ? (g_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void GameFramework::WaitForGPUComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
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
	dxgiTargetParameters.Width = g_nClientWidth;
	dxgiTargetParameters.Height = g_nClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < g_nSwapChainBuffers; i++)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i].Reset();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(g_nSwapChainBuffers, g_nClientWidth, g_nClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void GameFramework::RenderBegin()
{
	HRESULT hr;

	hr = m_pd3dCommandAllocator->Reset();
	hr = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);
	if (FAILED(hr)) {
		__debugbreak();
	}

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

}

void GameFramework::RenderEnd()
{
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGPUComplete();
}

void GameFramework::Present()
{
	/*DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;

	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);*/


	m_pdxgiSwapChain->Present(0, 0);

}

void GameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void GameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) {
		m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	}

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
	if (m_pScene) {
		m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	}

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
