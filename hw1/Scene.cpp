#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"
#include "Light.h"

Scene::Scene(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void Scene::BuildDefaultLightsAndMaterials()
{
	m_Lights.reserve(4);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	PointLight light1{};
	light1.m_bEnable = true;
	light1.m_fRange = 1000.0f;
	light1.m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	light1.m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	light1.m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	light1.m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	light1.m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	light1.m_fAttenuation0 = 1.0f;
	light1.m_fAttenuation1 = 0.001f;
	light1.m_fAttenuation2 = 0.0001f;
	m_Lights.push_back(light1);

	SpotLight light2{};
	light2.m_bEnable = true;
	light2.m_fRange = 500.0f;
	light2.m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light2.m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	light2.m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	light2.m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	light2.m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	light2.m_fAttenuation0 = 1.0f;
	light2.m_fAttenuation1 = 0.01f;
	light2.m_fAttenuation2 = 0.0001f;
	light2.m_fFalloff = 8.0f;
	light2.m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	light2.m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_Lights.push_back(light2);

	DirectionalLight light3{};
	light3.m_bEnable = true;
	light3.m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light3.m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	light3.m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	light3.m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

	SpotLight light4{};
	light4.m_bEnable = true;
	light4.m_fRange = 600.0f;
	light4.m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light4.m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	light4.m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	light4.m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	light4.m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	light4.m_fAttenuation0 = 1.0f;
	light4.m_fAttenuation1 = 0.01f;
	light4.m_fAttenuation2 = 0.0001f;
	light4.m_fFalloff = 8.0f;
	light4.m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	light4.m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_Lights.push_back(light4);
}

void Scene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CreateRootSignature(pd3dDevice);
	Material::PrepareShaders(pd3dDevice, m_pd3dRootSignature);
	BuildDefaultLightsAndMaterials();


	std::shared_ptr<GameObject> pApacheModel = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Apache.bin");
	std::shared_ptr<GameObject> pGunshipModel = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Gunship.bin");
	std::shared_ptr<GameObject> pSuperCobraModel = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/SuperCobra.bin");
	std::shared_ptr<GameObject> pMi24Model = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Mi24.bin");	
	std::shared_ptr<GameObject> pHummerModel = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/Hummer.bin");
	std::shared_ptr<GameObject> pAbramsModel = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/M26.bin");


	__debugbreak();



	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

bool Scene::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
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
	data.nLights = m_Lights.size();

	for (int i = 0; i < m_Lights.size(); ++i) {
		data.LightData[i] = m_Lights[i].MakeLightData();
	}

	m_LightCBuffer.UpdateData(pd3dCommandList, &data);
}

bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': m_pPlayer->MoveForward(+1.0f); break;
		case 'S': m_pPlayer->MoveForward(-1.0f); break;
		case 'A': m_pPlayer->MoveStrafe(-1.0f); break;
		case 'D': m_pPlayer->MoveStrafe(+1.0f); break;
		case 'Q': m_pPlayer->MoveUp(+1.0f); break;
		case 'R': m_pPlayer->MoveUp(-1.0f); break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return false;
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
	
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	D3D12_DESCRIPTOR_RANGE d3dInstancingDataDescriptorRange{};
	{
		d3dInstancingDataDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		d3dInstancingDataDescriptorRange.NumDescriptors = 1;
		d3dInstancingDataDescriptorRange.BaseShaderRegister = 0;	//	t0 : Texture
		d3dInstancingDataDescriptorRange.RegisterSpace = 0;
		d3dInstancingDataDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}
#endif

	D3D12_ROOT_PARAMETER d3dRootParameters[3];
	{
		// Per Scene
		d3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[0].DescriptorTable.pDescriptorRanges = &d3dPerSceneDescriptorRange;
		d3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Material
		d3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[1].DescriptorTable.pDescriptorRanges = &d3dMaterialDescriptorRange;
		d3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Instance data
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
		d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[2].DescriptorTable.pDescriptorRanges = &d3dInstancingDataDescriptorRange;
		d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

#else
		d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		d3dRootParameters[2].Descriptor.ShaderRegister = 0;
		d3dRootParameters[2].Descriptor.RegisterSpace = 0;
		d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

#endif // INSTANCING_USING_DESCRIPTOR_TABLE

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

std::shared_ptr<Camera> Scene::GetCamera() const
{
	return m_pPlayer->GetCamera();
}

std::shared_ptr<Player> Scene::GetPlayer() const
{
	return m_pPlayer;
}
