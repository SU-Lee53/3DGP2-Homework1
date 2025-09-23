#pragma once
#include "Light.h"
#include "Player.h"

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
	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void ReleaseUploadBuffers();

	bool ProcessInput(UCHAR* pKeysBuffer);
	void Update(float fTimeElapsed);
	void Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	virtual void CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	virtual void UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

private:
	void CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice);

public:
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

public:
	std::shared_ptr<Camera> GetCamera() const;
	const ConstantBuffer& GetCBuffer() const { return m_LightCBuffer; }
	std::shared_ptr<Player> GetPlayer() const;

private:
	std::shared_ptr<Player>						m_pPlayer;
	std::vector<std::shared_ptr<GameObject>>	m_pGameObjects;
	std::vector<std::shared_ptr<Light>>			m_pLights;
	XMFLOAT4									m_xmf4GlobalAmbient;

	ConstantBuffer								m_LightCBuffer;

private:
	ComPtr<ID3D12RootSignature> m_pd3dRootSignature;

public:
	const static UINT g_uiDescriptorCountPerScene = 2;	// Camera + Lights
	int m_nInstance = 0;

private:
	// Model Cache
	std::shared_ptr<GameObject> pApacheModel = nullptr;
	std::shared_ptr<GameObject> pGunshipModel = nullptr;
	std::shared_ptr<GameObject> pSuperCobraModel = nullptr;
	std::shared_ptr<GameObject> pHummerModel = nullptr;
	std::shared_ptr<GameObject> pAbramsModel = nullptr;
	std::shared_ptr<GameObject> pMi24Model = nullptr;
};

