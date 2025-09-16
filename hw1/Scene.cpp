#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"

Scene::Scene(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void Scene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CreateRootSignature(pd3dDevice);

	Material::PrepareShaders(pd3dDevice, m_pd3dRootSignature);

	std::shared_ptr<GameObject> pMi24Model = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Mi24.bin");
	__debugbreak();
}

void Scene::Update(float fTimeElapsed)
{
	if (m_pPlayer) {
		m_pPlayer->Update(fTimeElapsed);
	}

	for (auto& pObj : m_pGameObjects) {
		pObj->Update(fTimeElapsed);
	}
}

void Scene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());
	RENDER->SetDescriptorHeapToPipeline(pd3dCommandList);
	UpdateShaderVariable(pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = RENDER->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

	// Per Scene Descriptor ¿¡ º¹»ç
	pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, m_LightCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, m_pPlayer->GetCamera()->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(0, RENDER->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	if (m_pPlayer) {
		RENDER->Add(m_pPlayer);
	}

	for (auto& pObj : m_pGameObjects) {
		RENDER->Add(pObj);
	}

	RENDER->Render(pd3dCommandList);
}

void Scene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_LightCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_SCENE_DATA>::value, true);
}

void Scene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_SCENE_DATA data;
	data.nLights = m_pLights.size();

	for (int i = 0; i < m_pLights.size(); ++i) {
		data.LightData[i] = m_pLights[i]->MakeLightData();
	}

	m_LightCBuffer.UpdateData(pd3dCommandList, &data);
}

void Scene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE d3dPerSceneDescriptorRange{};
	{
		d3dPerSceneDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		d3dPerSceneDescriptorRange.NumDescriptors = 2;
		d3dPerSceneDescriptorRange.BaseShaderRegister = 0;		// c0, c1 : Camera, Lights
		d3dPerSceneDescriptorRange.RegisterSpace = 0;
		d3dPerSceneDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}

	D3D12_DESCRIPTOR_RANGE d3dMaterialDescriptorRange{};
	{
		d3dMaterialDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		d3dMaterialDescriptorRange.NumDescriptors = 1;
		d3dMaterialDescriptorRange.BaseShaderRegister = 2;		// c2 : Material
		d3dMaterialDescriptorRange.RegisterSpace = 0;
		d3dMaterialDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}
	
	D3D12_DESCRIPTOR_RANGE d3dInstancingDataDescriptorRange{};
	{
		d3dInstancingDataDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		d3dInstancingDataDescriptorRange.NumDescriptors = 1;
		d3dInstancingDataDescriptorRange.BaseShaderRegister = 0;	//	t0 : Texture
		d3dInstancingDataDescriptorRange.RegisterSpace = 0;
		d3dInstancingDataDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}

	D3D12_ROOT_PARAMETER d3dRootParameters[3];
	{
		// Per Scene
		d3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[0].DescriptorTable.pDescriptorRanges = &d3dPerSceneDescriptorRange;
		d3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Instance data
		d3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[1].DescriptorTable.pDescriptorRanges = &d3dInstancingDataDescriptorRange;
		d3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Material
		d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[2].DescriptorTable.pDescriptorRanges = &d3dMaterialDescriptorRange;
		d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc{};
	{
		d3dRootSignatureDesc.NumParameters = _countof(d3dRootParameters);
		d3dRootSignatureDesc.pParameters = d3dRootParameters;
		d3dRootSignatureDesc.NumStaticSamplers = 0;
		d3dRootSignatureDesc.pStaticSamplers = NULL;
		d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;
	}

	ComPtr<ID3DBlob> pd3dSignatureBlob = nullptr;
	ComPtr<ID3DBlob> pd3dErrorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());
	if (pd3dErrorBlob) {
		char* pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
		HWND hWnd = ::GetActiveWindow();
		MessageBoxA(hWnd, pErrorString, NULL, 0);
		OutputDebugStringA(pErrorString);
		__debugbreak();
	}


	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_pd3dRootSignature.GetAddressOf()));
}

std::shared_ptr<Camera> Scene::GetCamera()
{
	return m_pPlayer->GetCamera();
}
