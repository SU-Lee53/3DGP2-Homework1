#include "stdafx.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "Material.h"
#include "StructuredBuffer.h"

RenderManager::RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, ASSUMED_REQUIRED_STRUCTURED_BUFFER_SIZE, sizeof(INSTANCE_DATA), true);

#else
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, ASSUMED_INSTANCE_COUNT, sizeof(INSTANCE_DATA), false);

#endif
	 
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = ASSUMED_REQUIRED_DESCRIPTOR_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	m_pd3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pd3dDescriptorHeap.GetAddressOf()));
}

void RenderManager::Add(std::shared_ptr<GameObject> pGameObject)
{
	INSTANCE_KEY key{ pGameObject->GetMesh(), pGameObject->GetMaterials(), key.pMesh->GetSubSetCount() };
	XMFLOAT4X4 xmf4x4InstanceData;
	XMStoreFloat4x4(&xmf4x4InstanceData, XMMatrixTranspose(XMLoadFloat4x4(&pGameObject->GetWorldMatrix())));

	auto it = m_InstanceIndexMap.find(key);
	if (it == m_InstanceIndexMap.end()) {
		m_InstanceIndexMap[key] = m_nInstanceIndex++;
		m_InstanceDatas.push_back({ key, {} });

		m_InstanceDatas[m_InstanceIndexMap[key]].second.emplace_back(xmf4x4InstanceData);
	}
	else {
		m_InstanceDatas[it->second].second.emplace_back(xmf4x4InstanceData);
	}

}

void RenderManager::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	// Shader 에 넘길 SHADER_VISIBLE 한 m_pd3dDescriptorHeap은 아래처럼 구성
	// SRV 는 인스턴싱용 StructuredBuffer
	// CBV 는 SubSet 별 Material 정보 (Mesh 마다 갯수 가 다름) -> SubSet 순서대로 기록되도록
	//  
	//  
	//                    +---------> m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	//                    |  
	//                    +-----+-----+------+-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+----- 
	//  Descriptor 구성   | CBV | CBV | SRV  | CBV || CBV | ... || CBV || CBV | ... || CBV || CBV || CBV || CBV | ...
	//                    +-----+-----+------+-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+-----
	//    Resource 단위   |   Scene   | Inst |      Mesh 1       |      Mesh 2       |          Mesh 3          | ...
	//                    +-----------+------+-------------------+-------------------+--------------------------+-----
	//                          |
	//                          +-----------> 여기는 Scene::Render() 에서 복사함
	//

	UINT uiSBufferOffset = 0;
	UINT uiDescriptorOffset = Scene::g_uiDescriptorCountPerScene;	// Per Scene 정보 2개 넣고 시작

	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dCPUHandle.ptr += (uiDescriptorOffset * GameFramework::g_uiDescriptorHandleIncrementSize);

	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	d3dGPUHandle.ptr += (uiDescriptorOffset * GameFramework::g_uiDescriptorHandleIncrementSize);

	for (auto&& [instanceKey, instanceData] : m_InstanceDatas) {
		m_InstanceDataSBuffer.UpdateData(instanceData, uiSBufferOffset);
		uiSBufferOffset += instanceData.size();
	}

	// Instance 행렬 정보를 한번에 GPU에 바인딩
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	m_pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, m_InstanceDataSBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;
	uiDescriptorOffset++;

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, d3dGPUHandle);
	d3dGPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

#else
	m_InstanceDataSBuffer.SetBufferToPipeline(pd3dCommandList, 2);

#endif // INSTANCING_USING_DESCRIPTOR_TABLE

	int nInstanceBase = 0;
	int nInstanceCount = 0;
	for (auto&& [instanceKey, instanceData] : m_InstanceDatas) {
		nInstanceCount = instanceData.size();
		pd3dCommandList->SetGraphicsRoot32BitConstant(3, nInstanceBase, 0);

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

			instanceKey.pMesh->Render(pd3dCommandList, i, nInstanceCount);
		}

		nInstanceBase += nInstanceCount;
	}

}

void RenderManager::SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const
{
	pd3dCommandList->SetDescriptorHeaps(1, m_pd3dDescriptorHeap.GetAddressOf());
}

void RenderManager::Clear()
{
#ifdef INSTANCES_IN_HASHMAP
	m_InstanceMap.clear();

#else
	m_InstanceIndexMap.clear();
	m_InstanceDatas.clear();
	m_nInstanceIndex = 0;

#endif
	
}

size_t RenderManager::GetMeshCount()
{
#ifdef INSTANCES_IN_HASHMAP
	return m_InstanceMap.size();

#else
	return m_InstanceIndexMap.size();

#endif

}
