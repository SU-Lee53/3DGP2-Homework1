#pragma once
#include "Light.h"

class GameObject;

#define MAX_LIGHTS 16

struct CB_SCENE_DATA {
	CB_LIGHT_DATA	LightData[MAX_LIGHTS];
	XMFLOAT4		globalAmbientLight;
	int				nLights;
};

class Scene {
public:
	Scene(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

public:
	void BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Update(float fTimeElapsed);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	void UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

private:
	void CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice);

private:
	std::vector<std::shared_ptr<GameObject>>	m_pGameObjects;
	std::vector<std::shared_ptr<Light>>			m_pLights;
	XMFLOAT4									m_xmf4GlobalAmbient;

	std::shared_ptr<ConstantBuffer>				m_cbLights;

private:
	ComPtr<ID3D12RootSignature> m_pd3dRootSignature;
};

