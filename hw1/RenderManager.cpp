#include "stdafx.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "Material.h"
#include "StructuredBuffer.h"

RenderManager::RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pInstanceDataSBuffer = std::make_shared<StructuredBuffer>(pd3dDevice, pd3dCommandList, MAX_INSTANCING_COUNT, sizeof(INSTANCE_DATA));
}

void RenderManager::Add(std::shared_ptr<GameObject> pGameObject)
{
	INSTANCE_KEY key{};
	key.pMesh = pGameObject->GetMesh();
	key.pMaterials = pGameObject->GetMaterials();

	XMFLOAT4X4 xmf4x4InstanceData;
	XMStoreFloat4x4(&xmf4x4InstanceData, XMMatrixTranspose(XMLoadFloat4x4(&pGameObject->GetWorldMatrix())));

	INSTANCE_DATA data{ xmf4x4InstanceData };
	m_InstanceMap[key].push_back(data);
}

void RenderManager::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	UINT uiSBufferOffset = 0;
	for (auto&& [instanceKey, instanceData] : m_InstanceMap) {
		m_pInstanceDataSBuffer->UpdateData(instanceData, uiSBufferOffset);

		for (int i = 0; i < instanceKey.pMaterials.size(); ++i) {
			instanceKey.pMaterials[i]->OnPrepareRender(pd3dCommandList);

			instanceKey.pMaterials[i]->UpdateShaderVariable(pd3dCommandList);
			instanceKey.pMaterials[i]->SetMaterialToPipeline(pd3dCommandList, 1);

			m_pInstanceDataSBuffer->SetBufferToPipeline(pd3dCommandList, uiSBufferOffset, sizeof(INSTANCE_DATA), 2);

			instanceKey.pMesh->Render(pd3dCommandList, i, instanceData.size());
		}

		uiSBufferOffset += instanceData.size();
	}
}

void RenderManager::Clear()
{
	m_InstanceMap.clear();
}
