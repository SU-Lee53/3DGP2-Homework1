#pragma once

class Mesh;
class Material;
class StructuredBuffer;

#define MAX_INSTANCING_COUNT 300000

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
		return h1 ^ h2;	// XOR 연산을 이용한 해시 결합
	}
};

class RenderManager {
public:
	RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	void Add(std::shared_ptr<GameObject> pGameObject);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Clear();

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_pd3dDescriptorHeap; }
	void SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const;

private:
	std::unordered_map<INSTANCE_KEY, std::vector<INSTANCE_DATA>> m_InstanceMap;

	ComPtr<ID3D12Device>			m_pd3dDevice = nullptr;	// GameFramewok::m_pd3dDevice 의 참조
	ComPtr<ID3D12DescriptorHeap>	m_pd3dDescriptorHeap = nullptr;
	StructuredBuffer				m_InstanceDataSBuffer;

};
