#include "stdafx.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "Material.h"
#include "StructuredBuffer.h"

RenderManager::RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, MAX_INSTANCING_COUNT, sizeof(INSTANCE_DATA), true);

#else
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, MAX_INSTANCING_COUNT, sizeof(INSTANCE_DATA), false);

#endif



	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = MAX_INSTANCING_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	m_pd3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pd3dDescriptorHeap.GetAddressOf()));
}

void RenderManager::Add(std::shared_ptr<GameObject> pGameObject)
{
	INSTANCE_KEY key{};
	key.pMesh = pGameObject->GetMesh();
	key.pMaterials = pGameObject->GetMaterials();
	key.uiDescriptorCountPerInstance = key.pMesh->GetSubSetCount() + 1;	// SubSet �� Material + �ν��Ͻ̿� StructuredBuffer 1��

	XMFLOAT4X4 xmf4x4InstanceData;
	XMStoreFloat4x4(&xmf4x4InstanceData, XMMatrixTranspose(XMLoadFloat4x4(&pGameObject->GetWorldMatrix())));

	INSTANCE_DATA data{ xmf4x4InstanceData };
	m_InstanceMap[key].push_back(data);
}

void RenderManager::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	// Shader �� �ѱ� SHADER_VISIBLE �� m_pd3dDescriptorHeap�� �Ʒ�ó�� ����
	// SRV �� �ν��Ͻ̿� StructuredBuffer
	// CBV �� SubSet �� Material ���� (Mesh ���� ������ �ٸ�) -> SubSet ������� ��ϵǵ���
	//  
	//                     +---------> m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	//                     |  
	//                    +-----+-----+-----++-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+----- 
	//  Descriptor ����   | CBV | CBV | SRV || CBV || CBV | ... || SRV || CBV | ... || SRV || CBV || CBV || CBV | ...
	//                    +-----+-----+-----++-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+-----
	//    Resource ����   |   Scene   |         Mesh 1           |      Mesh 2       |          Mesh 3          | ...
	//                    +-----------+--------------------------+-------------------+--------------------------+-----
	//                          |
	//                          +-----------> ����� Scene::Render() ���� ������
	//

	UINT uiSBufferOffset = 0;
	UINT uiDescriptorOffset = Scene::g_uiDescriptorCountPerScene;	// Per Scene ���� 2�� �ְ� ����

	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dCPUHandle.ptr += (uiDescriptorOffset * GameFramework::g_uiDescriptorHandleIncrementSize);

	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	d3dGPUHandle.ptr += (uiDescriptorOffset * GameFramework::g_uiDescriptorHandleIncrementSize);

	// Per Object ���� Descriptor�� ����
	for (auto&& [instanceKey, instanceData] : m_InstanceMap) {
		m_InstanceDataSBuffer.UpdateData(instanceData, uiSBufferOffset);

#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
		m_pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, m_InstanceDataSBuffer.GetCPUDescriptorHandle(uiSBufferOffset), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;
		uiDescriptorOffset++;

		pd3dCommandList->SetGraphicsRootDescriptorTable(2, d3dGPUHandle);
		d3dGPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

#else
		m_InstanceDataSBuffer.SetBufferToPipeline(pd3dCommandList, 0, 0, 2);

#endif
		for (int i = 0; i < instanceKey.pMaterials.size(); ++i) {
			instanceKey.pMaterials[i]->UpdateShaderVariable(pd3dCommandList);
			m_pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, instanceKey.pMaterials[i]->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;
			uiDescriptorOffset++;
		}

		for (int i = 0; i < instanceKey.pMaterials.size(); ++i) {
			instanceKey.pMaterials[i]->OnPrepareRender(pd3dCommandList);

			pd3dCommandList->SetGraphicsRootDescriptorTable(1, d3dGPUHandle);
			d3dGPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

			instanceKey.pMesh->Render(pd3dCommandList, i, instanceData.size());
		}

		uiSBufferOffset += instanceData.size();
	}
}

void RenderManager::SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const
{
	pd3dCommandList->SetDescriptorHeaps(1, m_pd3dDescriptorHeap.GetAddressOf());
}

void RenderManager::Clear()
{
	m_InstanceMap.clear();
}
