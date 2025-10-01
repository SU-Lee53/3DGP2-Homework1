#pragma once

class Mesh;
class Material;
class StructuredBuffer;

#define MAX_INSTANCING_COUNT						10000
#define ASSUMED_MESH_PER_INSTANCE					30
#define ASSUMED_MATERIAL_PER_MESH					2
#define ASSUMED_REQUIRED_STRUCTURED_BUFFER_SIZE		MAX_INSTANCING_COUNT * ASSUMED_MESH_PER_INSTANCE * ASSUMED_MATERIAL_PER_MESH

#define ASSUMED_OBJECT_TYPE_PER_SCENE				2
#define ASSUMED_REQUIRED_DESCRIPTOR_COUNT			Scene::g_uiDescriptorCountPerScene + 1 + ASSUMED_MESH_PER_INSTANCE * ASSUMED_MATERIAL_PER_MESH * ASSUMED_OBJECT_TYPE_PER_SCENE

struct INSTANCE_DATA {
	XMFLOAT4X4 xmf4x4GameObject;
};

struct INSTANCE_KEY {
	std::shared_ptr<Mesh>					pMesh;
	std::vector<std::shared_ptr<Material>>	pMaterials;
	UINT									uiDescriptorCountPerInstance;

	bool operator==(const INSTANCE_KEY& other) const noexcept {
		return pMesh == other.pMesh && pMaterials == other.pMaterials;
	}
};

template<>
struct std::hash<INSTANCE_KEY> {
	size_t operator()(const INSTANCE_KEY& key) const {
		size_t h1 = std::hash<std::shared_ptr<Mesh>>{}(key.pMesh);
		size_t h2 = std::hash<std::shared_ptr<Material>>{}(key.pMaterials[0]);
		return h1 ^ h2;
	}
};

class RenderManager {
public:
	RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	void Add(std::shared_ptr<GameObject> pGameObject);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Clear();

	size_t GetMeshCount();
	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_pd3dDescriptorHeap; }
	void SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const;

private:
	std::unordered_map<INSTANCE_KEY, UINT> m_InstanceIndexMap;
	std::vector<std::pair<INSTANCE_KEY, std::vector<INSTANCE_DATA>>> m_InstanceDatas;
	UINT m_nInstanceIndex = 0;
	

	ComPtr<ID3D12Device>			m_pd3dDevice = nullptr;	// GameFramewok::m_pd3dDevice ÀÇ ÂüÁ¶
	ComPtr<ID3D12DescriptorHeap>	m_pd3dDescriptorHeap = nullptr;
	StructuredBuffer				m_InstanceDataSBuffer;

};
