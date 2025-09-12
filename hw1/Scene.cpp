#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"

void Scene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	std::shared_ptr<GameObject> pMi24Model = GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, "../Models/Mi24.bin");
	__debugbreak();
}

void Scene::Update(float fTimeElapsed)
{
}

void Scene::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}
