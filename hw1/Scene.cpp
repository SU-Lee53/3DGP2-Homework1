#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"

Scene::Scene(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_cbLights = std::make_shared<ConstantBuffer>(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_SCENE_DATA>::value);
}

void Scene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Material::PrepareShaders(pd3dDevice, m_pd3dRootSignature);

	std::shared_ptr<GameObject> pMi24Model = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Mi24.bin");
	__debugbreak();
}

void Scene::Update(float fTimeElapsed)
{
}

void Scene::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	UpdateShaderVariable(pd3dCommandList);

	for (auto& pObj : m_pGameObjects) {
		RENDER->Add(pObj);
	}

	m_cbLights->SetBufferToPipeline(pd3dCommandList, 3);
}

void Scene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_SCENE_DATA data;
	data.nLights = m_pLights.size();

	for (int i = 0; i < m_pLights.size(); ++i) {
		data.LightData[i] = m_pLights[i]->MakeLightData();
	}

	m_cbLights->UpdateData(pd3dCommandList, &data);
}

void Scene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	D3D12_ROOT_PARAMETER d3dRootParameters[4];

	// Camera
	d3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	d3dRootParameters[0].Descriptor.ShaderRegister = 1;
	d3dRootParameters[0].Descriptor.RegisterSpace = 0;
	d3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Material
	d3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	d3dRootParameters[1].Descriptor.ShaderRegister = 2;
	d3dRootParameters[1].Descriptor.RegisterSpace = 0;
	d3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Instance data
	d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	d3dRootParameters[2].Descriptor.ShaderRegister = 0;
	d3dRootParameters[2].Descriptor.RegisterSpace = 0;
	d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Scene(Lights)
	d3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	d3dRootParameters[3].Descriptor.ShaderRegister = 3;
	d3dRootParameters[3].Descriptor.RegisterSpace = 0;
	d3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_pd3dRootSignature.GetAddressOf()));
}