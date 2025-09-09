#pragma once

class GameObject;
class Light;

class Scene {
public:
	void BuildObjects(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Update(float fTimeElapsed);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

private:
	std::vector<std::shared_ptr<GameObject>> m_pGameObjects;
	std::vector<std::shared_ptr<Light>> m_pLights;

};

